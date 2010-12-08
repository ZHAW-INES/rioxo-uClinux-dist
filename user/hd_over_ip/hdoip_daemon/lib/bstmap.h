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

t_keyvalue* bstmap_add(t_bstmap** root, char* key, char* value);
void bstmap_set(t_bstmap** root, char* key, char* value);
char* bstmap_get(t_bstmap* root, char* key);
void bstmap_print(t_bstmap* root, int n);
void bstmap_get_all(t_bstmap* root, t_keyvalue *arr[], int *index);
void bstmap_cnt_elements(t_bstmap* root, int *cnt);
void bstmap_setp(t_bstmap** root, char* key, void* value);


#endif /* BSTMAP_H_ */
