#ifndef MHENLO_VFS_VFS_INTERNAL_H_INCLUDED
#define MHENLO_VFS_VFS_INTERNAL_H_INCLUDED 

#include "vfs.h"

struct vfs_archive {
    gboolean (*contains)(vfs_archive_t *archive, const gchar *filename);
    vfs_file_t *(*open_file)(vfs_archive_t *archive, const gchar *filename);
    void (*close)(vfs_archive_t *archive);
    void *data;
};
typedef struct vfs_archive vfs_archive_t;

#endif /* MHENLO_VFS_VFS_INTERNAL_H_INCLUDED */
