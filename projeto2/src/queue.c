#include "queue.h"

reply_queue create_queue(int capacity){
    reply_queue queue;
    queue.queue = malloc((sizeof(ftp_reply))*capacity);
    queue.front = 0;
    queue.back = -1;
    queue.current_size = 0;
    queue.capacity = capacity;
    return queue;
}

void destroy_queue(reply_queue *queue){
    free(queue->queue);
}

int enqueue(reply_queue *queue, ftp_reply value){
    if(!is_full(queue)){
        queue->back = (queue->back+1) % queue->capacity;
        queue->queue[queue->back] = value;
        queue->current_size++;
        return 0;
    }    
    return -1;
}

ftp_reply dequeue(reply_queue *queue){
    if(queue->current_size > 0){
        ftp_reply value = queue->queue[queue->front];
        queue->front = (queue->front + 1) % queue->capacity;
        queue->current_size--;
        return value;
    }
    ftp_reply invalid_reply = {-1, NULL};
    return invalid_reply;
}

int is_full(reply_queue *queue) {
    return (queue->capacity == queue->current_size);
}

int is_empty(reply_queue *queue){
    return queue->current_size == 0;
}