#ifndef _HAD_CHECKGROUP_H
#define _HAD_CHECKGROUP_H

struct file {
    char *tag;
    char *comment;
    int npart;
    long *artno;
    int new;
    long size;
    int part0;
};



extern FILE *conin, *conout;



int nntp_put(char *fmt, ...);
int nntp_resp(void);

#endif /* checkgroup.h */