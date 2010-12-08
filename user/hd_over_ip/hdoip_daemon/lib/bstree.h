/*
 * bstree.h
 *
 * balanced search tree
 *
 *  Created on: 02.12.2010
 *      Author: alda
 */

#ifndef BSTREE_H_
#define BSTREE_H_

// compare x=a:b (x=0:a match b, x<0:a left of b, x>0:a right of b)
typedef int (bstc)(void*, void*);

// balanced search tree node
typedef struct t_bstn {
    struct t_bstn*  left;
    struct t_bstn*  right;
    void*           elem;
    int             depth;
} t_bstn;


void* bst_add(t_bstn** root, void* e, bstc* f);
void* bst_find(t_bstn* root, void* e, bstc* f);


#endif /* BSTREE_H_ */
