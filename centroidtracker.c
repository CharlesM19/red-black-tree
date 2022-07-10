/*
Created by pratheek on 2019-11-27.
*/
#include "centroidtracker.h"

double calcDistance(Point p1, Point p2) {
    double x = p1.row - p2.row;
    double y = p1.col - p2.col;
    double dist = sqrt((x * x) + (y * y));       //calculating Euclidean distance

    return dist;
}

int distcmp(const void *a, const void *b) {
    Object p1 = *(const Object*)a;
    Object p2 = *(const Object*)b;

    return (int) ((p1.distance > p2.distance) - 
        (p1.distance < p2.distance));
}

void swapRow(void *src, int a, int b, int ncol) {
    Object temp;
    // swap each elemnt in a row
    for (int i=0; i<ncol; i++) {
        temp = ((Object*)src)[a*ncol+i];
        ((Object*)src)[a*ncol+i] = ((Object*)src)[b*ncol+i];
        ((Object*)src)[b*ncol+i] = temp;
    }
}

void swapCol(void *src, int a, int b, int nrow, int ncol) {
    Object temp;
    // swap same column for each row
    for (int i=0; i<nrow; i++) {
        temp = ((Object*)src)[i*ncol+a];
        ((Object*)src)[i*ncol+a] = ((Object*)src)[i*ncol+b];
        ((Object*)src)[i*ncol+b] = temp;
    }
}

int findMinRow(const void* src, int nrow, int ncol) {
    // objects
    Object* objects = (Object*)src;

    // length
    int length = nrow * ncol;

    // distance compare
    double minDist = objects[0].distance;
    int minIdx = 0;

    // loop through distances and find min
    for (int i=1; i<length; i++) {
        if (objects[i].distance < minDist) {
            minDist = objects[i].distance;
            minIdx = i;
        }
    }
    // divide to get row number
    return (minIdx / ncol);
}

void Bubble(void *src, int length) {
    Object temp = *(Object*)(src);
    for (int i=0; i<length-1; i++) {
        ((Object*)src)[i] = ((Object*)src)[i+1];
        ((Object*)src)[i+1] = temp;
    }
}

void CentroidTrackerInit(tracker *t, int maxDisappeared) {
    // initialize variables and parameters
    t->maxDisappeared = maxDisappeared;
    t->nextObjectID = 0;
    t->numReg = 0;

    // allocated centroids and disappear map
    t->objects = malloc(maxDisappeared*sizeof(Object));
    t->disappeared = malloc(maxDisappeared*sizeof(int));

    // set disappeared map to -1 as unused marker
    for (int i=0; i<maxDisappeared; i++) {
        t->disappeared[i] = -1;
    }

    // set distance function
    t->calcDistance = calcDistance;
}

void __register(tracker *t, Point centroid) {
    // get new object id
    int object_ID = t->nextObjectID;

    // add to object buffer
    t->objects[t->nextObjectID] = (Object){object_ID, 0, centroid};

    // add object to disappeared map
    t->disappeared[t->nextObjectID++] = 0;

    // increment number of registered objects
    t->numReg++;
}

void __deregister(tracker *t, int id) {
    // mark as unused id
    t->disappeared[id] = -1;

    // decrement number of registered objects
    t->numReg--;

    return;
}

void update(tracker *t, Point* centroids, int numC) {
    // grid comparison for matching [numC x numReg]
    Object* map = calloc(numC*t->numReg, sizeof(Object));
    int *indices = malloc(t->numReg*sizeof(int));

    // iterator for existing centroids
    int k = 0;
    int matched = 0;
    int c = numC;

    // distances for compare
    double currDist = 0;

    // temporary object
    Object temp;

    // check for null pointer to centroid
    if (centroids == NULL) {
        for (int i=0; i<t->maxDisappeared; i++) {
            t->disappeared[i] += (int)(t->disappeared[i] == 0);

            if (t->disappeared[i] > t->maxDisappeared) {
                __deregister(t, i);
            }
        }
    }

    // check for any current objects
    if (t->numReg == 0) {
        for (int i=0; i<numC; i++) {
            __register(t, centroids[i]);
        }
        return;
    } 

    // else find distance between old and new points to match (NM)
    for (int i=0; i<t->maxDisappeared; i++) {
        if (t->disappeared[i] != -1) {
            // check current registered object to each given centroid
            for (int j=0; j<numC; j++) {
                // update distance grid
                currDist = t->calcDistance(t->objects[i].pt, centroids[j]);
                map[k*numC+j] = (Object) {j, currDist, centroids[j]};
                indices[k] = i;
            }
            k++; // increment existing centroid index
        }
    }

    // argsort by closest new centroid to old centroid (NMlogN)
    for (int i=0; i<t->numReg; i++) {
        qsort(&map[i*numC], numC, sizeof(Object), distcmp);
    }

    for (k=t->numReg-1, c=numC; k>-1 && c>0; k--, c--) {
        // find row with smallest distance
        int row = findMinRow(map, k+1, c);

        // swap smallest distance row with last row
        swapRow(t->objects, row, k, c);

        // first element should closest for current row
        // so set id there
        matched = map[k*numC].id;
        map[k*numC].id = indices[row];

        // add object to list of objects
        t->objects[indices[row]] = map[k*numC];

        // for each remaining row remove the already matched
        // object in this iteration
        for (int i=0; i<k; i++) {
            // swap the matched id with the last/farthest id
            for (int j=0; j<c; j++) {
                if (map[i*numC+j].id == matched) {
                    temp = map[i*numC+j];
                    map[i*numC+j] = map[i*numC+c-1];
                    map[i*numC+c-1] = temp;
                    break;
                }
            }
            // sort the remaining columns
            qsort(&map[i*numC], c-1, sizeof(Object), distcmp);
            // Bubble(&map[i*numC], c-1);
        }
    }
    
    // check if we have leftover old centroid(s)
    for (; k>-1; k--) {
        c = t->objects[indices[k]].id;
        // deregister excess object(s)
        if (t->disappeared[c]++ > t->maxDisappeared) {
            __deregister(t, c);
        }
    }

    // check if we have extra new centroid(s)
    for (; c>0; c--) {
        // register excess new centroid(s)
        __register(t, map[c].pt);
    }
}

int main() {
    tracker t;
    Point pt[4] = {{0, 0}, {1,1}, {3,3}, {10,10}};
    Point pt2[5] = {{7,7}, {4,4}, {8,8}, {5,5}, {6,6}};

    // result should be {(0,0)->(6,6),(1,1)->(5,5),(3,3)->(4,4)}

    CentroidTrackerInit(&t, 10);

    update(&t, pt, 4);
    
    for (int i=0; i<t.maxDisappeared; i++) {
        if (t.disappeared[i] != -1) {
            printf("(%d,%d)\n", t.objects[i].pt.row, t.objects[i].pt.col);
        }
    }
    printf("\n");

    update(&t, pt2, 5);

    for (int i=0; i<t.maxDisappeared; i++) {
        if (t.disappeared[i] != -1) {
            printf("(%d,%d)\n", t.objects[i].pt.row, t.objects[i].pt.col);
        }
    }

    return 0;
}