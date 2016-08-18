/*
 * NEH.h
 *
 *  Created on: 16 Aug 2016
 *      Author: Hannah_Pinson
 */

#ifndef NEH_H_
#define NEH_H_

#include <vector>
#include "fsp.h"


using namespace std;



struct task_processingTime{
	double task;
	double procTime;
};

struct sequence_makespan{
	vector <long int>  sequence;
	double makespan;
};

vector<task_processingTime> sumProcessingTimes(FSP* fsp){
	vector<task_processingTime> tasks_procTimes;
	for(int i = 0; i < fsp->getNumberOfTasks(); i++){
		double sum = 0;
		for (int m = 0; m< fsp->getNumberOfMachines(); m++){
			sum += fsp->getTime(m, i);
		}
		task_processingTime tp;
		tp.task = i;
		tp.procTime = sum;
		tasks_procTimes.push_back(tp);
	}
	return tasks_procTimes;
}


vector<long int> sortTasks(FSP* fsp){
	vector<task_processingTime>::iterator it;
	vector<task_processingTime> unsorted_tasks_pt = sumProcessingTimes(fsp);
	vector<task_processingTime> sorted_tasks_pt;

	// insertion sort based on total processing time
	// non-increasing order
	sorted_tasks_pt.push_back(unsorted_tasks_pt[0]);
	for (int t = 1; t < fsp->getNumberOfTasks(); t++){
		it = sorted_tasks_pt.begin();
		bool inserted = false;
		for (int i = 0; i < t; i++){
			if (unsorted_tasks_pt[t].procTime >= sorted_tasks_pt[i].procTime){
				sorted_tasks_pt.insert(it+i,unsorted_tasks_pt[t]);
				inserted = true;
				break;
			}
		}
		//if not inserted before an element, add as last
		if (!inserted)
			sorted_tasks_pt.push_back(unsorted_tasks_pt[t]);
	}


	vector <long int> sorted_tasks;

	for (int i = 0; i < sorted_tasks_pt.size(); i++){
		sorted_tasks.push_back(sorted_tasks_pt[i].task);
	}

	return sorted_tasks;

}


double computeEFT(FSP* fsp, long int task, long int position, long int machine, double ** earliest_finish_times){
	double EST; // earliest starting time
	double precendent_same_machine = 0;
	double precedent_previous_machine = 0;

	if (position != 0)
		precendent_same_machine = (earliest_finish_times[machine])[position-1];
	if (machine != 0){
		precedent_previous_machine = (earliest_finish_times[machine-1])[position];
	}

	if (precendent_same_machine >= precedent_previous_machine){
		EST = precendent_same_machine;
	}
	else
		EST = precedent_previous_machine;
	double EFT = EST + fsp->getTime(machine, task);
	return EFT;
}

double computeMakespan(FSP* fsp, vector<long int> sequence){
	long int machines = fsp->getNumberOfMachines();
	long int tasks = sequence.size();


	double ** earliest_finish_times;
	earliest_finish_times = new double * [machines];
	for (int i = 0; i < machines; i++){
		earliest_finish_times[i] = new double [tasks];
	}
	for (int m = 0; m < machines; m++){
		for (int p=0; p < tasks; p++){
			earliest_finish_times[m][p] = computeEFT(fsp, sequence[p], p, m, earliest_finish_times);
		}
	}

	/*cout << "EFTS °°°°°°°°°°°" << endl;

	for (int m = 0; m < machines; m++){
		for (int p=0; p < tasks; p++){
			cout << earliest_finish_times[m][p] << " ";
		}
		cout << endl;
	}*/

	return earliest_finish_times[machines-1][tasks-1];

}


sequence_makespan NEH(FSP* fsp){

	long int tasks = fsp->getNumberOfTasks();

	vector<long int> sorted_tasks = sortTasks(fsp);

	/*cout << "sorted tasks: " << endl;
	for (int i = 0; i < sorted_tasks.size(); i++){
		cout << sorted_tasks[i];
	}
	cout << endl;*/


	vector<long int> seq_so_far;

	// place the first two tasks such that the partial makespan is minimized
	vector<long int> seq1;
	vector<long int> seq2;
	long int first_task = sorted_tasks[0];
	long int second_task = sorted_tasks[1];
	seq1.push_back(first_task);
	seq1.push_back(second_task);
	seq2.push_back(second_task);
	seq2.push_back(first_task);



	if(computeMakespan(fsp, seq1) > computeMakespan(fsp, seq2)){
		seq_so_far = seq2;
	}
	else
		seq_so_far = seq1;


	double minimal_makespan = LONG_MAX;
	double minimizing_position = -1;
	for (int k = 2; k < sorted_tasks.size(); k++){
		/*cout << "sorted tasks: " << endl;
		for (int i = 0; i < sorted_tasks.size(); i++){
			cout << sorted_tasks[i];
		}
		cout << endl;
		cout << "------------ sequence so far:" << endl;
		for (int j = 0; j < seq_so_far.size(); j++){
			cout << seq_so_far[j] << endl;
		}*/


		long int current_task = sorted_tasks[k];
		// determine minimal makespan amongst the k possible positions before an element
		for (int position = 0; position < k; position++){
			minimal_makespan = LONG_MAX;
			vector<long int>::iterator it = seq_so_far.begin() + position;
			seq_so_far.insert(it, current_task);

			/*cout << "****** considering sequence:";
			for (int j = 0; j < seq_so_far.size(); j++){
				cout << seq_so_far[j] << " ";
			}
			cout << endl;
			cout << " with makespan " << computeMakespan(fsp, seq_so_far) << endl;*/

			if( computeMakespan(fsp, seq_so_far) < minimal_makespan ){
				minimizing_position = position;
				minimal_makespan = computeMakespan(fsp, seq_so_far);
			}
			it = seq_so_far.begin() + position;
			seq_so_far.erase(it);
		}
		// determine makespan of position = last
		seq_so_far.push_back(current_task);
		// insert task at position that minimizes the partial makespan (possibly last)
		/*cout << "****** considering sequence:";
		for (int j = 0; j < seq_so_far.size(); j++){
			cout << seq_so_far[j] << " ";
		}
		cout << endl;*/
		double mkspn = computeMakespan(fsp, seq_so_far);
		//cout << " with makespan " << mkspn << endl;
		if (! ( mkspn < minimal_makespan)){
			seq_so_far.pop_back();
			vector<long int>::iterator it = seq_so_far.begin() + minimizing_position;
			seq_so_far.insert(it, current_task);
		}
		else //positioned last
			minimal_makespan = mkspn;

	}


	sequence_makespan seq_mkspan;
	seq_mkspan.sequence = seq_so_far;
	seq_mkspan.makespan = minimal_makespan;

	return seq_mkspan;

}




#endif /* SRC_NEH_H_ */