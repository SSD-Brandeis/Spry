/*
 *  Created on: April 23, 2023
 *  Author: yucheng
 */

/*Notice: anytime only one lock is locked in this class*/


#ifndef SYS_RDFILTER_H_
#define SYS_RDFILTER_H_


namespace rdfilter {
  // class PerlevelRangeDeleteFilterByVector {
  class PLRDF;
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
#include <utility>


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
using namespace ROCKSDB_NAMESPACE;
// using namespace workload_exec;

namespace rdfilter {
  using pll = std::pair<long long, long long>;
  
  // class PLRDF {
  // class PerlevelRangeDeleteFilterByVector {
  class PLRDF {
    // using PLRDF = PerlevelRangeDeleteFilterByVector;

    private:
      static const int KEY_SIZE = 12;
      std::mutex update_mutex; // locks for level > 0
      // std::mutex writeback_mutex; // locks for level > 0
      static std::mutex init_mutex; // locks for initialization of the function

      //semaphores below are implemented with condition variables
      std::unordered_map<uint64_t, std::vector<pll>> rd_filter_level0; //FileMetaData* -> fd .GetNumber();
      // std::unordered_map<uint64_t, std::binary_semaphore> semaphores_level0; // locks for level 0 rd_filter
      // std::unordered_map<uint64_t, std::pthread_cond_t> semaphores_level0; // locks for level 0 rd_filter
      // using pmcv = std::pair<std::mutex, std::condition_variable>;

      std::unordered_map<uint64_t, std::mutex> semaphores_m_level0; // locks for level 0 rd_filter
      std::unordered_map<uint64_t, std::condition_variable> semaphores_cv_level0; // locks for level 0 rd_filter
      //mutex is lvalue doesn't allow to be copied
      // semaphores_m_level0.emplace(std::piecewise_construct,
      //             std::forward_as_tuple(file_number),
      //             std::forward_as_tuple());
      std::mutex rd_filter_level0_mutex; // locks for rd_filter_level0 (level 0)
      std::mutex semaphores_level0_mutex; // locks for semaphores_level0 (level 0)
// pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;

      std::vector<std::vector<pll>> rd_filter; //list of range delete (start, end), all entries are non-overlapping
  
      // std::vector<pll> sortAndMerge(std::vector<pll> &range_delete_list_in);
      // oid print_internal();

      // void addRangeDelete(std::vector<pll> &range_delete_list, long long start, long long end);
      // void addRangeDelete(std::vector<pll> &range_delete_list, std::vector<pll> &range_delete_list_in);
      // void addRangeDelete_internal(uint level, std::vector<pll> &range_delete_list_in);
      // /*
      // * adjust range deletes as per the compaction
      // */
      // void adjustRangeDeletes(uint clevel, uint olevel, std::vector<std::pair<long long, long long>> one_level_compaction_file_boundaries);
      
      static PLRDF* plrdf_ptr;

      // -- no lock --


      void addRangeDelete_internal(uint level, std::vector<pll> &range_delete_list_in){
        // init();
        // update_mutex.lock();
        // std::lock_guard<std::mutex> guard(update_mutex);

        assert( rd_filter.size() >= level);
        while (rd_filter.size() <= level)
        {
          rd_filter.push_back(std::vector<pll>());
        }

        addRangeDelete(rd_filter[level], range_delete_list_in);

        // update_mutex.unlock();
      }



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

    std::cout << "rdList" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<  std::endl << std::endl;
    for(auto it = rdList.begin(); it != rdList.end(); it++){
      std::cout << "aaaa " << it->first << " " << it->second << std::endl;
    }
    std::cout << std::endl << std::endl;


    // std::cout << "rdList_in" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<  std::endl << std::endl;
    // for(auto it = rdList_in.begin(); it != rdList_in.end(); it++){
    //   std::cout << it->first << " " << it->second << std::endl;
    // }
    // std::cout << std::endl << std::endl;

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

