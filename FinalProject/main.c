#include "util.c"  // YOUR iget_iput_getino.c file with
                               // get_block/put_block, tst/set/clr bit function
#include "ls_cd_pwd.c"              
#include "mkdir_creat.c"
#include "rmdir.c"
#include "link.c"

main(int argc, char *argv[ ])   // run as a.out [diskname]
{
  int tempMode;

    if (argc > 1)
     disk = argv[1];

  printf("\n");
  system("./diskcopy.sh");

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
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;
  printf("ninodes = %d\n", sp->s_inodes_count);
  printf("nblocks = %d\n", sp->s_blocks_count);


  //Read GD block to get bmap, imap, iblock (and print their values)
  get_block(fd, 2, buf);
  gp = (GD *)buf;
  imap = gp->bg_inode_bitmap;
  bmap = gp->bg_block_bitmap;
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
     printf("\ninput command : [ls|cd|pwd|mkdir|creat|link|rmdir|quit] ");
     memset(&pathname[0], 0, sizeof(pathname));

     //use fgets() to get user inputs into line[128]
     fgets(line, 128, stdin);

     //kill the \r at end 
     line[strcspn(line, "\n")] = 0;
     line[strcspn(line, "\r")] = 0;
     
     if (strcmp(line, "") == 0) {
       continue;
     }

     sscanf(line,"%s %s %s %s",cmd, pathname, pathname2, pathname3);

     //Use sscanf() to extract cmd[ ] and pathname[] from line[128]
     printf("cmd=%s pathname=%s\n", cmd, pathname);

     // execute the cmd
     if (strcmp(cmd, "ls")==0){
        ls(pathname);
     }
     if (strcmp(cmd, "cd")==0){
        chdir(pathname);
     }
     if (strcmp(cmd, "pwd")==0){
        pwd(running->cwd);
     }
     if (strcmp(cmd, "mkdir")==0){
        printf("test");
        mkkdir(pathname);
     }
     if(strcmp(cmd, "link") == 0){
       link(pathname, pathname2);
     }
     if(strcmp(cmd, "unlink")==0){
       unlink(pathname);
     }
     if(strcmp(cmd, "symlink")==0){
       symlink(pathname, pathname2);
     }
     if(strcmp(cmd, "rmdir")==0){
       rmdir(pathname);
     }
     if(strcmp(cmd, "creat")==0){
       creat_file(pathname);
     }
    //  if(strcmp(cmd,"open")==0) {
    //    openFile(pathname,atoi(pathname2));
    //  }
    //  if(strcmp(cmd,"close")==0) {
    //    closeFile(atoi(pathname));
    //  }
    //  if(strcmp(cmd,"pfd")==0) {
    //    pfd();
    //  }
    //  if(strcmp(cmd,"lseek")==0) {
    //    lseekFD(atoi(pathname),atoi(pathname2));
    //  }
    //  if(strcmp(cmd,"read")==0) {
    //    myRead(atoi(pathname),buf, atoi(pathname2));
    //  }
    //  if(strcmp(cmd,"printSize")==0) {
    //    printSize(pathname);
    //  }
    //  if(strcmp(cmd,"touch")==0) {
    //    touch(pathname);
    //  }
    //  if(strcmp(cmd, "stat")==0) {
    //    statFile(pathname);
    //  }
     if(strcmp(cmd,"showLinks")==0) {
       showLinks(pathname);
     }
     if (strcmp(cmd, "quit")==0){
        quit();
     }
     
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

  // get_block(dev, 2, buf);
  // gp = (GD *)buf;

  // imap = gp->bg_inode_bitmap;
  // bmap = gp->bg_block_bitmap;

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
 


