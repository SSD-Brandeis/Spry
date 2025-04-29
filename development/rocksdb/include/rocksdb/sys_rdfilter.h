/*
 *  Created on: April 23, 2023
 *  Author: yucheng
 */

/*Notice: anytime only one lock is locked in this class*/


#ifndef SYS_RDFILTER_H_
#define SYS_RDFILTER_H_

#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include <utility>
#include <type_traits>

// namespace rdfilter {
//   // class PerlevelRangeDeleteFilterByVector {
//   class PLRDF;
// } 

using pll = std::pair<long long, long long>; //[start, end)
using pss = std::pair<std::string, std::string>; //[start, end)
using t3ll = std::tuple<long long, long long, long long>; //([start, end), time)
// using t2str1l = std::tuple<std::string, std::string, long long>; //([start, end), time)
  
// using namespace ROCKSDB_NAMESPACE;
// namespace ROCKSDB_NAMESPACE{
  class PLRDF;
  class SkyLineRDF;
// }

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
#include <utility>
#include <queue>

#include <string>
#include <chrono>
#include <thread>
#include <condition_variable>

// #if __has_include("semaphore.h")
// # include "semaphore.h"
// #else
// # include <ucontex.h>
// # include <sys/sem.h>
// #endif
// #include <semaphore.h>
// #include <ucontex.h>
#include <pthread.h>
#include <mutex>

// #include "../db/version_edit.h"



using namespace std;
// using namespace ROCKSDB_NAMESPACE;
// namespace ROCKSDB_NAMESPACE{
  // class PerlevelRangeDeleteFilterByVector {  


  // class PLRDF {  
  //   private:
  //     std::unordered_map<uint64_t, std::vector<pll>> rd_filter_level0; //for level 0, (file_num, RD_list), FileMetaData* -> fd .GetNumber();

  //     std::vector<std::vector<pll>> rd_filter; //for level > 0, list of range delete (start, end), all entries are non-overlapping
  //     std::vector<int> numbers_of_ranges_in_RDF_log; //for level > 0, number of ranges in RDF
      
  //     void addRangeDelete_internal(uint level, std::vector<pll> &range_delete_list_in);
  //     std::vector<pll> sortAndMerge(std::vector<pll> &range_delete_list_in);
  //     void addRangeDelete(std::vector<pll> &range_delete_list, std::vector<pll> &range_delete_list_in);
  //     void addRangeDelete(std::vector<pll> &range_delete_list, long long start, long long end);
  //     void print_internal();


  //     /*
  //       * adjust range deletes as per the compaction
  //       */
  //     void adjustRangeDeletesForLevel0Input(uint olevel, std::vector<uint64_t> file_numbers);
  //     void adjustRangeDeletes(uint clevel, uint olevel, std::vector<std::pair<long long, long long>> one_level_compaction_file_boundaries);

  //   public:
  //     // std::vector<pll> getRangeDeleteList();
  //     void insertRangeDeleteToLevel0(uint64_t file_num, std::vector<pll> &range_delete_list_in, std::vector<uint64_t> exist_level0_file_nums);

  //     void addRangeDelete(uint level, long long start, long long end);
  //     void addRangeDelete(uint level, std::vector<pll> &range_delete_list_in);
  //     void shiftRDFToOutputLevel(std::vector<std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>> *file_meta_data_vectors);
  //     void deleteLastLevelIfEqualsBottomLevel(uint bottom_level);
  //     // void deleteRDFAssociatedWithFilesAtCurrentLevel(std::tuple<int, const std::vector<FileMetaData*>*> *file_meta_data);
  //     void deleteRDFAssociatedWithFilesAtCurrentLevel(std::tuple<int, std::vector<pll>, std::vector<uint64_t>> *file_meta_data);

  //     std::vector<pll> getLevelRanges(int outlevel);
  //     void setLevelRanges(std::vector<pll> level_ranges_in, int outlevel);

  //     int getNumberOfTotalLevels();
  //     int getNumberOfTotalRanges();

  //     void printLevel0();
  //     void print();

  //     bool isEntryAlive(uint level, long long key);


  //     void splitRangesOnLevel(uint level, std::vector<long long> keys);
  //     // int getRangeDeleteCount();

  //     void logCurrentTotalNumbersOfRanges();
  //     std::vector<int> getNumbersOfRangesInRDFLog();
  // };


  class PLRDF_Env {
    private:
      static PLRDF_Env *plrdf_Env_ptr;
      bool flag_key_may_deleted = false; // used during get()
    public:
      static PLRDF_Env* getInstance();
      
      // Start: used during Get()
      void setFlagKeyMayDeleted(bool flag){
          flag_key_may_deleted = flag;
      }
      bool getFlagKeyMayDeleted(){
          return flag_key_may_deleted;
      }
      void clearFlagKeyMayDeleted(){
          flag_key_may_deleted = false;
          // key_search_count_kmd = 0;
      }
      // End: used during Get()
  };
  
  

  class SkyLineRDF {
    private:
      std::vector<t3ll> rd_list;
      std::vector<int> numbers_of_ranges_in_RDF_log;
      std::vector<int> memory_usage_in_RDF_log;
    public:      
      // void addRangeDelete(std::vector<t3ll> &range_delete_list_in);
      void addRangeTombstones(std::vector<t3ll> range){
        if(range.size() == 0){return;}

        std::vector<t3ll> tmp_v; //start, end, seq
        for(auto &r: range){
          tmp_v.push_back(r);
        }
        for(auto &r: rd_list){
          tmp_v.push_back(r);
        }

        std::sort(tmp_v.begin(), tmp_v.end());
        std::priority_queue<pll> pq; // seq, end
        std::vector<t3ll> out_v; // start, end, seq

        auto t_cur = std::get<0>(tmp_v[0]);
        for(auto &x: tmp_v){
          auto start = std::get<0>(x);
          auto end = std::get<1>(x);
          auto seq = std::get<2>(x);
          // if(!pq.empty() && +pq.top().second <= start){
          while(!pq.empty() && +pq.top().second <= start){
            pll p = pq.top();
            pq.pop();
            auto seq2 = +p.first;
            auto end2 = +p.second;
            if(end2 <= t_cur){continue;}
    // std::cout << " t_cur = " << t_cur << " end2 = " << end2 << " seq2 = " << seq2 << " start = " << start << " "
    //           << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
            out_v.push_back(std::make_tuple(t_cur, end2, seq2));
            t_cur = end2;
          }
          // }

          if(pq.empty()){t_cur = start;}
          else{
            auto seq2 = +pq.top().first;
            out_v.push_back(std::make_tuple(t_cur, start, seq2));
            t_cur = start;
          }

          pq.push(std::make_pair(+seq, +end));
        }
        while(!pq.empty()){
          pll p = pq.top();
          pq.pop();
          auto seq2 = +p.first;
          auto end2 = +p.second;
          if(end2 <= t_cur){continue;}

          out_v.push_back(std::make_tuple(t_cur, end2, seq2));
          t_cur = end2;
        }


        std::vector<t3ll> out_v2;
        int len_out_v = out_v.size();
        auto start = std::get<0>(out_v[0]);
        auto end = std::get<1>(out_v[0]);
        auto seq = std::get<2>(out_v[0]);
        for(int i = 1; i < len_out_v; i++){
          if(std::get<1>(out_v[i-1]) == std::get<0>(out_v[i]) && 
            std::get<2>(out_v[i-1]) == std::get<2>(out_v[i])){
            end = std::get<1>(out_v[i]);
          }else{
            out_v2.push_back(std::make_tuple(start, end, seq));
            start = std::get<0>(out_v[i]);
            end = std::get<1>(out_v[i]);
            seq = std::get<2>(out_v[i]);
          }
        }
        out_v2.push_back(std::make_tuple(start, end, seq));

        rd_list = out_v2;
        // skyline__numbers_of_ranges_in_rdf_log.push_back(out_v2.size());
      }

      bool isEntryAlive(long long key, long long seq){
          long long skyline__max_seq = getMaxSeq(key);
          return seq >= skyline__max_seq;
      }
      // void print();
      void logCurrentTotalNumbersOfRanges(){
        numbers_of_ranges_in_RDF_log.push_back(getNumberOfTotalRanges());
      }
      std::vector<int> getNumbersOfRangesInRDFLog(){
        return numbers_of_ranges_in_RDF_log;
      }
      void logCurrentTotalMemoryUsage(){
        memory_usage_in_RDF_log.push_back(getNumberOfTotalMemoryUsage());
      }
      std::vector<int> getMemoryUsageInRDFLog(){
        return memory_usage_in_RDF_log;
      }

      int getNumberOfTotalRanges(){
        return rd_list.size();
      }

      int getNumberOfTotalMemoryUsage(){
        return rd_list.size()*sizeof(t3ll);
      }

      long long getMaxSeq(long long key){
        auto it = std::lower_bound(rd_list.begin(), rd_list.end(), key, [](auto &a, long long b){return get<1>(a) <= b;} );
        if(it == rd_list.end()){return 0;}
    // std::cout << " min = " << std::get<0>(*it) << " max = " << std::get<1>(*it) << " seq = " << std::get<2>(*it) << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

        if(std::get<0>(*it) <= key && key < std::get<1>(*it)){
          return std::get<2>(*it);
        }
        return 0;
      }
      
      void print(){
        std::cout << "Skyline RDF" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        // for(auto it = rd_list.begin(); it != rd_list.end(); it++){
        for(auto &x: rd_list){
          auto start = std::get<0>(x);
          auto end = std::get<1>(x);
          auto seq = std::get<2>(x);
          std::cout << " [" << start << ", " << end << "] --(" << seq << ") ";
        }
        std::cout << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      }

  };
