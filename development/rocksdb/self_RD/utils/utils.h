

#ifndef UTILS_H
#define UTILS_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <chrono>
#include "utils.h"
#include "rocksdb/options.h"
#include "rocksdb/advanced_options.h"
#include "rocksdb/slice.h"
#include "rocksdb/cache.h"
#include "rocksdb/filter_policy.h"
#include "rocksdb/perf_context.h" // !YBS-sep01-XX!
#include "rocksdb/iostats_context.h" // !YBS-sep01-XX!
#include "../env_settings/emu_environment.h"


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
  table_options.block_cache = NewLRUCache(0.5*1024*1024);

  op.table_factory.reset(NewBlockBasedTableFactory(table_options));
  
  std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 10 second
}

void setNoBlockCacheForReading(Options& op){
  std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 10 second

  BlockBasedTableOptions table_options;
  table_options.no_block_cache = true; //disable block cache completely
  table_options.block_size = 4 * 1024;

  op.table_factory.reset(NewBlockBasedTableFactory(table_options));

  std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 10 second
}

std::string getkDBPath(EmuEnv* _env, string kDBPathBase){
  std::string kDBPath = kDBPathBase + "/" + _env->workload_file_name;
  return kDBPath;
}

void clearCache(Options& op){
  std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 10 second

  // Clearing the system cache
  std::cout << "Clearing system cache ..." << std::endl;
  std::cout << "Reminder: please put your sudo password in the secret.txt file under development/rocksdb/self_RD." << std::endl;
  // int clean_flag = system("sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'");
  int clean_flag = system("cat secret.txt | sudo -S sh -c 'echo 3 >/proc/sys/vm/drop_caches'");
  if (clean_flag) {
    std::cerr << "Cannot clean the system cache" << std::endl;
    exit(0);
  }

  std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 10 second
}

void clearBlockCache(DB* db, std::ostream& ofile){
  std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 10 second

  db->CleanTableCache(ofile);

  std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 10 second
}


void reset_perf_iostats_context(){
  rocksdb::get_perf_context()->Reset();
  rocksdb::get_iostats_context()->Reset();
  rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
}





void configOptions(EmuEnv* _env, Options *op, BlockBasedTableOptions *t_op, WriteOptions *w_op, ReadOptions *r_op, FlushOptions *f_op) {
    // *op = Options();
    op->statistics = CreateDBStatistics(); // !YBS-sep01-XX!
    op->write_buffer_size = _env->buffer_size; // !YBS-sep07-XX!
    op->max_write_buffer_number = _env->max_write_buffer_number;   // min 2 // !YBS-sep07-XX!


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




#endif