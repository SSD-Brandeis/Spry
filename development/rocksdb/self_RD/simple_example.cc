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

// #include "rocksdb/column_family.h"

// #include "rocksdb/util/cast_util.h"
#include "rocksdb/sys_rdfilter.h"



using namespace rocksdb;
std::string kDBPath = "/tmp/cs561_project1";

void printStats(DB* db, Options& options);
void print_perf_iostats_context(std::ostream& ofile, int N_repetitions = 1);
void init(DB **db_ptr2, Options& op, WriteOptions& write_op, ReadOptions& read_op);
long long parsing_value_from_string(std::string str, std::string pattern);
void reset_perf_iostats_context();


class TestingLogger{
  private:
    long long total_read_count;
    long long total_read_bytes;
    long long read_count_start;
    long long read_bytes_start;
    int i_round;

  public:
    TestingLogger(){
      total_read_count = 0;
      total_read_bytes = 0;
      i_round = 0;
    }

    void reset(){
      total_read_count = 0;
      total_read_bytes = 0;
      i_round = 0;
    }

    void reopen_DB(DB** db_ptr2, Options& op,  WriteOptions& write_op, ReadOptions& read_op){
      Status s = (*db_ptr2)->Close();
      if (!s.ok()) std::cerr << s.ToString() << std::endl;
      assert(s.ok());
      // DB* db;
      init(db_ptr2, op, write_op, read_op);

      s = DB::Open(op, kDBPath, db_ptr2);
      if (!s.ok()) std::cerr << s.ToString() << std::endl;
      assert(s.ok());
    }

    void set_to_start(Options& op){
        read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
          + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
        read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
          + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
        reset_perf_iostats_context();
        {
          rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
          rocksdb::get_perf_context()->Reset();
          rocksdb::get_iostats_context()->Reset();
        }
    }

    void set_to_end(Options& op, std::ostream& testing_result_file){
      i_round += 1;
      testing_result_file << i_round << " -----" << std::endl;    
  // long long total_read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
  //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
  // testing_result_file << "total_read_count_end = " << std::fixed << std::setprecision(2) << total_read_count_end << std::endl;
      long long read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
        + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
      long long read_bytes_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
        + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
      total_read_count += read_count_end - read_count_start;
      total_read_bytes += read_bytes_end - read_bytes_start;
      reset_perf_iostats_context();

      testing_result_file << "read_count_start = " << std::fixed << std::setprecision(2) << read_count_start << std::endl;
      testing_result_file << "read_count_end = " << std::fixed << std::setprecision(2) << read_count_end << std::endl;
      testing_result_file << "read_bytes_start = " << std::fixed << std::setprecision(2) << read_bytes_start << std::endl;
      testing_result_file << "read_bytes_end = " << std::fixed << std::setprecision(2) << read_bytes_end << std::endl;
      testing_result_file << i_round << " -----" << std::endl;  
    }


    void output_statistics(std::ostream& testing_result_file){
      testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count) * 1.0 / i_round << std::endl;
      testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes) * 1.0 / i_round << std::endl;
      print_perf_iostats_context(testing_result_file, i_round);
    }
};


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

void setNewBlockCacheForReading(Options& op){
  std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 10 second

  //Reference: options.cc
  BlockBasedTableOptions table_options;
  // table_options.block_cache = NewLRUCache(32*1024*1024);
  table_options.block_cache = NewLRUCache(0.5*1024*1024);
  // std::shared_ptr<LRUCahe> std::static_pointer_cast<LRUCahe>(table_options.block_cache);

  // table_options.block_cache = NULL; //default block cache
  // table_options.no_block_cache = true; //disable block cache completely

  // table_options.block_cache =
  //     (cache != nullptr) ? *cache : std::shared_ptr<Cache>();
  // 16MB block cache
  // std::shared_ptr<Cache> cache = NewLRUCache(16 << 20);
  // table_options.block_cache = cache;
  // table_options.cache_index_and_filter_blocks = true;
  // std::cout << " table_options.cache_index_and_filter_blocks  = " <<  table_options.cache_index_and_filter_blocks  << std::endl;
  // Two level iterator to avoid LRU cache imbalance
  // table_options.index_type =
  //     BlockBasedTableOptions::IndexType::kTwoLevelIndexSearch;
  op.table_factory.reset(NewBlockBasedTableFactory(table_options));
  
  std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 10 second
}

