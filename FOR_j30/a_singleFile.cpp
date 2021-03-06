#include <bits/stdc++.h>
#define NUM_RESOURCES 4
#define P_MAX 1
#define P_MIN 0

using namespace std; 

int N = 32;
int M = 1 ;							// No of particles
float G = 0.8 ;						// Contraction factor
int MAX_ITERS = 1; 					// Total no of schedules to be generated. 

vector<int> succ_list[123];			// Array of lists that stores successors for every vertex. 
vector<int> pred_list[123];			// Array of lists that stores predecesors for every vertex.
vector<int> res_req_list[123];		// 2D-Array storing Resource requiremments for all the activities.
int duration[123];					
int max_resources[NUM_RESOURCES];
double mBest[123] ;					// m-best used in QPSO equations

double gBest_pos[123];
int gBest_cost = INT_MAX;

// Generates a float random number between zero to one.
double getRandom0to1(){
	return (double) rand()/RAND_MAX ; 
}
void read_file(char*);

class Particle{
	public:
	double pos[123] ; 
	double best_pos[123] ;
	int best_cost ;
	Particle(){
		for(int i=0; i<N ; i++)
			best_pos[i] = pos[i] = getRandom0to1();
		best_cost = INT_MAX ;
	}
	void performOps();
	int evaluateSchedule(bool forward) ;
}particles[123];

class Activity{
	public:
	int activity_id ;
	int start_time ;
	int duration ;
	int f_time ;
};

// A comparator function for sorting of feasible activities on 
// the basis of decreasing priority.
int current_particle_id = 0;
int compare(const int &a, const int &b){
		return (particles[current_particle_id].pos[a] > particles[current_particle_id].pos[b]);
}

// Returns activities which are satisfying precedence constraints
// and have not been scheduled for execution. Resource contraints 
// are not checked in this function.
vector<int> getFeasibleActs(bool scheduled[], int completed_preds[], vector<int> *dList){
	vector<int> feasible_acts;
	for(int i=0 ; i<N ; i++){
		if(completed_preds[i] == dList[i].size()  &&  !scheduled[i])
			feasible_acts.push_back(i);
	}
	return feasible_acts ;
}

// Reads the files and then performs the required operations.
void executeOnFile(char* filename){
	read_file(filename);
	int i, iterations = MAX_ITERS ;
	while(iterations--){
		for(i=0 ; i<N ; i++){
			double b = 0 ;
			for(int j=0 ; j<M ; j++)
				b += particles[i].pos[j];
			mBest[i] = b/M ;
		}
		for(i=0 ; i<M ; i++){
			current_particle_id = i ;
			particles[i].performOps();
		}
	}
	printf("%d", gBest_cost) ;
}

// Perform operations on particle. Called once in every iterations.
// Operations include: Updating of particle's position.
void Particle::performOps(){
	
	double c1 = getRandom0to1();
	double c2 = getRandom0to1();
	double u = getRandom0to1();
	double r = getRandom0to1();
	if(u == 0.0)
		u = 0.5 ;

	double P[N] ;				//Optimization : Allocate dynamically and reuse 
	double diff[N], z ;
	for(int i=0 ; i<N ; i++){

		P[i] = (c1*best_pos[i] + c2* gBest_pos[i])/(c1+c2) ;
		diff[i] = abs(mBest[i] - pos[i]) ;
		diff[i] *= G*log(1/u) ;
		if(r<0.5)
			pos[i] = P[i] - diff[i] ;
		else
			pos[i] = P[i] + diff[i] ;

		if(pos[i]<P_MIN)
			pos[i] = P_MIN ;
		else if(pos[i] > P_MAX)
			pos[i] = P_MAX ;
	}	
	int cost = min(evaluateSchedule(true), evaluateSchedule(false));
	if(best_cost>cost){
		best_cost = cost ;
		for(int i=0 ; i<N; i++)
			best_pos[i] = pos[i] ;
	}

	if(gBest_cost>cost){
		gBest_cost = cost ;
		for(int i=0 ; i<N; i++)
			gBest_pos[i] = pos[i] ;
	}
}

