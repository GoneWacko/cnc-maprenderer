#ifndef MHENLO_VFS_MIXARCHIVE_H_INCLUDED
#define MHENLO_VFS_MIXARCHIVE_H_INCLUDED 

struct vfs_archive;
typedef struct vfs_archive vfs_archive_t;

struct vfs_file;
typedef struct vfs_file vfs_file_t;

vfs_archive_t *vfs_mixarchive_open(vfs_file_t *file);

#endif /* MHENLO_VFS_MIXARCHIVE_H_INCLUDED */
