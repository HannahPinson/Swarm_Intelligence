/*
 * fsp.h
 *
 *  Created on: 09 Aug 2016
 *      Author: Hannah_Pinson
 */

#ifndef FSP_H_
#define FSP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>


class FSP{
	FILE *fsp_file;
	long int **times;	//times[i][j] gives the time it takes machine i to perform task j
	long int n; //number of independent tasks
	long int m; //number of machines

public:
	//FSP (const char *fsp_file_name);
	FSP (std::string fsp_file_name);
	~FSP();
	long int getNumberOfTasks(){return n;};
	long int getNumberOfMachines(){return m;};
	long int getTime(long int machine, long int task){return (times[machine])[task];};
};


#endif /* FSP_H_ */
