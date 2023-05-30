#include "system_verifier.h"

using namespace std;
using namespace checking;

vector<int> SystemVerifier::checkOnExistingKeys() {
    vector<int> disk_access_count_list;

    //test on current existing keys
    map<long, string> groundtruth = WorkloadRecorder::getGroundTruth();
    for(int i = 0; i < 3; i++){
        if(i == 0){
            cout << "Perlevel Range Delete Filter:" << endl;
        }else if(i == 1){
            cout << "Skyline Range Delete Filter:" << endl;
        }else if(i == 2){
            cout << "Perlevel Split Range Delete Filter:" << endl;
        }else{
            cout << "ERROR: No such filter. Unimplemented if-else block. " << __FILE__ << " " << __LINE__ << " " << __func__ << endl; 
            exit(1);
        }
        
        cout << "verify on existing keys" << endl;
        cout << "number of point query: " << groundtruth.size() << endl;

        Query::resetDiskSSTFileAccessCount();

        int64_t total_elapsed_time = 0;
        for(int j_times = 0; j_times < EXPERIMENT_REPETITION_TIMES; j_times++){
            auto start_pq = std::chrono::high_resolution_clock::now();
            for(auto it = groundtruth.begin(); it != groundtruth.end(); it++)
            {
                long key = it->first;
                string value = it->second;

                pair<bool, string> point_query_result;
                if(i == 0){
                    point_query_result = Query::pointQuery_RDF(key);
                }else if(i == 1){
                    long timestamp = WorkloadRecorder::getInsertTimestamp(key);
                    point_query_result = Query::pointQuery_RDF_SkyLine(key, timestamp);
                }else if(i == 2){
                    point_query_result = Query::pointQuery_SplitRDF(key);
                }else{
                    cout << "ERROR: No such filter. Unimplemented if-else block. " << __FILE__ << " " << __LINE__ << " " << __func__ << endl; 
                    exit(1);
                }

                if( (point_query_result.first != true) || (point_query_result.second != value) ){            
                    cout << "@existing keys point query test" << endl;
                    cout << "ERROR: key: " << key << " value: " << (groundtruth[key]) << endl;
                    if(i == 0){
                        cout << " point_query_result: " << point_query_result.first << " " << point_query_result.second << endl; 
                    }else if(i == 1){
                        long timestamp = WorkloadRecorder::getInsertTimestamp(key);
                        cout << " point_query_result: " << point_query_result.first << " " << point_query_result.second << " " << timestamp << endl; 
                    }else if(i == 2){
                        cout << " point_query_result: " << point_query_result.first << " " << point_query_result.second << endl; 
                    }else{
                        cout << "ERROR: No such filter. Unimplemented if-else block. " << __FILE__ << " " << __LINE__ << " " << __func__ << endl; 
                        exit(1);
                    }
                    cout << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
                    exit(1);
                }
            }
            auto stop_pq = std::chrono::high_resolution_clock::now();
            auto duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
            total_elapsed_time += duration_pq.count();
        }
        cout << "point query time: " << total_elapsed_time/EXPERIMENT_REPETITION_TIMES << " microseconds" << endl;
        
        int disk_access_count = Query::getDiskSSTFileAccessCount()/EXPERIMENT_REPETITION_TIMES;
        cout << "disk access count: " << disk_access_count << endl;
        cout << endl << endl;

        disk_access_count_list.push_back(disk_access_count);
    }
    return disk_access_count_list;
}

