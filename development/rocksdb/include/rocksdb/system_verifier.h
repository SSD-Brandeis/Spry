/*
 *  Created on: April 23, 2023
 *  Author: yucheng
 */


#ifndef SYSTEM_VERIFIER_H_
#define SYSTEM_VERIFIER_H_


namespace checking {
  class SystemVerifier;
} 


#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <assert.h>
#include <vector>
#include <map>
#include <set>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <unordered_map>
// #include "../emu_environment.h"
// #include "../workload_executor.h"
// #include "../workload/workload.h"
// #include "../query_runner.h"

// #include "system_verifier.cc"



using namespace std;
// using namespace workload_exec;

namespace checking {

  struct RandomKeysTestingResult{
      public:
          vector<int> disk_access_count_list;
          int count_exist_key;
          int count_non_exist_key;
  };

  class SystemVerifier {
  private:
    static const int KEY_SIZE = 12;

    int CurrentlyNonInsertedKeysNum = 1000;

    bool flag_skip_reading_range_delete_block = false;

    int disk_access_count = 0;
    int read_entry_block_count = 0;
    int filtered_by_RDF_count = 0;
    int block_based_table_open_count = 0;

    //increasing in block_based_table_reader.cc
    int num_index_read_count = 0;
    int num_filter_read_count = 0;
    int num_range_del_read_count = 0; 
    int num_total_block_read_count = 0;

    //increasing in block_fetcher.cc
    int fetcher__num_compression_dict_block_read_count = 0;
    int fetcher__num_index_read_count = 0;
    int fetcher__num_filter_read_count = 0;
    int fetcher__num_range_del_read_count = 0; 
    int fetcher__num_data_read_count = 0; 
    int fetcher__num_total_block_read_count = 0;



    bool flag_log__deleted_keys__max_sequnce_number = false;
    std::unordered_map<long long, uint64_t> deleted_keys__max_sequnce_number;
    bool flag_is_RDF_filtered_entry = false;

    bool flag_testing_on_currently_deleted_keys = false;

    bool flag_open_table = false;
    bool flag_pq_tracing_on = false;
    unordered_map<long long, vector<tuple<unsigned long long, unsigned int, bool>>> map_pq_tracing_info; // key -> {(fd, LSM level, open file), ...}
    vector<tuple<unsigned long long, unsigned int, bool>> v_pq_tracing_info; // {(fd, LSM level, open file), ...}
  public:
    static SystemVerifier* system_verifier;

    // WorkloadRecorder();
    const static int EXPERIMENT_REPETITION_TIMES = 3;
    // const static int EXPERIMENT_REPETITION_TIMES = 1;

    static void init(){
      if(system_verifier == NULL){
        system_verifier = new SystemVerifier();
      }
    }

    static SystemVerifier* getSystemVerifier(){
      init();
      return system_verifier;
    }

    static int getKeySize(){
      return KEY_SIZE;
    }