void setNoBlockCacheForReading(Options& op){
  std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 10 second

  //Reference: options.cc
  BlockBasedTableOptions table_options;
  // table_options.block_cache = NewLRUCache(32*1024*1024);
  // table_options.block_cache = NewLRUCache(0.5*1024*1024);
  // table_options.block_cache = NULL; //default block cache
  table_options.no_block_cache = true; //disable block cache completely

  op.table_factory.reset(NewBlockBasedTableFactory(table_options));

  std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 10 second
}



void clearCache(Options& op){
  std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 10 second

  // Clearing the system cache
  std::cout << "Clearing system cache ..." << std::endl;
  int clean_flag = system("sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'");
  if (clean_flag) {
    std::cerr << "Cannot clean the system cache" << std::endl;
    exit(0);
  }

  std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 10 second

  // BlockBasedTableOptions table_options;
  // // table_options.no_block_cache = true; //disable block cache completely
  // // table_options.block_cache = NewLRUCache(0.5*1024*1024);
  // op.table_factory.reset(NewBlockBasedTableFactory(table_options));
  
  // std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 10 second
}
void clearBlockCache(DB* db, std::ostream& ofile){
  std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 10 second

  db->CleanTableCache(ofile);

  std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 10 second

  // ColumnFamilyData *cfd = db->DefaultColumnFamily()->cfd();
  // // auto* cfd =
  // //     static_cast_with_check<ColumnFamilyHandleImpl>()->cfd();
  // TableCache* table_cache = cfd->table_cache();


  // Cache* table_cache = db->TEST_table_cache();
  
  // Status s = db->SetOptions({{"block_cache", "32M"}});
  // if(!s.ok()){
  //   std::cout << "Error setting block cache size" << __FILE__ << ":" << __LINE__ << std::endl;
  // }
  // Status s = db->SetOptions(db->DefaultColumnFamily(), {{"ttl", "36000"}});
}

void init(DB **db_ptr2, Options& op, WriteOptions& write_op, ReadOptions& read_op){
  rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
  // rocksdb::get_perf_context()->Reset();
  // rocksdb::get_iostats_context()->Reset();
  
  checking::SystemVerifier::init();


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


  setNewBlockCacheForReading(op);
  // setNoBlockCacheForReading(op);

  clearCache(op);


}

void reset_perf_iostats_context(){
  rocksdb::get_perf_context()->Reset();
  rocksdb::get_iostats_context()->Reset();
}

void print_perf_iostats_context(std::ostream& ofile, int N_repetitions){
  
    // rocksdb::SetPerfLevel(rocksdb::PerfLevel::kDisable);
    std::string perf_context = rocksdb::get_perf_context()->ToString();
    
    // long long get_read_bytes = parsing_value_from_string(op.statistics->ToString(), "get_read_bytes[^:]*= ([0-9]+)");
    long long get_from_memtable_time = parsing_value_from_string(perf_context, "get_from_memtable_time[^:]*= ([0-9]+)");
    long long get_from_memtable_count = parsing_value_from_string(perf_context, "get_from_memtable_count[^:]*= ([0-9]+)");
    long long get_post_process_time = parsing_value_from_string(perf_context, "get_post_process_time[^:]*= ([0-9]+)");

    long long bloom_memtable_hit_count = parsing_value_from_string(perf_context, "bloom_memtable_hit_count[^:]*= ([0-9]+)");
    long long bloom_memtable_miss_count = parsing_value_from_string(perf_context, "bloom_memtable_miss_count[^:]*= ([0-9]+)");
    long long bloom_sst_hit_count = parsing_value_from_string(perf_context, "bloom_sst_hit_count[^:]*= ([0-9]+)");
    long long bloom_sst_miss_count = parsing_value_from_string(perf_context, "bloom_sst_miss_count[^:]*= ([0-9]+)");



    // rocksdb::SetPerfLevel(rocksdb::PerfLevel::kDisable);
    std::string iostats_context = rocksdb::get_iostats_context()->ToString();

    long long bytes_read = parsing_value_from_string(iostats_context, "bytes_read[^:]*= ([0-9]+)");
    long long bytes_written = parsing_value_from_string(iostats_context, "bytes_written[^:]*= ([0-9]+)");
    long long read_nanos = parsing_value_from_string(iostats_context, "read_nanos[^:]*= ([0-9]+)");
    long long write_nanos = parsing_value_from_string(iostats_context, "write_nanos[^:]*= ([0-9]+)");

    //print out all the above variable
    ofile << "get_from_memtable_time = " << std::fixed << std::setprecision(2) << get_from_memtable_time * 1.0 / N_repetitions << std::endl;
    ofile << "get_from_memtable_count = " << std::fixed << std::setprecision(2) << get_from_memtable_count * 1.0 / N_repetitions << std::endl;
    ofile << "get_post_process_time = " << std::fixed << std::setprecision(2) << get_post_process_time * 1.0 / N_repetitions << std::endl;
    ofile << "bloom_memtable_hit_count = " << std::fixed << std::setprecision(2) << bloom_memtable_hit_count * 1.0 / N_repetitions << std::endl;
    ofile << "bloom_memtable_miss_count = " << std::fixed << std::setprecision(2) << bloom_memtable_miss_count * 1.0 / N_repetitions << std::endl;
    ofile << "bloom_sst_hit_count = " << std::fixed << std::setprecision(2) << bloom_sst_hit_count * 1.0 / N_repetitions << std::endl;
    ofile << "bloom_sst_miss_count = " << std::fixed << std::setprecision(2) << bloom_sst_miss_count * 1.0 / N_repetitions << std::endl;
    ofile << "bytes_read = " << std::fixed << std::setprecision(2) << bytes_read * 1.0 / N_repetitions << std::endl;
    ofile << "bytes_written = " << std::fixed << std::setprecision(2) << bytes_written * 1.0 / N_repetitions << std::endl;
    ofile << "read_nanos = " << std::fixed << std::setprecision(2) << read_nanos * 1.0 / N_repetitions << std::endl;
    ofile << "write_nanos = " << std::fixed << std::setprecision(2) << write_nanos * 1.0 / N_repetitions << std::endl;
    //print out a separation line
    ofile << "--------------------------------------------------------------------" << std::endl;
}






