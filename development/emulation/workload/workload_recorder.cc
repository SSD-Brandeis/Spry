#include "workload_recorder.h"

using namespace std;
using namespace workload;

map<long, string> WorkloadRecorder::ground_truth;
map<long, long> WorkloadRecorder::insert_timestamp;
set<long> WorkloadRecorder::history_key_set;

void WorkloadRecorder::insert(long key, string value, long timestamp){
    ground_truth[key] = value;
    history_key_set.insert(key);

    insert_timestamp[key] = timestamp;
} 

void WorkloadRecorder::remove(long key){
    ground_truth.erase(key);
}

void WorkloadRecorder::remove(long startKey, long endKey){
    auto it = ground_truth.lower_bound(startKey);
    auto ite = ground_truth.upper_bound(endKey);
    ground_truth.erase(it, ite);
}

bool WorkloadRecorder::isExist(long key){
    return ground_truth.find(key) != ground_truth.end();
}

string WorkloadRecorder::getValue(long key){
    if(isExist(key) == false){
        cout << "Key not found in ground truth" << endl;
        cout << "Do isExist checking first" << endl;
        exit(1);
    }

    return ground_truth[key];
}

map<long, string> WorkloadRecorder::getGroundTruth(){
    return ground_truth;
}

void WorkloadRecorder::printGroundTruth(){
    int count = 0;
    for(auto it = ground_truth.begin(); it != ground_truth.end(); it++){
        cout << "[" << count << "]" << " " << it->first << " " << it->second << endl;
        count++;
    }
}

set<long> WorkloadRecorder::getHistoryKeySet(){
    return history_key_set;
}

bool WorkloadRecorder::isHistoryKey(long key){
    return history_key_set.find(key) != history_key_set.end();
}

long WorkloadRecorder::getInsertTimestamp(long key){
    return insert_timestamp[key];
}

long WorkloadRecorder::getCurrentExistingPointEntryCount(){
    return ground_truth.size();
}

using pll = pair<long, long>;
vector<pll> WorkloadRecorder::getNRangesOfSelectivity(int numN, double selectivity, int totPoint){
    if(selectivity < 0 || selectivity > 1){
        cout << "Error: Selectivity must be between 0 and 1" << endl;
        exit(1);
    }
    if(ground_truth.size() < totPoint){
        cout << "Error: number of existing point entry not enough for selectivity to generate range tombstone" << endl;
        exit(1);
    }
    
    long numP = ceil(totPoint * selectivity);
    if(numP == 0){
        cout << "Error:  Selectivity too small that results to 0 point chosen for a range." << endl;
        exit(1);
    }
    if( (numP + numN -1) > totPoint){
        cout << "Error: current number of exists point entry not enough to generate #" << numP 
                << " range tombstones under selectivity " << selectivity << endl;
        exit(1);
    }

    long max;
    
    int count = 0;
    auto it = ground_truth.begin();
    auto it2 = ground_truth.begin();
    auto ite = ground_truth.end();
    for(int i = 0; i < numP && it2 != ite; i++){
        it2++;
    }
    vector<pll> rangeList;
    while(it2 != ite && count < numN){
        rangeList.push_back(make_pair(it->first, it2->first));
        count++;
        it++;
        it2++;
    }


    return rangeList;
}