    void setFlagOpenTable(){
      flag_open_table = true;
      std::cout << "setFlagOpenTable " << __FILE__ << ":" << __LINE__ << std::endl;
    }
    void clearFlagOpenTable(){
      flag_open_table = false;
    }
    bool getFlagOpenTable(){
      return flag_open_table;
    }
    void startPQTracing(){
      flag_pq_tracing_on = true;
      clearFlagOpenTable();
    }
    void endPQTracing(){
      flag_pq_tracing_on = false;
    }
    void clearMapPQTracingInfo(){
      map_pq_tracing_info.clear();
    }
    void clearVPQTracingInfo(){
      v_pq_tracing_info.clear();
    }
    void logPQTracingInfo(long long key, unsigned long long fd, unsigned int LSM_level){
      if(!flag_pq_tracing_on){
        return;
      }

      if(map_pq_tracing_info.find(key) == map_pq_tracing_info.end()){
        map_pq_tracing_info[key] = vector<tuple<unsigned long long, unsigned int, bool>>();
      }

      bool flag_open_file = getFlagOpenTable();
      map_pq_tracing_info[key].push_back(make_tuple(fd, LSM_level, flag_open_file));
      v_pq_tracing_info.push_back(make_tuple(fd, LSM_level, flag_open_file));
      
      clearFlagOpenTable();
    }
    std::string getMapPQTracingInfo(std::string sep, std::string bracket, std::string prefix, int i_round){
      std::stringstream result;
      
      result << sep << bracket << prefix << " map_pq_tracing_info key2fd_level round" << to_string(i_round) << bracket << ": " << "{" << "\n";
      
      string sep2 = "";
      for(auto &[k, v]: map_pq_tracing_info){
        result << sep2 << "\"" << to_string(k) << "\"" <<  ": [";
        sep2 = ", ";
        string sep3 = "";
        for(auto &fd_level: v){
          auto fd = std::get<0>(fd_level);
          auto LSM_level = std::get<1>(fd_level);
          auto flag_open_file = std::get<2>(fd_level);
          result << sep3 << "[" << fd << ", " << LSM_level << ", " << flag_open_file << "] ";
          sep3 = ", ";
        }
        result << "] " << "\n";
      }
      result << "}" << "\n";

      return result.str();
    }
    std::string getVPQTracingInfo(std::string sep, std::string bracket, std::string prefix, int i_round){
      std::stringstream result;

      result << sep << bracket << prefix << " v_pq_tracing_info key2fd_level round" << to_string(i_round) << bracket << ": " << "[" << "\n";

      string sep2 = "";
      for(auto &fd_level: v_pq_tracing_info){
          auto fd = std::get<0>(fd_level);
          auto LSM_level = std::get<1>(fd_level);
          auto flag_open_file = std::get<2>(fd_level);
        result << sep2 << "[" << fd << ", " << LSM_level << ", " << flag_open_file << "] ";
        sep2 = ", ";
      }

      result << "]" << "\n";

      return result.str();
    }






    void setSkipReadingRangeDeleteBlock(bool flag){
      flag_skip_reading_range_delete_block = flag;
    }
    bool isSkipReadingRangeDeleteBlock(){
      return flag_skip_reading_range_delete_block;
    }

    bool flag_is_running_PQ = false;
    bool isRunningPQ(){
      return flag_is_running_PQ;
    }
    void setRunningPQ(){
      flag_is_running_PQ = true;
    }
    void resetRunningPQ(){
      flag_is_running_PQ = false;
    }


    void resetDiskAccessCount(){
      disk_access_count = 0;
    }

    void increaseDiskAccessCount(){
      disk_access_count++;
    }

    int getDiskAccessCount(){
      return disk_access_count;
    }

    void increaseeReadEntryBlockCount(){
      read_entry_block_count++;
    }

    void resetReadEntryBlockCount(){
      read_entry_block_count = 0;
    }

    int getReadEntryBlockCount(){
      return read_entry_block_count;
    }

    void increaseNumIndexReadCount(){
      num_index_read_count++;
    }
    void increaseNumFilterReadCount(){
      num_filter_read_count++;
    }
    void increaseNumRangeDelReadCount(){
      num_range_del_read_count++;
    }
    void increaseNumTotalBlockReadCount(){
      num_total_block_read_count++;
    }
    void resetNumIndexReadCount(){
      num_index_read_count = 0;
    }
    void resetNumFilterReadCount(){
      num_filter_read_count = 0;
    }
    void resetNumRangeDelReadCount(){
      num_range_del_read_count = 0;
    }
    void resetNumTotalBlockReadCount(){
      num_total_block_read_count = 0;
    }
    int getNumIndexReadCount(){
      return num_index_read_count;
    }
    int getNumFilterReadCount(){
      return num_filter_read_count;
    }
    int getNumRangeDelReadCount(){
      return num_range_del_read_count;
    }
    int getNumTotalBlockReadCount(){
      return num_total_block_read_count;
    }
    

    void increaseFilteredByRDFCount(){
      filtered_by_RDF_count++;
    }
    void resetFilteredByRDFCount(){
      filtered_by_RDF_count = 0;
    }
    int getFilteredByRDFCount(){
      return filtered_by_RDF_count;
    }

    void increaseBlockBasedTableOpenCount(){
      block_based_table_open_count++;
    }
    void resetBlockBasedTableOpenCount(){
      block_based_table_open_count = 0;
    }
    int getBlockBasedTableOpenCount(){
      return block_based_table_open_count;
    }





