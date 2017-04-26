#ifndef ls_cd_pwd_c
#define ls_cd_pwd_c

#include "type.h"
#include "util.c"

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
    i = 0;
    while(mip->INODE.i_block[i] != 0){
      get_block(dev, mip->INODE.i_block[0], tempBuf);
      dp = (DIR *)tempBuf;
      tempCP = tempBuf;
      printf("Permissions\tiNum\trec_len\tname_len\tFile Name\n"); 

      while (tempCP < &tempBuf[BLKSIZE]) {
        
        strncpy(tempName, dp->name, dp->name_len);
        tempName[dp->name_len] = 0;   //get rid of null terminating character.

        iNum = dp->inode;
        printPermissions(iNum);
        MINODE *lsinode = iget(dev, iNum);
        
        //printf("\t%d\t%d\t%d\t\t%s\n",dp->inode, dp->rec_len, dp->name_len, tempName);
        if(S_ISLNK(lsinode->INODE.i_mode)){
          // -> %s", (char *) mip->INODE.i_blocks);
          //printf("SYMLINK!");
          char symname[64];
          strcpy(symname,tempName);
          
          printf("\t%d\t%d\t%d\t\t%s\n",dp->inode, dp->rec_len, dp->name_len, tempName);
        }
        else{
          printf("\t%d\t%d\t%d\t\t%s\n",dp->inode, dp->rec_len, dp->name_len, tempName);
        }
        tempCP += dp->rec_len;
        dp = (DIR *)tempCP;
      }
    i++;
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
        iput(mip);
        return;
    }

    else {
      proc[0].cwd = mip;
      iput(mip);
    }
}    
 
int pwd(){//: YOU WRITE CODE FOR THIS ONE!!!
      int tempINum, i = 0, parentINum;
      int blk, disp;
      char *childName;
      char *tempPath;
      char *cp;
      char fullPath[64] = "/";
      char tempName[64];
      MINODE *mip;
      MINODE *origCWD;
      char *pwdArray[8] = {""};
      //running->cwd->inode search for ".." and get that in block ".." is a parent.
      origCWD = running->cwd;

      if (running->cwd->ino == 2) {
        printf("PWD: /");
        return;
      }

      i = 0;
      while (running->cwd->ino != 2) {    //Continue traversing parent Inodes until root is reached.
        memset(&tempName[0], 0, sizeof(tempName));
        mip = iget(dev, running->cwd->ino);
        get_block(dev, mip->INODE.i_block[0],buf);
        dp = (DIR *)buf;
        tempINum = dp->inode;
        //printf("iNum: %d\n",tempINum);


        parentINum = search(mip, "..");   //Move to parent. 
        //printf("parentINum: %d\n",parentINum);

        mip = iget(dev, parentINum);
        get_block(dev, mip->INODE.i_block[0],buf);
        dp = (DIR *)buf;
        cp = buf;

        while (cp < &buf[BLKSIZE]) {
          //printf("\ndp->inode: %d\n",dp->inode);
          //printf("tempINum: %d\n",tempINum);
          //printf("current name: %s\n",dp->name);
          if (dp->inode == tempINum) {
            strncpy(tempName, dp->name,64);
            //printf("tempName: %s\n",tempName);
            printf("i: %d\n",i);
            pwdArray[i] = tempName;
            printf("tempName: %s\n",pwdArray[i]);
            i++;
            break;
          }

          cp += dp->rec_len;
          dp = (DIR *)cp;
          
        }

        printf("pathArray[0]: %s\n",pwdArray[0]);
        printf("loop.\n");
        running->cwd = mip;
      }    

      //pwdArray[i] = 0;

      i -= 1;
      

      printf("\n");
      
      while (i >= 0) {
        printf("pwdArray[%d]: %s\n",i,pwdArray[i]);
        i--;
      }

      running->cwd = origCWD;
      printf("PWD:");
      printf("%s\n",fullPath);
      iput(mip);
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