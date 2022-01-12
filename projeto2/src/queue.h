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
} reply_queue;


reply_queue create_queue(int capacity);

void destroy_queue(reply_queue *queue);

int enqueue(reply_queue *queue, ftp_reply value);

ftp_reply dequeue(reply_queue *queue);

int is_full(reply_queue *queue);

int is_empty(reply_queue *queue);