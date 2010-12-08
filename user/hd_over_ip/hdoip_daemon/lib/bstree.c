/*
 * bstree.c
 *
 * balanced search tree
 *
 *  Created on: 02.12.2010
 *      Author: alda
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bstree.h"

static inline void bst_depth(t_bstn* root)
{
    int a, b;

    if (root->left) a = root->left->depth + 1; else a = 0;
    if (root->right) b = root->right->depth + 1; else b = 0;
    if (a > b) root->depth = a; else root->depth = b;
}

static inline int bst_diff(t_bstn* root)
{
    int a, b;

    if (root->left) a = root->left->depth + 1; else a = 0;
    if (root->right) b = root->right->depth + 1; else b = 0;

    return a - b;
}

void bst_rotate_right(t_bstn** root)
{
    if (!(*root) || !(*root)->left) return;
    t_bstn* tmp = (*root)->left;
    (*root)->left = tmp->right;
    tmp->right = *root;
    *root = tmp;
    bst_depth(tmp->right);
}

void bst_rotate_left(t_bstn** root)
{
    if (!(*root) || !(*root)->right) return;
    t_bstn* tmp = (*root)->right;
    (*root)->right = tmp->left;
    tmp->left = *root;
    *root = tmp;
    bst_depth(tmp->left);
}

void* bst_add(t_bstn** root, void* e, bstc* f)
{
    void* ret;

    if (!f) return 0;

    if (!(*root)) {
        // new bstn
        *root = malloc(sizeof(t_bstn));
        if (*root) {
            (*root)->depth = 0;
            (*root)->left = 0;
            (*root)->right = 0;
            (*root)->elem = e;
        } else {
            printf("bst_add.malloc: out of memory");
        }
        return 0;
    }

    int c = f(e, (*root)->elem);

    if (c == 0) {
        void* tmp = (*root)->elem;
        (*root)->elem = e;
        return tmp;
    } else if (c < 0) {
        // e left of *root->elem
        ret = bst_add(&(*root)->left, e, f);
        if (!ret) {
            if (bst_diff(*root) > 1 ) {
                if (bst_diff((*root)->right) >= 0) {
                    bst_rotate_right(root);
                } else {
                    bst_rotate_left(&(*root)->left);
                    bst_depth((*root)->left);
                    bst_rotate_right(root);
                }
            }
        }
    } else {
        // e right of *root->elem
        ret = bst_add(&(*root)->right, e, f);
        if (!ret) {
            if (bst_diff(*root) < -1 ) {
                if (bst_diff((*root)->left) <= 0) {
                    bst_rotate_left(root);
                } else {
                    bst_rotate_right(&(*root)->right);
                    bst_depth((*root)->right);
                    bst_rotate_left(root);
                }
            }
        }
    }

    bst_depth(*root);
    return ret;
}

void* bst_find(t_bstn* root, void* e, bstc* f)
{
    if (!f) return 0;
    if (!root) return 0;

    int c = f(e, root->elem);

    if (c == 0) {
        return root->elem;
    } else if (c < 0) {
        // e left of *root->elem
        return bst_find(root->left, e, f);
    } else {
        // e right of *root->elem
        return bst_find(root->right, e, f);
    }
}

