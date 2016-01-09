#include "shp.h"
#include "renderer.h"
#include "vfs/vfs.h"
#include <stdio.h>

#define SHP_HEADER_SIZE 14

enum ShpFormat {
    SHP_FMT_80 = 0x80,
    SHP_FMT_48 = 0x48,
    SHP_FMT_40 = 0x40,
    SHP_FMT_20 = 0x20
};

struct shp_offset {
    guint32 offset;
    guint32 ref_offset;
    enum ShpFormat format;
    enum ShpFormat ref_format;
};

shp_t *shp_init(vfs_file_t *fp) {
    gchar buf[SHP_HEADER_SIZE];
    guint16 num_images;
    shp_t *shp;

    //vfs_file_seek(fp, 0, VFS_SEEK_SET);
    vfs_file_read(buf, SHP_HEADER_SIZE, fp); 
    num_images = *(guint16 *)&buf[0];
    shp = g_malloc(sizeof(shp_t) + (num_images * sizeof(struct shp_offset)));
    shp->num_images = num_images;
    shp->width = *(guint16 *)&buf[6];
    shp->height = *(guint16 *)&buf[8];
    shp->offsets = (struct shp_offset *)(shp + 1);
    shp->fp = fp;
    shp->refindex = g_hash_table_new_full(g_int_hash, g_int_equal, g_free, g_free);

    for (int i = 0; i < num_images; ++i) {
        vfs_file_read((gchar *)&shp->offsets[i], 8, fp);
        shp->offsets[i].format = shp->offsets[i].offset >> 24;
        shp->offsets[i].offset &= 0x00FFFFFF;
        shp->offsets[i].ref_format = shp->offsets[i].ref_offset >> 24;
        shp->offsets[i].ref_offset &= 0x00FFFFFF;
        // TODO a bunch of error checking
        guint32 *offset = g_new(guint32, 1);
        *offset = shp->offsets[i].offset;
        gint *idx = g_new(gint, 1);
        *idx = i;
        g_hash_table_insert(shp->refindex, offset, idx);
    }

    return shp;
}

void shp_free(shp_t *shp) {
    vfs_file_close(shp->fp); // Not sure if shp_free should do this but whatever
    g_hash_table_destroy(shp->refindex);
    g_free(shp);
}

void decode_format80(gchar *dest, const gchar *data);
void decode_format40(gchar *dest, const gchar *data);

void shp_decompress(gchar *dest, shp_t *shp, gint index) {
    struct shp_offset *off = &shp->offsets[index];
    gsize data_length;
    if (index == shp->num_images - 1) {
        data_length = vfs_file_size(shp->fp) - off->offset;
    } else {
        data_length = (off+1)->offset - off->offset;
    }
    gchar* buf = g_malloc(data_length); 
    vfs_file_seek(shp->fp, off->offset, VFS_SEEK_SET);
    vfs_file_read(buf, data_length, shp->fp);

    if (off->format == SHP_FMT_80) {
        decode_format80(dest, buf);
    } else {
        if (off->format == SHP_FMT_40) {
            gpointer index = g_hash_table_lookup(shp->refindex, &off->ref_offset);
            shp_decompress(dest, shp, *(gint *)index);
        } else {
            shp_decompress(dest, shp, index - 1);
        }
        decode_format40(dest, buf);
    }
    g_free(buf);
}

void copy(gchar *dest, const gchar *src, gsize count) {
    while (count--) {
        *dest++ = *src++;
    }
}

void fill(gchar *dest, char value, gsize count) {
    while (count--) {
        *dest++ = value;
    }
}

void decode_format80(gchar *dest, const gchar *data) {
    guint16 count;
    guint16 pos;
    guchar *dp = (guchar *)dest;
    const guchar *sp = (guchar *)data;
    //fprintf(stderr, "---------------------------------\n");
    //fprintf(stderr, "Decoding format80...\n");
    //fprintf(stderr, "---------------------------------\n");
    while (1) {
        if (*sp == 0xFE) {
            //4
            count = *(guint16*)++sp;
            sp += 2;
            //fprintf(stderr, "Filling %u bytes with %d\n", count, *sp);
            fill((gchar*)dp, *sp++, count);
        } else if (*sp == 0xFF) {
            //5
            count = *(guint16*)++sp;
            sp += 2;
            pos = *(guint16*)sp;
            sp += 2;
            //fprintf(stderr, "Big copying %u bytes from abs. pos %u\n", count, pos);
            copy((gchar*)dp, &dest[pos], count);
        } else if (*sp >> 7 == 0) {
            //2
            count = ((*sp & 0x7F) >> 4) + 3;
            pos = ((*sp++) & 0x0F) << 8;
            pos |= *sp++;
            //fprintf(stderr, "Copying %u bytes from rel. pos -%u\n", count, pos);
            copy((gchar*)dp, (gchar*)dp-pos, count);
        } else if (*sp >> 6 == 0x02) {
            //1
            count = (*sp++ & 0x3F);
            if (!count) break;
            //fprintf(stderr, "Copying %u bytes from source\n", count);
            copy((gchar*)dp, (gchar*)sp, count);
            sp += count;
        } else if (*sp >> 6 == 0x03) {
            //3
            count = (*sp++ & 0x3F) + 3;
            pos = *(guint16*)sp;
            //fprintf(stderr, "Copying %u bytes from abs pos %u\n", count, pos);
            copy((gchar*)dp, &dest[pos], count);
            sp += 2;
        }
        dp += count;
    }
}

void xor(gchar *dest, const char *src, gsize count) {
    while (count--) {
        *dest++ ^= *src++;
    }
}

void xor_fill(gchar *dest, char value, gsize count) {
    while (count--) {
        *dest++ ^= value;
    }
}

void decode_format40(gchar *dest, const gchar *data) {
    guint16 count;
    guchar *dp = (guchar *)dest;
    const guchar *sp = (guchar *)data;
    //fprintf(stderr, "---------------------------------\n");
    //fprintf(stderr, "Decoding format40...\n");
    //fprintf(stderr, "---------------------------------\n");
    while (1) {
        if (*sp & 0x80) {
            if (*sp != 0x80) {
                count = *sp++ & 0x7F;
                dp += count;
                //fprintf(stderr, "Skipping %u bytes\n", count);
            } else {
                count = *(guint16*)++sp;
                if (!count) break;
                sp += 2;
                switch ((count & 0xC000) >> 14) {
                    case 0:
                    case 1:
                        //fprintf(stderr, "Big skipping %u bytes\n", count);
                        dp += count;
                        break;
                    case 2:
                        count = count & 0x3FFF;
                        xor((gchar*)dp, (gchar*)sp, count);
                        //fprintf(stderr, "Big XOR-ing %u bytes\n", count);
                        sp+=count;
                        dp+=count;
                        break;
                    case 3:
                        count = count & 0x3FFF;
                        //fprintf(stderr, "Big XOR-filling %u bytes with %d\n", count, *sp);
                        xor_fill((gchar*)dp, *sp++, count);
                        dp+=count;
                        break;
                }
            }
        } else {
            count = *sp++;
            if (!count) {
                count = *sp++;
                //fprintf(stderr, "XOR-filling %u bytes with %d\n", count, *sp);
                xor_fill((gchar*)dp, *sp++, count);
                dp+=count;
            } else {
                xor((gchar*)dp, (gchar*)sp, count);
                //fprintf(stderr, "XOR-ing %u bytes\n", count);
                sp+=count;
                dp+=count;
            }
        }
    }
    //fprintf(stderr, "---------------------------------\n");
}
