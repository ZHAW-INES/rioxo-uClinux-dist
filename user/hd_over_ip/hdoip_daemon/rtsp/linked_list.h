/*
 * linked_list.h
 *
 *  Created on: 10.11.2010
 *      Author: alda
 */

#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct t_node {
    struct t_node* next;        //!< next element in list
    struct t_node* prev;        //!< previous element in list
    void* elem;                 //!< content of this node
} t_node;

#define node_anchor(x) t_node x = { .next = &x, .prev = &x, .elem = 0 }

typedef void (f_node_callback)(void* elem, void* data);
typedef int (f_node_compare)(void* e1, void* e2);

t_node* node_create(void* elem);
void* node_free(t_node* node);
void* node_destroy(t_node* node);
t_node* node_insert_next(t_node* a, t_node* b);
t_node* node_insert_prev(t_node* a, t_node* b);
t_node* node_remove(t_node* node);
t_node* node_remove_next(t_node* node);
t_node* node_remove_prev(t_node* node);
bool node_others(t_node* node);
t_node* node_find_next(t_node* list, void* elem);
t_node* node_find_prev(t_node* list, void* elem);
t_node* node_traverse_next(t_node* list, f_node_callback* callback, void* data);
t_node* node_traverse_prev(t_node* list, f_node_callback* callback, void* data);
t_node* node_traverse_find_next(t_node* list, f_node_compare* callback, void* data);
t_node* node_traverse_find_prev(t_node* list, f_node_compare* callback, void* data);
t_node* node_traverse_destroy_next(t_node* list, f_node_callback* callback, void* data);
t_node* node_traverse_destroy_prev(t_node* list, f_node_callback* callback, void* data);


t_node* queue_create();
t_node* queue_put(t_node* anchor, void* elem);
void* queue_get(t_node* anchor);

t_node* stack_create();
t_node* stack_push(t_node* anchor, void* elem);
void* stack_pop(t_node* anchor);

t_node* list_create();
t_node* list_add(t_node* anchor, void* elem);
void* list_find(t_node* list, f_node_compare* callback, void* data);
t_node* list_free(t_node* anchor);
t_node* list_traverse_free(t_node* anchor, f_node_callback* callback, void* data);
t_node* list_traverse(t_node* anchor, f_node_callback* callback, void* data);
void* list_remove(t_node* node);

#endif /* LINKED_LIST_H_ */