// Fitness function : Evaluates the schedule in the direction given as a parameter
// If true then forward schedule is evaluated else the backward schedule is evaluated.
// Forms the schedule and returns the makespan(duration of the schedule).  
int Particle::evaluateSchedule(bool forward){
    vector<Activity> sList ;
	bool scheduled[N], finished[N] ;
	int completed_preds[N];
	int time = 0 ;

	for(int i=0 ; i<N ; i++){	
		scheduled[i] = finished[i] = false ;
		completed_preds[i] = 0 ;
	}		
	
	Activity a0 ;
    a0.activity_id = 0 ;
    a0.start_time = 0 ;
    a0.duration = 0 ;
    a0.f_time = 0 ;

    sList.push_back(a0) ;
    scheduled[0] = true ;
    
	int resources_left[NUM_RESOURCES] ;	//OPTIMIZATION: This is redundant
    for(int i=0; i<NUM_RESOURCES ; i++)
    	resources_left[i] = max_resources[i] ;

    for(int num_finished = 0 ; num_finished<N ; ){
    	// cout<<"==>Time:"<<time<<" Finished:"<<num_finished<<"\n";
	    for(int i=0 ; i<sList.size() ; i++){
	       	if(sList[i].f_time == time && !finished[sList[i].activity_id]){
	    		int id = sList[i].activity_id ;
	    		finished[id] = true ;
	    		// cout<<"Activity "<<id<<" finished.\n";
	    		num_finished++ ;
	    		for(int j=0 ; j<NUM_RESOURCES ; j++){
	    			resources_left[j] += res_req_list[id][j];
	    			// cout<<res_req_list[id][j]<<"/"<<resources_left[j]<<"/"<<max_resources[j]<<"\n";
	    		}
	    		
	    		if(forward)
	    			for(int j=0 ; j<succ_list[id].size(); j++)
	    				completed_preds[succ_list[id][j]]++ ;
	    		else
	    			for(int j=0 ; j<pred_list[id].size(); j++)
	    				completed_preds[pred_list[id][j]]++ ;
	    	}
	    }
			
        vector<int> feasible_activities ;
        if(forward)
        	feasible_activities = getFeasibleActs(scheduled, completed_preds, pred_list) ;
		else
			feasible_activities = getFeasibleActs(scheduled, completed_preds, succ_list) ;	
		sort( feasible_activities.begin(), feasible_activities.end(), compare ) ;	//sort by decreasing priority ---====CHECk

		int flag, id;
		for (int i=0 ; i<feasible_activities.size() ; i++){
			id = feasible_activities[i] ;
			flag = 0 ;
			for(int j=0; j<4 ;j++)
				if(resources_left[j] < res_req_list[id][j])
					flag = 1 ;
			if(flag == 0){
				// Schedule this activity
				Activity a1 ;
				a1.activity_id = id ;
				a1.start_time = time ;
				a1.duration = duration[id];
				a1.f_time = a1.start_time + a1.duration ;
				sList.push_back(a1) ;
				scheduled[id] = true ;
				if(a1.start_time == a1.f_time)
					time-- ;
				for(int j=0 ; j<NUM_RESOURCES ; ++j)
					resources_left[j] -= res_req_list[id][j] ;
			}
		}
		time++;
	}

	int cost = 0;
	for(int i=0 ; i<sList.size() ; i++){
	  cost = max(cost, sList[i].f_time) ; 
    }
    return cost ;
}


// You can modify N, M, G at the top. 
int main(int argc, char **argv){

	for(int i=0 ; i<N; i++)
		gBest_pos[i] = getRandom0to1();
	srand((unsigned)time(NULL));
	
	char filename[20];
	strcpy(filename, "j301_1.sm") ;

	executeOnFile(filename);
}

// File reading function.
void read_file(char* filename){		//Tested: Working perfectly!
	FILE *fp = fopen(filename, "r");
	char line[100];
	int i = 1 ;
	int num[10];
	while( fgets(line, 79, fp) != NULL ){
		istringstream iss(line) ;
		// cout<<" "<<line;
		if(19<=i && i<N+19){
			//Parse successors
			iss>>num[0]>>num[1]>>num[2] ;
			// cout<<num[0]<<" has "<<num[2]<<" successors\n";
			for(int k=0 ; k<num[2] ; k++){
				iss>>num[3];
				// cout<<" "<<num[3];
				succ_list[num[0]-1].push_back(num[3]-1) ;
				pred_list[num[3]-1].push_back(num[0]-1) ;
			}
			// cout<<"\n";
		}
		else if(N+19+4<=i && i<N+19+4+N){
			//Parse resource requirements
			iss>>num[0]>>num[1]>>num[5]>>num[1]>>num[2]>>num[3]>>num[4] ;
			// printf("Activity: %d R1:%d R2:%d R3:%d R4%d\n", num[0],num[1],num[2],num[3],num[4]);
			res_req_list[num[0]-1].push_back(num[1]);
			res_req_list[num[0]-1].push_back(num[2]);
			res_req_list[num[0]-1].push_back(num[3]);
			res_req_list[num[0]-1].push_back(num[4]);
			duration[num[0]-1] = num[5] ;
		}
		else if(i==2*N+19+4+3){
			iss>>max_resources[0]>>max_resources[1]>>max_resources[2]>>max_resources[3];
		}
			i++;
	}
}
