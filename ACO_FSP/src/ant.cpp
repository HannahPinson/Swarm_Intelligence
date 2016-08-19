#include "ant.h"

/* Default constructor*/
Ant::Ant(){
	init=false;
}

/* Constructor*/
Ant::Ant (FSP* fsp_arg, double ** prob_info, double ** phero){
	size = fsp_arg->getNumberOfTasks();
	tasks = fsp_arg->getNumberOfTasks();
	machines =  fsp_arg->getNumberOfMachines();
	fsp  = fsp_arg;
	probability = prob_info;
	pheromone = phero;
	sequence = new long int[size];
	scheduled = new long int[size];
	makespan = LONG_MAX;
	init = true;
}

/* Copy constructor*/
Ant::Ant (Ant const& other){
	size = other.size;
	tasks = other.tasks;
	machines = other.machines;
	fsp  = other.fsp;
	probability = other.probability;
	pheromone = other.pheromone;
	sequence = new long int[size];
	scheduled = new long int[size];
	for(int i=0; i<size ; i++){
		sequence[i] = other.sequence[i];
		scheduled[i] = other.scheduled[i];
	}
	makespan = other.makespan;
	init=true;

}

Ant::~Ant (){
	if(init){
		delete[] sequence;
		delete[] scheduled;
	}
	init=false;
}

bool Ant::alreadyScheduled(long int task, long int currentIndex){
	for (int i = 0; i < currentIndex; i++ ){
		if (task == scheduled[i])
			return true;
	}
	return false;
}



/*Generate tour using probabilities*/
void Ant::Search(long int seed, double beta, double ** heuristic){


	long int previous_job = -1;

	for (int i = 0; i < tasks; i++){

		double accul_prob = 0;
		vector<double> accul_prob_vec;
		vector<long int> toSelectFrom;

		// accumulate probabilities of not yet scheduled tasks
		for (int t = 0; t < tasks; t++){
			if (!alreadyScheduled(t, i)){
				toSelectFrom.push_back(t);
				if (previous_job != -1)
					accul_prob += (probability[i])[t] * pow((heuristic[previous_job])[t], beta);
				else
					accul_prob += (probability[i])[t];
				accul_prob_vec.push_back(accul_prob);
			}
		}
		// renormalize
		for (int j=0; j < toSelectFrom.size(); j++){
			accul_prob_vec[j] = accul_prob_vec[j] / accul_prob; //renormalization
		}

		double randVal = ((double) rand() / (RAND_MAX));
		srand(randVal * 10000000000);
		//double randVal = ran01(&seed);
		long int selectedTask;
		for (int j=0; j < toSelectFrom.size(); j++){
			if (accul_prob_vec[j] > randVal){
				selectedTask = toSelectFrom[j];
				break;
			}
		}
		sequence[i] = selectedTask;
		scheduled[i] = selectedTask;
		previous_job = selectedTask;

	}

	ComputeMakespan();
}


void Ant::searchMMAS(long int * best_sol_so_far, long int seed, double beta, double ** heuristic){

	long int previous_job = -1;

	for (int i = 0; i < tasks; i++){


		//collect 5 first unscheduled tasks in best solution so far
		// (or less if less than 5 remaining)
		int collected = 0;
		vector<long int> toSelectFrom;
		for (int t = 0; t < tasks; t++){
			long int task = best_sol_so_far[t];
			if (!alreadyScheduled(task, i)){
				toSelectFrom.push_back(task);
				collected++;
				if (collected == 5)
					break;
			}
		}


		long int selectedTask;
		double randVal = ((double) rand() / (RAND_MAX));
		srand(randVal * 10000000000);
		//double randVal = ran01(&seed);

		//select a task:


		// pick task with highest probability
		if (randVal <= (double)(tasks-4)/(double)tasks){
			double largest_value = 0;
			for (int t = 0; t < toSelectFrom.size(); t++){
				long int task = toSelectFrom[t];
				double prob;
				if (previous_job != -1)
					prob = (probability[i])[task] * pow((heuristic[previous_job])[task], beta);
				else
					prob = (probability[i])[task];

				if (prob > largest_value){
					selectedTask = task;
					largest_value = prob;
				}
			}
		}

		// pick task according to probability distribution
		else {

			// pick among 5 best according to probability distribution
			double accul_prob = 0;
			vector<double> accul_prob_vec;
			for (int t = 0; t < toSelectFrom.size(); t++){
				long int task = toSelectFrom[t];
				if (previous_job != -1)
					accul_prob += (probability[i])[task] * pow((heuristic[previous_job])[task], beta);
				else
					accul_prob += (probability[i])[task];
				accul_prob_vec.push_back(accul_prob);
			}
			// renormalize
			for (int j=0; j < toSelectFrom.size() ; j++){
				accul_prob_vec[j] = accul_prob_vec[j] / accul_prob; //renormalization
			}

			double randVal = ((double) rand() / (RAND_MAX));
			srand(randVal * 10000000000);
			randVal = ((double) rand() / (RAND_MAX));
			//double randVal = ran01(&seed);
			for (int j=0; j < toSelectFrom.size(); j++){
				if (accul_prob_vec[j] > randVal){
					selectedTask = toSelectFrom[j];
					break;
				}
			}
		}

		sequence[i] = selectedTask;
		scheduled[i] = selectedTask;
		previous_job = selectedTask;
	}

	ComputeMakespan();

}





double Ant::computeEFT(long int task, long int position, long int machine, double ** earliest_finish_times){
	double EST; // earliest starting time
	double precendent_same_machine = 0;
	double precedent_previous_machine = 0;
	if (position != 0)
		precendent_same_machine = (earliest_finish_times[machine])[position-1];
	if (machine != 0)
		precedent_previous_machine = (earliest_finish_times[machine-1])[position];
	if (precendent_same_machine >= precedent_previous_machine){
		EST = precendent_same_machine;
	}
	else
		EST = precedent_previous_machine;
	double EFT = EST + fsp->getTime(machine, task);
	return EFT;
}



/*Compute the makespan*/
void Ant::ComputeMakespan() {
	double ** earliest_finish_times;
	earliest_finish_times = new double * [machines];
	for (int i = 0; i < machines; i++){
		earliest_finish_times[i] = new double [tasks];
		//cout << &earliest_finish_times[i] << endl;
	}

	for (int m = 0; m < machines; m++){
		for (int p=0; p < tasks; p++){
			earliest_finish_times[m][p] = computeEFT(sequence[p], p, m, earliest_finish_times);
		}
	}

	makespan = earliest_finish_times[machines-1][tasks-1];
}


long int Ant::getMakespan(){
	return(makespan);
}

void Ant::depositPheromones(){
	for (int i = 0; i < size; i++){
		(pheromone[i])[sequence[i]] += 1./makespan;
	}
}

void Ant::copyFrom(Ant &other){

	size = other.size;
	tasks = other.tasks;
	machines = other.machines;
	fsp  = other.fsp;
	probability = other.probability;
	pheromone = other.pheromone;
	sequence = new long int[size];
	scheduled = new long int[size];
	for(int i=0; i<size ; i++){
		sequence[i] = other.sequence[i];
		scheduled[i] = other.scheduled[i];
	}
	makespan = other.makespan;
	init=true;


}