// }


// template<typename RangeType>
template<typename KeyType>
class PLRDF_t{
  using Pair = std::pair<KeyType, KeyType>;
  
  private:
    std::unordered_map<uint64_t, std::vector<Pair>> rd_filter_level0; //for level 0, (file_num, RD_list), FileMetaData* -> fd .GetNumber();

    std::vector<std::vector<Pair>> rd_filter; //for level > 0, list of range delete (start, end), all entries are non-overlapping
    std::vector<std::vector<KeyType>> split_keys; // for level > 0,
    std::vector<int> numbers_of_ranges_in_RDF_log; //for level > 0, number of ranges in RDF
    std::vector<int> numbers_of_split_keys_in_RDF_log; //for level > 0, number of split keys in RDF
    std::vector<int> memory_usage_in_RDF_log;

    // int may_be_deleted_count = 0;
    uint32_t key_search_count = 0, key_may_deleted_count = 0;
    uint32_t key_search_count_kmd = 0;
    bool flag_key_may_deleted = false;

    void addRangeDelete_internal(uint level, std::vector<Pair> &range_delete_list_in){
      // init();
      // update_mutex.lock();
      // std::lock_guard<std::mutex> guard(update_mutex);

      assert(rd_filter.size() >= level);
      while (rd_filter.size() <= level)
      {
        rd_filter.push_back(std::vector<Pair>());
        split_keys.push_back(std::vector<KeyType>());
      }

      addRangeDelete(rd_filter[level], range_delete_list_in);

      // update_mutex.unlock();
    }



    std::vector<Pair> sortAndMerge(std::vector<Pair> &range_delete_list_in){
      if(range_delete_list_in.size() == 0){
        return {};
      }

      std::sort(range_delete_list_in.begin(), range_delete_list_in.end(), [](Pair a, Pair b){
        return a.first < b.first;
      });

      std::vector<Pair> range_delete_list;
      range_delete_list.reserve(range_delete_list_in.size());
      auto itA = range_delete_list_in.begin();
      auto iteA = range_delete_list_in.end();
      Pair tmp_range = *itA;
      for(;itA != iteA; itA++){
        // if(tmp_range.second >= itA->first){
        if(tmp_range.second > itA->first){
            tmp_range.second = std::max(tmp_range.second, itA->second);
        }else{
          range_delete_list.push_back(tmp_range);
          tmp_range = *itA;
        }
      }

      range_delete_list.push_back(tmp_range);
      return range_delete_list;
    }



    void addRangeDelete(std::vector<Pair> &range_delete_list, std::vector<Pair> &range_delete_list_in){
    // init();
    // std::lock_guard<std::mutex> guard(init_mutex);

      auto& rdList = range_delete_list;
      auto& rdList_in = range_delete_list_in;

      if(rdList_in.size() == 0){return;}

      for(uint i = 1; i < rdList_in.size(); i++){
        if(rdList_in[i-1].first > rdList_in[i].first){
          std::cerr << "Error: rdList_in to be flushed has not been sorted in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl;
          exit(1);
        }
      }

      if(rdList.size() == 0){
        //do the merging first before adding to rdList
        // std::vector<Pair> rdList_new;
        rdList.reserve(rdList_in.size());
        auto itA = rdList_in.begin();
        auto iteA = rdList_in.end();
        Pair tmp_range = *itA;
        for(;itA != iteA; itA++){
          // if(tmp_range.second >= itA->first){
          if(tmp_range.second > itA->first){
              tmp_range.second = std::max(tmp_range.second, itA->second);
          }else{
            rdList.push_back(tmp_range);
            tmp_range = *itA;
          }
        }

        rdList.push_back(tmp_range);
        
        return;
      }




      std::vector<Pair> rdList_new;
      rdList_new.reserve(rdList.size() + rdList_in.size());



      auto itA = rdList.begin();
      auto iteA = rdList.end();
      auto itB = rdList_in.begin();
      auto iteB = rdList_in.end();


      Pair tmp_range;
      if(itA->first < itB->first){
        tmp_range = *itA;
      }else{
        tmp_range = *itB;
      }

      while(itA != iteA || itB != iteB){
        // if(itA != iteA && itA->first <= tmp_range.second){
        if(itA != iteA && itA->first < tmp_range.second){
          tmp_range.second = std::max(tmp_range.second, itA->second);
          itA++;
          continue;
        }
        // if(itB != iteB && itB->first <= tmp_range.second){
        if(itB != iteB && itB->first < tmp_range.second){
          tmp_range.second = std::max(tmp_range.second, itB->second);
          itB++;
          continue;
        }
        rdList_new.push_back(tmp_range);

        if(itA == iteA){
          tmp_range = *itB;
          itB++;
          continue;
        }
        if(itB == iteB){
          tmp_range = *itA;
          itA++;
          continue;
        }

        if(itA->first <= itB->first){
          tmp_range = *itA;
          itA++;
        }else{
          tmp_range = *itB;
          itB++;
        }
      }
      rdList_new.push_back(tmp_range);



      rdList.clear();
      rdList.reserve(rdList_new.size());
      for(auto &p : rdList_new){
        rdList.push_back(p);
      }
    }


    void addRangeDelete(std::vector<Pair> &range_delete_list, Pair rd){
    // init();
    // std::lock_guard<std::mutex> guard(init_mutex);

    
    auto start = rd.first;
    auto end = rd.second;

    auto& rdList = range_delete_list;
    #ifdef DEBUG
      cout << "Adding range delete: " << start << " " << end << endl;
    #endif
      std::vector<Pair> rdList_new;
      rdList_new.reserve(rdList.size()+1);


      auto minK = start;
      auto maxK = end;
      
      auto it = rdList.begin();
      while ( it != rdList.end() ){
          // [a,b], [c,d]
          
          //[a, b), [c,d)
          // if (it->second < start){ rdList_new.push_back(*it); it++; continue;}
          // if (it->first > end){ break;}
          if (it->second <= start){ rdList_new.push_back(*it); it++; continue;}
          if (it->first >= end){ break;}

          minK = std::min(minK, it->first);
          maxK = std::max(maxK, it->second);
          it++;
      }
      rdList_new.push_back(Pair({minK, maxK}));
      while(it != rdList.end()){
        rdList_new.push_back(*it);
        it++;
      }

      rdList.clear();
      rdList.reserve(rdList_new.size());
      for(auto &p : rdList_new){
        rdList.push_back(p);
      }
    }

    void print_internal(){
      // init();
      // std::lock_guard<std::mutex> guard(update_mutex);

      std::cout <<  std::setfill('-') << std::setw(60) << " START: Print PL RDF " << std::setfill('-') << "" << std::endl;
      for(uint l = 0; l < rd_filter.size(); l++){
        std::cout << "Level: " << l << std::endl;
        auto& rdList = rd_filter[l];
        for(auto it = rdList.begin(); it != rdList.end(); it++){
          std::cout << "(" << it->first << " " << it->second << ") ";
        }
        std::cout << std::endl;
        
        auto& splitKeyList = split_keys[l];
        for(auto it = splitKeyList.begin(); it != splitKeyList.end(); it++){
          std::cout << (*it) << " ";
        }
        std::cout << std::endl;
      }
      std::cout <<  std::setfill('-') << std::setw(60) << " END: Print PL RDF " << std::setfill('-') << "" << std::endl;
    }



