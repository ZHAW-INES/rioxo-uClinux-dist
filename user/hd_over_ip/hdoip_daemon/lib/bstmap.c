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

static int cmp(t_keyvalue* a, t_keyvalue* b)
{
    return strcmp(a->key, b->key);
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

    ret = bst_add(root, kv, cmp);

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

    p = bst_find(root, &kv, cmp);

    if (!p) return 0;

    return p->value;
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

void bstmap_cnt_elements(t_bstmap* root, int *cnt)
{
    if(root) {
        *cnt += 1;
        bstmap_cnt_elements(root->left, cnt);
        bstmap_cnt_elements(root->right, cnt);
    }
}

void bstmap_get_all(t_bstmap* root, t_keyvalue *arr[], int *index) 
{
    if(root) {
        arr[(*index)] = root->elem;
        *index += 1;
        bstmap_get_all(root->left, arr, index);
        bstmap_get_all(root->right, arr, index);
    }
}

void bstmap_setp(t_bstmap** root, char* key, void* value)
{
    t_keyvalue* ret = 0;
    t_keyvalue* kv = 0;

    kv = malloc(sizeof(t_keyvalue));
    if (!kv) {
        printf("bstmap_setp.malloc(sizeof(t_keyvalue)): out of memory");
        return 0;
    }

    kv->key = key;
    kv->value = value;

    ret = bst_add(root, kv, cmp);

    if (ret) {
        free(ret);
    }
}


