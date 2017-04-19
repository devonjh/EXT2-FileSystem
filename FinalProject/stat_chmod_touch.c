#ifndef stat_chmod_touch_c
#define stat_chmod_touch_c
#include "type.h"

int touch (char *pathname) {
    int iNum;
    MINODE *mip;

    iNum = getino(running->cwd->dev,pathname);

    if (!iNum) {
        printf("%s not found.",pathname);
        return -1;
    }

    mip = iget(running->cwd->dev,iNum);

    printf("old time value:\t%s", ctime(&mip->INODE.i_atime));
    mip->INODE.i_atime = time(0L);
    printf("new time value:\t%s", ctime(&mip->INODE.i_atime));
    return 1;
}

int statFile (char *pathname) {
    int iNum;
    MINODE *mip;

    iNum = getino(running->cwd, pathname);

    if (!iNum)  {
        printf("%s not found.\n",pathname);
        return -1;
    }

    mip = iget(running->cwd->dev, iNum);

    printf("fileSize: %d\n",mip->INODE.i_size);
    printf("Inode Number: %d\n",mip->ino);
    printf("UID: %d\n",mip->INODE.i_uid);
    printf("GID: %d\n",mip->INODE.i_gid);
    printf("Links: %d\n",mip->INODE.i_links_count);
    printf("Access: %s", ctime(&mip->INODE.i_atime));
    printf("Modify: %s", ctime(&mip->INODE.i_mtime));
    printf("Change: %s", ctime(&mip->INODE.i_ctime));

}
#endif
