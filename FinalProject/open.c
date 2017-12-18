#ifndef open_c
#define open_c


#include "type.h"
#include "util.c"

int openFile(char *pathname, int mode){
    MINODE *mip;
    INODE *ip;
    OFT *oftp;
    int iNum, i=0;

    //let the device be point at the root
    if(pathname[0] == '/'){
        dev = root->dev;
    }
    //else let it be at cwd
    else{
        dev = running->cwd->dev;
    }

    printf("openFile executing.\n");

    //grab its inode using the pathname and its MINODE;
    iNum = getino(dev, pathname);
    mip = iget(dev, iNum);

    //can't open if its a dir'
    if(S_ISDIR(mip->INODE.i_mode)){
        printf("Desired location is a dir.\n");
        iput(mip);
        return -1;
    }
    if(!S_ISREG(mip->INODE.i_mode)){
        printf("Desired location is not regular file");
        iput(mip);
        return -1;
    }

    //there are total of 16 FD's in this file system:
    //check all of em and see if its already opn. if it is, exit:
    for(i = 0; i < 16; i++){
        if(running->fd[i] != NULL){
            if(running->fd[i]->mptr->ino == iNum){
                if(running->fd[i]->mode != 0){
                    printf("File is already open in a non-reading mode.\n");
                    iput(mip);
                    return -1;
                }
            }
        }
    }

    //now we are ready for allocating oftp for a file.
    oftp = malloc(sizeof(OFT));

    //set oftp's variables.
    oftp->mode = mode;
    oftp->refCount = 1;
    oftp->mptr = mip;

    switch(mode){
        case 0: oftp->offset = 0;           //set for read
                oftp->mptr->INODE.i_atime = time(0L);
                break;
        case 1: truncate(dev,mip);
                oftp->offset = 0;           //set for write
                oftp->mptr->INODE.i_atime = oftp->mptr->INODE.i_mtime = time(0L);
                break;
        case 2: oftp->offset = 0;           //set for WR;
                oftp->mptr->INODE.i_atime = oftp->mptr->INODE.i_mtime = time(0L);
                break;
        case 3: oftp->offset = oftp->mptr->INODE.i_size;    //set for append
                oftp->mptr->INODE.i_atime = oftp->mptr->INODE.i_mtime = time(0L);
                break;
        default: printf("Invalid mode. \n");
                iput(mip);
                break;
    }

    //label which ofpn you're in:'
    for (i = 0; i<16; i++){
        if(running->fd[i] == NULL){
            running->fd[i] = oftp;
            printf("fd[%d] set as ofp.\n",i);
            break;
        }
    }
    //mark dirty
    mip->dirty = 1;
    return i;
}


int closeFile(int fdNum){
    OFT *oftp;
    MINODE *mip;

    if(fdNum > 16 || fdNum < 0){
        printf("fd out of range. \n");
        return -1;
    }
    if(running->fd[fdNum] == NULL){
        printf("fd doesn't have a file to open.");
        return -1;
    }

    //reset the oft status back;
    oftp = running->fd[fdNum];
    running->fd[fdNum] = 0;
    oftp->refCount--;
    if(oftp->refCount > 0){
        return 0;
    }

    //last user of this OFT entry => dispose of MINODE[]
    mip = oftp->mptr;
    iput(mip);

    printf("close successes\n");
    return 0;
}

int pfd() {                     //Function to print all open files.
    int i = 0;

    printf("FD\tMODE\tOFFSET\tINODE\n");
    //go throug heach fd and print out available open oftp:
    for(i = 0; i < 16; i++) {
        if (running->fd[i] != NULL) {
            printf("%d\t%d\t%d\t%d\n",i,running->fd[i]->mode,running->fd[i]->offset,running->fd[i]->mptr->ino);
        }
    }
}

//changes the oftp entry and its offset:
int lseekFD(int fdNum, int offsetNum) {
    OFT *oftp;
    oftp = running->fd[fdNum];
    oftp->offset += offsetNum;
    printf("lseek complete.\n");
}

#endif