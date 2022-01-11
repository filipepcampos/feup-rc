#include <stdlib.h>

typedef struct{
    int code;
    char *message;
} ftp_reply;

typedef struct{
    ftp_reply *queue;
    int capacity;
    int current_size;
    size_t front;
    size_t back;
} queue;


queue create_queue(int capacity);

void destroy_queue(queue *queue);

int enqueue(queue *queue, ftp_reply value);

ftp_reply dequeue(queue *queue);

int is_full(queue *queue);

int is_empty(queue *queue);