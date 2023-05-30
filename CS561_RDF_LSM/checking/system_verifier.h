/*
 *  Created on: April 23, 2023
 *  Author: yucheng
 */


#ifndef SYSTEM_VERIFIER_H_
#define SYSTEM_VERIFIER_H_


namespace checking {
  class SystemVerifier;
} 


#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <assert.h>
#include <vector>
#include <map>
#include <set>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include "../emu_environment.h"
#include "../workload_executor.h"
#include "../workload/workload.h"
#include "../query_runner.h"



using namespace std;
// using namespace workload_exec;

namespace checking {

  struct RandomKeysTestingResult{
      public:
          vector<int> disk_access_count_list;
          int count_exist_key;
          int count_non_exist_key;
  };

  class SystemVerifier {
  private:

  public:
    // WorkloadRecorder();
    const static int EXPERIMENT_REPETITION_TIMES = 3;

    static vector<int> checkOnExistingKeys();
    static vector<int> checkOnAllInsertedKeys();
    static vector<int> checkOnAllCurrentlyDeletedKeys();
    static RandomKeysTestingResult checkOnRandomKeys(int num);
    static void checkEquation();
  };


} // namespace

#endif /* SYSTEM_VERIFIER_H_ */
