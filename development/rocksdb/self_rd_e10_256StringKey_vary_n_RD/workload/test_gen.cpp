/*
 *  Created on: May 13, 2019
 *  Author: Subhadeep, Papon
 */


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

#include "args.hxx"
// #include "emu_environment.h"
// #include "tree_builder/tree_builder.h"
// #include "workload_executor.h"
#include "workload_generator.h"
// #include "query_runner.h"

// #include "instruction_decoder.h"
// #include "checking/checking.h"

using namespace std;
// using namespace tree_builder;
// using namespace workload_exec;






/*
 * DECLARATIONS
*/
// int Query::delete_key;
// int Query::range_start_key;
// int Query::range_end_key;
// int Query::sec_range_start_key;
// int Query::sec_range_end_key;
// int Query::iterations_point_query;

//long inserts(EmuEnv* _env);
int parse_arguments2(int argc, char *argvx[]);
void printEmulationOutput();
void calculateDeleteTileSize();

int runWorkload();


// int parse_arguments2(int argc, char *argvx[], EmuEnv* _env);
// void printEmulationOutput(EmuEnv* _env);
// void calculateDeleteTileSize(EmuEnv* _env);

// int runWorkload(EmuEnv* _env);



void showProgress_w(const uint32_t &workload_size, const uint32_t &counter) {
    if (counter / (max(workload_size,(uint32_t)100)/100) >= 1) {
      for (int i = 0; i<104; i++){
        std::cout << "\b";
        fflush(stdout);
      }
    }

    for (int i = 0; i<counter / (max(workload_size,(uint32_t)100)/100); i++){
      std::cout << "=" ;
      fflush(stdout);
    }

    std::cout << std::setfill(' ') << std::setw(101 - counter / (max(workload_size,(uint32_t)100)/100));
    std::cout << counter*100/workload_size << "%";
      fflush(stdout);

  if (counter == workload_size) {
    std::cout << "\n";
    return;
  }
}


int main(int argc, char *argvx[]) {


  args::ArgumentParser parser("RDF Emulator", "");

  args::Group group1(parser, "This group is all exclusive:", args::Group::Validators::DontCare);


  args::ValueFlag<int> entry_size_cmd(group1, "E", "Entry size in bytes [def: 128 B]", {'E', "entry_size"});
  args::ValueFlag<double> cor_cmd(group1, "#correlation", "Correlation between sort key and delete key [def: 0]", {'c', "correlation"});
  args::ValueFlag<long long> num_inserts_cmd(group1, "#inserts", "The number of unique inserts to issue in the experiment [def: 0]", {'i', "num_inserts"});
  args::ValueFlag<int> RD_cmd(group1, "range_delete", "Count of range delete [def:1]", {'R', "RD"});
  args::ValueFlag<double> selectivity_cmd(group1, "selectivity_of_range_delete", "Selectivity of range delete [def:0.001]", {"z", "selectivity"});
  args::ValueFlag<string> workload_filename_cmd(group1, "workload_filename", "workload filename [def:0.001]", {"f", "workload_filename"});
  args::ValueFlag<double> insert_before_range_delete_cmd(group1, "insert_before_range_delete", "percent of inserts in insert themself that precede any of the range delete [def:0.5]", {"b", "insert_before_range_delete"});




  try {
      parser.ParseCLI(argc, argvx);
  } catch (args::Help&) {
      std::cout << parser;
      return 0;
  } catch (args::ParseError& e) {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
  } catch (args::ValidationError& e) {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
  }



  int entry_size = entry_size_cmd ? args::get(entry_size_cmd) : 128;
  double correlation = cor_cmd ? args::get(cor_cmd) : 0;
  long long num_inserts = num_inserts_cmd ? args::get(num_inserts_cmd) : 0;
  int rd_count = RD_cmd ? args::get(RD_cmd) : 1;
  double selectivity = selectivity_cmd ? args::get(selectivity_cmd) : 0.001;
  string workloadFilename = workload_filename_cmd ? args::get(workload_filename_cmd) : "workload.txt";
  double insertBeforeRangeDelete = insert_before_range_delete_cmd ? args::get(insert_before_range_delete_cmd) : 0.5;

  // check emu_environment.h for the contents of EmuEnv and also the definitions of the singleton experimental environment 
  // EmuEnv* _env = EmuEnv::getInstance();
  //parse the command line arguments
  // if (parse_arguments2(argc, argvx, _env)){
  // if (parse_arguments2(argc, argvx)){
  //   exit(1);
  // }

cout << "sizeof(int) = " << sizeof(int) << endl;
cout << "sizeof(long) = " << sizeof(long) << endl;
cout << "sizeof(long long) = " << sizeof(long long) << endl;
 
  
    
  WorkloadGenerator workload_generator;
cout << "num_inserts = " << num_inserts << endl;
cout << "rd_count = " << rd_count << endl;
cout << "selectivity = " << selectivity << endl;
cout << "insertBeforeRangeDelete = " << insertBeforeRangeDelete << endl;
  long numberOfPointInTheBeginning = (long) ceil(num_inserts * insertBeforeRangeDelete);
  // string workloadFilename = _workloadFilename;
cout << "num_inserts*1.0 = " << num_inserts*1.0 << endl;
cout << "rd_count*selectivity = " << rd_count*selectivity << endl; 
  assert(num_inserts*1.0 >= 1.0*rd_count*selectivity);
  workload_generator.generateWorkload((long)num_inserts, (long)entry_size, (double) correlation, 
          (long)rd_count, (double) selectivity, (long) numberOfPointInTheBeginning, (string) workloadFilename);    

  std::cout << "Workload Generated!" << std::endl;


  return 0;
}

