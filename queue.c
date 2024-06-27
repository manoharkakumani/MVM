#include "queue.h"
#include "memory.h"
#include "datatypes/nil.h"


MyMOPriorityQueue *newPQ()
{
    MyMOPriorityQueue *queue = New(MyMOPriorityQueue, 1);
    queue->size = 0;
    queue->capacity = 8;
    return queue;
}

void enqueuePQ(MVM *vm, MyMOPriorityQueue *queue, MyMoObject *object, int priority)
{
    if (queue->size+1 > queue->capacity)
    {
        int oldCapacity = queue->capacity;
        queue->capacity = ResizeCapacity(queue->capacity);
        queue->elements = ResizeArray(vm, Element, queue->elements, oldCapacity, queue->capacity);
    }
    Element element = {object, priority};
    queue->elements[queue->size] = element;
    queue->size++;
    heapSort(queue->elements, queue->size);
}

MyMoObject *dequeuePQ(MyMOPriorityQueue *queue)
{
    if (queue->size == 0)
    {
        return NEW_EMPTY;
    }
    int min = 0;
    for (int i = 0; i < queue->size; i++)
    {
        if (queue->elements[i].priority < queue->elements[min].priority)
        {
            min = i;
        }
    }
    MyMoObject *object = queue->elements[min].object;
    queue->elements[min] = queue->elements[queue->size-1];
    queue->size--;
    return object;
}

void swap(Element *a, Element *b)
{
    Element temp = *a;
    *a = *b;
    *b = temp;
}

void heapify(Element *elements, int size, int i)
{
    int smallest = i;
    int left = 2*i + 1;
    int right = 2*i + 2;
    if (left < size && elements[left].priority < elements[smallest].priority)
    {
        smallest = left;
    }
    if (right < size && elements[right].priority < elements[smallest].priority)
    {
        smallest = right;
    }
    if (smallest != i)
    {
        swap(&elements[i], &elements[smallest]);
        heapify(elements, size, smallest);
    }
}

void heapSort(Element *elements, int size)
{
    for (int i = size/2 - 1; i >= 0; i--)
    {
        heapify(elements, size, i);
    }
    for (int i = size-1; i >= 0; i--)
    {
        swap(&elements[0], &elements[i]);
        heapify(elements, i, 0);
    }
}

void printPQ(MyMOPriorityQueue *queue)
{
    for (int i = 0; i < queue->size; i++)
    {
        printf("%d ", queue->elements[i].priority);
    }
    printf("\n");
}

void freePQ(MVM *vm, MyMOPriorityQueue *queue)
{
    FreeArray(vm, Element, queue->elements, queue->capacity);
    Free(vm, MyMOPriorityQueue, queue);
}

int sizePQ(MyMOPriorityQueue *queue)
{
    return queue->size;
}

