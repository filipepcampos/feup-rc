#include "queue.h"

queue create_queue(int capacity){
    queue queue;
    queue.queue = malloc((sizeof(ftp_reply))*capacity);
    queue.front = 0;
    queue.back = -1;
    queue.current_size = 0;
    queue.capacity = capacity;
    return queue;
}

void destroy_queue(queue *queue){
    free(queue->queue);
}

#include<stdio.h>//TODO: Remove
int enqueue(queue *queue, ftp_reply value){
    if(!is_full(queue)){
        queue->back = (queue->back+1) % queue->capacity;
        queue->queue[queue->back] = value;
        queue->current_size++;
        return 0;
    }    
    return -1;
}

ftp_reply dequeue(queue *queue){
    if(queue->current_size > 0){
        ftp_reply value = queue->queue[queue->front];
        queue->front = (queue->front + 1) % queue->capacity;
        queue->current_size--;
        return value;
    }
    ftp_reply invalid_reply = {-1, NULL};
    return invalid_reply;
}

int is_full(queue *queue) {
    return (queue->capacity == queue->current_size);
}

int is_empty(queue *queue){
    return queue->current_size == 0;
}