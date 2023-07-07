#include "math.h"
#include <algorithm>
#include "time.h"
#include <random>
#include <fstream>
#include <iomanip>


using namespace std;
const int VALUE_SIZE = 4;

string get_value(){
	string value;
	for(int i = 0; i < VALUE_SIZE; i++){
		value.push_back(rand() % 26 + 'a');
	}
	return value;
}

int main(){
 	ofstream workload4_file;
	workload4_file.open("workload4.txt");
	
	
	for(int j = 1; j < 10000; j+=2){

		if(j > 50 && (j-1) % 50 == 0){
			workload4_file << "D Range " << (j-49) << " " << (j-1) << endl;
		}else{
			workload4_file << "I " << j << " " << get_value << endl;
		}	
	}
}
