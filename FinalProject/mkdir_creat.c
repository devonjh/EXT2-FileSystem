#ifndef mkdir_creat_c
#define mkdir_creat_c

#include "ls_cd_pwd.c"
#include "alloc_dealloc.c"

int pino, checkerino;

int mkkdir(char *pathname){
    //makes sure that i can paarse in the parent name nad child name. childname will be the one created.
    char tempbase[BLKSIZE], tempdir[BLKSIZE];
    char *tempparent, *tempchild;
    MINODE *pip, *mip;
    int mino;
    strcpy(tempbase, pathname);
    strcpy(tempdir, pathname);

    //from root:
    tempparent = dirname(tempdir);
    tempchild = basename(tempbase);
    if(pathname[0] == '/'){
        dev = root->dev;
        pino = getino(dev, tempparent);
        pip = iget(dev,pino);
    }
    else{
        dev = running->cwd->dev;
        pino = running->cwd->ino;
        pip = iget(dev, pino);
    }
    
    if(pip == 0){
        printf("Desired location not found. \n");
        iput(0);
        return -1;
    }

    printf("parent: %s   child: %s   \n", tempparent, tempchild);
    //still gotta verify if it exists in the parent directory lmao:
    //search:
    if(search(pip,tempchild)!=0){
        printf("Already exists. \n");
        iput(pip);
        return -1;
    }
    //run mymakedir which adds the actual file there:
    mymkdir(pip, tempchild);

    //last step: put that dir(block) back to the disk: 
    pip->dirty =  1;
    iput(pip);
}

int mymkdir(MINODE *pip, char *name){
    MINODE *mip;
    INODE *ip;

    // gotta allocate inode and block for new dir:
    int ino = ialloc(dev);
    int bno = balloc(dev);

    mip = iget(dev, ino);   //saving/loading minode 
    ip = &mip->INODE;   //faster referencing

    //set everything to the directory settings:
    ip->i_mode = 0040775;
    ip->i_uid = running->uid;
    ip->i_gid = running->pid;
    ip->i_size = BLKSIZE;
    ip->i_links_count = 2;      //cuz of two file 
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_block[0] = bno;
    
    //set everything after the first block to 0, since it'll be empty neways'
    for(int i = 1; i < 14; i++){
        ip->i_block[i] = 0;
    }


    mip->dirty = 1; //been changed so mark dirty
    iput(mip);      //save this ish in block

    //now lets create . and .. yuh yuh
    get_block(dev, bno, buf);
    //update dir contents:
    cp = buf;
    dp = (DIR *) buf;

    strcpy(dp->name, ".");
    dp->inode = ino;
    dp->name_len = 1;
    //ideal len:
    dp-> rec_len = (4 * ((8 + 1 + 3) / 4));

    printf("dp->name: %s\n", dp->name);
    printf("dp->name_len: %d\n", dp->name_len);
    printf("dp->ino: %d\n", dp->inode);
    printf("dp->reclen : %d\n", dp->rec_len);
    put_block(dev, ino, buf);
    //move to ".." now:
    cp += dp->rec_len;
    dp = (DIR *)cp;

    strcpy(dp->name, "..");
    dp->inode = pino;
    dp->name_len = 2;
    dp-> rec_len = BLKSIZE - (4 * ((8 + 2 + 3) / 4));
    printf("dp->name: %s\n", dp->name);
    printf("dp->name_len: %d\n", dp->name_len);
    printf("dp->ino: %d\n", dp->inode);
    printf("dp->reclen : %d\n", dp->rec_len);

    put_block(dev, bno, buf);
    //put in name:
    enter_name(pip, ino, name);
    return 1;
}

