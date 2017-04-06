#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h> 

#include "p1.h"

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;

int dev;
int nblocks;
int ninodes;
int bmap;
int imap;
int iblock;
int fd;
char *cp;
char buf[BLKSIZE];

