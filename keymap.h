#ifndef _HAD_KEYMAP_H
#define _HAD_KEYMAP_H

enum _type { t_array, t_list, t_key };

struct keymap {
    enum _type type;
    union {
	struct km_array *a;
	struct km_list *l;
	struct km_key *k;
    } v;
};

struct km_array {
    struct *keymap[MAX_KEY];
};

struct km_list {
    struct km_list *next;
    int key;
    struct keymap *val;
};

struct km_key {
};

#endif /* keymap.h */
