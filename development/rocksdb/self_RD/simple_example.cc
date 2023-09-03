#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <chrono>
#include <regex>
#include <chrono>
#include <thread>
#include <cstdlib>


// #include "rocksdb/dbformat.h"
// #include "file/writable_file_writer.h"
// #include "options/cf_options.h"


#include "rocksdb/statistics.h"
#include "rocksdb/advanced_options.h"
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
std::mutex rdfilter::PLRDF::init_mutex;
rdfilter::PLRDF* rdfilter::PLRDF::plrdf_ptr; 



using namespace rocksdb;
std::string kDBPath = "/tmp/cs561_project1";
// std::string kDBPath = "/home/tan/cs561_project1";

void printStats(DB* db, Options& options);
void print_perf_iostats_context(std::ostream& ofile, int N_repetitions = 1);
void init(DB **db_ptr2, Options& op, WriteOptions& write_op, ReadOptions& read_op, int max_background_jobs);
long long parsing_value_from_string(std::string str, std::string pattern);
void reset_perf_iostats_context();



struct Params{
  int entry_size;
  double correlation;
  long long num_inserts;
  int rd_count;
  double selectivity;
  string workload_file_name;
  double insert_before_rangeDelete;
  bool gen_workload;
};




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
      int max_background_jobs = 0;
      init(db_ptr2, op, write_op, read_op, max_background_jobs);

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
          rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
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
      testing_result_file << i_round << " -----" << std::endl << std::endl;  
    }


    void output_statistics(std::ostream& testing_result_file, std::ostream& testing_result_file2, std::string prefix){
      testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count) * 1.0 / i_round << std::endl;
      testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes) * 1.0 / i_round << std::endl;
      testing_result_file << std::endl;
      print_perf_iostats_context(testing_result_file, i_round);
      testing_result_file << std::endl;

      testing_result_file2 << ",\"" << prefix << "Avg_read_count\" : " << std::fixed << std::setprecision(2) << (total_read_count) * 1.0 / i_round << std::endl;
      testing_result_file2 << ",\"" << prefix << "Avg_read_bytes\" : " << std::fixed << std::setprecision(2) << (total_read_bytes) * 1.0 / i_round << std::endl;
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



    rocksdb::SetPerfLevel(rocksdb::PerfLevel::kDisable);

    std::cout << std::endl;
    std::cout << "RocksDB perf_context : " << std::endl;
    std::string perf_context = rocksdb::get_perf_context()->ToString();
    std::cout << perf_context << std::endl;  
    std::cout << "----------------------------------------" << std::endl;


    std::cout << std::endl;
    std::cout << "RocksDB iostats_context : " << std::endl;
    std::string iostats_context = rocksdb::get_iostats_context()->ToString();
    std::cout << iostats_context << std::endl;  
    std::cout << "----------------------------------------" << std::endl;

    rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
 
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

void init(DB **db_ptr2, Options& op, WriteOptions& write_op, ReadOptions& read_op, int max_background_jobs,
          EmuEnv* _env){
          // Params &params){
  rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
  // rocksdb::get_perf_context()->Reset();
  // rocksdb::get_iostats_context()->Reset();
  
  checking::SystemVerifier::init();
  rdfilter::PLRDF::init();

  kDBPath = kDBPath + "/" + _env->workload_file_name;

  // Check if the directory exists
  if (std::system(("test -d " + kDBPath).c_str()) == 0) {
      // Remove the directory
      if (std::system(("rm -rf " + kDBPath).c_str()) == 0) {
          std::cout << "Existing directory removed: " << kDBPath << std::endl;
      } else {
          std::cerr << "Error removing directory" << std::endl;
      }
  } else {
      std::cout << "Directory does not exist: " << kDBPath << std::endl;
  }

  // Create the directory
  if (std::system(("mkdir -p " + kDBPath).c_str()) == 0) {
      std::cout << "Directory created: " << kDBPath << std::endl;
  } else {
      std::cerr << "Error creating directory" << std::endl;
  }



  // // op.write_buffer_size = 1024 * 256; // -> 256 kB    
  // // op.write_buffer_size = 1024 * 8; // -> 256 kB    
  // op.write_buffer_size = 256 * 1024; // -> 256 kB    
  // // op.max_background_jobs = 1;
  // op.max_background_jobs = max_background_jobs; // -> no background jobs, really???
  // op.level0_file_num_compaction_trigger = 1;
  // op.target_file_size_base = op.write_buffer_size; // -> same as buffer size
  // op.target_file_size_multiplier = 1;  // Same files size across levels
  // op.max_write_buffer_number = 1;      // 1 buffer in-memory
  // op.max_bytes_for_level_base = op.write_buffer_size; // same as write buffer size
  // op.max_bytes_for_level_multiplier = 5;  // T-ratio
  // op.num_levels = 11;
  // op.statistics = CreateDBStatistics();
  // op.create_if_missing = true;
  // // op.write_buffer_size = 8 * 1024 * 1024;
  // op.soft_pending_compaction_bytes_limit = 0;
  // op.hard_pending_compaction_bytes_limit = 0;

  // {
  //   op.memtable_factory =
  //       std::shared_ptr<VectorRepFactory>(new VectorRepFactory);
  //   op.allow_concurrent_memtable_write = false;
  // }

  // {
  //     // op.memtable_factory = std::shared_ptr<SkipListFactory>(new
  //     // SkipListFactory);
  // }

  // {
  //     // op.memtable_factory =
  //     // std::shared_ptr<MemTableRepFactory>(NewHashSkipListRepFactory());
  //     // op.allow_concurrent_memtable_write = false;
  // }

  // {
  //   // op.memtable_factory =
  //   // std::shared_ptr<MemTableRepFactory>(NewHashLinkListRepFactory());
  //   // op.allow_concurrent_memtable_write = false;
  // }


  // write_op.low_pri = true;

  

  // setNewBlockCacheForReading(op);
  setNoBlockCacheForReading(op);

  clearCache(op);


}

void reset_perf_iostats_context(){
  rocksdb::get_perf_context()->Reset();
  rocksdb::get_iostats_context()->Reset();
  rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
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


    long long block_read_count = parsing_value_from_string(perf_context, "block_read_count[^:]*= ([0-9]+)");
    long long block_read_byte = parsing_value_from_string(perf_context, "block_read_byte[^:]*= ([0-9]+)");
    long long block_read_time = parsing_value_from_string(perf_context, "block_read_time[^:]*= ([0-9]+)");
    long long block_read_cpu_time = parsing_value_from_string(perf_context, "block_read_cpu_time[^:]*= ([0-9]+)");
    long long index_block_read_count = parsing_value_from_string(perf_context, "index_block_read_count[^:]*= ([0-9]+)");
    long long filter_block_read_count = parsing_value_from_string(perf_context, "filter_block_read_count[^:]*= ([0-9]+)");
    long long compression_dict_block_read_count = parsing_value_from_string(perf_context, "compression_dict_block_read_count[^:]*= ([0-9]+)");
    long long get_read_bytes = parsing_value_from_string(perf_context, "get_read_bytes[^:]*= ([0-9]+)");
    long long read_index_block_nanos = parsing_value_from_string(perf_context, "read_index_block_nanos[^:]*= ([0-9]+)");
    long long read_filter_block_nanos = parsing_value_from_string(perf_context, "read_filter_block_nanos[^:]*= ([0-9]+)");
    long long internal_key_skipped_count = parsing_value_from_string(perf_context, "internal_key_skipped_count[^:]*= ([0-9]+)");
    long long internal_delete_skipped_count = parsing_value_from_string(perf_context, "internal_delete_skipped_count[^:]*= ([0-9]+)");
    long long internal_recent_skipped_count = parsing_value_from_string(perf_context, "internal_recent_skipped_count[^:]*= ([0-9]+)");
    long long internal_range_del_reseek_count = parsing_value_from_string(perf_context, "internal_range_del_reseek_count[^:]*= ([0-9]+)");


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
    

    ofile << "block_read_count = " << std::fixed << std::setprecision(2) << block_read_count * 1.0 / N_repetitions << std::endl;
    ofile << "block_read_byte = " << std::fixed << std::setprecision(2) << block_read_byte * 1.0 / N_repetitions << std::endl;
    ofile << "block_read_time = " << std::fixed << std::setprecision(2) << block_read_time * 1.0 / N_repetitions << std::endl;
    ofile << "block_read_cpu_time = " << std::fixed << std::setprecision(2) << block_read_cpu_time * 1.0 / N_repetitions << std::endl;
    ofile << "index_block_read_count = " << std::fixed << std::setprecision(2) << index_block_read_count * 1.0 / N_repetitions << std::endl;
    ofile << "filter_block_read_count = " << std::fixed << std::setprecision(2) << filter_block_read_count * 1.0 / N_repetitions << std::endl;
    ofile << "compression_dict_block_read_count = " << std::fixed << std::setprecision(2) << compression_dict_block_read_count * 1.0 / N_repetitions << std::endl;
    ofile << "get_read_bytes = " << std::fixed << std::setprecision(2) << get_read_bytes * 1.0 / N_repetitions << std::endl;
    ofile << "read_index_block_nanos = " << std::fixed << std::setprecision(2) << read_index_block_nanos * 1.0 / N_repetitions << std::endl;
    ofile << "read_filter_block_nanos = " << std::fixed << std::setprecision(2) << read_filter_block_nanos * 1.0 / N_repetitions << std::endl;
    ofile << "internal_key_skipped_count = " << std::fixed << std::setprecision(2) << internal_key_skipped_count * 1.0 / N_repetitions << std::endl;
    ofile << "internal_delete_skipped_count = " << std::fixed << std::setprecision(2) << internal_delete_skipped_count * 1.0 / N_repetitions << std::endl;
    ofile << "internal_recent_skipped_count = " << std::fixed << std::setprecision(2) << internal_recent_skipped_count * 1.0 / N_repetitions << std::endl;
    ofile << "internal_range_del_reseek_count = " << std::fixed << std::setprecision(2) << internal_range_del_reseek_count * 1.0 / N_repetitions << std::endl;


    ofile << "bytes_read = " << std::fixed << std::setprecision(2) << bytes_read * 1.0 / N_repetitions << std::endl;
    ofile << "bytes_written = " << std::fixed << std::setprecision(2) << bytes_written * 1.0 / N_repetitions << std::endl;
    ofile << "read_nanos = " << std::fixed << std::setprecision(2) << read_nanos * 1.0 / N_repetitions << std::endl;
    ofile << "write_nanos = " << std::fixed << std::setprecision(2) << write_nanos * 1.0 / N_repetitions << std::endl;
    //print out a separation line
    ofile << "--------------------------------------------------------------------" << std::endl;
}


