
#include "aco.h"
#include "NEH.h"


using namespace std;

string instance_file;
FSP* fsp;

/*Probabilistic rule related variables*/
double  ** pheromone;   /* pheromone matrix */
double  ** heuristic;   /* heuristic information matrix */
double  ** probability;    /* combined value of pheromone X heuristic information */
double initial_pheromone=1;

long int max_iterations;   //Max iterations
long int iterations=0;
long int max_tours;        //Max tours
long int tours=0;
double alpha;
double beta;
double rho;
long int n_ants;
long int seed = -1;


vector<Ant> colony;
Ant global_best_ant;
Ant iteration_best_ant;
Ant iteration_worst_ant;
long int global_best_makespan=LONG_MAX;     /* length of the shortest makespan found */
long int iteration_best_makespan = LONG_MAX;
long int iteration_worst_makespan = 0;

long int machines;
long int tasks;


string algo; //type of algorithm
double stagnation_treshold;

// Max Min Ant System variables
double lower_limit;
double upper_limit;
double a = 1.5; //  lower limit = upper limit / a



// Best Ant Worst Ant variables
double mutation_treshold;
double last_restart = 0;
double mutation_probability;
double sigma;

/*Default parameters: set them!*/
void setDefaultParameters(){
	alpha=1;
	beta=1;
	rho=0.1;
	n_ants=5;
	max_tours=50;
	max_iterations= max_tours / n_ants;
	instance_file="";
	seed = (long int) time(NULL);

	stagnation_treshold = 0.20;
	mutation_probability = 0.10;
	sigma = 4;

	algo = "bawa"; //set Best Ant Worst Ant as default algorithm
	initial_pheromone = 1;

}

/*Print default parameters*/
void printParameters(){
	cout << "\nACO parameters:\n"
			<< "  ants: "  << n_ants << "\n"
			<< "  alpha: " << alpha << "\n"
			<< "  beta: "  << beta << "\n"
			<< "  rho: "   << rho << "\n"
			<< "  tours: " << max_tours << "\n"
			<< "  iterations: "   << max_iterations << "\n"
			<< "  seed: "   << seed << "\n"
			<< "  initial pheromone: "   << initial_pheromone << "\n"
			<< endl;
}

/* Read arguments from command line */
bool readArguments(int argc, char *argv[] ){

	setDefaultParameters();

	for(int i=1; i< argc ; i++){
		if(strcmp(argv[i], "--ants") == 0){
			n_ants = atol(argv[i+1]);
			i++;
		} else if(strcmp(argv[i], "--alpha") == 0){
			alpha = atof(argv[i+1]);
			i++;
		} else if(strcmp(argv[i], "--beta") == 0){
			beta = atof(argv[i+1]);
			i++;
		} else if(strcmp(argv[i], "--rho") == 0) {
			rho = atof(argv[i+1]);
			i++;
		} else if(strcmp(argv[i], "--iterations") == 0) {
			max_iterations = atol(argv[i+1]);
			i++;
		} else if(strcmp(argv[i], "--tours") == 0) {
			max_tours = atol(argv[i+1]);
			i++;
		} else if(strcmp(argv[i], "--seed") == 0) {
			seed = atol(argv[i+1]);
			i++;
		}else if(strcmp(argv[i], "--instance") == 0) {
			instance_file = argv[i+1];
			i++;
		}else if(strcmp(argv[i], "--sigma") == 0) {
			sigma = atol(argv[i+1]);
			i++;
		}else if(strcmp(argv[i], "--stagnation_treshold") == 0) {
			stagnation_treshold = atol(argv[i+1]);
			i++;
		}
		else if(strcmp(argv[i], "--mutation_probability") == 0) {
			mutation_probability = atol(argv[i+1]);
			i++;
		}
		else if(strcmp(argv[i], "--algo") == 0) {
			algo = argv[i+1];
			i++;
		}
		else{
			cout << "Parameter " << argv[i] << "no recognized.\n";
			return(false);
		}
	}
	if(instance_file.empty()){
		cout << "No instance file provided.\n";
		return(false);
	}
	if( (algo!="bawa") && (algo!="mmas") ){
		cout << "Algorithm not recognized. Try bawa or mmas. \n";
		return(false);
	}
	//printParameters();
	max_iterations= max_tours / n_ants;
	return(true);
}

