#ifndef DIRARCHIVE_H_INCLUDED
#define DIRARCHIVE_H_INCLUDED 

#include <glib.h>

struct vfs_archive;
typedef struct vfs_archive vfs_archive_t;

struct vfs_file;
typedef struct vfs_file vfs_file_t;

vfs_archive_t *vfs_dirarchive_new(const gchar *path);
vfs_file_t *vfs_dirfile_new(const gchar* path);

#endif /* DIRARCHIVE_H_INCLUDED */