    void increaseFetcherNumCompressionDictBlockReadCount(){
      fetcher__num_compression_dict_block_read_count++;
    }
    void increaseFetcherNumIndexReadCount(){
      fetcher__num_index_read_count++;
    }
    void increaseFetcherNumFilterReadCount(){
      fetcher__num_filter_read_count++;
    }
    void increaseFetcherNumRangeDelReadCount(){
      fetcher__num_range_del_read_count++;
    }
    void increaseFetcherNumTotalBlockReadCount(){
      fetcher__num_total_block_read_count++;
    }
    void increaseFetcherNumDataReadCount(){
      fetcher__num_data_read_count++;
    }
    void resetFetcherNumCompressionDictBlockReadCount(){
      fetcher__num_compression_dict_block_read_count = 0;
    }
    void resetFetcherNumIndexReadCount(){
      fetcher__num_index_read_count = 0;
    }
    void resetFetcherNumFilterReadCount(){
      fetcher__num_filter_read_count = 0;
    }
    void resetFetcherNumRangeDelReadCount(){
      fetcher__num_range_del_read_count = 0;
    }
    void resetFetcherNumTotalBlockReadCount(){
      fetcher__num_total_block_read_count = 0;
    }
    void resetFetcherNumDataReadCount(){
      fetcher__num_data_read_count = 0;
    }
    int getFetcherNumCompressionDictBlockReadCount(){
      return fetcher__num_compression_dict_block_read_count;
    }
    int getFetcherNumIndexReadCount(){
      return fetcher__num_index_read_count;
    }
    int getFetcherNumFilterReadCount(){
      return fetcher__num_filter_read_count;
    }
    int getFetcherNumRangeDelReadCount(){
      return fetcher__num_range_del_read_count;
    }
    int getFetcherNumTotalBlockReadCount(){
      return fetcher__num_total_block_read_count;
    }
    int getFetcherNumDataReadCount(){
      return fetcher__num_data_read_count;
    }


    std::string getAllCount(std::string sep, std::string bracket, std::string prefix, int N_repetitions){
      std::stringstream result;
      
      result << sep << bracket << prefix << "disk_access_count" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*disk_access_count / N_repetitions << "\n";
      result << sep << bracket << prefix << "read_entry_block_count" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*read_entry_block_count / N_repetitions << "\n";
      result << "\n";

      // result << sep << bracket << prefix << "duration__get_rdf" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*get_total_duration__get_rdf() / N_repetitions / 1000 << "\n";
      // result << sep << bracket << prefix << "duration__get_max_seq" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*get_total_duration__get_max_seq() / N_repetitions / 1000 << " ms " << "\n";
      // result << sep << bracket << prefix << "duration__retrieve_block" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*get_total_duration__retrieve_block() / N_repetitions / 1000 << " ms " << "\n";
      result << sep << bracket << prefix << "duration__get_rdf" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*get_total_duration__get_rdf() / N_repetitions << "\n";
      result << sep << bracket << prefix << "duration__get_max_seq" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*get_total_duration__get_max_seq() / N_repetitions  << "\n";
      result << sep << bracket << prefix << "duration__retrieve_block" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*get_total_duration__retrieve_block() / N_repetitions  << "\n";
      result << sep << bracket << prefix << "duration__find_table" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*get_total_duration__find_table() / N_repetitions  << "\n";
      result << sep << bracket << prefix << "duration__get_from_row_cache" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*get_total_duration__get_from_row_cache() / N_repetitions  << "\n";
      result << sep << bracket << prefix << "duration__remaining_get_path" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*get_total_duration__remaining_get_path() / N_repetitions  << "\n";
      result << "\n";

      result << sep << bracket << prefix << "num_index_read_count" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*num_index_read_count / N_repetitions << "\n";
      result << sep << bracket << prefix << "num_filter_read_count" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*num_filter_read_count / N_repetitions << "\n";
      result << sep << bracket << prefix << "num_range_del_read_count" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*num_range_del_read_count / N_repetitions << "\n";
      result << sep << bracket << prefix << "num_total_block_read_count" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*num_total_block_read_count / N_repetitions << "\n";
      result << sep << bracket << prefix << "block_based_table_open_count" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*block_based_table_open_count / N_repetitions << "\n";

      result << "\n";

      result << sep << bracket << prefix << "filtered_by_RDF_count" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*filtered_by_RDF_count / N_repetitions << "\n";
      result << "\n";

      result << sep << bracket << prefix << "fetcher__num_compression_dict_block_read_count" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*fetcher__num_compression_dict_block_read_count / N_repetitions << "\n";
      result << sep << bracket << prefix << "fetcher__num_index_read_count" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*fetcher__num_index_read_count / N_repetitions << "\n";
      result << sep << bracket << prefix << "fetcher__num_filter_read_count" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*fetcher__num_filter_read_count / N_repetitions << "\n";
      result << sep << bracket << prefix << "fetcher__num_range_del_read_count" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*fetcher__num_range_del_read_count / N_repetitions << "\n";
      result << sep << bracket << prefix << "fetcher__num_data_read_count" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*fetcher__num_data_read_count / N_repetitions << "\n";
      result << sep << bracket << prefix << "fetcher__num_total_block_read_count" << bracket << ": " << std::fixed << std::setprecision(2) << 1.0*fetcher__num_total_block_read_count / N_repetitions << "\n";
      result << "\n";  

      return result.str();
    }


