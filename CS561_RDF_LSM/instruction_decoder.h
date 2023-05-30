/*
 *  Created on: May 13, 2019
 *  Author: Subhadeep
 */

#ifndef INSTRUCTION_DECODER_H_
#define INSTRUCTION_DECODER_H_
 

#include <iostream>
#include "workload_executor.h"
#include <sstream>
#include <iostream>
#include <cstdio>
#include <sys/time.h>
#include <cmath>
#include <unistd.h>
#include <assert.h>
#include <fstream>
#include <stdlib.h>
#include <cstdlib>
#include <iomanip>

#include "workload/workload.h"

using namespace std;
using namespace workload_exec;
using namespace workload;


class InstructionDecoder
{
private:
  long instructionSequenceNumber = 0;
  

public:
  void decode(string &instruction, WorkloadExecutor &workloadExecutor);
  void incInstructionSequenceNumber();
  long getInstructionSequenceNumber();

};

#endif /*INSTRUCTION_DECODER_H_*/

