#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <chrono>
#include <regex>
#include <thread>
#include <cstdlib>

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
#include "workload/workload_generator.cc"
#include "env_settings/emu_environment.cc"


#include "rocksdb/sys_rdfilter.h"
#include "rocksdb/slice.h"

#include "utils/utils.h"
#include "utils/utils_log.h"
#include "utils/utils_gen_workload.h"
#include "utils/utils_run_workload.h"
#include "utils/utils_run_verification.h"
#include "utils/utils_db.h"
#include "utils/utils_rdf.h"

#include "rocksdb/SuRF/include/surf.hpp"
#include "rocksdb/system_verifier.h"

using namespace rocksdb;
std::string kDBPath = "/tmp/cs561_project1";

void printStats(DB* db, Options& options);
void print_perf_iostats_context(std::ostream& ofile, int N_repetitions = 1);
void init(DB **db_ptr2, Options& op, WriteOptions& write_op, ReadOptions& read_op, int max_background_jobs);
long long parsing_value_from_string(std::string str, std::string pattern);
void reset_perf_iostats_context();


int main(int argc, char *argv[]) {
  
  auto start_all = std::chrono::high_resolution_clock::now();

  // check emu_environment.h for the contents of EmuEnv and also the definitions of the singleton experimental environment 
  EmuEnv *_env = EmuEnv::getInstance();
  surf::SuRF_Env *_surf_env = surf::SuRF_Env::getInstance();
  checking::SystemVerifier *system_verifier = checking::SystemVerifier::getSystemVerifier();
  //parse the command line arguments
  if (parse_arguments2(argc, argv, _env, _surf_env, system_verifier)){
    exit(1);
  }
  
  checking::SystemVerifier::getSystemVerifier()->setSkipReadingRangeDeleteBlock(_env->skip_reading_RD_blocks);
  
  Options options;
  WriteOptions write_op;
  ReadOptions read_op;
  BlockBasedTableOptions table_options;
  FlushOptions f_options;

  DB* db;
  DB** db_ptr2 = &db;

  int max_background_jobs = 1;
  if(_env->gen_workload == true){
    auto start_wg = std::chrono::high_resolution_clock::now();
    gen_workload(_env);
    auto stop_wg = std::chrono::high_resolution_clock::now();
    auto duration_wg = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_wg - start_wg);
    unsigned long long workload_generation_time_ns = duration_wg.count();
    std::cout << "workload_generation_time_ns = " << workload_generation_time_ns << std::endl;
  }

  std::string kDBPathBase = "/tmp/cs561_project1";
  std::string kDBPath = getkDBPath(_env, kDBPathBase);
  init(db_ptr2, options, write_op, read_op, max_background_jobs, _env, kDBPath);

  configOptions(_env, &options, &table_options, &write_op, &read_op, &f_options);
  std::cout << "table_op->filter_policy = " << table_options.filter_policy << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  start(db_ptr2, options, write_op, read_op, _env, kDBPath);
  runWorkload(db_ptr2, options, write_op, read_op, _env, kDBPath);

  
  PLRDF plrdf_prime, split_plrdf_prime;
  PLRDF top_level_rdf_prime;
  SkyLineRDF skyline_rdf_prime;

  reset_perf_iostats_context();
  
  std::string out, out2, out3, out4;
  (*db_ptr2)->GetProperty("rocksdb.block-cache-usage", &out);
  (*db_ptr2)->GetProperty("rocksdb.estimate-table-readers-mem", &out2);
  (*db_ptr2)->GetProperty("rocksdb.block-cache-capacity", &out3);
  (*db_ptr2)->GetProperty("rocksdb.size-all-mem-tables", &out4);
  cout << "rocksdb.block-cache-usage: " << out << endl;
  cout << "rocksdb.estimate-table-readers-mem: " << out2 << endl;
  cout << "rocksdb.block-cache-capacity: " << out3 << endl;
  cout << "rocksdb.size-all-mem-tables: " << out4 << endl;



  std::this_thread::sleep_for(std::chrono::seconds(10));  // Sleep for 1 second
  {
    verification_runner::initPQVerification(db_ptr2, read_op, _env);
    
    std::cout << "!!! runQPVerification start " << std::endl;

    if(_env->system_check_test_on_all_PQ == true){
      int numbers_of_PQs = -1;
      verification_runner::runPQVerification(db_ptr2, options, write_op, read_op, _env, numbers_of_PQs, kDBPath);
    }
    {
      int numbers_of_PQs = _env->number_of_PQ;
      verification_runner::runPQVerification(db_ptr2, options, write_op, read_op, _env, numbers_of_PQs, kDBPath);
    }
    verification_runner::endPQVerification();
  }
  
  auto stop_all = std::chrono::high_resolution_clock::now();
  auto duration_all = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_all - start_all);
  unsigned long long all_time_ns = duration_all.count();
  std::cout << "all_time_ns = " << all_time_ns << std::endl;
  
  print_perf_iostats_context(std::cout, 1);

  set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime);

  std::cout << "!!! runQPVerification done " << std::endl;
  


  end(db_ptr2);
}
