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
#include "debug.h"

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

void bst_balance(t_bstn** root)
{
    int b;
    if (!(*root) || !root) return;
    b = bst_diff(*root);
    if (b > 1 ) { // left side is deeper
        // balance to the right
        if (bst_diff((*root)->right) >= 0) {
            bst_rotate_right(root);
        } else {
            bst_rotate_left(&(*root)->left);
            bst_depth((*root)->left);
            bst_rotate_right(root);
        }
    } else if (b < -1) { // right side is deeper
        // balance to the left
        if (bst_diff((*root)->left) <= 0) {
            bst_rotate_left(root);
        } else {
            bst_rotate_right(&(*root)->right);
            bst_depth((*root)->right);
            bst_rotate_left(root);
        }
    }
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
            report("bst_add.malloc: out of memory");
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
        bst_balance(root);
    } else {
        // e right of *root->elem
        ret = bst_add(&(*root)->right, e, f);
        bst_balance(root);
    }

    bst_depth(*root);
    return ret;
}

void* bst_remove_left(t_bstn** root)
{
    void* ret = 0;

    if ((*root)->left) {
        ret = bst_remove_left(&(*root)->left);
        bst_balance(root);
    } else {
        ret = (*root)->elem;
        t_bstn* tmp = (*root)->right;
        free(*root);
        *root = tmp;
    }

    return ret;
}

void* bst_remove_right(t_bstn** root)
{
    void* ret = 0;

    if ((*root)->right) {
        ret = bst_remove_right(&(*root)->right);
        bst_balance(root);
    } else {
        ret = (*root)->elem;
        t_bstn* tmp = (*root)->left;
        free(*root);
        *root = tmp;
    }

    return ret;
}

void* bst_remove(t_bstn** root, void* e, bstc* f)
{
    void* ret;

    if (!f) return 0;

    if (!(*root)) {
        return 0;
    }

    int c = f(e, (*root)->elem);

    if (c == 0) {
        // remove this node
        ret = (*root)->elem;

        if (!(*root)->left && !(*root)->right) {
            // no children
            free((*root));
            *root = 0;
        } else if (!(*root)->right) {
            // left child only
            t_bstn* tmp = (*root)->left;
            free((*root));
            *root = tmp;
        } else if (!(*root)->left) {
            // right child only
            t_bstn* tmp = (*root)->right;
            free((*root));
            *root = tmp;
        } else {
            // both children
            if (bst_diff(*root) > 0) { // left side deeper
                (*root)->elem = bst_remove_left(&(*root)->right);
            } else { // right side deeper
                (*root)->elem = bst_remove_right(&(*root)->left);
            }
            bst_balance(root);
        }

    } else if (c > 0) {
        // remove it from the right tree
        ret = bst_remove(&(*root)->right, e, f);
        bst_balance(root);
    } else {
        // e right of *root->elem
        ret = bst_remove(&(*root)->left, e, f);
        bst_balance(root);
    }

    bst_depth(*root);
    return ret;
}

void* bst_delete(t_bstn** root)
{
    void* ret;

    if (!root || !(*root)) {
        return 0;
    }

    // remove this node
    ret = (*root)->elem;

    if (!(*root)->left && !(*root)->right) {
        // no children
        free((*root));
        *root = 0;
    } else if (!(*root)->right) {
        // left child only
        t_bstn* tmp = (*root)->left;
        free((*root));
        *root = tmp;
    } else if (!(*root)->left) {
        // right child only
        t_bstn* tmp = (*root)->right;
        free((*root));
        *root = tmp;
    } else {
        // both children
        if (bst_diff(*root) > 0) { // left side deeper
            (*root)->elem = bst_remove_left(&(*root)->right);
        } else { // right side deeper
            (*root)->elem = bst_remove_right(&(*root)->left);
        }
        bst_balance(root);
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

void bst_traverse(t_bstn* root, bstt* f, void* d)
{
    if (!root) return;

    if (root->left) bst_traverse(root->left, f, d);
    f(root->elem, d);
    if (root->right) bst_traverse(root->right, f, d);
}

void bst_free(t_bstn** root, bstc* f, void* d)
{
    int b;
    if (!root || !(*root)) return;
    if ((*root)->left) bst_free(&(*root)->left, f, d);
    b = f((*root)->elem, d);
    if ((*root)->right) bst_free(&(*root)->right, f, d);
    if (b) bst_delete(root);
    bst_balance(root);
}