int enter_name(MINODE * pip, int myino, char * myname) {
  int i = 0;
  int need_len, ideal_last_len, rem;
  int bno;
  char buf[BLKSIZE];

  printf("name inserting: %s\n", myname);
  //need to add the file count thoo:
  pip->INODE.i_links_count += 1;
  
  //each data block of parent needs to get across:
  for (i = 0; i < 12; i++) {

    printf("Block position: %d\n", i);
    //no more space?
    if (pip->INODE.i_block[i] == 0) {
        break;
    }
    //bno = pip->INODE.i_block[i];
    //grab parent's data in a buf:
    get_block(dev, pip->INODE.i_block[i], buf);

    cp = buf;
    dp = (DIR *) buf;

    //go through last block:
    while (cp + dp->rec_len < buf + BLKSIZE) {
      printf("%d\t%d\t%d\t\t%s\n", dp->inode, dp->rec_len, dp->name_len, dp->name);
      cp += dp->rec_len;
      dp = (DIR * ) cp;
    }
    printf("Blockposition: %d\n", i);
    printf("Last entry name: %s\n\n", dp->name);
    //printf("Does it go to the last block? dp should be last entry\n");
    //now will be pointing at last block
    need_len = 4 * ((8 + strlen(myname) + 3) / 4);

    ideal_last_len = 4 * ((8 + dp->name_len + 3) / 4);
    rem = dp->rec_len - ideal_last_len;
    printf("reclen: %d   need: %d   rem: %d\n", dp->rec_len, need_len, rem);
    //there are available blocks:
    if (rem >= need_len) {
      dp->rec_len = ideal_last_len;
      printf("Last entry name: %s\n\n", dp->name);
      cp += dp ->rec_len;
      dp = (DIR * ) cp;
      strncpy(dp->name, myname, strlen(myname));
      dp->inode = myino;
      dp-> rec_len = BLKSIZE - (cp - buf);
      dp-> file_type = 2;
      dp-> name_len = strlen(myname);

      put_block(dev, pip->INODE.i_block[i], buf);
      return 1;
    }
  }

  //need to allocate cuz no more data blocks , i < available blcoks:
  //i--;
  printf("does it put here??\n");
  printf("bpos: i = %d\n", i);
  int newbno = balloc(dev);
  printf("last dir? %s\n", dp->name);
  printf("baloc() : %d\n", newbno);
  //pip->INODE.i_block[i] = balloc(dev);
  //pip->INODE.i_block[i] = bno;
  pip->INODE.i_size += BLKSIZE;
  pip->INODE.i_blocks += BLKSIZE / 512;
  pip->INODE.i_block[i] = newbno;
  pip->dirty = 1;

  //MOVE UP TO GET THROUGH THE NEXT BLOCK:
  get_block(dev, newbno, buf);
  cp = buf;
  dp = (DIR *) buf;
  

  //create new directory here:
  strncpy(dp->name, myname, strlen(myname));
  printf("new dir? %s\n", dp->name);
  dp->inode = myino;
  //dp->rec_len = BLKSIZE - (cp - buf);
  dp->rec_len = BLKSIZE;
  dp->file_type = 2;
  dp->name_len = strlen(myname);

  put_block(dev, pip->INODE.i_block[i], buf);
  return 1;
}

int creat_file(char *pathname){
    //similar as mkdir: 
    //makes sure that i can paarse in the parent name nad child name. childname will be the one created.
    
    char tempbase[BLKSIZE], tempdir[BLKSIZE];
    char *tempparent, *tempchild;
    MINODE *pip, *mip;
    int mino;
    strcpy(tempbase, pathname);
    strcpy(tempdir, pathname);

    //verify its there:
    
        
    //from root:
    tempparent = dirname(tempdir);
    tempchild = basename(tempbase);
    if(pathname[0] == '/'){
        dev = root->dev;
        pino = getino(dev, tempparent);
        pip = iget(dev,pino);
        if(search(pip,pathname)!=0){
            printf("Already exists");
            return -1;
        }
    }
    else{
        dev = running->cwd->dev;
        pino = running->cwd->ino;
        pip = iget(dev, pino);
    }
    
    if(pip == 0){
        printf("Desired location not found. \n");
        iput(0);
        return -1;
    }
    //now we can call mycreat:
    my_creat(pip, tempchild);
}