/*Create colony structure*/
void createColony (){
	colony.clear();
	//cout << "Creating colony.\n\n";
	for (int i = 0 ; i < n_ants ; i++ ) {
		colony.push_back(Ant(fsp, probability, pheromone));
	}
}

/*Initialize pheromone with an initial value*/
void initializePheromone( double initial_value ) {
	pheromone = new double * [tasks];
	for (int i = 0 ; i < tasks ; i++ ) {
		pheromone[i] = new double [tasks];
		for (int j = 0  ; j < tasks ; j++ )
			pheromone[i][j] = initial_value;
	}
}

void printMatrix ( double ** matrix ) {
	long int i, j;
	long int size = tasks;
	printf("\n");
	for ( i = 0 ; i < size ; i++ ) {
		for ( j = 0 ; j < size ; j++ ) {
			printf(" %lf ", matrix[i][j]);
		}
		printf("\n");
	}
}

// Initialize the heuristic information matrix
void initializeHeuristic () {
	long int size = tasks;
	heuristic = new double * [size];
	for (int i = 0; i < size; i++){
		heuristic[i] = new double [size];
		for (int j = 0; j < size; j++){
			if (i!=j){
				heuristic[i][j] = 0; // 1./(tsp->getDistance(i,j));
			}
			else {
				heuristic[i][j] = 0;
			}
		}
	}
}


// Calculate probability in base of heuristic
// information and pheromone
void calculateProbability () {
	long int size = tasks;
	for (int i = 0; i < size; i++){
		for (int j = 0; j < size; j++){
			(probability[i])[j] = pow(pheromone[i][j], alpha); // * pow(heuristic[i][j], beta);
		}
	}
}

void initializeProbability(){
	probability = new double * [tasks];
	for (int i = 0; i < tasks; i++){
		probability[i] = new double [tasks];
	}
	calculateProbability();
}


//calculate the mutation treshold = average of pheromone over the trails of the best solution
double mutationTreshold(){
	double sum = 0;
	for (int i = 0; i < tasks; i++){
		sum += (pheromone[i])[global_best_ant.getSequence()[i]];
	}
	return (double) sum / (double) tasks;
}

double mutationValue(){
	return  (double) (iterations - last_restart)/ (double)(max_iterations - last_restart) * sigma * mutation_treshold;
}

double mutate(){
	for (int i = 0; i < tasks; i++){
		for (int j = 0; j <tasks; j++){

			/*long int seed = std::chrono::system_clock::now().time_since_epoch().count()/rand();
			srand(seed);
			double randVal = ((double) rand() / (RAND_MAX));*/

			double randVal = ran01(&seed);

			//mutate with probability = mutation_probability
			if (mutation_probability > randVal){

				/*seed = std::chrono::system_clock::now().time_since_epoch().count()/rand();
				srand(seed);
				randVal = ((double) rand() / (RAND_MAX));*/
				randVal = ran01(&seed);

				if (randVal < 0.5){ // a = 0 : add mutation value
					(pheromone[i])[j] += mutationValue();
				}
				else{  // a = 1 : subtract mutation value
					(pheromone[i])[j] -= mutationValue();
					if ((pheromone[i])[j] < 0)
						(pheromone[i])[j] = 0;
				}
			}
		}
	}
}

void setLimits(){
	upper_limit = 1/(rho * global_best_makespan);
	lower_limit = upper_limit / a;

}


//bring all pheromone values within range [lower_limit, upper_limit]
void bound(){
	setLimits();
	for (int i = 0; i < tasks; i++){
		for (int j = 0; j < tasks; j++){
			if( (pheromone[i])[j] < lower_limit)
				(pheromone[i])[j] = lower_limit;
			if ( (pheromone[i])[j] > upper_limit )
				(pheromone[i])[j] = upper_limit;
		}
	}
}

/*Pheromone evaporation*/
void evaporatePheromone(){
	long int size = tasks;
	for (int i = 0; i < size; i++){
		for (int j = 0; j < size; j++){
			(pheromone[i])[j] = (1-rho)*(pheromone[i])[j];
		}
	}

	//additional evaporation for edges present in current worst solution but not in global best solution
	if (algo == "bawa" ){
		for (int i = 0; i < tasks; i++){
			if (iteration_worst_ant.getSequence()[i] != global_best_ant.getSequence()[i]){
				(pheromone[i])[ iteration_worst_ant.getSequence()[i] ] = (1-rho) * (pheromone[i])[iteration_worst_ant.getSequence()[i] ];
			}
		}
	}
}