          // std::cout << "after_direct insert to rdList: " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<  std::endl << std::endl;
          // for(auto it = rdList.begin(); it != rdList.end(); it++){
          //   std::cout << it->first << " " << it->second << std::endl;
          // }


          // //adding to rdList
          // rdList.reserve(rdList_new.size());
          // for(auto &p : rdList_new){
          //   rdList.push_back(p);
          // }
          
          return;
        }




        std::vector<pll> rdList_new;
        rdList_new.reserve(rdList.size() + rdList_in.size());



        auto itA = rdList.begin();
        auto iteA = rdList.end();
        auto itB = rdList_in.begin();
        auto iteB = rdList_in.end();

        // long long minK = start;
        // long long maxK = end;
        pll tmp_range;
        if(itA->first < itB->first){
          tmp_range = *itA;
        }else{
          tmp_range = *itB;
        }

        while(itA != iteA || itB != iteB){
    // std::cout << " itA = " << itA->first << " " << itA->second << std::endl;
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


    // std::cout << "rdList_new" << std::endl << std::endl;
    // for(auto it = rdList_new.begin(); it != rdList_new.end(); it++){
    //   std::cout << it->first << " " << it->second << std::endl;
    // }
    // std::cout << "rdList" << std::endl << std::endl;
    // for(auto it = rdList.begin(); it != rdList.end(); it++){
    //   std::cout << it->first << " " << it->second << std::endl;
    // }
    // std::cout << std::endl << std::endl;


        rdList.clear();
        rdList.reserve(rdList_new.size());
        for(auto &p : rdList_new){
          rdList.push_back(p);
        }
    }


      void addRangeDelete(std::vector<pll> &range_delete_list, long long start, long long end){
      // init();
      // std::lock_guard<std::mutex> guard(init_mutex);

      auto& rdList = range_delete_list;
    #ifdef DEBUG
        cout << "Adding range delete: " << start << " " << end << endl;
    #endif
        std::vector<pll> rdList_new;
        rdList_new.reserve(rdList.size()+1);

        long long minK = start;
        long long maxK = end;
        
        auto it = rdList.begin();
        while ( it != rdList.end() ){
            // [a,b], [c,d]
            // if (it->second < start-1){ rdList_new.push_back(*it); it++; continue;}
            // if (it->first > end+1){ break;}
            
            //[a, b), [c,d)
            if (it->second < start){ rdList_new.push_back(*it); it++; continue;}
            if (it->first > end){ break;}

            minK = std::min(minK, it->first);
            maxK = std::max(maxK, it->second);
            it++;
        }
        rdList_new.push_back(pll({minK, maxK}));
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
        }
        std::cout <<  std::setfill('-') << std::setw(60) << " END: Print PL RDF " << std::setfill('-') << "" << std::endl;

        // auto& rdList = range_delete_list;

        // for(auto it = rdList.begin(); it != rdList.end(); it++){
        //   std::cout << "(" << it->first << " " << it->second << ") ";
        // }
        // std::cout << std::endl;
      }


      // -- with lock --

