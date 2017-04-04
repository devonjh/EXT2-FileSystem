#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h> 

#include "p1.h"

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;

int dev;
int nblocks;
int ninodes;
int bmap;
int imap;
int iblock;
int fd;
char *cp;
char buf[BLKSIZE];

// /********** Functions as BEFORE ***********/
// //these were somewhat mailmain's algorithm'
// int get_block(int fd, int blk, char buf[ ])
// {
//   lseek(fd, (long)blk*BLKSIZE, 0);
//   read(fd, buf, BLKSIZE);
// }
// int put_block(int fd, int blk, char buf[ ])
// {
//   lseek(fd, (long)blk*BLKSIZE, 0);
//   write(fd, buf, BLKSIZE);
// }
// //mailman
// int tst_bit(char *buf, int bit)
// {
//   int i, j;
//   i = bit/8; j=bit%8;
//   if (buf[i] & (1 << j))
//      return 1;
//   return 0;
// }


// MINODE *iget(int dev, int ino)
// {
//     //declare local variables needed:
//   int i, blk, disp;
//   char buf[BLKSIZE];
//   MINODE *mip;
//   INODE *ip;
//   //search through minod[] array for an item pointed y mip with teh same (dev, ino);
//   for (i=0; i < NMINODE; i++){
//       //grab each minode from minode array
//     mip = &minode[i];
//     //if found:
//     if (mip->dev == dev && mip->ino == ino){
//         //mip->refcount++ //increment user count by 1 and return 1;
//        mip->refCount++;
//        printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
//        return mip;
//     }
//   }
//   //search through the minode array again for an item pointed by mip whos refcount ==0;
//   for (i=0; i < NMINODE; i++){
//     mip = &minode[i];
//     //found refcount =0 ;
//     if (mip->refCount == 0){
//         //allocate it now
//        printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
//        mip->refCount = 1;
//        //assign to (dev,ino);
//        mip->dev = dev; mip->ino = ino;  // assing to (dev, ino)
//        //initilize:
//        mip->dirty = mip->mounted = mip->mptr = 0;
//        //MAILMANS:
//        // get INODE of ino into buf[ ]      
//        blk  = (ino-1)/8 + iblock;  // iblock = Inodes start block #
//        disp = (ino-1) % 8;
//        //printf("iget: ino=%d blk=%d disp=%d\n", ino, blk, disp);
//        get_block(dev, blk, buf);
//        ip = (INODE *)buf + disp;
//        // copy INODE to mp->INODE
//        mip->INODE = *ip;
//        return mip;
//     }
//   }   
//   printf("PANIC: no more free minodes\n");
//   return 0;
// }

// //initialize data structures lvl 1:
// // int init(){
// //     /*
// //     1. 2 procs, p01 uid = 0 and p1 uid = 1 and all proc.cwd = 0
// //     */
// //     proc[0].uid = 0;
// //     proc[0].pid = 1;
// //     proc[1].pid = 2;
// //     proc[1].uid = 1;
// //     MINODE minode[100];
// //     MINODE *root = 0;
// // }

// int iput(MINODE *mip){ //dispose of a minode[] pointed by mip
//     int blk, disp;
//     char buf[BLKSIZE];
//     mip->refCount --;
//     if(mip->refCount > 0){
//         return 0;
//     }
//     if(!mip->dirty){
//         return 0;
//     }

//     //write inode back to the disk:
//     printf("iput : dev= %d ino= %d\n", mip->dev, mip->ino);
//     //mailmain's algorithm:
//     blk = ((mip->ino -1)/8)+ iblock;//+beginnodeblock
//     disp = ((mip->ino)%8);

//     get_block(mip->dev,blk, buf);
//     ip = (INODE *)buf + disp;
//     *ip = mip->INODE;

//     put_block(mip->dev, blk, buf);
// }


// int search(MINODE *mip, char *name){
//     int i = 0;
//     char *cp;

//     INODE *ip = &mip->INODE;

//     for (i; i < 12; i++) {
//         if (ip->i_block[i] == 0) {
//             return 0;
//         }
        
//         get_block(fd, ip->i_block[i], buf);
//         dp = (DIR *)buf;
//         cp = buf;

//         while (cp < (buf + BLKSIZE)) {
//             if(!strcmp(name, dp->name) == 0) {
//                 printf("INODE NAME = %s\n");
//                 return dp->inode;
//             }

//             cp += dp->rec_len;
//             dp = (DIR *)cp;
//         }
//     }

//     //get_block(fd, mip->INODE.i_block[0]+1, buf);
//     dp = (DIR *)buf;
//     cp = buf;

//     while (cp < &buf[1024]) {
//         if (strcmp(dp->name,name) == 0) {
//             return dp->inode;
//         }

//         else {

//             printf("inum  rec_len  name_len  filen\n");
//             printf("%d    %d       %d        %s\n\n", dp->inode, dp->rec_len, dp->name_len, dp->name);
//             cp += dp->rec_len;
//             dp = (DIR *)cp;
//         }
//     }

//     return 0;
// }
// int getino(int *dev, char *pathname)
// {
//   int i, ino, blk, disp, n;
//   char  *name[BLKSIZE], *tempname;
//   INODE *ip;
//   MINODE *mip;

//   printf("getino: pathname=%s\n", pathname);
//   if (strcmp(pathname, "/")==0)
//       return 2;

//   if (pathname[0]=='/')
//      mip = iget(*dev, 2);
//   else
//      mip = iget(running->cwd->dev, running->cwd->ino);

//   strcpy(buf, pathname);
//   name[0] = strtok(buf,"/");
//   n = 1;
//   while(tempname = strtok(NULL,"/")){ // n = number of token strings
//     name[n] = tempname;
//     n++;
//   }
//   for (i=0; i < n; i++){
//       printf("===========================================\n");
//       printf("getino: i=%d name[%d]=%s\n", i, i, name[i]);
 
//       ino = search(mip, name[i]);

//       if (ino==0){
//          iput(mip);
//          printf("name %s does not exist\n", name[i]);
//          return 0;
//       }
//       iput(mip);
//       mip = iget(*dev, ino);
//    }
//    return ino;
// }

// // char *disk = "mydisk";

// // int main(int argc, char *argv[]){

// //     if (argc > 1)
// //         disk = argv[1];
// //     fd = open(disk, O_RDONLY);
// //     if (fd < 0){
// //         printf("open failed\n");
// //         exit(1);
// //     }
// //         init();
// //     return 0;
// // }