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

int cmp(t_keyvalue* a, t_keyvalue* b)
{
    return strcmp(a->key, b->key);
}

int bstmap_free_node(t_keyvalue* kv, void UNUSED *d)
{
    free(kv);
    return 1;
}

void bstmap_freep(t_bstmap** root)
{
    bst_free(root, (bstc*)bstmap_free_node, 0);
    *root = 0;
}

t_keyvalue* bstmap_add(t_bstmap** root, char* key, char* value)
{
    t_keyvalue* ret = 0;
    t_keyvalue* kv = 0;

    kv = malloc(sizeof(t_keyvalue));
    if (!kv) {
        printf("bstmap_add.malloc(sizeof(t_keyvalue)): out of memory");
        return 0;
    }

    kv->key = malloc(strlen(key) + 1);
    if (!kv->key) {
        printf("bstmap_add.malloc(strlen(key)): out of memory");
        free(kv);
        return 0;
    }
    kv->value = malloc(strlen(value) + 1);
    if (!kv->value) {
        printf("bstmap_add.malloc(strlen(value)): out of memory");
        free(kv->key);
        free(kv);
        return 0;
    }

    strcpy(kv->value, value);
    strcpy(kv->key, key);

    ret = bst_add(root, kv, (bstc*)cmp);

    return ret;
}

void bstmap_set(t_bstmap** root, char* key, char* value)
{
    t_keyvalue *ret = bstmap_add(root, key, value);

    if (ret) {
        free(ret->key);
        free(ret->value);
        free(ret);
    }
}

char* bstmap_get(t_bstmap* root, char* key)
{
    t_keyvalue kv, *p;
    kv.key = key;

    p = bst_find(root, &kv, (bstc*)cmp);

    if (!p) return 0;

    return p->value;
}

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

void bstmap_print(t_bstmap* root, int n)
{
    if (!root) {
        return;
    }

    bstmap_print(root->left, n + 1);
    for (int m=n;m;m--) printf("        ");
    printf("(%d)> %s: %s\n", root->depth, ((t_keyvalue*)root->elem)->key, ((t_keyvalue*)root->elem)->value);
    bstmap_print(root->right, n + 1);

}

void bstmap_setp(t_bstmap** root, char* key, void* value)
{
    t_keyvalue* ret = 0;
    t_keyvalue* kv = 0;

    kv = malloc(sizeof(t_keyvalue));
    if (!kv) {
        printf("bstmap_setp.malloc(sizeof(t_keyvalue)): out of memory");
        return;
    }

    kv->key = key;
    kv->value = value;

    ret = bst_add(root, kv, (bstc*)cmp);

    if (ret) {
        free(ret);
    }
}

typedef struct {
    void (*f)(char*, char*, void*);
    int (*c)(char*, char*, void*);
    void *d, *cd;
} t_kvd;

static void callback_cond(t_keyvalue* elem, t_kvd* kvd)
{
    if (kvd->c(elem->key, elem->value, kvd->cd))
        kvd->f(elem->key, elem->value, kvd->d);
}

static void callback(t_keyvalue* elem, t_kvd* kvd)
{
    kvd->f(elem->key, elem->value, kvd->d);
}

static int callback_cond_free(t_keyvalue* elem, t_kvd* kvd)
{
    int ret = 0;
    if (kvd->c(elem->key, elem->value, kvd->cd)) {
        kvd->f(elem->key, elem->value, kvd->d);
        free(elem);
        ret = 1;
    }
    return ret;
}

static int callback_free(t_keyvalue* elem, t_kvd* kvd)
{
    kvd->f(elem->key, elem->value, kvd->d);
    free(elem);
    return 1;
}

void bstmap_traverse(t_bstmap* root, void (*f)(char*, char*, void*), void* d)
{
    t_kvd kvd;
    kvd.f = f;
    kvd.d = d;
    bst_traverse(root, (bstt*)callback, &kvd);
}

void bstmap_traverse_free(t_bstmap** root, void (*f)(char*, char*, void*), void* d)
{
    t_kvd kvd;
    kvd.f = f;
    kvd.d = d;
    bst_free(root, callback_free, &kvd);
}

void bstmap_ctraverse(t_bstmap* root, int (*c)(char*, char*, void*), void* cd, void (*f)(char*, char*, void*), void* d)
{
    t_kvd kvd;
    kvd.f = f;
    kvd.d = d;
    kvd.c = c;
    kvd.cd = cd;
    bst_traverse(root, (bstt*)callback_cond, &kvd);
}

void bstmap_ctraverse_free(t_bstmap** root, int (*c)(char*, char*, void*), void* cd, void (*f)(char*, char*, void*), void* d)
{
    t_kvd kvd;
    kvd.f = f;
    kvd.d = d;
    kvd.c = c;
    kvd.cd = cd;
    bst_free(root, callback_cond_free, &kvd);
}
