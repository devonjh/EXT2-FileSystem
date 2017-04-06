#ifndef alloc_dealloc_c
#define alloc_dealloc_c

#include "util.c"

//allocs free inode num:
int ialloc(int dev)
{
  int  i;
  char buff[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, 4, buff);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       decFreeInodes(dev);

       put_block(dev, imap, buf);

       return i+1;
    }
  }
  printf("ialloc(): no more free inodes\n");
  return 0;
}


int dealloc(int dev){
    int i;

    //
}

#endif