#include "mixarchive.h"
#include "vfs_internal.h"

struct index_entry {
    guint32 offset;
    guint32 size;
};

struct vfs_mixarchive {
    guint16 file_count;
    guint32 data_size;
    GHashTable *index;
    vfs_file_t *source_file;
};
typedef struct vfs_mixarchive vfs_mixarchive_t;

guint32 compute_id(const gchar *filename) {
    guint32 id = 0;
    gchar *buf;
    gchar *p, *end;
    buf = g_ascii_strup(filename, 12); 
    for (p = buf, end = buf + 12; p != end && *p != 0; p += 4) {
        id = (id << 1) | ((id >> 31) & 1);
        id += *(gint32*)p;
    }
    g_free(buf);
    return id;
}

#define MIX_HEADER_SIZE 6
#define MIX_INDEX_ENTRY_SIZE 12

gboolean vfs_mixarchive_contains(vfs_archive_t *archive, const gchar *filename);
vfs_file_t *vfs_mixarchive_open_file(vfs_archive_t *archive, const gchar *filename);
void vfs_mixarchive_close(vfs_archive_t *archive);

vfs_archive_t *vfs_mixarchive_open(vfs_file_t *file) {
    gchar buf[MIX_INDEX_ENTRY_SIZE];
    vfs_mixarchive_t *mix = g_new(vfs_mixarchive_t, 1);

    vfs_file_read(buf, MIX_HEADER_SIZE, file);
    mix->file_count = *(guint16*)(&buf[0]);
    mix->data_size = *(guint32*)(&buf[2]);
    mix->source_file = file;
    mix->index = g_hash_table_new_full(g_int_hash, g_int_equal, g_free, g_free);

    struct index_entry *entry;
    guint32 *id;
    for (int i = 0; i < mix->file_count; ++i) {
        entry = g_new(struct index_entry, 1);
        id = g_new(guint32, 1);
        vfs_file_read(buf, MIX_INDEX_ENTRY_SIZE, file);
        *id = *(guint32*)(&buf[0]);
        entry->offset = *(guint32*)(&buf[4]) + (MIX_INDEX_ENTRY_SIZE * mix->file_count) + MIX_HEADER_SIZE;
        entry->size = *(guint32*)(&buf[8]);
        g_hash_table_insert(mix->index, id, entry);
    }

    vfs_archive_t *arch = g_new(vfs_archive_t, 1);
    arch->contains = vfs_mixarchive_contains;
    arch->open_file = vfs_mixarchive_open_file;
    arch->close = vfs_mixarchive_close;
    arch->data = mix;

    return arch;
}

gboolean vfs_mixarchive_contains(vfs_archive_t *archive, const gchar *filename) {
    vfs_mixarchive_t *mix = (vfs_mixarchive_t *)archive->data;
    guint32 id = compute_id(filename);
    return g_hash_table_contains(mix->index, &id);
}

vfs_file_t *vfs_mixarchive_open_file(vfs_archive_t *archive, const gchar *filename) {
    vfs_mixarchive_t *mix = (vfs_mixarchive_t *)archive->data;
    guint32 id = compute_id(filename);
    struct index_entry *entry = g_hash_table_lookup(mix->index, &id);
    if (!entry) {
        return NULL;
    }

    vfs_file_t *fp = vfs_subfile(mix->source_file, entry->offset, entry->size);
    return fp;
}

void vfs_mixarchive_close(vfs_archive_t *archive) {
    vfs_mixarchive_t *mix = (vfs_mixarchive_t *)archive->data;
    vfs_file_close(mix->source_file);
    g_free(mix);
}

