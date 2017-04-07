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
  get_block(dev, bmap, buf);
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

// int dealloc(int dev){
//     int i;

//     //
// }

#endif