int my_creat(MINODE *pip, char *name){
    //very similar to mymkdir:
    MINODE *mip;
    INODE *ip;

    // gotta allocate inode and block for new file:
    int ino = ialloc(dev);
    int bno = balloc(dev);

    mip = iget(dev, ino);   //saving/loading minode 
    ip = &mip->INODE;   //faster referencing

    //set everything to the file settings:
    ip->i_mode = 0100644;
    ip->i_uid = running->uid;
    ip->i_gid = running->pid;
    ip->i_size = 0;             //no data blocks so size is 0
    ip->i_links_count = 1;      //cuz a file 
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_block[0] = bno;
    
    for(int i = 0; i < 14; i++){
        ip->i_block[i] = 0;
    }

    mip->dirty = 1; //been changed so mark dirty
    iput(mip);      //save this ish in block
    //now lets put it in the from the parent:
    enter_creat_name(pip, ino, name);

    return 0;
}

int enter_creat_name(MINODE *pip, int myino, char *myname){
    int i = 0;
    int need_len, ideal_last_len, rem;
    int bno;

    printf("name inserting: %s\n", myname);
    printf("inode's size: %d\n", pip->INODE.i_size);

    //each data block of parent needs to get across:
    for ( i = 0; i < 12; i++){
        if(pip->INODE.i_block[i] == 0){
            break;
        }

        //bno = pip->INODE.i_block[i];

        //grab parent's data in a buf:
        get_block(dev, pip->INODE.i_block[i], buf);

        cp = buf;
        dp = (DIR *)buf;

        //go through last block:
        while(cp + dp->rec_len < buf + BLKSIZE){
            printf("%d\t%d\t%d\t\t%s\n",dp->inode, dp->rec_len, dp->name_len, dp->name);
            cp += dp->rec_len;
            dp = (DIR *) cp;
            printf("last block's name: %s\n", dp->name);
        }
        //printf("Does it go to the last block? dp should be last entry\n");
        //now will be pointing at last block
        need_len = 4*((8 + strlen(myname) + 3)/4);

        ideal_last_len = 4*((8 + dp->name_len + 3)/4);
        rem = dp->rec_len - ideal_last_len;
        printf("reclen: %d   need: %d   rem: %d\n", dp->rec_len, need_len, rem);
        //there are available blocks:
        if(rem >= need_len){
            dp->rec_len = ideal_last_len;
            cp += dp->rec_len;
            dp = (DIR *)cp;
            strcpy(dp->name, myname);
            dp->inode = myino;
            dp->file_type = 1;
            dp->rec_len = BLKSIZE - (cp - buf);
            dp->name_len = strlen(myname);
            
            put_block(dev, pip->INODE.i_block[i], buf);
            return 1;
        }
    }    

    //need to allocate cuz no more data blocks , i < available blcoks:

    printf("does it put here??\n");
    printf("i = %d\n", i);
    int binoo = balloc(dev);
    printf("bino : %d\n", binoo);
    pip->INODE.i_block[i] = binoo;
    //pip->INODE.i_block[i] = bno;
    pip->INODE.i_size += BLKSIZE;
    pip->dirty = 1;

    get_block(dev, pip->INODE.i_block[i], buf);
    cp = buf;
    dp = (DIR *) buf;

    strcpy(dp->name, myname);
    dp->inode = myino;
    dp->file_type = 1;
    dp->rec_len = BLKSIZE - (cp - buf);
    dp->name_len = strlen(myname);

    put_block(dev, pip->INODE.i_block[i], buf);
    return 1;
}

int creat_sym_file(char *pathname){
    //similar as mkdir: 
    //makes sure that i can paarse in the parent name nad child name. childname will be the one created.
    
    char tempbase[BLKSIZE], tempdir[BLKSIZE];
    char *tempparent, *tempchild;
    MINODE *pip, *mip;
    int mino;
    strcpy(tempbase, pathname);
    strcpy(tempdir, pathname);

    //verify its there:
    
        
    //from root:
    tempparent = dirname(tempdir);
    tempchild = basename(tempbase);
    if(pathname[0] == '/'){
        dev = root->dev;
        pino = getino(dev, tempparent);
        pip = iget(dev,pino);
        if(search(pip,pathname)!=0){
            printf("Already exists");
            return -1;
        }
    }
    else{
        dev = running->cwd->dev;
        pino = running->cwd->ino;
        pip = iget(dev, pino);
    }
    
    if(pip == 0){
        printf("Desired location not found. \n");
        iput(0);
        return -1;
    }
    my_sym_creat(pip, tempchild);
}

