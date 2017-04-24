#ifndef read_c
#define read_c
#include "type.h"

int myRead(int fdNum, char *buf, int nbytes) {
    int count = 0;
    int avail;
    int lblk;
    int startByte;
    int realBlk;
    int remaining;
    char *cq = buf;
    char *cp;

    if (running->fd[fdNum]->mode == 1 || running->fd[fdNum]->mode == 3) {
        printf("fd[%d] is not open in a compatible type.\n",fdNum);
        return -1;
    }

    printf("i_size: %d\n",running->fd[fdNum]->mptr->INODE.i_size);
    avail = running->fd[fdNum]->mptr->INODE.i_size - running->fd[fdNum]->offset;
    printf("avail: %d\n",avail);

    while (nbytes && avail) {
        lblk = running->fd[fdNum]->offset/BLKSIZE;
        startByte = running->fd[fdNum]->offset%BLKSIZE;

        printf("lblk: %d\n",lblk);
        printf("startByte: %d\n",startByte);

        if (lblk < 12) {                //Direct Blocks
            realBlk = running->fd[fdNum]->mptr->INODE.i_block[lblk];
        }

        else if (lblk >= 12 && lblk < 256 + 12) {
            realBlk = running->fd[fdNum]->mptr->INODE.i_block[12];
        }

        else {
            realBlk = running->fd[fdNum]->mptr->INODE.i_block[13];
        }

        get_block(running->cwd->dev, realBlk, readBuf);

        cp = readBuf + startByte;
        remaining = BLKSIZE - startByte;

        while (remaining > 0) {
            *cq++ = *cp++;
            running->fd[fdNum]->offset++;
            count++;
            avail--;
            nbytes--;
            remaining--;
            if (nbytes <= 0 || avail <= 0) {
                break;
            }
        }
    }

    printf("myread: read %d char from file descriptor %d\n", count, fdNum);  
    printf("buf: %s ",buf);
    return count;   // count is the actual number of bytes read
}
#endif