void write_log2(std::ostream &outStream, EmuEnv* _env){
  outStream << ",\"T\" : " <<_env->size_ratio << std::endl;
  outStream << ",\"P\" : " <<_env->buffer_size_in_pages << std::endl;
  outStream << ",\"B\" : " <<_env->entries_per_page << std::endl;
  outStream << ",\"E\" : " <<_env->entry_size << std::endl;
  outStream << ",\"write_buffer_size\" : " <<_env->buffer_size << std::endl;
  // outStream << ",\"bits_per_key\" : " <<_env->bits_per_key << std::endl;
  outStream << ",\"correlation\" : " <<_env->correlation << std::endl;
  outStream << ",\"num_inserts\" : " <<_env->num_inserts << std::endl;
  outStream << ",\"rd_count\" : " <<_env->rd_count << std::endl;
  outStream << ",\"selectivity\" : " <<_env->selectivity << std::endl;
  outStream << ",\"workload_file_name\" : " << "\"" << _env->workload_file_name << "\"" << std::endl;
  outStream << ",\"insert_before_rangeDelete\" : " <<_env->insert_before_rangeDelete << std::endl;
  outStream << ",\"gen_workload\" : " <<_env->gen_workload << std::endl;
  outStream << ",\"max_background_jobs\" : " <<_env->max_background_jobs << std::endl;
  outStream << ",\"target_file_size_base\" : " <<_env->target_file_size_base << std::endl;
  outStream << ",\"target_file_size_multiplier\" : " <<_env->target_file_size_multiplier << std::endl;
  outStream << ",\"max_bytes_for_level_base\" : " <<_env->max_bytes_for_level_base << std::endl;
  // outStream << ",\"max_bytes_for_level_multiplier\" : " <<_env->max_bytes_for_level_multiplier << std::endl;
  outStream << ",\"num_levels\" : " <<_env->num_levels << std::endl;
  outStream << ",\"max_write_buffer_number\" : " <<_env->max_write_buffer_number << std::endl;
  outStream << ",\"level0_file_num_compaction_trigger\" : " <<_env->level0_file_num_compaction_trigger << std::endl;
}




