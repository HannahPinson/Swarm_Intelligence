/*
 * fsp.cpp
 *
 *  Created on: 09 Aug 2016
 *      Author: Hannah_Pinson
 */


#include "fsp.h"

using namespace std;

FSP::FSP(string filename){
	ifstream infile(filename);
	string line;
	getline(infile, line);
	istringstream iss(line);
	iss >> n >> m;
	times = new long int * [m];

	int index = 0;

	while (std::getline(infile, line))
	{
		times[index] = new long int [n];
		istringstream iss(line);
		int index2 = 0;
		while(iss){
			iss >> (times[index])[index2];
			index2++;
		}
		index++;
	}

}


FSP::~FSP(){
	for(int i=0; i < n; i++){
		delete[] times[i];
	}
	delete[] times;

	/*free(times);
	times = NULL;*/
}


long int FSP::getDistance(long int task1, long int task2){
	long int distance = 0;
	distance += getTime(1, task1);
	for (int j = 1; j < m; j++){
		distance += (m-(j+1)) * abs(getTime(j, task1) - getTime(j-1, task2));
	}
	distance += getTime(m-1, task2);
	return distance;
}