    void resetAllCount(){
      resetDiskAccessCount();
      resetReadEntryBlockCount();

      resetFilteredByRDFCount();
      resetBlockBasedTableOpenCount();

      resetNumIndexReadCount();
      resetNumFilterReadCount();
      resetNumRangeDelReadCount();
      resetNumTotalBlockReadCount();

      resetFilteredByRDFCount();

      resetFetcherNumCompressionDictBlockReadCount();
      resetFetcherNumIndexReadCount();
      resetFetcherNumFilterReadCount();
      resetFetcherNumRangeDelReadCount();
      resetFetcherNumDataReadCount();
      resetFetcherNumTotalBlockReadCount();
    }

    void resetAllDuration(){
      reset_total_duration__get_rdf();
      reset_total_duration__get_max_seq();
      reset_total_duration__retrieve_block();
      reset_total_duration__find_table();
      reset_total_duration__get_from_row_cache();
      reset_total_duration__remaining_get_path();
    }


    // std::unordered_map<int, std::string> RDFTypes = {{0, "NONE"}, {1, "PLRDF"}, {2, "SPLIT_PLRDF"}};
    // std::unordered_map<int, std::string> RDFTypes = {{0, "NONE"}, {1, "PLRDF"}};
    // std::unordered_map<int, std::string> RDFTypes = {{0, "NONE"}, {1, "PLRDF"}, {2, "NONE"}};
    // std::unordered_map<int, std::string> RDFTypes = {{0, "NONE"}, {1, "PLRDF"}, {2, "SPLIT_PLRDF"}, {3, "NONE"}};
    // std::unordered_map<int, std::string> RDFTypes = {{0, "NONE"}, {1, "PLRDF"}, {2, "SPLIT_PLRDF"}, {3, "TOP_LEVEL_RDF"}, {4, "NONE2"}};
    // std::unordered_map<int, std::string> RDFTypes = { {0, "NONE"}, {1, "PLRDF"}, {2, "SPLIT_PLRDF"}, {3, "TOP_LEVEL_RDF"}}; // <-- debugging
    // std::unordered_map<int, std::string> RDFTypes = { {0, "SPLIT_PLRDF"}, {1, "TOP_LEVEL_RDF"}}; // <-- debugging
    // std::unordered_map<int, std::string> RDFTypes = { {0, "SKYLINE_RDF"}}; // <-- debugging
    // std::unordered_map<int, std::string> RDFTypes = {{0, "NONE"}, {1, "PLRDF"}, {2, "SPLIT_PLRDF"}, {3, "TOP_LEVEL_RDF"}, {4, "SKYLINE_RDF"},  {5, "NONE_DUMMY"}, {6, "NONE2"}};
    // std::unordered_map<int, std::string> RDFTypes = {{0, "NONE_DUMMY"}, {1, "NONE"}, {2, "NONE2"}, {3, "PLRDF"}, {4, "SPLIT_PLRDF"}, {5, "TOP_LEVEL_RDF"}, {6, "SKYLINE_RDF"},  {7, "NONE_DUMMY"}};
    // std::unordered_map<int, std::string> RDFTypes = {{0, "NONE_DUMMY"}, {1, "NONE"}, {2, "NONE2"}, {3, "PLRDF"}, {4, "SPLIT_PLRDF"}, {5, "TOP_LEVEL_RDF"}, {6, "SKYLINE_RDF"},  {7, "SuRF_LF_RDF"}, {8, "NONE_DUMMY"}};
    std::unordered_map<int, std::string> RDFTypes = {{0, "SuRF_LF_RDF"}, {1, "NONE_DUMMY"}};
    // std::unordered_map<int, std::string> RDFTypes = {{0, "TOP_LEVEL_RDF"}}; 