vector<int> SystemVerifier::checkOnAllInsertedKeys() {
    vector<int> disk_access_count_list;

    //test on all historical keys
    map<long, string> groundtruth = WorkloadRecorder::getGroundTruth();
    set<long> history_key_set = WorkloadRecorder::getHistoryKeySet();
    for(int i = 0; i < 3; i++){
        if(i == 0){
            cout << "Perlevel Range Delete Filter:" << endl;
        }else if(i == 1){
            cout << "Skyline Range Delete Filter:" << endl;
        }else if(i == 2){
            cout << "Perlevel Split Range Delete Filter:" << endl;
        }else{
            cout << "ERROR: No such filter. Unimplemented if-else block. " << __FILE__ << " " << __LINE__ << " " << __func__ << endl; 
            exit(1);
        }
        cout << "verify on all inserted keys" << endl;
        cout << "number of point query: " << history_key_set.size() << endl;

        Query::resetDiskSSTFileAccessCount();
        
        int64_t total_elapsed_time = 0;
        for(int j_times = 0; j_times < EXPERIMENT_REPETITION_TIMES; j_times++){
            auto start_pq = std::chrono::high_resolution_clock::now();
            for(auto it = history_key_set.begin(); it != history_key_set.end(); it++)
            {
                long key = *it;
                pair<bool, string> point_query_result;
                
                if(i == 0){
                    point_query_result = Query::pointQuery_RDF(key);
                }else if(i == 1){
                    long timestamp = WorkloadRecorder::getInsertTimestamp(key);
                    point_query_result = Query::pointQuery_RDF_SkyLine(key, timestamp);
                }else if(i == 2){
                    point_query_result = Query::pointQuery_SplitRDF(key);
                }else{
                    cout << "ERROR: No such filter. Unimplemented if-else block. " << __FILE__ << " " << __LINE__ << " " << __func__ << endl; 
                    exit(1);
                }

                auto it_groundtruth = groundtruth.find(key);
                bool is_exist = groundtruth.find(key) != groundtruth.end();
                bool flag1 =  (point_query_result.first != is_exist);
                bool flag2 =  is_exist&&(point_query_result.second != it_groundtruth->second);
                if( flag1 || flag2){
                    cout << "@historical key point query test" << endl;
                    cout << "ERROR: key: " << key << " is exist: " << is_exist << " value: " << (is_exist? groundtruth[key] : "") << endl;
                    if(i == 0){
                        cout << " point_query_result: " << point_query_result.first << " " << point_query_result.second << endl; 
                    }else if(i == 1){
                        long timestamp = WorkloadRecorder::getInsertTimestamp(key);
                        cout << " point_query_result: " << point_query_result.first << " " << point_query_result.second << " " << timestamp << endl; 
                    }else if(i == 2){
                        cout << " point_query_result: " << point_query_result.first << " " << point_query_result.second << endl; 
                    }else{
                        cout << "ERROR: No such filter. Unimplemented if-else block. " << __FILE__ << " " << __LINE__ << " " << __func__ << endl; 
                        exit(1);
                    }
                    cout << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
                    exit(1);
                }
            }
            auto stop_pq = std::chrono::high_resolution_clock::now();
            auto duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
            total_elapsed_time += duration_pq.count();
        }
        cout << "point query time: " << total_elapsed_time/EXPERIMENT_REPETITION_TIMES << " microseconds" << endl;
        
        int disk_access_count = Query::getDiskSSTFileAccessCount()/EXPERIMENT_REPETITION_TIMES;
        cout << "disk access count: " << disk_access_count << endl;
        cout << endl << endl;

        disk_access_count_list.push_back(disk_access_count);
    }
    return disk_access_count_list;
}