      void adjustRangeDeletesForLevel0Input(uint olevel, std::vector<uint64_t> file_numbers){

        std::vector<pll> to_be_added_in_next_level_rdf;


        for(uint64_t &file_num: file_numbers){
          {
            // -- semaphores_level0 --
            semaphores_level0_mutex.lock();
            // if(semaphores_level0.count(file_num) == 0){
              // semaphores_level0[file_num] = std::binary_semaphore{0};
              // semaphores_level0[file_num] = make_pair(std::mutex(), std::condition_variable());
std::cout << "Compact From Level0 " << "semaphores_m_level0.count(file_num) : " << semaphores_m_level0.count(file_num) << " file_num =  " << file_num << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
std::cout << "Compact From Level0 " << "semaphores_cv_level0.count(file_num) : " << semaphores_cv_level0.count(file_num) << " file_num =  " << file_num << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
            if(semaphores_m_level0.count(file_num) == 0){
              semaphores_m_level0.emplace(std::piecewise_construct,
                      std::forward_as_tuple(file_num),
                      std::forward_as_tuple());
              semaphores_cv_level0.emplace(std::piecewise_construct,
                      std::forward_as_tuple(file_num),
                      std::forward_as_tuple());
            }
            semaphores_level0_mutex.unlock();
            // -- semaphores_level0 --

std::cout << "Compact From Level0 " << "file_num: " << file_num << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

            // -- wait on semaphores_level0 --
            // waiting for the signal from flushJob that the RD of the file 
            // is already added to the rd_filter_level0
            // semaphores_level0[file_num].acquire();  
            // std::unique_lock lk(semaphores_level0[file_num].first); 
            // semaphores_level0[file_num].second.wait(lk, [&] {return rd_filter_level0.count(file_num) > 0;}); // waken when condition becomes true
            std::unique_lock lk(semaphores_m_level0[file_num]); 
            semaphores_cv_level0[file_num].wait(lk, [&] {return rd_filter_level0.count(file_num) > 0;}); // waken when condition becomes true
            // semaphores_cv_level0[file_num].wait(lk);
            lk.unlock();
std::cout << "Compact From Level0 " << "UnLocked !!" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
            // -- wait on semaphores_level0 --
          }

          {
            // -- rd_filter_level0 --
            rd_filter_level0_mutex.lock();

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
            
            rd_filter_level0_mutex.unlock();
            // -- rd_filter_level0 -- 
          }

          {
            // -- semaphores_level0 --
            semaphores_level0_mutex.lock();

            // semaphores_level0.erase(file_num);  // also remove the semaphore of the current file_num
            semaphores_m_level0.erase(file_num);  // also remove the semaphore of the current file_num
            semaphores_cv_level0.erase(file_num);  // also remove the semaphore of the current file_num

            semaphores_level0_mutex.unlock();
            // -- semaphores_level0 --
          }
        }

        std::sort(to_be_added_in_next_level_rdf.begin(), to_be_added_in_next_level_rdf.end(), [](const pll a, const pll b)
                { return a.first < b.first; });



        // -- updating rd_filter_level0 --
        std::lock_guard<std::mutex> guard(rd_filter_level0_mutex);

        addRangeDelete_internal(olevel, to_be_added_in_next_level_rdf);
      }



