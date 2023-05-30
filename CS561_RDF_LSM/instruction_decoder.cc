#include "instruction_decoder.h"

// using namespace std;
// using namespace workload_exec;

 void InstructionDecoder::decode(string &instruction, WorkloadExecutor &workload_executer){
    stringstream ss(instruction);
    string type;
    ss >> type;
    if(type == "I"){
      //insert
      long sortkey;
      long deletekey;
      string value;
      ss >> sortkey >> deletekey >> value;
      workload_executer.insert(sortkey, deletekey, value, this->getInstructionSequenceNumber());
      this->incInstructionSequenceNumber();

      WorkloadRecorder::insert(sortkey, value, this->getInstructionSequenceNumber());
      return;
    }else if(type == "RD"){
      long startKey;
      long endKey;
      ss >> startKey >> endKey;
      workload_executer.rangeDeleteOnPrimaryKey(startKey, endKey, this->getInstructionSequenceNumber());
      this->incInstructionSequenceNumber();

      WorkloadRecorder::remove(startKey, endKey);
      
      return ;
    }
    
    std::cerr << "Unknown instruction type: " << type << std::endl;
    std::abort();
 }

 void InstructionDecoder::incInstructionSequenceNumber(){
    instructionSequenceNumber++;
 }

 long InstructionDecoder::getInstructionSequenceNumber(){
    return instructionSequenceNumber;
 }