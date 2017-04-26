#ifndef WRITE_C
#define WRITE_C

#include "type.h"
#include "util.c"
#include "mkdir_creat.c"
#include "link.c"

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
    char *cq = tempbuf;
    char *cp;
    char indirBuff[BLKSIZE];
    char dblinderBuff[BLKSIZE];
    char wbuf[BLKSIZE];

    printf("cq = %s\n", cq);

    memset(&wbuf[0], 0, sizeof(wbuf));
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
        else if(lblk >= 12 && lblk < 256+12){
            //indir blocks hasn't been touched.
            if(!indirFlag){
                //grabbed the single indirect and make it physical
                get_block(running->fd[fdNum]->mptr->dev, running->fd[fdNum]->mptr->INODE.i_block[12], indirBuff);
                indirFlag = 1;
            }
            indirect = (long *)indirBuff;
            realBlk = *(indirect + (lblk - 12));
        }
        //double indirect:
        else{
            //getting block for the fourteenth block
            if(!dblIndirect){
                get_block(running->fd[fdNum]->mptr->dev, running->fd[fdNum]->mptr->INODE.i_block[13], dblinderBuff);
                dblIndirect = 1; //don't need to go here now, already grabbed it in dblkbuf'
            }
            //grab the first 13 and have this be a position in the blocks;
            dblIndirect = (long *)dblinderBuff;
            if(secondLevel != *(dblIndirect + ((lblk - 268)/256))){
                secondLevel = *(dblIndirect+((lblk - 268)/256)); //grab that avialable block to second level;
                get_block(running->fd[fdNum]->mptr->dev, secondLevel, indirBuff);
            }
            indirect = (long *)indirBuff;
            realBlk = *(indirect) + ((lblk - 268) % 256); //kinda like mailman, make that block given a real block/physical block
        }
        //now that we got all sof the blocks handled,
        //write to the data block:
        printf("printhere?\n");
        get_block(running->fd[fdNum]->mptr->dev, realBlk, wbuf);
        
        cp = wbuf+startByte;
        remain = BLKSIZE - startByte;
        while(remain > 0){
            *cp++ = *cq++; //move everytime.
            nbytes--; remain--;
            running->fd[fdNum]->offset++;
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
    char srcBuf[BLKSIZE];

    srcFD = openFile(src, 0);
    //gd should be open for write:
    destFD = openFile(dest, 2);
    //if failed, create that file and reopen for WR;
    if(destFD == -1){
        creat_file(dest);
        destFD = openFile(dest,2);
    }
    
    while(n = myRead(srcFD, srcBuf, BLKSIZE)){
        mywrite(destFD, srcBuf, n);
    }
    closeFile(destFD);
    closeFile(srcFD);
    return n;
}

//basically renaming in a different spot.
//verify that src exists:
//is src->dev same as dest->dev
//if so, hard link dest with sorc and unlink src
//not same dev = cp src to dst, unlink src
int mv(char *src, char *dest){
    MINODE *srcmip, *destmip;
    int srcFD, destFD;
    //open should verify if src is all available
    srcFD = openFile(src, 0);
    destFD = openFile(src, 2);
    if(srcFD == -1){
        printf("unable to open src. Exiting \n");
        return -1;
    }
    //grab the src's mip 
    srcmip = running->fd[srcFD]->mptr;
    destmip = running->fd[destFD]->mptr;
    if(srcmip->dev == destmip->dev){
        printf("samedev\n");
        link(src, dest);
        unlink(src);
        closeFile(srcFD);
        closeFile(destFD);
        return 0;
    }
    
    //not same dev:
    closeFile(srcFD);
    closeFile(destFD);
    cpFile(src,dest);
    unlink(src);
    return 0;

}

// int logicaltoreal(MINODE *mip, int lblk){
//     int realblk, indirectbuf[BLKSIZE], dblindirectbuf[BLKSIZE],dbl2[BLKSIZE];

//     if(lblk < 12){
//         //if we have to allocate if nonexistant
//         if(mip->INODE.i_block[lblk] == 0){
//             //allocate that logical blk given
//             mip->INODE.i_block[lblk] = balloc(dev);
//             //erase block:
//             freeblock(dev, mip->INODE.i_block[lblk]);
//             mip->dirty = 1;
//             iput(mip);
//         }

//         realblk = mip->INODE.i_block[lblk]);
//     }

//     //single indir block:
//     if(lblk>= 12 && lblk< 268){
//         //no blocks? create the blocks and free the block of 0;
//         if(mip->INODE.i_block[12] == 0){
//             mip->INODE.i_block[12] = balloc(dev);
//             freeblock(dev,mip->INODE.i_block[12]);
//         }
//         get_block(dev, mip->INODE.i_block[12], indirectbuf);
//         realblk = indirectbuf[lblk - 12);

//         //if there's nothing in that indirect block, set up for it by allocating and puttin back:
//         if(blk == 0){
//             blk = balloc(dev);
//             //erase block:
//             freeblock(dev, blk);
//             //put it back to the physcial:
//             put_block(dev, mip->INODE.i_node, blk-12);
//         }
        
    
//     } 
//     //double indirect:
//     else{
//         //none in the double indirectblock:
//         if(mip->INODE.i_block[13] == 0){
//             mip->INODE.i_block[13] = balloc(dev);
//             freeblock(dev,mip->INODE.i_block[13]);
//         }
//         //grab the first lvl of indirect block
//         get_block(dev, mip->INODE.i_block[13], indirectbuf);

//         lblk == 12+256;
//         dblindirectblock = realblk[indirectbuf / 256];

//         if(doubleindirectbuf )
//         //if there's nothing in that indirect block, set up for it by allocating and puttin back:
//         if(dblindirectblock == 0){
//             dblindirectblock = balloc(dev);     
//             indirectbuf[(lblk - 12)/256] = dblindirectblock;
//             bfree(dev, dblindirectblock);
//             put_block(dev, mip->INODE.i_block[13], indirectblock);

//         }
//         get_block(dev, doubleindirectbuf, db2);
//         realblk = db2[lblk % 256];
//         //if there's nothing in that indirect block, set up for it by allocating and puttin back:
//         if(realblk == 0){
//             realblk = balloc(dev);     
//             bfree(dev, realblock);
//             db2[(lblk - 12)% 256] = realblk;
//             put_block(dev, mip->INODE.i_block[13], db2);

//         }
//     }
//     return realblk;
//     //returns a bno
// }
#endif