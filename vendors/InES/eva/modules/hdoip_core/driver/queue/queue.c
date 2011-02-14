/*
 * queue.c
 *
 *  Created on: 17.09.2010
 *      Author: alda
 */
#include "std.h"
#include "queue.h"

t_queue* queue_init(uint32_t count)
{
	t_queue* queue = kzalloc(sizeof(t_queue) + count * sizeof(uint32_t), GFP_KERNEL);

	if (!queue) {
	    REPORT(ERROR, "event queue(%d) init failed", count);
	    return 0;
	}

	queue->count = count;
	queue->read = 0;
	queue->write = 0;

	return queue;
}

void queue_put(t_queue* queue, uint32_t elem)
{
    uint32_t write = queue->write + 1;

    if (!elem) return;
    if (!queue) return;

    if (write >= queue->count) {
        write = 0;
    }

    if (write != queue->read) {
        queue->buf[queue->write] = elem;
        queue->write = write;
    } else {
        REPORT(ERROR, "queue has not enough space for <%08x>", elem);
    }
}


uint32_t queue_get(t_queue* queue)
{
    uint32_t elem = 0;
    uint32_t read = queue->read;

    if (!queue) return 0;

    if (read != queue->write) {
        elem = queue->buf[read];
        read = read + 1;
        if (read >= queue->count) {
            read = 0;
        }
        queue->read = read;
    }

    return elem;
}

bool queue_empty(t_queue* queue)
{
    if (!queue) return true;
    return (queue->write == queue->read);
}

void queue_flush(t_queue* queue)
{
    queue->read = queue->write;
}
