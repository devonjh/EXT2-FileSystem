#include "util.c"
#include "alloc_dealloc.c"

#ifndef RMDIR_C
#define RMDIR_C
int rmdir(char *pathname) {

    //Variables
    int iNum, parentINum, testSize = 0;
    MINODE *mip, *pip;
    INODE *iNode, *pino;
    char *cp;

    printf("%s\n",pathname);

    //INum of pathname.
    parentINum = getino(running->cwd->dev,pathname);
    printf("iNum: %d\n",iNum);

    if (parentINum == 0) {
        printf("Desired location not found.\n");
        iput(mip);
        return -1;
    }

    //get pip for inode.
    pip = iget(running->cwd->dev, parentINum);
    
    //now inode:
    iNum = search(pip);
    mip =  iget(dev, iNum);

    //Check status of dir to make sure we can remove it.

    //Check to make sure pathname is a dir and not a regular file.
    if (!S_ISDIR(mip->INODE.i_mode)) {
        printf("Desired location is not a directory.\n");
        iput(mip);
        return -1;
    }

    //Make sure that the directory is not BUSY, or that refCount is greater than 0.
    if (mip->refCount > 1) {
        printf("Desired directory is in use.\n");
        iput(mip);
        return -1;
    }

    printf("Location found.\n");

    //Check to make sure that there is nothing more than '.' and '..'
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

    //rmchild(pip, name);
    //jjust faster references:
    iNode = &mip->INODE;
    pino = &pip->INODE;
    //deallocate blocks and the one inode removed:
    bdealloc(dev,iNode,pino);
    idealloc(dev,iNode);
    //update:
    mip->refCount = 0;

    pip_atime = pip->i_ctime = pip->i_mtime = time(0L);
    pip->dirty = 1;
    iput(pip);

    return 0;
}

int rmchild(MINODE *parent, char *name){
    //faster ref
    INODE *pino = &parent->INODE;
    DIR *dp, prevdp;
    char *cp;

    for(int i = 0; i < 12; i++){
        //check if theres something, if there is, rem it
        if(pino->i_block[i] !=0){
            get_block(parent->dev, pino->iblock[i], buf);
            dp = (DIR *)buf;
            cp = buf;

            //lets search now:
            while(cp < &buf[BLKSIZE]){
                //... gotta go for the parent name now -_-:

            }
        }
    }
}

#endif