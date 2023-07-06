/*
 *  Created on: April 23, 2023
 *  Author: yucheng
 */


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

#include <string>
#include <chrono>
#include <thread>
#include <mutex>

#include "../db/version_edit.h"



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
      std::mutex update_mutex, writeback_mutex;
      std::mutex level0_mutex;
      static std::mutex init_mutex;
      std::unordered_map<uint64_t, std::vector<pll>> rd_filter_level0; //FileMetaData* -> fd .GetNumber();

      std::vector<std::vector<pll>> rd_filter; //list of range delete (start, end), all entries are non-overlapping
  
      std::vector<pll> sortAndMerge(std::vector<pll> &range_delete_list_in);

      void addRangeDelete(std::vector<pll> &range_delete_list, long long start, long long end);
      void addRangeDelete(std::vector<pll> &range_delete_list, std::vector<pll> &range_delete_list_in);
      /*
      * adjust range deletes as per the compaction
      */
      void adjustRangeDeletes(uint clevel, uint olevel, std::vector<std::pair<long long, long long>> one_level_compaction_file_boundaries);


    public:
      static PLRDF* plrdf_ptr;

      
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

      void insertRangeDeleteToLevel0(uint64_t file_num, std::vector<pll> &range_delete_list_in);
      void printLevel0();

    
      // std::vector<pll> getRangeDeleteList();
      void addRangeDelete(uint level, long long start, long long end);
      void addRangeDelete(uint level, std::vector<pll> &range_delete_list_in);
      void shiftRDFToOutputLevel(std::vector<std::tuple<int, int, const std::vector<FileMetaData*>*>> *file_meta_data_vectors);
      void deleteLastLevelIfEqualsBottomLevel(uint bottom_level);
      void deleteRDFAssociatedWithFilesAtCurrentLevel(std::tuple<int, const std::vector<FileMetaData*>*> *file_meta_data);

      void print();

      bool isEntryAlive(uint level, long long key);
  };



  std::mutex PLRDF::init_mutex;


  std::vector<pll> PLRDF::sortAndMerge(std::vector<pll> &range_delete_list_in){
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

  void PLRDF::insertRangeDeleteToLevel0(uint64_t file_num, std::vector<pll> &range_delete_list_in){
    std::vector<pll> sorted_merged_rdlist = sortAndMerge(range_delete_list_in);

    init();
    std::lock_guard<std::mutex> guard(level0_mutex);

    if(rd_filter_level0.count(file_num) > 0){
      std:cerr << "Error: file_num already exists in rd_filter_level0" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl;
    }
    rd_filter_level0[file_num] = sorted_merged_rdlist;
  }

  void PLRDF::printLevel0(){
    init();
    std::lock_guard<std::mutex> guard(level0_mutex);

    std::cout << "rd_filter_level0" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<  std::endl << std::endl;
    for(auto it = rd_filter_level0.begin(); it != rd_filter_level0.end(); it++){
      std::cout << it->first << " " << it->second.size() << std::endl;
      //print all ranges
      for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++){
        std::cout << it2->first << " " << it2->second << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl << std::endl;
  }



void PLRDF::addRangeDelete(uint level, std::vector<pll> &range_delete_list_in){
  init();
  // update_mutex.lock();
  std::lock_guard<std::mutex> guard(init_mutex);

  assert( rd_filter.size() >= level);
  while (rd_filter.size() <= level)
  {
    rd_filter.push_back(std::vector<pll>());
  }

  addRangeDelete(rd_filter[level], range_delete_list_in);

  // update_mutex.unlock();
}

void PLRDF::addRangeDelete(std::vector<pll> &range_delete_list, std::vector<pll> &range_delete_list_in){
  init();
  std::lock_guard<std::mutex> guard(init_mutex);

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



void PLRDF::addRangeDelete(uint level, long long start, long long end){
  init();
  std::lock_guard<std::mutex> guard(init_mutex);

  assert( rd_filter.size() >= level);
  if(rd_filter.size() == level){
    rd_filter.push_back(std::vector<pll>());
  }

  addRangeDelete(rd_filter[level], start, end);
}


void PLRDF::addRangeDelete(std::vector<pll> &range_delete_list, long long start, long long end){
  init();
  std::lock_guard<std::mutex> guard(init_mutex);

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

// This would be used for trivial compaction and normal compaction
void PLRDF::adjustRangeDeletes(uint clevel, uint olevel, std::vector<std::pair<long long, long long>> one_level_compaction_file_boundaries){
  init();
  std::lock_guard<std::mutex> guard(init_mutex);
  
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

  addRangeDelete(olevel, to_be_added_in_next_level_rdf);

}

void PLRDF::shiftRDFToOutputLevel(std::vector<std::tuple<int, int, const std::vector<FileMetaData*>*>>  *file_meta_data_vectors){
  init();
  std::lock_guard<std::mutex> guard(init_mutex);

  // FIXME: FOR TESTING (next 2 lines)
  std::cout << "Before Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
  print();

  for (auto file_meta_data : *file_meta_data_vectors)
  {
    // file ranges
    std::vector<std::pair<long long, long long>> one_level_file_boundries;
    auto meta_data = std::get<2>(file_meta_data);

    for (auto meta : *meta_data)
    {
      one_level_file_boundries.push_back(std::make_pair(std::stoll(meta->smallest.user_key().ToString()), std::stoll(meta->largest.user_key().ToString())));
    }

    adjustRangeDeletes(std::get<0>(file_meta_data), std::get<1>(file_meta_data), one_level_file_boundries);

  }

  // FIXME: FOR TESTING (next 2 lines)
  std::cout << "After Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
  print();
}

// this is only used for direct compaction //
void PLRDF::deleteRDFAssociatedWithFilesAtCurrentLevel(std::tuple<int, const std::vector<FileMetaData*>*> *file_meta_data){
  init();
  std::lock_guard<std::mutex> guard(init_mutex);

  std::cout << "Before Deletion Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
  print();

  std::vector<std::pair<long long, long long>> one_level_file_boundries;
  auto level = std::get<0>(*file_meta_data);

  if (rd_filter.size() <= (uint)level)
  {
    return;
  }

  auto meta_data = std::get<1>(*file_meta_data);

  for (auto meta : *meta_data)
  {
      one_level_file_boundries.push_back(std::make_pair(std::stoll(meta->smallest.user_key().ToString()), std::stoll(meta->largest.user_key().ToString())));
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
  print();
}


void PLRDF::print(){
  init();
  std::lock_guard<std::mutex> guard(init_mutex);

  std::cout <<  std::setfill('-') << std::setw(60) << " START: Print PL RDF " << std::setfill('-') << "" << std::endl;
  for(uint l = 0; l < rd_filter.size(); l++){
    std::cout << "Level: " << l << std::endl;
    auto& rdList = PLRDF::rd_filter[l];
    for(auto it = rdList.begin(); it != rdList.end(); it++){
      std::cout << "(" << it->first << " " << it->second << ") ";
    }
    std::cout << std::endl;
  }
  std::cout <<  std::setfill('-') << std::setw(60) << " END: Print PL RDF " << std::setfill('-') << "" << std::endl;

  // auto& rdList = PLRDF::range_delete_list;

  // for(auto it = rdList.begin(); it != rdList.end(); it++){
  //   std::cout << "(" << it->first << " " << it->second << ") ";
  // }
  // std::cout << std::endl;
}


bool PLRDF::isEntryAlive(uint level, long long key){
  init();
  std::lock_guard<std::mutex> guard(init_mutex);

  assert(rd_filter.size() > level);

  if(level >= rd_filter.size()){
    return true;
  }

  auto& rdList = PLRDF::rd_filter[level];
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

void PLRDF::deleteLastLevelIfEqualsBottomLevel(uint bottom_level){
  init();
  std::lock_guard<std::mutex> guard(init_mutex);

  if (rd_filter.size()-1 == bottom_level)
  {
    rd_filter[bottom_level].clear();
  }
}

// bool PLRDF::isEntryAlive(long long start){
//   auto& rdList = PLRDF::range_delete_list;
//   if(rdList.size() == 0){return true;}

//   auto it = upper_bound(rdList.begin(), rdList.end(), pll(start, start), [](const pll& a, const pll& b){return a.first < b.first;});
//   if(it != rdList.begin()){it--;}
//   if(start >= it->first && start <= it->second){return false;}
//   return true;
// }


// int PLRDF::getRangeDeleteCount(){
//   return PLRDF::range_delete_list.size();
// }
//Self Added --- END PL-RDF ---



} // namespace

#endif /* SYS_RDFILTER_H_ */



