#include "iget_iput_getino.c"

int ls(char *pathname)  // dig out YOUR OLD lab work for ls() code 
{
  //WRITE C code for these:
  int iNum, i;
  char tempName[128];
  MINODE *mip,*tempInode;
  char tempBuf[BLKSIZE];
  char *tempCP;

  //determine initial dev: 
  if (pathname[0]== '/') {
    dev = root->dev;
  }
  else {
    dev = running->cwd->dev;
  }

  if (pathname[0] == 0) {
    mip = iget(dev, running->cwd->ino);
    printf("mip->ino: %d\n",mip->ino);

    if ((mip->INODE.i_mode & 0040000) != 0040000) {   //Location is a file, not a directory.
        printf("iNum\trec_len\tname_len\tFile Name\n");
        get_block(dev, mip->ino, tempBuf);
        dp = (DIR *)tempBuf;
        cp = tempBuf;

        printf("%d\t%d\t%d\t\t%s\n",dp->inode, dp->rec_len, dp->name_len, dp->name);    //Not printing correctly.
        return;
      }

    get_block(dev, mip->INODE.i_block[0], tempBuf);
    dp = (DIR *)tempBuf;
    tempCP = tempBuf;
    printf("iNum\trec_len\tname_len\tFile Name\n"); 

    while (tempCP < &tempBuf[BLKSIZE]) {
      strncpy(tempName, dp->name, dp->name_len);
      tempName[dp->name_len] = 0;   //get rid of null terminating character.

      iNum = dp->inode;
      printf("%d\t%d\t%d\t\t%s\n",dp->inode, dp->rec_len, dp->name_len, tempName);

      tempCP += dp->rec_len;
      dp = (DIR *)tempCP;
    }
  }

  else {
      iNum = getino(dev, pathname);

      if (iNum == 0) {
        //printf("Desired File/Path does not exist.")
        return 0;
      }

      mip = iget(dev, iNum);
      printf("mip->ino: %d\n",mip->ino);
      
      if ((mip->INODE.i_mode & 0040000) != 0040000) {   //Location is a file, not a directory.
        printf("iNum\trec_len\tname_len\tFile Name\n");
        get_block(dev, mip->ino, tempBuf);
        dp = (DIR *)tempBuf;
        cp = tempBuf;

        printf("%d\t%d\t%d\t\t%s\n",dp->inode, dp->rec_len, dp->name_len, dp->name);    //Not printing correctly.
        iput(mip);
        return;
      }

      //Location is a directory. Step into it and print contents.
      get_block(dev, mip->INODE.i_block[0], tempBuf);
      dp = (DIR *)tempBuf;
      tempCP = tempBuf;

      printf("iNum\trec_len\tname_len\tFile Name\n"); 

      while (tempCP < &tempBuf[BLKSIZE]) {
        strncpy(tempName, dp->name, dp->name_len);
        tempName[dp->name_len] = 0;   //get rid of null terminating character.

        iNum = dp->inode;
        printf("%d\t%d\t%d\t\t%s\n",dp->inode, dp->rec_len, dp->name_len, tempName);

        tempCP += dp->rec_len;
        dp = (DIR *)tempCP;
      }
      iput(mip);
  }
}

int chdir(char *pathname)
{
    int iNum;
    //determine initial dev: 
    if (pathname[0]== '/') {
      dev = root->dev;
    }
    else {
      dev = running->cwd->dev;
    }
      
    //convert pathname to (dev, ino);
    iNum = getino(&dev, pathname);

    //get a MINODE *mip pointing at (dev, ino) in a minode[ ];
    MINODE *mip = iget(dev, iNum);

    get_block(fd, mip->INODE.i_block[0],buf);

    //if mip->INODE is NOT a DIR: reject and print error message;
    if (!S_ISDIR(mip->INODE.i_mode)) {
        printf("%s is not a directory.\n",pathname);
        return;
    }

    else {
      proc[0].cwd = mip;
    }
}    
 
//int pwd(running->cwd){//: YOU WRITE CODE FOR THIS ONE!!!
    /*
    how to get to parent:
        running->cwd->inode search for ".." and get that in block ".." is a parent.
        use mailman's algorithm
        save each string locally in a stack 
        once you read the top, that is number 2 inode and go down recursively.
    */