#ifndef alloc_dealloc_c
#define alloc_dealloc_c

#include "util.c"

//allocs free inode num:
int ialloc(int dev)
{
  int  i;
  char buff[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buff);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buff, i)==0){
       set_bit(buff,i);
       decFreeInodes(dev);

       put_block(dev, imap, buff);

       return i+1;
    }
  }
  printf("ialloc(): no more free inodes\n");
  return 0;
}

int balloc(int dev){
  int i;
  char buff[BLKSIZE];

  //read inode_bitmap block:
  get_block(dev, bmap, buff);
  for (i=0; i < nblocks; i++){
    if (tst_bit(buff, i)==0){
       set_bit(buff,i);
       decFreeInodes(dev);

       put_block(dev, bmap, buff);

       return i+1;
    }
  }
  printf("ialloc(): no more free inodes\n");
}

//clears the inode from given inode
int idealloc(int dev, int inode){
    char buff[BLKSIZE];
    
    //read the imap:
    get_block(dev, imap, buff);

    //subtract it by 1, got removed:
    //set_bit(buff, inode - 1);
    clr_bit(buff, inode - 1);


    printf("Does it go here?\n");
    put_block(dev, imap, buff);

    //increase blocks:
    incFreeInodes(dev);
}

//removes block from the inode:
// int bdealloc(int dev, INODE *ino, INODE *pino){
//     int i = 0, blocknum;
//     char buff[BLKSIZE];

//     // read bmpa:
//     get_block(dev, bmap, buff);
//     for(i = 0; i < 12; i++){
//         if(ino[i] != 0){
//           blocknum = pino->i_block[i];
//           ino->i_block[i] = 0;
//           set_bit(buff, blocknum);
//         }
//     }
// }

int freeblock(int dev, int blockindex){
  printf("Does it go here?\n");
  char buff[BLKSIZE];
  get_block(dev,bmap,buff);
  //set_bit(buff,blockindex);
  clr_bit(buff, blockindex);
  put_block(dev, bmap, buff);

  //increase blocks:
  incFreeInodes(dev);
}



int truncate(int dev, MINODE *mip){

}
#endif