    int RDFType_chosed = 0;
        
    uint getNumberOfRDFTypes(){
      return RDFTypes.size();
    }

    //{0, "NONE"}, {1, "PLRDF"}, {2, "SPLIT_PLRDF"}
    void setRDFTypeChosed(int id){
      RDFType_chosed = id;
    }

    std::string getStringOfRDFTypeChosed(){
      return RDFTypes[RDFType_chosed];
    }





    void enable_log__deleted_keys__max_sequnce_number(){
      flag_log__deleted_keys__max_sequnce_number = true;
    }

    void disable_log__deleted_keys__max_sequnce_number(){
      flag_log__deleted_keys__max_sequnce_number = false;
    }

    bool is_enable_log__deleted_keys__max_sequnce_number(){
      return flag_log__deleted_keys__max_sequnce_number;
    }

    void insert_deleted_keys__max_sequnce_number(long long key, uint64_t max_sequnce_number){
      deleted_keys__max_sequnce_number[key] = max_sequnce_number;
    }

    uint64_t get_deleted_keys__max_sequnce_number(long long key){
      if(deleted_keys__max_sequnce_number.find(key) == deleted_keys__max_sequnce_number.end()){
        return 0;
      }
      return deleted_keys__max_sequnce_number[key];
    }

    void set_flag_is_RDF_filtered_entry(){
      flag_is_RDF_filtered_entry = true;
    }

    void reset_flag_is_RDF_filtered_entry(){
      flag_is_RDF_filtered_entry = false;
    }

    bool get_flag_is_RDF_filtered_entry(){
      return flag_is_RDF_filtered_entry;
    }

    void set_flag_testing_on_currently_deleted_keys(){
      flag_testing_on_currently_deleted_keys = true;
    }

    void reset_flag_testing_on_currently_deleted_keys(){
      flag_testing_on_currently_deleted_keys = false;
    }

    bool get_flag_testing_on_currently_deleted_keys(){
      return flag_testing_on_currently_deleted_keys;
    }





    std::chrono::_V2::system_clock::time_point  start__get_rdf = std::chrono::high_resolution_clock::now();
    std::chrono::_V2::system_clock::time_point  stop__get_rdf = std::chrono::high_resolution_clock::now();
    std::chrono::nanoseconds duration__get_rdf_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop__get_rdf - start__get_rdf);
    // unsigned long long duration__get_rdf = duration__get_max_seq_us.count();
    unsigned long long total_duation__get_rdf = 0;


    std::chrono::_V2::system_clock::time_point  start__get_max_seq = std::chrono::high_resolution_clock::now();
    std::chrono::_V2::system_clock::time_point  stop__get_max_seq = std::chrono::high_resolution_clock::now();
    std::chrono::nanoseconds duration__get_max_seq_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop__get_max_seq - start__get_max_seq);
    // unsigned long long duration__get_max_seq = duration__get_max_seq_us.count();
    unsigned long long total_duation__get_max_seq = 0;

    std::chrono::_V2::system_clock::time_point  start__retrieve_block = std::chrono::high_resolution_clock::now();
    std::chrono::_V2::system_clock::time_point  stop__retrieve_block = std::chrono::high_resolution_clock::now();
    std::chrono::nanoseconds duration__retrieve_block_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop__retrieve_block - start__retrieve_block);
    // unsigned long long duration__retrieve_block = duration__retrieve_block_us.count();
    unsigned long long total_duation__retrieve_block = 0;


    std::chrono::_V2::system_clock::time_point  start__find_table = std::chrono::high_resolution_clock::now();
    std::chrono::_V2::system_clock::time_point  stop__find_table = std::chrono::high_resolution_clock::now();
    std::chrono::nanoseconds duration__find_table_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop__find_table - start__find_table);
    unsigned long long total_duation__find_table = 0;


    std::chrono::_V2::system_clock::time_point  start__get_from_row_cache = std::chrono::high_resolution_clock::now();
    std::chrono::_V2::system_clock::time_point  stop__get_from_row_cache = std::chrono::high_resolution_clock::now();
    std::chrono::nanoseconds duration__get_from_row_cache_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop__get_from_row_cache - start__get_from_row_cache);
    unsigned long long total_duation__get_from_row_cache = 0;


    std::chrono::_V2::system_clock::time_point  start__remaining_get_path = std::chrono::high_resolution_clock::now();
    std::chrono::_V2::system_clock::time_point  stop__remaining_get_path = std::chrono::high_resolution_clock::now();
    std::chrono::nanoseconds duration__remaining_get_path_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop__remaining_get_path - start__remaining_get_path);
    unsigned long long total_duation__remaining_get_path = 0;




    void reset_total_duration__get_rdf(){
      total_duation__get_rdf = 0;
    }
    unsigned long long get_total_duration__get_rdf(){
      return total_duation__get_rdf;
    }
    void start_get_rdf(){
      start__get_rdf = std::chrono::high_resolution_clock::now();
    }
    void stop_get_rdf(){
      stop__get_rdf = std::chrono::high_resolution_clock::now();
      duration__get_rdf_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop__get_rdf - start__get_rdf);
      total_duation__get_rdf += duration__get_rdf_ns.count();
    }

    void reset_total_duration__get_max_seq(){
      total_duation__get_max_seq = 0;
    }
    unsigned long long get_total_duration__get_max_seq(){
      return total_duation__get_max_seq;
    }
    void start_get_max_seq(){
      start__get_max_seq = std::chrono::high_resolution_clock::now();
    }
    void stop_get_max_seq(){
      stop__get_max_seq = std::chrono::high_resolution_clock::now();
      duration__get_max_seq_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop__get_max_seq - start__get_max_seq);