void runPQVerification(DB** db_ptr2, Options& op, WriteOptions& write_op, ReadOptions& read_op, EmuEnv* _env){//Params &params){
  DB* db = *db_ptr2;
  Status s;

  checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();
  int KEY_SIZE = checking::SystemVerifier::getKeySize();


  system_verifier->enable_log__deleted_keys__max_sequnce_number();
  system_verifier->setRDFTypeChosed(0); // 0: NONE
  for(auto &x: system_verifier->getCurrentlyDeletedKeys()){
    std::string value;
    std::stringstream searching_key;
    searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;

    s = db->Get(read_op, searching_key.str(), &value);
  }
  system_verifier->disable_log__deleted_keys__max_sequnce_number();




  std::string testing_result_file_name = _env->workload_file_name + ".testing_log";
  std::string testing_result_file_name2 = _env->workload_file_name + ".testing_log2";
  std::ofstream testing_result_file, testing_result_file2;
  // testing_result_file.open("testing_result.txt");
  testing_result_file.open(testing_result_file_name);
  testing_result_file2.open(testing_result_file_name2);
  testing_result_file2 << "{"<< std::endl;
  testing_result_file2 << "\"start\" : \"start\""<< std::endl;
  write_log2(testing_result_file2, _env);
  TestingLogger testing_logger;

  testing_result_file << "PLRDF Number Of Total Ranges: " << db->getPLRDFNumberOfTotalRanges() << std::endl;
  testing_result_file << "Split PLRDF Number Of Total Ranges: " << db->getSplitPLRDFNumberOfTotalRanges() << std::endl;
  testing_result_file << "TopLevel RDF Number Of Total Ranges: " << db->getTopLevelRDFNumberOfTotalRanges() << std::endl;
  testing_result_file << "Skyline RDF Number Of Total Ranges: " << db->getSkylineRDFNumberOfTotalRanges() << std::endl;
  testing_result_file << std::endl;

  testing_result_file2 << ",\"PLRDF Number Of Total Ranges\" : " << db->getPLRDFNumberOfTotalRanges() << std::endl;
  testing_result_file2 << ",\"Split PLRDF Number Of Total Ranges\" : " << db->getSplitPLRDFNumberOfTotalRanges() << std::endl;
  testing_result_file2 << ",\"TopLevel RDF Number Of Total Ranges\" : " << db->getTopLevelRDFNumberOfTotalRanges() << std::endl;
  testing_result_file2 << ",\"Skyline RDF Number Of Total Ranges\" : " << db->getSkylineRDFNumberOfTotalRanges() << std::endl;

  
  
  vector<int> ranges_log_PLRDF = db->getLogOfNumbersOfRangesInPLRDF();
  vector<int> ranges_log_SplitPLRDF = db->getLogOfNumbersOfRangesInSplitPLRDF();
  vector<int> ranges_log_TopLevelRDF = db->getLogOfNumbersOfRangesInTopLevelRDF();
  vector<int> ranges_log_SkylineRDF = db->getLogOfNumbersOfRangesInSkylineRDF();
  testing_result_file2 << ",\"Log Of Numbers Of Ranges In PLRDF\" : [";
  for(int i = 0; i < ranges_log_PLRDF.size(); i++){
    testing_result_file2 << ranges_log_PLRDF[i];
    if(i != ranges_log_PLRDF.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;
  testing_result_file2 << ",\"Log Of Numbers Of Ranges In SplitPLRDF\" : [";
  for(int i = 0; i < ranges_log_SplitPLRDF.size(); i++){
    testing_result_file2 << ranges_log_SplitPLRDF[i];
    if(i != ranges_log_SplitPLRDF.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;
  testing_result_file2 << ",\"Log Of Numbers Of Ranges In TopLevelRDF\" : [";
  for(int i = 0; i < ranges_log_TopLevelRDF.size(); i++){
    testing_result_file2 << ranges_log_TopLevelRDF[i];
    if(i != ranges_log_TopLevelRDF.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;;
  testing_result_file2 << ",\"Log Of Numbers Of Ranges In SkylineRDF\" : [";
  for(int i = 0; i < ranges_log_SkylineRDF.size(); i++){
    testing_result_file2 << ranges_log_SkylineRDF[i];
    if(i != ranges_log_SkylineRDF.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;


  const long long N_repetitions = checking::SystemVerifier::EXPERIMENT_REPETITION_TIMES;
  testing_result_file << "N_repetitions = " << N_repetitions << std::endl << std::endl;
  testing_result_file2 << ",\"N_repetitions\" : " << N_repetitions << std::endl;
  testing_result_file2 << ",\"Total number of SST Files\" : " << db->getTotalNumberOfSSTFiles() << std::endl << std::endl;
  // long long num_RDF_types = getNumberOfRDFTypes();
  long long disk_access_count = 0;

  auto start_pq = std::chrono::high_resolution_clock::now();
  auto stop_pq = std::chrono::high_resolution_clock::now();
  auto duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
  unsigned long long point_query_time = duration_pq.count();

std::cout << "!!! Testing On Existing Keys " << std::endl;

//   testing_result_file << std::endl << std::endl;
//   testing_result_file << "----------------------Testing On Existing Keys-----------------------" << std::endl; 
//   // system_verifier->resetDiskAccessCount();
//   for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
//     // long long total_read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
//     //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
//     // long long total_read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
//     //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
//     // reset_perf_iostats_context();
//     long long total_read_count = 0;
//     long long total_read_bytes = 0;
//     testing_logger.reset();



//     // system_verifier->resetDiskAccessCount();
//     system_verifier->resetFilteredByRDFCount();
//     system_verifier->setRDFTypeChosed(t);
//     system_verifier->resetAllCount();
//     system_verifier->reset_total_duration__get_rdf();
//     system_verifier->reset_total_duration__get_max_seq();
//     system_verifier->reset_total_duration__retrieve_block();
//     system_verifier->reset_total_duration__remaining_get_path();
//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << std::endl;
//     disk_access_count = 0;
//     point_query_time = 0;
//     start_pq = std::chrono::high_resolution_clock::now();
//     rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
//     rocksdb::get_perf_context()->Reset();
//     rocksdb::get_iostats_context()->Reset();
//     for(auto i = 0; i < N_repetitions; i++){
//       clearCache(op);
//       // clearBlockCache(db, testing_result_file);
//       // rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
//       // rocksdb::get_perf_context()->Reset();
//       // rocksdb::get_iostats_context()->Reset();
//       // setNewBlockCacheForReading(op);
//       // setNoBlockCacheForReading(op);
//       // {
//       //   s = db->Close();
//       //   if (!s.ok()) std::cerr << s.ToString() << std::endl;
//       //   assert(s.ok());
//       //   // DB* db;
//       //   init(&db, op, write_op, read_op);

//       //   Status s = DB::Open(op, kDBPath, &db);
//       //   if (!s.ok()) std::cerr << s.ToString() << std::endl;
//       //   assert(s.ok());
//       // }
//       // long long read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
//       //   + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
//       // long long read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
//       //   + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
//       // reset_perf_iostats_context();
//       // {
//       //   rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
//       //   rocksdb::get_perf_context()->Reset();
//       //   rocksdb::get_iostats_context()->Reset();
//       // }

//       // testing_logger.reopen_DB(db_ptr2, op, write_op, read_op);
//       db = *db_ptr2;
//       // db->SetOptions({{"disable_auto_compactions", "true"}});
//       // db->printAllFileRanges();
//       // db->printPLRDF();    
//       system_verifier->resetDiskAccessCount();
//       testing_logger.set_to_start(op);

//   // system_verifier->setRDFTypeChosed(1);
//       for(auto &x: system_verifier->getAllExistingKeys()){
//         bool gt_is_exist = system_verifier->isKeyExist(x);
//         std::string gt_value = system_verifier->get(x);

//         std::string value;
//         std::string time_stamp;
//         std::stringstream searching_key;
//         searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;

//         start_pq = std::chrono::high_resolution_clock::now();
//         system_verifier->start_remaining_get_path();
//         s = db->Get(read_op, searching_key.str(), &value); 
//         system_verifier->stop_remaining_get_path(); 
//         stop_pq = std::chrono::high_resolution_clock::now();
//         duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
//         point_query_time += duration_pq.count();
//         size_t separator_pos = value.find("|");
//         time_stamp = value.substr(separator_pos + 1);
//         value = value.substr(0, separator_pos);
//         // std::cout << x << " " << s.ok() << " " << value << std::endl;
//         // std::cout << x << " " << gt_is_exist << " " << gt_value << std::endl;
      
//         if(s.ok() != gt_is_exist){
//           testing_result_file << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
//         }
//         if(gt_is_exist == false){continue;}
//         if(value != gt_value){
//           testing_result_file << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
//         }
//       }
//       disk_access_count += system_verifier->getDiskAccessCount();
//       // stop_pq = std::chrono::high_resolution_clock::now();
//       // duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
//       // point_query_time += duration_pq.count();

// // testing_result_file << i << " -----" << std::endl;    
// // // long long total_read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
// // //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
// // // testing_result_file << "total_read_count_end = " << std::fixed << std::setprecision(2) << total_read_count_end << std::endl;
// //     long long read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
// //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
// //     long long read_bytes_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
// //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
//       // total_read_count += read_count_start - read_count_end;
//       // total_read_bytes += read_bytes_start - read_bytes_end;
// //     reset_perf_iostats_context();

// //     testing_result_file << "read_count_start = " << std::fixed << std::setprecision(2) << read_count_start << std::endl;
// //     testing_result_file << "read_count_end = " << std::fixed << std::setprecision(2) << read_count_end << std::endl;
// //     testing_result_file << "read_bytes_start = " << std::fixed << std::setprecision(2) << read_bytes_start << std::endl;
// //     testing_result_file << "read_bytes_end = " << std::fixed << std::setprecision(2) << read_bytes_end << std::endl;
// // testing_result_file << i << " -----" << std::endl;    
//       testing_result_file << " Disk Access count = " << system_verifier->getDiskAccessCount() << std::endl;
//       testing_logger.set_to_end(op, testing_result_file);
//       // if(i == 0){
//       //   testing_logger.reset();
//       // }

//   // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/1 << std::endl;
//     }  
//     double block_read_cpu_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), ".*block_read_cpu_time = ([0-9.]+)");

//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*disk_access_count/N_repetitions << std::endl;
//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions/1e3 << " (ms) " << std::endl << std::endl;
//     testing_result_file << "filtered by RDF count = " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl; 
//     testing_result_file << "number of PQ = " << system_verifier->getAllExistingKeys().size() << std::endl;
//     testing_result_file << system_verifier->getAllCount("", "", "", N_repetitions) << std::endl;
//     testing_result_file << "block_read_cpu_time = " << 1.0*block_read_cpu_time/N_repetitions/1e3  << "" << std::endl;

//     std::string prefix = " (Exist Keys) " + system_verifier->getStringOfRDFTypeChosed() + " ";
//     testing_result_file2 << ",\"" + prefix + " elapsed time\" : " << 1.0*point_query_time/N_repetitions/1e3 << std::endl;
//     testing_result_file2 << ",\"" + prefix + " filtered by RDF count\" : " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl;
//     testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << system_verifier->getAllExistingKeys().size() << std::endl;
//     testing_result_file2 << system_verifier->getAllCount(",", "\"", prefix, N_repetitions) << std::endl;
//     testing_result_file2 << ",\"" + prefix + " block_read_cpu_time\" : " << 1.0*block_read_cpu_time/N_repetitions/1e3 << std::endl;

//     testing_logger.output_statistics(testing_result_file, testing_result_file2, prefix);


//     // long long total_read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")  
//     //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
//     // long long total_read_bytes_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
//     //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");

//     // testing_result_file << "total_read_count_start = " << std::fixed << std::setprecision(2) << total_read_count_start << std::endl;
//     // testing_result_file << "total_read_count_end = " << std::fixed << std::setprecision(2) << total_read_count_end << std::endl;
//     // testing_result_file << "total_read_bytes_start = " << std::fixed << std::setprecision(2) << total_read_bytes_start << std::endl;
//     // testing_result_file << "total_read_bytes_end = " << std::fixed << std::setprecision(2) << total_read_bytes_end << std::endl;
//     // testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count_end - total_read_count_start) * 1.0 / N_repetitions << std::endl;
//     // testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes_end - total_read_bytes_start) * 1.0 / N_repetitions << std::endl;
//     // testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count) * 1.0 / N_repetitions << std::endl;
//     // testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes) * 1.0 / N_repetitions << std::endl;
//     // print_perf_iostats_context(testing_result_file, N_repetitions);
//   }

// std::cout << "!!! Testing On historic-existing Keys " << std::endl;
// system_verifier->set_flag_testing_on_currently_deleted_keys();

//   testing_result_file << std::endl << std::endl;
//   testing_result_file << "----------------------Testing On historic-existing Keys-----------------------" << std::endl;
//   // system_verifier->resetDiskAccessCount();
//   for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
//     // long long total_read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
//     //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
//     // long long total_read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
//     //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
//     // reset_perf_iostats_context();
//     long long total_read_count = 0;
//     long long total_read_bytes = 0;
//     testing_logger.reset();



//     // system_verifier->resetDiskAccessCount();
//     system_verifier->resetFilteredByRDFCount();
//     system_verifier->setRDFTypeChosed(t);
//     system_verifier->resetAllCount();
//     system_verifier->reset_total_duration__get_rdf();
//     system_verifier->reset_total_duration__get_max_seq();
//     system_verifier->reset_total_duration__retrieve_block();
//     system_verifier->reset_total_duration__remaining_get_path();
//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << std::endl;
//     disk_access_count = 0;
//     point_query_time = 0;
//     start_pq = std::chrono::high_resolution_clock::now();
//     rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
//     rocksdb::get_perf_context()->Reset();
//     rocksdb::get_iostats_context()->Reset();
//     for(auto i = 0; i < N_repetitions; i++){
//       clearCache(op);
//       // clearBlockCache(db, testing_result_file);
//       // rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
//       // rocksdb::get_perf_context()->Reset();
//       // rocksdb::get_iostats_context()->Reset();
//       // setNewBlockCacheForReading(op);
//       // setNoBlockCacheForReading(op);
//       // {
//       //   s = db->Close();
//       //   if (!s.ok()) std::cerr << s.ToString() << std::endl;
//       //   assert(s.ok());
//       //   // DB* db;
//       //   init(&db, op, write_op, read_op);

//       //   Status s = DB::Open(op, kDBPath, &db);
//       //   if (!s.ok()) std::cerr << s.ToString() << std::endl;
//       //   assert(s.ok());
//       // }
//       // long long read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
//       //   + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
//       // long long read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
//       //   + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
//       // reset_perf_iostats_context();
//       // {
//       //   rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
//       //   rocksdb::get_perf_context()->Reset();
//       //   rocksdb::get_iostats_context()->Reset();
//       // }

//       // testing_logger.reopen_DB(db_ptr2, op, write_op, read_op);
//       db = *db_ptr2;
//       // db->SetOptions({{"disable_auto_compactions", "true"}});
//       // db->printAllFileRanges();
//       system_verifier->resetDiskAccessCount();
//       testing_logger.set_to_start(op);



//   // system_verifier->setRDFTypeChosed(1);
//       for(auto &x: system_verifier->getHistoricExistingKeys()){
//         bool gt_is_exist = system_verifier->isKeyExist(x);
//         std::string gt_value = system_verifier->get(x);

//         std::string value;
//         std::string time_stamp;
//         std::stringstream searching_key;
//         searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;

//         start_pq = std::chrono::high_resolution_clock::now();
//         system_verifier->start_remaining_get_path();
//         s = db->Get(read_op, searching_key.str(), &value);
//         system_verifier->stop_remaining_get_path(); 
//         stop_pq = std::chrono::high_resolution_clock::now();
//         duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
//         point_query_time += duration_pq.count();
//         size_t separator_pos = value.find("|");
//         time_stamp = value.substr(separator_pos + 1);
//         value = value.substr(0, separator_pos);
//         // testing_result_file << x << " " << s.ok() << " " << value << std::endl;
//         // testing_result_file << x << " " << gt_is_exist << " " << gt_value << std::endl;
      
//         if(s.ok() != gt_is_exist){
//           testing_result_file << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
//         }
//         if(gt_is_exist == false){continue;}
//         if(value != gt_value){
//           testing_result_file << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
//         }
//       }
//       disk_access_count += system_verifier->getDiskAccessCount();
//       // stop_pq = std::chrono::high_resolution_clock::now();
//       // duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
//       // point_query_time += duration_pq.count();

// // testing_result_file << i << " -----" << std::endl;    
// // // long long total_read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
// // //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
// // // testing_result_file << "total_read_count_end = " << std::fixed << std::setprecision(2) << total_read_count_end << std::endl;
// //     long long read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
// //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
// //     long long read_bytes_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
// //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
//       // total_read_count += read_count_start - read_count_end;
//       // total_read_bytes += read_bytes_start - read_bytes_end;
// //     reset_perf_iostats_context();

// //     testing_result_file << "read_count_start = " << std::fixed << std::setprecision(2) << read_count_start << std::endl;
// //     testing_result_file << "read_count_end = " << std::fixed << std::setprecision(2) << read_count_end << std::endl;
// //     testing_result_file << "read_bytes_start = " << std::fixed << std::setprecision(2) << read_bytes_start << std::endl;
// //     testing_result_file << "read_bytes_end = " << std::fixed << std::setprecision(2) << read_bytes_end << std::endl;
// // testing_result_file << i << " -----" << std::endl;    

//       testing_result_file << " Disk Access count = " << system_verifier->getDiskAccessCount() << std::endl;
//       testing_logger.set_to_end(op, testing_result_file);
//       // if(i == 0){
//       //   testing_logger.reset();
//       // } 

//   // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/1 << std::endl;
//     }  
//     double block_read_cpu_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), ".*block_read_cpu_time = ([0-9.]+)");

//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*disk_access_count/N_repetitions << std::endl;
//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions/1e3 << " (ms) " << std::endl << std::endl;
//     testing_result_file << "filtered by RDF count = " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl; 
//     testing_result_file << "number of PQ = " << system_verifier->getHistoricExistingKeys().size() << std::endl;
//     testing_result_file << system_verifier->getAllCount("", "", "", N_repetitions) << std::endl;
//     testing_result_file << "block_read_cpu_time = " << 1.0*block_read_cpu_time/N_repetitions/1e3  << "" << std::endl;

//     std::string prefix = " (Historcially Exist Keys) " + system_verifier->getStringOfRDFTypeChosed() + " ";
//     testing_result_file2 << ",\"" + prefix + " elapsed time\" : " << 1.0*point_query_time/N_repetitions/1e3 << std::endl;
//     testing_result_file2 << ",\"" + prefix + " filtered by RDF count\" : " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl;
//     testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << system_verifier->getHistoricExistingKeys().size() << std::endl;
//     testing_result_file2 << system_verifier->getAllCount(",", "\"", prefix, N_repetitions) << std::endl;
//     testing_result_file2 << ",\"" + prefix + " block_read_cpu_time\" : " << 1.0*block_read_cpu_time/N_repetitions/1e3 << std::endl;

//     testing_logger.output_statistics(testing_result_file, testing_result_file2, prefix);

    
//     // long long total_read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
//     //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
//     // long long total_read_bytes_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
//     //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");

//     // testing_result_file << "total_read_count_start = " << std::fixed << std::setprecision(2) << total_read_count_start << std::endl;
//     // testing_result_file << "total_read_count_end = " << std::fixed << std::setprecision(2) << total_read_count_end << std::endl;
//     // testing_result_file << "total_read_bytes_start = " << std::fixed << std::setprecision(2) << total_read_bytes_start << std::endl;
//     // testing_result_file << "total_read_bytes_end = " << std::fixed << std::setprecision(2) << total_read_bytes_end << std::endl;
//     // testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count_end - total_read_count_start) * 1.0 / N_repetitions << std::endl;
//     // testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes_end - total_read_bytes_start) * 1.0 / N_repetitions << std::endl;
//     // testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count) * 1.0 / N_repetitions << std::endl;
//     // testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes) * 1.0 / N_repetitions << std::endl;
//     // print_perf_iostats_context(testing_result_file, N_repetitions);
//   }
// system_verifier->reset_flag_testing_on_currently_deleted_keys();

std::cout << "!!! Testing On Currently Deleted Keys " << std::endl;
system_verifier->set_flag_testing_on_currently_deleted_keys();

  testing_result_file << std::endl << std::endl;
  testing_result_file << "----------------------Testing On Currently Deleted Keys-----------------------" << std::endl;

  // for(auto &x: system_verifier->getCurrentlyDeletedKeys()){
  //   testing_result_file << x << " " << std::endl;
  // }
  // testing_result_file << std::endl << std::endl;

  // system_verifier->resetDiskAccessCount();
  for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
    // long long total_read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
    // long long total_read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
    // reset_perf_iostats_context();
    long long total_read_count = 0;
    long long total_read_bytes = 0;
    testing_logger.reset();



    // system_verifier->resetDiskAccessCount();
    system_verifier->resetFilteredByRDFCount();
    system_verifier->setRDFTypeChosed(t);
    system_verifier->resetAllCount();
    system_verifier->reset_total_duration__get_rdf();
    system_verifier->reset_total_duration__get_max_seq();
    system_verifier->reset_total_duration__retrieve_block();
    system_verifier->reset_total_duration__remaining_get_path();
    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << std::endl;
    disk_access_count = 0;
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

      // testing_logger.reopen_DB(db_ptr2, op, write_op, read_op);
      db = *db_ptr2;
      // db->SetOptions({{"disable_auto_compactions", "true"}});
      // db->printAllFileRanges();
      system_verifier->resetDiskAccessCount();
      testing_logger.set_to_start(op);



  // system_verifier->setRDFTypeChosed(1);
      for(auto &x: system_verifier->getCurrentlyDeletedKeys()){
        bool gt_is_exist = system_verifier->isKeyExist(x);
        std::string gt_value = system_verifier->get(x);

        std::string value;
        std::string time_stamp;
        std::stringstream searching_key;
        searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;

        start_pq = std::chrono::high_resolution_clock::now();
        system_verifier->start_remaining_get_path();
        s = db->Get(read_op, searching_key.str(), &value);
        system_verifier->stop_remaining_get_path(); 
        stop_pq = std::chrono::high_resolution_clock::now();
        duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
        point_query_time += duration_pq.count();
        size_t separator_pos = value.find("|");
        time_stamp = value.substr(separator_pos + 1);
        value = value.substr(0, separator_pos);
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
      disk_access_count += system_verifier->getDiskAccessCount();
      // stop_pq = std::chrono::high_resolution_clock::now();
      // duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
      // point_query_time += duration_pq.count();

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

      testing_result_file << " Disk Access count = " << system_verifier->getDiskAccessCount() << std::endl;
      testing_logger.set_to_end(op, testing_result_file);
      // if(i == 0){
      //   testing_logger.reset();
      // }

  // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/1 << std::endl;
    }  
    double block_read_cpu_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), ".*block_read_cpu_time = ([0-9.]+)");

    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*disk_access_count/N_repetitions << std::endl;
    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions/1e3 << " (ms) " << std::endl << std::endl;
    testing_result_file << "filtered by RDF count = " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl; 
    testing_result_file << "number of PQ = " << system_verifier->getCurrentlyDeletedKeys().size() << std::endl;
    testing_result_file << system_verifier->getAllCount("", "", "", N_repetitions) << std::endl;
    testing_result_file << "block_read_cpu_time = " << 1.0*block_read_cpu_time/N_repetitions/1e3  << "" << std::endl;

    std::string prefix = " (Currently Deleted Keys) " + system_verifier->getStringOfRDFTypeChosed() + " ";
    testing_result_file2 << ",\"" + prefix + " elapsed time\" : " << 1.0*point_query_time/N_repetitions/1e3 << std::endl;
    testing_result_file2 << ",\"" + prefix + " filtered by RDF count\" : " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl;
    testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << system_verifier->getCurrentlyDeletedKeys().size() << std::endl;
    testing_result_file2 << system_verifier->getAllCount(",", "\"", prefix, N_repetitions) << std::endl;
    testing_result_file2 << ",\"" + prefix + " block_read_cpu_time\" : " << 1.0*block_read_cpu_time/N_repetitions/1e3 << std::endl;

    testing_logger.output_statistics(testing_result_file, testing_result_file2, prefix);

    
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
    // testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count) * 1.0 / N_repetitions << std::endl;
    // testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes) * 1.0 / N_repetitions << std::endl;
    // print_perf_iostats_context(testing_result_file, N_repetitions);
  }
system_verifier->reset_flag_testing_on_currently_deleted_keys();

std::cout << "!!! Testing On Currently Non-inserted Keys " << std::endl;

  testing_result_file << std::endl << std::endl;
  testing_result_file << "----------------------Testing On Currently Non-inserted Keys-----------------------" << std::endl;
  // system_verifier->resetDiskAccessCount();
  int CurrentlyNonInsertedKeysNum = 1000;
  testing_result_file << "number of currently non-inserted keys = " << CurrentlyNonInsertedKeysNum << std::endl;
  system_verifier->genCurrentlyNonInsertedKeys(CurrentlyNonInsertedKeysNum);
  for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
    // long long total_read_count_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
    // long long total_read_bytes_start = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
    //       + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
    // reset_perf_iostats_context();
    long long total_read_count = 0;
    long long total_read_bytes = 0;
    testing_logger.reset();



    // system_verifier->resetDiskAccessCount();
    system_verifier->resetFilteredByRDFCount();
    system_verifier->setRDFTypeChosed(t);
    system_verifier->resetAllCount();
    system_verifier->reset_total_duration__get_rdf();
    system_verifier->reset_total_duration__get_max_seq();
    system_verifier->reset_total_duration__retrieve_block();
    system_verifier->reset_total_duration__remaining_get_path();
    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << std::endl;
    disk_access_count = 0;
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

      // testing_logger.reopen_DB(db_ptr2, op, write_op, read_op);
      db = *db_ptr2;
      // db->SetOptions({{"disable_auto_compactions", "true"}});
      // db->printAllFileRanges();
      system_verifier->resetDiskAccessCount();
      testing_logger.set_to_start(op);



  // system_verifier->setRDFTypeChosed(1);
      for(auto &x: system_verifier->getCurrentlyNonInsertedKeys()){ // test on 1000 keys
        bool gt_is_exist = system_verifier->isKeyExist(x); // should be false
        std::string gt_value = system_verifier->get(x); // should be ""

        std::string value;
        std::string time_stamp;
        std::stringstream searching_key;
        searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;

        start_pq = std::chrono::high_resolution_clock::now();
        system_verifier->start_remaining_get_path();
        s = db->Get(read_op, searching_key.str(), &value);
        system_verifier->stop_remaining_get_path(); 
        stop_pq = std::chrono::high_resolution_clock::now();
        duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
        point_query_time += duration_pq.count();
        size_t separator_pos = value.find("|");
        time_stamp = value.substr(separator_pos + 1);
        value = value.substr(0, separator_pos);
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
      disk_access_count += system_verifier->getDiskAccessCount();
      // stop_pq = std::chrono::high_resolution_clock::now();
      // duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
      // point_query_time += duration_pq.count();

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

      testing_result_file << " Disk Access count = " << system_verifier->getDiskAccessCount() << std::endl;
      testing_logger.set_to_end(op, testing_result_file);
      // if(i == 0){
      //   testing_logger.reset();
      // }

  // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*count/1 << std::endl;
    }  
    double block_read_cpu_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), ".*block_read_cpu_time = ([0-9.]+)");

    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*disk_access_count/N_repetitions << std::endl;
    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions/1e3 << " (ms) " << std::endl << std::endl;
    testing_result_file << "filtered by RDF count = " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl; 
    testing_result_file << "number of PQ = " << system_verifier->getCurrentlyNonInsertedKeys().size() << std::endl;
    testing_result_file << system_verifier->getAllCount("", "", "", N_repetitions) << std::endl;
    testing_result_file << "block_read_cpu_time = " << 1.0*block_read_cpu_time/N_repetitions/1e3  << "" << std::endl;

    std::string prefix = " (Non-inserted Keys) " + system_verifier->getStringOfRDFTypeChosed() + " ";
    testing_result_file2 << ",\"" + prefix + " elapsed time\" : " << 1.0*point_query_time/N_repetitions/1e3 << std::endl;
    testing_result_file2 << ",\"" + prefix + " filtered by RDF count\" : " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl;
    testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << system_verifier->getCurrentlyNonInsertedKeys().size() << std::endl;
    testing_result_file2 << system_verifier->getAllCount(",", "\"", prefix, N_repetitions) << std::endl;
    testing_result_file2 << ",\"" + prefix + " block_read_cpu_time\" : " << 1.0*block_read_cpu_time/N_repetitions/1e3 << std::endl;

    testing_logger.output_statistics(testing_result_file, testing_result_file2, prefix);

    
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
    // testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count) * 1.0 / N_repetitions << std::endl;
    // testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes) * 1.0 / N_repetitions << std::endl;
    // print_perf_iostats_context(testing_result_file, N_repetitions);
  }

  testing_result_file << std::endl << std::endl;
  testing_result_file << "----------------------End Testing-----------------------" << std::endl;

  testing_result_file.close();

  
  testing_result_file2 << ",\"End\" : \"End\""<< std::endl;
  testing_result_file2 << "}"<< std::endl;
  testing_result_file2.close();
  // Status s = DB::Open(op, kDBPath, &db);
}


void runWorkload(DB* db, Options& op, WriteOptions& write_op, ReadOptions& read_op, 
                 EmuEnv* _env){
                //  Params &params) {

  string &workload_file_name = _env->workload_file_name;

  Status s = DB::Open(op, kDBPath, &db);
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());

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

  std::this_thread::sleep_for(std::chrono::seconds(60));  // Sleep for 10 second

  std::cout << "!!! After sleep." << std::endl;


  db->printAllFileRanges();


  printStats(db, op);



  s = db->SetOptions({{"disable_auto_compactions", "true"}}); // is there any compaction happended after this????
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());
  std::cout << "!!! Disable auto compaction" << std::endl; 

  


  std::this_thread::sleep_for(std::chrono::seconds(60));  // Sleep for 10 second

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


  std::cout << "!!! several gets done " << std::endl;

  std::cout << "!!! print stats " << std::endl;

  
  printStats(db, op);

  std::cout << "!!! runQPVerification start " << std::endl;

  
  vector<int> ranges_log_PLRDF = db->getLogOfNumbersOfRangesInPLRDF();
  vector<int> ranges_log_SplitPLRDF = db->getLogOfNumbersOfRangesInSplitPLRDF();
  vector<int> ranges_log_TopLevelRDF = db->getLogOfNumbersOfRangesInTopLevelRDF();
  vector<int> ranges_log_SkylineRDF = db->getLogOfNumbersOfRangesInSkylineRDF();

  std::cout << "Ranges Log Of PLRDF: " << std::endl;
  for (int i = 0; i < ranges_log_PLRDF.size(); i++) {
    std::cout << ranges_log_PLRDF[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "Ranges Log Of SplitPLRDF: " << std::endl;
  for (int i = 0; i < ranges_log_SplitPLRDF.size(); i++) {
    std::cout << ranges_log_SplitPLRDF[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "Ranges Log Of TopLevelRDF: " << std::endl;
  for (int i = 0; i < ranges_log_TopLevelRDF.size(); i++) {
    std::cout << ranges_log_TopLevelRDF[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "Ranges Log Of SkylineRDF: " << std::endl;
  for (int i = 0; i < ranges_log_SkylineRDF.size(); i++) {
    std::cout << ranges_log_SkylineRDF[i] << " ";
  }
  std::cout << std::endl;




  std::chrono::_V2::system_clock::time_point  timer_start = std::chrono::high_resolution_clock::now();
  std::chrono::_V2::system_clock::time_point  timer_end = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timer_end - timer_start);

  std::chrono::_V2::system_clock::time_point  timer_start__testing = std::chrono::high_resolution_clock::now();
  std::chrono::_V2::system_clock::time_point  timer_end__testing = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds duration_ns__testing = std::chrono::duration_cast<std::chrono::nanoseconds>(timer_end__testing - timer_start__testing);



  std::cout << "Testing 10000 start,end duration = " << std::endl;
  checking::SystemVerifier::getSystemVerifier()->reset_total_duration__remaining_get_path();
  // checking::SystemVerifier::getSystemVerifier()->start_remaining_get_path();
  timer_start = std::chrono::high_resolution_clock::now();
  for(int i_timing = 0; i_timing < 10000; i_timing++){
    checking::SystemVerifier::getSystemVerifier()->start_remaining_get_path();
    checking::SystemVerifier::getSystemVerifier()->stop_remaining_get_path();
  }
  timer_end = std::chrono::high_resolution_clock::now();
  // checking::SystemVerifier::getSystemVerifier()->stop_remaining_get_path();
  unsigned long long total_duration__remaining_get_path = checking::SystemVerifier::getSystemVerifier()->get_total_duration__remaining_get_path();
  std::cout << "total_duration__remaining_get_path = " << total_duration__remaining_get_path << std::endl;

  duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timer_end - timer_start);
  std::cout << "timer duration1 = " << duration_ns.count() << std::endl;


  checking::SystemVerifier *system_verifier_timing = checking::SystemVerifier::getSystemVerifier();
  system_verifier_timing->reset_total_duration__remaining_get_path();
  timer_start = std::chrono::high_resolution_clock::now();
  // system_verifier_timing->start_remaining_get_path();
  for(int i_timing = 0; i_timing < 10000; i_timing++){
    system_verifier_timing->start_remaining_get_path();
    system_verifier_timing->stop_remaining_get_path();
  }
  timer_end = std::chrono::high_resolution_clock::now();
  // system_verifier_timing->stop_remaining_get_path();
  total_duration__remaining_get_path = system_verifier_timing->get_total_duration__remaining_get_path();
  std::cout << "total_duration__remaining_get_path = " << total_duration__remaining_get_path << std::endl;

  duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timer_end - timer_start);
  std::cout << "timer duration2 = " << duration_ns.count() << std::endl;


  unsigned long long sum_duration__testing = 0;
  timer_start = std::chrono::high_resolution_clock::now();
  for(int i_timing = 0; i_timing < 10000; i_timing++){
    timer_start__testing = std::chrono::high_resolution_clock::now();
    timer_end__testing = std::chrono::high_resolution_clock::now();
    duration_ns__testing = std::chrono::duration_cast<std::chrono::nanoseconds>(timer_end__testing - timer_start__testing);
    sum_duration__testing += duration_ns__testing.count();
  }
  timer_end = std::chrono::high_resolution_clock::now();
  std::cout << "total_duration__testing_chrono_clock = " << sum_duration__testing  << std::endl;

  duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timer_end - timer_start);
  std::cout << "timer duration3 = " << duration_ns.count() << std::endl;

  sum_duration__testing = 0;
  duration_ns__testing = std::chrono::duration_cast<std::chrono::nanoseconds>(timer_start__testing - timer_start__testing);
  timer_start = std::chrono::high_resolution_clock::now();
  for(int i_timing = 0; i_timing < 10000; i_timing++){
    timer_start__testing = std::chrono::high_resolution_clock::now();
    timer_end__testing = std::chrono::high_resolution_clock::now();
    duration_ns__testing += std::chrono::duration_cast<std::chrono::nanoseconds>(timer_end__testing - timer_start__testing);
    // sum_duration__testing += duration_ns__testing.count();
  }
  timer_end = std::chrono::high_resolution_clock::now();
  std::cout << "total_duration__testing_chrono_clock = " << duration_ns__testing.count()  << std::endl;

  duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timer_end - timer_start);
  std::cout << "timer duration4 = " << duration_ns.count() << std::endl;


  //CPU Timing
  std::clock_t c_start_testing = std::clock();
  std::clock_t c_end_testing = std::clock();
  std::clock_t c_start = std::clock();
  std::clock_t c_end = std::clock();
  double total_cpu_time_ns = 0;
  timer_start = std::chrono::high_resolution_clock::now();
  c_start_testing = std::clock();
  for(int i_timing = 0; i_timing < 10000; i_timing++){
    c_start = std::clock();
    c_end = std::clock();
    total_cpu_time_ns +=  1e9 * (c_end - c_start) / CLOCKS_PER_SEC;
  }
  c_end_testing = std::clock();
  timer_end = std::chrono::high_resolution_clock::now();

  std::cout << "total_cpu_time_ns = " << total_cpu_time_ns << std::endl;
  std::cout << "whole block total_cpu_time_ns = " << 1e9 * (c_end_testing - c_start_testing) / CLOCKS_PER_SEC << std::endl;

  duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timer_end - timer_start);
  std::cout << "timer duration5 = " << duration_ns.count() << std::endl;



  std::this_thread::sleep_for(std::chrono::seconds(10));  // Sleep for 1 second
  {
    std::clog << "Press Enter to continue...";
    std::cin.ignore(); // Waits for user to press Enter key
    runPQVerification(&db, op, write_op, read_op, _env);
  }

  std::cout << "!!! runQPVerification done " << std::endl;



 


  workload_file.close();
  s = db->Close();
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());
  delete db;
  std::cout << "\n----------------------Closing DB-----------------------"
            << std::endl;

  return;
}




