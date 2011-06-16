/*
 * linked_list.c
 *
 * circular doubly linked lists (with anchor node).
 * not thread safe.
 *
 * anchor node has no element(nullpointer)
 *
 *  Created on: 10.11.2010
 *      Author: alda
 */

#include <stdlib.h>
#include "linked_list.h"
#include "hdoipd.h"

t_node* node_create(void* elem)
{
    t_node* node = malloc(sizeof(t_node));
    if (node) {
        node->next = node;
        node->prev = node;
        node->elem = elem;
    } else {
        report("node_create.malloc: out of memory");
    }
    return node;
}


void* node_free(t_node* node)
{
    if (node) {
        void* elem = node->elem;
        free(node);
        return elem;
    }
    return 0;
}

void* node_destroy(t_node* node)
{
    return node_free(node_remove(node));
}


t_node* node_insert_next(t_node* a, t_node* b)
{
    if (a && b) {
        a->next->prev = b->prev;
        b->prev->next = a->next;
        a->next = b;
        b->prev = a;
        return b;
    }
    return 0;
}


t_node* node_insert_prev(t_node* a, t_node* b)
{
    if (a && b) {
        b->next->prev = a->prev;
        a->prev->next = b->next;
        b->next = a;
        a->prev = b;
        return b;
    }
    return 0;
}


t_node* node_remove(t_node* node)
{
    if (node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        node->prev = node;
        node->next = node;
    }
    return node;
}


t_node* node_remove_next(t_node* node)
{
    return node_remove(node->next);
}


t_node* node_remove_prev(t_node* node)
{
    return node_remove(node->prev);
}


bool node_others(t_node* node)
{
    if (node) {
        if (node->next != node) {
            return true;
        }
    }
    return false;
}


t_node* node_find_next(t_node* list, void* elem)
{
    t_node* node = list;
    if (list) {
        do {
            if (node->elem == elem) return node;
            node = node->next;
        } while(node != list);
    }
    return 0;
}


t_node* node_find_prev(t_node* list, void* elem)
{
    t_node* node = list;
    if (list) {
        do {
            if (node->elem == elem) return node;
            node = node->prev;
        } while(node != list);
    }
    return 0;
}


t_node* node_traverse_next(t_node* list, f_node_callback* callback, void* data)
{
    t_node* node = list, *next;
    if (list && callback) {
        do {
            next = node->next;
            if (node->elem) {
                callback(node->elem, data);
            }
            node = next;
        } while(node != list);
    }
    return list;
}


t_node* node_traverse_prev(t_node* list, f_node_callback* callback, void* data)
{
    t_node* node = list, *prev;
    if (list && callback) {
        do {
            prev = node->prev;
            if (node->elem) {
                callback(node->elem, data);
            }
            node = prev;
        } while(node != list);
    }
    return list;
}


t_node* node_traverse_find_next(t_node* list, f_node_compare* callback, void* data)
{
    t_node* node = list;
    if (list && callback) {
        do {
            if (node->elem) {
                if (callback(node->elem, data) == 0) {
                    return node;
                }
            }
            node = node->next;
        } while(node != list);
    }
    return 0;
}


t_node* node_traverse_find_prev(t_node* list, f_node_compare* callback, void* data)
{
    t_node* node = list;
    if (list && callback) {
        do {
            if (node->elem) {
                if (callback(node->elem, data) == 0) {
                    return node;
                }
            }
            node = node->prev;
        } while(node != list);
    }
    return 0;
}


t_node* node_traverse_destroy_next(t_node* list, f_node_callback* callback, void* data)
{
    t_node *node = list, *next = 0;
    while (node_others(node)) {
        next = node->next;
        if (node->elem) {
            if (callback) {
                callback(node_destroy(node), data);
            }
        }
        node = next;
    }
    if (node->elem) {
        if (callback) {
            callback(node_free(node), data);
        }
        return 0;
    }
    return node;
}


t_node* node_traverse_destroy_prev(t_node* list, f_node_callback* callback, void* data)
{
    t_node *node = list, *next = 0;
    while (node_others(node)) {
        next = node->prev;
        if (node->elem) {
            if (callback) {
                callback(node_destroy(node), data);
            }
        }
        node = next;
    }
    if (node->elem) {
        if (callback) {
            callback(node_free(node), data);
        }
        return 0;
    }
    return node;
}


//------------------------------------------------------------------------------
// Queue (with anchor element)

t_node* queue_create()
{
    return node_create(0);
}

t_node* queue_put(t_node* anchor, void* elem)
{
    t_node* node = node_create(elem);
    return node_insert_next(anchor, node);
}

void* queue_get(t_node* anchor)
{
    if (node_others(anchor)) {
        return node_free(node_remove_prev(anchor));
    }
    return 0;
}


//------------------------------------------------------------------------------
// Stack (with anchor element)

t_node* stack_create()
{
    return node_create(0);
}

t_node* stack_push(t_node* anchor, void* elem)
{
    t_node* node = node_create(elem);
    return node_insert_next(anchor, node);
}

void* stack_pop(t_node* anchor)
{
    if (node_others(anchor)) {
        return node_free(node_remove_next(anchor));
    }
    return 0;
}


//------------------------------------------------------------------------------
// List (with anchor element)

t_node* list_create()
{
    return node_create(0);
}

t_node* list_copy(t_node* old)
{
    t_node* list = node_create(0);
    list_add_all(list, old);
    return list;
}

t_node* list_add(t_node* anchor, void* elem)
{
    t_node* node = node_create(elem);
    return node_insert_prev(anchor, node);
}

t_node* list_add_all(t_node* anchor, t_node* list)
{
    t_node* node = list;
    if (list) {
        do {
            if (node->elem) {
                t_node* new = node_create(node->elem);
                node_insert_prev(anchor, new);
            }
            node = node->next;
        } while(node != list);
    }
    return list;
}

t_node* list_free(t_node* anchor)
{
    return node_free(node_traverse_destroy_next(anchor, 0, 0));
}

t_node* list_traverse_free(t_node* anchor, f_node_callback* callback, void* data)
{
    return node_free(node_traverse_destroy_next(anchor, callback, data));
}

t_node* list_traverse(t_node* anchor, f_node_callback* callback, void* data)
{
    return node_traverse_next(anchor, callback, data);
}

void* list_find(t_node* list, f_node_compare* callback, void* data)
{
    t_node* node = node_traverse_find_next(list, callback, data);
    if (node) return node->elem;
    return 0;
}

void* list_remove(t_node* node)
{
    return node_destroy(node);
}

void* list_peek(t_node* anchor)
{
    void* elem = 0;
    if (anchor && (anchor->next != anchor)) {
        elem = anchor->next->elem;
    }
    return elem;
}

bool list_contains(t_node* anchor, void* elem)
{
    return node_find_next(anchor, elem);
}
