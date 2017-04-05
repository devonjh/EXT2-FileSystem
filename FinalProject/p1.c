#include "iget_iput_getino.c"  // YOUR iget_iput_getino.c file with
                               // get_block/put_block, tst/set/clr bit functions

// global variables

MINODE minode[NMINODE];        // global minode[ ] array           
MINODE *root;                  // root pointer: the /    
PROC   proc[NPROC], *running;  // PROC; using only proc[0]

int fd, dev;                               // file descriptor or dev
int nblocks, ninodes, bmap, imap, iblock;  // FS constants
int start_block;


char *disk = "mydisk";
char line[128], cmd[64], pathname[64];
//char buf[BLKSIZE];              // define buf1[ ], buf2[ ], etc. as you need

/********** Functions as BEFORE ***********/
//these were somewhat mailmain's algorithm'

int get_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  read(fd, buf, BLKSIZE);
}
int put_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  write(fd, buf, BLKSIZE);
}

//mailman
int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}


MINODE *iget(int dev, int ino)
{
  //declare local variables needed:
  int i, blk, disp;
  char buf[BLKSIZE];
  MINODE *mip;
  INODE *ip;
 
  for (i = 0; i < NMINODE; i++) {
    if (minode[i].refCount) {
      if (minode[i].dev == dev && minode[i].ino == ino) {
        minode[i].refCount++;
        return &minode[i];
      }
    }
  }

  for (i = 0; i < NMINODE; i++){
    if (minode[i].refCount == 0) {
      blk = (ino-1)/8+start_block;
      disp = (ino-1)%8;

      get_block(dev, blk, buf);
      ip = (INODE *)buf + disp;

      minode[i].INODE = *ip;
      minode[i].dev = dev;
      minode[i].ino = ino;
      minode[i].refCount = 1;
      minode[i].dirty = 0;
      return &minode[i];
    }
  }

  printf("UH OH. No remaning minodes.\n");
  exit(1);
}

int iput(MINODE *mip){ //dispose of a minode[] pointed by mip
    int blk, disp;
    
    mip->refCount--; //decrement mip refcount.

    if ((mip->refCount) || mip->dirty == 0) {
      return;
    }

    blk = (mip->ino-1)/8+start_block;
    disp = (mip->ino-1)%8;

    get_block(mip->dev, blk, buf);

    ip = (INODE *)buf + disp;
    *ip = mip->INODE;
    put_block(mip->dev, blk, buf);
}


int search(MINODE *mip, char *name)
{
  char tempName[128];
  int i;

  for (i = 0; i <=11; i++) {
    if (mip->INODE.i_block[i]) {
      get_block(dev, mip->INODE.i_block[i],buf);
      dp = (DIR *)buf;
      cp = buf;

      while (cp < &buf[BLKSIZE]) {
        strncpy(tempName, dp->name, dp->name_len);
        tempName[dp->name_len] = 0;

        if (strcmp(tempName, name) == 0) {
          return dp->inode;
        }

        cp += dp->rec_len;
        dp = (DIR *)cp;
      }
    }
  }
  return 0;
}

int getino(int *dev, char *pathname)
{
  int i, ino, blk, disp, n, tempInum, pathSize = 0;
  char  *name[128];
  char *tempname;
  char *pathElements[128];
  INODE *ip;
  MINODE *mip;

  //printf("getino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/")==0)
      return 2;

  if (pathname[0] == '/') {
    dev = root->dev;   //causing seg fault.
    tempInum = root->ino;
  }

  else {
    dev = running->cwd->dev;
    tempInum = running->cwd->ino;
  }

  tempname = strtok(pathname, "/");
  pathSize = 1;

  i = 0;
  while (tempname != NULL) {
    pathElements[i] = tempname;
    tempname = strtok(NULL, "/");
    i++;
    pathSize++;
  }

  pathElements[i] = 0;

  i = 0;
  while(pathElements[i] != NULL) {
    mip = iget(dev, tempInum);
    tempInum = search(mip, pathElements[i]);
    printf("tempInum in getino: %d\n",tempInum);

    if (tempInum == 0) {
      printf("%s does not exist.\n",pathElements[i]);
      iput(mip);
      return 0;
    }

    if ((mip->INODE.i_mode & 0040000) != 0040000) {
      printf("%s is not a directory.\n",pathElements[i]);
      iput(mip);
      return 0;
    }

    iput(mip);
    i++;
  }

  return tempInum;
}