// Params parse_input(int argc, char *argvx[]){

//   args::ArgumentParser parser("RDF Emulator", "");

//   args::Group group1(parser, "This group is all exclusive:", args::Group::Validators::DontCare);


//   args::ValueFlag<int> entry_size_cmd(group1, "E", "Entry size in bytes [def: 128 B]", {'E', "entry_size"});
//   args::ValueFlag<double> cor_cmd(group1, "#correlation", "Correlation between sort key and delete key [def: 0]", {'c', "correlation"});
//   args::ValueFlag<long long> num_inserts_cmd(group1, "#inserts", "The number of unique inserts to issue in the experiment [def: 0]", {'i', "num_inserts"});
//   args::ValueFlag<int> RD_cmd(group1, "range_delete", "Count of range delete [def:1]", {'R', "RD"});
//   args::ValueFlag<double> selectivity_cmd(group1, "selectivity_of_range_delete", "Selectivity of range delete [def:0.001]", {"z", "selectivity"});
//   args::ValueFlag<string> workload_filename_cmd(group1, "workload_filename", "workload filename [def:0.001]", {"f", "workload_filename"});
//   args::ValueFlag<double> insert_before_range_delete_cmd(group1, "insert_before_range_delete", "percent of inserts in insert themself that precede any of the range delete [def:0.5]", {"b", "insert_before_range_delete"});
//   args::ValueFlag<int> gen_workload_cmd(group1, "gen_workload", "want to generate workload [def:1]", {"g", "gen_workload"});



