#include "vfs.h"
#include "vfs_internal.h"

#include "dirarchive.h"
#include "mixarchive.h"

#include <string.h>

// ---------------------------------------------------------

GSequence *archives;

void vfs_archive_destroy(gpointer data);

void vfs_init() {
    if (!archives) {
        archives = g_sequence_new(vfs_archive_destroy);
    }
}

void vfs_close() {
    if (archives) {
        g_sequence_free(archives);
        archives = NULL;
    }
}

gboolean vfs_file_exists(gchar *filename) {
    GSequenceIter *iter = g_sequence_get_begin_iter(archives);
    while (!g_sequence_iter_is_end(iter)) {
        vfs_archive_t *arch = (vfs_archive_t *)g_sequence_get(iter);
        if (arch->contains(arch, filename)) {
            // TODO buffer recent results so vfs_file_open doesn't have to recheck
            // Invalidate the buffer when an archive gets destroyed(?)
            return TRUE;
        }
        iter = g_sequence_iter_next(iter);
    }
    return FALSE;
}

vfs_file_t *vfs_file_open(gchar *filename) {
    GSequenceIter *iter = g_sequence_get_begin_iter(archives);
    while (!g_sequence_iter_is_end(iter)) {
        vfs_archive_t *arch = (vfs_archive_t *)g_sequence_get(iter);
        if (arch->contains(arch, filename)) {
            return arch->open_file(arch, filename);
        }
        iter = g_sequence_iter_next(iter);
    }
    return NULL;
}

void vfs_add_archive(const gchar *path) {
    vfs_archive_t *archive = NULL;
    if (g_file_test(path, G_FILE_TEST_EXISTS)) {
        if (g_file_test(path, G_FILE_TEST_IS_DIR)) {
            archive = vfs_dirarchive_new(path);
        } else if (g_str_has_suffix(path, ".mix") || g_str_has_suffix(path, ".MIX")) {
            vfs_file_t *fp = vfs_dirfile_new(path);
            archive = vfs_mixarchive_open(fp);
        }

        if (archive) {
            g_sequence_append(archives, archive);
        }
    }
}

void vfs_archive_destroy(gpointer data) {
    vfs_archive_t* arch = (vfs_archive_t *)data;
    arch->close(arch);
    g_free(data);
}

// ---------------------------------------------------------

struct vfs_file {
    GMappedFile *file;
    gsize offset;
    gsize len;
    gsize pos;
};

vfs_file_t *vfs_file_new(GMappedFile *file) {
    vfs_file_t *fp = g_new(vfs_file_t, 1);
    fp->file = file;
    fp->offset = 0;
    fp->len = g_mapped_file_get_length(file);
    fp->pos = 0;
    return fp;
}

vfs_file_t *vfs_subfile(vfs_file_t *file, gsize offset, gsize len) {
    vfs_file_t *fp = g_new(vfs_file_t, 1);
    fp->file = g_mapped_file_ref(file->file);
    fp->offset = offset;
    fp->len = len;
    fp->pos = 0;
    return fp;
}

void vfs_file_close(vfs_file_t *file) {
    g_mapped_file_unref(file->file);
    g_free(file);
}

gsize vfs_file_tell(vfs_file_t *file) {
    return file->pos;
}

gsize vfs_file_size(vfs_file_t *file) {
    return file->len;
}

gsize vfs_file_read(gchar *buf, gsize len, vfs_file_t *file) {
    gsize count;
    if (file->pos + len > file->len) {
        count = file->len - file->pos;
    } else {
        count = len;
    }

    gchar *data = &(g_mapped_file_get_contents(file->file)[file->offset]);
    memcpy(buf, &(data[file->pos]), count);
    file->pos += count;
    return count;
};

gsize vfs_file_seek(vfs_file_t *file, long offset, enum VfsSeekType type) {
    long newpos;
    switch (type) {
        case VFS_SEEK_SET:
            if (offset > file->len) {
                offset = file->len;
            }
            file->pos = offset;
            break;
        case VFS_SEEK_CUR:
            newpos = file->pos + offset;
            if (newpos < 0) {
                file->pos = 0;
            } else if (newpos > file->len) {
                file->pos = file->len;
            } else {
                file->pos = newpos;
            }
            break;
        case VFS_SEEK_END:
            if (offset >= file->len) {
                file->pos = 0;
            } else {
                file->pos = file->len - offset;
            }
            break;
    }

    return file->pos;
}
