/*
 * bstmap.c
 *
 *  Created on: 02.12.2010
 *      Author: alda
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bstmap.h"
#include "debug.h"

typedef struct {
    void (*f)(char*, char*, void*);
    int (*c)(char*, char*, void*);
    void *d, *cd;
} t_kvd;


int cmp(t_keyvalue* a, t_keyvalue* b)
{
    return strcmp(a->key, b->key);
}


/** Stores copy of Key/Value in MAP (String)
 *
 * @return the old Key/Value pair if available with the same Key
 */
t_keyvalue* bstmap_add(t_bstmap** root, char* key, char* value)
{
    t_keyvalue* ret = 0;
    t_keyvalue* kv = 0;

    kv = malloc(sizeof(t_keyvalue));
    if (!kv) {
        report("bstmap_add.malloc(sizeof(t_keyvalue)): out of memory");
        return 0;
    }

    kv->key = malloc(strlen(key) + 1);
    if (!kv->key) {
        report("bstmap_add.malloc(strlen(key)): out of memory");
        free(kv);
        return 0;
    }
    kv->value = malloc(strlen(value) + 1);
    if (!kv->value) {
        report("bstmap_add.malloc(strlen(value)): out of memory");
        free(kv->key);
        free(kv);
        return 0;
    }

    strcpy(kv->value, value);
    strcpy(kv->key, key);

    ret = bst_add(root, kv, (bstc*)cmp);

    return ret;
}

/** Replace or Add a new Key/Value pair (String)
 *
 */
void bstmap_set(t_bstmap** root, char* key, char* value)
{
    t_keyvalue *ret = bstmap_add(root, key, value);

    if (ret) {
        free(ret->key);
        free(ret->value);
        free(ret);
    }
}

/** Get Value based on Key (String)
 *
 */
char* bstmap_get(t_bstmap* root, char* key)
{
    t_keyvalue kv, *p;
    kv.key = key;

    p = bst_find(root, &kv, (bstc*)cmp);

    if (!p) return 0;

    return p->value;
}


/** Remove Key/Value pair (String)
 *
 */
void bstmap_remove(t_bstmap** root, char* key)
{
    t_keyvalue kv, *ret;
    kv.key = key;

    ret = bst_remove(root, &kv, (bstc*)cmp);

    if (ret) {
        free(ret->key);
        free(ret->value);
        free(ret);
    }
}

// Pointer version


/** Replace or Add a new Key/Value pair (Pointer)
 *
 */
void bstmap_setp(t_bstmap** root, char* key, void* value)
{
    t_keyvalue* ret = 0;
    t_keyvalue* kv = 0;

    kv = malloc(sizeof(t_keyvalue));
    if (!kv) {
        report("bstmap_setp.malloc(sizeof(t_keyvalue)): out of memory");
        return;
    }

    kv->key = key;
    kv->value = value;

    ret = bst_add(root, kv, (bstc*)cmp);

    if (ret) {
        free(ret);
    }
}


/** Remove Key/Value pair (Pointer)
 *
 */
char* bstmap_removep(t_bstmap** root, char* key)
{
    t_keyvalue kv, *p;
    kv.key = key;

    p = bst_remove(root, &kv, (bstc*)cmp);

    if (!p) return 0;

    char* ret = p->value;
    free(p);

    return ret;
}

// Callback

static void callback(t_keyvalue* elem, t_kvd* kvd)
{
    kvd->f(elem->key, elem->value, kvd->d);
}

static void callback_traverse_cond(t_keyvalue* elem, t_kvd* kvd)
{
    if (kvd->c)
        if (kvd->c(elem->key, elem->value, kvd->cd))
            if (kvd->f) kvd->f(elem->key, elem->value, kvd->d);
}

static int callback_traverse_freep(t_keyvalue* elem, t_kvd* kvd)
{
    if (kvd->f) kvd->f(elem->key, elem->value, kvd->d);
    free(elem);
    return 1;
}

static int callback_traverse_cond_freep(t_keyvalue* elem, t_kvd* kvd)
{
    int ret = 0;
    if (kvd->c)
        if (kvd->c(elem->key, elem->value, kvd->cd)) {
            if (kvd->f) kvd->f(elem->key, elem->value, kvd->d);
            free(elem);
            ret = 1;
        }
    return ret;
}

static int callback_freep(t_keyvalue* kv, void UNUSED *d)
{
    free(kv);
    return 1;
}


/** Free complete map (Pointer)
 *
 */
void bstmap_freep(t_bstmap** root)
{
    bst_free(root, (bstc*)callback_freep, 0);
    *root = 0;
}

/** Traverse and callback (Pointer & String)
 *
 */
void bstmap_traverse(t_bstmap* root, void (*f)(char*, char*, void*), void* d)
{
    t_kvd kvd;
    kvd.f = f;
    kvd.d = d;
    bst_traverse(root, (bstt*)callback, &kvd);
}

/** Traverse and callback and then free (Pointer)
 *
 */
void bstmap_traverse_freep(t_bstmap** root, void (*f)(char*, char*, void*), void* d)
{
    t_kvd kvd;
    kvd.f = f;
    kvd.d = d;
    bst_free(root, (bstc*)callback_traverse_freep, &kvd);
}

/** Traverse and callback under condition (String & Pointer)
 *
 */
void bstmap_ctraverse(t_bstmap* root, int (*c)(char*, char*, void*), void* cd, void (*f)(char*, char*, void*), void* d)
{
    t_kvd kvd;
    kvd.f = f;
    kvd.d = d;
    kvd.c = c;
    kvd.cd = cd;
    bst_traverse(root, (bstt*)callback_traverse_cond, &kvd);
}

/** Traverse and (callback and then free) under condition (Pointer)
 *
 */
void bstmap_ctraverse_freep(t_bstmap** root, int (*c)(char*, char*, void*), void* cd, void (*f)(char*, char*, void*), void* d)
{
    t_kvd kvd;
    kvd.f = f;
    kvd.d = d;
    kvd.c = c;
    kvd.cd = cd;
    bst_free(root, (bstc*)callback_traverse_cond_freep, &kvd);
}