//   try {
//       parser.ParseCLI(argc, argvx);
//   } catch (args::Help&) {
//       std::cout << parser;
//       exit(1);
//   } catch (args::ParseError& e) {
//       std::cerr << e.what() << std::endl;
//       std::cerr << parser;
//       exit(1);
//   } catch (args::ValidationError& e) {
//       std::cerr << e.what() << std::endl;
//       std::cerr << parser;
//       exit(1);
//   }



//   int entry_size = entry_size_cmd ? args::get(entry_size_cmd) : 128;
//   double correlation = cor_cmd ? args::get(cor_cmd) : 0;
//   long long num_inserts = num_inserts_cmd ? args::get(num_inserts_cmd) : 0;
//   int rd_count = RD_cmd ? args::get(RD_cmd) : 1;
//   double selectivity = selectivity_cmd ? args::get(selectivity_cmd) : 0.001;
//   string workload_file_name = workload_filename_cmd ? args::get(workload_filename_cmd) : "workload.txt";
//   double insert_before_rangeDelete = insert_before_range_delete_cmd ? args::get(insert_before_range_delete_cmd) : 0.5;
//   bool gen_workload = gen_workload_cmd ? (args::get(gen_workload_cmd) != 0) : 1;


//   Params params;
//   params.entry_size = entry_size;
//   params.correlation = correlation;
//   params.num_inserts = num_inserts;
//   params.rd_count = rd_count;
//   params.selectivity = selectivity;
//   params.workload_file_name = workload_file_name;
//   params.insert_before_rangeDelete = insert_before_rangeDelete;
//   params.gen_workload = gen_workload;