    void adjustRangeDeletesForLevel0Input(uint olevel, std::vector<uint64_t> file_numbers){

      std::vector<Pair> to_be_added_in_next_level_rdf;


      for(uint64_t &file_num: file_numbers){

        {
          // // -- rd_filter_level0 --
          // rd_filter_level0_mutex.lock();

          auto it = rd_filter_level0.find(file_num);
          if(it == rd_filter_level0.end()){
            assert(it != rd_filter_level0.end());
            std::cerr << "File number not found in level 0 " << "File number " << file_num << " " << __FILE__ << ":" << __LINE__ << std::endl; 
            std::cerr << "Remindation: Do the manually flush after all the insert workload are done. So no entries lie inside memtable anymore. In case those entries will go through the track of bulk buiding from WAL and no going through the path of flushJob." << "File number " << file_num << " " << __FILE__ << ":" << __LINE__ << std::endl; 
            exit(1);
            // continue;
          }
          auto val = it->second;
          to_be_added_in_next_level_rdf.insert(to_be_added_in_next_level_rdf.end(), val.begin(), val.end());
          rd_filter_level0.erase(it);
          
        //   rd_filter_level0_mutex.unlock();
        //   // -- rd_filter_level0 -- 
        }

      }

      std::sort(to_be_added_in_next_level_rdf.begin(), to_be_added_in_next_level_rdf.end(), [](const Pair a, const Pair b)
              { return a.first < b.first; });



      // // -- updating rd_filter_level0 --
      // std::lock_guard<std::mutex> guard(rd_filter_level0_mutex);

      addRangeDelete_internal(olevel, to_be_added_in_next_level_rdf);
    }



