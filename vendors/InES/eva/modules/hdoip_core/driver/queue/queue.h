/*
 * queue.h
 *
 *  Created on: 17.09.2010
 *      Author: alda
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include <linux/types.h>

typedef struct {
	uint32_t        write;
	uint32_t        read;
	uint32_t        count;
	uint32_t        buf[];
} t_queue;

t_queue* queue_init(size_t size);
void queue_put(t_queue* queue, uint32_t elem);
uint32_t queue_get(t_queue* queue);
bool queue_empty(t_queue* queue);
void queue_flush(t_queue* queue);



#endif /* QUEUE_H_ */
