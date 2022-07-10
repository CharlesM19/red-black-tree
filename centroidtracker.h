/*
Created by pratheek on 2019-11-27.
*/

#ifndef C___CENTROIDTRACKER_H
#define C___CENTROIDTRACKER_H

#include <math.h>
// #include "rb.h"
#include "point.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct Object {
    int id;
    double distance;
    Point pt;
} Object;

typedef struct tracker {
    int maxDisappeared; // number of possible disappeared objects
    int nextObjectID; // ID index
    int numReg; // number of currently tracked objects
    int* disappeared; // table of disappeared objects
    Object* objects; // centroids
    double (*calcDistance) (const Point p1, const Point p2); // distance calculation function
} tracker;

void CentroidTrackerInit(tracker *t, int maxDisappeared);

void __register(tracker *t, Point centroid);

void update(tracker *t, Point* centroids, int numC);

#endif //C___CENTROIDTRACKER_H
