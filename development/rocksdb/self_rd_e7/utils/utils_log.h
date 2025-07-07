#ifndef UTILS_LOG_H

#define UTILS_LOG_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include "utils.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/advanced_options.h"
#include "rocksdb/slice.h"
#include "rocksdb/system_verifier.h"



void print_perf_iostats_context(std::ostream& ofile, const std::string &prefix, int N_repetitions);
void write_log2(std::ostream &outStream, EmuEnv* _env, surf::SuRF_Env* _surf_env);
void io_timing_test(DB* db);





//logging from perfIOstats and perfContext to file
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
      long long read_count_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.count[^:]*: ([0-9]+)")
        + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.count[^:]*: ([0-9]+)");
      long long read_bytes_end = parsing_value_from_string(op.statistics->ToString(), "last.level.read.bytes[^:]*: ([0-9]+)")
        + parsing_value_from_string(op.statistics->ToString(), "non.last.level.read.bytes[^:]*: ([0-9]+)");
      total_read_count += read_count_end - read_count_start;
      total_read_bytes += read_bytes_end - read_bytes_start;
      // reset_perf_iostats_context();

      testing_result_file << "read_count_start = " << std::fixed << std::setprecision(2) << read_count_start << std::endl;
      testing_result_file << "read_count_end = " << std::fixed << std::setprecision(2) << read_count_end << std::endl;
      testing_result_file << "read_bytes_start = " << std::fixed << std::setprecision(2) << read_bytes_start << std::endl;
      testing_result_file << "read_bytes_end = " << std::fixed << std::setprecision(2) << read_bytes_end << std::endl;
      testing_result_file << i_round << " -----" << std::endl << std::endl;  
    }


    void output_statistics(std::ostream& testing_result_file, std::ostream& testing_result_file2, const std::string &prefix){
      testing_result_file << "Avg_read_count = " << std::fixed << std::setprecision(2) << (total_read_count) * 1.0 / i_round << std::endl;
      testing_result_file << "Avg_read_bytes = " << std::fixed << std::setprecision(2) << (total_read_bytes) * 1.0 / i_round << std::endl;
      testing_result_file << std::endl;
      print_perf_iostats_context(testing_result_file, prefix, i_round);
      testing_result_file << std::endl;

      testing_result_file2 << ",\"" << prefix << "Avg_read_count\" : " << std::fixed << std::setprecision(2) << (total_read_count) * 1.0 / i_round << std::endl;
      testing_result_file2 << ",\"" << prefix << "Avg_read_bytes\" : " << std::fixed << std::setprecision(2) << (total_read_bytes) * 1.0 / i_round << std::endl;
    }
};







