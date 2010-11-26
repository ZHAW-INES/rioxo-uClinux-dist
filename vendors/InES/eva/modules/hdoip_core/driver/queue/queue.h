/*
 * queue.h
 *
 *  Created on: 17.09.2010
 *      Author: alda
 */

#ifndef QUEUE_H_
#define QUEUE_H_

typedef struct {
	uint32_t        write;
	uint32_t        read;
	uint32_t        count;
	uint32_t        buf[];
} t_queue;

typedef struct {
    uint32_t        id;
    void*           msg;
} t_queue_element;

typedef struct {
    uint32_t        write;
    uint32_t        read;
    uint32_t        count;
    t_queue_element buf[];
} t_msg_queue;


t_queue* queue_init(size_t size);
void queue_put(t_queue* queue, uint32_t elem);
uint32_t queue_get(t_queue* queue);
void queue_flush(t_queue* queue);

t_msg_queue* msg_queue_init(size_t size);
void msg_queue_put(t_msg_queue* queue, t_queue_element elem);
t_queue_element msg_queue_get(t_msg_queue* queue);


#endif /* QUEUE_H_ */
