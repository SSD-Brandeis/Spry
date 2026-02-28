#ifndef UTILS_RUN_WORKLOAD_H
#define UTILS_RUN_WORKLOAD_H

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "../env_settings/emu_environment.h"
#include "../workload/args.hxx"
#include "../workload/workload_generator.h"
#include "rocksdb/SuRF/include/surf.hpp"
#include "rocksdb/advanced_options.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/system_verifier.h"
#include "utils_logger_during_insertion.h"
#include "utils_verification_during_run.h"

// #define CHECKING_QUERY_ENABLED

void runWorkload(DB** db_ptr2, Options& op, WriteOptions& write_op,
                 ReadOptions& read_op, EmuEnv* _env, std::string kDBPath) {
  DB* db = *db_ptr2;

  string& workload_file_name = _env->workload_file_name;

  Status s;

  // opening workload file for the first time
  std::ifstream workload_file;
  workload_file.open(workload_file_name);
  assert(workload_file);
  // doing a first pass to get the workload size
  uint64_t workload_size = 0;
  std::string line;
  while (std::getline(workload_file, line)) ++workload_size;
  workload_file.close();

  workload_file.open(workload_file_name);
  assert(workload_file);

  checking::SystemVerifier* system_verifier =
      checking::SystemVerifier::getSystemVerifier();
  system_verifier->resetRunningPQ();

  LoggerDuringInsertion* logger_during_insertion =
      LoggerDuringInsertion::getInstance(_env);

  Iterator* it = db->NewIterator(read_op);  // for range reads
  uint64_t counter = 0;                     // for progress bar
  int KEY_SIZE = checking::SystemVerifier::getSystemVerifier()->getKeySize();
  int flag_using_string_key =
      checking::SystemVerifier::getSystemVerifier()->usingStringKey();

  std::cout << "KEY_SIZE = " << KEY_SIZE << " " << __FILE__ << ":" << __LINE__
            << " " << __FUNCTION__ << std::endl;
  std::cout << "flag_using_string_key = " << flag_using_string_key << " "
            << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  // int TIME_STAMP_SIZE = 7;  // shall == rocksdb sequence num
  long long i_instruction = 0;

  auto start_time = std::chrono::high_resolution_clock::now();
  auto stop_time = std::chrono::high_resolution_clock::now();
  auto duration_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
      stop_time - start_time);
  unsigned long long insertion_time_ns = 0;
  unsigned long long rd_time_ns = 0;
  unsigned long long point_query_time_ns = 0;
  unsigned long long scan_time_ns = 0;
  unsigned long long point_query_time_on_existing_keys_ns = 0;
  unsigned long long point_query_time_on_deleted_keys_ns = 0;
  unsigned long long point_query_time_on_non_inserted_keys_ns = 0;

  reset_perf_iostats_context();
  if (op.statistics) {
    op.statistics->Reset();
  }
  system_verifier->resetAllCount();
  system_verifier->resetAllDuration();

  while (!workload_file.eof()) {
    i_instruction++;
    while (db->existFlushJob() == true || db->existCompactionJob() == true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    char instruction;
    // std::string time_stamp;
    // std::stringstream ss_time_stamp;
    // long long key, start_key, end_key;
    std::string key, start_key, end_key;
    std::string type;
    std::string value;
    std::stringstream ss_key, ss_start_key, ss_end_key;
    size_t separator_pos = 0;
    workload_file >> instruction;
    if (workload_file.eof()) {
      break;
    }

    switch (instruction) {
      case 'U':
      case 'I':  // insert
        workload_file >> key >> value;
        if (flag_using_string_key == 0) {
          ss_key << std::setfill('0') << std::setw(KEY_SIZE) << key;
          // std::cout << key << " " << __FILE__ << ":" << __LINE__ << " " <<
          // __FUNCTION__ << std::endl;
          key = ss_key.str();
        }

        // if (_env->load_pq_workload == false) {
        system_verifier->insert(key, value);
        // }

        // ss_key << std::setfill('0') << std::setw(KEY_SIZE) << key;
        // ss_time_stamp << std::setfill('0') << std::setw(TIME_STAMP_SIZE) <<
        // i_instruction;
        start_time = std::chrono::high_resolution_clock::now();
        // std::cout << "Insert " << ss_key.str() << std::endl;
        // s = db->Put(write_op, ss_key.str(), value + "|" +
        // ss_time_stamp.str()); s = db->Put(write_op, ss_key.str(), value);
        s = db->Put(write_op, key, value);
        stop_time = std::chrono::high_resolution_clock::now();
        duration_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
            stop_time - start_time);
        insertion_time_ns += duration_time.count();
        if (!s.ok()) std::cerr << s.ToString() << std::endl;
        assert(s.ok());
        counter++;

        if (_env->log_during_insertion == true) {
          logger_during_insertion->recordCurrentMemoryFootprint(db_ptr2);
        }
        break;

      case 'P':
      case 'Q':  // probe: point query
        workload_file >> key;

        if (flag_using_string_key == 0) {
          ss_key << std::setfill('0') << std::setw(KEY_SIZE) << key;
          key = ss_key.str();
        }

        // std::cout << "Query " << key << std::endl;
        // ss_key << std::setfill('0') << std::setw(KEY_SIZE) << key;
        // s = db->Get(read_op, ss_key.str(), &value);
        start_time = std::chrono::high_resolution_clock::now();
        s = db->Get(read_op, key, &value);
        stop_time = std::chrono::high_resolution_clock::now();
        duration_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
            stop_time - start_time);
        point_query_time_ns += duration_time.count();

        if (system_verifier->isKeyExist(key)) {
          point_query_time_on_existing_keys_ns += duration_time.count();
        } else if (system_verifier->historicExistingKeys.count(key) > 0) {
          point_query_time_on_deleted_keys_ns += duration_time.count();
        } else {
          point_query_time_on_non_inserted_keys_ns += duration_time.count();
        }

#ifdef CHECKING_QUERY_ENABLED
        verification::verifyPointQuery(key, s, value, system_verifier);
#endif

        // separator_pos = value.find("|");
        // time_stamp = value.substr(separator_pos + 1);
        // value = value.substr(0, separator_pos);
        counter++;
        break;

      case 'S':  // scan: range query
        workload_file >> start_key >> end_key;

        // std::cout << "start_key = " << start_key << " " << "end_key = " <<
        // end_key << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<
        // std::endl;
        if (flag_using_string_key == 0) {
          ss_start_key << std::setfill('0') << std::setw(KEY_SIZE) << start_key;
          ss_end_key << std::setfill('0') << std::setw(KEY_SIZE) << end_key;
          start_key = ss_start_key.str();
          end_key = ss_end_key.str();
        }
        // std::cout << "start_key = " << start_key << " " << "end_key = " <<
        // end_key << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<
        // std::endl;

        it->Refresh();
        assert(it->status().ok());
        // ss_start_key << std::setfill('0') << std::setw(KEY_SIZE) <<
        // start_key; ss_end_key << std::setfill('0') << std::setw(KEY_SIZE) <<
        // end_key;
        // for (it->Seek(ss_start_key.str()); it->Valid(); it->Next()) {
        //   if (it->key().ToString() == ss_end_key.str()) {
        //     break;
        //   }
        // }
        {
          std::vector<std::pair<std::string, std::string>> scan_results;
          start_time = std::chrono::high_resolution_clock::now();
          for (it->Seek(start_key); it->Valid(); it->Next()) {
            if (it->key().ToString() >= end_key) {
              break;
            }
            scan_results.push_back(
                {it->key().ToString(), it->value().ToString()});
          }
          stop_time = std::chrono::high_resolution_clock::now();
          duration_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
              stop_time - start_time);
          scan_time_ns += duration_time.count();

#ifdef CHECKING_QUERY_ENABLED
          verification::verifyScan(start_key, end_key, scan_results,
                                   system_verifier);
#endif
        }

        if (!it->status().ok()) {
          std::cerr << it->status().ToString() << std::endl;
        }
        counter++;
        break;

      case 'D':  // delete
        workload_file >> type >> start_key >> end_key;
        if (flag_using_string_key == 0) {
          ss_start_key << std::setfill('0') << std::setw(KEY_SIZE) << start_key;
          ss_end_key << std::setfill('0') << std::setw(KEY_SIZE) << end_key;
          start_key = ss_start_key.str();
          end_key = ss_end_key.str();
        }

        if (type == "Range") {
          while (db->existFlushJob() == true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }

          // if (_env->load_pq_workload == false) {
          system_verifier->rangeDelete(start_key, end_key);
          // }

          while (db->existFlushJob() == true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }

          // ss_start_key << std::setfill('0') << std::setw(KEY_SIZE) <<
          // start_key; ss_end_key << std::setfill('0') << std::setw(KEY_SIZE)
          // << end_key; std::cout << "RD " << ss_start_key.str() << " " <<
          // ss_end_key.str() << std::endl;
          start_time = std::chrono::high_resolution_clock::now();
          // s = db->DeleteRange(write_op, db->DefaultColumnFamily(),
          //                     ss_start_key.str(), ss_end_key.str());
          s = db->DeleteRange(write_op, db->DefaultColumnFamily(), start_key,
                              end_key);
          stop_time = std::chrono::high_resolution_clock::now();
          duration_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
              stop_time - start_time);
          rd_time_ns += duration_time.count();
          if (!s.ok()) std::cerr << s.ToString() << std::endl;
          assert(s.ok());
          counter++;
        } else {
          std::cerr << "ERROR: Case match NOT found !!" << std::endl;
          std::cerr << "instruction = " << instruction << std::endl;
          std::cerr << "type = " << type << std::endl;
          std::cerr << "start_key = " << start_key << std::endl;
          std::cerr << "end_key = " << end_key << std::endl;
          break;
        }
        break;

      case 'R':
        workload_file >> start_key >> end_key;
        if (flag_using_string_key == 0) {
          ss_start_key << std::setfill('0') << std::setw(KEY_SIZE) << start_key;
          ss_end_key << std::setfill('0') << std::setw(KEY_SIZE) << end_key;
          start_key = ss_start_key.str();
          end_key = ss_end_key.str();
        }

        while (db->existFlushJob() == true) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // if (_env->load_pq_workload == false) {
        system_verifier->rangeDelete(start_key, end_key);
        // }

        while (db->existFlushJob() == true) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // ss_start_key << std::setfill('0') << std::setw(KEY_SIZE) <<
        // start_key; ss_end_key << std::setfill('0') << std::setw(KEY_SIZE) <<
        // end_key;
        start_time = std::chrono::high_resolution_clock::now();
        // s = db->DeleteRange(write_op, db->DefaultColumnFamily(),
        //                     ss_start_key.str(), ss_end_key.str());
        s = db->DeleteRange(write_op, db->DefaultColumnFamily(), start_key,
                            end_key);
        stop_time = std::chrono::high_resolution_clock::now();
        duration_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
            stop_time - start_time);
        rd_time_ns += duration_time.count();
        if (!s.ok()) std::cerr << s.ToString() << std::endl;
        assert(s.ok());
        counter++;
        break;

      default:
        std::cerr << "ERROR: Case match NOT found !!" << std::endl;
        std::cerr << "instruction = " << instruction << std::endl;
        std::cout << "ERROR: Case match NOT found !!" << std::endl;
        std::cout << "instruction = " << instruction << std::endl;
        break;
    }

    // {
    //   if (workload_size < 100) workload_size = 100;
    //   if (counter % (workload_size / 100) == 0) {
    //     showProgress(workload_size, counter);
    //   }
    // }

    // // run PQ and log memory footprint during insertion
    // vector<string> currently_deleted_keys =
    //     system_verifier->getCurrentlyDeletedKeys();

    // if (counter % _env->run_pq_during_insertion_interval == 0 &&
    //     currently_deleted_keys.size() > 100) {
    //   logger_during_insertion->writeRecord(db_ptr2);

    //   std::this_thread::sleep_for(
    //       std::chrono::seconds(30));  // Sleep for 10 second

    //   if (_env->log_during_insertion == true) {
    //     logger_during_insertion->runPQonCurrentlyDeletedKeys(
    //         counter, db_ptr2, op, write_op, read_op, _env, 500, kDBPath);
    //   }
    // }
  }
  std::cout << "insertion_time_ns _out = " << insertion_time_ns << std::endl;
  std::cout << "rd_time_ns _out = " << rd_time_ns << std::endl;
  std::cout << "point_query_time_ns _out = " << point_query_time_ns
            << std::endl;
  std::cout << "scan_time_ns _out = " << scan_time_ns << std::endl;
  std::cout << "point_query_time_on_existing_keys_ns _out = "
            << point_query_time_on_existing_keys_ns << std::endl;
  std::cout << "point_query_time_on_deleted_keys_ns _out = "
            << point_query_time_on_deleted_keys_ns << std::endl;
  std::cout << "point_query_time_on_non_inserted_keys_ns _out = "
            << point_query_time_on_non_inserted_keys_ns << std::endl;

  print_RDF_memory_usage_generic(db, system_verifier);
  print_RDF_false_positive_rate_generic(db, system_verifier);

  std::string prefix = "utils_run_worload_test ";
  print_perf_iostats_context(std::cout, prefix, 1);

  std::cout << "!!! Final Flush. (Manually Flush) " << std::endl;

  while (db->existFlushJob() == true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  FlushOptions flush_opts;
  s = db->Flush(flush_opts);
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());

  std::cout << "!!! Insertion Workload Ends." << std::endl;

  // std::this_thread::sleep_for(std::chrono::seconds(10));  // Sleep for 10
  // second

  // std::cout << "!!! After sleep." << std::endl;

  // logger_during_insertion->recordCurrentMemoryFootprint(db_ptr2);
  // logger_during_insertion->writeRecord(db_ptr2);
  // logger_during_insertion->end();

  db->printAllFileRanges();

  printStats(db, op);

  db->printRDF();

  // s = db->SetOptions(
  //     {{"disable_auto_compactions",
  //       "true"}});  // is there any compaction happended after this????
  // if (!s.ok()) std::cerr << s.ToString() << std::endl;
  // assert(s.ok());
  // std::cout << "!!! Disable auto compaction" << std::endl;

  // // std::this_thread::sleep_for(std::chrono::seconds(10));  // Sleep for 10
  // // second

  // while (db->existFlushJob() == true || db->existCompactionJob() == true) {
  //   std::this_thread::sleep_for(std::chrono::milliseconds(10));
  // }

  // db->printAllFileRanges();

  // db->printRDF();

  // printStats(db, op);

  // // uint num_SST_files = db->getTotalNumberOfSSTFiles();
  // // std::cout << "num_SST_files _out = " << num_SST_files << std::endl;

  // // {
  // //   // std::vector<string> testing_key_list({"2500", "5000", "5001"});
  // //   long long total_read_count_start =
  // //       parsing_value_from_string(op.statistics->ToString(),
  // //                                 "last.level.read.count[^:]*: ([0-9]+)")
  // +
  // //       parsing_value_from_string(op.statistics->ToString(),
  // //                                 "non.last.level.read.count[^:]*:
  // ([0-9]+)");
  // //   long long total_read_bytes_start =
  // //       parsing_value_from_string(op.statistics->ToString(),
  // //                                 "last.level.read.bytes[^:]*: ([0-9]+)")
  // +
  // //       parsing_value_from_string(op.statistics->ToString(),
  // //                                 "non.last.level.read.bytes[^:]*:
  // ([0-9]+)");
  // //   // reset_perf_iostats_context();

  // //   // for (auto& x : testing_key_list) {
  // //   //   bool gt_is_exist = system_verifier->isKeyExist(x);
  // //   //   std::string gt_value = system_verifier->get(x);

  // //   //   std::string value;
  // //   //   // std::string time_stamp;
  // //   //   std::stringstream searching_key;
  // //   //   searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;
  // //   //   s = db->Get(read_op, searching_key.str(), &value);
  // //   //   // size_t separator_pos = value.find("|");
  // //   //   // time_stamp = value.substr(separator_pos + 1);
  // //   //   // value = value.substr(0, separator_pos);
  // //   //   std::cout << x << " " << s.ok() << " " << value << std::endl;
  // //   //   std::cout << x << " " << gt_is_exist << " " << gt_value <<
  // std::endl;

  // //   //   if (s.ok() != gt_is_exist) {
  // //   //     std::cout << "ERROR (Existence inconsistency): " << x
  // //   //               << " (result, gt_result) " << s.ok() << " " <<
  // //   //               gt_is_exist
  // //   //               << std::endl;
  // //   //   }
  // //   //   if (gt_is_exist == false) {
  // //   //     continue;
  // //   //   }
  // //   //   if (value != gt_value) {
  // //   //     std::cout << "ERROR (Value inconsistency): " << x
  // //   //               << " (value, gt_value) " << value << " " << gt_value
  // //   //               << std::endl;
  // //   //   }
  // //   // }

  // //   long long total_read_count_end =
  // //       parsing_value_from_string(op.statistics->ToString(),
  // //                                 "last.level.read.count[^:]*: ([0-9]+)")
  // +
  // //       parsing_value_from_string(op.statistics->ToString(),
  // //                                 "non.last.level.read.count[^:]*:
  // ([0-9]+)");
  // //   long long total_read_bytes_end =
  // //       parsing_value_from_string(op.statistics->ToString(),
  // //                                 "last.level.read.bytes[^:]*: ([0-9]+)")
  // +
  // //       parsing_value_from_string(op.statistics->ToString(),
  // //                                 "non.last.level.read.bytes[^:]*:
  // ([0-9]+)");

  // //   std::cout << "total_read_count_start _out = " <<
  // total_read_count_start
  // //             << std::endl;
  // //   std::cout << "total_read_count_end _out = " << total_read_count_end
  // //             << std::endl;
  // //   std::cout << "total_read_bytes_start _out = " <<
  // total_read_bytes_start
  // //             << std::endl;
  // //   std::cout << "total_read_bytes_end _out = " << total_read_bytes_end
  // //             << std::endl;
  // //   std::cout << "total_read_count _out = "
  // //             << total_read_count_end - total_read_count_start <<
  // std::endl;
  // //   std::cout << "total_read_bytes _out = "
  // //             << total_read_bytes_end - total_read_bytes_start <<
  // std::endl;

  // //   // std::string prefix = "utils_run_worload_test ";
  // //   // print_perf_iostats_context(std::cout, prefix, 1);
  // // }

  // // std::cout << "!!! several gets done " << std::endl;

  // // std::cout << "!!! print stats " << std::endl;

  // // printStats(db, op);

  system_verifier->getAllCount("", "", " ", 1);

  io_timing_test(db);

  workload_file.close();

  return;
}

#endif  // UTILS_RUN_WORKLOAD_H
