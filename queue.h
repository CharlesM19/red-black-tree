#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "point.h"

//======================Point=========================================//

#define ITEM Point

extern const Point p; 

typedef struct Queue {
    int front, rear, size;
    unsigned capacity;
    ITEM* array;
} Queue;
  
// function to create a queue
// of given capacity.
// It initializes size of queue as 0
Queue* createQueue(unsigned capacity);
  
// Queue is full when size becomes
// equal to the capacity
int isFull(Queue* queue);
  
// Queue is empty when size is 0
int isEmpty(Queue* queue);
  
// Function to add an item to the queue.
// It changes rear and size
void enqueue(Queue* queue, ITEM item);
  
// Function to remove an item from queue.
// It changes front and size
ITEM dequeue(Queue* queue);
  
// Function to get front of queue
ITEM front(Queue* queue);
  
// Function to get rear of queue
ITEM rear(Queue* queue);
  
#endif