#ifndef _HAD_RANGES_H
#define _HAD_RANGES_H

struct range {
    int first, length;
    unsigned char *bitmap;
};



struct range *range_init(int lower, int upper, int num);
void range_free(struct range *r);
int range_isin(struct range *r, int n);
void range_set(struct range *r, int n);
void range_clear(struct range *r, int n);
void range_fill(struct range *r, int lower, int upper, int bit);
int range_get(struct range *r, int *lower, int *upper, int bit);

#endif /* ranges.h */