//   return params;
// }

void gen_workload(EmuEnv* _env){


  int entry_size = _env->entry_size;
  double correlation = _env->correlation;
  long long num_inserts = _env->num_inserts;
  int rd_count = _env->rd_count;
  double selectivity = _env->selectivity;
  string workload_file_name = _env->workload_file_name;
  double insert_before_rangeDelete = _env->insert_before_rangeDelete;

// cout << "sizeof(int) = " << sizeof(int) << endl;
// cout << "sizeof(long) = " << sizeof(long) << endl;
// cout << "sizeof(long long) = " << sizeof(long long) << endl;
 
  
    
  WorkloadGenerator workload_generator;
// cout << "num_inserts = " << num_inserts << endl;
// cout << "rd_count = " << rd_count << endl;
// cout << "selectivity = " << selectivity << endl;
// cout << "insertBeforeRangeDelete = " << insertBeforeRangeDelete << endl;
  long number_Of_point_in_the_beginning = (long) ceil(num_inserts * insert_before_rangeDelete);
  // string workloadFilename = _workloadFilename;
// cout << "num_inserts*1.0 = " << num_inserts*1.0 << endl;
// cout << "rd_count*selectivity = " << rd_count*selectivity << endl; 
  assert(1.0*rd_count*selectivity <= 1.0);
  workload_generator.generateWorkload((long)num_inserts, (long)entry_size, (double) correlation, 
          (long)rd_count, (double) selectivity, (long) number_Of_point_in_the_beginning, (string) workload_file_name,   
          (int) checking::SystemVerifier::getKeySize()
          );    

  std::cout << "Workload Generated!" << std::endl;
}


int parse_arguments2(int argc, char *argv[], EmuEnv* _env) {
  args::ArgumentParser parser("RocksDB_parser.", "");

  args::Group group1(parser, "This group is all exclusive:", args::Group::Validators::DontCare);
/*
  args::Group group1(parser, "This group is all exclusive:", args::Group::Validators::AtMostOne);
  args::Group group2(parser, "Path is needed:", args::Group::Validators::All);
  args::Group group3(parser, "This group is all exclusive (either N or L):", args::Group::Validators::Xor);
  args::Group group4(parser, "Optional switches and parameters:", args::Group::Validators::DontCare);
  args::Group group5(parser, "Optional less frequent switches and parameters:", args::Group::Validators::DontCare);
*/

  args::ValueFlag<int> destroy_database_cmd(group1, "d", "Destroy and recreate the database [def: 1]", {'d', "destroy"});
  args::ValueFlag<int> clear_system_cache_cmd(group1, "cc", "Clear system cache [def: 1]", {"cc"}); // !YBS-sep09-XX!

  args::ValueFlag<int> size_ratio_cmd(group1, "T", "The number of unique inserts to issue in the experiment [def: 10]", {'T', "size_ratio"});
  args::ValueFlag<int> buffer_size_in_pages_cmd(group1, "P", "The number of unique inserts to issue in the experiment [def: 4096]", {'P', "buffer_size_in_pages"});
  args::ValueFlag<int> entries_per_page_cmd(group1, "B", "The number of unique inserts to issue in the experiment [def: 4]", {'B', "entries_per_page"});
  args::ValueFlag<int> entry_size_cmd(group1, "E", "The number of unique inserts to issue in the experiment [def: 1024 B]", {'E', "entry_size"});
  args::ValueFlag<long> buffer_size_cmd(group1, "M", "The number of unique inserts to issue in the experiment [def: 16 MB]", {'M', "memory_size"});
  args::ValueFlag<int> file_to_memtable_size_ratio_cmd(group1, "file_to_memtable_size_ratio", "The number of unique inserts to issue in the experiment [def: 1]", {'f', "file_to_memtable_size_ratio"});
  args::ValueFlag<long> file_size_cmd(group1, "file_size", "The number of unique inserts to issue in the experiment [def: 256 KB]", {'F', "file_size"});
  args::ValueFlag<int> verbosity_cmd(group1, "verbosity", "The verbosity level of execution [0,1,2; def: 0]", {'V', "verbosity"});
  args::ValueFlag<int> compaction_pri_cmd(group1, "compaction_pri", "[Compaction priority: 1 for kMinOverlappingRatio, 2 for kByCompensatedSize, 3 for kOldestLargestSeqFirst, 4 for kOldestSmallestSeqFirst; def: 1]", {'c', "compaction_pri"});
  args::ValueFlag<int> compaction_style_cmd(group1, "compaction_style", "[Compaction priority: 1 for kCompactionStyleLevel, 2 for kCompactionStyleUniversal, 3 for kCompactionStyleFIFO, 4 for kCompactionStyleNone; def: 1]", {'C', "compaction_style"}); // !YBS-sep07-XX!
  args::ValueFlag<int> bits_per_key_cmd(group1, "bits_per_key", "The number of bits per key assigned to Bloom filter [def: 10]", {'b', "bits_per_key"});
  args::ValueFlag<int> block_cache_cmd(group1, "bb", "Block cache size in MB [def: 8 MB]", {"bb"}); // !YBS-sep09-XX!
  args::ValueFlag<int> show_progress_cmd(group1, "show_progress", "Show progress [def: 0]", {'s', "sp"}); // !YBS-sep17-XX!

  args::ValueFlag<long> num_inserts_cmd(group1, "inserts", "The number of unique inserts to issue in the experiment [def: 0]", {'i', "inserts"});





  //YuCheng Added Start
  // args::ValueFlag<int> entry_size_cmd(group1, "E", "Entry size in bytes [def: 128 B]", {'E', "entry_size"});
  args::ValueFlag<double> cor_cmd(group1, "#correlation", "Correlation between sort key and delete key [def: 0]", {"correlation"});
  // args::ValueFlag<long long> num_inserts_cmd(group1, "#inserts", "The number of unique inserts to issue in the experiment [def: 0]", {'i', "num_inserts"});
  args::ValueFlag<int> RD_cmd(group1, "range_delete", "Count of range delete [def:1]", {'R', "RD"});
  args::ValueFlag<double> selectivity_cmd(group1, "selectivity_of_range_delete", "Selectivity of range delete [def:0.001]", {"selectivity"});
  args::ValueFlag<string> workload_filename_cmd(group1, "workload_filename", "workload filename [def:0.001]", {"workload_filename"});
  args::ValueFlag<double> insert_before_range_delete_cmd(group1, "insert_before_range_delete", "percent of inserts in insert themself that precede any of the range delete [def:0.5]", {"insert_before_range_delete"});
  args::ValueFlag<int> gen_workload_cmd(group1, "gen_workload", "want to generate workload [def:1]", {"gen_workload"});
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

  _env->destroy_database = destroy_database_cmd ? args::get(destroy_database_cmd) : 1;
  _env->clear_system_cache = clear_system_cache_cmd ? args::get(clear_system_cache_cmd) : 1; // !YBS-sep09-XX!

  _env->size_ratio = size_ratio_cmd ? args::get(size_ratio_cmd) : 10;
  _env->buffer_size_in_pages = buffer_size_in_pages_cmd ? args::get(buffer_size_in_pages_cmd) : 4096;
  _env->entries_per_page = entries_per_page_cmd ? args::get(entries_per_page_cmd) : 4;
  _env->entry_size = entry_size_cmd ? args::get(entry_size_cmd) : 1024;
  _env->buffer_size = buffer_size_cmd ? args::get(buffer_size_cmd) : _env->buffer_size_in_pages * _env->entries_per_page * _env->entry_size;
  _env->file_to_memtable_size_ratio = file_to_memtable_size_ratio_cmd ? args::get(file_to_memtable_size_ratio_cmd) : 1;
  _env->file_size = file_size_cmd ? args::get(file_size_cmd) : _env->buffer_size;
  _env->verbosity = verbosity_cmd ? args::get(verbosity_cmd) : 0;
  _env->compaction_pri = compaction_pri_cmd ? args::get(compaction_pri_cmd) : 1;
  _env->compaction_style = compaction_style_cmd ? args::get(compaction_style_cmd) : 1; // !YBS-sep07-XX!
  _env->bits_per_key = bits_per_key_cmd ? args::get(bits_per_key_cmd) : 10;
  _env->block_cache = block_cache_cmd ? args::get(block_cache_cmd) : 8; // !YBS-sep09-XX!
  _env->show_progress = show_progress_cmd ? args::get(show_progress_cmd) : 0; // !YBS-sep17-XX!

  _env->num_inserts = num_inserts_cmd ? args::get(num_inserts_cmd) : 0;

  _env->target_file_size_base = _env->buffer_size; // !YBS-sep07-XX! <-----------
  // _env->max_bytes_for_level_base = _env->buffer_size * _env->size_ratio; // !YBS-sep07-XX!
  // _env->max_bytes_for_level_base = _env->buffer_size; // <-------------------------------------------------------------
  // op->max_bytes_for_level_base = _env->buffer_size; // <-------------------------------------------------------------





  //YuCheng Added Start
  // int entry_size = entry_size_cmd ? args::get(entry_size_cmd) : 128;
  double correlation = cor_cmd ? args::get(cor_cmd) : 0;
  // long long num_inserts = num_inserts_cmd ? args::get(num_inserts_cmd) : 0;
  int rd_count = RD_cmd ? args::get(RD_cmd) : 1;
  double selectivity = selectivity_cmd ? args::get(selectivity_cmd) : 0.001;
  string workload_file_name = workload_filename_cmd ? args::get(workload_filename_cmd) : "workload.txt";
  double insert_before_rangeDelete = insert_before_range_delete_cmd ? args::get(insert_before_range_delete_cmd) : 0.5;
  bool gen_workload = gen_workload_cmd ? (args::get(gen_workload_cmd) != 0) : 1;


  // _env->entry_size = entry_size;
  _env->correlation = correlation;
  // _env.num_inserts = num_inserts;
  _env->rd_count = rd_count;
  _env->selectivity = selectivity;
  _env->workload_file_name = workload_file_name;
  _env->insert_before_rangeDelete = insert_before_rangeDelete;
  _env->gen_workload = gen_workload;
  //YuCheng Added End
  return 0;
}


