#include <bits/stdc++.h>

using namespace std;

int main(){
    
    std::ofstream workload_file;
    workload_file.open("workload.txt");
    assert(workload_file);

    long long key = 0;
    for(; key < 5000; key++){
	string data;
	for(int i = 0; i < 4; i++){
		data.push_back('a' + rand()%26);
	}
    	workload_file << "I" << " " << key << " " << data << endl;
    }

    workload_file << "D Range " << "0" << " " << "2500" << endl;
    workload_file << "D Range " << "2500" << " " << "5000" << endl;

    for(; key < 43330; key++){	
	string data;
	for(int i = 0; i < 4; i++){
		data.push_back('a' + rand()%26);
	}
    	workload_file << "I" << " " << key << " " << data << endl;
    }

}