vector<int> SystemVerifier::checkOnAllCurrentlyDeletedKeys() {
    vector<int> disk_access_count_list;

    //test on all historical keys
    map<long, string> groundtruth = WorkloadRecorder::getGroundTruth();
    set<long> history_key_set = WorkloadRecorder::getHistoryKeySet();
    for(int i = 0; i < 3; i++){
        if(i == 0){
            cout << "Perlevel Range Delete Filter:" << endl;
        }else if(i == 1){
            cout << "Skyline Range Delete Filter:" << endl;
        }else if(i == 2){
            cout << "Perlevel Split Range Delete Filter:" << endl;
        }else{
            cout << "ERROR: No such filter. Unimplemented if-else block. " << __FILE__ << " " << __LINE__ << " " << __func__ << endl; 
            exit(1);
        }
        cout << "verify on all currently deleted point entries" << endl;
        cout << "number of point query: " << (history_key_set.size() - groundtruth.size()) << endl;

        Query::resetDiskSSTFileAccessCount();
        
        int64_t total_elapsed_time = 0;
        for(int j_times = 0; j_times < EXPERIMENT_REPETITION_TIMES; j_times++){
            auto start_pq = std::chrono::high_resolution_clock::now();
            for(auto it = history_key_set.begin(); it != history_key_set.end(); it++)
            {
                long key = *it;
                
                auto it_groundtruth = groundtruth.find(key);
                bool is_exist = groundtruth.find(key) != groundtruth.end();
                if(is_exist == true){continue;}
                
                pair<bool, string> point_query_result;
                if(i == 0){
                    point_query_result = Query::pointQuery_RDF(key);
                }else if(i == 1){
                    long timestamp = WorkloadRecorder::getInsertTimestamp(key);
                    point_query_result = Query::pointQuery_RDF_SkyLine(key, timestamp);
                }else if(i == 2){
                    point_query_result = Query::pointQuery_SplitRDF(key);
                }else{
                    cout << "ERROR: No such filter. Unimplemented if-else block. " << __FILE__ << " " << __LINE__ << " " << __func__ << endl; 
                    exit(1);
                }


                bool flag1 =  (point_query_result.first != is_exist);
                bool flag2 =  is_exist&&(point_query_result.second != it_groundtruth->second);
                if( flag1 || flag2){
                    cout << "@currently deleted keys point query test" << endl;
                    cout << "ERROR: key: " << key << " is exist: " << is_exist << " value: " << (is_exist? groundtruth[key] : "") << endl;
                    if(i == 0){
                        cout << " point_query_result: " << point_query_result.first << " " << point_query_result.second << endl; 
                    }else if(i == 1){
                        long timestamp = WorkloadRecorder::getInsertTimestamp(key);
                        cout << " point_query_result: " << point_query_result.first << " " << point_query_result.second << " " << timestamp << endl; 
                    }else if(i == 2){
                        cout << " point_query_result: " << point_query_result.first << " " << point_query_result.second << endl; 
                    }else{
                        cout << "ERROR: No such filter. Unimplemented if-else block. " << __FILE__ << " " << __LINE__ << " " << __func__ << endl; 
                        exit(1);
                    }
                    cout << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
                    exit(1);
                }
            }
            auto stop_pq = std::chrono::high_resolution_clock::now();
            auto duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
            total_elapsed_time += duration_pq.count();
        }
        cout << "point query time: " << total_elapsed_time/EXPERIMENT_REPETITION_TIMES << " microseconds" << endl;
        
        int disk_access_count = Query::getDiskSSTFileAccessCount()/EXPERIMENT_REPETITION_TIMES;
        cout << "disk access count: " << disk_access_count << endl;
        cout << endl << endl;

        disk_access_count_list.push_back(disk_access_count);
    }
    return disk_access_count_list;
}


