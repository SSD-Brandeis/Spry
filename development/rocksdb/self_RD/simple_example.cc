#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <chrono>
#include <regex>
#include <chrono>
#include <thread>



#include "rocksdb/statistics.h"
#include "rocksdb/advanced_options.h"
#include "rocksdb/cache.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"
#include "rocksdb/table.h"

#include "rocksdb/system_verifier.h"

#include "rocksdb/iostats_context.h"
#include "rocksdb/perf_context.h"


using namespace rocksdb;
std::string kDBPath = "/tmp/cs561_project1";

inline void showProgress(const uint64_t& workload_size,
                         const uint64_t& counter) {
  if (counter / (workload_size / 100) >= 1) {
    for (int i = 0; i < 104; i++) {
      std::cout << "\b";
      fflush(stdout);
    }
  }
  for (int i = 0; i < counter / (workload_size / 100); i++) {
    std::cout << "=";
    fflush(stdout);
  }
  std::cout << std::setfill(' ')
            << std::setw(101 - counter / (workload_size / 100));
  std::cout << counter * 100 / workload_size << "%";
  fflush(stdout);

  if (counter == workload_size) {
    std::cout << "\n";
    return;
  }
}

void printStats(DB* db, Options& options) {
    std::string each_level_stats;
    std::string sst_file_size;
    std::string all_stats = options.statistics->ToString();

    std::cout << all_stats << std::endl;  // printing all stats

    bool result = db->GetProperty("rocksdb.levelstats", &each_level_stats);
    bool live_sst_file_size = db->GetProperty("rocksdb.live-sst-files-size", &sst_file_size);

    std::cout << std::endl;
    std::cout << "Level Statistics" << std::endl;

    if (result){
        std::cout << "Level, Total Files, " << each_level_stats << std::endl;  // printing level stats
    }
    if (live_sst_file_size) {
        std::cout << "Total SST Files Size : " << sst_file_size << std::endl;  // printing sst file size
    }
    std::cout << "----------------------------------------" << std::endl;

    std::cout << std::endl;
    std::cout << "RocksDB Statistics : " << std::endl;
    std::cout << "----------------------------------------" << std::endl;



    // std::cout << std::endl;
    // std::cout << "RocksDB perf_context : " << std::endl;
    // rocksdb::SetPerfLevel(rocksdb::PerfLevel::kDisable);
    // std::string perf_context = rocksdb::get_perf_context()->ToString();
    // std::cout << perf_context << std::endl;  
    // std::cout << "----------------------------------------" << std::endl;


    // std::cout << std::endl;
    // std::cout << "RocksDB iostats_context : " << std::endl;
    // rocksdb::SetPerfLevel(rocksdb::PerfLevel::kDisable);
    // std::string iostats_context = rocksdb::get_iostats_context()->ToString();
    // std::cout << iostats_context << std::endl;  
    // std::cout << "----------------------------------------" << std::endl;

 
}


long long parsing_value_from_string(std::string str, std::string pattern){
    std::regex re(pattern);
    std::smatch match;
    if(std::regex_search(str, match, re)){
      std::string result = match[1];
      // std:cout << str << std::endl;
      // std::cout << "value from string: " << result << std::endl;
      return std::stoll(result);
    }
    //raise error
    std::cerr << "Error parsing string, pattern not found." << std::endl;
    std::exit(1);
}

void init(){
  rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);

  // rocksdb::get_perf_context()->Reset();
  // rocksdb::get_iostats_context()->Reset();
  
  checking::SystemVerifier::init();
}