// std::cout << "duration__get_max_seq_us.count() = " << duration__get_max_seq_us.count() << " " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl;
      total_duation__get_max_seq += duration__get_max_seq_ns.count();
    }

    void reset_total_duration__retrieve_block(){
      total_duation__retrieve_block = 0;
    }
    unsigned long long get_total_duration__retrieve_block(){
      return total_duation__retrieve_block;
    }
    void start_retrieve_block(){
      start__retrieve_block = std::chrono::high_resolution_clock::now();
    }
    void stop_retrieve_block(){
      stop__retrieve_block = std::chrono::high_resolution_clock::now();
      duration__retrieve_block_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop__retrieve_block - start__retrieve_block);
// std::cout << "duration__retrieve_block_us.count() = " << duration__retrieve_block_us.count() << " " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl;
      total_duation__retrieve_block += duration__retrieve_block_ns.count();
    }

    void reset_total_duration__find_table(){
      total_duation__find_table = 0;
    }
    unsigned long long get_total_duration__find_table(){
      return total_duation__find_table;
    }
    void start_find_table(){
      start__find_table = std::chrono::high_resolution_clock::now();
    }
    void stop_find_table(){
      stop__find_table = std::chrono::high_resolution_clock::now();
      duration__find_table_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop__find_table - start__find_table);
// std::cout << "duration__remaining_get_path_us.count() = " << duration__remaining_get_path_us.count() << " " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl;
      total_duation__find_table += duration__find_table_ns.count();
    }


    void reset_total_duration__get_from_row_cache(){
      total_duation__get_from_row_cache = 0;
    }
    unsigned long long get_total_duration__get_from_row_cache(){
      return total_duation__get_from_row_cache;
    }
    void start_get_from_row_cache(){
      start__get_from_row_cache = std::chrono::high_resolution_clock::now();
    }
    void stop_get_from_row_cache(){
      stop__get_from_row_cache = std::chrono::high_resolution_clock::now();
      duration__get_from_row_cache_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop__get_from_row_cache - start__get_from_row_cache);
      total_duation__get_from_row_cache += duration__get_from_row_cache_ns.count();
    }


    void reset_total_duration__remaining_get_path(){
      total_duation__remaining_get_path = 0;
    }
    unsigned long long get_total_duration__remaining_get_path(){
      return total_duation__remaining_get_path;
    }
    void start_remaining_get_path(){
      start__remaining_get_path = std::chrono::high_resolution_clock::now();
    }
    void stop_remaining_get_path(){
      stop__remaining_get_path = std::chrono::high_resolution_clock::now();
      duration__remaining_get_path_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop__remaining_get_path - start__remaining_get_path);
