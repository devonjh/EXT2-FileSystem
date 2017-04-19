#ifndef util_c
#define util_c

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h> 

#include "type.h"


char *cp;
char buf[BLKSIZE];

// global variables

MINODE minode[NMINODE];        // global minode[ ] array           
MINODE *root;                  // root pointer: the /    
PROC   proc[NPROC], *running;  // PROC; using only proc[0]

int fd, dev;                               // file descriptor or dev
int nblocks, ninodes, bmap, imap, iblock;  // FS constants
int start_block;


char *disk = "mydisk";
char line[128], cmd[64], pathname[64], pathname2[64], pathname3[64];
//char buf[BLKSIZE];              // define buf1[ ], buf2[ ], etc. as you need

/********** Functions as BEFORE ***********/
//these were somewhat mailmain's algorithm'

int get_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  read(fd, buf, BLKSIZE);
}
int put_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  write(fd, buf, BLKSIZE);
}

//mailman
int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}

//set bit clear bit and decrementinginodes:
int set_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] &= ~(1 << j);
}

int decFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

int incFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // inc free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  printf("superblock size: %d\n", sp->s_free_inodes_count);
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  printf("group discriptor size: %d\n", gp->bg_free_inodes_count);
  put_block(dev, 2, buf);
}


MINODE *iget(int dev, int ino)
{
  //declare local variables needed:
  int i, blk, disp;
  char buf[BLKSIZE];
  MINODE *mip;
  INODE *ip;
 
  for (i = 0; i < NMINODE; i++) {
    if (minode[i].refCount) {
      if (minode[i].dev == dev && minode[i].ino == ino) {
        minode[i].refCount++;
        return &minode[i];
      }
    }
  }

  for (i = 0; i < NMINODE; i++){
    if (minode[i].refCount == 0) {
      blk = (ino-1)/8+start_block;
      disp = (ino-1)%8;

      get_block(dev, blk, buf);
      ip = (INODE *)buf + disp;

      minode[i].INODE = *ip;
      minode[i].dev = dev;
      minode[i].ino = ino;
      minode[i].refCount = 1;
      minode[i].dirty = 0;
      return &minode[i];
    }
  }

  printf("UH OH. No remaning minodes.\n");
  return 0;
  //exit(1);
}

int iput(MINODE *mip){ //dispose of a minode[] pointed by mip
    int blk, disp;
    
    mip->refCount--; //decrement mip refcount.

    if ((mip->refCount) || mip->dirty == 0) {
      return;
    }

    blk = (mip->ino-1)/8+start_block;
    disp = (mip->ino-1)%8;

    get_block(mip->dev, blk, buf);

    ip = (INODE *)buf + disp;
    *ip = mip->INODE;
    put_block(mip->dev, blk, buf);
}


int search(MINODE *mip, char *name)
{
  char tempName[128];
  int i;

  for (i = 0; i <=11; i++) {
    if (mip->INODE.i_block[i]) {
      get_block(dev, mip->INODE.i_block[i],buf);
      dp = (DIR *)buf;
      cp = buf;

      while (cp < &buf[BLKSIZE]) {
        strncpy(tempName, dp->name, dp->name_len);
        tempName[dp->name_len] = 0;

        if (strcmp(tempName, name) == 0) {
          return dp->inode;
        }

        cp += dp->rec_len;
        dp = (DIR *)cp;
      }
    }
  }
  return 0;
}

int getino(int *dev, char *pathname)
{
  int i, ino, blk, disp, n, tempInum, pathSize = 0;
  char  *name[128];
  char *tempname;
  char *pathElements[128];
  INODE *ip;
  MINODE *mip;

  //printf("getino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/")==0)
      return 2;

  if (pathname[0] == '/') {
    dev = root->dev;   //causing seg fault.
    tempInum = root->ino;
  }

  else {
    dev = running->cwd->dev;
    tempInum = running->cwd->ino;
  }

  tempname = strtok(pathname, "/");
  pathSize = 1;

  i = 0;
  while (tempname != NULL) {
    pathElements[i] = tempname;
    tempname = strtok(NULL, "/");
    i++;
    pathSize++;
  }

  pathElements[i] = 0;

  i = 0;
  while(pathElements[i] != NULL) {
    mip = iget(dev, tempInum);
    tempInum = search(mip, pathElements[i]);
    printf("tempInum in getino: %d\n",tempInum);

    if (tempInum == 0) {
      printf("%s does not exist.\n",pathElements[i]);
      iput(mip);
      return 0;
    }

    if ((mip->INODE.i_mode & 0040000) != 0040000) {
      printf("%s is not a directory.\n",pathElements[i]);
      iput(mip);
      return 0;
    }

    iput(mip);
    i++;
  }

  return tempInum;
}

void printSize(char *pathname) {
  int iNum;
  MINODE *mip;

  iNum = getino(running->cwd->dev,pathname);
  printf("iNum: %d\n",iNum);

  mip = iget(running->cwd->dev, iNum);

  printf("File Size: %d\n",mip->INODE.i_size);
}


#endif