#ifndef mkdir_creat_c
#define mkdir_creat_c

#include "ls_cd_pwd.c"
#include "alloc_dealloc.c"

int mkkdir(char *pathname){
    //makes sure that i can paarse in the parent name nad child name. childname will be the one created.
    char tempbase[BLKSIZE], tempdir[BLKSIZE];
    char *tempparent, *tempchild;
    int pino;
    MINODE *pip;
    strcpy(tempbase, pathname);
    strcpy(tempdir, pathname);

    tempparent = dirname(tempdir);
    tempchild = basename(tempbase);

    printf("parent: %s   child: %s   \n", tempparent, tempchild);
    //grab the parent's inode:
    pino = getino(dev, tempparent);
    pip = iget(dev, pino);
    //still gotta verify if it exists in the parent directory lmao:
    printf("parent's mnode: %d\n", pino);

    //run mymakedir which adds the actual file there:
    mymkdir(pip, tempchild);

    //last step: put that dir(block) back to the disk: 
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
    ip->i_links_count = 2;
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_block[0] = bno;
    
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
    //move to ".." now:
    cp += dp->rec_len;
    dp = (DIR *)cp;

    strcpy(dp->name, "..");
    dp->inode = ino;
    dp->name_len = 2;
    dp-> rec_len = BLKSIZE - (4 * ((8 + 2 + 3) / 4));
    printf("dp->name: %s\n", dp->name);
    printf("dp->name_len: %d\n", dp->name_len);
    printf("dp->ino: %d\n", dp->inode);
    printf("dp->reclen : %d\n", dp->rec_len);

    put_block(dev, bno, buf);
    enter_name(pip, ino, name);

    return 1;
}


int enter_name(MINODE *pip, int myino, char *myname){
    int i = 0;
    int need_len, ideal_last_len, rem;
    int bno;

    printf("name inserting: %s\n", myname);
    printf("inode's size: %d\n", pip->INODE.i_size);

    //each data block of parent needs to get across:
    for ( i = 0; i < (pip->INODE.i_size / BLKSIZE); i++){
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
        }
        printf("Does it go to the last block? dp should be last entry\n");
        //now will be pointing at last block
        need_len = 4*((8 + strlen(myname) + 3)/4);

        ideal_last_len = 4*((8 + dp->name_len + 3)/4);
        rem = dp->rec_len - ideal_last_len;
        printf("reclen: %d   need: %d   rem: %d\n", dp->rec_len, need_len, rem);
        if(rem >= need_len){
            dp->rec_len = need_len;
            cp += dp->rec_len;
            dp = (DIR *)cp;
            strcpy(dp->name, myname);
            dp->inode = myino;
            dp->rec_len = BLKSIZE - (cp - buf);
            dp->name_len = strlen(myname);
            
            put_block(dev, pip->INODE.i_block[i], buf);
            return 1;
        }
    }    

    //need to allocate cuz no more data blocks , i < available blcoks:

    printf("does it put here??\n");
    printf("i = %d\n", i);
    pip->INODE.i_block[i] = balloc(dev);
    //pip->INODE.i_block[i] = bno;
    pip->INODE.i_size += BLKSIZE;
    pip->dirty = 1;

    get_block(dev, pip->INODE.i_block[i], buf);
    cp = buf;
    dp = (DIR *) buf;

    strcpy(dp->name, myname);
    dp->inode = myino;
    dp->rec_len = BLKSIZE - (cp - buf);
    dp->name_len = strlen(myname);

    put_block(dev, pip->INODE.i_block[i], buf);
    return 1;
}
#endif