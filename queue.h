#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "common.h"
#include "datatypes/object.h"

//priority queue implementation

typedef struct _Element {
    MyMoObject *object;
    int priority;
} Element;

typedef struct _PriorityQueue {
    Element *elements;
    int size;
    int capacity;
} MyMOPriorityQueue;

MyMOPriorityQueue *newPQ();
void enqueuePQ(MVM *vm, MyMOPriorityQueue *queue, MyMoObject *object, int priority);
MyMoObject *dequeuePQ(MyMOPriorityQueue *queue);
void swap(Element *a, Element *b);
void heapify(Element *elements, int size, int i);
void heapSort(Element *elements, int size);
void printPQ(MyMOPriorityQueue *queue);
void freePQ(MVM *vm, MyMOPriorityQueue *queue);
int sizePQ(MyMOPriorityQueue *queue);
int isEmptyPQ(MyMOPriorityQueue *queue);

#endif