void print_perf_iostats_context(std::ostream& ofile, const std::string &prefix, int nth_round){
  
    std::string perf_context = rocksdb::get_perf_context()->ToString();
    std::cout << "perf_context = " << perf_context << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    // std::cout << " rocksdb::get_perf_context()->bloom_sst_miss_count = " <<  rocksdb::get_perf_context()->bloom_sst_miss_count << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    // std::cout << " rocksdb::get_perf_context()->bloom_sst_hit_count = " <<  rocksdb::get_perf_context()->bloom_sst_hit_count << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

    long long get_from_memtable_time = parsing_value_from_string(perf_context, "get_from_memtable_time[^=]*=.([0-9]+)");
    long long get_from_memtable_count = parsing_value_from_string(perf_context, "get_from_memtable_count[^=]*=.([0-9]+)");
    long long get_post_process_time = parsing_value_from_string(perf_context, "get_post_process_time[^=]*=.([0-9]+)");

    long long bloom_memtable_hit_count = parsing_value_from_string(perf_context, "bloom_memtable_hit_count[^=]*=.([0-9]+)");
    long long bloom_memtable_miss_count = parsing_value_from_string(perf_context, "bloom_memtable_miss_count[^=]*=.([0-9]+)");
    long long bloom_sst_hit_count = parsing_value_from_string(perf_context, "bloom_sst_hit_count[^=]*=.([0-9]+)");
    long long bloom_sst_miss_count = parsing_value_from_string(perf_context, "bloom_sst_miss_count[^=]*=.([0-9]+)");


    long long block_read_count = parsing_value_from_string(perf_context, "block_read_count[^=]*=.([0-9]+)");
    long long block_read_byte = parsing_value_from_string(perf_context, "block_read_byte[^=]*=.([0-9]+)");
    long long block_read_time = parsing_value_from_string(perf_context, "block_read_time[^=]*=.([0-9]+)");
    long long block_read_cpu_time = parsing_value_from_string(perf_context, "block_read_cpu_time[^=]*=.([0-9]+)");
    long long index_block_read_count = parsing_value_from_string(perf_context, "index_block_read_count[^=]*=.([0-9]+)");
    long long filter_block_read_count = parsing_value_from_string(perf_context, "filter_block_read_count[^=]*=.([0-9]+)");
    long long compression_dict_block_read_count = parsing_value_from_string(perf_context, "compression_dict_block_read_count[^=]*=.([0-9]+)");
    long long get_read_bytes = parsing_value_from_string(perf_context, "get_read_bytes[^=]*=.([0-9]+)");
    long long read_index_block_nanos = parsing_value_from_string(perf_context, "read_index_block_nanos[^=]*=.([0-9]+)");
    long long read_filter_block_nanos = parsing_value_from_string(perf_context, "read_filter_block_nanos[^=]*=.([0-9]+)");
    long long internal_key_skipped_count = parsing_value_from_string(perf_context, "internal_key_skipped_count[^=]*=.([0-9]+)");
    long long internal_delete_skipped_count = parsing_value_from_string(perf_context, "internal_delete_skipped_count[^=]*=.([0-9]+)");
    long long internal_recent_skipped_count = parsing_value_from_string(perf_context, "internal_recent_skipped_count[^=]*=.([0-9]+)");
    long long internal_range_del_reseek_count = parsing_value_from_string(perf_context, "internal_range_del_reseek_count[^=]*=.([0-9]+)");


    std::string iostats_context = rocksdb::get_iostats_context()->ToString();
    std::cout << "iostats_context = " << iostats_context << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

    long long bytes_read = parsing_value_from_string(iostats_context, "bytes_read[^=]*=.([0-9]+)");
    long long bytes_written = parsing_value_from_string(iostats_context, "bytes_written[^=]*=.([0-9]+)");
    long long read_nanos = parsing_value_from_string(iostats_context, "read_nanos[^=]*=.([0-9]+)");
    long long write_nanos = parsing_value_from_string(iostats_context, "write_nanos[^=]*=.([0-9]+)");

    //print out all the above variable
    ofile << "nth_round = " << nth_round << " get_from_memtable_time = " << std::fixed << std::setprecision(2) << get_from_memtable_time * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " get_from_memtable_count = " << std::fixed << std::setprecision(2) << get_from_memtable_count * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " get_post_process_time = " << std::fixed << std::setprecision(2) << get_post_process_time * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " bloom_memtable_hit_count = " << std::fixed << std::setprecision(2) << bloom_memtable_hit_count * 1.0  << std::endl;
    ofile << "nth_round = " << nth_round << " bloom_memtable_miss_count = " << std::fixed << std::setprecision(2) << bloom_memtable_miss_count * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " bloom_sst_hit_count = " << std::fixed << std::setprecision(2) << bloom_sst_hit_count * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " bloom_sst_miss_count = " << std::fixed << std::setprecision(2) << bloom_sst_miss_count * 1.0 << std::endl;
    

    ofile << "nth_round = " << nth_round << " block_read_count = " << std::fixed << std::setprecision(2) << block_read_count * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " block_read_byte = " << std::fixed << std::setprecision(2) << block_read_byte * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " block_read_time = " << std::fixed << std::setprecision(2) << block_read_time * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " block_read_cpu_time = " << std::fixed << std::setprecision(2) << block_read_cpu_time * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " index_block_read_count = " << std::fixed << std::setprecision(2) << index_block_read_count * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " filter_block_read_count = " << std::fixed << std::setprecision(2) << filter_block_read_count * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " compression_dict_block_read_count = " << std::fixed << std::setprecision(2) << compression_dict_block_read_count * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " get_read_bytes = " << std::fixed << std::setprecision(2) << get_read_bytes * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " read_index_block_nanos = " << std::fixed << std::setprecision(2) << read_index_block_nanos * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " read_filter_block_nanos = " << std::fixed << std::setprecision(2) << read_filter_block_nanos * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " internal_key_skipped_count = " << std::fixed << std::setprecision(2) << internal_key_skipped_count * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " internal_delete_skipped_count = " << std::fixed << std::setprecision(2) << internal_delete_skipped_count * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " internal_recent_skipped_count = " << std::fixed << std::setprecision(2) << internal_recent_skipped_count * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " internal_range_del_reseek_count = " << std::fixed << std::setprecision(2) << internal_range_del_reseek_count * 1.0 << std::endl;


    ofile << "nth_round = " << nth_round << " bytes_read = " << std::fixed << std::setprecision(2) << bytes_read * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " bytes_written = " << std::fixed << std::setprecision(2) << bytes_written * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " read_nanos = " << std::fixed << std::setprecision(2) << read_nanos * 1.0 << std::endl;
    ofile << "nth_round = " << nth_round << " write_nanos = " << std::fixed << std::setprecision(2) << write_nanos * 1.0 << std::endl;
    ofile << "--------------------------------------------------------------------" << std::endl;

    // std::cout << prefix + "get_from_memtable_time _out = " << std::fixed << std::setprecision(2) << get_from_memtable_time * 1.0 / N_repetitions << std::endl;
    // std::cout << prefix + "get_from_memtable_count _out = " << std::fixed << std::setprecision(2) << get_from_memtable_count * 1.0 / N_repetitions << std::endl;
    // std::cout << prefix + "get_post_process_time _out = " << std::fixed << std::setprecision(2) << get_post_process_time * 1.0 / N_repetitions << std::endl;
    std::cout << prefix + "bloom_memtable_hit_count _out = " << std::fixed << std::setprecision(2) << bloom_memtable_hit_count * 1.0 << std::endl;
    std::cout << prefix + "bloom_memtable_miss_count _out = " << std::fixed << std::setprecision(2) << bloom_memtable_miss_count * 1.0 << std::endl;
    std::cout << prefix + "bloom_sst_hit_count _out = " << std::fixed << std::setprecision(2) << bloom_sst_hit_count * 1.0 << std::endl;
    std::cout << prefix + "bloom_sst_miss_count _out = " << std::fixed << std::setprecision(2) << bloom_sst_miss_count * 1.0 << std::endl;
    

    std::cout << prefix + "block_read_count _out = " << std::fixed << std::setprecision(2) << block_read_count * 1.0 << std::endl;
    std::cout << prefix + "block_read_byte _out = " << std::fixed << std::setprecision(2) << block_read_byte * 1.0 << std::endl;
    std::cout << prefix + "block_read_time _out = " << std::fixed << std::setprecision(2) << block_read_time * 1.0 << std::endl;
    std::cout << prefix + "block_read_cpu_time _out = " << std::fixed << std::setprecision(2) << block_read_cpu_time * 1.0 << std::endl;
    std::cout << prefix + "index_block_read_count _out = " << std::fixed << std::setprecision(2) << index_block_read_count * 1.0 << std::endl;
    std::cout << prefix + "filter_block_read_count _out = " << std::fixed << std::setprecision(2) << filter_block_read_count * 1.0 << std::endl;
    // std::cout << prefix + "compression_dict_block_read_count _out = " << std::fixed << std::setprecision(2) << compression_dict_block_read_count * 1.0 / N_repetitions << std::endl;
    std::cout << prefix + "get_read_bytes _out = " << std::fixed << std::setprecision(2) << get_read_bytes * 1.0 << std::endl;
    // std::cout << prefix + "read_index_block_nanos _out = " << std::fixed << std::setprecision(2) << read_index_block_nanos * 1.0 / N_repetitions << std::endl;
    // std::cout << prefix + "read_filter_block_nanos _out = " << std::fixed << std::setprecision(2) << read_filter_block_nanos * 1.0 / N_repetitions << std::endl;
    // std::cout << prefix + "internal_key_skipped_count _out = " << std::fixed << std::setprecision(2) << internal_key_skipped_count * 1.0 / N_repetitions << std::endl;
    // std::cout << prefix + "internal_delete_skipped_count _out = " << std::fixed << std::setprecision(2) << internal_delete_skipped_count * 1.0 / N_repetitions << std::endl;
    // std::cout << prefix + "internal_recent_skipped_count _out = " << std::fixed << std::setprecision(2) << internal_recent_skipped_count * 1.0 / N_repetitions << std::endl;
    // std::cout << prefix + "internal_range_del_reseek_count _out = " << std::fixed << std::setprecision(2) << internal_range_del_reseek_count * 1.0 / N_repetitions << std::endl;


    std::cout << prefix + "bytes_read _out = " << std::fixed << std::setprecision(2) << bytes_read * 1.0 << std::endl;
    std::cout << prefix + "bytes_written _out = " << std::fixed << std::setprecision(2) << bytes_written * 1.0 << std::endl;
    // std::cout << prefix + "read_nanos _out = " << std::fixed << std::setprecision(2) << read_nanos * 1.0 / N_repetitions << std::endl;
    // std::cout << prefix + "write_nanos _out = " << std::fixed << std::setprecision(2) << write_nanos * 1.0 / N_repetitions << std::endl;
}


