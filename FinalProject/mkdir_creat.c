#ifndef mkdir_creat_c
#define mkdir_creat_c

#include "ls_cd_pwd.c"
#include "alloc_dealloc.c"

int mkdir(char *pathname){
    char tempparent[BLKSIZE], tempchild[BLKSIZE];
    int pino;
    MINODE *pip;
    strcpy(tempbase, pathname);
    strcpy(tempdir, pathname);
    tempparent = dirname(tempdir);
    tempchild = basename(tempbase);

    pino = getino(&dev, tempparent);
    pip = iget(dev, pino);



    //last step: put that dir(block) back to the disk: 
    iput(pip);
}

int mymkdir(MINODE *pip, char *name){
    MINODE *mip;
    INODE *ip;

    //efinitely gotta get the alloc:
    int ino = ialloc(dev);
    int bno = balloc(dev);

    mip = iget(dev, ino);
    ip = &mip->INODE;

    ip->i_mode = 0040775;
    ip->i_uid = running->uid;
    ip->i_gid = running->gid;
    ip->i_size = BLKSIZE;
    ip->link_count = 2;
    ip->iatime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_block[0] = bno;
    
    for(int i = 1; i < 14; i++){
        ip->i_block[i] = 0;
    }


    mip->dirty = 1;
    iput(mip);

    //now lets create . and .. yuh yuh



}


int enter_name(MINODE *pip, int myino, char *myname){
    int i = 0;

    char *blk, *cp;
    int need_len, ideal_last_len, rem;
    int bno;

    for ( i = 0; i < (pip->INODE->i_size / BLKSIZE); i++){
        if(pip->INODE->i_block[i] == 0){
            break;
        }

        bno = pip->INODE.i_block[i];
        
        get_block(dev, bno, buf);

        cp = buf;
        dp = (DIR *)buf;

        //go through last block:
        blk = pip->INODe.i_block[i];
        while(cp + dp->rec_len < buf + BLKSIZE){
            cp += dp->rec_len;
            dp = (DIR *) cp;
        }
        //now will be pointing at last block
        need_len = 4*((8 + strlen(myname) + 3)/4);

        ideal_last_len = 4*((8 + dp->name_len + 3)/4);
        rem = dp->rec_len - ideal_last_len;

        if(rem >= need_len){
            dp->rec_len = ideal_last_len;

            cp += dp->rec_len;
            dp = (DIR *)cp;

            strcpy(dp->name, myname);
            dp->inode = myino;
            dp->rec_len = BLKSIZE - (cp - blk);
            dp->name_len = strlen(myname);
            
            putblock(bno);
            return 1;
        }
        
        

    }    
    //need to allocate cuz no more data blocks:

    bno = balloc(dev);
    pip->INODE.i_block[i] = bno;
    pip->INODE.i_size += BLKSIZE;
    pip->dirty = 1;

    get_block(dev, bno, buf);
    cp = buf;
    dp = (DIR *) buf;

    strcpy(dp->name, myname);
    dp->inode = myino;
    dp->rec_len = BLKSIZE - (cp - blk);
    dp->name_len = strlen(myname);

    putblock(bno);
    return 0;

}
#endif