void configOptions(EmuEnv* _env, Options *op, BlockBasedTableOptions *t_op, WriteOptions *w_op, ReadOptions *r_op, FlushOptions *f_op) {
    // *op = Options();
    op->statistics = CreateDBStatistics(); // !YBS-sep01-XX!
    op->write_buffer_size = _env->buffer_size; // !YBS-sep07-XX!
    op->max_write_buffer_number = _env->max_write_buffer_number;   // min 2 // !YBS-sep07-XX!
    

  // op.write_buffer_size = 1024 * 256; // -> 256 kB    
  // op.write_buffer_size = 1024 * 8; // -> 256 kB    
  // op.write_buffer_size = 256 * 1024; // -> 256 kB    
  // op.max_background_jobs = 1;
  // op.max_background_jobs = 1; // -> no background jobs, really???
  // op.level0_file_num_compaction_trigger = 1;
  // op.target_file_size_base = op.write_buffer_size; // -> same as buffer size
  // op.target_file_size_multiplier = 1;  // Same files size across levels
  // op.max_write_buffer_number = 1;      // 1 buffer in-memory
  // op.max_bytes_for_level_base = op.write_buffer_size; // same as write buffer size
  // op.max_bytes_for_level_multiplier = 5;  // T-ratio
  // op.num_levels = 11;
  // op.statistics = CreateDBStatistics();
  // op.create_if_missing = true;
  // op.write_buffer_size = 8 * 1024 * 1024;
  // op.soft_pending_compaction_bytes_limit = 0;
  // op.hard_pending_compaction_bytes_limit = 0;

  // write_op.low_pri = true;




    switch (_env->memtable_factory) {
      case 1:
        op->memtable_factory = std::shared_ptr<SkipListFactory>(new SkipListFactory); break;
      case 2:
        op->memtable_factory = std::shared_ptr<VectorRepFactory>(new VectorRepFactory); break;
      case 3:
        op->memtable_factory.reset(NewHashSkipListRepFactory()); break;
      case 4:
        op->memtable_factory.reset(NewHashLinkListRepFactory()); break;
      default:
        std::cerr << "error: memtable_factory" << std::endl;
    }

    // Compaction
    switch (_env->compaction_pri) {
      case 1:
        op->compaction_pri = kMinOverlappingRatio; break;
      case 2:
        op->compaction_pri = kByCompensatedSize; break;
      case 3:
        op->compaction_pri = kOldestLargestSeqFirst; break;
      case 4:
        op->compaction_pri = kOldestSmallestSeqFirst; break;
      // case 5: 
        // op->compaction_pri = kFADE; break;
      case 6: // !YBS-sep06-XX!
        op->compaction_pri = kRoundRobin; break; // !YBS-sep06-XX!
      // case 7: // !YBS-sep07-XX!
        // op->compaction_pri = kMinOverlappingGrandparent; break; // !YBS-sep07-XX!
      // case 8: // !YBS-sep08-XX!
        // op->compaction_pri = kFullLevel; break; // !YBS-sep08-XX!
      default:
        std::cerr << "ERROR: INVALID Data movement policy!" << std::endl;
    }

    op->max_bytes_for_level_multiplier = _env->size_ratio;
    op->allow_concurrent_memtable_write = _env->allow_concurrent_memtable_write;
    op->create_if_missing = _env->create_if_missing;
    op->target_file_size_base = _env->target_file_size_base;
    // op->target_file_size_base = _env->buffer_size; // <------------------------------------------
    op->level_compaction_dynamic_level_bytes = _env->level_compaction_dynamic_level_bytes;
    switch (_env->compaction_style) {
      case 1:
        op->compaction_style = kCompactionStyleLevel; break;
      case 2:
        op->compaction_style = kCompactionStyleUniversal; break;
      case 3:
        op->compaction_style = kCompactionStyleFIFO; break;
      case 4:
        op->compaction_style = kCompactionStyleNone; break;
      default:
        std::cerr << "ERROR: INVALID Compaction eagerness!" << std::endl;
    }
    
    op->disable_auto_compactions = _env->disable_auto_compactions;
    if (_env->compaction_filter == 0) {
      ;// do nothing
    } 
    else {
      ;// invoke manual compaction_filter
    }
    if (_env->compaction_filter_factory == 0) {
      ;// do nothing
    } 
    else {
      ;// invoke manual compaction_filter_factory
    }
    switch (_env->access_hint_on_compaction_start) {
      case 1:
        op->access_hint_on_compaction_start = DBOptions::AccessHint::NONE; break;
      case 2:
        op->access_hint_on_compaction_start = DBOptions::AccessHint::NORMAL; break;
      case 3:
        op->access_hint_on_compaction_start = DBOptions::AccessHint::SEQUENTIAL; break;
      case 4:
        op->access_hint_on_compaction_start = DBOptions::AccessHint::WILLNEED; break;
      default:
        std::cerr << "error: access_hint_on_compaction_start" << std::endl;
    }
    
    if (op->compaction_style != kCompactionStyleUniversal) // !YBS-sep07-XX!
      op->level0_file_num_compaction_trigger = _env->level0_file_num_compaction_trigger; // !YBS-sep07-XX!
    op->target_file_size_multiplier = _env->target_file_size_multiplier;
    op->max_background_jobs = _env->max_background_jobs;
    op->max_compaction_bytes = _env->max_compaction_bytes;
    // op->max_bytes_for_level_base = _env->buffer_size * _env->size_ratio; 
    op->max_bytes_for_level_base = _env->buffer_size; // <-------------------------------------------------------------
    // op->max_bytes_for_level_base = _env->max_bytes_for_level_base; // <-------------------------------------------------------------
    std::cout << "printing: max_bytes_for_level_base = " << op->max_bytes_for_level_base << " buffer_size = " << _env->buffer_size << " size_ratio = " << _env->size_ratio << std::endl;
    if (_env->merge_operator == 0) {
      ;// do nothing
    } 
    else {
      ;// use custom merge operator
    }
    op->soft_pending_compaction_bytes_limit = _env->soft_pending_compaction_bytes_limit;    // No pending compaction anytime, try and see
    op->hard_pending_compaction_bytes_limit = _env->hard_pending_compaction_bytes_limit;    // No pending compaction anytime, try and see
    op->periodic_compaction_seconds = _env->periodic_compaction_seconds;
    op->use_direct_io_for_flush_and_compaction = _env->use_direct_io_for_flush_and_compaction;
    if (op->compaction_style != kCompactionStyleUniversal) // !YBS-sep07-XX!
      op->num_levels = _env->num_levels; // !YBS-sep07-XX!


    //Compression
    switch (_env->compression) {
      case 1:
        op->compression = kNoCompression; break;
      case 2:
        op->compression = kSnappyCompression; break;
      case 3:
        op->compression = kZlibCompression; break;
      case 4:
        op->compression = kBZip2Compression; break;
      case 5:
      op->compression = kLZ4Compression; break;
      case 6:
      op->compression = kLZ4HCCompression; break;
      case 7:
      op->compression = kXpressCompression; break;
      case 8:
      op->compression = kZSTD; break;
      case 9:
      op->compression = kZSTDNotFinalCompression; break;
      case 10:
      op->compression = kDisableCompressionOption; break;

      default:
        std::cerr << "error: compression" << std::endl;
    }

  // table_options.enable_index_compression = kNoCompression;

  // Other CFOptions
  switch (_env->comparator) {
      case 1:
        op->comparator = BytewiseComparator(); break;
      case 2:
        op->comparator = ReverseBytewiseComparator(); break;
      case 3:
        // use custom comparator
        break;
      default:
        std::cerr << "error: comparator" << std::endl;
    }

  op->max_sequential_skip_in_iterations = _env-> max_sequential_skip_in_iterations;
  op->memtable_prefix_bloom_size_ratio = _env-> memtable_prefix_bloom_size_ratio;    // disabled
  op->level0_stop_writes_trigger = _env->level0_stop_writes_trigger;
  op->paranoid_file_checks = _env->paranoid_file_checks;
  op->optimize_filters_for_hits = _env->optimize_filters_for_hits;
  op->inplace_update_support = _env->inplace_update_support;
  op->inplace_update_num_locks = _env->inplace_update_num_locks;
  op->report_bg_io_stats = _env->report_bg_io_stats;
  op->max_successive_merges = _env->max_successive_merges;   // read-modified-write related

  //Other DBOptions
  op->create_if_missing = _env->create_if_missing;
  op->delayed_write_rate = _env->delayed_write_rate;
  op->max_open_files = _env->max_open_files;
  op->max_file_opening_threads = _env->max_file_opening_threads;
  op->bytes_per_sync = _env->bytes_per_sync;
  op->stats_persist_period_sec = _env->stats_persist_period_sec;
  op->enable_thread_tracking = _env->enable_thread_tracking;
  op->stats_history_buffer_size = _env->stats_history_buffer_size;
  // op->allow_concurrent_memtable_write = _env->allow_concurrent_memtable_write;
  op->dump_malloc_stats = _env->dump_malloc_stats;
  op->use_direct_reads = _env->use_direct_reads;
  op->avoid_flush_during_shutdown = _env->avoid_flush_during_shutdown;
  op->advise_random_on_open = _env->advise_random_on_open;
  op->delete_obsolete_files_period_micros = _env->delete_obsolete_files_period_micros;   // 6 hours
  op->allow_mmap_reads = _env->allow_mmap_reads;
  op->allow_mmap_writes = _env->allow_mmap_writes;

  //TableOptions
  // !YBS-sep09-XX
  if (_env->block_cache == 0) {
    t_op->no_block_cache = true;
    t_op->cache_index_and_filter_blocks = false;
  } // TBC
  else {
    t_op->no_block_cache = false;
    std::shared_ptr<Cache> cache = NewLRUCache(_env->block_cache*1024*1024, -1, false, _env->block_cache_high_priority_ratio);
    t_op->block_cache = cache;
    t_op->cache_index_and_filter_blocks = _env->cache_index_and_filter_blocks;
  }
  _env->no_block_cache = t_op->no_block_cache;
  // !END

  if (_env->bits_per_key == 0) {
    ;// do nothing
  } 
  else {
    t_op->filter_policy.reset(NewBloomFilterPolicy(_env->bits_per_key, false));    // currently build full filter instead of block-based filter
  }

  
  t_op->cache_index_and_filter_blocks_with_high_priority = _env->cache_index_and_filter_blocks_with_high_priority;    // Deprecated by no_block_cache
  t_op->read_amp_bytes_per_bit = _env->read_amp_bytes_per_bit;
  
  switch (_env->data_block_index_type) {
      case 1:
        t_op->data_block_index_type = BlockBasedTableOptions::kDataBlockBinarySearch; break;
      case 2:
        t_op->data_block_index_type = BlockBasedTableOptions::kDataBlockBinaryAndHash; break;
      default:
        std::cerr << "error: TableOptions::data_block_index_type" << std::endl;
  }
  switch (_env->index_type) {
      case 1:
        t_op->index_type = BlockBasedTableOptions::kBinarySearch; break;
      case 2:
        t_op->index_type = BlockBasedTableOptions::kHashSearch; break;
      case 3:
        t_op->index_type = BlockBasedTableOptions::kTwoLevelIndexSearch; break;
      case 4:
        t_op->index_type = BlockBasedTableOptions::kBinarySearchWithFirstKey; break;
      default:
        std::cerr << "error: TableOptions::index_type" << std::endl;
  }
  t_op->partition_filters = _env->partition_filters;
  t_op->block_size = _env->entries_per_page * _env->entry_size;
  t_op->metadata_block_size = _env->metadata_block_size;
  t_op->pin_top_level_index_and_filter = _env->pin_top_level_index_and_filter;
  
  switch (_env->index_shortening) {
      case 1:
        t_op->index_shortening = BlockBasedTableOptions::IndexShorteningMode::kNoShortening; break;
      case 2:
        t_op->index_shortening = BlockBasedTableOptions::IndexShorteningMode::kShortenSeparators; break;
      case 3:
        t_op->index_shortening = BlockBasedTableOptions::IndexShorteningMode::kShortenSeparatorsAndSuccessor; break;
      default:
        std::cerr << "error: TableOptions::index_shortening" << std::endl;
  }
  t_op->block_size_deviation = _env->block_size_deviation;
  t_op->enable_index_compression = _env->enable_index_compression;
  // Set all table options
  op->table_factory.reset(NewBlockBasedTableFactory(*t_op));

  //WriteOptions
  w_op->sync = _env->sync; // make every write wait for sync with log (so we see real perf impact of insert) -- DOES NOT CAUSE SLOWDOWN
  w_op->low_pri = _env->low_pri; // every insert is less important than compaction -- CAUSES SLOWDOWN
  w_op->disableWAL = _env->disableWAL; 
  w_op->no_slowdown = _env->no_slowdown; // enabling this will make some insertions fail -- DOES NOT CAUSE SLOWDOWN
  w_op->ignore_missing_column_families = _env->ignore_missing_column_families;
  
  //ReadOptions
  r_op->verify_checksums = _env->verify_checksums;
  r_op->fill_cache = _env->fill_cache;
  // r_op->iter_start_seqnum = _env->iter_start_seqnum;
  r_op->ignore_range_deletions = _env->ignore_range_deletions;
  switch (_env->read_tier) {
    case 1:
      r_op->read_tier = kReadAllTier; break;
    case 2:
      r_op->read_tier = kBlockCacheTier; break;
    case 3:
      r_op->read_tier = kPersistedTier; break;
    case 4:
      r_op->read_tier = kMemtableTier; break;
    default:
      std::cerr << "error: ReadOptions::read_tier" << std::endl;
  }

  //FlushOptions
  // f_op->wait = _env->wait;
  // f_op->allow_write_stall = _env->allow_write_stall;
}