void runPQVerification(DB* db, Options& op, WriteOptions& write_op, ReadOptions& read_op){
  checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();
  int KEY_SIZE = checking::SystemVerifier::getKeySize();

  Status s;

  std::ofstream testing_result_file;
  testing_result_file.open("testing_result.txt");
  TestingLogger testing_logger;
  const long long N_repetitions = 4;
  // long long num_RDF_types = getNumberOfRDFTypes();
  long long count = 0;

  auto start_pq = std::chrono::high_resolution_clock::now();
  auto stop_pq = std::chrono::high_resolution_clock::now();
  auto duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
  unsigned long long point_query_time = duration_pq.count();

  testing_result_file << std::endl << std::endl;
  testing_result_file << "----------------------Testing On Existing Keys-----------------------" << std::endl; 
  // system_verifier->resetDiskAccessCount();
  for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << std::endl;
    // long long total_read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
    // long long total_read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
    // reset_perf_iostats_context();
    long long total_read_count = 0;
    long long total_read_bytes = 0;
    testing_logger.reset();



    system_verifier->resetDiskAccessCount();
    system_verifier->setRDFTypeChosed(t);
    count = 0;
    point_query_time = 0;
    start_pq = std::chrono::high_resolution_clock::now();
    rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
    rocksdb::get_perf_context()->Reset();
    rocksdb::get_iostats_context()->Reset();
    for(auto i = 0; i < N_repetitions; i++){
      clearCache(op);
      // clearBlockCache(db, testing_result_file);
      // rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
      // rocksdb::get_perf_context()->Reset();
      // rocksdb::get_iostats_context()->Reset();
      // setNewBlockCacheForReading(op);
      // setNoBlockCacheForReading(op);
      // {
      //   s = db->Close();
      //   if (!s.ok()) std::cerr << s.ToString() << std::endl;
      //   assert(s.ok());
      //   // DB* db;
      //   init(&db, op, write_op, read_op);

      //   Status s = DB::Open(op, kDBPath, &db);
      //   if (!s.ok()) std::cerr << s.ToString() << std::endl;
      //   assert(s.ok());
      // }
      // long long read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
      //   + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
      // long long read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
      //   + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
      // reset_perf_iostats_context();
      // {
      //   rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
      //   rocksdb::get_perf_context()->Reset();
      //   rocksdb::get_iostats_context()->Reset();
      // }
      testing_logger.reopen_DB(&db, op, write_op, read_op);
      testing_logger.set_to_start(op);

  // system_verifier->setRDFTypeChosed(1);
      for(auto &x: system_verifier->getAllExistingKeys()){
        bool gt_is_exist = system_verifier->isKeyExist(x);
        std::string gt_value = system_verifier->get(x);

        std::string value;
        std::stringstream searching_key;
        searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;
        s = db->Get(read_op, searching_key.str(), &value);
        // std::cout << x << " " << s.ok() << " " << value << std::endl;
        // std::cout << x << " " << gt_is_exist << " " << gt_value << std::endl;
      
        if(s.ok() != gt_is_exist){
          testing_result_file << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
        }
        if(gt_is_exist == false){continue;}
        if(value != gt_value){
          testing_result_file << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
        }
      }
      count += system_verifier->getDiskAccessCount();
      stop_pq = std::chrono::high_resolution_clock::now();
      duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
      point_query_time += duration_pq.count();

// testing_result_file << i << " -----" << std::endl;    
// // long long total_read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
// //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
// // testing_result_file << "total_read_count_end = " << std::fixed << std::setprecision(2) << total_read_count_end << std::endl;
//     long long read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
//       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
//     long long read_bytes_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
//       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
      // total_read_count += read_count_start - read_count_end;
      // total_read_bytes += read_bytes_start - read_bytes_end;
//     reset_perf_iostats_context();

//     testing_result_file << "read_count_start = " << std::fixed << std::setprecision(2) << read_count_start << std::endl;
//     testing_result_file << "read_count_end = " << std::fixed << std::setprecision(2) << read_count_end << std::endl;
//     testing_result_file << "read_bytes_start = " << std::fixed << std::setprecision(2) << read_bytes_start << std::endl;
//     testing_result_file << "read_bytes_end = " << std::fixed << std::setprecision(2) << read_bytes_end << std::endl;
// testing_result_file << i << " -----" << std::endl;    

      testing_logger.set_to_end(op, testing_result_file);
      if(i == 0){
        testing_logger.reset();
      }

  // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/1 << std::endl;
    }  
    // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/N_repetitions << std::endl;
    // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions << " (ms) " << std::endl;
  
    testing_logger.output_statistics(testing_result_file);

    // long long total_read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
    // long long total_read_bytes_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");

    // testing_result_file << "total_read_count_start = " << std::fixed << std::setprecision(2) << total_read_count_start << std::endl;
    // testing_result_file << "total_read_count_end = " << std::fixed << std::setprecision(2) << total_read_count_end << std::endl;
    // testing_result_file << "total_read_bytes_start = " << std::fixed << std::setprecision(2) << total_read_bytes_start << std::endl;
    // testing_result_file << "total_read_bytes_end = " << std::fixed << std::setprecision(2) << total_read_bytes_end << std::endl;
    // testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count_end - total_read_count_start) * 1.0 / N_repetitions << std::endl;
    // testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes_end - total_read_bytes_start) * 1.0 / N_repetitions << std::endl;
    testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count) * 1.0 / N_repetitions << std::endl;
    testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes) * 1.0 / N_repetitions << std::endl;
    print_perf_iostats_context(testing_result_file, N_repetitions);
  }


  testing_result_file << std::endl << std::endl;
  testing_result_file << "----------------------Testing On historic-existing Keys-----------------------" << std::endl;
  // system_verifier->resetDiskAccessCount();
  for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << std::endl;
    // long long total_read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
    // long long total_read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
    // reset_perf_iostats_context();
    long long total_read_count = 0;
    long long total_read_bytes = 0;
    testing_logger.reset();



    system_verifier->resetDiskAccessCount();
    system_verifier->setRDFTypeChosed(t);
    count = 0;
    point_query_time = 0;
    start_pq = std::chrono::high_resolution_clock::now();
    rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
    rocksdb::get_perf_context()->Reset();
    rocksdb::get_iostats_context()->Reset();
    for(auto i = 0; i < N_repetitions; i++){
      clearCache(op);
      // clearBlockCache(db, testing_result_file);
      // rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
      // rocksdb::get_perf_context()->Reset();
      // rocksdb::get_iostats_context()->Reset();
      // setNewBlockCacheForReading(op);
      // setNoBlockCacheForReading(op);
      // {
      //   s = db->Close();
      //   if (!s.ok()) std::cerr << s.ToString() << std::endl;
      //   assert(s.ok());
      //   // DB* db;
      //   init(&db, op, write_op, read_op);

      //   Status s = DB::Open(op, kDBPath, &db);
      //   if (!s.ok()) std::cerr << s.ToString() << std::endl;
      //   assert(s.ok());
      // }
      // long long read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
      //   + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
      // long long read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
      //   + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
      // reset_perf_iostats_context();
      // {
      //   rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
      //   rocksdb::get_perf_context()->Reset();
      //   rocksdb::get_iostats_context()->Reset();
      // }
      testing_logger.reopen_DB(&db, op, write_op, read_op);
      testing_logger.set_to_start(op);



  // system_verifier->setRDFTypeChosed(1);
      for(auto &x: system_verifier->getHistoricExistingKeys()){
        bool gt_is_exist = system_verifier->isKeyExist(x);
        std::string gt_value = system_verifier->get(x);

        std::string value;
        std::stringstream searching_key;
        searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;
        s = db->Get(read_op, searching_key.str(), &value);
        // testing_result_file << x << " " << s.ok() << " " << value << std::endl;
        // testing_result_file << x << " " << gt_is_exist << " " << gt_value << std::endl;
      
        if(s.ok() != gt_is_exist){
          testing_result_file << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
        }
        if(gt_is_exist == false){continue;}
        if(value != gt_value){
          testing_result_file << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
        }
      }
      count += system_verifier->getDiskAccessCount();
      stop_pq = std::chrono::high_resolution_clock::now();
      duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
      point_query_time += duration_pq.count();

// testing_result_file << i << " -----" << std::endl;    
// // long long total_read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
// //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
// // testing_result_file << "total_read_count_end = " << std::fixed << std::setprecision(2) << total_read_count_end << std::endl;
//     long long read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
//       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
//     long long read_bytes_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
//       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
      // total_read_count += read_count_start - read_count_end;
      // total_read_bytes += read_bytes_start - read_bytes_end;
//     reset_perf_iostats_context();

//     testing_result_file << "read_count_start = " << std::fixed << std::setprecision(2) << read_count_start << std::endl;
//     testing_result_file << "read_count_end = " << std::fixed << std::setprecision(2) << read_count_end << std::endl;
//     testing_result_file << "read_bytes_start = " << std::fixed << std::setprecision(2) << read_bytes_start << std::endl;
//     testing_result_file << "read_bytes_end = " << std::fixed << std::setprecision(2) << read_bytes_end << std::endl;
// testing_result_file << i << " -----" << std::endl;    

      testing_logger.set_to_end(op, testing_result_file);
      if(i == 0){
        testing_logger.reset();
      } 

  // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/1 << std::endl;
    }  
    // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/N_repetitions << std::endl;
    // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions << " (ms) " << std::endl;
  
    testing_logger.output_statistics(testing_result_file);
    
    // long long total_read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
    // long long total_read_bytes_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");

    // testing_result_file << "total_read_count_start = " << std::fixed << std::setprecision(2) << total_read_count_start << std::endl;
    // testing_result_file << "total_read_count_end = " << std::fixed << std::setprecision(2) << total_read_count_end << std::endl;
    // testing_result_file << "total_read_bytes_start = " << std::fixed << std::setprecision(2) << total_read_bytes_start << std::endl;
    // testing_result_file << "total_read_bytes_end = " << std::fixed << std::setprecision(2) << total_read_bytes_end << std::endl;
    // testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count_end - total_read_count_start) * 1.0 / N_repetitions << std::endl;
    // testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes_end - total_read_bytes_start) * 1.0 / N_repetitions << std::endl;
    testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count) * 1.0 / N_repetitions << std::endl;
    testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes) * 1.0 / N_repetitions << std::endl;
    print_perf_iostats_context(testing_result_file, N_repetitions);
  }

  testing_result_file << std::endl << std::endl;
  testing_result_file << "----------------------Testing On Currently Deleted Keys-----------------------" << std::endl;
  // system_verifier->resetDiskAccessCount();
  for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << std::endl;
    // long long total_read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
    // long long total_read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
    // reset_perf_iostats_context();
    long long total_read_count = 0;
    long long total_read_bytes = 0;
    testing_logger.reset();



    system_verifier->resetDiskAccessCount();
    system_verifier->setRDFTypeChosed(t);
    count = 0;
    point_query_time = 0;
    start_pq = std::chrono::high_resolution_clock::now();
    rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
    rocksdb::get_perf_context()->Reset();
    rocksdb::get_iostats_context()->Reset();
    for(auto i = 0; i < N_repetitions; i++){
      clearCache(op);
      // clearBlockCache(db, testing_result_file);
      // rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
      // rocksdb::get_perf_context()->Reset();
      // rocksdb::get_iostats_context()->Reset();
      // setNewBlockCacheForReading(op);
      // setNoBlockCacheForReading(op);
      // {
      //   s = db->Close();
      //   if (!s.ok()) std::cerr << s.ToString() << std::endl;
      //   assert(s.ok());
      //   // DB* db;
      //   init(&db, op, write_op, read_op);

      //   Status s = DB::Open(op, kDBPath, &db);
      //   if (!s.ok()) std::cerr << s.ToString() << std::endl;
      //   assert(s.ok());
      // }
      // long long read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
      //   + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
      // long long read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
      //   + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
      // reset_perf_iostats_context();
      // {
      //   rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
      //   rocksdb::get_perf_context()->Reset();
      //   rocksdb::get_iostats_context()->Reset();
      // }
      testing_logger.reopen_DB(&db, op, write_op, read_op);
      testing_logger.set_to_start(op);



  // system_verifier->setRDFTypeChosed(1);
      for(auto &x: system_verifier->getCurrentlyDeletedKeys()){
        bool gt_is_exist = system_verifier->isKeyExist(x);
        std::string gt_value = system_verifier->get(x);

        std::string value;
        std::stringstream searching_key;
        searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;
        s = db->Get(read_op, searching_key.str(), &value);
        // testing_result_file << x << " " << s.ok() << " " << value << std::endl;
        // testing_result_file << x << " " << gt_is_exist << " " << gt_value << std::endl;
      
        if(s.ok() != gt_is_exist){
          testing_result_file << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
        }
        if(gt_is_exist == false){continue;}
        if(value != gt_value){
          testing_result_file << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
        }
      }
      count += system_verifier->getDiskAccessCount();
      stop_pq = std::chrono::high_resolution_clock::now();
      duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
      point_query_time += duration_pq.count();

// testing_result_file << i << " -----" << std::endl;    
// // long long total_read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
// //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
// // testing_result_file << "total_read_count_end = " << std::fixed << std::setprecision(2) << total_read_count_end << std::endl;
//     long long read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
//       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
//     long long read_bytes_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
//       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
      // total_read_count += read_count_start - read_count_end;
      // total_read_bytes += read_bytes_start - read_bytes_end;
//     reset_perf_iostats_context();

//     testing_result_file << "read_count_start = " << std::fixed << std::setprecision(2) << read_count_start << std::endl;
//     testing_result_file << "read_count_end = " << std::fixed << std::setprecision(2) << read_count_end << std::endl;
//     testing_result_file << "read_bytes_start = " << std::fixed << std::setprecision(2) << read_bytes_start << std::endl;
//     testing_result_file << "read_bytes_end = " << std::fixed << std::setprecision(2) << read_bytes_end << std::endl;
// testing_result_file << i << " -----" << std::endl;    

      testing_logger.set_to_end(op, testing_result_file);
      if(i == 0){
        testing_logger.reset();
      }

  // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/1 << std::endl;
    }  
    // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/N_repetitions << std::endl;
    // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions << " (ms) " << std::endl;
  
    testing_logger.output_statistics(testing_result_file);
    
    // long long total_read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
    // long long total_read_bytes_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");

    // testing_result_file << "total_read_count_start = " << std::fixed << std::setprecision(2) << total_read_count_start << std::endl;
    // testing_result_file << "total_read_count_end = " << std::fixed << std::setprecision(2) << total_read_count_end << std::endl;
    // testing_result_file << "total_read_bytes_start = " << std::fixed << std::setprecision(2) << total_read_bytes_start << std::endl;
    // testing_result_file << "total_read_bytes_end = " << std::fixed << std::setprecision(2) << total_read_bytes_end << std::endl;
    // testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count_end - total_read_count_start) * 1.0 / N_repetitions << std::endl;
    // testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes_end - total_read_bytes_start) * 1.0 / N_repetitions << std::endl;
    testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count) * 1.0 / N_repetitions << std::endl;
    testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes) * 1.0 / N_repetitions << std::endl;
    print_perf_iostats_context(testing_result_file, N_repetitions);
  }

  testing_result_file << std::endl << std::endl;
  testing_result_file << "----------------------Testing On Currently Non-inserted Keys-----------------------" << std::endl;
  // system_verifier->resetDiskAccessCount();
  for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << std::endl;
    // long long total_read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
    // long long total_read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
    // reset_perf_iostats_context();
    long long total_read_count = 0;
    long long total_read_bytes = 0;
    testing_logger.reset();



    system_verifier->resetDiskAccessCount();
    system_verifier->setRDFTypeChosed(t);
    count = 0;
    point_query_time = 0;
    start_pq = std::chrono::high_resolution_clock::now();
    rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
    rocksdb::get_perf_context()->Reset();
    rocksdb::get_iostats_context()->Reset();
    for(auto i = 0; i < N_repetitions; i++){
      clearCache(op);
      // clearBlockCache(db, testing_result_file);
      // rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
      // rocksdb::get_perf_context()->Reset();
      // rocksdb::get_iostats_context()->Reset();
      // setNewBlockCacheForReading(op);
      // setNoBlockCacheForReading(op);
      // {
      //   s = db->Close();
      //   if (!s.ok()) std::cerr << s.ToString() << std::endl;
      //   assert(s.ok());
      //   // DB* db;
      //   init(&db, op, write_op, read_op);

      //   Status s = DB::Open(op, kDBPath, &db);
      //   if (!s.ok()) std::cerr << s.ToString() << std::endl;
      //   assert(s.ok());
      // }
      // long long read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
      //   + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
      // long long read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
      //   + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
      // reset_perf_iostats_context();
      // {
      //   rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
      //   rocksdb::get_perf_context()->Reset();
      //   rocksdb::get_iostats_context()->Reset();
      // }
      testing_logger.reopen_DB(&db, op, write_op, read_op);
      testing_logger.set_to_start(op);



  // system_verifier->setRDFTypeChosed(1);
      for(auto &x: system_verifier->getCurrentlyNonInsertedKeys(1000)){ // test on 1000 keys
        bool gt_is_exist = system_verifier->isKeyExist(x); // should be false
        std::string gt_value = system_verifier->get(x); // should be ""

        std::string value;
        std::stringstream searching_key;
        searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;
        s = db->Get(read_op, searching_key.str(), &value);
        // testing_result_file << x << " " << s.ok() << " " << value << std::endl;
        // testing_result_file << x << " " << gt_is_exist << " " << gt_value << std::endl;
      
        if(s.ok() != gt_is_exist){
          testing_result_file << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
        }
        if(gt_is_exist == false){continue;}
        if(value != gt_value){
          testing_result_file << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
        }
      }
      count += system_verifier->getDiskAccessCount();
      stop_pq = std::chrono::high_resolution_clock::now();
      duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
      point_query_time += duration_pq.count();

// testing_result_file << i << " -----" << std::endl;    
// // long long total_read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
// //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
// // testing_result_file << "total_read_count_end = " << std::fixed << std::setprecision(2) << total_read_count_end << std::endl;
//     long long read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
//       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
//     long long read_bytes_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
//       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
      // total_read_count += read_count_start - read_count_end;
      // total_read_bytes += read_bytes_start - read_bytes_end;
//     reset_perf_iostats_context();

//     testing_result_file << "read_count_start = " << std::fixed << std::setprecision(2) << read_count_start << std::endl;
//     testing_result_file << "read_count_end = " << std::fixed << std::setprecision(2) << read_count_end << std::endl;
//     testing_result_file << "read_bytes_start = " << std::fixed << std::setprecision(2) << read_bytes_start << std::endl;
//     testing_result_file << "read_bytes_end = " << std::fixed << std::setprecision(2) << read_bytes_end << std::endl;
// testing_result_file << i << " -----" << std::endl;    

      testing_logger.set_to_end(op, testing_result_file);
      if(i == 0){
        testing_logger.reset();
      }

  // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/1 << std::endl;
    }  
    // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/N_repetitions << std::endl;
    // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions << " (ms) " << std::endl;
  
    testing_logger.output_statistics(testing_result_file);
    
    // long long total_read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
    // long long total_read_bytes_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");

    // testing_result_file << "total_read_count_start = " << std::fixed << std::setprecision(2) << total_read_count_start << std::endl;
    // testing_result_file << "total_read_count_end = " << std::fixed << std::setprecision(2) << total_read_count_end << std::endl;
    // testing_result_file << "total_read_bytes_start = " << std::fixed << std::setprecision(2) << total_read_bytes_start << std::endl;
    // testing_result_file << "total_read_bytes_end = " << std::fixed << std::setprecision(2) << total_read_bytes_end << std::endl;
    // testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count_end - total_read_count_start) * 1.0 / N_repetitions << std::endl;
    // testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes_end - total_read_bytes_start) * 1.0 / N_repetitions << std::endl;
    testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count) * 1.0 / N_repetitions << std::endl;
    testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes) * 1.0 / N_repetitions << std::endl;
    print_perf_iostats_context(testing_result_file, N_repetitions);
  }

  testing_result_file << std::endl << std::endl;
  testing_result_file << "----------------------End Testing-----------------------" << std::endl;

  testing_result_file.close();

  
  // Status s = DB::Open(op, kDBPath, &db);
}


