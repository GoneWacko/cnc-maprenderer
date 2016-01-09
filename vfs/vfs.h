#ifndef MHENLO_VFS_VFS_H_INCLUDED
#define MHENLO_VFS_VFS_H_INCLUDED 

#include <glib.h>

struct vfs_archive;
typedef struct vfs_archive vfs_archive_t;

struct vfs_file;
typedef struct vfs_file vfs_file_t;

enum VfsSeekType {
    VFS_SEEK_SET,
    VFS_SEEK_CUR,
    VFS_SEEK_END
};

void vfs_init();
void vfs_close();
gboolean vfs_file_exists(gchar *filename);
vfs_file_t *vfs_file_open(gchar *filename);
void vfs_add_archive(const gchar *path);

vfs_file_t *vfs_file_new(GMappedFile *file);
vfs_file_t *vfs_subfile(vfs_file_t *file, gsize offset, gsize len);
gsize vfs_file_read(gchar *buf, gsize len, vfs_file_t *file);
gsize vfs_file_seek(vfs_file_t *file, long offset, enum VfsSeekType type);
gsize vfs_file_tell(vfs_file_t *file);
gsize vfs_file_size(vfs_file_t *file);
void vfs_file_close(vfs_file_t *file);

#endif /* MHENLO_VFS_VFS_H_INCLUDED */
