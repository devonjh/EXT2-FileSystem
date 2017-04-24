#ifndef WRITE_C
#define WRITE_C

#include "type.h"
#include "util.c"
#include "mkdir_creat.c"

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
    int remain;
    int indirFlag;
    int dblFlag;
    long *indirect;
    long *dblIndirect;
    long secondLevel;
    char *cq;
    //char *cp;
    char indirBuff[BLKSIZE];
    char dblinderBuff[BLKSIZE];
    char wbuf[BLKSIZE];

    //set it equal to 0:
    // memset(&buf[0], 0, sizeof(buf));
    // memset(&tempbuf[0],0,sizeof(writeBuf));

    //check if avaialbe for WR RW APD:
    if(running->fd[fdNum]->mode==0){
        printf("fd[%d] is not open in a compatible type. \n",fdNum);
        return -1;
    }   
    
    available = running->fd[fdNum]->mptr->INODE.i_size - running->fd[fdNum]->offset;

    while(nbytes > 0){
        //logical blocks and direct and indirects:
        lblk = running->fd[fdNum]->offset / BLKSIZE;
        startByte = running->fd[fdNum]->offset % BLKSIZE;

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
            //indir blocks hasn't been touched.
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
            //getting block for the fourteenth block
            if(!dblIndirect){
                get_block(running->fd[fdNum]->mptr->dev, running->fd[fdNum]->mptr->INODE.i_block[14], dblinderBuff);
                dblIndirect = 1; //don't need to go here now, already grabbed it in dblkbuf'
            }
            //grab the first 13 and have this be a position in the blocks;
            dblIndirect = (long *)buf;
            if(secondLevel != *(dblIndirect + ((lblk - 268)/256))){
                secondLevel = *(dblIndirect+((lblk - 268)/256)); //grab that avialable block to second level;
                get_block(running->fd[fdNum]->mptr->dev, secondLevel, indirBuff);
            }
            indirect = (long *)buf;
            realBlk = *(indirect) + ((lblk - 12) % 256); //kinda like mailman, make that block given a real block/physical block
        }
        //write to the data block:

        get_block(running->fd[fdNum]->mptr->dev, realBlk, wbuf);
        cq = buf;
        char *cp = wbuf+startByte;
        remain = BLKSIZE - startByte;
        while(remain > 0){
            *cp++ = *cq++; //move everytime.
            nbytes--; remain--;
            running->fd[fdNum]->offset--;
            if(running->fd[fdNum]->offset > running->fd[fdNum]->mptr->INODE.i_size)
                running->fd[fdNum]->mptr->INODE.i_size++;
            if(nbytes <= 0)
                break;
            
        }
        put_block(running->fd[fdNum]->mptr->dev, realBlk, wbuf);    //write it back to disk;
    }
    running->fd[fdNum]->mptr->dirty = 1;
    iput(running->fd[fdNum]->mptr);
    printf("Write %d char into file descriptor fd = %d\n", nbytes, fd);
    return nbytes;
}   

/*

                      HOW TO cp ONE file:

cp src dest:

1. fd = open src for READ;

2. gd = open dst for WR|CREAT; 

   NOTE:In the project, you may have to creat the dst file first, then open it 
        for WR, OR  if open fails due to no file yet, creat it and then open it
        for WR.

3. while( n=read(fd, buf[ ], BLKSIZE) ){
       write(gd, buf, n);  // notice the n in write()
   }
*/

int cpFile(char *src, char *dest){
    //src should be for read;
    int srcFD, destFD;
    int n, i;
    char srcBuf[BLKSIZE], destBuf[BLKSIZE];

    srcFD = openFile(src, 0);
    //gd should be open for write:
    destFD = openFile(dest, 2);
    //if failed, create that file and reopen for WR;
    if(destFD == -1){
        creat_file(dest);
        destFD = openFile(dest,2);
    }
    
    while(n = myRead(srcFD, srcBuf, BLKSIZE)){
        printf("Temp: %s\n", srcBuf);
        mywrite(destFD, destBuf, n);
    }
    printf("src: %s\n", srcBuf);
    closeFile(destFD);
    closeFile(srcFD);
    return n;
}
/*//directblocks:
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
            //indir blocks hasn't been touched.
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
            //getting block for the fourteenth block
            if(!dblIndirect){
                get_block(running->fd[fdNum]->mptr->dev, running->fd[fdNum]->mptr->INODE.i_block[14], dblinderBuff);
                dblIndirect = 1; //don't need to go here now, already grabbed it in dblkbuf'
            }
            //grab the first 13 and have this be a position in the blocks;
            dblIndirect = (long *)buf;
            if(secondLevel != *(dblIndirect + ((lblk - 268)/256))){
                secondLevel = *(dblIndirect+((lblk - 268)/256)); //grab that avialable block to second level;
                get_block(running->fd[fdNum]->mptr->dev, secondLevel, indirBuff);
            }
            indirect = (long *)buf;
            realBlk = *(indirect) + ((lblk - 12) % 256); //kinda like mailman, make that block given a real block/physical block
        }
*/
#endif