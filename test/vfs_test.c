#include "../vfs/vfs.h"
#include "../vfs/mixarchive.h"
#include "../vfs/memfile.h"
#include <stdio.h>

int main(int argc, char *argv[]) {

    FILE *general = fopen("../data/GENERAL.MIX", "rb");
    if (!general) {
        perror("Can't open GENERAL.MIX");
        return 1;
    }

    fseek(general, 0, SEEK_END);
    size_t size = ftell(general);
    fseek(general, 0, SEEK_SET);

    char *buf = g_malloc(size);
    fread(buf, 1, size, general);
    fclose(general);

    vfs_file_t *fp = vfs_memfile_new(size);
    vfs_file_write(buf, size, fp);
    vfs_file_seek(fp, 0, VFS_SEEK_SET);

    vfs_mix_archive_t *ar = vfs_mix_archive_new(fp);
    printf("Archive has %u files.\n", *(guint16*)ar);

    vfs_file_t *ini = vfs_mix_archive_fopen(ar, "scg01ea.ini");
    vfs_file_seek(ini, 0, VFS_SEEK_END);
    guint32 inisize = vfs_file_tell(ini);
    vfs_file_seek(ini, 0, VFS_SEEK_SET);
    char *inibuf = g_malloc0(inisize+1);
    vfs_file_read(inibuf, inisize, ini);

    printf(inibuf);

    vfs_file_free(ini);

    return 0;

};