    // This would be used for trivial compaction and normal compaction
    // input_level, output_level, file_boundaries
    void adjustRangeDeletes(uint clevel, uint olevel, std::vector<Pair> one_level_compaction_file_boundaries){
      // // init();
      // std::lock_guard<std::mutex> guard(update_mutex);

      
      std::vector<Pair> new_current_level_rdf;
      std::vector<Pair> to_be_added_in_next_level_rdf;

      if (rd_filter.size() <= clevel)
      {
        return;
      }
      
      auto old_current_level_rdf = rd_filter[clevel];

      // FIXME: (Shubham) This might not be required
      if (one_level_compaction_file_boundaries.size() == 0)
      {
        return;
      }

      auto it = old_current_level_rdf.begin();
      auto itf = one_level_compaction_file_boundaries.begin();

      while (it != old_current_level_rdf.end())
      {
        Pair val = *it;

        /*
        *    |--|
        *         -----
        *         |   |
        *         -----
        */
      //  if (itf == one_level_compaction_file_boundaries.end() || (val.second <= file_boundry.first))
       if (itf == one_level_compaction_file_boundaries.end() || (val.second <= itf->first))
       {
          // new_current_level_rdf.push_back(val);
          if(val.first != val.second){
            new_current_level_rdf.push_back(val);
          }
          it++;
          continue;
        }

        
        auto file_boundries = *itf;
        Pair file_boundry = std::make_pair(file_boundries.first, file_boundries.second);
        /*
        *             |--|
        *     ------
        *     |    |
        *     ------
        */
        if (val.first > file_boundry.second)
        {
          itf++;
        }
        /*
        *    |------||||
        *         ------
        *         |    |
        *         ------
        */
        else if (val.first < file_boundry.first && val.second > file_boundry.first && val.second <= file_boundry.second)
        {
          // new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
          // to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first, val.second));
          if(val.first != file_boundry.first){
            new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
          }
          if(file_boundry.first != val.second){
            to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first, val.second));
          }
          it++;
        }
        /*
        *    |||--|||
        *    --------
        *    |      |
        *    --------
        */
        else if (val.first >= file_boundry.first && val.second <= file_boundry.second)
        {
          // to_be_added_in_next_level_rdf.push_back(val);
          if(val.first != val.second){
            to_be_added_in_next_level_rdf.push_back(val);
          }
          it++;
        }
        /*
        *     ||||-------|
        *     --------
        *     |      |
        *     --------
        */
        else if (val.first >= file_boundry.first && val.first <= file_boundry.second && val.second > file_boundry.second)
        {
          // to_be_added_in_next_level_rdf.push_back(std::make_pair(val.first, file_boundry.second + 1));
          // (*it).first = file_boundry.second + 1;
          to_be_added_in_next_level_rdf.push_back(std::make_pair(val.first, file_boundry.second));
          (*it).first = file_boundry.second;
          if((*it).first >= (*it).second){ it++; } // <------------------------
          itf++;
        }
        /*
        *  |------------|
        *     --------
        *     |      |
        *     --------
        */
        else if (val.first < file_boundry.first && val.second > file_boundry.second)
        {
          // new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
          if(val.first != file_boundry.first){
            new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
          }
          // to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first, file_boundry.second + 1));
          // (*it).first = file_boundry.second + 1;
          to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first, file_boundry.second));
          (*it).first = file_boundry.second;
          if((*it).first >= (*it).second){ it++; } // <------------------------
          itf++;
        }else{
          std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
          std::cerr << "val.first: " << val.first << " val.second: " << val.second << " file_boundry.first: " << file_boundry.first << " file_boundry.second: " << file_boundry.second << std::endl;
          assert(false);
          exit(1);
        }
      }

      rd_filter[clevel] = new_current_level_rdf;
      std::sort(to_be_added_in_next_level_rdf.begin(), to_be_added_in_next_level_rdf.end(), [](const Pair a, const Pair b)
              { return a.first < b.first; });

      addRangeDelete_internal(olevel, to_be_added_in_next_level_rdf);

      
      auto old_current_level_split_keys = split_keys[clevel];
      std::vector<KeyType> new_current_level_split_keys;
      auto it_p = old_current_level_split_keys.begin();
      auto it_pe = old_current_level_split_keys.end();
      auto it_r = new_current_level_rdf.begin();
      auto it_re = new_current_level_rdf.end();
      //it_p has keys sorted in ascending order 
      //it_r has ranges sorted in ascending order 
      //The goal is to populate new_current_level_split_keys with only those keys from old_current_level_split_keys 
      //that fall within the given ranges in new_current_level_rdf.
      while(it_p != it_pe && it_r != it_re){
        // if (*it_p < it_r->first) {
        if (*it_p <= it_r->first) {
            ++it_p;
        } else if (*it_p >= it_r->second) {
            ++it_r;
        } else {
            // *it_p is within the range [it_r->first, it_r->second)
            new_current_level_split_keys.push_back(*it_p);
            ++it_p;
        }
      }
      split_keys[clevel] = new_current_level_split_keys;
    }




  public:

    void insertRangeDeleteToLevel0(uint64_t file_num, std::vector<Pair> &range_delete_list_in, std::vector<uint64_t> exist_level0_file_nums){
      std::vector<Pair> sorted_merged_rdlist = sortAndMerge(range_delete_list_in);
      // init();

      // -- updating rd_filter_level0 --
      // std::lock_guard<std::mutex> guard(rd_filter_level0_mutex);
      // rd_filter_level0_mutex.lock();

      // if(rd_filter_level0.count(file_num) > 0){
      if(std::binary_search(exist_level0_file_nums.begin(), exist_level0_file_nums.end(), file_num) == true){
        std::cerr << "Error: file_num already exists in rd_filter_level0 " << "file_num = " << file_num << "\t" << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
        exit(1);
      }
      rd_filter_level0[file_num] = sorted_merged_rdlist;


      // rd_filter_level0_mutex.unlock();
      // -- updating rd_filter_level0 --
    }

    void printLevel0(){
      // init();
      // std::lock_guard<std::mutex> guard(rd_filter_level0_mutex);

      std::cout << "rd_filter_level0" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<  std::endl << std::endl;
      for(auto it = rd_filter_level0.begin(); it != rd_filter_level0.end(); it++){
        std::cout << "rd_filter_level0 file number: " << it->first << " number of RD: " << it->second.size() << std::endl;
        //print all ranges
        for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++){
          std::cout <<  "rd_filter_level0" << " " << it2->first << " " << it2->second << " ";
        }
        std::cout << std::endl;
      }
      std::cout << std::endl << std::endl;
    }



    void addRangeDelete(uint level, std::vector<Pair> &range_delete_list_in){
      // init();
      // // update_mutex.lock();
      // std::lock_guard<std::mutex> guard(update_mutex);

      while (rd_filter.size() <= level)
      {
        rd_filter.push_back(std::vector<Pair>());
      }

      addRangeDelete(rd_filter[level], range_delete_list_in);

      // update_mutex.unlock();
    }


    /*
    *Do insertion, even if the vector is empty, because we need to set condition_variable of mutex (semaphore) for compaction
    */
    // input_level, output_level, file_boundries, file_numbers
    void shiftRDFToOutputLevel(std::vector<std::tuple<int, int, std::vector<Pair>, std::vector<uint64_t>>>  *file_meta_data_vectors){
        // init();
      // std::lock_guard<std::mutex> guard(update_mutex);

      // // // FIXME: FOR TESTING (next 2 lines)


      for (auto file_meta_data : *file_meta_data_vectors)
      {
        int clevel = std::get<0>(file_meta_data);
        if(clevel == 0){
          adjustRangeDeletesForLevel0Input(std::get<1>(file_meta_data), std::get<3>(file_meta_data));
        }else{
          // file ranges
          std::vector<Pair> one_level_file_boundries;
          auto meta_data = std::get<2>(file_meta_data);

          for (auto meta : meta_data)
          {
  // std::cout << meta.first << " " << meta.second << endl;
            one_level_file_boundries.push_back(std::make_pair(meta.first, meta.second));
          }

          adjustRangeDeletes(std::get<0>(file_meta_data), std::get<1>(file_meta_data), one_level_file_boundries);
        }
      }

      // // // FIXME: FOR TESTING (next 2 lines)
      // // std::cout << "After Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      // // // update_mutex.lock();
      // print_internal();
      // // update_mutex.unlock();
    }




    // this is only used for direct compaction //
    // input_level, file_boundries, file_numbers
    void deleteRDFAssociatedWithFilesAtCurrentLevel(std::tuple<int, std::vector<Pair>, std::vector<uint64_t>> *file_meta_data){
      // init();
      // std::lock_guard<std::mutex> guard(update_mutex);

      // std::cout << "Before Deletion Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      // print_internal();

      std::vector<Pair> one_level_file_boundries;
      auto level = std::get<0>(*file_meta_data);

      if(level == 0){
        auto it = rd_filter_level0.find(level);
        if(it == rd_filter_level0.end()){
          assert(it != rd_filter_level0.end());
          std::cerr << "Error: file_num does not exist in rd_filter_level0" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl;
          exit(1);
        }
        rd_filter_level0.erase(it);
        return;
      }


      if (rd_filter.size() <= (uint)level)
      {
        return;
      }

      auto meta_data = std::get<1>(*file_meta_data);

      for (auto meta : meta_data)
      {
          one_level_file_boundries.push_back(std::make_pair(meta.first, meta.second));
      }

      std::vector<Pair> new_current_level_rdf;
      auto old_current_level_rdf = rd_filter[level];
      auto it = old_current_level_rdf.begin();
      auto itf = one_level_file_boundries.begin();

      while (it != old_current_level_rdf.end())
      {
        Pair val = *it;
        auto file_boundries = *itf;
        Pair file_boundry = std::make_pair(file_boundries.first, file_boundries.second);

        /*
        *    |--|
        *         -----
        *         |   |
        *         -----
        */
        if (itf == one_level_file_boundries.end() || (val.second <= file_boundry.first))
        {
          new_current_level_rdf.push_back(val);
          it++;
        }
        /*
        *             |--|
        *     ------
        *     |    |
        *     ------
        */
        else if (val.first > file_boundry.second)
        {
          itf++;
        }
        /*
        *    |------||||
        *         ------
        *         |    |
        *         ------
        */
        else if (val.first < file_boundry.first && val.second > file_boundry.first && val.second <= file_boundry.second)
        {
          new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
          it++;
        }
        /*
        *    |||--|||
        *    --------
        *    |      |
        *    --------
        */
        else if (val.first >= file_boundry.first && val.second <= file_boundry.second)
        {
          it++;
        }
        /*
        *     ||||-------|
        *     --------
        *     |      |
        *     --------
        */
        else if (val.first >= file_boundry.first && val.first <= file_boundry.second && val.second > file_boundry.second)
        {
          // (*it).first = file_boundry.second + 1;
          (*it).first = file_boundry.second;
          if((*it).first >= (*it).second){ it++; } // <------------------------
          itf++;
        }
        /*
        *  |------------|
        *     --------
        *     |      |
        *     --------
        */
        else if (val.first < file_boundry.first && val.second > file_boundry.second)
        {
          new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
          // (*it).first = file_boundry.second + 1;
          (*it).first = file_boundry.second;
          if((*it).first >= (*it).second){ it++; } // <------------------------
          itf++;
        }else{
          std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
          std::cerr << "val.first: " << val.first << " val.second: " << val.second << " file_boundry.first: " << file_boundry.first << " file_boundry.second: " << file_boundry.second << std::endl;
          assert(false);
          exit(1);
        }
      }

      rd_filter[level] = new_current_level_rdf;

      // std::cout << "After Deletion Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      // print_internal();
      
      auto old_current_level_split_keys = split_keys[level];
      std::vector<KeyType> new_current_level_split_keys;
      auto it_p = old_current_level_split_keys.begin();
      auto it_pe = old_current_level_split_keys.end();
      auto it_r = new_current_level_rdf.begin();
      auto it_re = new_current_level_rdf.end();
      //it_p has keys sorted in ascending order 
      //it_r has ranges sorted in ascending order 
      //The goal is to populate new_current_level_split_keys with only those keys from old_current_level_split_keys 
      //that fall within the given ranges in new_current_level_rdf.
      while(it_p != it_pe && it_r != it_re){
        // if (*it_p < it_r->first) {
        if (*it_p <= it_r->first) {
            ++it_p;
        } else if (*it_p >= it_r->second) {
            ++it_r;
        } else {
            // *it_p is within the range [it_r->first, it_r->second)
            new_current_level_split_keys.push_back(*it_p);
            ++it_p;
        }
      }
      split_keys[level] = new_current_level_split_keys;
    }




    vector<Pair> getLevelRanges(int outlevel){
      // init();
      // std::lock_guard<std::mutex> guard(update_mutex);
      if((uint)outlevel >= rd_filter.size()){return {};}
      return rd_filter[outlevel];
    }
    void setLevelRanges(vector<Pair> level_ranges_in, int outlevel){
      // init();
      // std::lock_guard<std::mutex> guard(update_mutex);
      while(rd_filter.size() <= (uint)outlevel){
        rd_filter.push_back(vector<Pair>());
      }
      
      if(level_ranges_in != rd_filter[outlevel]){
        std::cout << "^^^ outlevel: " << outlevel 
                  << " level_ranges_in.size(): " << level_ranges_in.size() 
                  << " rd_filter[outlevel].size() " << rd_filter[outlevel].size() << std::endl;
        std::cout << "^^^ level_ranges_in: " << std::endl;
        for(auto it = level_ranges_in.begin(); it != level_ranges_in.end(); it++){
          std::cout << "(" << it->first << " " << it->second << ") ";
        }
        std::cout << std::endl;
        std::cout << "^^^ rd_filter[outlevel]: " << std::endl;
        for(auto it = rd_filter[outlevel].begin(); it != rd_filter[outlevel].end(); it++){
          std::cout << "(" << it->first << " " << it->second << ") ";
        }
        std::cout << std::endl;
      }

      
      rd_filter[outlevel] = level_ranges_in;

    }


    int getNumberOfTotalLevels(){
      int num = 0;
      int len = rd_filter.size();
      for(int i = 1; i < len; i++){
        if(rd_filter[i].size() > 0){
          num = i+1;
        }
      }
      return num;
    }

    int getNumberOfTotalRanges(){
      int num = 0;
      for(auto it = rd_filter.begin(); it != rd_filter.end(); it++){
        num += it->size();
      }
      return num;
    }
    
    int getNumberOfTotalSplitKeys(){
      int num = 0;
      for(auto it = split_keys.begin(); it != split_keys.end(); it++){
        num += it->size();
      }
      return num;
    }

    
    int getNumberOfTotalMemoryUsage(){
      int num = 0;
      
      if constexpr (std::is_same<KeyType, std::string>::value) {
          // std::cout << "KeyType is std::string\n";
          for(auto it = rd_filter.begin(); it != rd_filter.end(); it++){
            for(auto it2: *it){
              num += (it2->first).size() + (it2->second).size(); // size of a range: sizeof(Pair)
            }
          }
          for(auto it = split_keys.begin(); it != split_keys.end(); it++){
            for(auto it2: *it){
              num += it2->size(); // size of a key: sizeof(KeyType)
            }
          }
      } else {
          // std::cout << "KeyType is not std::string\n";
          for(auto it = rd_filter.begin(); it != rd_filter.end(); it++){
            num += sizeof(it->first) + sizeof(it->second); // size of a range: sizeof(Pair)
          }
          for(auto it = split_keys.begin(); it != split_keys.end(); it++){
            num += sizeof(*it); // size of a key: sizeof(KeyType)
          }
      }
      
      return num;
    }

    void print(){
      // init();
      // std::lock_guard<std::mutex> guard(update_mutex);

      std::cout <<  std::setfill('-') << std::setw(60) << " START: Print  RDF " << std::setfill('-') << "" << std::endl;
      for(uint l = 0; l < rd_filter.size(); l++){
        std::cout << "Level: " << l << std::endl;
        auto& rdList = rd_filter[l];
        for(auto it = rdList.begin(); it != rdList.end(); it++){
          std::cout << "(" << it->first << " " << it->second << ") ";
        }
        std::cout << std::endl;
      }
      std::cout <<  std::setfill('-') << std::setw(60) << " END: Print  RDF " << std::setfill('-') << "" << std::endl;
    }

    // void clearMayBeDeletedCount(){
    //   may_be_deleted_count = 0;
    // }

    // int getMayBeDeletedCount(){
    //   return may_be_deleted_count;
    // }

    // void incMayBeDeletedCount(){
    //   may_be_deleted_count++;
    // }

    void clearFilterFalsePositiveRate(){
      clearKeySearchCount();
      clearKeyMayDeletedCount();
    }
    double getFilterFalsePositiveRate(){
        uint32_t total_key_search_count = getKeySearchCount();
        if(total_key_search_count == 0){return -1;}
        return 1.0*getKeyMayDeletedCount()/total_key_search_count;
    }
    // use clearFilterFalsePositiveRate for public called
    void clearKeySearchCount(){
        key_search_count = 0;
    }
    void incKeySearchCount(){
        if(key_search_count == std::numeric_limits<decltype(key_search_count)>::max()){
            std::cerr << "Error: overflow of key_search_count " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        }
        key_search_count += 1;
    }
    uint32_t getKeySearchCount(){
        return key_search_count;
    }
    // use clearFilterFalsePositiveRate for public called
    void clearKeyMayDeletedCount(){
        key_may_deleted_count = 0;
    }
    uint32_t getKeyMayDeletedCount(){
        return key_may_deleted_count;
    }
    void setFlagKeyMayDeleted(){
        if(key_may_deleted_count == std::numeric_limits<decltype(key_may_deleted_count)>::max()){
            std::cerr << "Error: overflow of key_may_deleted_count " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        }
        key_may_deleted_count += 1;
        flag_key_may_deleted = true;
    }
    bool getFlagKeyMayDeleted(){
        checkProperUsageOfFlagKeyMayDeleted();
        return flag_key_may_deleted;
    }

    void clearFlagKeyMayDeleted(){
        flag_key_may_deleted = false;
        key_search_count_kmd = 0;
    }
    void incKeySearchCountKMD(){
        key_search_count_kmd += 1;
    }
    void checkProperUsageOfFlagKeyMayDeleted(){
        if(key_search_count_kmd > 1){
            std::cout << "Error: key_may_deleted_count shall be 0 or 1."
                    << " Make sure clearFlagKeyMayDeleted is called in the beginning of all the series of isEntryAlive functions"
                    << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
            std::cerr << "Error: key_may_deleted_count shall be 0 or 1."
                    << " Make sure clearFlagKeyMayDeleted is called in the beginning of all the series of isEntryAlive functions"
                    << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
            exit(-1);
        }
    }


    bool isEntryAlive(uint level, KeyType key){
      // init();
      // std::lock_guard<std::mutex> guard(update_mutex);
      clearFlagKeyMayDeleted();      
      incKeySearchCountKMD();
      incKeySearchCount();

      assert(rd_filter.size() > level);

      if(level >= rd_filter.size()){
        return true;
      }
      
      // range tombstones
      auto& rdList = rd_filter[level];
      if(rdList.size() == 0){return true;}

      // split keys
      auto &splitKeyList = split_keys[level];
      auto it_p = lower_bound(splitKeyList.begin(), splitKeyList.end(), key);
      // if key exact the same return true
      if(it_p != splitKeyList.end() && *it_p == key){return true;}


      // range tombstones
      auto it = upper_bound(rdList.begin(), rdList.end(), Pair(key, key), [](const Pair& a, const Pair& b){return a.first < b.first;});
      if(it != rdList.begin()){it--;}
      //[a,b], [c,d]
      // if(key >= it->first && key <= it->second){return false;}

      //[a,b), [c,d)
      // if(key >= it->first && key < it->second){return false;}
// std::cout <<  " key = " << key << " range = " << it->first << "," << it->second << " " << __FILE__ << ":" << __LINE__ << std::endl;
      // when keys falling on the range boundary --> the result is uncertain for Pair keys
      if(key > it->first && key < it->second){return false;}
      if(key == it->first || key == it->second){
        setFlagKeyMayDeleted();
      }
      return true;
    }

    void deleteLastLevelIfEqualsBottomLevel(uint bottom_level){
      // init();
      // std::lock_guard<std::mutex> guard(update_mutex);

      if (rd_filter.size()-1 == bottom_level)
      {
        rd_filter[bottom_level].clear();
        split_keys[bottom_level].clear();
      }
    }


    //TODO:
    void splitRangesOnLevel(uint level, std::vector<KeyType> keys){
      if(rd_filter.size() <= level){
        std::cerr << "Error: splitRangesOnLevel: level: " << level << " is not present in PLRDF" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        exit(1);
      }

      // auto &rdList = rd_filter[level];
      //   std::vector<Pair> rdList_new;
      auto &splitKeyList = split_keys[level];
      std::vector<KeyType> splitKeysList_new;
      
      
      auto &rangeList = rd_filter[level];
      // std::vector<KeyType> splitKeyList;
      // std::vector<KeyType> keys;
      // std::vector<KeyType> splitKeysList_new;
      auto it1 = splitKeyList.begin();
      auto it1e = splitKeyList.end();
      auto it2 = keys.begin();
      auto it2e = keys.end(); 
      // while(it1 != it1e || it2 != it2e){
      //   if(it1 == it1e){

      //   }
      // }
      auto it_r = rangeList.begin();
      auto it_re = rangeList.end();
      while (it1 != it1e || it2 != it2e) {
          KeyType candidate;
      
          if (it1 == it1e) {
              candidate = *it2;
              ++it2;
          } else if (it2 == it2e) {
              candidate = *it1;
              ++it1;
          } else if (*it1 < *it2) {
              candidate = *it1;
              ++it1;
          } else if (*it2 < *it1) {
              candidate = *it2;
              ++it2;
          } else {
              // Equal keys — advance both
              candidate = *it1;
              ++it1;
              ++it2;
          }

          // only storing split keys covered by some ranges
          {
            while(it_r != it_re && it_r->second <= candidate){
              it_r++;
            }
            if(it_r == it_re){break;}
            if(candidate <= it_r->first){continue;}
          }

          // Avoid inserting duplicate consecutive values
          if (splitKeysList_new.empty() || (candidate != splitKeysList_new.back())) {
              splitKeysList_new.push_back(candidate);
          }
      }
      split_keys[level] = splitKeysList_new;
    }


    void logCurrentTotalNumbersOfRanges(){
      numbers_of_ranges_in_RDF_log.push_back(getNumberOfTotalRanges());
    }

    std::vector<int> getNumbersOfRangesInRDFLog(){
      if(numbers_of_ranges_in_RDF_log.size() == 0){
        return {};
      }else{
        return numbers_of_ranges_in_RDF_log;
      }
    }
    

    void logCurrentTotalNumbersOfSplitKeys(){
      numbers_of_split_keys_in_RDF_log.push_back(getNumberOfTotalSplitKeys());
    }

    std::vector<int> getNumbersOfSplitKeysInRDFLog(){
      if(numbers_of_split_keys_in_RDF_log.size() == 0){
        return {};
      }else{
        return numbers_of_split_keys_in_RDF_log;
      }
    }
    

    void logCurrentTotalMemoryUsage(){
      memory_usage_in_RDF_log.push_back(getNumberOfTotalMemoryUsage());
    }
    std::vector<int> getMemoryUsageInRDFLog(){
      return memory_usage_in_RDF_log;
    }
};



