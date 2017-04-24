#ifndef WRITE_C
#define WRITE_C

#include "type.h"
#include "util.c"

int mywrite(int fdNum, char *tempbuf, int nbytes){
    //preps: Ask for fd and writemode.
    //fd has to be open for WR or RW or APD
    //cpy text string to buf[] with len;(oftp)

    //mywrite(fd,buf,nbytes)
    int count = 0;
    int available;
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
    char dblinderBuff[BLKSIZE];

    //check if avaialbe for WR RW APD:
    if(running->fd[fdNum]->mode==0){
        printf("fd[%d] is not open in a compatible type. \n",fdNum);
        return -1;
    }   
    
    //available = running->fd[fdNum]->mptr->INODE.i_size - running->fd[fdNum]->offset;

    while(nbytes > 0){
        //compute logical block and startbyte in that lblk:
        lblk = running->fd[fdNum]->offset / BLKSIZE;
        startByte = running->fd[fdNum]->offset % BLKSIZE;

        //directblocks:
        if(lblk < 12){
            //
            //must allocate blocks if it isn't avialable:'
            if(running->fd[fdNum]->mptr->INODE.i_block[lblk] == 0){
                running->fd[fdNum]->mptr->INODE.i_block[lblk] = balloc(running->fd[fdNum]->mptr->dev);
            }
            //available for disk, now written in teh real block:
            realBlk = running->fd[fdNum]->mptr->INODE.i_block[lblk];
        }
        //indirect block:
        else if(lblk >= 12 && lblk <lblk < 256+12){
            if(!indirFlag){
                //grabbed the single indirect and make it physical
                get_block(running->fd[fdNum]->mptr->dev, running->fd[fdNum]->mptr->INODE.i_block[13], indirBuff);
                indirFlag = 1;
            }

            indirect = (long *)buf;
            realBlk = *(indirect + (lblk - 12));
        }
        //double indiriect:
        else{
            
        }

        //now that blocks are available:
        get_block(running->fd[fdNum]->mptr->dev, realBlk, tempbuf);    //store the block in a temporary buff
        char *cp = tempbuf + startByte;                             //cp points to the startbyte of tempBuff
        remaining = BLKSIZE - startByte;                            //Bytes remaining in this block;

        while(remaining > 0){
            *cp++ = *cq++;
            running->fd[fdNum]->offset++;
            count++;
            available--;
            nbytes--;
            remaining--;
            
            if(running->fd[fdNum]->offset++ > running->fd[fdNum]->mptr->INODE.i_size){
                running->fd[fdNum]->mptr->INODE.i_size++;
            }
            if(nbytes <=0 || available <= 0){
                break;
            }
        }
        put_block(running->fd[fdNum]->mptr->dev, realBlk, tempbuf); //write tempbuf back to disk

    }

    running->fd[fdNum]->mptr->dirty = 1;
    printf("Write %d char into file descpriotr fd = %d\n", nbytes, fd);
    return nbytes;
}   

#endif