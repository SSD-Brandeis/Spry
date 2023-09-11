#include <iostream>
#include <cmath>
#include <sys/time.h>
#include "emu_environment.h"
#include <algorithm>



/*Set up the singleton object with the experiment wide options*/
EmuEnv* EmuEnv::instance = 0;


EmuEnv::EmuEnv() 
{

  // delete_persistence_latency = 2250; // in secs
  // level_delete_persistence_latency = new double[20]; // in secs
  // RR_level_last_file_selected = new int[20]; // !YBS-sep06-XX!

  // flag = 0;
  // oldest_delete_file_timestamp = std::chrono::system_clock::now();

  // First-Entry Flags (FEFs) 
  version_set_FEF = false; // !YBS-sep06-XX!
  
  destroy_database = 1;
  clear_system_cache = 1; // !YBS-sep09-XX!
  show_progress = 0; // !YBS-sep17-XX!
  current_op = ' '; // !YBS-sep18-XX!

// Options set through command line 
  size_ratio = 10;
  buffer_size_in_pages = 4096;
  entries_per_page = 4;
  entry_size = 1024; // in Bytes 
  buffer_size = buffer_size_in_pages * entries_per_page * entry_size; // M = P*B*E = 128 * 128 * 128 B = 2 MB 
  file_to_memtable_size_ratio = 1; // f
  file_size = buffer_size * file_to_memtable_size_ratio; // F
  verbosity = 0;

  // adding new parameters with Guanting
  compaction_pri = 1; // c // 1 for kMinOverlappingRatio, 2 for kByCompensatedSize, 3 for kOldestLargestSeqFirst, 4 for kOldestSmallestSeqFirst, 5 for kEffacingCompaction
  bits_per_key = 10; // b


// Options hardcoded in code
  // Memory allocation options
    max_write_buffer_number = 2;
    // max_write_buffer_number = 1; // <------------------------------------------------------------------
    memtable_factory = 3; // 1 for skiplist, 2 for vector, 3 for hash skiplist, 4 for hash linklist
    // memtable_factory = 1; // 1 for skiplist, 2 for vector, 3 for hash skiplist, 4 for hash linklist
    target_file_size_base = buffer_size;
    level_compaction_dynamic_level_bytes = false;
    compaction_style = 1; // 1 for kCompactionStyleLevel, 2 for kCompactionStyleUniversal, 3 for kCompactionStyleFIFO, 4 for kCompactionStyleNone 
    disable_auto_compactions = false; // TBC
    compaction_filter = 0; // 0 for nullptr, 1 for invoking custom compaction filter, if any
    compaction_filter_factory = 0; // 0 for nullptr, 1 for invoking custom compaction filter factory, if any
    access_hint_on_compaction_start = 2; // TBC
    level0_file_num_compaction_trigger = 1; // TBC
    target_file_size_multiplier = 1; // TBC
    max_background_jobs = 1; // TBC
    max_compaction_bytes = 0; // TBC
    max_bytes_for_level_base = buffer_size * size_ratio; // TBC
    merge_operator = 0; 
    soft_pending_compaction_bytes_limit = 0;    // No pending compaction anytime, try and see
    hard_pending_compaction_bytes_limit = 0;    // No pending compaction anytime, try and see
    periodic_compaction_seconds = 0;
    use_direct_io_for_flush_and_compaction = true;
    live_levels = 0; //!YBS-sep07-XX!
    num_levels = 10; // Maximum number of levels that a tree may have [RDB_default: 7]
    // num_levels = 11; // <--------------------------------------------------------------

  // TableOptions
    no_block_cache = false; // TBC
    block_cache = 0; // in MB //!YBS-sep09-XX!
    block_cache_high_priority_ratio = 0.5; //!YBS-sep09-XX!
    cache_index_and_filter_blocks = true;
    cache_index_and_filter_blocks_with_high_priority = true;    // Deprecated by no_block_cache
    read_amp_bytes_per_bit = 4; // temporarily 4; why 4 ?
    data_block_index_type = 1; // 1 for kDataBlockBinarySearch, 2 for kDataBlockBinaryAndHash
    index_type = 1; // 1 for kBinarySearch, 2 for kHashSearch, 3 for kTwoLevelIndexSearch, 4 for kBinarySearchWithFirstKey
    partition_filters = false;
    metadata_block_size = 4096;   // currently deprecated by data_block_index_type; TBC
    pin_top_level_index_and_filter = false; // TBC
    index_shortening = 1; // 1 for kNoShortening, 2 for kShortenSeparators, 3 for kShortenSeparatorsAndSuccessor
    block_size_deviation = 0; // TBC
    enable_index_compression = false; // TBC

  //Compression
    compression = 1; // 1 for kNoCompression, 2 for kSnappyCompression, 3 for kZlibCompression, 4 for kBZip2Compression, 5 for kLZ4Compression, 6 for kLZ4HCCompression, 7 for kXpressCompression, 8 for kZSTD, 9 for kZSTDNotFinalCompression, 10 for kDisableCompressionOption

  // ReadOptions
    verify_checksums = true; // TBC
    fill_cache = false; // data block/index block read for this iteration will not be cached
    iter_start_seqnum = 0; // TBC
    ignore_range_deletions = false; // TBC
    read_tier = 1; // 1 for kReadAllTier, 2 for kBlockCacheTier, 3 for kPersistedTier, 4 for kMemtableTier

  // WriteOptions
    low_pri = true; // every insert is less important than compaction
    sync = false; // make every write wait for sync with log (so we see real perf impact of insert)
    disableWAL = false; // TBC
    no_slowdown = false; // enabling this will make some insertions fail 
    ignore_missing_column_families = false; // TBC

  // Other CFOptions
    comparator = 1; // 1 for BytewiseComparator(), 2 for ...
    max_sequential_skip_in_iterations = 8; // TBC
    memtable_prefix_bloom_size_ratio = 0;    // disabled
    level0_stop_writes_trigger = 2; // need to try with 1 ; TBC
    paranoid_file_checks = false;
    optimize_filters_for_hits = false;
    inplace_update_support = false;
    inplace_update_num_locks = 10000;
    report_bg_io_stats = true;
    max_successive_merges = 0;   // read-modified-write related

  // Other DBOptions
    create_if_missing = true;
    delayed_write_rate = 0;
    // max_open_files = 99999;    // resetting to 20 //-1 for always open (no closing)
    max_open_files = 20;   
    max_file_opening_threads = 80;
    bytes_per_sync = 0;
    stats_persist_period_sec = 600;
    enable_thread_tracking = false;
    stats_history_buffer_size = 1024 * 1024;
    allow_concurrent_memtable_write = false;
    dump_malloc_stats = false;
    use_direct_reads = true;
    avoid_flush_during_shutdown = false;
    advise_random_on_open = true;
    delete_obsolete_files_period_micros = 6ULL * 60 * 60 * 1000000;   // 6 hours
    allow_mmap_reads = false;
    allow_mmap_writes = false;

    // Flush Options
    wait = true;
    allow_write_stall = true;



  // Workload options -- not sure if necessary to have these here!
    int num_inserts = 0;

// old options


  path = ""; 
  debugging = false; 
  FPR_optimization_level = 1; 
  derived_num_levels=-1; 
  N=-1; 
  derived_N=-1; 
  K=1; 
  Z=1; 
  use_block_based_filter=false;
  string experiment_name = ""; 
  string experiment_starting_time = "";
  max_levels=1000;
  measure_IOs=false; 
  total_IOs=0;
  target_level_for_non_zero_result_point_lookups = num_levels;
  key_prefix_for_entries_to_target_in_queries = "+";
  clean_caches_for_experiments=false;
  print_IOs_per_file=false;

  
  file_system_page_size=4096;
  only_tune=false;
  num_read_query_sessions=1;
}

