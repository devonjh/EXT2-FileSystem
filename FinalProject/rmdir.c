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

int rmchild(MINODE *parent, char *name){
    //faster ref
    INODE *piNode = &parent->INODE;
    DIR *dp, *prevdp;
    char *cp;
    int blockindex, tempparentblock, remainblock;

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

            //lets search now:
            while(blockindex < BLKSIZE){
                //found?
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
                        //keep it if it aint soloo:
                        else{
                            prevdp->rec_len += dp-> rec_len;
                            put_block(dev,piNode->i_block[i],buf);
                            return 1;
                        }
                    }

                    //now the hard part: keeping track of the previous entry:
                    prevdp = dp;
                    tempparentblock = blockindex;

                    //now move dp up:
                    cp += dp->rec_len;
                    tempparentblock += dp->rec_len;
                    dp = (DIR *)cp;

                    //I FORGOT THIS ONE: We need to remember the rest of the blocks:
                    remainblock = prevdp->rec_len;

                    //push everything to the left if there's anything in the left:
                    while(tempparentblock < BLKSIZE){
                        //update
                        prevdp-> rec_len = dp->rec_len;

                        //push everything to the left memory:
                        prevdp-> inode = dp->inode;
                        prevdp-> name_len = dp-> name_len;
                        prevdp->file_type = dp -> file_type;
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