// std::cout << "duration__remaining_get_path_us.count() = " << duration__remaining_get_path_us.count() << " " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl;
      total_duation__remaining_get_path += duration__remaining_get_path_ns.count();
    }







    map<long long, string> groundTruth;
    set<long long> historicExistingKeys;
    vector<long long> currentlyNonInsertedKeys;
    using pll2 = pair<long long, long long>;
    vector<pll2> RDs;
    int deleted_key_count = 0;


    void insert(long long key, string value){
      groundTruth[key] = value;
      historicExistingKeys.insert(key);
    }

    void rangeDelete(long long start_key, long long end_key){
      auto it = groundTruth.lower_bound(start_key);
      for(;it != groundTruth.end() && it->first < end_key;){
        groundTruth.erase(it++); 
        deleted_key_count++;
      }
      RDs.push_back(make_pair(start_key, end_key));
    }

    int getDeletedKeyCount(){
      return deleted_key_count;
    }

    bool isKeyExist(long long key){
      return groundTruth.find(key) != groundTruth.end();
    }

    string get(long long key){
      if(groundTruth.find(key) == groundTruth.end()){
        return "";
      }
      return groundTruth[key];
    }



    int getCurrentlyNonInsertedKeysNum(){
      return CurrentlyNonInsertedKeysNum;
    }

    vector<vector<long long>> workload_all_existing_keys;
    vector<vector<long long>> workload_historic_existing_keys;
    vector<vector<long long>> workload_currently_deleted_keys;
    vector<vector<long long>> workload_currently_non_inserted_keys;

    void gen_workload_with_numbers_of_PQ(int N_repetitions, int number_of_PQs){
      workload_all_existing_keys.clear();
      workload_historic_existing_keys.clear();
      workload_currently_deleted_keys.clear();
      workload_currently_non_inserted_keys.clear();


      genCurrentlyNonInsertedKeys(1000);
      
      vector<long long> all_existing_keys = getAllExistingKeys(); 
      vector<long long> historic_existing_keys = getHistoricExistingKeys();
      vector<long long> currently_deleted_keys = getCurrentlyDeletedKeys();
      vector<long long> currently_non_inserted_keys = getCurrentlyNonInsertedKeys();

      if(number_of_PQs == -1){
        for(int i = 0; i < N_repetitions; i++){
          workload_all_existing_keys.push_back(all_existing_keys);
          workload_historic_existing_keys.push_back(historic_existing_keys);
          workload_currently_deleted_keys.push_back(currently_deleted_keys);
          workload_currently_non_inserted_keys.push_back(currently_non_inserted_keys);
        }

        return;
      }


      workload_all_existing_keys = vector<vector<long long>>(N_repetitions, vector<long long>(number_of_PQs));
      workload_historic_existing_keys = vector<vector<long long>>(N_repetitions, vector<long long>(number_of_PQs));
      workload_currently_deleted_keys = vector<vector<long long>>(N_repetitions, vector<long long>(number_of_PQs));
      workload_currently_non_inserted_keys = vector<vector<long long>>(N_repetitions, vector<long long>(number_of_PQs));

      size_t len_all_existing_keys = all_existing_keys.size();
      size_t len_historic_existing_keys = historic_existing_keys.size();
      size_t len_currently_deleted_keys = currently_deleted_keys.size();
      size_t len_currently_non_inserted_keys = currently_non_inserted_keys.size();
      
      if(len_currently_deleted_keys <= 0){
        workload_currently_deleted_keys = vector<vector<long long>>(N_repetitions, vector<long long>());
      }
      
      //random picking num_of_PQs points
      for(int i = 0; i < N_repetitions; i++){
        int idx;
        for(int j = 0; j < number_of_PQs; j++){
          idx = (int) (rand() % len_all_existing_keys);
          workload_all_existing_keys[i][j] = all_existing_keys[idx];

          idx = (int) (rand() % len_historic_existing_keys);
          workload_historic_existing_keys[i][j] = historic_existing_keys[idx];

          if(len_currently_deleted_keys > 0){
            idx = (int) (rand() % len_currently_deleted_keys);
            workload_currently_deleted_keys[i][j] = currently_deleted_keys[idx];
          }

          idx = (int) (rand() % len_currently_non_inserted_keys);
          workload_currently_non_inserted_keys[i][j] = currently_non_inserted_keys[idx];
        }
      }

      return;
    }


    vector<long long> getAllExistingKeys(){
      vector<long long> result;
      for(auto it = groundTruth.begin(); it != groundTruth.end(); it++){
        result.push_back(it->first);
      }
      return result;
    }    
    
    vector<long long> getHistoricExistingKeys(){
      vector<long long> result;
      for(auto it = historicExistingKeys.begin(); it != historicExistingKeys.end(); it++){
        result.push_back(*it);
      }
      return result;
    }

    vector<long long> getCurrentlyDeletedKeys(){
      vector<long long> result;
      for(auto it = historicExistingKeys.begin(); it != historicExistingKeys.end(); it++){
        if(groundTruth.count(*it) == 0){
          result.push_back(*it);
        }
      }
      return result;
    }

    void genCurrentlyNonInsertedKeys(int num){
      if(RDs.size() > 0){
        sort(RDs.begin(), RDs.end());
      }
      vector<pll2> RDS2;
      if(RDs.size() > 0){
        pll2 range = RDs[0];
        auto it = RDs.begin();
        auto ite = RDs.end();
        for(; it != ite; it++){
          if(it->first <= range.second){
            range.second = max(range.second, it->second);
          }else{
            RDS2.push_back(range);
            range = *it;
          }
        }
        RDS2.push_back(range);
      }
      RDs = RDS2;

      // long long tot_range = 0;
      // for(auto &range: RDs){
      //   tot_range += range.second - range.first;
      // }
      size_t len_RDs = RDs.size();

      currentlyNonInsertedKeys.clear();
      int max_trial = num * 3;
      int i_trial = 0;
      while(num && len_RDs > 0){
        int i_RDs = rand() % len_RDs;
        pll2 range = RDs[i_RDs];
        int len_range = range.second - range.first;
        long long diff = rand() % len_range;
        long long key = range.first + diff;
        if(groundTruth.count(key) == 0){
          currentlyNonInsertedKeys.push_back(key);
          num--;
        }


        i_trial ++;
        if(i_trial >= max_trial){
          break;
        }
      }

      // currentlyNonInsertedKeys.clear();
      while(num){
        long long key = rand() % 100000000;
        if(groundTruth.count(key) == 0){
          currentlyNonInsertedKeys.push_back(key);
          num--;
        }
      }
      return;
    }

    // vector<long long> getCurrentlyNonInsertedKeys(int num){
    vector<long long> getCurrentlyNonInsertedKeys(){
      return currentlyNonInsertedKeys;
    }


    vector<long long> getAllExistingKeysAtNRound(int n_round){
      assert(n_round < workload_all_existing_keys.size());
      return workload_all_existing_keys[n_round];
    }

    vector<long long> getHistoricExistingKeysAtNRound(int n_round){
      assert(n_round < workload_historic_existing_keys.size());
      return workload_historic_existing_keys[n_round];
    }

    vector<long long> getCurrentlyDeletedKeysAtNRound(int n_round){
      assert(n_round < workload_currently_deleted_keys.size());
      return workload_currently_deleted_keys[n_round];
    }

    vector<long long> getCurrentlyNonInsertedKeysAtNRound(int n_round){
      assert(n_round < workload_currently_non_inserted_keys.size());
      return workload_currently_non_inserted_keys[n_round];
    }




     std::string getCurrentlyDeletedKeysVec2dString(std::string sep, std::string bracket, std::string prefix){
      std::stringstream result;

      vector<vector<long long>> &vec2d = workload_currently_deleted_keys;

      int len =  vec2d.size();
      vector<long long> currently_deleted_keys = getCurrentlyDeletedKeys();
      size_t len_currently_deleted_keys = currently_deleted_keys.size();
      result << sep << bracket << prefix << " logNumCurrentlyDeletedDistinctKeys"  << bracket << ": " << len_currently_deleted_keys << "\n";
      result << sep << bracket << prefix << " logCurrentlyDeletedKeysVec2d"  << bracket << ": " << "[" << "\n";

      string sep2 = "";
      for(int i = 0; i < len; i++){
        result << sep2 << "[";

        string sep3 = "";
        vector<long long> &vec1d = vec2d[i];
        for(auto &key: vec1d){
          result << sep3 << key;
          sep3 = ", ";
        }
        result << "]" << "\n";
        sep2 = ", ";
      }
      result << "]" << "\n";


      return result.str();
    }

    // vector<int> checkOnExistingKeys();
    // vector<int> checkOnAllInsertedKeys();
    // vector<int> checkOnAllCurrentlyDeletedKeys();
    // RandomKeysTestingResult checkOnRandomKeys(int num);
    // void checkEquation();
  };


} // namespace




#endif /* SYSTEM_VERIFIER_H_ */