/*Update pheromone*/
void depositPheromone(){
	// for MMAS, best ant is the iteration best ant
	// for BAWA, best ant is the global best ant
	if(algo =="mmas")
		iteration_best_ant.depositPheromones();
	if (algo=="bawa")
		global_best_ant.depositPheromones();
}

bool stagnation(){
	int number_of_different_edges = 0;
	for (int i = 0; i < tasks; i++){
		if (iteration_worst_ant.getSequence()[i] != iteration_best_ant.getSequence()[i]){
			number_of_different_edges++;
		}
	}
	return ((double)number_of_different_edges / (double)tasks) < stagnation_treshold;
}

/*Check termination condition*/
bool terminationCondition(){
	return iterations == max_iterations;
}

/*Free memory used*/
void freeMemory(){
	for(int i=0; i < fsp->getNumberOfTasks(); i++){
		delete[] pheromone[i];
		delete[] heuristic[i];
		delete[] probability[i];
	}


	delete[] pheromone;
	delete[] heuristic;
	delete[] probability;
	//delete fsp;
}


int main(int argc, char *argv[] ){
	if(!readArguments(argc, argv)){
		exit(1);
	}


	fsp= new FSP(instance_file);
	machines = fsp->getNumberOfMachines();
	tasks = fsp -> getNumberOfTasks();

	int trials = 1;
	ofstream boxPlotFile;
	boxPlotFile.open("boxPlot.txt");



	for (int trial = 0; trial < trials; trial++){

		tours = 0;
		iterations = 0;

		boxPlotFile << trial+1 ;

		global_best_makespan = LONG_MAX;

		//initialisation
		if (algo == "mmas"){
			sequence_makespan seedSequenceVec = NEH(fsp);
			long int* NEH_seq = new long int [tasks];
			for (int i = 0; i < tasks; i++ ){
				NEH_seq[i] = seedSequenceVec.sequence[i];
			}
			global_best_ant = Ant();
			global_best_ant.setSequence(NEH_seq);
			global_best_ant.setMakespan(seedSequenceVec.makespan);
			global_best_makespan = seedSequenceVec.makespan;
			upper_limit = 1/(rho * seedSequenceVec.makespan);
			lower_limit = upper_limit / a;
			initial_pheromone = upper_limit;
		}

		initializePheromone(initial_pheromone);
		initializeHeuristic();
		initializeProbability();
		createColony();





		//Iterations loop
		while(!terminationCondition()){

			iteration_worst_makespan = 0;
			iteration_best_makespan = LONG_MAX;



			for(int i=0; i< n_ants; i++){

				if (algo == "bawa")
					colony[i].Search(seed);
				if (algo == "mmas")
					colony[i].searchMMAS(global_best_ant.getSequence(), seed);




				if(global_best_makespan > colony[i].getMakespan()){
					global_best_makespan = colony[i].getMakespan();
					global_best_ant.copyFrom(colony[i]);
					mutation_treshold = mutationTreshold();

				}
				if(iteration_best_makespan > colony[i].getMakespan()){
					iteration_best_makespan = colony[i].getMakespan();
					iteration_best_ant.copyFrom(colony[i]);
					mutation_treshold = mutationTreshold();

				}
				if (iteration_worst_makespan < colony[i].getMakespan()){
					iteration_worst_makespan = colony[i].getMakespan();
					iteration_worst_ant.copyFrom(colony[i]);
				}
				tours++;
			}


			evaporatePheromone();
			depositPheromone();

			if(algo=="bawa")
				mutate();
			if(algo == "mmas"){
				setLimits();
				bound();
			}


			if (stagnation()){
				initializePheromone(initial_pheromone);
				last_restart = iterations;
			}

			calculateProbability();
			iterations++;
		}

		boxPlotFile << ":" << global_best_makespan ;
		boxPlotFile << endl;

		cout << global_best_ant.getMakespan() << endl;
	}


	freeMemory();   // Free memory.



}




