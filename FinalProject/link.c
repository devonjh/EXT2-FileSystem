#ifndef LINK_C
#define LINK_C

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

    while (cp < &buf[BLKSIZE]) {
        printf("dp->name: %s\n",dp->name);
        if (strcmp(dp->name, basePath) == 0) {
            printf("Desired file already exists. Cannot create link.\n");
            iput(oldMIP);
            iput(newMIP);
            return -1;
        }

        cp += dp->rec_len;
        dp = (DIR *)cp;
    }

    
    return 0;
}


#endif