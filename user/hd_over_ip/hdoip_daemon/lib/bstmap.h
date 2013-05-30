/*
 * bstmap.h
 *
 *  Created on: 02.12.2010
 *      Author: alda
 */

#ifndef BSTMAP_H_
#define BSTMAP_H_

#include "bstree.h"

typedef struct {
    char* key;
    char* value;
} t_keyvalue;


typedef t_bstn t_bstmap;

int bstmap_empty(t_bstmap* root);

void bstmap_set(t_bstmap** root, char* key, char* value);
char* bstmap_get(t_bstmap* root, char* key);
void bstmap_remove(t_bstmap** root, char* key);

void bstmap_setp(t_bstmap** root, char* key, void* value);
#define bstmap_getp bstmap_get
char* bstmap_removep(t_bstmap** root, char* key);
void bstmap_freep(t_bstmap** root);

void bstmap_traverse(t_bstmap* root, void (*f)(char*, char*, void*), void* d);
void bstmap_ctraverse(t_bstmap* root, int (*c)(char*, char*, void*), void* cd, void (*f)(char*, char*, void*), void* d);

void bstmap_traverse_freep(t_bstmap** root, void (*f)(char*, char*, void*), void* d);
void bstmap_ctraverse_freep(t_bstmap** root, int (*c)(char*, char*, void*), void* cd, void (*f)(char*, char*, void*), void* d);

#endif /* BSTMAP_H_ */
