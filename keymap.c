struct keymap **
lookup(struct keymap *global, struct keymap *local, enum km_op what)
{
    int key;
    struct keymap **k;

    while (!end && (key=tty_readkey()) != EOF) {
	if (abort_key(key))
	    return NULL;
	if (local) {
	    k = getkey(local, key, what == km_op_ins_local);

	    if (k == NULL)
		local = NULL;
	    else if (*k == NULL) {
		if (what == km_op_ins_local)
		    return k;
		else
		    local = NULL;
	    }
	    else {
		switch ((*k)->type) {
		case t_array:
		case t_list:
		    local = *k;
		    break;
		case t_key:
		    return k;
		}
	    }
	}
	if (global) {
	    k = getkey(local, key, what == km_op_ins_global);

	    if (k == NULL)
		global = NULL;
	    else if (*k == NULL) {
		if (what == km_op_ins_global)
		    return k;
		else
		    global = NULL;
	    }
	    else {
		switch ((*k)->type) {
		case t_array:
		case t_list:
		    local = *k;
		    break;
		case t_key:
		    return k;
		}
	    }
	}
    }
}



struct keymap **
getkey(struct keymap *m, int key, enum km_op what)
{
    struct keymap *l;

    switch (m->type) {
    case t_array:
	if (key < 0 || key >= MAX_KEY)
	    return NULL;
	return &(m->v.a->keymap+key);

    case t_list:
	for (l=m->v.l; l->next && l->next->key <= key; l=l->next) {
	    if (l->next->key == key)
		return &(l->next->val);
	}

	/* XXX: insert */
	return NULL;
}

