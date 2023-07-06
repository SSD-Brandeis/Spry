/*
 *  Created on: April 23, 2023
 *  Author: yucheng
 */


#ifndef SYSTEM_VERIFIER_H_
#define SYSTEM_VERIFIER_H_


namespace rdfilter {
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
// #include "../emu_environment.h"
// #include "../workload_executor.h"
// #include "../workload/workload.h"
// #include "../query_runner.h"

// #include "system_verifier.cc"



using namespace std;
// using namespace workload_exec;

namespace rdfilter {
  class PLRDF {
    private:
      static const int KEY_SIZE = 12;

    public:
      static PLRDF* plrdf_ptr;

    
    static void init(){
      if(plrdf_ptr == NULL){
        plrdf_ptr = new PLRDF();
      }
    }


} // namespace





#endif /* SYSTEM_VERIFIER_H_ */