void write_log2(std::ostream &outStream, EmuEnv* _env, surf::SuRF_Env* _surf_env){
  outStream << ",\"T\" : " <<_env->size_ratio << std::endl;
  outStream << ",\"P\" : " <<_env->buffer_size_in_pages << std::endl;
  outStream << ",\"B\" : " <<_env->entries_per_page << std::endl;
  outStream << ",\"E\" : " <<_env->entry_size << std::endl;
  outStream << ",\"write_buffer_size\" : " <<_env->buffer_size << std::endl;
  outStream << ",\"correlation\" : " <<_env->correlation << std::endl;
  outStream << ",\"num_inserts\" : " <<_env->num_inserts << std::endl;
  outStream << ",\"rd_count\" : " <<_env->rd_count << std::endl;
  outStream << ",\"selectivity\" : " <<_env->selectivity << std::endl;
  outStream << ",\"workload_file_name\" : " << "\"" << _env->workload_file_name << "\"" << std::endl;
  outStream << ",\"insert_before_rangeDelete\" : " <<_env->insert_before_range_delete << std::endl;
  outStream << ",\"gen_workload\" : " <<_env->gen_workload << std::endl;
  outStream << ",\"max_background_jobs\" : " <<_env->max_background_jobs << std::endl;
  outStream << ",\"target_file_size_base\" : " <<_env->target_file_size_base << std::endl;
  outStream << ",\"target_file_size_multiplier\" : " <<_env->target_file_size_multiplier << std::endl;
  outStream << ",\"max_bytes_for_level_base\" : " <<_env->max_bytes_for_level_base << std::endl;
  outStream << ",\"num_levels\" : " <<_env->num_levels << std::endl;
  outStream << ",\"max_write_buffer_number\" : " <<_env->max_write_buffer_number << std::endl;
  outStream << ",\"level0_file_num_compaction_trigger\" : " <<_env->level0_file_num_compaction_trigger << std::endl;

  outStream << ",\"surf__key_len_in_bytes\" : " <<_surf_env->getSuRFKeyLenInBytes() << std::endl;
  outStream << ",\"surf__hash_suffix_len\" : " <<_surf_env->getSuRFHashSuffixLen() << std::endl;
  outStream << ",\"surf__real_suffix_len\" : " <<_surf_env->getSuRFRealSuffixLen() << std::endl;
  outStream << ",\"surf__include_dense\" : " <<_surf_env->getSuRFIncludeDense() << std::endl;
  outStream << ",\"surf__sparse_dense_ratio\" : " <<_surf_env->getSuRFSparseDenseRatio() << std::endl;
  outStream << ",\"surf__flag_bypass_if_same_key\" : " <<_surf_env->getFlagBypassIfSameKey() << std::endl;
  outStream << ",\"surf__flag_allow_range_boundary_overlapped\" : " <<_surf_env->getFlagAllowRangeBoundaryOverlapped() << std::endl;
  outStream << ",\"surf__flag_use_condensed_digit_key\" : " <<_surf_env->getFlagSurfUseCondensedDigitKey() << std::endl;
  outStream << ",\"surf__length_of_condensed_digit_key\" : " <<_surf_env->getLengthOfCondensedDigitKey() << std::endl;
  outStream << ",\"show_surf_compaction_info\" : " <<_surf_env->getShowSurfCompactionInfo() << std::endl;
  outStream << ",\"use_surf_base\" : " <<_surf_env->getFlagUseSuRFBase() << std::endl;
  outStream << ",\"surf_base_store_key_to_k_diff\" : " <<_surf_env->getSuRFBaseStoreKeyToKDiff() << std::endl;
}


