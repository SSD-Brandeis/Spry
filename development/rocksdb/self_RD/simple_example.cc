#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <chrono>
#include <regex>
#include <thread>
#include <cstdlib>


// #include "rocksdb/dbformat.h"
// #include "file/writable_file_writer.h"
// #include "options/cf_options.h"


#include "rocksdb/statistics.h"
#include "rocksdb/cache.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/advanced_options.h"
#include "rocksdb/slice.h"
#include "rocksdb/table.h"

#include "rocksdb/filter_policy.h"
#include "rocksdb/perf_context.h" // !YBS-sep01-XX!
#include "rocksdb/iostats_context.h" // !YBS-sep01-XX!

#include "rocksdb/system_verifier.h"

#include "rocksdb/iostats_context.h"
#include "rocksdb/perf_context.h"

#include "workload/args.hxx"
// #include "workload/workload_generator.h"
#include "workload/workload_generator.cc"
#include "env_settings/emu_environment.cc"


// #include "rocksdb/column_family.h"

// #include "rocksdb/util/cast_util.h"
#include "rocksdb/sys_rdfilter.h"
#include "rocksdb/slice.h"

#include "utils/utils.h"
#include "utils/utils_log.h"
#include "utils/utils_gen_workload.h"
#include "utils/utils_run_workload.h"
#include "utils/utils_run_verification.h"
#include "utils/utils_db.h"
#include "utils/utils_rdf.h"

// std::mutex rdfilter::PLRDF::init_mutex;
// rdfilter::PLRDF* rdfilter::PLRDF::plrdf_ptr; 



using namespace rocksdb;
// std::string kDBPath = "/tmp/cs561_project1";
// std::string kDBPath = "/home/tan/cs561_project1";

// void printStats(DB* db, Options& options);
// void print_perf_iostats_context(std::ostream& ofile, int N_repetitions = 1);
// void init(DB **db_ptr2, Options& op, WriteOptions& write_op, ReadOptions& read_op, int max_background_jobs);
// long long parsing_value_from_string(std::string str, std::string pattern);
// void reset_perf_iostats_context();


int main(int argc, char *argv[]) {
  // check emu_environment.h for the contents of EmuEnv and also the definitions of the singleton experimental environment 
  EmuEnv* _env = EmuEnv::getInstance();
  //parse the command line arguments
  if (parse_arguments2(argc, argv, _env)){
    exit(1);
  }



  // speed_test();
  Options options;
  WriteOptions write_op;
  ReadOptions read_op;
  BlockBasedTableOptions table_options;
  FlushOptions f_options;

  DB* db;
  DB** db_ptr2 = &db;

  int max_background_jobs = 1;
  if(_env->gen_workload == true){
    gen_workload(_env);
  }

  std::string kDBPathBase = "/tmp/cs561_project1";
  std::string kDBPath = getkDBPath(_env, kDBPathBase);
  // init(db_ptr2, options, write_op, read_op, max_background_jobs, _env);
  init(db_ptr2, options, write_op, read_op, max_background_jobs, _env, kDBPath);

  configOptions(_env, &options, &table_options, &write_op, &read_op, &f_options);

  // start(db_ptr2, options, write_op, read_op, _env);
  start(db_ptr2, options, write_op, read_op, _env, kDBPath);
  // runWorkload(db, options, write_op, read_op, _env);
  runWorkload(db_ptr2, options, write_op, read_op, _env, kDBPath);

  
  PLRDF plrdf_prime, split_plrdf_prime;
  PLRDF top_level_rdf_prime;
  std::vector<t3ll> skyline_rdf_prime;
  std::vector<int> skyline__numbers_of_ranges_in_rdf_log;
  // retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
  // {
  //   retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
  //   reopen_DB(db_ptr2, options, write_op, read_op, _env, kDBPath);
  //   set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
  // }

  reset_perf_iostats_context();
  

  std::this_thread::sleep_for(std::chrono::seconds(10));  // Sleep for 1 second
  {
    // std::clog << "Press Enter to continue...";
    // std::cin.ignore(); // Waits for user to press Enter key
    verification_runner::initPQVerification(db_ptr2, read_op, _env);
    
    std::cout << "!!! runQPVerification start " << std::endl;

    // verification_runner::runPQVerification(db_ptr2, options, write_op, read_op, _env);
    int numbers_of_PQs = -1;
    verification_runner::runPQVerification(db_ptr2, options, write_op, read_op, _env, numbers_of_PQs, kDBPath);
    numbers_of_PQs = 5000;
    verification_runner::runPQVerification(db_ptr2, options, write_op, read_op, _env, numbers_of_PQs, kDBPath);
    
    verification_runner::endPQVerification();
  }

  set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);

  std::cout << "!!! runQPVerification done " << std::endl;
  
  print_perf_iostats_context(std::cout, 1);


  end(db_ptr2);
}