void runWorkload(DB* db, Options& op, WriteOptions& write_op, ReadOptions& read_op) {

  Status s = DB::Open(op, kDBPath, &db);
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());

  // opening workload file for the first time
  std::ifstream workload_file;
  workload_file.open("workload2.txt");
  assert(workload_file);
  // doing a first pass to get the workload size
  uint64_t workload_size = 0;
  std::string line;
  while (std::getline(workload_file, line)) ++workload_size;
  workload_file.close();

  workload_file.open("workload2.txt");
  assert(workload_file);

  checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();

  Iterator* it = db->NewIterator(read_op);  // for range reads
  uint64_t counter = 0;                     // for progress bar
  int KEY_SIZE = checking::SystemVerifier::getKeySize();

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

  
  std::this_thread::sleep_for(std::chrono::seconds(10));  // Sleep for 10 second
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

    // rocksdb::SetPerfLevel(rocksdb::PerfLevel::kDisable);
    // // std::string perf_context = rocksdb::get_perf_context()->ToString();
    
    // // long long get_read_bytes = parsing_value_from_string(op.statistics->ToString(), "get_read_bytes[^:]*= ([0-9]+)");
    // long long get_from_memtable_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), "get_from_memtable_time[^:]*= ([0-9]+)");
    // long long get_from_memtable_count = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), "get_from_memtable_count[^:]*= ([0-9]+)");
    // long long get_post_process_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), "get_post_process_time[^:]*= ([0-9]+)");

    // long long bloom_memtable_hit_count = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), "bloom_memtable_hit_count[^:]*= ([0-9]+)");
    // long long bloom_memtable_miss_count = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), "bloom_memtable_miss_count[^:]*= ([0-9]+)");
    // long long bloom_sst_hit_count = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), "bloom_sst_hit_count[^:]*= ([0-9]+)");
    // long long bloom_sst_miss_count = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), "bloom_sst_miss_count[^:]*= ([0-9]+)");



    // rocksdb::SetPerfLevel(rocksdb::PerfLevel::kDisable);
    // std::string iostats_context = rocksdb::get_iostats_context()->ToString();

    // long long bytes_read = parsing_value_from_string(iostats_context, "bytes_read[^:]*= ([0-9]+)");
    // long long bytes_written = parsing_value_from_string(iostats_context, "bytes_written[^:]*= ([0-9]+)");
    // long long read_nanos = parsing_value_from_string(iostats_context, "read_nanos[^:]*= ([0-9]+)");
    // long long write_nanos = parsing_value_from_string(iostats_context, "write_nanos[^:]*= ([0-9]+)");

    // //print out all the above variable
    // std::cout << "get_from_memtable_time = " << get_from_memtable_time << std::endl;
    // std::cout << "get_from_memtable_count = " << get_from_memtable_count << std::endl;
    // std::cout << "get_post_process_time = " << get_post_process_time << std::endl;
    // std::cout << "bloom_memtable_hit_count = " << bloom_memtable_hit_count << std::endl;
    // std::cout << "bloom_memtable_miss_count = " << bloom_memtable_miss_count << std::endl;
    // std::cout << "bloom_sst_hit_count = " << bloom_sst_hit_count << std::endl;
    // std::cout << "bloom_sst_miss_count = " << bloom_sst_miss_count << std::endl;
    // std::cout << "bytes_read = " << bytes_read << std::endl;
    // std::cout << "bytes_written = " << bytes_written << std::endl;
    // std::cout << "read_nanos = " << read_nanos << std::endl;
    // std::cout << "write_nanos = " << write_nanos << std::endl;
    // //print out a separation line
    // std::cout << "--------------------------------------------------------------------" << std::endl;
    print_perf_iostats_context(std::cout, 1);
  }



  
  printStats(db, op);


  std::this_thread::sleep_for(std::chrono::seconds(10));  // Sleep for 1 second
  {
    runPQVerification(db, op, write_op, read_op);
  }



 


  workload_file.close();
  s = db->Close();
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());
  delete db;
  std::cout << "\n----------------------Closing DB-----------------------"
            << std::endl;

  return;
}

int main() {
  Options options;
  WriteOptions write_op;
  ReadOptions read_op;

  DB* db;

  init(&db, options, write_op, read_op);
  runWorkload(db, options, write_op, read_op);
}