void io_timing_test(DB* db){
  vector<int> ranges_log_PLRDF = db->getLogOfNumbersOfRangesInPLRDF();
  vector<int> ranges_log_SplitPLRDF = db->getLogOfNumbersOfRangesInSplitPLRDF();
  vector<int> ranges_log_PLRDFStringKey = db->getLogOfNumbersOfRangesInPLRDFStringKey();
  vector<int> ranges_log_SplitPLRDFStringKey = db->getLogOfNumbersOfRangesInSplitPLRDFStringKey();
  vector<int> ranges_log_TopLevelRDF = db->getLogOfNumbersOfRangesInTopLevelRDF();
  vector<int> ranges_log_SkylineRDF = db->getLogOfNumbersOfRangesInSkylineRDF();
  vector<int> ranges_log_SuRFLevelFileRDF = db->getLogOfNumbersOfRangesInSuRFLevelFileRDF();
  vector<int> ranges_log_SuRFLevelFileSplitRDF = db->getLogOfNumbersOfRangesInSuRFLevelFileSplitRDF();

  vector<int> memory_usage_log_PLRDF = db->getLogOfMemoryUsageInPLRDF();
  vector<int> memory_usage_log_SplitPLRDF = db->getLogOfMemoryUsageInSplitRDF();
  vector<int> memory_usage_log_PLRDFStringKey = db->getLogOfMemoryUsageInPLRDFStringKey();
  vector<int> memory_usage_log_SplitPLRDFStringKey = db->getLogOfMemoryUsageInSplitRDFStringKey();
  vector<int> memory_usage_log_TopLevelRDF = db->getLogOfMemoryUsageInTopLevelRDF();
  vector<int> memory_usage_log_SkylineRDF = db->getLogOfMemoryUsageInSkylineRDF();
  vector<int> memory_usage_log_SuRFLevelFileRDF = db->getLogOfMemoryUsageInSuRFLevelFileRDF();
  vector<int> memory_usage_log_SuRFLevelFileSplitRDF = db->getLogOfMemoryUsageInSuRFLevelFileSplitRDF();

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
  std::cout << "Ranges Log Of PLRDFStringKey: " << std::endl;
  for (int i = 0; i < ranges_log_PLRDFStringKey.size(); i++) {
    std::cout << ranges_log_PLRDFStringKey[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "Ranges Log Of SplitPLRDFStringKey: " << std::endl;
  for (int i = 0; i < ranges_log_SplitPLRDFStringKey.size(); i++) {
    std::cout << ranges_log_SplitPLRDFStringKey[i] << " ";
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
  std::cout << "Ranges Log Of SuRF_LF_RDF: " << std::endl;
  for (int i = 0; i < ranges_log_SuRFLevelFileRDF.size(); i++) {
    std::cout << ranges_log_SuRFLevelFileRDF[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "Ranges Log Of SuRF_LFS_RDF: " << std::endl;
  for (int i = 0; i < ranges_log_SuRFLevelFileSplitRDF.size(); i++) {
    std::cout << ranges_log_SuRFLevelFileSplitRDF[i] << " ";
  }
  std::cout << std::endl;

  
  std::cout << "Memory Usage Log Of PLRDF: " << std::endl;
  for (int i = 0; i < memory_usage_log_PLRDF.size(); i++) {
    std::cout << memory_usage_log_PLRDF[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "Memory Usage Log Of SplitPLRDF: " << std::endl;
  for (int i = 0; i < memory_usage_log_SplitPLRDF.size(); i++) {
    std::cout << memory_usage_log_SplitPLRDF[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "Memory Usage Log Of PLRDFStringKey: " << std::endl;
  for (int i = 0; i < memory_usage_log_PLRDFStringKey.size(); i++) {
    std::cout << memory_usage_log_PLRDFStringKey[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "Memory Usage Log Of SplitPLRDFStringKey: " << std::endl;
  for (int i = 0; i < memory_usage_log_SplitPLRDFStringKey.size(); i++) {
    std::cout << memory_usage_log_SplitPLRDFStringKey[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "Memory Usage Log Of TopLevelRDF: " << std::endl;
  for (int i = 0; i < memory_usage_log_TopLevelRDF.size(); i++) {
    std::cout << memory_usage_log_TopLevelRDF[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "Memory Usage Log Of SkylineRDF: " << std::endl;
  for (int i = 0; i < memory_usage_log_SkylineRDF.size(); i++) {
    std::cout << memory_usage_log_SkylineRDF[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "Memory Usage Log Of SuRF_LF_RDF: " << std::endl;
  for (int i = 0; i < memory_usage_log_SuRFLevelFileRDF.size(); i++) {
    std::cout << memory_usage_log_SuRFLevelFileRDF[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "Memory Usage Log Of SuRF_LFS_RDF: " << std::endl;
  for (int i = 0; i < memory_usage_log_SuRFLevelFileSplitRDF.size(); i++) {
    std::cout << memory_usage_log_SuRFLevelFileSplitRDF[i] << " ";
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
  timer_start = std::chrono::high_resolution_clock::now();
  for(int i_timing = 0; i_timing < 10000; i_timing++){
    checking::SystemVerifier::getSystemVerifier()->start_remaining_get_path();
    checking::SystemVerifier::getSystemVerifier()->stop_remaining_get_path();
  }
  timer_end = std::chrono::high_resolution_clock::now();
  unsigned long long total_duration__remaining_get_path = checking::SystemVerifier::getSystemVerifier()->get_total_duration__remaining_get_path();
  std::cout << "total_duration__remaining_get_path = " << total_duration__remaining_get_path << std::endl;

  duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timer_end - timer_start);
  std::cout << "timer duration1 = " << duration_ns.count() << std::endl;


  checking::SystemVerifier *system_verifier_timing = checking::SystemVerifier::getSystemVerifier();
  system_verifier_timing->reset_total_duration__remaining_get_path();
  timer_start = std::chrono::high_resolution_clock::now();
  for(int i_timing = 0; i_timing < 10000; i_timing++){
    system_verifier_timing->start_remaining_get_path();
    system_verifier_timing->stop_remaining_get_path();
  }
  timer_end = std::chrono::high_resolution_clock::now();
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
}

#endif