#include "mkdir_creat.c"
#include "util.c"
#include "alloc_dealloc.c"

#ifndef LINK_C
#define LINK_C

int pino, ino;
int link (char *oldName, char *newName) {
    int oldINODE, newINODE;
    MINODE *oldMIP, *newMIP;
    char tempDir[64];
    char tempBase[64];
    char *dirPath, *basePath;
    char *cp;
    printf("oldname: %s\n",oldName);
    printf("newName: %s\n",newName);

    oldINODE = getino(dev, oldName);              //getIno and mip pointer for 'oldname' path.
    printf("oldINODE: %d\n",oldINODE);
    oldMIP = iget(dev, oldINODE);

    if (S_ISDIR(oldMIP->INODE.i_mode)) {                         //check if oldname mip inode is a dir or not. 
        printf("Cannot link to a directory\n");
        iput(oldMIP);
        return -1;
    }

    printf("Original pathname located.\n");

    strncpy(tempDir,newName,64);                                  //convert newpath to dirname to find potential location of new file.
    dirPath = dirname(tempDir);
    printf("dirPath: %s\n",dirPath);

    newINODE = getino(dev, dirPath);              //Get inode and mip of the location for the new link file.           
    printf("newINODE: %d\n",newINODE);
    newMIP = iget(dev, newINODE);

    strncpy(tempBase,newName,64);                               //Get baseName of newName to search if it already exists.
    basePath = basename(tempBase);

    get_block(dev, newMIP->INODE.i_block[0], buf);
    dp = (DIR *)buf;
    cp = buf;

    printf("basePath: %s\n",basePath);

    //search through dir:
    while (cp < &buf[BLKSIZE]) {
        //printf("dp->name: %s\n",dp->name);
        if (strcmp(dp->name, basePath) == 0) {
            printf("Desired file already exists. Cannot create link.\n");
            iput(oldMIP);
            iput(newMIP);
            return -1;
        }

        cp += dp->rec_len;
        dp = (DIR *)cp;
    }


    enter_name(newMIP, oldINODE, basePath);

    oldMIP->INODE.i_links_count += 1;
    oldMIP->dirty = 1;
    printf("Links Count: %d\n",oldMIP->INODE.i_links_count);
    printf("New Links Count: %d\n",newMIP->INODE.i_links_count);

    put_block(dev, oldMIP->ino, buf);           //Not Writing Inode back.
    //MINODE *newiNode = iget(dev, oldMIP);
    
    iput(oldMIP);
    iput(newMIP);
    return 0;
}

int unlink (char *pathname) {
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
    }
    else{
        dev = running->cwd->dev;
    }


    printf("parent: %s   child: %s   \n", tempparent, tempchild);
    if(strcmp(tempparent,".")==0){
        pino = running->cwd->ino;
        pip = iget(dev,running->cwd->ino);
    }

    
    //grab the parent's inode:
    //pino = getino(dev, tempparent);
    else{
        pip = iget(dev, pino);
    }
    
    if (pip == 0) {
        printf("Desired location not found.\n");
        iput(mip);
        return -1;
    }
    ino = search(pip, tempchild);
    printf("Inode: %d\n", ino);
    printf("Parent ino: %d\n", pino);
    mip =  iget(dev, ino);

    //check if parent is a there or a dir:
    if (!S_ISREG(mip->INODE.i_mode)) {
        printf("Desired location is not a file.\n");
        iput(mip);
        return -1;
    }

    printf("MIP INODE: %d\n", ino);

    mip->INODE.i_links_count -= 1;
    printf("Link count: %d\n", mip->INODE.i_links_count);
    if(mip->INODE.i_links_count==0){
        
        printf("No more links with this file, deallocating:\n");
        truncate(dev,mip);
        //incFreeInodes(dev);
        //BASICALLY TRUNCATE: 
        // for(int i = 0; i < 12; i++){
        //     if(mip->INODE.i_block[i] !=0){
        //         printf("does it go here\n");
        //         int tempparentdir = pip->INODE.i_block[i];
        //         mip->INODE.i_block[i] = 0;
        //         //now free the parent's dir:
        //         freeblock(dev, tempparentdir);
        //     }

        // }
        //free the inode:
        idealloc(dev,ino);
    }

    rmchild(pip, tempchild);

    pip->INODE.i_atime = pip->INODE.i_ctime = pip->INODE.i_mtime = time(0L);
    mip->dirty = 1;
    pip->dirty = 1;
    iput(&pip->INODE);
    iput(&mip->INODE);
}

int showLinks (char *pathname) {
    int iNum;
    MINODE *mip;

    iNum = getino(running->cwd->dev, pathname);

    if (iNum == 0) {
        printf("pathname not found.\n");
        return -1;
    }

    mip = iget(running->cwd->dev, iNum);

    printf("%d\n",mip->INODE.i_links_count);
    iput(mip);
    return mip->INODE.i_links_count;
}

int symlink(char *oldName, char *newName){
    int oldino, newino, creat_test;
    MINODE *omip, *nmip;
    char *basePath, *dirPath;

    oldino = getino(running->cwd->dev, oldName);
    if(oldino ==0){
        printf("exists\n");
        return -1;
    }

    omip = iget(dev, oldino);
    creat_sym_file(newName);

    printf("Newfile: %s\n", oldName);
    newino = getino(dev, newName);
    nmip = iget(dev, newino);
    INODE *nino = &nmip->INODE;
    nmip->dirty = 1;
    nmip->refCount ++;
    nino->i_links_count++;
    //nino->i_mode = 0xA1A4;
    nino->i_size = strlen(oldName);
    return 0;
}

#endif