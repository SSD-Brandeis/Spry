#ifndef UTILS_GEN_PQ_WORKLOAD_H
#define UTILS_GEN_PQ_WORKLOAD_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <cmath>
#include <map>
#include <set>
#include <vector>

#include "../workload/args.hxx"
// #include "../workload/workload_generator.h"
// #include "../env_settings/emu_environment.h"
// #include "rocksdb/db.h"
// #include "rocksdb/options.h"
// #include "rocksdb/advanced_options.h"
#include "../../include/rocksdb/system_verifier.h"
// #include "utils_logger_during_insertion.h"
// #include "rocksdb/SuRF/include/surf.hpp"
#include "../utils/utils_read_write.h"


using namespace std;

// template<typename KeyT>
using KeyT = string;
// void runInsertRDWorkload(const string workload_file_name){
void runInsertRDWorkload(const string workload_file_name, checking::SystemVerifier *system_verifier){
  // // opening workload file for the first time
  // const string pq_workload_all_existing_keys_file_name = workload_file_name + "_all_existing_keys";
  // const string pq_workload_historic_existing_keys_file_name = workload_file_name + "_historic_existing_keys";
  // const string pq_workload_currently_deleted_keys_file_name = workload_file_name + "_currently_deleted_keys";
  // const string pq_workload_currently_non_inserted_keys_file_name = workload_file_name + "_currently_non_inserted_keys";
  // std::ofstream pq_workload_all_existing_keys_file;
  // std::ofstream pq_workload_historic_existing_keys_file;
  // std::ofstream pq_workload_currently_deleted_keys_file;
  // std::ofstream pq_workload_currently_non_inserted_keys_file;
  // pq_workload_all_existing_keys_file.open(pq_workload_all_existing_keys_file_name);
  // pq_workload_historic_existing_keys_file.open(pq_workload_historic_existing_keys_file_name);
  // pq_workload_currently_deleted_keys_file.open(pq_workload_currently_deleted_keys_file_name);
  // pq_workload_currently_non_inserted_keys_file.open(pq_workload_currently_non_inserted_keys_file_name);
  // assert(pq_workload_all_existing_keys_file);
  // assert(pq_workload_historic_existing_keys_file);
  // assert(pq_workload_currently_deleted_keys_file);
  // assert(pq_workload_currently_non_inserted_keys_file);


  std::ifstream workload_file;
  workload_file.open(workload_file_name);
  assert(workload_file);


  // checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();
  system_verifier->resetRunningPQ();
  int KEY_SIZE = checking::SystemVerifier::getSystemVerifier()->getKeySize();
  int flag_using_string_key = checking::SystemVerifier::getSystemVerifier()->usingStringKey();

  std::cout << "KEY_SIZE = " << KEY_SIZE << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
  std::cout << "flag_using_string_key = " << flag_using_string_key << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  long long i_instruction = 0;

  // auto start_time = std::chrono::high_resolution_clock::now();
  // auto stop_time = std::chrono::high_resolution_clock::now();
  // auto duration_time = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_time - start_time);
  // unsigned long long gen_PQ_workload_time_ns = 0;

  while (!workload_file.eof()) {
    i_instruction ++;

    char instruction;
    // std::string time_stamp;
    // std::stringstream ss_time_stamp;
    KeyT key, start_key, end_key;
    std::string type;
    std::string value;    
    std::stringstream ss_key, ss_start_key, ss_end_key;
    // std::stringstream ss_key, ss_start_key, ss_end_key;
    // size_t separator_pos = 0;
    workload_file >> instruction;
    if(workload_file.eof()){break;}
    // std::cout << "instruction = " << instruction << std::endl;

    switch (instruction) {
      case 'I':  // insert
        workload_file >> key >> value;
        // std::cout << "I key = " << key << " value = " << value << std::endl;
        if(flag_using_string_key == 0){
          ss_key << std::setfill('0') << std::setw(KEY_SIZE) << key;
          key = ss_key.str();
        }

        system_verifier->insert(key, value);
        break;

      case 'Q':  // probe: point query
        workload_file >> key;
        if(flag_using_string_key == 0){
          ss_key << std::setfill('0') << std::setw(KEY_SIZE) << key;
          key = ss_key.str();
        }
        break;

      case 'S':  // scan: range query
        workload_file >> start_key >> end_key;
        if(flag_using_string_key == 0){
          ss_start_key << std::setfill('0') << std::setw(KEY_SIZE) << start_key;
          ss_end_key << std::setfill('0') << std::setw(KEY_SIZE) << end_key;
          start_key = ss_start_key.str();
          end_key = ss_end_key.str();
        }
        break;

      case 'D':  // delete
        workload_file >> type >> start_key >> end_key;
        if(flag_using_string_key == 0){
          ss_start_key << std::setfill('0') << std::setw(KEY_SIZE) << start_key;
          ss_end_key << std::setfill('0') << std::setw(KEY_SIZE) << end_key;
          start_key = ss_start_key.str();
          end_key = ss_end_key.str();
        }

        if (type == "Range") {
          system_verifier->rangeDelete(start_key, end_key);
        } else {
          break;
        }
        break;
      
      case 'R':
        workload_file >> start_key >> end_key;
        if(flag_using_string_key == 0){
          ss_start_key << std::setfill('0') << std::setw(KEY_SIZE) << start_key;
          ss_end_key << std::setfill('0') << std::setw(KEY_SIZE) << end_key;
          start_key = ss_start_key.str();
          end_key = ss_end_key.str();
        }

        system_verifier->rangeDelete(start_key, end_key);

        break;

      default:
        std::cerr << "ERROR: Case match NOT found !!" << std::endl;
        std::cerr << "instruction = " << instruction << std::endl;
        std::cout << "ERROR: Case match NOT found !!" << std::endl;
        std::cout << "instruction = " << instruction << std::endl;
        break;
    }
  }

  workload_file.close();

  // stop_time = std::chrono::high_resolution_clock::now();
  // duration_time = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_time - start_time);
  // gen_PQ_workload_time_ns = duration_time.count();
  // std::cout << "gen_PQ_workload_time_ns = " << gen_PQ_workload_time_ns << std::endl;



  return;
}


