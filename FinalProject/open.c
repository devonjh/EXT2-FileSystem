#ifndef open_c
#define open_c


#include "type.h"
#include "util.c"

int openFile(char *pathname, int mode) {
    MINODE *mip;
    OFT *oftp;
    int iNum, i = 0;

    if (pathname[0] == '/') {               //Determine Dev to use.
        dev = root->dev;
    }

    dev = running->cwd->dev;

    iNum = getino(dev, pathname);           //Get iNum of pathname.
    printf("iNum: %d\n",iNum);

    mip = iget(dev, iNum);                  //Use inum to get mip pointer.
    printf("iSize: %d\n",mip->INODE.i_size);

    if (S_ISDIR(mip->INODE.i_mode)) {       //Check if location is a directory instead of a file.
        printf("Desired location is a directory.\n");
        iput(mip);
        return -1;
    }
    
    if (!S_ISREG(mip->INODE.i_mode)) {      //Check to ensure that file is regular.
        printf("Desired location is not a regular file.\n");
        iput(mip);
        return -1;
    }

    //Check if file is open in any other fd's in any mode other than 'R'.
    for (i = 0; i < 16; i++) {
        if (running->fd[i] != NULL) {
            if (running->fd[i]->mptr->ino == iNum) {
                if (running->fd[i]->mode != 0) {
                    printf("File is already open in a non-reading mode.\n");
                    iput(mip);
                    return -1;
                }
            }
        }
    }

    //We have made it this far, so allocate the oftp for the file.
    oftp = malloc(sizeof(OFT));

    oftp->mode = mode;                      //Set oftp variables.
    oftp->refCount = 1;
    oftp->mptr = mip;

    switch(mode) {
        case 0: oftp->offset = 0;
                break;
        case 1: truncate(dev,mip);
                oftp->offset = 0;
                break;
        case 2: oftp->offset = 0;       //Does not truncate, instead writes over existing text.
                break;
        case 3: oftp->offset = oftp->mptr->INODE.i_size;
                break;
        default:printf("Invalid mode.\n");
                iput(mip);
                break;
    }

    for(i = 0; i < 16; i++) {
        if (running->fd[i] == NULL) {
            running->fd[i] = oftp;
            printf("fd[%d] set as oftp.\n",i);
            break;
        }
    }

    //update Inodes time field.

    return i;
}

int closeFile(int fdNum) {
    OFT *oftp;
    MINODE *mip;

    if(fdNum > 16 || fdNum < 0) {
        printf("fd out of range.\n");
        return -1;
    }

    if (running->fd[fdNum] == NULL) {
        printf("fd does not have a file open.");
        return -1;
    }

    oftp = running->fd[fdNum];
    running->fd[fdNum] = 0;
    oftp->refCount--;

    if (oftp->refCount > 0) {
        return 0;
    }

    mip = oftp->mptr;
    iput(mip);

    printf("fd[%d] closed successfully.\n",fdNum);
    return 0;
}

int pfd() {                     //Function to print all open files.
    int i = 0;

    printf("FD\tMODE\tOFFSET\tINODE\n");

    for(i = 0; i < 16; i++) {
        if (running->fd[i] != NULL) {
            printf("%d\t%d\t%d\t%d\n",i,running->fd[i]->mode,running->fd[i]->offset,running->fd[i]->mptr->ino);
        }
    }
}

int lseekFD(int fdNum, int offsetNum) {
    OFT *oftp;

    oftp = running->fd[fdNum];

    oftp->offset += offsetNum;

    printf("lseek complete.\n");
}

#endif