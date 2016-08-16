/*
 * ant.h
 *
 *  Created on: 08 Aug 2016
 *      Author: Hannah_Pinson
 */

#ifndef ANT_H_
#define ANT_H_



#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <vector>

#include "fsp.h"
#include "utils.h"

using namespace std;

class Ant{
    bool init;
    long int* sequence;           /* solution */
    long int* scheduled;        /* auxiliary array for construction to keep track of scheduled tasks */
    long int makespan;

    double ** probability;    /*pointer to the pheromone (+heuristic) of the colony!*/
    double ** pheromone;
    FSP * fsp;
    long int size;
    long int machines;
    long int tasks;

    void ComputeMakespan(); //to implement
    double computeEFT(long int task, long int position, long int machine, double ** earliest_finish_times);

    public:
       Ant();
       Ant(FSP* fsp_arg, double ** prob_info, double ** phero);
       ~Ant();
       Ant(Ant const &other);

       void Search(); //to implement
       long int getMakespan();
       long int * getSequence(){return sequence;};

       bool alreadyScheduled(long int task, long int currentIndex);
       void depositPheromones();
       void copyFrom(Ant &other);
};


#endif /* ANT_H_ */