class PLRDF{
  private:
    std::unordered_map<uint64_t, std::vector<pll>> rd_filter_level0; //for level 0, (file_num, RD_list), FileMetaData* -> fd .GetNumber();

    std::vector<std::vector<pll>> rd_filter; //for level > 0, list of range delete (start, end), all entries are non-overlapping
    std::vector<int> numbers_of_ranges_in_RDF_log; //for level > 0, number of ranges in RDF
    std::vector<int> memory_usage_in_RDF_log;


    void addRangeDelete_internal(uint level, std::vector<pll> &range_delete_list_in){
      // init();
      // update_mutex.lock();
      // std::lock_guard<std::mutex> guard(update_mutex);

      assert(rd_filter.size() >= level);
      while (rd_filter.size() <= level)
      {
        rd_filter.push_back(std::vector<pll>());
      }

      addRangeDelete(rd_filter[level], range_delete_list_in);

      // update_mutex.unlock();
    }


    // ToDO: string cannot be merged on the boundary overlap

    std::vector<pll> sortAndMerge(std::vector<pll> &range_delete_list_in){
      if(range_delete_list_in.size() == 0){
        return {};
      }

      std::sort(range_delete_list_in.begin(), range_delete_list_in.end(), [](pll a, pll b){
        return a.first < b.first;
      });

      std::vector<pll> range_delete_list;
      range_delete_list.reserve(range_delete_list_in.size());
      auto itA = range_delete_list_in.begin();
      auto iteA = range_delete_list_in.end();
      pll tmp_range = *itA;
      for(;itA != iteA; itA++){
        if(tmp_range.second >= itA->first){
          tmp_range.second = std::max(tmp_range.second, itA->second);
        }else{
          range_delete_list.push_back(tmp_range);
          tmp_range = *itA;
        }
      }

      range_delete_list.push_back(tmp_range);
      return range_delete_list;
    }



