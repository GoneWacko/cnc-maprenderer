#include "dirarchive.h"
#include "vfs_internal.h"
#include <stdio.h>
#include <string.h>

struct vfs_dirarchive {
    GDir *dir;
    gchar *path;
};
typedef struct vfs_dirarchive vfs_dirarchive_t;

gboolean vfs_dirarchive_contains(vfs_archive_t *archive, const gchar *filename);
vfs_file_t *vfs_dirarchive_open_file(vfs_archive_t *archive, const gchar *filename);
void vfs_dirarchive_close(vfs_archive_t *archive);

vfs_archive_t *vfs_dirarchive_new(const gchar *path) {
    GError *err = NULL;
    GDir *dir = g_dir_open(path, 0, &err);
    if (!dir) {
        fprintf(stderr, "Unable to open directory '%s': %s\n", path, err->message);
        g_error_free(err);
        return NULL;
    }

    vfs_dirarchive_t *dirarchive = g_new(vfs_dirarchive_t, 1);
    dirarchive->dir = dir;
    dirarchive->path = g_strdup(path);

    vfs_archive_t *arch = g_new(vfs_archive_t, 1);
    arch->contains = vfs_dirarchive_contains;
    arch->open_file = vfs_dirarchive_open_file;
    arch->close = vfs_dirarchive_close;
    arch->data = dirarchive;

    return arch;
}

gboolean vfs_dirarchive_contains(vfs_archive_t *archive, const gchar *filename) {
    vfs_dirarchive_t *dir = (vfs_dirarchive_t *)archive->data;
    const gchar *name = NULL;
    g_dir_rewind(dir->dir);
    while ((name = g_dir_read_name(dir->dir)) && g_strcmp0(name, filename) != 0);
    return name != NULL;
}

vfs_file_t *vfs_dirarchive_open_file(vfs_archive_t *archive, const gchar *filename) {
    vfs_dirarchive_t *dir = (vfs_dirarchive_t *)archive->data;
    gulong count = strlen(filename) + strlen(dir->path) + 2;
    gchar *name = g_malloc(count);
    g_snprintf(name, count, "%s/%s", dir->path, filename);
    vfs_file_t *fp = vfs_dirfile_new(name);
    g_free(name);
    return fp;
}

void vfs_dirarchive_close(vfs_archive_t *archive) {
    vfs_dirarchive_t *dir = (vfs_dirarchive_t *)archive->data;
    g_free(dir->path);
    g_dir_close(dir->dir);
    g_free(dir);
}

// ---------------------------------------------------------

vfs_file_t *vfs_dirfile_new(const gchar* path) {
    GError *err = NULL;
    GMappedFile *file = g_mapped_file_new(path, 0, &err);
    if (!file) {
        fprintf(stderr, "Unable to open file '%s': %s\n", path, err->message);
        g_error_free(err);
        return NULL;
    }

    return vfs_file_new(file);
}