int my_sym_creat(MINODE *pip, char *name){
    //very similar to mymkdir:
    MINODE *mip;
    INODE *ip;

    // gotta allocate inode and block for new file:
    int ino = ialloc(dev);
    int bno = balloc(dev);

    mip = iget(dev, ino);   //saving/loading minode 
    ip = &mip->INODE;   //faster referencing

    //set everything to the file settings:
    ip->i_mode = 0xA000;
    ip->i_uid = running->uid;
    ip->i_gid = running->pid;
    ip->i_size = 0;             //no data blocks so size is 0
    ip->i_links_count = 1;      //cuz a file 
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_block[0] = bno;
    
    for(int i = 0; i < 14; i++){
        ip->i_block[i] = 0;
    }

    mip->dirty = 1; //been changed so mark dirty
    iput(mip);      //save this ish in block
    //now lets put it in the from the parent:
    enter_creat_sym_name(pip, ino, name);

    return 0;
}

int enter_creat_sym_name(MINODE *pip, int myino, char *myname){
    int i = 0;
    int need_len, ideal_last_len, rem;
    int bno;

    printf("name inserting: %s\n", myname);
    printf("inode's size: %d\n", pip->INODE.i_size);

    //each data block of parent needs to get across:
    for ( i = 0; i < 12; i++){
        if(pip->INODE.i_block[i] == 0){
            break;
        }

        //bno = pip->INODE.i_block[i];

        //grab parent's data in a buf:
        get_block(dev, pip->INODE.i_block[i], buf);

        cp = buf;
        dp = (DIR *)buf;

        //go through last block:
        while(cp + dp->rec_len < buf + BLKSIZE){
            printf("%d\t%d\t%d\t\t%s\n",dp->inode, dp->rec_len, dp->name_len, dp->name);
            cp += dp->rec_len;
            dp = (DIR *) cp;
            printf("last block's name: %s\n", dp->name);
        }
        //printf("Does it go to the last block? dp should be last entry\n");
        //now will be pointing at last block
        need_len = 4*((8 + strlen(myname) + 3)/4);

        ideal_last_len = 4*((8 + dp->name_len + 3)/4);
        rem = dp->rec_len - ideal_last_len;
        printf("reclen: %d   need: %d   rem: %d\n", dp->rec_len, need_len, rem);
        //there are available blocks:
        if(rem >= need_len){
            dp->rec_len = ideal_last_len;
            cp += dp->rec_len;
            dp = (DIR *)cp;
            strcpy(dp->name, myname);
            dp->inode = myino;
            dp->file_type = 7;
            dp->rec_len = BLKSIZE - (cp - buf);
            dp->name_len = strlen(myname);
            
            put_block(dev, pip->INODE.i_block[i], buf);
            return 1;
        }
    }    

    //need to allocate cuz no more data blocks , i < available blcoks:

    printf("does it put here??\n");
    printf("i = %d\n", i);
    int binoo = balloc(dev);
    printf("bino : %d\n", binoo);
    pip->INODE.i_block[i] = binoo;
    //pip->INODE.i_block[i] = bno;
    pip->INODE.i_size += BLKSIZE;
    pip->dirty = 1;

    get_block(dev, pip->INODE.i_block[i], buf);
    cp = buf;
    dp = (DIR *) buf;

    strcpy(dp->name, myname);
    dp->inode = myino;
    dp->file_type = 7;
    dp->rec_len = BLKSIZE - (cp - buf);
    dp->name_len = strlen(myname);

    put_block(dev, pip->INODE.i_block[i], buf);
    return 1;
}
#endif