void runWorkload(Options& op, WriteOptions& write_op, ReadOptions& read_op) {
  DB* db;


  // op.write_buffer_size = 1024 * 256; // -> 256 kB    
  // op.write_buffer_size = 1024 * 8; // -> 256 kB    
  op.write_buffer_size = 32; // -> 256 kB    
  op.max_background_jobs = 1;
  op.level0_file_num_compaction_trigger = 1;
  op.target_file_size_base = op.write_buffer_size; // -> same as buffer size
  op.target_file_size_multiplier = 1;  // Same files size across levels
  op.max_write_buffer_number = 1;      // 1 buffer in-memory
  op.max_bytes_for_level_base =
      op.write_buffer_size;               // same as write buffer size
  op.max_bytes_for_level_multiplier = 2;  // T-ratio
  op.statistics = CreateDBStatistics();
  op.create_if_missing = true;
  // op.write_buffer_size = 8 * 1024 * 1024;

  {
    op.memtable_factory =
        std::shared_ptr<VectorRepFactory>(new VectorRepFactory);
    op.allow_concurrent_memtable_write = false;
  }

  {
      // op.memtable_factory = std::shared_ptr<SkipListFactory>(new
      // SkipListFactory);
  }

  {
      // op.memtable_factory =
      // std::shared_ptr<MemTableRepFactory>(NewHashSkipListRepFactory());
      // op.allow_concurrent_memtable_write = false;
  }

  {
    // op.memtable_factory =
    // std::shared_ptr<MemTableRepFactory>(NewHashLinkListRepFactory());
    // op.allow_concurrent_memtable_write = false;
  }

  // BlockBasedTableOptions table_options;
  // table_options.block_cache = NewLRUCache(8*1048576);
  // op.table_factory.reset(NewBlockBasedTableFactory(table_options));

  Status s = DB::Open(op, kDBPath, &db);
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());

  // opening workload file for the first time
  std::ifstream workload_file;
  workload_file.open("workload.txt");
  assert(workload_file);
  // doing a first pass to get the workload size
  uint64_t workload_size = 0;
  std::string line;
  while (std::getline(workload_file, line)) ++workload_size;
  workload_file.close();

  workload_file.open("workload.txt");
  assert(workload_file);

  // Clearing the system cache
  std::cout << "Clearing system cache ..." << std::endl;
  int clean_flag = system("sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'");
  if (clean_flag) {
    std::cerr << "Cannot clean the system cache" << std::endl;
    exit(0);
  }

  checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();

  Iterator* it = db->NewIterator(read_op);  // for range reads
  uint64_t counter = 0;                     // for progress bar
  int KEY_SIZE = 12;

  while (!workload_file.eof()) {
    char instruction;
    long long key, start_key, end_key;
    std::string type;
    std::string value;
    std::stringstream ss_key, ss_start_key, ss_end_key;
    workload_file >> instruction;
    switch (instruction) {
      case 'I':  // insert
        workload_file >> key >> value;

        system_verifier->insert(key, value);

        // std::cout << "Insert " << key << std::endl;
        ss_key << std::setfill('0') << std::setw(KEY_SIZE) << key;
        // std::cout << "Insert " <<  ss_key.str() << std::endl;
        // Put key-value
        s = db->Put(write_op, ss_key.str(), value);
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

          system_verifier->rangeDelete(start_key, end_key);


          ss_start_key << std::setfill('0') << std::setw(KEY_SIZE) << start_key;
          ss_end_key << std::setfill('0') << std::setw(KEY_SIZE) << end_key;
          s = db->DeleteRange(write_op, db->DefaultColumnFamily(),
                              ss_start_key.str(), ss_end_key.str());
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
        break;
    }

    if (workload_size < 100) workload_size = 100;
    if (counter % (workload_size / 100) == 0) {
      showProgress(workload_size, counter);
    }
  }

  
  std::this_thread::sleep_for(std::chrono::seconds(10));  // Sleep for 1 second
  {
    std::vector<long long> testing_key_list({2500, 5000, 5001});
    long long total_read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
          + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
    long long total_read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
          + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
    rocksdb::get_perf_context()->Reset();
    rocksdb::get_iostats_context()->Reset();


    for(auto &x: testing_key_list){
      bool gt_is_exist = system_verifier->isKeyExist(x);
      std::string gt_value = system_verifier->get(x);

      std::string value;
      std::stringstream searching_key;
      searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;
      s = db->Get(read_op, searching_key.str(), &value);
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

    rocksdb::SetPerfLevel(rocksdb::PerfLevel::kDisable);
    // std::string perf_context = rocksdb::get_perf_context()->ToString();
    
    // long long get_read_bytes = parsing_value_from_string(op.statistics->ToString(), "get_read_bytes[^:]*= ([0-9]+)");
    long long get_from_memtable_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), "get_from_memtable_time[^:]*= ([0-9]+)");
    long long get_from_memtable_count = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), "get_from_memtable_count[^:]*= ([0-9]+)");
    long long get_post_process_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), "get_post_process_time[^:]*= ([0-9]+)");

    long long bloom_memtable_hit_count = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), "bloom_memtable_hit_count[^:]*= ([0-9]+)");
    long long bloom_memtable_miss_count = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), "bloom_memtable_miss_count[^:]*= ([0-9]+)");
    long long bloom_sst_hit_count = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), "bloom_sst_hit_count[^:]*= ([0-9]+)");
    long long bloom_sst_miss_count = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), "bloom_sst_miss_count[^:]*= ([0-9]+)");



    rocksdb::SetPerfLevel(rocksdb::PerfLevel::kDisable);
    std::string iostats_context = rocksdb::get_iostats_context()->ToString();

    long long bytes_read = parsing_value_from_string(iostats_context, "bytes_read[^:]*= ([0-9]+)");
    long long bytes_written = parsing_value_from_string(iostats_context, "bytes_written[^:]*= ([0-9]+)");
    long long read_nanos = parsing_value_from_string(iostats_context, "read_nanos[^:]*= ([0-9]+)");
    long long write_nanos = parsing_value_from_string(iostats_context, "write_nanos[^:]*= ([0-9]+)");

    //print out all the above variable
    std::cout << "get_from_memtable_time = " << get_from_memtable_time << std::endl;
    std::cout << "get_from_memtable_count = " << get_from_memtable_count << std::endl;
    std::cout << "get_post_process_time = " << get_post_process_time << std::endl;
    std::cout << "bloom_memtable_hit_count = " << bloom_memtable_hit_count << std::endl;
    std::cout << "bloom_memtable_miss_count = " << bloom_memtable_miss_count << std::endl;
    std::cout << "bloom_sst_hit_count = " << bloom_sst_hit_count << std::endl;
    std::cout << "bloom_sst_miss_count = " << bloom_sst_miss_count << std::endl;
    std::cout << "bytes_read = " << bytes_read << std::endl;
    std::cout << "bytes_written = " << bytes_written << std::endl;
    std::cout << "read_nanos = " << read_nanos << std::endl;
    std::cout << "write_nanos = " << write_nanos << std::endl;
    //print out a separation line
    std::cout << "--------------------------------------------------------------------" << std::endl;
  }



  // {
  //   std::ofstream testing_result_file;
  //   testing_result_file.open("testing_result.txt");
  //   const long long N_repetitions = 3;
  //   // long long num_RDF_types = getNumberOfRDFTypes();
  //   long long count = 0;

  //   auto start_pq = std::chrono::high_resolution_clock::now();
  //   auto stop_pq = std::chrono::high_resolution_clock::now();
  //   auto duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
  //   unsigned long long point_query_time = duration_pq.count();



  //   testing_result_file << std::endl << std::endl;
  //   testing_result_file << "----------------------Testing On Existing Keys-----------------------" << std::endl; 
  //   // system_verifier->resetDiskAccessCount();
  //   for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
  //     system_verifier->resetDiskAccessCount();
  //     system_verifier->setRDFTypeChosed(t);
  //     count = 0;
  //     point_query_time = 0;
  //     start_pq = std::chrono::high_resolution_clock::now();
  //     for(auto i = 0; i < N_repetitions; i++){
  //   // system_verifier->setRDFTypeChosed(1);
  //       for(auto &x: system_verifier->getAllExistingKeys()){
  //         bool gt_is_exist = system_verifier->isKeyExist(x);
  //         std::string gt_value = system_verifier->get(x);

  //         std::string value;
  //         std::stringstream searching_key;
  //         searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;
  //         s = db->Get(read_op, searching_key.str(), &value);
  //         // std::cout << x << " " << s.ok() << " " << value << std::endl;
  //         // std::cout << x << " " << gt_is_exist << " " << gt_value << std::endl;
        
  //         if(s.ok() != gt_is_exist){
  //           std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
  //         }
  //         if(gt_is_exist == false){continue;}
  //         if(value != gt_value){
  //           std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
  //         }
  //       }
  //       count += system_verifier->getDiskAccessCount();
  //       stop_pq = std::chrono::high_resolution_clock::now();
  //       duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
  //       point_query_time += duration_pq.count();

  //   // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/1 << std::endl;
  //     }  
  //     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/N_repetitions << std::endl;
  //     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions << " (ms) " << std::endl;
  //   }

  //   testing_result_file << std::endl << std::endl;
  //   testing_result_file << "----------------------Testing On historic-existing Keys-----------------------" << std::endl;
  //   // system_verifier->resetDiskAccessCount();
  //   for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
  //     system_verifier->resetDiskAccessCount();
  //     system_verifier->setRDFTypeChosed(t);
  //     count = 0;
  //     point_query_time = 0;
  //     start_pq = std::chrono::high_resolution_clock::now();
  //     for(auto i = 0; i < N_repetitions; i++){
  //   // system_verifier->setRDFTypeChosed(1);
  //       for(auto &x: system_verifier->getHistoricExistingKeys()){
  //         bool gt_is_exist = system_verifier->isKeyExist(x);
  //         std::string gt_value = system_verifier->get(x);

  //         std::string value;
  //         std::stringstream searching_key;
  //         searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;
  //         s = db->Get(read_op, searching_key.str(), &value);
  //         // std::cout << x << " " << s.ok() << " " << value << std::endl;
  //         // std::cout << x << " " << gt_is_exist << " " << gt_value << std::endl;
        
  //         if(s.ok() != gt_is_exist){
  //           std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
  //         }
  //         if(gt_is_exist == false){continue;}
  //         if(value != gt_value){
  //           std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
  //         }
  //       }
  //       count += system_verifier->getDiskAccessCount();
  //       stop_pq = std::chrono::high_resolution_clock::now();
  //       duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
  //       point_query_time += duration_pq.count();

  //   // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/1 << std::endl;
  //     }  
  //     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/N_repetitions << std::endl;
  //     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions << " (ms) " << std::endl;
  //   }

  //   testing_result_file << std::endl << std::endl;
  //   testing_result_file << "----------------------Testing On Currently Deleted Keys-----------------------" << std::endl;
  //   // system_verifier->resetDiskAccessCount();
  //   for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
  //     system_verifier->resetDiskAccessCount();
  //     system_verifier->setRDFTypeChosed(t);
  //     count = 0;
  //     point_query_time = 0;
  //     start_pq = std::chrono::high_resolution_clock::now();
  //     for(auto i = 0; i < N_repetitions; i++){
  //   // system_verifier->setRDFTypeChosed(1);
  //       for(auto &x: system_verifier->getCurrentlyDeletedKeys()){
  //         bool gt_is_exist = system_verifier->isKeyExist(x);
  //         std::string gt_value = system_verifier->get(x);

  //         std::string value;
  //         std::stringstream searching_key;
  //         searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;
  //         s = db->Get(read_op, searching_key.str(), &value);
  //         // std::cout << x << " " << s.ok() << " " << value << std::endl;
  //         // std::cout << x << " " << gt_is_exist << " " << gt_value << std::endl;
        
  //         if(s.ok() != gt_is_exist){
  //           std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
  //         }
  //         if(gt_is_exist == false){continue;}
  //         if(value != gt_value){
  //           std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
  //         }
  //       }
  //       count += system_verifier->getDiskAccessCount();
  //       stop_pq = std::chrono::high_resolution_clock::now();
  //       duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
  //       point_query_time += duration_pq.count();

  //   // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/1 << std::endl;
  //     }  
  //     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/N_repetitions << std::endl;
  //     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions << " (ms) " << std::endl;
  //   }

  //   testing_result_file << std::endl << std::endl;
  //   testing_result_file << "----------------------Testing On Currently Non-inserted Keys-----------------------" << std::endl;
  //   // system_verifier->resetDiskAccessCount();
  //   for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
  //     system_verifier->resetDiskAccessCount();
  //     system_verifier->setRDFTypeChosed(t);
  //     count = 0;
  //     point_query_time = 0;
  //     start_pq = std::chrono::high_resolution_clock::now();
  //     for(auto i = 0; i < N_repetitions; i++){
  //   // system_verifier->setRDFTypeChosed(1);
  //       for(auto &x: system_verifier->getCurrentlyNonInsertedKeys(1000)){ // test on 1000 keys
  //         bool gt_is_exist = system_verifier->isKeyExist(x); // should be false
  //         std::string gt_value = system_verifier->get(x); // should be ""

  //         std::string value;
  //         std::stringstream searching_key;
  //         searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;
  //         s = db->Get(read_op, searching_key.str(), &value);
  //         // std::cout << x << " " << s.ok() << " " << value << std::endl;
  //         // std::cout << x << " " << gt_is_exist << " " << gt_value << std::endl;
        
  //         if(s.ok() != gt_is_exist){
  //           std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
  //         }
  //         if(gt_is_exist == false){continue;}
  //         if(value != gt_value){
  //           std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
  //         }
  //       }
  //       count += system_verifier->getDiskAccessCount();
  //       stop_pq = std::chrono::high_resolution_clock::now();
  //       duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
  //       point_query_time += duration_pq.count();

  //   // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/1 << std::endl;
  //     }  
  //     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/N_repetitions << std::endl;
  //     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions << " (ms) " << std::endl;
  //   }

  //   testing_result_file << std::endl << std::endl;
  //   testing_result_file << "----------------------End Testing-----------------------" << std::endl;

  //   testing_result_file.close();
  // }

  


  workload_file.close();
  printStats(db, op);
  s = db->Close();
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());
  delete db;
  std::cout << "\n----------------------Closing DB-----------------------"
            << std::endl;

  return;
}

int main() {
  init();

  Options options;
  WriteOptions write_op;
  ReadOptions read_op;
  runWorkload(options, write_op, read_op);
}
