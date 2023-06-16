#include<bits/stdc++.h>

using namespace std;

int main(){

	//File *fp = fopen("../workload.txt");
	fstream workload_file;
	workload_file.open("../workload.txt");
	assert(workload_file);

	unordered_set<int>  Set;

	string instruction;
	while(getline(workload_file, instruction)){
		stringstream ss(instruction);
		
		string type;
		ss >> type;

		if(type == "I"){
			long sortkey;
			long deletekey;
			string value;

			ss >> sortkey >> deletekey >> value;
			Set.insert(sortkey);
		}
	}

	cout << "Total disctinct key = " << Set.size() << endl;
	

}