    void addRangeDelete(std::vector<pll> &range_delete_list, std::vector<pll> &range_delete_list_in){
    // init();
    // std::lock_guard<std::mutex> guard(init_mutex);

      auto& rdList = range_delete_list;
      auto& rdList_in = range_delete_list_in;

      if(rdList_in.size() == 0){return;}

      for(uint i = 1; i < rdList_in.size(); i++){
        if(rdList_in[i-1].first > rdList_in[i].first){
          std::cerr << "Error: rdList_in to be flushed has not been sorted in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl;
          exit(1);
        }
      }

      if(rdList.size() == 0){
        //do the merging first before adding to rdList
        // std::vector<pll> rdList_new;
        rdList.reserve(rdList_in.size());
        auto itA = rdList_in.begin();
        auto iteA = rdList_in.end();
        pll tmp_range = *itA;
        for(;itA != iteA; itA++){
          if(tmp_range.second >= itA->first){
            tmp_range.second = std::max(tmp_range.second, itA->second);
          }else{
            rdList.push_back(tmp_range);
            tmp_range = *itA;
          }
        }

        rdList.push_back(tmp_range);
        
        return;
      }




      std::vector<pll> rdList_new;
      rdList_new.reserve(rdList.size() + rdList_in.size());



      auto itA = rdList.begin();
      auto iteA = rdList.end();
      auto itB = rdList_in.begin();
      auto iteB = rdList_in.end();


      pll tmp_range;
      if(itA->first < itB->first){
        tmp_range = *itA;
      }else{
        tmp_range = *itB;
      }

      while(itA != iteA || itB != iteB){
        if(itA != iteA && itA->first <= tmp_range.second){
          tmp_range.second = std::max(tmp_range.second, itA->second);
          itA++;
          continue;
        }
        if(itB != iteB && itB->first <= tmp_range.second){
          tmp_range.second = std::max(tmp_range.second, itB->second);
          itB++;
          continue;
        }
        rdList_new.push_back(tmp_range);

        if(itA == iteA){
          tmp_range = *itB;
          itB++;
          continue;
        }
        if(itB == iteB){
          tmp_range = *itA;
          itA++;
          continue;
        }

        if(itA->first <= itB->first){
          tmp_range = *itA;
          itA++;
        }else{
          tmp_range = *itB;
          itB++;
        }
      }
      rdList_new.push_back(tmp_range);



      rdList.clear();
      rdList.reserve(rdList_new.size());
      for(auto &p : rdList_new){
        rdList.push_back(p);
      }
    }


    // // void addRangeDelete(std::vector<pll> &range_delete_list, long long start, long long end){
    // void addRangeDelete(std::vector<pll> &range_delete_list, pll rd){
    //     // init();
    // // std::lock_guard<std::mutex> guard(init_mutex);

    // auto start = rd.first;
    // auto end = rd.second;

    // auto& rdList = range_delete_list;
    // #ifdef DEBUG
    //   cout << "Adding range delete: " << start << " " << end << endl;
    // #endif
    //   std::vector<pll> rdList_new;
    //   rdList_new.reserve(rdList.size()+1);

    //   auto minK = start;
    //   auto maxK = end;
      
    //   auto it = rdList.begin();
    //   while ( it != rdList.end() ){
    //       // [a,b], [c,d]
          
    //       //[a, b), [c,d)
    //       if (it->second < start){ rdList_new.push_back(*it); it++; continue;}
    //       if (it->first > end){ break;}

    //       minK = std::min(minK, it->first);
    //       maxK = std::max(maxK, it->second);
    //       it++;
    //   }
    //   rdList_new.push_back(pll({minK, maxK}));
    //   while(it != rdList.end()){
    //     rdList_new.push_back(*it);
    //     it++;
    //   }

    //   rdList.clear();
    //   rdList.reserve(rdList_new.size());
    //   for(auto &p : rdList_new){
    //     rdList.push_back(p);
    //   }
    // }

    void print_internal(){
      // init();
      // std::lock_guard<std::mutex> guard(update_mutex);

      std::cout <<  std::setfill('-') << std::setw(60) << " START: Print PL RDF " << std::setfill('-') << "" << std::endl;
      for(uint l = 0; l < rd_filter.size(); l++){
        std::cout << "Level: " << l << std::endl;
        auto& rdList = rd_filter[l];
        for(auto it = rdList.begin(); it != rdList.end(); it++){
          std::cout << "(" << it->first << " " << it->second << ") ";
        }
        std::cout << std::endl;
      }
      std::cout <<  std::setfill('-') << std::setw(60) << " END: Print PL RDF " << std::setfill('-') << "" << std::endl;
    }



    void adjustRangeDeletesForLevel0Input(uint olevel, std::vector<uint64_t> file_numbers){

      std::vector<pll> to_be_added_in_next_level_rdf;


      for(uint64_t &file_num: file_numbers){

        {
          // // -- rd_filter_level0 --
          // rd_filter_level0_mutex.lock();

          auto it = rd_filter_level0.find(file_num);
          if(it == rd_filter_level0.end()){
            assert(it != rd_filter_level0.end());
            std::cerr << "File number not found in level 0 " << "File number " << file_num << " " << __FILE__ << ":" << __LINE__ << std::endl; 
            std::cerr << "Remindation: Do the manually flush after all the insert workload are done. So no entries lie inside memtable anymore. In case those entries will go through the track of bulk buiding from WAL and no going through the path of flushJob." << "File number " << file_num << " " << __FILE__ << ":" << __LINE__ << std::endl; 
            exit(1);
            // continue;
          }
          auto val = it->second;
          to_be_added_in_next_level_rdf.insert(to_be_added_in_next_level_rdf.end(), val.begin(), val.end());
          rd_filter_level0.erase(it);
          
        //   rd_filter_level0_mutex.unlock();
        //   // -- rd_filter_level0 -- 
        }

      }

      std::sort(to_be_added_in_next_level_rdf.begin(), to_be_added_in_next_level_rdf.end(), [](const pll a, const pll b)
              { return a.first < b.first; });



      // // -- updating rd_filter_level0 --
      // std::lock_guard<std::mutex> guard(rd_filter_level0_mutex);

      addRangeDelete_internal(olevel, to_be_added_in_next_level_rdf);
    }