      // This would be used for trivial compaction and normal compaction
      // input_level, output_level, file_boundaries
      void adjustRangeDeletes(uint clevel, uint olevel, std::vector<std::pair<long long, long long>> one_level_compaction_file_boundaries){
        // init();
        std::lock_guard<std::mutex> guard(update_mutex);

        
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
          auto file_boundries = *itf;
          pll file_boundry = std::make_pair(file_boundries.first, file_boundries.second);

          /*
          *    |--|
          *         -----
          *         |   |
          *         -----
          */
          if (itf == one_level_compaction_file_boundaries.end() || (val.second <= file_boundry.first))
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
            to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first, val.second));
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
            to_be_added_in_next_level_rdf.push_back(val);
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
            to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first, file_boundry.second + 1));
            (*it).first = file_boundry.second + 1;
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

      
      static void init(){
        std::lock_guard<std::mutex> guard(init_mutex);

        if(plrdf_ptr == NULL){
          plrdf_ptr = new PLRDF();
        }
      }

      static PLRDF* getRDFilter(){
        init();
        return plrdf_ptr;
      }






      // void insertRangeDeleteToLevel0(uint64_t file_num, std::vector<pll> &range_delete_list_in, std::vector<uint64_t> exist_level0_file_nums);
      // void printLevel0();

    
      // // std::vector<pll> getRangeDeleteList();
      // void addRangeDelete(uint level, long long start, long long end);
      // void addRangeDelete(uint level, std::vector<pll> &range_delete_list_in);
      // void shiftRDFToOutputLevel(std::vector<std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>> *file_meta_data_vectors);
      // void deleteLastLevelIfEqualsBottomLevel(uint bottom_level);
      // void deleteRDFAssociatedWithFilesAtCurrentLevel(std::tuple<int, std::vector<pll>, std::vector<uint64_t>> *file_meta_data);

      // void print();

      // bool isEntryAlive(uint level, long long key);



      void insertRangeDeleteToLevel0(uint64_t file_num, std::vector<pll> &range_delete_list_in, std::vector<uint64_t> exist_level0_file_nums){
        std::vector<pll> sorted_merged_rdlist = sortAndMerge(range_delete_list_in);
        init();

        // -- updating rd_filter_level0 --
        // std::lock_guard<std::mutex> guard(rd_filter_level0_mutex);
        rd_filter_level0_mutex.lock();

        // if(rd_filter_level0.count(file_num) > 0){
        if(std::binary_search(exist_level0_file_nums.begin(), exist_level0_file_nums.end(), file_num) == true){
          std::cerr << "Error: file_num already exists in rd_filter_level0 " << "file_num = " << file_num << "\t" << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
          exit(1);
        }
        rd_filter_level0[file_num] = sorted_merged_rdlist;

        std::cout << "rd_filter_Level0 " << "file_num: " << file_num << " number of RD: " << sorted_merged_rdlist.size() << std::endl;
      
        rd_filter_level0_mutex.unlock();
        // -- updating rd_filter_level0 --
      

        {
          // -- semaphores_level0 --
          semaphores_level0_mutex.lock();
          // if(semaphores_level0.count(file_num) == 0){
          //   semaphores_level0[file_num] = std::binary_semaphore{0};
          // }
          if(semaphores_m_level0.count(file_num) == 0){
            // semaphores_level0[file_num] = std::binary_semaphore{0};
            // semaphores_level0[file_num] = make_pair(std::mutex(), std::condition_variable());
            semaphores_m_level0.emplace(std::piecewise_construct,
                    std::forward_as_tuple(file_num),
                    std::forward_as_tuple());
            semaphores_cv_level0.emplace(std::piecewise_construct,
                    std::forward_as_tuple(file_num),
                    std::forward_as_tuple());
          }

std::cout << "Flush To Level0 " << "file_num: " << file_num << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
          // signaling compaction thread (which uses adjustRangeDeletesForLevel0Input) 
          // that RDs of the file_num has already been inserted
          // semaphores_level0[file_num].release();
          semaphores_cv_level0[file_num].notify_one();

          semaphores_level0_mutex.unlock();
          // -- semaphores_level0 --
        }
      }

      void printLevel0(){
        init();
        std::lock_guard<std::mutex> guard(rd_filter_level0_mutex);

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
        init();
        // update_mutex.lock();
        std::lock_guard<std::mutex> guard(update_mutex);

        assert( rd_filter.size() >= level);
        while (rd_filter.size() <= level)
        {
          rd_filter.push_back(std::vector<pll>());
        }

        addRangeDelete(rd_filter[level], range_delete_list_in);

        // update_mutex.unlock();
      }



      void addRangeDelete(uint level, long long start, long long end){
        init();
        std::lock_guard<std::mutex> guard(update_mutex);

        assert( rd_filter.size() >= level);
        if(rd_filter.size() == level){
          rd_filter.push_back(std::vector<pll>());
        }

        addRangeDelete(rd_filter[level], start, end);
      }


      /*
      *Do insertion, even if the vector is empty, because we need to set condition_variable of mutex (semaphore) for compaction
      */
      // input_level, output_level, file_boundries, file_numbers
      void shiftRDFToOutputLevel(std::vector<std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>>  *file_meta_data_vectors){
        init();
        // std::lock_guard<std::mutex> guard(update_mutex);

        // FIXME: FOR TESTING (next 2 lines)
        std::cout << "Before Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        update_mutex.lock();
        print_internal();
        update_mutex.unlock();

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

        // FIXME: FOR TESTING (next 2 lines)
        std::cout << "After Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        update_mutex.lock();
        print_internal();
        update_mutex.unlock();
      }

      // this is only used for direct compaction //
      // input_level, file_boundries, file_numbers
      void deleteRDFAssociatedWithFilesAtCurrentLevel(std::tuple<int, std::vector<pll>, std::vector<uint64_t>> *file_meta_data){
        init();
        std::lock_guard<std::mutex> guard(update_mutex);

        std::cout << "Before Deletion Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        print_internal();

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
          *     ||||-------|
          *     --------
          *     |      |
          *     --------
          */
          else if (val.first >= file_boundry.first && val.first <= file_boundry.second && val.second > file_boundry.second)
          {
            (*it).first = file_boundry.second + 1;
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
            itf++;
          }else{
            std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
            std::cerr << "val.first: " << val.first << " val.second: " << val.second << " file_boundry.first: " << file_boundry.first << " file_boundry.second: " << file_boundry.second << std::endl;
            assert(false);
            exit(1);
          }
        }

        rd_filter[level] = new_current_level_rdf;

        std::cout << "After Deletion Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        print_internal();
      }


      void print(){
        init();
        std::lock_guard<std::mutex> guard(update_mutex);

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

        // auto& rdList = range_delete_list;

        // for(auto it = rdList.begin(); it != rdList.end(); it++){
        //   std::cout << "(" << it->first << " " << it->second << ") ";
        // }
        // std::cout << std::endl;
      }


      bool isEntryAlive(uint level, long long key){
        init();
        std::lock_guard<std::mutex> guard(update_mutex);

        assert(rd_filter.size() > level);

        if(level >= rd_filter.size()){
          return true;
        }

        auto& rdList = rd_filter[level];
        if(rdList.size() == 0){return true;}

        // std::cout << "rdList: " <<" Level: " << level << " ,filter_size = " <<  rd_filter.size() << std::endl;
        // for(unsigned int i = 0 ; i < rd_filter.size(); i++){
        //   std::cout << "rdList: list " << i << std::endl;
        //   for(auto it = rd_filter[i].begin(); it != rd_filter[i].end(); it++){
        //     std::cout << "(" << it->first << " " << it->second << ") ";
        //   }
        // }
        // for(auto &x: rdList){
        //   std::cout << " --**- " << x.first << " " << x.second  << std::endl;
        // }
        // if(rdList.size() == 0){return true;}

        auto it = upper_bound(rdList.begin(), rdList.end(), pll(key, key), [](const pll& a, const pll& b){return a.first < b.first;});
        if(it != rdList.begin()){it--;}
        //[a,b], [c,d]
        // if(key >= it->first && key <= it->second){return false;}

        //[a,b), [c,d)
        if(key >= it->first && key < it->second){return false;}
        return true;
      }

      void deleteLastLevelIfEqualsBottomLevel(uint bottom_level){
        init();
        std::lock_guard<std::mutex> guard(update_mutex);

        if (rd_filter.size()-1 == bottom_level)
        {
          rd_filter[bottom_level].clear();
        }
      }

      // bool isEntryAlive(long long start){
      //   auto& rdList = range_delete_list;
      //   if(rdList.size() == 0){return true;}

      //   auto it = upper_bound(rdList.begin(), rdList.end(), pll(start, start), [](const pll& a, const pll& b){return a.first < b.first;});
      //   if(it != rdList.begin()){it--;}
      //   if(start >= it->first && start <= it->second){return false;}
      //   return true;
      // }


      // int getRangeDeleteCount(){
      //   return range_delete_list.size();
      // }




  };



  // std::mutex PLRDF::init_mutex;






} // namespace

#endif /* SYS_RDFILTER_H_ */