main(int argc, char *argv[ ])   // run as a.out [diskname]
{
  if (argc > 1)
     disk = argv[1];

  if ((dev = fd = open(disk, O_RDWR)) < 0){
     printf("open %s failed\n", disk);  
     exit(1);
  }
  
  //print fd or dev to see its value!!!
  printf("fd value: %d\n",fd);

  printf("checking EXT2 FS\n");

  //Write C code to check EXT2 FS; if not EXT2 FS: exit
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  printf("s_magic = %x\n", sp->s_magic);
  if (sp->s_magic != 0xEF53){
    printf("NOT an EXT2 FS\n");
    exit(1);
  }

  printf("EXT2 FS OK. Continuing. . .\n");
     

  //get ninodes, nblocks (and print their values)
  printf("ninodes = %d\n", sp->s_inodes_count);
  printf("nblocks = %d\n", sp->s_blocks_count);


  //Read GD block to get bmap, imap, iblock (and print their values)
  get_block(fd, 2, buf);
  gp = (GD *)buf;
  printf("bg_block_bitmap: %d\n", gp->bg_block_bitmap);
  printf("bg_inode_bitmap: %d\n", gp->bg_inode_bitmap);
  start_block = gp->bg_inode_table;
  printf("inode_start_block: %d\n", start_block);

  init();         // write C code 

  mount_root();   // write C code

  printf("creating P0 as running process\n");

  running = &proc[0];         //set running pointer to point at proc[0];
  running->cwd = root; //set running's cwd   to point at / in memory;*/
  
  while(1){       // command processing loop
     printf("input command : [ls|cd|pwd|quit] ");
     memset(&pathname[0], 0, sizeof(pathname));

     //use fgets() to get user inputs into line[128]
     fgets(line, 128, stdin);

     //kill the \r at end 
     line[strcspn(line, "\n")] = 0;
     line[strcspn(line, "\r")] = 0;
     
     if (strcmp(line, "") == 0) {
       continue;
     }

     sscanf(line,"%s %s",cmd, pathname);

     //Use sscanf() to extract cmd[ ] and pathname[] from line[128]
     //printf("cmd=%s pathname=%s\n", cmd, pathname);

     // execute the cmd
     if (strcmp(cmd, "ls")==0)
        ls(pathname);
     if (strcmp(cmd, "cd")==0)
        chdir(pathname);
     if (strcmp(cmd, "pwd")==0)
        //pwd(running->cwd);
     if (strcmp(cmd, "quit")==0)
        quit();
   }
}


int init()
{
  int i = 0;

  //Initialize all minodes refCount to 0.
  for (i = 0; i < 99; i++) {
    minode[i].refCount = 0;
  }

  //Initialize proc[0]'s pid to 1, uid to 0, cwd to 0, and all fd[] to 0.
  proc[0].pid = 1;
  proc[0].uid = 0;
  proc[0].cwd = 0;

  i = 0;
  for (i = 0; i < 15; i++) {
    proc[0].fd[i] = 0;
  }

  //Initialize proc[0]'s pid to 2, uid to 1, cwd to 0 and all fd[] to 0.
  proc[1].pid = 2;
  proc[1].uid = 1;
  proc[1].cwd = 0;

  root = 0;

  i = 0;
  for (i = 0; i < 15; i++) {
    proc[1].fd[i] = 0;
  }
}

// load root INODE and set root pointer to it
int mount_root()
{  
  int nblocks, bfree, ninodes, ifree;

  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  nblocks = sp->s_blocks_count;
  bfree = sp->s_free_blocks_count;
  ninodes = sp->s_inodes_count;
  ifree = sp->s_free_inodes_count;

  printf("mount_root()\n");
  root = iget(dev, 2);         // Do you understand this?
  root->mptr = (MOUNT *)malloc(sizeof(MOUNT));
  root->mptr->ninodes = ninodes;
  root->mptr->nblocks = nblocks;
  root->mptr->dev = dev;
  root->mptr->status = 1;
  root->mptr->mountedInode = root;
  strcpy(root->mptr->name,"/");
  root->mounted = 1;
  root->refCount = 3;
}
 
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
int quit()
{
    /*
  for each minode[ ] do {
      if  minode[ ]'s refCount != 0: 
          write its INODE back to disk; 
          if dirty, write it back out in disk
  }*/
  return 0;
  
  exit(1);  // terminate program
}