EmuEnv* EmuEnv::getInstance()
{
  if (instance == 0)
    instance = new EmuEnv();

  return instance;
}


// void EmuEnv::PopulatingVector(uint64_t _file_id){
//   EmuEnv* _env = EmuEnv::getInstance();
//   if (_file_id > 0) {
//     // for (int i = 0; i < _env->vec.size(); ++i) 
//     //   std::cout << "printing vector state BEFORE: " << _env->vec[i] << "\t";
//     // std::cout << "\n";

//     _env->vec.push_back(_file_id);
//     // std::cout << "pushing file_id = " << _file_id << std::endl;

//     // for (int i = 0; i < _env->vec.size(); ++i) 
//     //   std::cout << "printing vector state AFTER : " << _env->vec[i] << "\t";
//     // std::cout << "\n";
//   }
// }

// int EmuEnv::CheckingVector(uint64_t _file_id){
//   EmuEnv* _env = EmuEnv::getInstance();
//   bool match = false;
//   // for (int i = 0; i < _env->vec.size(); ++i) 
//   //   std::cout << "printing vector state BEFORE: " << _env->vec[i] << "\t";
//   // std::cout << "\n";

//   for (int i = 0; i < _env->vec.size(); ++i) {
//     if (_env->vec[i] == _file_id) {
//       // std::cout << "MATCH FOUND file_id = " << _file_id << std::endl;
//       _env->vec.erase(std::remove(_env->vec.begin(), _env->vec.end(), _file_id), _env->vec.end());
//       match = true;
//     }
//   }
//   // for (int i = 0; i < _env->vec.size(); ++i) 
//   //   std::cout << "printing vector state AFTER : " << _env->vec[i] << "\t";
//   // std::cout << "\n";

//   if(match) return 2; return 1;

// }


