#ifndef ls_cd_pwd_c
#define ls_cd_pwd_c

#include "util.c"

int ls(char *pathname)  // dig out YOUR OLD lab work for ls() code 
{
  //WRITE C code for these:
  int iNum, i;
  char tempName[BLKSIZE];
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

    i = 0;
    while (mip->INODE.i_block[i] != 0) {
      get_block(dev, mip->INODE.i_block[0], tempBuf);
      dp = (DIR *)tempBuf;
      tempCP = tempBuf;
      ip = (INODE *)tempCP;
      printf("Permissions\tiNum\trec_len\tname_len\tFile Name\n"); 

      while (tempCP < &tempBuf[BLKSIZE]) {
        
        strncpy(tempName, dp->name, dp->name_len);
        tempName[dp->name_len] = 0;   //get rid of null terminating character.

        iNum = dp->inode;
        printPermissions(iNum);

        if (ip->i_mode == 0xA000) {
          printf("INSIDE.\n");
          MINODE *linkMIP = iget(mip->dev, dp->inode);
          char linkName[BLKSIZE];
          strcpy(linkName, (char *)linkMIP->INODE.i_block);
          printf("\t%d\t%d\t%d\t\t%s->%s\n",dp->inode, dp->rec_len, dp->name_len, tempName,linkName);
          iput(linkMIP);
        }

        else {
          printf("\t%d\t%d\t%d\t\t%s\n",dp->inode, dp->rec_len, dp->name_len, tempName);
        }

        tempCP += dp->rec_len;
        dp = (DIR *)tempCP;
        ip = (INODE *)tempCP;
      }
     // iput(lsinode);
      i++;
    }
    iput(mip);
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

      i = 0;
      //Location is a directory. Step into it and print contents.
      while (mip->INODE.i_block[i] != 0) {
        get_block(dev, mip->INODE.i_block[i], tempBuf);
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
        i++;
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
        iput(mip);
        return;
    }

    else {
      proc[0].cwd = mip;
      iput(mip);
    }
}    
 
int pwd(MINODE *mip, int childIno){
  char pwdBuf[BLKSIZE];
  char currName[64];
  char *cp;    
  MINODE *parentMIP;
  int tempIno;

  memset(&pwdBuf[0], 0, sizeof(pwdBuf));
  memset(&currName[0], 0, sizeof(currName));

  if (mip->ino == root->ino) {
    printf("/");
  }

  get_block(dev, mip->INODE.i_block[0], pwdBuf);

  dp = (DIR *)pwdBuf;                //set dp to '.'
  cp = pwdBuf + dp->rec_len;         //move to '..'
  dp = (DIR *)cp;

  if (mip->ino != root->ino) {        //Recursively go through parent Inodes until root is hit.
    tempIno = dp->inode;
    parentMIP = iget(running->cwd->dev, tempIno);
    pwd(parentMIP, mip->ino);
  }

  if (childIno != 0) {
    while (childIno != dp->inode) {     //Search block for current dir.
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }

    //We have now found our desired dir.
    strncpy(currName, dp->name, dp->name_len);
    currName[dp->name_len] = '\0';
    printf("%s/",currName);  
  }

  return;
}

int quit()
{
    /*
  for each minode[ ] do {
      if  minode[ ]'s refCount != 0: 
          write its INODE back to disk; 
          if dirty, write it back out in disk
  }*/
    for(int i = 0; i < NMINODE; i++){
      if(minode[i].refCount > 0 && minode[i].dirty == 1){
        minode[i].refCount = 1;
        iput(&minode[i]);
      }
    }
  
  exit(1);  // terminate program
  return 0;
}


#endif