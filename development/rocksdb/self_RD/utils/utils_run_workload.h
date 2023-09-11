#ifndef UTILS_RUN_WORKLOAD_H
#define UTILS_RUN_WORKLOAD_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "../workload/args.hxx"
// #include "workload/workload_generator.h"
#include "../workload/workload_generator.h"
#include "../env_settings/emu_environment.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/advanced_options.h"
#include "rocksdb/system_verifier.h"
// #include "utils_run_verification.h"


void runWorkload(DB** db_ptr2, Options& op, WriteOptions& write_op, ReadOptions& read_op, 
                 EmuEnv* _env, std::string kDBPath){
                //  Params &params) {
  DB* db = *db_ptr2;

  string &workload_file_name = _env->workload_file_name;

  Status s;

  // opening workload file for the first time
  std::ifstream workload_file;
  // workload_file.open("workload.txt");
  workload_file.open(workload_file_name);
  assert(workload_file);
  // doing a first pass to get the workload size
  uint64_t workload_size = 0;
  std::string line;
  while (std::getline(workload_file, line)) ++workload_size;
  workload_file.close();

  // workload_file.open("workload.txt");
  workload_file.open(workload_file_name);
  assert(workload_file);

  checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();
  system_verifier->resetRunningPQ();

  Iterator* it = db->NewIterator(read_op);  // for range reads
  uint64_t counter = 0;                     // for progress bar
  int KEY_SIZE = checking::SystemVerifier::getKeySize();
  int TIME_STAMP_SIZE = 7;  // shall == rocksdb sequence num 
  long long i_instruction = 0;

  while (!workload_file.eof()) {
    i_instruction ++;
    // std::cout << " i_instruction = " << i_instruction << std::endl;
    // while(db->getFlushQueueSize() > 0 || db->getCompactionQueueSize() > 0) {
    //   std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // }
    // while(db->getFlushQueueSize() > 0) {
    // while(db->existFlushJob() == true){
    //   std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // }
    while(db->existFlushJob() == true || db->existCompactionJob() == true){
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }


    char instruction;
    std::string time_stamp;
    std::stringstream ss_time_stamp;
    long long key, start_key, end_key;
    std::string type;
    std::string value;
    std::stringstream ss_key, ss_start_key, ss_end_key;
    size_t separator_pos = 0;
    workload_file >> instruction;
    switch (instruction) {
      case 'I':  // insert
        workload_file >> key >> value;

        system_verifier->insert(key, value);

        // std::cout << "Insert " << key << std::endl;
        ss_key << std::setfill('0') << std::setw(KEY_SIZE) << key;
        ss_time_stamp << std::setfill('0') << std::setw(TIME_STAMP_SIZE) << i_instruction;
        // std::cout << "Insert " << ss_key.str() << " time_stamp = " << ss_time_stamp.str() << endl;
        // std::cout << "Insert " <<  ss_key.str() << std::endl;
        // Put key-value
        // s = db->Put(write_op, ss_key.str(), value);
        s = db->Put(write_op, ss_key.str(), value + "|" + ss_time_stamp.str());
        // s = db->Put(write_op, ss_key.str(), value, Slice(std::to_string(i_instruction)));
        if (!s.ok()) std::cerr << s.ToString() << std::endl;
        assert(s.ok());
        counter++;
        break;

      case 'Q':  // probe: point query
        workload_file >> key;

        // bool gt_is_exist = system_verifier->isKeyExist(key);
        // std::string gt_value = system_verifier->get(key);

        std::cout << "Query " << key << std::endl;
        ss_key << std::setfill('0') << std::setw(KEY_SIZE) << key;
        s = db->Get(read_op, ss_key.str(), &value);
        separator_pos = value.find("|");
        time_stamp = value.substr(separator_pos + 1);
        value = value.substr(0, separator_pos);
        // if (!s.ok()) std::cerr << s.ToString() << "key = " << key <<
        // std::endl;
        //  assert(s.ok());
        counter++;
        break;

      case 'S':  // scan: range query
        workload_file >> start_key >> end_key;


        it->Refresh();
        assert(it->status().ok());
        ss_start_key << std::setfill('0') << std::setw(KEY_SIZE) << start_key;
        ss_end_key << std::setfill('0') << std::setw(KEY_SIZE) << end_key;
        for (it->Seek(ss_start_key.str()); it->Valid(); it->Next()) {
          // std::cout << "found key = " << it->key().ToString() << std ::endl;
          if (it->key().ToString() == ss_end_key.str()) {
            break;
          }
        }
        if (!it->status().ok()) {
          std::cerr << it->status().ToString() << std::endl;
        }
        counter++;
        break;

      case 'D':  // delete
        workload_file >> type >> start_key >> end_key;
        if (type == "Range") {
          
          FlushOptions flush_opts;
          s = db->Flush(flush_opts);
          while(db->existFlushJob() == true){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }
          system_verifier->rangeDelete(start_key, end_key);
          while(db->existFlushJob() == true){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }
          s = db->Flush(flush_opts);


          ss_start_key << std::setfill('0') << std::setw(KEY_SIZE) << start_key;
          ss_end_key << std::setfill('0') << std::setw(KEY_SIZE) << end_key;
          s = db->DeleteRange(write_op, db->DefaultColumnFamily(),
                              ss_start_key.str(), ss_end_key.str());
          // s = db->DeleteRange(write_op, db->DefaultColumnFamily(),
          //                     ss_start_key.str(), ss_end_key.str(), 
          //                     Slice(std::to_string(i_instruction)));
          if (!s.ok()) std::cerr << s.ToString() << std::endl;
          assert(s.ok());
          counter++;
        } else {
          std::cerr << "ERROR: Case match NOT found !!" << std::endl;
          std::cerr << "instruction = " << instruction << std::endl;
          std:cerr << "type = " << type << std::endl;
          std::cerr << "start_key = " << start_key << std::endl;
          std::cerr << "end_key = " << end_key << std::endl;
          break;
        }
        break;

      default:
        std::cerr << "ERROR: Case match NOT found !!" << std::endl;
        std::cerr << "instruction = " << instruction << std::endl;
        std::cout << "ERROR: Case match NOT found !!" << std::endl;
        std::cout << "instruction = " << instruction << std::endl;
        break;
    }

    if (workload_size < 100) workload_size = 100;
    if (counter % (workload_size / 100) == 0) {
      showProgress(workload_size, counter);
    }
  }


  std::cout << "!!! Final Flush. (Manually Flush) " << std::endl;

  while(db->existFlushJob() == true){
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  
  FlushOptions flush_opts;
  s = db->Flush(flush_opts);
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());
  // Status s = db->Flush(flush_opts, {db->DefaultColumnFamily()});

  std::cout << "!!! Insertion Workload Ends." << std::endl;

  std::this_thread::sleep_for(std::chrono::seconds(10));  // Sleep for 10 second

  std::cout << "!!! After sleep." << std::endl;


  db->printAllFileRanges();


  printStats(db, op);



  s = db->SetOptions({{"disable_auto_compactions", "true"}}); // is there any compaction happended after this????
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());
  std::cout << "!!! Disable auto compaction" << std::endl; 

  


  std::this_thread::sleep_for(std::chrono::seconds(10));  // Sleep for 10 second

  db->printAllFileRanges();

  printStats(db, op);

  uint num_SST_files = db->getTotalNumberOfSSTFiles();
  std::cout << "!!! Number of SST files = " << num_SST_files << std::endl;



  {
    std::vector<long long> testing_key_list({2500, 5000, 5001});
    long long total_read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
          + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
    long long total_read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
          + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
    // rocksdb::get_perf_context()->Reset();
    // rocksdb::get_iostats_context()->Reset();
    reset_perf_iostats_context();


    for(auto &x: testing_key_list){
      bool gt_is_exist = system_verifier->isKeyExist(x);
      std::string gt_value = system_verifier->get(x);

      std::string value;
      std::string time_stamp;
      std::stringstream searching_key;
      searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;
      s = db->Get(read_op, searching_key.str(), &value);
      size_t separator_pos = value.find("|");
      time_stamp = value.substr(separator_pos + 1);
      value = value.substr(0, separator_pos);
      std::cout << x << " " << s.ok() << " " << value << std::endl;
      std::cout << x << " " << gt_is_exist << " " << gt_value << std::endl;
    
      if(s.ok() != gt_is_exist){
        std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
      }
      if(gt_is_exist == false){continue;}
      if(value != gt_value){
        std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
      }
    }

    


    long long total_read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
          + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
    long long total_read_bytes_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
          + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");

    std::cout << "total_read_count_start = " << total_read_count_start << std::endl;
    std::cout << "total_read_count_end = " << total_read_count_end << std::endl;
    std::cout << "total_read_bytes_start = " << total_read_bytes_start << std::endl;
    std::cout << "total_read_bytes_end = " << total_read_bytes_end << std::endl;
    std::cout << "total_read_count = " << total_read_count_end - total_read_count_start << std::endl;
    std::cout << "total_read_bytes = " << total_read_bytes_end - total_read_bytes_start << std::endl;

    print_perf_iostats_context(std::cout, 1);
  }


  std::cout << "!!! several gets done " << std::endl;

  std::cout << "!!! print stats " << std::endl;

  
  printStats(db, op);

  std::cout << "!!! runQPVerification start " << std::endl;

  io_timing_test(db);


 
  workload_file.close();


  return;
}

#endif  // UTILS_RUN_WORKLOAD_H