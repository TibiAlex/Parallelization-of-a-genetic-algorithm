#ifndef THREAD_PARAMETERS_H
#define THREAD_PARAMETERS_H

// structure that contains the parameters of each thread
typedef struct _threadparam {
	int start, end;
    long id;
    int object_count;
    int sack_capacity;
    int generations_count;
    int cores;
    sack_object *objects;
    individual *current_generation;
	individual *next_generation;
    pthread_barrier_t *barrier;
} threadparam;

#endif