RandomKeysTestingResult SystemVerifier::checkOnRandomKeys(int num) {
    RandomKeysTestingResult result;
    vector<int> disk_access_count_list;
    
    //generate random keys
    int count_exist_key = 0;
    int count_non_exist_key = 0;
    map<long, string> groundtruth = WorkloadRecorder::getGroundTruth();
    vector<pll> random_keys;
    for(int i_count = 0; i_count < num; i_count++){
        long key = rand() %  WorkloadGenerator::KEY_DOMAIN_SIZE;
        if(groundtruth.find(key) != groundtruth.end()){
            count_exist_key++;
        }else{
            count_non_exist_key++;
        }

        long timestamp;
        if(WorkloadRecorder::isHistoryKey(key)){
            timestamp = WorkloadRecorder::getInsertTimestamp(key);
        }else{
            timestamp = rand() % ((int)1e9);
        }
        random_keys.push_back(make_pair(key, timestamp));
    }

    //test on random keys
    for(int i = 0; i < 3; i++){
        if(i == 0){
            cout << "Perlevel Range Delete Filter:" << endl;
        }else if(i == 1){
            cout << "Skyline Range Delete Filter:" << endl;
        }else if(i == 2){
            cout << "Perlevel Split Range Delete Filter:" << endl;
        }else{
            cout << "ERROR: No such filter. Unimplemented if-else block. " << __FILE__ << " " << __LINE__ << " " << __func__ << endl; 
            exit(1);
        }
        
        cout << "test on random keys" << endl;
        cout << "exist key: " << count_exist_key << " non-exist key: " << count_non_exist_key << endl;
        cout << "number of point query: " << num << endl;

        Query::resetDiskSSTFileAccessCount();

        int64_t total_elapsed_time = 0;
        for(int j_times = 0; j_times < EXPERIMENT_REPETITION_TIMES; j_times++){
            auto start_pq = std::chrono::high_resolution_clock::now();
            for(int i_count = 0; i_count < num; i_count++){
                // long key = rand() %  WorkloadGenerator::KEY_DOMAIN_SIZE;
                pair<bool, string> point_query_result;
                long key = random_keys[i_count].first;
                if(i == 0){
                    point_query_result = Query::pointQuery_RDF(key);
                }else if(i == 1){
                    long timestamp = random_keys[i_count].second;
                    point_query_result = Query::pointQuery_RDF_SkyLine(key, timestamp);
                }else if(i == 2){
                    point_query_result = Query::pointQuery_SplitRDF(key);
                }else{
                    cout << "ERROR: No such filter. Unimplemented if-else block. " << __FILE__ << " " << __LINE__ << " " << __func__ << endl; 
                    exit(1);
                }
                
                auto it_groundtruth = groundtruth.find(key);
                bool is_exist = groundtruth.find(key) != groundtruth.end();
                bool flag1 =  (point_query_result.first != is_exist);
                bool flag2 =  is_exist&&(point_query_result.second != it_groundtruth->second);
                if( flag1 || flag2){
                    cout << "@random key point query test" << endl;                
                    cout << "ERROR: key: " << key << " is exist: " << is_exist << " value: " << (is_exist? groundtruth[key] : "") << endl;
                    if(i == 0){
                        cout << " point_query_result: " << point_query_result.first << " " << point_query_result.second << endl; 
                    }else if(i == 1){
                        long timestamp = WorkloadRecorder::getInsertTimestamp(key);
                        cout << " point_query_result: " << point_query_result.first << " " << point_query_result.second << " " << timestamp << endl; 
                    }else if(i == 2){
                        cout << " point_query_result: " << point_query_result.first << " " << point_query_result.second << endl; 
                    }else{
                        cout << "ERROR: No such filter. Unimplemented if-else block. " << __FILE__ << " " << __LINE__ << " " << __func__ << endl; 
                        exit(1);
                    }
                    cout << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
                    exit(1);
                }
            }
            auto stop_pq = std::chrono::high_resolution_clock::now();
            auto duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
            total_elapsed_time += duration_pq.count();
        }
        cout << "point query time: " << total_elapsed_time/EXPERIMENT_REPETITION_TIMES << " microseconds" << endl;

        int disk_access_count = Query::getDiskSSTFileAccessCount()/EXPERIMENT_REPETITION_TIMES;
        cout << "disk access count: " << disk_access_count << endl;
        cout << endl << endl;

        disk_access_count_list.push_back(disk_access_count);
    }
    result.disk_access_count_list = disk_access_count_list;
    result.count_exist_key = count_exist_key;
    result.count_non_exist_key = count_non_exist_key;
    return result;
}

void SystemVerifier::checkEquation() {
    //assertion
    long a1 = MemoryBuffer::current_buffer_point_entry_count;
    long a2 = WorkloadExecutor::buffer_update_count;
    long a3 = DiskMetaFile::getTotalPointEntryCount();
    long a4 = WorkloadExecutor::total_merge_deleted_point_entry_count;
    long a5 = WorkloadExecutor::total_range_deleted_point_entry_count;
    //range tombstone deletes point entry in buffer
    long a6 = WorkloadExecutor::total_buffer_range_tombstone_deleted_point_entry_count; 

    long a = WorkloadExecutor::total_insert_count;

    long b1 = MemoryBuffer::current_buffer_range_entry_count;
    long b2 = DiskMetaFile::getTotalRangeEntryCount();
    long b3 = WorkloadExecutor::total_merge_deleted_range_entry_count;
    long b4m = WorkloadExecutor::total_file_boundary_split_range_entry_count;
    
    long b = WorkloadExecutor::total_range_tombstone_count;
    assert(a1 + a2 + a3 + a4 + a5 + a6 == a);
    printf("Point entry count equation/assertion passed\n");
    assert(b1 + b2 + b3 - b4m == b);
    printf("Range entry count equation/assertion passed\n");
    cout << endl << endl;
}