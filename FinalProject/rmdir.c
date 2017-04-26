#include "util.c"
#include "alloc_dealloc.c"

#ifndef RMDIR_C
#define RMDIR_C

int pino, ino;
int rmdir(char *pathname) {

    //Variables
    int testSize = 0, tempparentdir;
    MINODE *mip;
    INODE *iNode, *piNode;
    char *cp;
    char tempbase[BLKSIZE], tempdir[BLKSIZE];
    char *tempparent, *tempchild;
    MINODE *pip;
    strcpy(tempbase, pathname);
    strcpy(tempdir, pathname);

    //INum of pathname.
    //parentINum = getino(running->cwd->dev,pathname);
    tempparent = dirname(tempdir);
    tempchild = basename(tempbase);
    if(pathname[0] == '/'){
        dev = root->dev;
        pino = getino(dev, tempparent);
        pip = iget(dev, pino);
    }
    else{
        dev = running->cwd->dev;
        pino = running->cwd->ino;
        pip = iget(dev, pino);
    }

    if (pip == 0) {
        printf("Desired location not found.\n");
        iput(mip);
        return -1;
    }
    //now inode:
    ino = search(pip, tempchild);
    printf("Inode: %d\n", ino);
    printf("Parent ino: %d\n", pino);
    mip =  iget(dev, ino);

    //Check to make sure pathname is a dir and not a regular file.
    if (!S_ISDIR(mip->INODE.i_mode)) {
        printf("Desired location is not a directory.\n");
        iput(mip);
        return -1;
    }

    printf("RefCount: %d\n", mip->refCount);
    //Make sure that the directory is not BUSY, or that refCount is greater than 0.
    if (mip->refCount > 1) {
        printf("Desired directory is in use.\n");
        iput(mip);
        return -1;
    }

    printf("Location found.\n");

    get_block(running->cwd->dev, mip->INODE.i_block[0], buf);
    dp = (DIR *)buf;
    cp = buf;

    while (cp < &buf[BLKSIZE]) {
        testSize++;
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }

    printf("tempSize: %d\n",testSize);
    
    if (testSize > 2) {             //Directory contains more than '.' and '..'
        printf("Directory is not empty. Cannot remove.\n");
        iput(mip);
        return -1;
    }

    int result = rmchild(pip, tempchild);
    printf("result: %d\n", result);
    //jjust faster references:
    iNode = &mip->INODE;
    piNode = &pip->INODE;
    //deallocate blocks and the one inode removed:
    //bdealloc(dev,iNode,pino);
    for(int i = 0; i < 12; i++){
        if(iNode->i_block[i] !=0){
            int tempparentdir = piNode->i_block[i];
            iNode->i_block[i] = 0;
            //now free the parent's dir:
            freeblock(dev, tempparentdir);
        }
    }

    idealloc(dev,ino);
    printf("Does this get through?\n");

    //update:
    mip->refCount = 0;

    piNode->i_atime = piNode->i_ctime = piNode->i_mtime = time(0L);
    pip->dirty = 1;
    //write back to block:
    iput(piNode);

    return 0;
}