    // This would be used for trivial compaction and normal compaction
    // input_level, output_level, file_boundaries
    void adjustRangeDeletes(uint clevel, uint olevel, std::vector<pll> one_level_compaction_file_boundaries){
      // // init();
      // std::lock_guard<std::mutex> guard(update_mutex);

      
      std::vector<pll> new_current_level_rdf;
      std::vector<pll> to_be_added_in_next_level_rdf;

      if (rd_filter.size() <= clevel)
      {
        return;
      }
      
      auto old_current_level_rdf = rd_filter[clevel];

      // FIXME: (Shubham) This might not be required
      if (one_level_compaction_file_boundaries.size() == 0)
      {
        return;
      }

      auto it = old_current_level_rdf.begin();
      auto itf = one_level_compaction_file_boundaries.begin();

      while (it != old_current_level_rdf.end())
      {
        pll val = *it;
        /*
        *    |--|
        *         -----
        *         |   |
        *         -----
        */
      //  if (itf == one_level_compaction_file_boundaries.end() || (val.second <= file_boundry.first))
       if (itf == one_level_compaction_file_boundaries.end() || (val.second <= itf->first))
       {
          // new_current_level_rdf.push_back(val);
          if(val.first != val.second){
            new_current_level_rdf.push_back(val);
          }
          it++;
          continue;
        }
        
        auto file_boundries = *itf;
        pll file_boundry = std::make_pair(file_boundries.first, file_boundries.second);

        /*
        *             |--|
        *     ------
        *     |    |
        *     ------
        */
        if (val.first > file_boundry.second)
        {
          itf++;
        }
        /*
        *    |------||||
        *         ------
        *         |    |
        *         ------
        */
        else if (val.first < file_boundry.first && val.second > file_boundry.first && val.second <= file_boundry.second)
        {
          // new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
          // to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first, val.second));
          if(val.first != file_boundry.first){
            new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
          }
          if(file_boundry.first != val.second){
            to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first, val.second));
          }
          it++;
        }
        /*
        *    |||--|||
        *    --------
        *    |      |
        *    --------
        */
        else if (val.first >= file_boundry.first && val.second <= file_boundry.second)
        {
          // to_be_added_in_next_level_rdf.push_back(val);
          if(val.first != val.second){
            to_be_added_in_next_level_rdf.push_back(val);
          }
          it++;
        }
        /*
        *     ||||-------|
        *     --------
        *     |      |
        *     --------
        */
        else if (val.first >= file_boundry.first && val.first <= file_boundry.second && val.second > file_boundry.second)
        {
          to_be_added_in_next_level_rdf.push_back(std::make_pair(val.first, file_boundry.second + 1));
          (*it).first = file_boundry.second + 1;
          if((*it).first >= (*it).second){ it++; } // <------------------------
          itf++;
        }
        /*
        *  |------------|
        *     --------
        *     |      |
        *     --------
        */
        else if (val.first < file_boundry.first && val.second > file_boundry.second)
        {
          // new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
          if(val.first != file_boundry.first){
            new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
          }
          to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first, file_boundry.second + 1));
          (*it).first = file_boundry.second + 1;
          if((*it).first >= (*it).second){ it++; } // <------------------------
          itf++;
        }else{
          std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
          std::cerr << "val.first: " << val.first << " val.second: " << val.second << " file_boundry.first: " << file_boundry.first << " file_boundry.second: " << file_boundry.second << std::endl;
          assert(false);
          exit(1);
        }
      }

      rd_filter[clevel] = new_current_level_rdf;
      std::sort(to_be_added_in_next_level_rdf.begin(), to_be_added_in_next_level_rdf.end(), [](const pll a, const pll b)
              { return a.first < b.first; });

      addRangeDelete_internal(olevel, to_be_added_in_next_level_rdf);
    }




  public:





    void insertRangeDeleteToLevel0(uint64_t file_num, std::vector<pll> &range_delete_list_in, std::vector<uint64_t> exist_level0_file_nums){
      std::vector<pll> sorted_merged_rdlist = sortAndMerge(range_delete_list_in);
      // init();

      // -- updating rd_filter_level0 --
      // std::lock_guard<std::mutex> guard(rd_filter_level0_mutex);
      // rd_filter_level0_mutex.lock();

      // if(rd_filter_level0.count(file_num) > 0){
      if(std::binary_search(exist_level0_file_nums.begin(), exist_level0_file_nums.end(), file_num) == true){
        std::cerr << "Error: file_num already exists in rd_filter_level0 " << "file_num = " << file_num << "\t" << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
        exit(1);
      }
      rd_filter_level0[file_num] = sorted_merged_rdlist;


      // rd_filter_level0_mutex.unlock();
      // -- updating rd_filter_level0 --


    }

    void printLevel0(){
      // init();
      // std::lock_guard<std::mutex> guard(rd_filter_level0_mutex);

      std::cout << "rd_filter_level0" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<  std::endl << std::endl;
      for(auto it = rd_filter_level0.begin(); it != rd_filter_level0.end(); it++){
        std::cout << "rd_filter_level0 file number: " << it->first << " number of RD: " << it->second.size() << std::endl;
        //print all ranges
        for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++){
          std::cout <<  "rd_filter_level0" << " " << it2->first << " " << it2->second << " ";
        }
        std::cout << std::endl;
      }
      std::cout << std::endl << std::endl;
    }



    void addRangeDelete(uint level, std::vector<pll> &range_delete_list_in){
      // init();
      // // update_mutex.lock();
      // std::lock_guard<std::mutex> guard(update_mutex);

      while (rd_filter.size() <= level)
      {
        rd_filter.push_back(std::vector<pll>());
      }

      addRangeDelete(rd_filter[level], range_delete_list_in);

      // update_mutex.unlock();
    }







    /*
    *Do insertion, even if the vector is empty, because we need to set condition_variable of mutex (semaphore) for compaction
    */
    // input_level, output_level, file_boundries, file_numbers
    void shiftRDFToOutputLevel(std::vector<std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>>  *file_meta_data_vectors){
        // init();
      // std::lock_guard<std::mutex> guard(update_mutex);

      // // // FIXME: FOR TESTING (next 2 lines)


      for (auto file_meta_data : *file_meta_data_vectors)
      {
        int clevel = std::get<0>(file_meta_data);
        if(clevel == 0){
          adjustRangeDeletesForLevel0Input(std::get<1>(file_meta_data), std::get<3>(file_meta_data));
        }else{
          // file ranges
          std::vector<std::pair<long long, long long>> one_level_file_boundries;
          auto meta_data = std::get<2>(file_meta_data);

          for (auto meta : meta_data)
          {
            one_level_file_boundries.push_back(std::make_pair(meta.first, meta.second));
          }

          adjustRangeDeletes(std::get<0>(file_meta_data), std::get<1>(file_meta_data), one_level_file_boundries);
        }
      }

      // // // FIXME: FOR TESTING (next 2 lines)
      // // std::cout << "After Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      // // // update_mutex.lock();
      // print_internal();
      // // update_mutex.unlock();
    }




    // this is only used for direct compaction //
    // input_level, file_boundries, file_numbers
    void deleteRDFAssociatedWithFilesAtCurrentLevel(std::tuple<int, std::vector<pll>, std::vector<uint64_t>> *file_meta_data){
      // init();
      // std::lock_guard<std::mutex> guard(update_mutex);

      // std::cout << "Before Deletion Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      // print_internal();

      std::vector<std::pair<long long, long long>> one_level_file_boundries;
      auto level = std::get<0>(*file_meta_data);

      if(level == 0){
        auto it = rd_filter_level0.find(level);
        if(it == rd_filter_level0.end()){
          assert(it != rd_filter_level0.end());
          std::cerr << "Error: file_num does not exist in rd_filter_level0" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl;
          exit(1);
        }
        rd_filter_level0.erase(it);
        return;
      }


      if (rd_filter.size() <= (uint)level)
      {
        return;
      }

      auto meta_data = std::get<1>(*file_meta_data);

      for (auto meta : meta_data)
      {
          one_level_file_boundries.push_back(std::make_pair(meta.first, meta.second));
      }

      std::vector<pll> new_current_level_rdf;
      auto old_current_level_rdf = rd_filter[level];
      auto it = old_current_level_rdf.begin();
      auto itf = one_level_file_boundries.begin();

      while (it != old_current_level_rdf.end())
      {
        pll val = *it;
        auto file_boundries = *itf;
        pll file_boundry = std::make_pair(file_boundries.first, file_boundries.second);

        /*
        *    |--|
        *         -----
        *         |   |
        *         -----
        */
        if (itf == one_level_file_boundries.end() || (val.second <= file_boundry.first))
        {
          new_current_level_rdf.push_back(val);
          it++;
        }
        /*
        *             |--|
        *     ------
        *     |    |
        *     ------
        */
        else if (val.first > file_boundry.second)
        {
          itf++;
        }
        /*
        *    |------||||
        *         ------
        *         |    |
        *         ------
        */
        else if (val.first < file_boundry.first && val.second > file_boundry.first && val.second <= file_boundry.second)
        {
          new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
          it++;
        }
        /*
        *    |||--|||
        *    --------
        *    |      |
        *    --------
        */
        else if (val.first >= file_boundry.first && val.second <= file_boundry.second)
        {
          it++;
        }
        /*
        *     ||||-------|
        *     --------
        *     |      |
        *     --------
        */
        else if (val.first >= file_boundry.first && val.first <= file_boundry.second && val.second > file_boundry.second)
        {
          (*it).first = file_boundry.second + 1;
          if((*it).first >= (*it).second){ it++; } // <------------------------
          itf++;
        }
        /*
        *  |------------|
        *     --------
        *     |      |
        *     --------
        */
        else if (val.first < file_boundry.first && val.second > file_boundry.second)
        {
          new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
          (*it).first = file_boundry.second + 1;
          if((*it).first >= (*it).second){ it++; } // <------------------------
          itf++;
        }else{
          std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
          std::cerr << "val.first: " << val.first << " val.second: " << val.second << " file_boundry.first: " << file_boundry.first << " file_boundry.second: " << file_boundry.second << std::endl;
          assert(false);
          exit(1);
        }
      }

      rd_filter[level] = new_current_level_rdf;

      // std::cout << "After Deletion Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      // print_internal();
    }




    vector<pll> getLevelRanges(int outlevel){
      // init();
      // std::lock_guard<std::mutex> guard(update_mutex);
      if((uint)outlevel >= rd_filter.size()){return {};}
      return rd_filter[outlevel];
    }
    void setLevelRanges(vector<pll> level_ranges_in, int outlevel){
      // init();
      // std::lock_guard<std::mutex> guard(update_mutex);
      while(rd_filter.size() <= (uint)outlevel){
        rd_filter.push_back(vector<pll>());
      }
      
      if(level_ranges_in != rd_filter[outlevel]){
        std::cout << "^^^ outlevel: " << outlevel 
                  << " level_ranges_in.size(): " << level_ranges_in.size() 
                  << " rd_filter[outlevel].size() " << rd_filter[outlevel].size() << std::endl;
        std::cout << "^^^ level_ranges_in: " << std::endl;
        for(auto it = level_ranges_in.begin(); it != level_ranges_in.end(); it++){
          std::cout << "(" << it->first << " " << it->second << ") ";
        }
        std::cout << std::endl;
        std::cout << "^^^ rd_filter[outlevel]: " << std::endl;
        for(auto it = rd_filter[outlevel].begin(); it != rd_filter[outlevel].end(); it++){
          std::cout << "(" << it->first << " " << it->second << ") ";
        }
        std::cout << std::endl;
      }

      
      rd_filter[outlevel] = level_ranges_in;

    }


    int getNumberOfTotalLevels(){
      int num = 0;
      int len = rd_filter.size();
      for(int i = 1; i < len; i++){
        if(rd_filter[i].size() > 0){
          num = i+1;
        }
      }
      return num;
    }

    int getNumberOfTotalRanges(){
      int num = 0;
      for(auto it = rd_filter.begin(); it != rd_filter.end(); it++){
        num += it->size();
      }
      return num;
    }

    
    int getNumberOfTotalMemoryUsage(){
      int num = 0;
      for(auto it = rd_filter.begin(); it != rd_filter.end(); it++){
        num += it->size();
      }
      return num*sizeof(pll); // size of a ranges is the size of pair<long long, long long> = 16 bytes
    }

    void print(){
      // init();
      // std::lock_guard<std::mutex> guard(update_mutex);

      std::cout <<  std::setfill('-') << std::setw(60) << " START: Print  RDF " << std::setfill('-') << "" << std::endl;
      for(uint l = 0; l < rd_filter.size(); l++){
        std::cout << "Level: " << l << std::endl;
        auto& rdList = rd_filter[l];
        for(auto it = rdList.begin(); it != rdList.end(); it++){
          std::cout << "(" << it->first << " " << it->second << ") ";
        }
        std::cout << std::endl;
      }
      std::cout <<  std::setfill('-') << std::setw(60) << " END: Print  RDF " << std::setfill('-') << "" << std::endl;
    }


    bool isEntryAlive(uint level, long long key){
      // init();
      // std::lock_guard<std::mutex> guard(update_mutex);

      assert(rd_filter.size() > level);

      if(level >= rd_filter.size()){
        return true;
      }

      auto& rdList = rd_filter[level];
      if(rdList.size() == 0){return true;}

      auto it = upper_bound(rdList.begin(), rdList.end(), pll(key, key), [](const pll& a, const pll& b){return a.first < b.first;});
      if(it != rdList.begin()){it--;}
      //[a,b], [c,d]
      // if(key >= it->first && key <= it->second){return false;}

      //[a,b), [c,d)
      if(key >= it->first && key < it->second){return false;}
      return true;
    }

    void deleteLastLevelIfEqualsBottomLevel(uint bottom_level){
      // init();
      // std::lock_guard<std::mutex> guard(update_mutex);

      if (rd_filter.size()-1 == bottom_level)
      {
        rd_filter[bottom_level].clear();
      }
    }



    void splitRangesOnLevel(uint level, std::vector<long long> keys){
      if(rd_filter.size() <= level){
        std::cerr << "Error: splitRangesOnLevel: level: " << level << " is not present in PLRDF" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        exit(1);
      }

      auto rdList = rd_filter[level];
      std::vector<pll> rdList_new;
      
      int idx = 0;
      int len = rdList.size();

      for(auto &key_in: keys){
    // std::cout << "key_in: " << key_in << std::endl;
          while(idx < len && rdList[idx].second <= key_in){
            rdList_new.push_back(rdList[idx]);
            idx += 1;
          }
          if(idx < len && rdList[idx].first > key_in){
            continue;
          }


          if(idx < len && rdList[idx].first == key_in){
            if(key_in + 1 >= rdList[idx].second){
              idx += 1;
            }else{
              rdList[idx].first = key_in + 1;
            }
            continue;
          }

          if(idx < len && rdList[idx].first < key_in && rdList[idx].second  >  key_in){
            auto tmp = rdList[idx];
            tmp.second = key_in;
            rdList_new.push_back(tmp);
            if(key_in + 1 >= rdList[idx].second){
              idx += 1;
            }else{
              rdList[idx].first = key_in + 1;
            }
            continue;
          }
      }

      while(idx < len){
        rdList_new.push_back(rdList[idx]);
        idx += 1;
      }
      
      // std::cout << std::endl;

      rd_filter[level] = rdList_new;
    }


    void logCurrentTotalNumbersOfRanges(){
      numbers_of_ranges_in_RDF_log.push_back(getNumberOfTotalRanges());
    }

    std::vector<int> getNumbersOfRangesInRDFLog(){
      if(numbers_of_ranges_in_RDF_log.size() == 0){
        return {};
      }else{
        return numbers_of_ranges_in_RDF_log;
      }
    }
    
    void logCurrentTotalMemoryUsage(){
      memory_usage_in_RDF_log.push_back(getNumberOfTotalMemoryUsage());
    }
    std::vector<int> getMemoryUsageInRDFLog(){
      return memory_usage_in_RDF_log;
    }
};






//Self Added --- END PL-RDF ---


#endif /* SYS_RDFILTER_H_ */