struct EnvGenPQ{
  // int number_of_PQs = 5000;
  int number_of_PQs_on_existing_keys = 5000;
  int number_of_PQs_on_historic_existing_keys = 5000;
  int number_of_PQs_on_currently_deleted_keys = 5000;
  int number_of_PQs_on_currently_non_inserted_keys = 5000;
  
  std::string workload_file_name = "";
};


int parse_arguments(int argc, char *argv[], checking::SystemVerifier *system_verifier, EnvGenPQ* env_gen_pq) {
  args::ArgumentParser parser("RocksDB_parser.", "");

  args::Group group1(parser, "This group is all exclusive:", args::Group::Validators::DontCare);
  //YuCheng Added Start
  // args::ValueFlag<int> number_of_PQ_cmd(group1, "number_of_PQ", "number_of_PQ [def:5000]", {"number_of_PQ"});
  args::ValueFlag<int> number_of_PQ_on_existing_keys_cmd(group1, "number_of_PQ_on_existing_keys", "number_of_PQ_on_existing_keys [def:5000]", {"number_of_PQ_on_existing_keys"});
  args::ValueFlag<int> number_of_PQ_on_historic_existing_keys_cmd(group1, "number_of_PQ_on_historic_existing_keys", "number_of_PQ_on_historic_existing_keys [def:5000]", {"number_of_PQ_on_historic_existing_keys"});
  args::ValueFlag<int> number_of_PQ_on_currently_deleted_keys_cmd(group1, "number_of_PQ_on_currently_deleted_keys", "number_of_PQ_on_currently_deleted_keys [def:5000]", {"number_of_PQ_on_currently_deleted_keys"});
  args::ValueFlag<int> number_of_PQ_on_currently_non_inserted_keys_cmd(group1, "number_of_PQ_on_currently_non_inserted_keys", "number_of_PQ_on_currently_non_inserted_keys [def:5000]", {"number_of_PQ_on_currently_non_inserted_keys"});

  args::ValueFlag<int> using_string_key_cmd(group1, "using_string_key", "Using string key [def: 0]", {"using_string_key", "USING_STRING_KEY"});

  args::ValueFlag<string> workload_filename_cmd(group1, "workload_filename", "workload filename [def:0.001]", {"workload_filename"});
  
  args::ValueFlag<double> key_size_to_insert_cmd(group1, "key_size_to_insert", "key_size_to_insert [def: 12]", {"key_size_to_insert"});
  //YuCheng Added End

  try {
      parser.ParseCLI(argc, argv);
  }
  catch (args::Help&) {
      std::cout << parser;
      exit(0);
      // return 0;
  }
  catch (args::ParseError& e) {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
  }
  catch (args::ValidationError& e) {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
  }
  // int number_of_PQs = number_of_PQ_cmd ? args::get(number_of_PQ_cmd) : 5000;
  int number_of_PQs_on_existing_keys = number_of_PQ_on_existing_keys_cmd ? args::get(number_of_PQ_on_existing_keys_cmd) : 5000;
  int number_of_PQs_on_historic_existing_keys = number_of_PQ_on_historic_existing_keys_cmd ? args::get(number_of_PQ_on_historic_existing_keys_cmd) : 5000;
  int number_of_PQs_on_currently_deleted_keys = number_of_PQ_on_currently_deleted_keys_cmd ? args::get(number_of_PQ_on_currently_deleted_keys_cmd) : 5000;
  int number_of_PQs_on_currently_non_inserted_keys = number_of_PQ_on_currently_non_inserted_keys_cmd ? args::get(number_of_PQ_on_currently_non_inserted_keys_cmd) : 5000;

  string workload_file_name = workload_filename_cmd ? args::get(workload_filename_cmd) : "workload.txt";
  // env_gen_pq->number_of_PQs = number_of_PQs;
  env_gen_pq->number_of_PQs_on_existing_keys = number_of_PQs_on_existing_keys;
  env_gen_pq->number_of_PQs_on_historic_existing_keys = number_of_PQs_on_historic_existing_keys;
  env_gen_pq->number_of_PQs_on_currently_deleted_keys = number_of_PQs_on_currently_deleted_keys;
  env_gen_pq->number_of_PQs_on_currently_non_inserted_keys = number_of_PQs_on_currently_non_inserted_keys;
  env_gen_pq->workload_file_name = workload_file_name;

  
  int key_size_to_insert = key_size_to_insert_cmd ? args::get(key_size_to_insert_cmd) : 12;
  system_verifier->setKeySize(key_size_to_insert);

  int using_string_key = using_string_key_cmd ? args::get(using_string_key_cmd) : 0;
  system_verifier->setFlagUsingStringKey(using_string_key);

  return 0;
}

#endif  // UTILS_GEN_PQ_WORKLOAD_H