// void speed_test(){
//   std::string speed_test_result_file_name = "speed_test.txt";
//   std::string speed_test_result_file_name2 = "speed_test2.txt";
//   std::ofstream speed_test_result_file;
//   std::ofstream speed_test_result_file2;
//   speed_test_result_file.open(speed_test_result_file_name);
//   speed_test_result_file.open(speed_test_result_file_name2);
//   for(int i = 0; i < 256 * 1000; i++){
//     speed_test_result_file << i << " ";
//   }
//   speed_test_result_file << std::endl;
//   speed_test_result_file2 << std::endl;
//   speed_test_result_file.close();
//   speed_test_result_file2.close();

  

//   std::ifstream speed_test_result_file1;
//   std::ifstream speed_test_result_file12;
//   // testing_result_file.open("testing_result.txt");

//   std::string num = "-1";
//   auto start_pq = std::chrono::high_resolution_clock::now();
//   speed_test_result_file1.open(speed_test_result_file_name);
//   // speed_test_result_file2.open(speed_test_result_file_name2);
//   while(speed_test_result_file1 >> num){
//     if(num == "1"){break;}
//     continue;
//   }



//   auto stop_pq = std::chrono::high_resolution_clock::now();
//   auto duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
//   unsigned long long point_query_time = duration_pq.count();
//   std::cout << "done " << num << std::endl;
//   std::cout << "time elapsed = " << point_query_time << std::endl;
//   speed_test_result_file1.close();
//   speed_test_result_file12.close();



//   vector<int> v;
//   for(int i = 0; i < 1000000; i++){
//     v.push_back(i);
//   }

//   start_pq = std::chrono::high_resolution_clock::now();
//   // for(int i = 0; i < 1000000; i++){
//   //   std::binary_search(v.begin(), v.end(), i);
//   // }    
//   // for(int i = 0; i < 1000000; i++){
//   //   std::lower_bound(v.begin(), v.end(), i);
//   // }    
//   // std::binary_search(v.begin(), v.end(), 100000);
//   std::lower_bound(v.begin(), v.end(), 100000);
//   stop_pq = std::chrono::high_resolution_clock::now();
//   duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
//   point_query_time = duration_pq.count();
//   std::cout << "done " << 1000000 << std::endl;
//   std::cout << "time elapsed = " << point_query_time << std::endl;

// }


// void speed_test2(){
//   std::unordered_map<long long, uint64_t> map;
//   for(int i = 0; i < 1000000; i++){
//     map[i] = i;
//   }
//   // testing_result_file.open("testing_result.txt");

//   uint64_t num = -1;
//   auto start_pq = std::chrono::high_resolution_clock::now();

//   for(int i = 0; i < 1000000; i++){
//     num = map[i];
//   }

//   auto stop_pq = std::chrono::high_resolution_clock::now();
//   auto duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
//   unsigned long long point_query_time = duration_pq.count();
//   std::cout << "done " << num << std::endl;
//   std::cout << "time elapsed = " << point_query_time << std::endl;

// }

// Status DumpTable(const std::string& out_filename) {
//   std::unique_ptr<WritableFile> out_file;
//   Env* env = options_.env;
//   Status s = env->NewWritableFile(out_filename, &out_file, soptions_);
//   if (s.ok()) {
//     s = table_reader_->DumpTable(out_file.get());
//   }
//   if (!s.ok()) {
//     // close the file before return error, ignore the close error if there's any
//     out_file->Close().PermitUncheckedError();
//     return s;
//   }
//   return out_file->Close();
// }



int main(int argc, char *argv[]) {
  // check emu_environment.h for the contents of EmuEnv and also the definitions of the singleton experimental environment 
  EmuEnv* _env = EmuEnv::getInstance();
  //parse the command line arguments
  if (parse_arguments2(argc, argv, _env)){
    exit(1);
  }

  // int s = runWorkload(_env); 



  // speed_test();
  Options options;
  WriteOptions write_op;
  ReadOptions read_op;
  BlockBasedTableOptions table_options;
  FlushOptions f_options;

  DB* db;

  int max_background_jobs = 1;
  // Params params = parse_input(argc, argv);
  // if(params.gen_workload == true){
  if(_env->gen_workload == true){
    // gen_workload(params);
    gen_workload(_env);
  }
  init(&db, options, write_op, read_op, max_background_jobs, _env);
  
  configOptions(_env, &options, &table_options, &write_op, &read_op, &f_options);

  runWorkload(db, options, write_op, read_op, _env);
}
