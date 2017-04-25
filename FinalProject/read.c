#ifndef TYPE_H
#define TYPE_H

#include "type.h"

int myRead(int fdNum, char *tempBuf, int nbytes) {
    int count = 0;
    int avail;
    long lblk;
    int startByte;
    long realBlk;
    int remaining;
    int indirFlag;
    int dblFlag;
    long *indirect;
    long *dblIndirect;
    long secondLevel;
    char *cq;
    char *cp;
    char indirBuff[BLKSIZE];
    char dblindirBuff[BLKSIZE];

    memset(&buf[0], 0, sizeof(buf));
    memset(&tempBuf[0], 0, sizeof(readBuf));

    if (running->fd[fdNum]->mode == 1 || running->fd[fdNum]->mode == 3) {
        printf("fd[%d] is not open in a compatible type.\n",fdNum);
        return -1;
    }

    //printf("i_size: %d\n",running->fd[fdNum]->mptr->INODE.i_size);
    avail = running->fd[fdNum]->mptr->INODE.i_size - running->fd[fdNum]->offset;

    //printf("\n********************************************************************************************\n");

    while ((nbytes > 0) && (avail > 0)) {
        memset(&buf[0], 0, sizeof(buf));
        memset(&tempBuf[0], 0, sizeof(readBuf));

        lblk = running->fd[fdNum]->offset / BLKSIZE;
        startByte = running->fd[fdNum]->offset % BLKSIZE;

        //printf("avail: %d\n",avail);
        //printf("lblk: %d\n",lblk);
        //printf("startByte: %d\n",startByte);

        if (lblk < 12) {                //Direct Blocks
            realBlk = running->fd[fdNum]->mptr->INODE.i_block[lblk];
        }

        else if ((lblk >= 12) && (lblk < 256 + 12)) {
            if (!indirFlag) {                               //Only need to getblock once for indirect blocks.
                get_block(running->fd[fdNum]->mptr->dev, running->fd[fdNum]->mptr->INODE.i_block[12], indirBuff);
                indirFlag = 1;
            }

            indirect = (long *)buf;
            realBlk = *(indirect+(lblk-12));
        }

        else {
            if (!dblIndirect) {                         //Only need to get_block for block 13 once.
                get_block(running->fd[fdNum]->mptr->dev, running->fd[fdNum]->mptr->INODE.i_block[13], dblindirBuff);
                dblIndirect = 1;
            }

            dblIndirect = (long *)buf;

            if (secondLevel != *(dblIndirect+((lblk-268)/256))) {                 //Check if we need to change indirect blocks within the double indirect block.
                secondLevel = *(dblIndirect+((lblk-268)/256));
                get_block(running->fd[fdNum]->mptr->dev, secondLevel, indirBuff);
            }

            indirect = (long *)buf;
            realBlk = *(indirect+((lblk-12)%256));
        }

        get_block(running->cwd->dev, realBlk, tempBuf);

        cq = buf;
        cp = tempBuf + startByte;
        remaining = BLKSIZE - startByte;
        //printf("remaining: %d\n", remaining);

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

        //printf("%s",buf);
    }

    //printf("\n********************************************************************************************\n");
    //printf("\nmyread: read %d char from file descriptor %d\n", count, fdNum);  

    running->fd[fdNum]->mptr->dirty = 1;
    iput(running->fd[fdNum]->mptr);
    return count;   // count is the actual number of bytes read
}

int catFile(char *pathname) {
    int tempFD;
    int n;
    int i;
    char catBuf[BLKSIZE];
    tempFD = openFile(pathname, 0);             //Open desired file for read.
    
    printf("\n===============File Contents===============\n");

    while ((n = myRead(tempFD, catBuf, BLKSIZE)) > 0) {
        catBuf[n] = 0;
        //printf("%s",catBuf);
        
        for(i = 0; i < BLKSIZE; i++) {
            if (catBuf[i] != NULL) {
                putchar(catBuf[i]);
            }
        }
    }

    printf("===========================================\n");

    closeFile(tempFD);


}

#endif