/*

*/
int rmchild(MINODE *parent, char *name){
    //faster ref
    INODE *piNode = &parent->INODE;
    DIR *dp, *prevdp;
    char *cp;
    int blockindex, tempparentblock, remainblock;

/*
1. Search parent INODE's data block(s) for the entry of name

   2. Erase name entry from parent directory by
    
  (1). if LAST entry in block{
                                         |remove this entry   |
          -----------------------------------------------------
          xxxxx|INO rlen nlen NAME |yyy  |zzz                 | 
          -----------------------------------------------------

                  becomes:
          -----------------------------------------------------
          xxxxx|INO rlen nlen NAME |yyy (add zzz len to yyy)  |
          -----------------------------------------------------

      }
    
  (2). if (first entry in a data block){
          deallocate the data block; modify parent's file size;

          -----------------------------------------------
          |INO Rlen Nlen NAME                           | 
          -----------------------------------------------
          
          Assume this is parent's i_block[i]:
          move parent's NONZERO blocks upward, i.e. 
               i_block[i+1] becomes i_block[i]
               etc.
          so that there is no HOLEs in parent's data block numbers
      }

  (3). if in the middle of a block{
          move all entries AFTER this entry LEFT;
          add removed rec_len to the LAST entry of the block;
          no need to change parent's fileSize;

               | remove this entry   |
          -----------------------------------------------
          xxxxx|INO rlen nlen NAME   |yyy  |zzz         | 
          -----------------------------------------------

                  becomes:
          -----------------------------------------------
          xxxxx|yyy |zzz (rec_len INC by rlen)          |
          -----------------------------------------------

      }
    
  3. Write the parent's data block back to disk;
     mark parent minode DIRTY for write-back
*/
    //go through indoes:
    for(int i = 0; i < 12; i++){
        //check if theres something, if there is, rem it
        if(piNode->i_block[i] !=0){
            get_block(dev, piNode->i_block[i], buf);
            dp = (DIR *)buf;
            printf("dpname: %s\n", dp->name);
            printf("name: %s\n", name);
            cp = buf;
            blockindex = 0;

            //lets the whole data block:
            while(blockindex < BLKSIZE){
                //found IN THE MIDDLEEE:
                if(strcmp(dp->name,name) == 0){
                    //last block?
                    if(blockindex + dp->rec_len == BLKSIZE){
                        //only one in there?
                        if(blockindex == 0){
                            tempparentblock = piNode->i_block[i];
                            piNode->i_block[i] = 0;
                            freeblock(dev, tempparentblock);
                            return 1;
                        }
                        //keep it if it aint soloo, KEEP GOING:
                        else{
                            prevdp->rec_len += dp-> rec_len;
                            put_block(dev,piNode->i_block[i],buf);
                            return 1;
                        }
                    }

                    //now the hard part: keeping track of the previous entry:
                    prevdp = dp;
                    tempparentblock = blockindex; //gonna traverse through with next entries

                    //now move dp up:
                    cp += dp->rec_len;
                    tempparentblock += dp->rec_len;
                    dp = (DIR *)cp;

                    //I FORGOT THIS ONE: We need to remember the rest of the blocks from old entry's lenght:
                    remainblock = prevdp->rec_len;

                    //NOW WHILE WERE KEEPING TABS WITH THE PREVOIUS BLOCK:
                    //push everything to the left if there's anything in the left:
                    while(tempparentblock < BLKSIZE){
                        //update
                        prevdp-> rec_len = dp->rec_len;
                        //push everything to the left memory:
                        prevdp-> inode = dp->inode;
                        prevdp-> name_len = dp-> name_len;
                        prevdp-> file_type = dp -> file_type;
                        //prevdp->name= dp->name;
                        strcpy(prevdp->name, dp->name);

                        //last entry now?
                        if(tempparentblock + dp-> rec_len == BLKSIZE){
                            prevdp-> rec_len += remainblock;
                            put_block(dev, piNode->i_block[i], buf);
                            return 1;
                        }
                        printf("Next inode entry: %s  ", dp->name);
                        //next entry pls:
                        cp += dp->rec_len;
                        tempparentblock += dp->rec_len;
                        dp = (DIR *)cp;

                        cp = (char *) prevdp + prevdp->rec_len;
                        tempparentblock += prevdp->rec_len;
                        prevdp = (DIR*) cp;
                        printf("Previous iode entry: %s\n", dp->name);
                    }
                }
                
                //keep going down:
                prevdp = dp;
                //next entry pls:
                cp += dp->rec_len;
                blockindex += dp->rec_len;
                dp = (DIR *)cp;
            }
        }
    }
    return -1;
}

#endif