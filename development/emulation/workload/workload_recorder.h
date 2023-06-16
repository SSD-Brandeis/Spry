/*
 *  Created on: April 23, 2023
 *  Author: yucheng
 */


#ifndef WORKLOAD_RECORDER_H_
#define WORKLOAD_RECORDER_H_
// #endif /* WORKLOAD_RECORDER_H_ */


namespace workload {
  class WorkloadRecorder;
} 


#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <vector>
#include <map>
#include <set>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include "../emu_environment.h"
#include "../workload_executor.h"


#include "../tree_builder/delete_tile.h"
#include "../tree_builder/disk_meta_file.h"
#include "../tree_builder/memory_buffer.h"
#include "../tree_builder/page.h"
#include "../tree_builder/sst_file.h"




using namespace std;

namespace workload {
  using pll = pair<long, long>;

  class WorkloadRecorder {
  private:
    static map<long, string> ground_truth;  
    static set<long> history_key_set; 
    static map<long, long> insert_timestamp;


  public:
    
    static void insert(long key, string value, long timestamp);
    static void remove(long key);
    static void remove(long startKey, long endKey);

    static bool isExist(long key);
    static string getValue(long key);

    static map<long, string> getGroundTruth();
    static void printGroundTruth();

    static set<long> getHistoryKeySet();
    static bool isHistoryKey(long key);

    static long getInsertTimestamp(long key);

    static long getCurrentExistingPointEntryCount();
    static vector<pll> getNRangesOfSelectivity(int numN, double selectivity, int totPoint);

  };
} // namespace

#endif /* WORKLOAD_RECORDER_H_ */
