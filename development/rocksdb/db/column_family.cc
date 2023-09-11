//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).
//
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "db/column_family.h"

#include <algorithm>
#include <cinttypes>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include <tuple>

#include "db/blob/blob_file_cache.h"
#include "db/blob/blob_source.h"
#include "db/compaction/compaction_picker.h"
#include "db/compaction/compaction_picker_fifo.h"
#include "db/compaction/compaction_picker_level.h"
#include "db/compaction/compaction_picker_universal.h"
#include "db/db_impl/db_impl.h"
#include "db/internal_stats.h"
#include "db/job_context.h"
#include "db/range_del_aggregator.h"
#include "db/table_properties_collector.h"
#include "db/version_set.h"
#include "db/write_controller.h"
#include "file/sst_file_manager_impl.h"
#include "logging/logging.h"
#include "monitoring/thread_status_util.h"
#include "options/options_helper.h"
#include "port/port.h"
#include "rocksdb/convenience.h"
#include "rocksdb/table.h"
#include "table/merging_iterator.h"
#include "util/autovector.h"
#include "util/cast_util.h"
#include "util/compression.h"

//Self Added Start
// #include "include/rocksdb/sys_rdfilter.h"
// #include "rocksdb/sys_rdfilter.h"
#include "../include/rocksdb/sys_rdfilter.h"
//Self Added End

namespace ROCKSDB_NAMESPACE {


// //Self Added --- START PL-RDF ---


// void PLRDF::addRangeDelete_internal(uint level, std::vector<pll> &range_delete_list_in){
//   // init();
//   // update_mutex.lock();
//   // std::lock_guard<std::mutex> guard(update_mutex);

//   assert(rd_filter.size() >= level);
//   while (rd_filter.size() <= level)
//   {
//     rd_filter.push_back(std::vector<pll>());
//   }

//   addRangeDelete(rd_filter[level], range_delete_list_in);

//   // update_mutex.unlock();
// }



// std::vector<pll> PLRDF::sortAndMerge(std::vector<pll> &range_delete_list_in){
//   if(range_delete_list_in.size() == 0){
//     return {};
//   }

//   std::sort(range_delete_list_in.begin(), range_delete_list_in.end(), [](pll a, pll b){
//     return a.first < b.first;
//   });

//   std::vector<pll> range_delete_list;
//   range_delete_list.reserve(range_delete_list_in.size());
//   auto itA = range_delete_list_in.begin();
//   auto iteA = range_delete_list_in.end();
//   pll tmp_range = *itA;
//   for(;itA != iteA; itA++){
//     if(tmp_range.second >= itA->first){
//       tmp_range.second = std::max(tmp_range.second, itA->second);
//     }else{
//       range_delete_list.push_back(tmp_range);
//       tmp_range = *itA;
//     }
//   }

//   range_delete_list.push_back(tmp_range);
//   return range_delete_list;
// }



// void PLRDF::addRangeDelete(std::vector<pll> &range_delete_list, std::vector<pll> &range_delete_list_in){
// // init();
// // std::lock_guard<std::mutex> guard(init_mutex);

//   auto& rdList = range_delete_list;
//   auto& rdList_in = range_delete_list_in;

// // // std::cout << "rdList" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<  std::endl << std::endl;
// // // for(auto it = rdList.begin(); it != rdList.end(); it++){
// // // std::cout << "aaaa " << it->first << " " << it->second << std::endl;
// // // }
// // // std::cout << std::endl << std::endl;
// // std::cout << "rdList_in" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<  std::endl << std::endl;
// // for(auto it = rdList_in.begin(); it != rdList_in.end(); it++){
// // std::cout << "aaaa2 " << it->first << " " << it->second << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
// // }
// // std::cout << std::endl << std::endl;


// // std::cout << "rdList_in" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<  std::endl << std::endl;
// // for(auto it = rdList_in.begin(); it != rdList_in.end(); it++){
// //   std::cout << it->first << " " << it->second << std::endl;
// // }
// // std::cout << std::endl << std::endl;

//   if(rdList_in.size() == 0){return;}

//   for(uint i = 1; i < rdList_in.size(); i++){
//     if(rdList_in[i-1].first > rdList_in[i].first){
//       std::cerr << "Error: rdList_in to be flushed has not been sorted in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl;
//       exit(1);
//     }
//   }

//   if(rdList.size() == 0){
//     //do the merging first before adding to rdList
//     // std::vector<pll> rdList_new;
//     rdList.reserve(rdList_in.size());
//     auto itA = rdList_in.begin();
//     auto iteA = rdList_in.end();
//     pll tmp_range = *itA;
//     for(;itA != iteA; itA++){
//       if(tmp_range.second >= itA->first){
//         tmp_range.second = std::max(tmp_range.second, itA->second);
//       }else{
//         rdList.push_back(tmp_range);
//         tmp_range = *itA;
//       }
//     }

//     rdList.push_back(tmp_range);

//     // std::cout << "after_direct insert to rdList: " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<  std::endl << std::endl;
//     // for(auto it = rdList.begin(); it != rdList.end(); it++){
//     //   std::cout << it->first << " " << it->second << std::endl;
//     // }


//     // //adding to rdList
//     // rdList.reserve(rdList_new.size());
//     // for(auto &p : rdList_new){
//     //   rdList.push_back(p);
//     // }
    
//     return;
//   }




//   std::vector<pll> rdList_new;
//   rdList_new.reserve(rdList.size() + rdList_in.size());



//   auto itA = rdList.begin();
//   auto iteA = rdList.end();
//   auto itB = rdList_in.begin();
//   auto iteB = rdList_in.end();

//   // long long minK = start;
//   // long long maxK = end;
//   pll tmp_range;
//   if(itA->first < itB->first){
//     tmp_range = *itA;
//   }else{
//     tmp_range = *itB;
//   }

//   while(itA != iteA || itB != iteB){
// // std::cout << " itA = " << itA->first << " " << itA->second << std::endl;
//     if(itA != iteA && itA->first <= tmp_range.second){
//       tmp_range.second = std::max(tmp_range.second, itA->second);
//       itA++;
//       continue;
//     }
//     if(itB != iteB && itB->first <= tmp_range.second){
//       tmp_range.second = std::max(tmp_range.second, itB->second);
//       itB++;
//       continue;
//     }
//     rdList_new.push_back(tmp_range);

//     if(itA == iteA){
//       tmp_range = *itB;
//       itB++;
//       continue;
//     }
//     if(itB == iteB){
//       tmp_range = *itA;
//       itA++;
//       continue;
//     }

//     if(itA->first <= itB->first){
//       tmp_range = *itA;
//       itA++;
//     }else{
//       tmp_range = *itB;
//       itB++;
//     }
//   }
//   rdList_new.push_back(tmp_range);


// // std::cout << "rdList_new" << std::endl << std::endl;
// // for(auto it = rdList_new.begin(); it != rdList_new.end(); it++){
// //   std::cout << it->first << " " << it->second << std::endl;
// // }
// // std::cout << "rdList" << std::endl << std::endl;
// // for(auto it = rdList.begin(); it != rdList.end(); it++){
// //   std::cout << it->first << " " << it->second << std::endl;
// // }
// // std::cout << std::endl << std::endl;


//   rdList.clear();
//   rdList.reserve(rdList_new.size());
//   for(auto &p : rdList_new){
//     rdList.push_back(p);
//   }
// }


// void PLRDF::addRangeDelete(std::vector<pll> &range_delete_list, long long start, long long end){
// // init();
// // std::lock_guard<std::mutex> guard(init_mutex);

// auto& rdList = range_delete_list;
// #ifdef DEBUG
//   cout << "Adding range delete: " << start << " " << end << endl;
// #endif
//   std::vector<pll> rdList_new;
//   rdList_new.reserve(rdList.size()+1);

//   long long minK = start;
//   long long maxK = end;
  
//   auto it = rdList.begin();
//   while ( it != rdList.end() ){
//       // [a,b], [c,d]
//       // if (it->second < start-1){ rdList_new.push_back(*it); it++; continue;}
//       // if (it->first > end+1){ break;}
      
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

// void PLRDF::print_internal(){
//   // init();
//   // std::lock_guard<std::mutex> guard(update_mutex);

//   std::cout <<  std::setfill('-') << std::setw(60) << " START: Print PL RDF " << std::setfill('-') << "" << std::endl;
//   for(uint l = 0; l < rd_filter.size(); l++){
//     std::cout << "Level: " << l << std::endl;
//     auto& rdList = rd_filter[l];
//     for(auto it = rdList.begin(); it != rdList.end(); it++){
//       std::cout << "(" << it->first << " " << it->second << ") ";
//     }
//     std::cout << std::endl;
//   }
//   std::cout <<  std::setfill('-') << std::setw(60) << " END: Print PL RDF " << std::setfill('-') << "" << std::endl;

//   // auto& rdList = range_delete_list;

//   // for(auto it = rdList.begin(); it != rdList.end(); it++){
//   //   std::cout << "(" << it->first << " " << it->second << ") ";
//   // }
//   // std::cout << std::endl;
// }











//   // // This would be used for trivial compaction and normal compaction
//   // void PerlevelRangeDeleteFilterByVector::adjustRangeDeletes(uint clevel, uint olevel, std::vector<std::pair<long long, long long>> one_level_compaction_file_boundaries)
//   // {
//   //   std::vector<pll> new_current_level_rdf;
//   //   std::vector<pll> to_be_added_in_next_level_rdf;
//   //
//   //   if (rd_filter.size() <= clevel)
//   //   {
//   //     return;
//   //   }
//   //
//   //   auto old_current_level_rdf = rd_filter[clevel];
//   //
//   //   // FIXME: (Shubham) This might not be required
//   //   if (one_level_compaction_file_boundaries.size() == 0)
//   //   {
//   //     return;
//   //   }
//   //
//   //   auto it = old_current_level_rdf.begin();
//   //   auto itf = one_level_compaction_file_boundaries.begin();
//   //
//   //   while (it != old_current_level_rdf.end())
//   //   {
//   //     pll val = *it;
//   //     auto file_boundries = *itf;
//   //     pll file_boundry = std::make_pair(file_boundries.first, file_boundries.second);
//   //
//   //     /*
//   //     *    |--|
//   //     *         -----
//   //     *         |   |
//   //     *         -----
//   //     */
//   //     if (itf == one_level_compaction_file_boundaries.end() || (val.second <= file_boundry.first))
//   //     {
//   //       new_current_level_rdf.push_back(val);
//   //       it++;
//   //     }
//   //     /*
//   //     *             |--|
//   //     *     ------
//   //     *     |    |
//   //     *     ------
//   //     */
//   //     else if (val.first > file_boundry.second)
//   //     {
//   //       itf++;
//   //     }
//   //     /*
//   //     *    |------||||
//   //     *         ------
//   //     *         |    |
//   //     *         ------
//   //     */
//   //     else if (val.first < file_boundry.first && val.second > file_boundry.first && val.second <= file_boundry.second)
//   //     {
//   //       new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
//   //       to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first, val.second));
//   //       it++;
//   //     }
//   //     /*
//   //     *    |||--|||
//   //     *    --------
//   //     *    |      |
//   //     *    --------
//   //     */
//   //     else if (val.first >= file_boundry.first && val.second <= file_boundry.second)
//   //     {
//   //       to_be_added_in_next_level_rdf.push_back(val);
//   //       it++;
//   //     }
//   //     /*
//   //     *     ||||-------|
//   //     *     --------
//   //     *     |      |
//   //     *     --------
//   //     */
//   //     else if (val.first >= file_boundry.first && val.first <= file_boundry.second && val.second > file_boundry.second)
//   //     {
//   //       to_be_added_in_next_level_rdf.push_back(std::make_pair(val.first, file_boundry.second + 1));
//   //       (*it).first = file_boundry.second + 1;
//   //       itf++;
//   //     }
//   //     /*
//   //     *  |------------|
//   //     *     --------
//   //     *     |      |
//   //     *     --------
//   //     */
//   //     else if (val.first < file_boundry.first && val.second > file_boundry.second)
//   //     {
//   //       new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
//   //       to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first, file_boundry.second + 1));
//   //       (*it).first = file_boundry.second + 1;
//   //       itf++;
//   //     }else{
//   //       std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//   //       std::cerr << "val.first: " << val.first << " val.second: " << val.second << " file_boundry.first: " << file_boundry.first << " file_boundry.second: " << file_boundry.second << std::endl;
//   //       assert(false);
//   //       exit(1);
//   //     }
//   //   }
//   //
//   //   rd_filter[clevel] = new_current_level_rdf;
//   //   std::sort(to_be_added_in_next_level_rdf.begin(), to_be_added_in_next_level_rdf.end(), [](const pll a, const pll b)
//   //           { return a.first < b.first; });
//   //
//   //   addRangeDelete(olevel, to_be_added_in_next_level_rdf);
//   //
//   // }



// void PLRDF::adjustRangeDeletesForLevel0Input(uint olevel, std::vector<uint64_t> file_numbers){

//   std::vector<pll> to_be_added_in_next_level_rdf;


//   for(uint64_t &file_num: file_numbers){
// // //     {
// // //       // -- semaphores_level0 --
// // //       semaphores_level0_mutex.lock();
// // //       // if(semaphores_level0.count(file_num) == 0){
// // //         // semaphores_level0[file_num] = std::binary_semaphore{0};
// // //         // semaphores_level0[file_num] = make_pair(std::mutex(), std::condition_variable());
// // // std::cout << "Compact From Level0 " << "semaphores_m_level0.count(file_num) : " << semaphores_m_level0.count(file_num) << " file_num =  " << file_num << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
// // // std::cout << "Compact From Level0 " << "semaphores_cv_level0.count(file_num) : " << semaphores_cv_level0.count(file_num) << " file_num =  " << file_num << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
// // //       if(semaphores_m_level0.count(file_num) == 0){
// // //         semaphores_m_level0.emplace(std::piecewise_construct,
// // //                 std::forward_as_tuple(file_num),
// // //                 std::forward_as_tuple());
// // //         semaphores_cv_level0.emplace(std::piecewise_construct,
// // //                 std::forward_as_tuple(file_num),
// // //                 std::forward_as_tuple());
// // //       }
// // //       semaphores_level0_mutex.unlock();
// // //       // -- semaphores_level0 --

// // std::cout << "Compact From Level0 " << "file_num: " << file_num << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

// // //       // -- wait on semaphores_level0 --
// // //       // waiting for the signal from flushJob that the RD of the file 
// // //       // is already added to the rd_filter_level0
// // //       // semaphores_level0[file_num].acquire();  
// // //       // std::unique_lock lk(semaphores_level0[file_num].first); 
// // //       // semaphores_level0[file_num].second.wait(lk, [&] {return rd_filter_level0.count(file_num) > 0;}); // waken when condition becomes true
// // //       std::unique_lock lk(semaphores_m_level0[file_num]); 
// // //       semaphores_cv_level0[file_num].wait(lk, [&] {return rd_filter_level0.count(file_num) > 0;}); // waken when condition becomes true
// // //       // semaphores_cv_level0[file_num].wait(lk);
// // //       lk.unlock();
// // // std::cout << "Compact From Level0 " << "UnLocked !!" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
// // //       // -- wait on semaphores_level0 --
// // //     }

//     {
//       // // -- rd_filter_level0 --
//       // rd_filter_level0_mutex.lock();

//       auto it = rd_filter_level0.find(file_num);
//       if(it == rd_filter_level0.end()){
//         assert(it != rd_filter_level0.end());
//         std::cerr << "File number not found in level 0 " << "File number " << file_num << " " << __FILE__ << ":" << __LINE__ << std::endl; 
//         std::cerr << "Remindation: Do the manually flush after all the insert workload are done. So no entries lie inside memtable anymore. In case those entries will go through the track of bulk buiding from WAL and no going through the path of flushJob." << "File number " << file_num << " " << __FILE__ << ":" << __LINE__ << std::endl; 
//         exit(1);
//         // continue;
//       }
//       auto val = it->second;
//       to_be_added_in_next_level_rdf.insert(to_be_added_in_next_level_rdf.end(), val.begin(), val.end());
//       rd_filter_level0.erase(it);
      
//     //   rd_filter_level0_mutex.unlock();
//     //   // -- rd_filter_level0 -- 
//     }

//     // {
//     //   // -- semaphores_level0 --
//     //   semaphores_level0_mutex.lock();

//     //   // semaphores_level0.erase(file_num);  // also remove the semaphore of the current file_num
//     //   semaphores_m_level0.erase(file_num);  // also remove the semaphore of the current file_num
//     //   semaphores_cv_level0.erase(file_num);  // also remove the semaphore of the current file_num

//     //   semaphores_level0_mutex.unlock();
//     //   // -- semaphores_level0 --
//     // }
//   }

//   std::sort(to_be_added_in_next_level_rdf.begin(), to_be_added_in_next_level_rdf.end(), [](const pll a, const pll b)
//           { return a.first < b.first; });



//   // // -- updating rd_filter_level0 --
//   // std::lock_guard<std::mutex> guard(rd_filter_level0_mutex);

//   addRangeDelete_internal(olevel, to_be_added_in_next_level_rdf);
// }



// // This would be used for trivial compaction and normal compaction
// // input_level, output_level, file_boundaries
// void PLRDF::adjustRangeDeletes(uint clevel, uint olevel, std::vector<std::pair<long long, long long>> one_level_compaction_file_boundaries){
//   // // init();
//   // std::lock_guard<std::mutex> guard(update_mutex);

  
//   std::vector<pll> new_current_level_rdf;
//   std::vector<pll> to_be_added_in_next_level_rdf;

//   if (rd_filter.size() <= clevel)
//   {
//     return;
//   }
  
//   auto old_current_level_rdf = rd_filter[clevel];

//   // FIXME: (Shubham) This might not be required
//   if (one_level_compaction_file_boundaries.size() == 0)
//   {
//     return;
//   }

//   auto it = old_current_level_rdf.begin();
//   auto itf = one_level_compaction_file_boundaries.begin();

//   while (it != old_current_level_rdf.end())
//   {
//     pll val = *it;
//     auto file_boundries = *itf;
//     pll file_boundry = std::make_pair(file_boundries.first, file_boundries.second);

//     /*
//     *    |--|
//     *         -----
//     *         |   |
//     *         -----
//     */
//     if (itf == one_level_compaction_file_boundaries.end() || (val.second <= file_boundry.first))
//     {
//       new_current_level_rdf.push_back(val);
//       it++;
//     }
//     /*
//     *             |--|
//     *     ------
//     *     |    |
//     *     ------
//     */
//     else if (val.first > file_boundry.second)
//     {
//       itf++;
//     }
//     /*
//     *    |------||||
//     *         ------
//     *         |    |
//     *         ------
//     */
//     else if (val.first < file_boundry.first && val.second > file_boundry.first && val.second <= file_boundry.second)
//     {
//       new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
//       to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first, val.second));
//       it++;
//     }
//     /*
//     *    |||--|||
//     *    --------
//     *    |      |
//     *    --------
//     */
//     else if (val.first >= file_boundry.first && val.second <= file_boundry.second)
//     {
//       to_be_added_in_next_level_rdf.push_back(val);
//       it++;
//     }
//     /*
//     *     ||||-------|
//     *     --------
//     *     |      |
//     *     --------
//     */
//     else if (val.first >= file_boundry.first && val.first <= file_boundry.second && val.second > file_boundry.second)
//     {
//       to_be_added_in_next_level_rdf.push_back(std::make_pair(val.first, file_boundry.second + 1));
//       (*it).first = file_boundry.second + 1;
//       if((*it).first >= (*it).second){ it++; } // <------------------------
//       itf++;
//     }
//     /*
//     *  |------------|
//     *     --------
//     *     |      |
//     *     --------
//     */
//     else if (val.first < file_boundry.first && val.second > file_boundry.second)
//     {
//       new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
//       to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first, file_boundry.second + 1));
//       (*it).first = file_boundry.second + 1;
//       if((*it).first >= (*it).second){ it++; } // <------------------------
//       itf++;
//     }else{
//       std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//       std::cerr << "val.first: " << val.first << " val.second: " << val.second << " file_boundry.first: " << file_boundry.first << " file_boundry.second: " << file_boundry.second << std::endl;
//       assert(false);
//       exit(1);
//     }
//   }

//   rd_filter[clevel] = new_current_level_rdf;
//   std::sort(to_be_added_in_next_level_rdf.begin(), to_be_added_in_next_level_rdf.end(), [](const pll a, const pll b)
//           { return a.first < b.first; });

//   addRangeDelete_internal(olevel, to_be_added_in_next_level_rdf);
// }






// void PLRDF::insertRangeDeleteToLevel0(uint64_t file_num, std::vector<pll> &range_delete_list_in, std::vector<uint64_t> exist_level0_file_nums){
//   std::vector<pll> sorted_merged_rdlist = sortAndMerge(range_delete_list_in);
//   // init();

//   // -- updating rd_filter_level0 --
//   // std::lock_guard<std::mutex> guard(rd_filter_level0_mutex);
//   // rd_filter_level0_mutex.lock();

//   // if(rd_filter_level0.count(file_num) > 0){
//   if(std::binary_search(exist_level0_file_nums.begin(), exist_level0_file_nums.end(), file_num) == true){
//     std::cerr << "Error: file_num already exists in rd_filter_level0 " << "file_num = " << file_num << "\t" << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
//     exit(1);
//   }
//   rd_filter_level0[file_num] = sorted_merged_rdlist;

//   // std::cout << "rd_filter_Level0 " << "file_num: " << file_num << " number of RD: " << sorted_merged_rdlist.size() << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

//   // rd_filter_level0_mutex.unlock();
//   // -- updating rd_filter_level0 --


// //   {
// //     // -- semaphores_level0 --
// //     semaphores_level0_mutex.lock();
// //     // if(semaphores_level0.count(file_num) == 0){
// //     //   semaphores_level0[file_num] = std::binary_semaphore{0};
// //     // }
// //     if(semaphores_m_level0.count(file_num) == 0){
// //       // semaphores_level0[file_num] = std::binary_semaphore{0};
// //       // semaphores_level0[file_num] = make_pair(std::mutex(), std::condition_variable());
// //       semaphores_m_level0.emplace(std::piecewise_construct,
// //               std::forward_as_tuple(file_num),
// //               std::forward_as_tuple());
// //       semaphores_cv_level0.emplace(std::piecewise_construct,
// //               std::forward_as_tuple(file_num),
// //               std::forward_as_tuple());
// //     }

// // std::cout << "Flush To Level0 " << "file_num: " << file_num << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
// //     // signaling compaction thread (which uses adjustRangeDeletesForLevel0Input) 
// //     // that RDs of the file_num has already been inserted
// //     // semaphores_level0[file_num].release();
// //     semaphores_cv_level0[file_num].notify_one();

// //     semaphores_level0_mutex.unlock();
// //     // -- semaphores_level0 --
// //   }
// }

// void PLRDF::printLevel0(){
//   // init();
//   // std::lock_guard<std::mutex> guard(rd_filter_level0_mutex);

//   std::cout << "rd_filter_level0" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<  std::endl << std::endl;
//   for(auto it = rd_filter_level0.begin(); it != rd_filter_level0.end(); it++){
//     std::cout << "rd_filter_level0 file number: " << it->first << " number of RD: " << it->second.size() << std::endl;
//     //print all ranges
//     for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++){
//       std::cout <<  "rd_filter_level0" << " " << it2->first << " " << it2->second << " ";
//     }
//     std::cout << std::endl;
//   }
//   std::cout << std::endl << std::endl;
// }



// void PLRDF::addRangeDelete(uint level, std::vector<pll> &range_delete_list_in){
//   // init();
//   // // update_mutex.lock();
//   // std::lock_guard<std::mutex> guard(update_mutex);

//   // assert( rd_filter.size() >= level);
//   while (rd_filter.size() <= level)
//   {
//     rd_filter.push_back(std::vector<pll>());
//   }

//   addRangeDelete(rd_filter[level], range_delete_list_in);

//   // update_mutex.unlock();
// }



// // void addRangeDelete(uint level, long long start, long long end){
// //   // init();
// //   // std::lock_guard<std::mutex> guard(update_mutex);
// //
// //   assert( rd_filter.size() >= level);
// //   if(rd_filter.size() == level){
// //     rd_filter.push_back(std::vector<pll>());
// //   }
// //
// //   addRangeDelete(rd_filter[level], start, end);
// // }






//   // void PerlevelRangeDeleteFilterByVector::shiftRDFToOutputLevel(std::vector<std::tuple<int, int, const std::vector<FileMetaData*>*>>  *file_meta_data_vectors)
//   // {
//   //   // FIXME: FOR TESTING (next 2 lines)
//   //   std::cout << "Before Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//   //   print();
//   //
//   //   for (auto file_meta_data : *file_meta_data_vectors)
//   //   {
//   //     // file ranges
//   //     std::vector<std::pair<long long, long long>> one_level_file_boundries;
//   //     auto meta_data = std::get<2>(file_meta_data);
//   //
//   //     for (auto meta : *meta_data)
//   //     {
//   //       one_level_file_boundries.push_back(std::make_pair(std::stoll(meta->smallest.user_key().ToString()), std::stoll(meta->largest.user_key().ToString())));
//   //     }
//   //
//   //     adjustRangeDeletes(std::get<0>(file_meta_data), std::get<1>(file_meta_data), one_level_file_boundries);
//   //
//   //   }
//   //
//   //   // FIXME: FOR TESTING (next 2 lines)
//   //   std::cout << "After Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//   //   print();
//   // }



// /*
// *Do insertion, even if the vector is empty, because we need to set condition_variable of mutex (semaphore) for compaction
// */
// // input_level, output_level, file_boundries, file_numbers
// void PLRDF::shiftRDFToOutputLevel(std::vector<std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>>  *file_meta_data_vectors){
//     // init();
//   // std::lock_guard<std::mutex> guard(update_mutex);

//   // // // FIXME: FOR TESTING (next 2 lines)
//   // // std::cout << "Before Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//   // // // update_mutex.lock();
//   // print_internal();
//   // // update_mutex.unlock();

//   for (auto file_meta_data : *file_meta_data_vectors)
//   {
//     int clevel = std::get<0>(file_meta_data);
//     if(clevel == 0){
//       adjustRangeDeletesForLevel0Input(std::get<1>(file_meta_data), std::get<3>(file_meta_data));
//     }else{
//       // file ranges
//       std::vector<std::pair<long long, long long>> one_level_file_boundries;
//       auto meta_data = std::get<2>(file_meta_data);

//       for (auto meta : meta_data)
//       {
//         one_level_file_boundries.push_back(std::make_pair(meta.first, meta.second));
//       }

//       adjustRangeDeletes(std::get<0>(file_meta_data), std::get<1>(file_meta_data), one_level_file_boundries);
//     }
//   }

//   // // // FIXME: FOR TESTING (next 2 lines)
//   // // std::cout << "After Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//   // // // update_mutex.lock();
//   // print_internal();
//   // // update_mutex.unlock();
// }


//   // // this is only used for direct compaction //
//   // void PerlevelRangeDeleteFilterByVector::deleteRDFAssociatedWithFilesAtCurrentLevel(std::tuple<int, const std::vector<FileMetaData*>*> *file_meta_data)
//   // {
//   //   std::cout << "Before Deletion Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//   //   print();
//   //
//   //
//   //   std::vector<std::pair<long long, long long>> one_level_file_boundries;
//   //   auto level = std::get<0>(*file_meta_data);
//   //
//   //   if (rd_filter.size() <= (uint)level)
//   //   {
//   //     return;
//   //   }
//   //
//   //   auto meta_data = std::get<1>(*file_meta_data);
//   //
//   //   for (auto meta : *meta_data)
//   //   {
//   //       one_level_file_boundries.push_back(std::make_pair(std::stoll(meta->smallest.user_key().ToString()), std::stoll(meta->largest.user_key().ToString())));
//   //   }
//   // 
//   //   std::vector<pll> new_current_level_rdf;
//   //   auto old_current_level_rdf = rd_filter[level];
//   //   auto it = old_current_level_rdf.begin();
//   //   auto itf = one_level_file_boundries.begin();
//   //
//   //   while (it != old_current_level_rdf.end())
//   //   {
//   //     pll val = *it;
//   //     auto file_boundries = *itf;
//   //     pll file_boundry = std::make_pair(file_boundries.first, file_boundries.second);
//   //
//   //     /*
//   //     *    |--|
//   //     *         -----
//   //     *         |   |
//   //     *         -----
//   //     */
//   //     if (itf == one_level_file_boundries.end() || (val.second <= file_boundry.first))
//   //     {
//   //       new_current_level_rdf.push_back(val);
//   //       it++;
//   //     }
//   //     /*
//   //     *             |--|
//   //     *     ------
//   //     *     |    |
//   //     *     ------
//   //     */
//   //     else if (val.first > file_boundry.second)
//   //     {
//   //       itf++;
//   //     }
//   //     /*
//   //     *    |------||||
//   //     *         ------
//   //     *         |    |
//   //     *         ------
//   //     */
//   //     else if (val.first < file_boundry.first && val.second > file_boundry.first && val.second <= file_boundry.second)
//   //     {
//   //       new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
//   //       it++;
//   //     }
//   //     /*
//   //     *     ||||-------|
//   //     *     --------
//   //     *     |      |
//   //     *     --------
//   //     */
//   //     else if (val.first >= file_boundry.first && val.first <= file_boundry.second && val.second > file_boundry.second)
//   //     {
//   //       (*it).first = file_boundry.second + 1;
//   //       itf++;
//   //     }
//   //     /*
//   //     *  |------------|
//   //     *     --------
//   //     *     |      |
//   //     *     --------
//   //     */
//   //     else if (val.first < file_boundry.first && val.second > file_boundry.second)
//   //     {
//   //       new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
//   //       (*it).first = file_boundry.second + 1;
//   //       itf++;
//   //     }else{
//   //       std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//   //       std::cerr << "val.first: " << val.first << " val.second: " << val.second << " file_boundry.first: " << file_boundry.first << " file_boundry.second: " << file_boundry.second << std::endl;
//   //       assert(false);
//   //       exit(1);
//   //     }
//   //   }
//   //
//   //   rd_filter[level] = new_current_level_rdf;
//   //
//   //   std::cout << "After Deletion Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//   //   print();
//   //  }


// // this is only used for direct compaction //
// // input_level, file_boundries, file_numbers
// void PLRDF::deleteRDFAssociatedWithFilesAtCurrentLevel(std::tuple<int, std::vector<pll>, std::vector<uint64_t>> *file_meta_data){
//   // init();
//   // std::lock_guard<std::mutex> guard(update_mutex);

//   // std::cout << "Before Deletion Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//   // print_internal();

//   std::vector<std::pair<long long, long long>> one_level_file_boundries;
//   auto level = std::get<0>(*file_meta_data);

//   if(level == 0){
//     auto it = rd_filter_level0.find(level);
//     if(it == rd_filter_level0.end()){
//       assert(it != rd_filter_level0.end());
//       std::cerr << "Error: file_num does not exist in rd_filter_level0" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl;
//       exit(1);
//     }
//     rd_filter_level0.erase(it);
//     return;
//   }


//   if (rd_filter.size() <= (uint)level)
//   {
//     return;
//   }

//   auto meta_data = std::get<1>(*file_meta_data);

//   for (auto meta : meta_data)
//   {
//       one_level_file_boundries.push_back(std::make_pair(meta.first, meta.second));
//   }

//   std::vector<pll> new_current_level_rdf;
//   auto old_current_level_rdf = rd_filter[level];
//   auto it = old_current_level_rdf.begin();
//   auto itf = one_level_file_boundries.begin();

//   while (it != old_current_level_rdf.end())
//   {
//     pll val = *it;
//     auto file_boundries = *itf;
//     pll file_boundry = std::make_pair(file_boundries.first, file_boundries.second);

//     /*
//     *    |--|
//     *         -----
//     *         |   |
//     *         -----
//     */
//     if (itf == one_level_file_boundries.end() || (val.second <= file_boundry.first))
//     {
//       new_current_level_rdf.push_back(val);
//       it++;
//     }
//     /*
//     *             |--|
//     *     ------
//     *     |    |
//     *     ------
//     */
//     else if (val.first > file_boundry.second)
//     {
//       itf++;
//     }
//     /*
//     *    |------||||
//     *         ------
//     *         |    |
//     *         ------
//     */
//     else if (val.first < file_boundry.first && val.second > file_boundry.first && val.second <= file_boundry.second)
//     {
//       new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
//       it++;
//     }
//     /*
//     *    |||--|||
//     *    --------
//     *    |      |
//     *    --------
//     */
//     else if (val.first >= file_boundry.first && val.second <= file_boundry.second)
//     {
//       it++;
//     }
//     /*
//     *     ||||-------|
//     *     --------
//     *     |      |
//     *     --------
//     */
//     else if (val.first >= file_boundry.first && val.first <= file_boundry.second && val.second > file_boundry.second)
//     {
//       (*it).first = file_boundry.second + 1;
//       if((*it).first >= (*it).second){ it++; } // <------------------------
//       itf++;
//     }
//     /*
//     *  |------------|
//     *     --------
//     *     |      |
//     *     --------
//     */
//     else if (val.first < file_boundry.first && val.second > file_boundry.second)
//     {
//       new_current_level_rdf.push_back(std::make_pair(val.first, file_boundry.first));
//       (*it).first = file_boundry.second + 1;
//       if((*it).first >= (*it).second){ it++; } // <------------------------
//       itf++;
//     }else{
//       std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//       std::cerr << "val.first: " << val.first << " val.second: " << val.second << " file_boundry.first: " << file_boundry.first << " file_boundry.second: " << file_boundry.second << std::endl;
//       assert(false);
//       exit(1);
//     }
//   }

//   rd_filter[level] = new_current_level_rdf;

//   // std::cout << "After Deletion Comapction" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//   // print_internal();
// }




// vector<pll> PLRDF::getLevelRanges(int outlevel){
//   // init();
//   // std::lock_guard<std::mutex> guard(update_mutex);
//   if((uint)outlevel >= rd_filter.size()){return {};}
//   return rd_filter[outlevel];
// }
// void PLRDF::setLevelRanges(vector<pll> level_ranges_in, int outlevel){
//   // init();
//   // std::lock_guard<std::mutex> guard(update_mutex);
//   while(rd_filter.size() <= (uint)outlevel){
//     rd_filter.push_back(vector<pll>());
//   }
  
//   if(level_ranges_in != rd_filter[outlevel]){
//     std::cout << "^^^ outlevel: " << outlevel 
//               << " level_ranges_in.size(): " << level_ranges_in.size() 
//               << " rd_filter[outlevel].size() " << rd_filter[outlevel].size() << std::endl;
//     std::cout << "^^^ level_ranges_in: " << std::endl;
//     for(auto it = level_ranges_in.begin(); it != level_ranges_in.end(); it++){
//       std::cout << "(" << it->first << " " << it->second << ") ";
//     }
//     std::cout << std::endl;
//     std::cout << "^^^ rd_filter[outlevel]: " << std::endl;
//     for(auto it = rd_filter[outlevel].begin(); it != rd_filter[outlevel].end(); it++){
//       std::cout << "(" << it->first << " " << it->second << ") ";
//     }
//     std::cout << std::endl;
//   }

  
//   rd_filter[outlevel] = level_ranges_in;

// }


// int PLRDF::getNumberOfTotalLevels(){
//   int num = 0;
//   int len = rd_filter.size();
//   for(int i = 1; i < len; i++){
//     if(rd_filter[i].size() > 0){
//       num = i+1;
//     }
//   }
//   return num;
// }

// int PLRDF::getNumberOfTotalRanges(){
//   int num = 0;
//   for(auto it = rd_filter.begin(); it != rd_filter.end(); it++){
//     num += it->size();
//   }
//   return num;
// }

// void PLRDF::print(){
//   // init();
//   // std::lock_guard<std::mutex> guard(update_mutex);

//   std::cout <<  std::setfill('-') << std::setw(60) << " START: Print  RDF " << std::setfill('-') << "" << std::endl;
//   for(uint l = 0; l < rd_filter.size(); l++){
//     std::cout << "Level: " << l << std::endl;
//     auto& rdList = rd_filter[l];
//     for(auto it = rdList.begin(); it != rdList.end(); it++){
//       std::cout << "(" << it->first << " " << it->second << ") ";
//     }
//     std::cout << std::endl;
//   }
//   std::cout <<  std::setfill('-') << std::setw(60) << " END: Print  RDF " << std::setfill('-') << "" << std::endl;
// }


// bool PLRDF::isEntryAlive(uint level, long long key){
//   // init();
//   // std::lock_guard<std::mutex> guard(update_mutex);

//   assert(rd_filter.size() > level);

//   if(level >= rd_filter.size()){
//     return true;
//   }

//   auto& rdList = rd_filter[level];
//   if(rdList.size() == 0){return true;}

//   auto it = upper_bound(rdList.begin(), rdList.end(), pll(key, key), [](const pll& a, const pll& b){return a.first < b.first;});
//   if(it != rdList.begin()){it--;}
//   //[a,b], [c,d]
//   // if(key >= it->first && key <= it->second){return false;}

//   //[a,b), [c,d)
//   if(key >= it->first && key < it->second){return false;}
//   return true;
// }

// void PLRDF::deleteLastLevelIfEqualsBottomLevel(uint bottom_level){
//   // init();
//   // std::lock_guard<std::mutex> guard(update_mutex);

//   if (rd_filter.size()-1 == bottom_level)
//   {
//     rd_filter[bottom_level].clear();
//   }
// }




// // bool PerlevelRangeDeleteFilterByVector::isEntryAlive(long long start){
// //   auto& rdList = PerlevelRangeDeleteFilterByVector::range_delete_list;
// //   if(rdList.size() == 0){return true;}

// //   auto it = upper_bound(rdList.begin(), rdList.end(), pll(start, start), [](const pll& a, const pll& b){return a.first < b.first;});
// //   if(it != rdList.begin()){it--;}
// //   if(start >= it->first && start <= it->second){return false;}
// //   return true;
// // }


// // int PerlevelRangeDeleteFilterByVector::getRangeDeleteCount(){
// //   return PerlevelRangeDeleteFilterByVector::range_delete_list.size();
// // }






// void PLRDF::splitRangesOnLevel(uint level, std::vector<long long> keys){
//   if(rd_filter.size() <= level){
//     std::cerr << "Error: splitRangesOnLevel: level: " << level << " is not present in PLRDF" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//     exit(1);
//   }

//   auto rdList = rd_filter[level];
//   std::vector<pll> rdList_new;
  
//   int idx = 0;
//   int len = rdList.size();

//   for(auto &key_in: keys){
// // std::cout << "key_in: " << key_in << std::endl;
//       while(idx < len && rdList[idx].second <= key_in){
//         rdList_new.push_back(rdList[idx]);
//         idx += 1;
//       }
//       if(idx < len && rdList[idx].first > key_in){
//         continue;
//       }


//       if(idx < len && rdList[idx].first == key_in){
//         if(key_in + 1 >= rdList[idx].second){
//           idx += 1;
//         }else{
//           rdList[idx].first = key_in + 1;
//         }
//         continue;
//       }

//       if(idx < len && rdList[idx].first < key_in && rdList[idx].second  >  key_in){
//         auto tmp = rdList[idx];
//         tmp.second = key_in;
//         rdList_new.push_back(tmp);
//         if(key_in + 1 >= rdList[idx].second){
//           idx += 1;
//         }else{
//           rdList[idx].first = key_in + 1;
//         }
//         continue;
//       }
//   }

//   while(idx < len){
//     rdList_new.push_back(rdList[idx]);
//     idx += 1;
//   }
  
//   std::cout << std::endl;

//   rd_filter[level] = rdList_new;
// }


// void PLRDF::logCurrentTotalNumbersOfRanges(){
//   numbers_of_ranges_in_RDF_log.push_back(getNumberOfTotalRanges());
// }

// std::vector<int> PLRDF::getNumbersOfRangesInRDFLog(){
//   return numbers_of_ranges_in_RDF_log;
// }











// //Self Added --- END PL-RDF ---


// //Self Added --- END PL-RDF ---




















ColumnFamilyHandleImpl::ColumnFamilyHandleImpl(
    ColumnFamilyData* column_family_data, DBImpl* db, InstrumentedMutex* mutex)
    : cfd_(column_family_data), db_(db), mutex_(mutex) {
  if (cfd_ != nullptr) {
    cfd_->Ref();
  }
}

ColumnFamilyHandleImpl::~ColumnFamilyHandleImpl() {
  if (cfd_ != nullptr) {
    for (auto& listener : cfd_->ioptions()->listeners) {
      listener->OnColumnFamilyHandleDeletionStarted(this);
    }
    // Job id == 0 means that this is not our background process, but rather
    // user thread
    // Need to hold some shared pointers owned by the initial_cf_options
    // before final cleaning up finishes.
    ColumnFamilyOptions initial_cf_options_copy = cfd_->initial_cf_options();
    JobContext job_context(0);
    mutex_->Lock();
    bool dropped = cfd_->IsDropped();
    if (cfd_->UnrefAndTryDelete()) {
      if (dropped) {
        db_->FindObsoleteFiles(&job_context, false, true);
      }
    }
    mutex_->Unlock();
    if (job_context.HaveSomethingToDelete()) {
      bool defer_purge =
          db_->immutable_db_options().avoid_unnecessary_blocking_io;
      db_->PurgeObsoleteFiles(job_context, defer_purge);
    }
    job_context.Clean();
  }
}

uint32_t ColumnFamilyHandleImpl::GetID() const { return cfd()->GetID(); }

const std::string& ColumnFamilyHandleImpl::GetName() const {
  return cfd()->GetName();
}

Status ColumnFamilyHandleImpl::GetDescriptor(ColumnFamilyDescriptor* desc) {
  // accessing mutable cf-options requires db mutex.
  InstrumentedMutexLock l(mutex_);
  *desc = ColumnFamilyDescriptor(cfd()->GetName(), cfd()->GetLatestCFOptions());
  return Status::OK();
}

const Comparator* ColumnFamilyHandleImpl::GetComparator() const {
  return cfd()->user_comparator();
}

void GetIntTblPropCollectorFactory(
    const ImmutableCFOptions& ioptions,
    IntTblPropCollectorFactories* int_tbl_prop_collector_factories) {
  assert(int_tbl_prop_collector_factories);

  auto& collector_factories = ioptions.table_properties_collector_factories;
  for (size_t i = 0; i < ioptions.table_properties_collector_factories.size();
       ++i) {
    assert(collector_factories[i]);
    int_tbl_prop_collector_factories->emplace_back(
        new UserKeyTablePropertiesCollectorFactory(collector_factories[i]));
  }
}

Status CheckCompressionSupported(const ColumnFamilyOptions& cf_options) {
  if (!cf_options.compression_per_level.empty()) {
    for (size_t level = 0; level < cf_options.compression_per_level.size();
         ++level) {
      if (!CompressionTypeSupported(cf_options.compression_per_level[level])) {
        return Status::InvalidArgument(
            "Compression type " +
            CompressionTypeToString(cf_options.compression_per_level[level]) +
            " is not linked with the binary.");
      }
    }
  } else {
    if (!CompressionTypeSupported(cf_options.compression)) {
      return Status::InvalidArgument(
          "Compression type " +
          CompressionTypeToString(cf_options.compression) +
          " is not linked with the binary.");
    }
  }
  if (cf_options.compression_opts.zstd_max_train_bytes > 0) {
    if (cf_options.compression_opts.use_zstd_dict_trainer) {
      if (!ZSTD_TrainDictionarySupported()) {
        return Status::InvalidArgument(
            "zstd dictionary trainer cannot be used because ZSTD 1.1.3+ "
            "is not linked with the binary.");
      }
    } else if (!ZSTD_FinalizeDictionarySupported()) {
      return Status::InvalidArgument(
          "zstd finalizeDictionary cannot be used because ZSTD 1.4.5+ "
          "is not linked with the binary.");
    }
    if (cf_options.compression_opts.max_dict_bytes == 0) {
      return Status::InvalidArgument(
          "The dictionary size limit (`CompressionOptions::max_dict_bytes`) "
          "should be nonzero if we're using zstd's dictionary generator.");
    }
  }

  if (!CompressionTypeSupported(cf_options.blob_compression_type)) {
    std::ostringstream oss;
    oss << "The specified blob compression type "
        << CompressionTypeToString(cf_options.blob_compression_type)
        << " is not available.";

    return Status::InvalidArgument(oss.str());
  }

  return Status::OK();
}

Status CheckConcurrentWritesSupported(const ColumnFamilyOptions& cf_options) {
  if (cf_options.inplace_update_support) {
    return Status::InvalidArgument(
        "In-place memtable updates (inplace_update_support) is not compatible "
        "with concurrent writes (allow_concurrent_memtable_write)");
  }
  if (!cf_options.memtable_factory->IsInsertConcurrentlySupported()) {
    return Status::InvalidArgument(
        "Memtable doesn't concurrent writes (allow_concurrent_memtable_write)");
  }
  return Status::OK();
}

Status CheckCFPathsSupported(const DBOptions& db_options,
                             const ColumnFamilyOptions& cf_options) {
  // More than one cf_paths are supported only in universal
  // and level compaction styles. This function also checks the case
  // in which cf_paths is not specified, which results in db_paths
  // being used.
  if ((cf_options.compaction_style != kCompactionStyleUniversal) &&
      (cf_options.compaction_style != kCompactionStyleLevel)) {
    if (cf_options.cf_paths.size() > 1) {
      return Status::NotSupported(
          "More than one CF paths are only supported in "
          "universal and level compaction styles. ");
    } else if (cf_options.cf_paths.empty() && db_options.db_paths.size() > 1) {
      return Status::NotSupported(
          "More than one DB paths are only supported in "
          "universal and level compaction styles. ");
    }
  }
  return Status::OK();
}

namespace {
const uint64_t kDefaultTtl = 0xfffffffffffffffe;
const uint64_t kDefaultPeriodicCompSecs = 0xfffffffffffffffe;
}  // anonymous namespace

ColumnFamilyOptions SanitizeOptions(const ImmutableDBOptions& db_options,
                                    const ColumnFamilyOptions& src) {
  ColumnFamilyOptions result = src;
  size_t clamp_max = std::conditional<
      sizeof(size_t) == 4, std::integral_constant<size_t, 0xffffffff>,
      std::integral_constant<uint64_t, 64ull << 30>>::type::value;
  ClipToRange(&result.write_buffer_size, (static_cast<size_t>(64)) << 10,
              clamp_max);
  // if user sets arena_block_size, we trust user to use this value. Otherwise,
  // calculate a proper value from writer_buffer_size;
  if (result.arena_block_size <= 0) {
    result.arena_block_size =
        std::min(size_t{1024 * 1024}, result.write_buffer_size / 8);

    // Align up to 4k
    const size_t align = 4 * 1024;
    result.arena_block_size =
        ((result.arena_block_size + align - 1) / align) * align;
  }
  result.min_write_buffer_number_to_merge =
      std::min(result.min_write_buffer_number_to_merge,
               result.max_write_buffer_number - 1);
  if (result.min_write_buffer_number_to_merge < 1) {
    result.min_write_buffer_number_to_merge = 1;
  }

  if (db_options.atomic_flush && result.min_write_buffer_number_to_merge > 1) {
    ROCKS_LOG_WARN(
        db_options.logger,
        "Currently, if atomic_flush is true, then triggering flush for any "
        "column family internally (non-manual flush) will trigger flushing "
        "all column families even if the number of memtables is smaller "
        "min_write_buffer_number_to_merge. Therefore, configuring "
        "min_write_buffer_number_to_merge > 1 is not compatible and should "
        "be satinized to 1. Not doing so will lead to data loss and "
        "inconsistent state across multiple column families when WAL is "
        "disabled, which is a common setting for atomic flush");

    result.min_write_buffer_number_to_merge = 1;
  }

  if (result.num_levels < 1) {
    result.num_levels = 1;
  }
  if (result.compaction_style == kCompactionStyleLevel &&
      result.num_levels < 2) {
    result.num_levels = 2;
  }

  if (result.compaction_style == kCompactionStyleUniversal &&
      db_options.allow_ingest_behind && result.num_levels < 3) {
    result.num_levels = 3;
  }

  if (result.max_write_buffer_number < 2) {
    result.max_write_buffer_number = 2;
  }
  // fall back max_write_buffer_number_to_maintain if
  // max_write_buffer_size_to_maintain is not set
  if (result.max_write_buffer_size_to_maintain < 0) {
    result.max_write_buffer_size_to_maintain =
        result.max_write_buffer_number *
        static_cast<int64_t>(result.write_buffer_size);
  } else if (result.max_write_buffer_size_to_maintain == 0 &&
             result.max_write_buffer_number_to_maintain < 0) {
    result.max_write_buffer_number_to_maintain = result.max_write_buffer_number;
  }
  // bloom filter size shouldn't exceed 1/4 of memtable size.
  if (result.memtable_prefix_bloom_size_ratio > 0.25) {
    result.memtable_prefix_bloom_size_ratio = 0.25;
  } else if (result.memtable_prefix_bloom_size_ratio < 0) {
    result.memtable_prefix_bloom_size_ratio = 0;
  }

  if (!result.prefix_extractor) {
    assert(result.memtable_factory);
    Slice name = result.memtable_factory->Name();
    if (name.compare("HashSkipListRepFactory") == 0 ||
        name.compare("HashLinkListRepFactory") == 0) {
      result.memtable_factory = std::make_shared<SkipListFactory>();
    }
  }

  if (result.compaction_style == kCompactionStyleFIFO) {
    // since we delete level0 files in FIFO compaction when there are too many
    // of them, these options don't really mean anything
    result.level0_slowdown_writes_trigger = std::numeric_limits<int>::max();
    result.level0_stop_writes_trigger = std::numeric_limits<int>::max();
  }

  if (result.max_bytes_for_level_multiplier <= 0) {
    result.max_bytes_for_level_multiplier = 1;
  }

  if (result.level0_file_num_compaction_trigger == 0) {
    ROCKS_LOG_WARN(db_options.logger,
                   "level0_file_num_compaction_trigger cannot be 0");
    result.level0_file_num_compaction_trigger = 1;
  }

  if (result.level0_stop_writes_trigger <
          result.level0_slowdown_writes_trigger ||
      result.level0_slowdown_writes_trigger <
          result.level0_file_num_compaction_trigger) {
    ROCKS_LOG_WARN(db_options.logger,
                   "This condition must be satisfied: "
                   "level0_stop_writes_trigger(%d) >= "
                   "level0_slowdown_writes_trigger(%d) >= "
                   "level0_file_num_compaction_trigger(%d)",
                   result.level0_stop_writes_trigger,
                   result.level0_slowdown_writes_trigger,
                   result.level0_file_num_compaction_trigger);
    if (result.level0_slowdown_writes_trigger <
        result.level0_file_num_compaction_trigger) {
      result.level0_slowdown_writes_trigger =
          result.level0_file_num_compaction_trigger;
    }
    if (result.level0_stop_writes_trigger <
        result.level0_slowdown_writes_trigger) {
      result.level0_stop_writes_trigger = result.level0_slowdown_writes_trigger;
    }
    ROCKS_LOG_WARN(db_options.logger,
                   "Adjust the value to "
                   "level0_stop_writes_trigger(%d)"
                   "level0_slowdown_writes_trigger(%d)"
                   "level0_file_num_compaction_trigger(%d)",
                   result.level0_stop_writes_trigger,
                   result.level0_slowdown_writes_trigger,
                   result.level0_file_num_compaction_trigger);
  }

  if (result.soft_pending_compaction_bytes_limit == 0) {
    result.soft_pending_compaction_bytes_limit =
        result.hard_pending_compaction_bytes_limit;
  } else if (result.hard_pending_compaction_bytes_limit > 0 &&
             result.soft_pending_compaction_bytes_limit >
                 result.hard_pending_compaction_bytes_limit) {
    result.soft_pending_compaction_bytes_limit =
        result.hard_pending_compaction_bytes_limit;
  }

  // When the DB is stopped, it's possible that there are some .trash files that
  // were not deleted yet, when we open the DB we will find these .trash files
  // and schedule them to be deleted (or delete immediately if SstFileManager
  // was not used)
  auto sfm =
      static_cast<SstFileManagerImpl*>(db_options.sst_file_manager.get());
  for (size_t i = 0; i < result.cf_paths.size(); i++) {
    DeleteScheduler::CleanupDirectory(db_options.env, sfm,
                                      result.cf_paths[i].path)
        .PermitUncheckedError();
  }

  if (result.cf_paths.empty()) {
    result.cf_paths = db_options.db_paths;
  }

  if (result.level_compaction_dynamic_level_bytes) {
    if (result.compaction_style != kCompactionStyleLevel) {
      ROCKS_LOG_WARN(db_options.info_log.get(),
                     "level_compaction_dynamic_level_bytes only makes sense"
                     "for level-based compaction");
      result.level_compaction_dynamic_level_bytes = false;
    } else if (result.cf_paths.size() > 1U) {
      // we don't yet know how to make both of this feature and multiple
      // DB path work.
      ROCKS_LOG_WARN(db_options.info_log.get(),
                     "multiple cf_paths/db_paths and"
                     "level_compaction_dynamic_level_bytes"
                     "can't be used together");
      result.level_compaction_dynamic_level_bytes = false;
    }
  }

  if (result.max_compaction_bytes == 0) {
    result.max_compaction_bytes = result.target_file_size_base * 25;
  }

  bool is_block_based_table = (result.table_factory->IsInstanceOf(
      TableFactory::kBlockBasedTableName()));

  const uint64_t kAdjustedTtl = 30 * 24 * 60 * 60;
  if (result.ttl == kDefaultTtl) {
    if (is_block_based_table &&
        result.compaction_style != kCompactionStyleFIFO) {
      result.ttl = kAdjustedTtl;
    } else {
      result.ttl = 0;
    }
  }

  const uint64_t kAdjustedPeriodicCompSecs = 30 * 24 * 60 * 60;

  // Turn on periodic compactions and set them to occur once every 30 days if
  // compaction filters are used and periodic_compaction_seconds is set to the
  // default value.
  if (result.compaction_style != kCompactionStyleFIFO) {
    if ((result.compaction_filter != nullptr ||
         result.compaction_filter_factory != nullptr) &&
        result.periodic_compaction_seconds == kDefaultPeriodicCompSecs &&
        is_block_based_table) {
      result.periodic_compaction_seconds = kAdjustedPeriodicCompSecs;
    }
  } else {
    // result.compaction_style == kCompactionStyleFIFO
    if (result.ttl == 0) {
      if (is_block_based_table) {
        if (result.periodic_compaction_seconds == kDefaultPeriodicCompSecs) {
          result.periodic_compaction_seconds = kAdjustedPeriodicCompSecs;
        }
        result.ttl = result.periodic_compaction_seconds;
      }
    } else if (result.periodic_compaction_seconds != 0) {
      result.ttl = std::min(result.ttl, result.periodic_compaction_seconds);
    }
  }

  // TTL compactions would work similar to Periodic Compactions in Universal in
  // most of the cases. So, if ttl is set, execute the periodic compaction
  // codepath.
  if (result.compaction_style == kCompactionStyleUniversal && result.ttl != 0) {
    if (result.periodic_compaction_seconds != 0) {
      result.periodic_compaction_seconds =
          std::min(result.ttl, result.periodic_compaction_seconds);
    } else {
      result.periodic_compaction_seconds = result.ttl;
    }
  }

  if (result.periodic_compaction_seconds == kDefaultPeriodicCompSecs) {
    result.periodic_compaction_seconds = 0;
  }

  return result;
}

int SuperVersion::dummy = 0;
void* const SuperVersion::kSVInUse = &SuperVersion::dummy;
void* const SuperVersion::kSVObsolete = nullptr;

SuperVersion::~SuperVersion() {
  for (auto td : to_delete) {
    delete td;
  }
}

SuperVersion* SuperVersion::Ref() {
  refs.fetch_add(1, std::memory_order_relaxed);
  return this;
}

bool SuperVersion::Unref() {
  // fetch_sub returns the previous value of ref
  uint32_t previous_refs = refs.fetch_sub(1);
  assert(previous_refs > 0);
  return previous_refs == 1;
}

void SuperVersion::Cleanup() {
  assert(refs.load(std::memory_order_relaxed) == 0);
  // Since this SuperVersion object is being deleted,
  // decrement reference to the immutable MemtableList
  // this SV object was pointing to.
  imm->Unref(&to_delete);
  MemTable* m = mem->Unref();
  if (m != nullptr) {
    auto* memory_usage = current->cfd()->imm()->current_memory_usage();
    assert(*memory_usage >= m->ApproximateMemoryUsage());
    *memory_usage -= m->ApproximateMemoryUsage();
    to_delete.push_back(m);
  }
  current->Unref();
  cfd->UnrefAndTryDelete();
}

void SuperVersion::Init(ColumnFamilyData* new_cfd, MemTable* new_mem,
                        MemTableListVersion* new_imm, Version* new_current) {
  cfd = new_cfd;
  mem = new_mem;
  imm = new_imm;
  current = new_current;
  cfd->Ref();
  mem->Ref();
  imm->Ref();
  current->Ref();
  refs.store(1, std::memory_order_relaxed);
}

namespace {
void SuperVersionUnrefHandle(void* ptr) {
  // UnrefHandle is called when a thread exits or a ThreadLocalPtr gets
  // destroyed. When the former happens, the thread shouldn't see kSVInUse.
  // When the latter happens, only super_version_ holds a reference
  // to ColumnFamilyData, so no further queries are possible.
  SuperVersion* sv = static_cast<SuperVersion*>(ptr);
  bool was_last_ref __attribute__((__unused__));
  was_last_ref = sv->Unref();
  // Thread-local SuperVersions can't outlive ColumnFamilyData::super_version_.
  // This is important because we can't do SuperVersion cleanup here.
  // That would require locking DB mutex, which would deadlock because
  // SuperVersionUnrefHandle is called with locked ThreadLocalPtr mutex.
  assert(!was_last_ref);
}
}  // anonymous namespace

std::vector<std::string> ColumnFamilyData::GetDbPaths() const {
  std::vector<std::string> paths;
  paths.reserve(ioptions_.cf_paths.size());
  for (const DbPath& db_path : ioptions_.cf_paths) {
    paths.emplace_back(db_path.path);
  }
  return paths;
}

const uint32_t ColumnFamilyData::kDummyColumnFamilyDataId =
    std::numeric_limits<uint32_t>::max();

ColumnFamilyData::ColumnFamilyData(
    uint32_t id, const std::string& name, Version* _dummy_versions,
    Cache* _table_cache, WriteBufferManager* write_buffer_manager,
    const ColumnFamilyOptions& cf_options, const ImmutableDBOptions& db_options,
    const FileOptions* file_options, ColumnFamilySet* column_family_set,
    BlockCacheTracer* const block_cache_tracer,
    const std::shared_ptr<IOTracer>& io_tracer, const std::string& db_id,
    const std::string& db_session_id)
    : id_(id),
      name_(name),
      dummy_versions_(_dummy_versions),
      current_(nullptr),
      refs_(0),
      initialized_(false),
      dropped_(false),
      internal_comparator_(cf_options.comparator),
      initial_cf_options_(SanitizeOptions(db_options, cf_options)),
      ioptions_(db_options, initial_cf_options_),
      mutable_cf_options_(initial_cf_options_),
      is_delete_range_supported_(
          cf_options.table_factory->IsDeleteRangeSupported()),
      write_buffer_manager_(write_buffer_manager),
      mem_(nullptr),
      imm_(ioptions_.min_write_buffer_number_to_merge,
           ioptions_.max_write_buffer_number_to_maintain,
           ioptions_.max_write_buffer_size_to_maintain),
      super_version_(nullptr),
      super_version_number_(0),
      local_sv_(new ThreadLocalPtr(&SuperVersionUnrefHandle)),
      next_(nullptr),
      prev_(nullptr),
      log_number_(0),
      column_family_set_(column_family_set),
      queued_for_flush_(false),
      queued_for_compaction_(false),
      prev_compaction_needed_bytes_(0),
      allow_2pc_(db_options.allow_2pc),
      last_memtable_id_(0),
      db_paths_registered_(false),
      mempurge_used_(false),
      next_epoch_number_(1) {
  if (id_ != kDummyColumnFamilyDataId) {
    // TODO(cc): RegisterDbPaths can be expensive, considering moving it
    // outside of this constructor which might be called with db mutex held.
    // TODO(cc): considering using ioptions_.fs, currently some tests rely on
    // EnvWrapper, that's the main reason why we use env here.
    Status s = ioptions_.env->RegisterDbPaths(GetDbPaths());
    if (s.ok()) {
      db_paths_registered_ = true;
    } else {
      ROCKS_LOG_ERROR(
          ioptions_.logger,
          "Failed to register data paths of column family (id: %d, name: %s)",
          id_, name_.c_str());
    }
  }
  Ref();

  // Convert user defined table properties collector factories to internal ones.
  GetIntTblPropCollectorFactory(ioptions_, &int_tbl_prop_collector_factories_);

  // if _dummy_versions is nullptr, then this is a dummy column family.
  if (_dummy_versions != nullptr) {
    internal_stats_.reset(
        new InternalStats(ioptions_.num_levels, ioptions_.clock, this));
    table_cache_.reset(new TableCache(ioptions_, file_options, _table_cache,
                                      block_cache_tracer, io_tracer,
                                      db_session_id));
    blob_file_cache_.reset(
        new BlobFileCache(_table_cache, ioptions(), soptions(), id_,
                          internal_stats_->GetBlobFileReadHist(), io_tracer));
    blob_source_.reset(new BlobSource(ioptions(), db_id, db_session_id,
                                      blob_file_cache_.get()));

    if (ioptions_.compaction_style == kCompactionStyleLevel) {
      compaction_picker_.reset(
          new LevelCompactionPicker(ioptions_, &internal_comparator_));
    } else if (ioptions_.compaction_style == kCompactionStyleUniversal) {
      compaction_picker_.reset(
          new UniversalCompactionPicker(ioptions_, &internal_comparator_));
    } else if (ioptions_.compaction_style == kCompactionStyleFIFO) {
      compaction_picker_.reset(
          new FIFOCompactionPicker(ioptions_, &internal_comparator_));
    } else if (ioptions_.compaction_style == kCompactionStyleNone) {
      compaction_picker_.reset(
          new NullCompactionPicker(ioptions_, &internal_comparator_));
      ROCKS_LOG_WARN(ioptions_.logger,
                     "Column family %s does not use any background compaction. "
                     "Compactions can only be done via CompactFiles\n",
                     GetName().c_str());
    } else {
      ROCKS_LOG_ERROR(ioptions_.logger,
                      "Unable to recognize the specified compaction style %d. "
                      "Column family %s will use kCompactionStyleLevel.\n",
                      ioptions_.compaction_style, GetName().c_str());
      compaction_picker_.reset(
          new LevelCompactionPicker(ioptions_, &internal_comparator_));
    }

    if (column_family_set_->NumberOfColumnFamilies() < 10) {
      ROCKS_LOG_INFO(ioptions_.logger,
                     "--------------- Options for column family [%s]:\n",
                     name.c_str());
      initial_cf_options_.Dump(ioptions_.logger);
    } else {
      ROCKS_LOG_INFO(ioptions_.logger, "\t(skipping printing options)\n");
    }
  }

  RecalculateWriteStallConditions(mutable_cf_options_);

  if (cf_options.table_factory->IsInstanceOf(
          TableFactory::kBlockBasedTableName()) &&
      cf_options.table_factory->GetOptions<BlockBasedTableOptions>()) {
    const BlockBasedTableOptions* bbto =
        cf_options.table_factory->GetOptions<BlockBasedTableOptions>();
    const auto& options_overrides = bbto->cache_usage_options.options_overrides;
    const auto file_metadata_charged =
        options_overrides.at(CacheEntryRole::kFileMetadata).charged;
    if (bbto->block_cache &&
        file_metadata_charged == CacheEntryRoleOptions::Decision::kEnabled) {
      // TODO(hx235): Add a `ConcurrentCacheReservationManager` at DB scope
      // responsible for reservation of `ObsoleteFileInfo` so that we can keep
      // this `file_metadata_cache_res_mgr_` nonconcurrent
      file_metadata_cache_res_mgr_.reset(new ConcurrentCacheReservationManager(
          std::make_shared<
              CacheReservationManagerImpl<CacheEntryRole::kFileMetadata>>(
              bbto->block_cache)));
    }
  }
}

// DB mutex held
ColumnFamilyData::~ColumnFamilyData() {
  assert(refs_.load(std::memory_order_relaxed) == 0);
  // remove from linked list
  auto prev = prev_;
  auto next = next_;
  prev->next_ = next;
  next->prev_ = prev;

  if (!dropped_ && column_family_set_ != nullptr) {
    // If it's dropped, it's already removed from column family set
    // If column_family_set_ == nullptr, this is dummy CFD and not in
    // ColumnFamilySet
    column_family_set_->RemoveColumnFamily(this);
  }

  if (current_ != nullptr) {
    current_->Unref();
  }

  // It would be wrong if this ColumnFamilyData is in flush_queue_ or
  // compaction_queue_ and we destroyed it
  assert(!queued_for_flush_);
  assert(!queued_for_compaction_);
  assert(super_version_ == nullptr);

  if (dummy_versions_ != nullptr) {
    // List must be empty
    assert(dummy_versions_->Next() == dummy_versions_);
    bool deleted __attribute__((__unused__));
    deleted = dummy_versions_->Unref();
    assert(deleted);
  }

  if (mem_ != nullptr) {
    delete mem_->Unref();
  }
  autovector<MemTable*> to_delete;
  imm_.current()->Unref(&to_delete);
  for (MemTable* m : to_delete) {
    delete m;
  }

  if (db_paths_registered_) {
    // TODO(cc): considering using ioptions_.fs, currently some tests rely on
    // EnvWrapper, that's the main reason why we use env here.
    Status s = ioptions_.env->UnregisterDbPaths(GetDbPaths());
    if (!s.ok()) {
      ROCKS_LOG_ERROR(
          ioptions_.logger,
          "Failed to unregister data paths of column family (id: %d, name: %s)",
          id_, name_.c_str());
    }
  }
}

bool ColumnFamilyData::UnrefAndTryDelete() {
  int old_refs = refs_.fetch_sub(1);
  assert(old_refs > 0);

  if (old_refs == 1) {
    assert(super_version_ == nullptr);
    delete this;
    return true;
  }

  if (old_refs == 2 && super_version_ != nullptr) {
    // Only the super_version_ holds me
    SuperVersion* sv = super_version_;
    super_version_ = nullptr;

    // Release SuperVersion references kept in ThreadLocalPtr.
    local_sv_.reset();

    if (sv->Unref()) {
      // Note: sv will delete this ColumnFamilyData during Cleanup()
      assert(sv->cfd == this);
      sv->Cleanup();
      delete sv;
      return true;
    }
  }
  return false;
}

void ColumnFamilyData::SetDropped() {
  // can't drop default CF
  assert(id_ != 0);
  dropped_ = true;
  write_controller_token_.reset();

  // remove from column_family_set
  column_family_set_->RemoveColumnFamily(this);
}

ColumnFamilyOptions ColumnFamilyData::GetLatestCFOptions() const {
  return BuildColumnFamilyOptions(initial_cf_options_, mutable_cf_options_);
}

uint64_t ColumnFamilyData::OldestLogToKeep() {
  auto current_log = GetLogNumber();

  if (allow_2pc_) {
    auto imm_prep_log = imm()->PrecomputeMinLogContainingPrepSection();
    auto mem_prep_log = mem()->GetMinLogContainingPrepSection();

    if (imm_prep_log > 0 && imm_prep_log < current_log) {
      current_log = imm_prep_log;
    }

    if (mem_prep_log > 0 && mem_prep_log < current_log) {
      current_log = mem_prep_log;
    }
  }

  return current_log;
}

const double kIncSlowdownRatio = 0.8;
const double kDecSlowdownRatio = 1 / kIncSlowdownRatio;
const double kNearStopSlowdownRatio = 0.6;
const double kDelayRecoverSlowdownRatio = 1.4;

namespace {
// If penalize_stop is true, we further reduce slowdown rate.
std::unique_ptr<WriteControllerToken> SetupDelay(
    WriteController* write_controller, uint64_t compaction_needed_bytes,
    uint64_t prev_compaction_need_bytes, bool penalize_stop,
    bool auto_compactions_disabled) {
  const uint64_t kMinWriteRate = 16 * 1024u;  // Minimum write rate 16KB/s.

  uint64_t max_write_rate = write_controller->max_delayed_write_rate();
  uint64_t write_rate = write_controller->delayed_write_rate();

  if (auto_compactions_disabled) {
    // When auto compaction is disabled, always use the value user gave.
    write_rate = max_write_rate;
  } else if (write_controller->NeedsDelay() && max_write_rate > kMinWriteRate) {
    // If user gives rate less than kMinWriteRate, don't adjust it.
    //
    // If already delayed, need to adjust based on previous compaction debt.
    // When there are two or more column families require delay, we always
    // increase or reduce write rate based on information for one single
    // column family. It is likely to be OK but we can improve if there is a
    // problem.
    // Ignore compaction_needed_bytes = 0 case because compaction_needed_bytes
    // is only available in level-based compaction
    //
    // If the compaction debt stays the same as previously, we also further slow
    // down. It usually means a mem table is full. It's mainly for the case
    // where both of flush and compaction are much slower than the speed we
    // insert to mem tables, so we need to actively slow down before we get
    // feedback signal from compaction and flushes to avoid the full stop
    // because of hitting the max write buffer number.
    //
    // If DB just falled into the stop condition, we need to further reduce
    // the write rate to avoid the stop condition.
    if (penalize_stop) {
      // Penalize the near stop or stop condition by more aggressive slowdown.
      // This is to provide the long term slowdown increase signal.
      // The penalty is more than the reward of recovering to the normal
      // condition.
      write_rate = static_cast<uint64_t>(static_cast<double>(write_rate) *
                                         kNearStopSlowdownRatio);
      if (write_rate < kMinWriteRate) {
        write_rate = kMinWriteRate;
      }
    } else if (prev_compaction_need_bytes > 0 &&
               prev_compaction_need_bytes <= compaction_needed_bytes) {
      write_rate = static_cast<uint64_t>(static_cast<double>(write_rate) *
                                         kIncSlowdownRatio);
      if (write_rate < kMinWriteRate) {
        write_rate = kMinWriteRate;
      }
    } else if (prev_compaction_need_bytes > compaction_needed_bytes) {
      // We are speeding up by ratio of kSlowdownRatio when we have paid
      // compaction debt. But we'll never speed up to faster than the write rate
      // given by users.
      write_rate = static_cast<uint64_t>(static_cast<double>(write_rate) *
                                         kDecSlowdownRatio);
      if (write_rate > max_write_rate) {
        write_rate = max_write_rate;
      }
    }
  }
  return write_controller->GetDelayToken(write_rate);
}

int GetL0ThresholdSpeedupCompaction(int level0_file_num_compaction_trigger,
                                    int level0_slowdown_writes_trigger) {
  // SanitizeOptions() ensures it.
  assert(level0_file_num_compaction_trigger <= level0_slowdown_writes_trigger);

  if (level0_file_num_compaction_trigger < 0) {
    return std::numeric_limits<int>::max();
  }

  const int64_t twice_level0_trigger =
      static_cast<int64_t>(level0_file_num_compaction_trigger) * 2;

  const int64_t one_fourth_trigger_slowdown =
      static_cast<int64_t>(level0_file_num_compaction_trigger) +
      ((level0_slowdown_writes_trigger - level0_file_num_compaction_trigger) /
       4);

  assert(twice_level0_trigger >= 0);
  assert(one_fourth_trigger_slowdown >= 0);

  // 1/4 of the way between L0 compaction trigger threshold and slowdown
  // condition.
  // Or twice as compaction trigger, if it is smaller.
  int64_t res = std::min(twice_level0_trigger, one_fourth_trigger_slowdown);
  if (res >= std::numeric_limits<int32_t>::max()) {
    return std::numeric_limits<int32_t>::max();
  } else {
    // res fits in int
    return static_cast<int>(res);
  }
}
}  // anonymous namespace

std::pair<WriteStallCondition, WriteStallCause>
ColumnFamilyData::GetWriteStallConditionAndCause(
    int num_unflushed_memtables, int num_l0_files,
    uint64_t num_compaction_needed_bytes,
    const MutableCFOptions& mutable_cf_options,
    const ImmutableCFOptions& immutable_cf_options) {
  if (num_unflushed_memtables >= mutable_cf_options.max_write_buffer_number) {
    return {WriteStallCondition::kStopped, WriteStallCause::kMemtableLimit};
  } else if (!mutable_cf_options.disable_auto_compactions &&
             num_l0_files >= mutable_cf_options.level0_stop_writes_trigger) {
    return {WriteStallCondition::kStopped, WriteStallCause::kL0FileCountLimit};
  } else if (!mutable_cf_options.disable_auto_compactions &&
             mutable_cf_options.hard_pending_compaction_bytes_limit > 0 &&
             num_compaction_needed_bytes >=
                 mutable_cf_options.hard_pending_compaction_bytes_limit) {
    return {WriteStallCondition::kStopped,
            WriteStallCause::kPendingCompactionBytes};
  } else if (mutable_cf_options.max_write_buffer_number > 3 &&
             num_unflushed_memtables >=
                 mutable_cf_options.max_write_buffer_number - 1 &&
             num_unflushed_memtables - 1 >=
                 immutable_cf_options.min_write_buffer_number_to_merge) {
    return {WriteStallCondition::kDelayed, WriteStallCause::kMemtableLimit};
  } else if (!mutable_cf_options.disable_auto_compactions &&
             mutable_cf_options.level0_slowdown_writes_trigger >= 0 &&
             num_l0_files >=
                 mutable_cf_options.level0_slowdown_writes_trigger) {
    return {WriteStallCondition::kDelayed, WriteStallCause::kL0FileCountLimit};
  } else if (!mutable_cf_options.disable_auto_compactions &&
             mutable_cf_options.soft_pending_compaction_bytes_limit > 0 &&
             num_compaction_needed_bytes >=
                 mutable_cf_options.soft_pending_compaction_bytes_limit) {
    return {WriteStallCondition::kDelayed,
            WriteStallCause::kPendingCompactionBytes};
  }
  return {WriteStallCondition::kNormal, WriteStallCause::kNone};
}

WriteStallCondition ColumnFamilyData::RecalculateWriteStallConditions(
    const MutableCFOptions& mutable_cf_options) {
  auto write_stall_condition = WriteStallCondition::kNormal;
  if (current_ != nullptr) {
    auto* vstorage = current_->storage_info();
    auto write_controller = column_family_set_->write_controller_;
    uint64_t compaction_needed_bytes =
        vstorage->estimated_compaction_needed_bytes();

    auto write_stall_condition_and_cause = GetWriteStallConditionAndCause(
        imm()->NumNotFlushed(), vstorage->l0_delay_trigger_count(),
        vstorage->estimated_compaction_needed_bytes(), mutable_cf_options,
        *ioptions());
    write_stall_condition = write_stall_condition_and_cause.first;
    auto write_stall_cause = write_stall_condition_and_cause.second;

    bool was_stopped = write_controller->IsStopped();
    bool needed_delay = write_controller->NeedsDelay();

    if (write_stall_condition == WriteStallCondition::kStopped &&
        write_stall_cause == WriteStallCause::kMemtableLimit) {
      write_controller_token_ = write_controller->GetStopToken();
      internal_stats_->AddCFStats(InternalStats::MEMTABLE_LIMIT_STOPS, 1);
      ROCKS_LOG_WARN(
          ioptions_.logger,
          "[%s] Stopping writes because we have %d immutable memtables "
          "(waiting for flush), max_write_buffer_number is set to %d",
          name_.c_str(), imm()->NumNotFlushed(),
          mutable_cf_options.max_write_buffer_number);
    } else if (write_stall_condition == WriteStallCondition::kStopped &&
               write_stall_cause == WriteStallCause::kL0FileCountLimit) {
      write_controller_token_ = write_controller->GetStopToken();
      internal_stats_->AddCFStats(InternalStats::L0_FILE_COUNT_LIMIT_STOPS, 1);
      if (compaction_picker_->IsLevel0CompactionInProgress()) {
        internal_stats_->AddCFStats(
            InternalStats::L0_FILE_COUNT_LIMIT_STOPS_WITH_ONGOING_COMPACTION,
            1);
      }
      ROCKS_LOG_WARN(ioptions_.logger,
                     "[%s] Stopping writes because we have %d level-0 files",
                     name_.c_str(), vstorage->l0_delay_trigger_count());
    } else if (write_stall_condition == WriteStallCondition::kStopped &&
               write_stall_cause == WriteStallCause::kPendingCompactionBytes) {
      write_controller_token_ = write_controller->GetStopToken();
      internal_stats_->AddCFStats(
          InternalStats::PENDING_COMPACTION_BYTES_LIMIT_STOPS, 1);
      ROCKS_LOG_WARN(
          ioptions_.logger,
          "[%s] Stopping writes because of estimated pending compaction "
          "bytes %" PRIu64,
          name_.c_str(), compaction_needed_bytes);
    } else if (write_stall_condition == WriteStallCondition::kDelayed &&
               write_stall_cause == WriteStallCause::kMemtableLimit) {
      write_controller_token_ =
          SetupDelay(write_controller, compaction_needed_bytes,
                     prev_compaction_needed_bytes_, was_stopped,
                     mutable_cf_options.disable_auto_compactions);
      internal_stats_->AddCFStats(InternalStats::MEMTABLE_LIMIT_DELAYS, 1);
      ROCKS_LOG_WARN(
          ioptions_.logger,
          "[%s] Stalling writes because we have %d immutable memtables "
          "(waiting for flush), max_write_buffer_number is set to %d "
          "rate %" PRIu64,
          name_.c_str(), imm()->NumNotFlushed(),
          mutable_cf_options.max_write_buffer_number,
          write_controller->delayed_write_rate());
    } else if (write_stall_condition == WriteStallCondition::kDelayed &&
               write_stall_cause == WriteStallCause::kL0FileCountLimit) {
      // L0 is the last two files from stopping.
      bool near_stop = vstorage->l0_delay_trigger_count() >=
                       mutable_cf_options.level0_stop_writes_trigger - 2;
      write_controller_token_ =
          SetupDelay(write_controller, compaction_needed_bytes,
                     prev_compaction_needed_bytes_, was_stopped || near_stop,
                     mutable_cf_options.disable_auto_compactions);
      internal_stats_->AddCFStats(InternalStats::L0_FILE_COUNT_LIMIT_DELAYS, 1);
      if (compaction_picker_->IsLevel0CompactionInProgress()) {
        internal_stats_->AddCFStats(
            InternalStats::L0_FILE_COUNT_LIMIT_DELAYS_WITH_ONGOING_COMPACTION,
            1);
      }
      ROCKS_LOG_WARN(ioptions_.logger,
                     "[%s] Stalling writes because we have %d level-0 files "
                     "rate %" PRIu64,
                     name_.c_str(), vstorage->l0_delay_trigger_count(),
                     write_controller->delayed_write_rate());
    } else if (write_stall_condition == WriteStallCondition::kDelayed &&
               write_stall_cause == WriteStallCause::kPendingCompactionBytes) {
      // If the distance to hard limit is less than 1/4 of the gap between soft
      // and
      // hard bytes limit, we think it is near stop and speed up the slowdown.
      bool near_stop =
          mutable_cf_options.hard_pending_compaction_bytes_limit > 0 &&
          (compaction_needed_bytes -
           mutable_cf_options.soft_pending_compaction_bytes_limit) >
              3 *
                  (mutable_cf_options.hard_pending_compaction_bytes_limit -
                   mutable_cf_options.soft_pending_compaction_bytes_limit) /
                  4;

      write_controller_token_ =
          SetupDelay(write_controller, compaction_needed_bytes,
                     prev_compaction_needed_bytes_, was_stopped || near_stop,
                     mutable_cf_options.disable_auto_compactions);
      internal_stats_->AddCFStats(
          InternalStats::PENDING_COMPACTION_BYTES_LIMIT_DELAYS, 1);
      ROCKS_LOG_WARN(
          ioptions_.logger,
          "[%s] Stalling writes because of estimated pending compaction "
          "bytes %" PRIu64 " rate %" PRIu64,
          name_.c_str(), vstorage->estimated_compaction_needed_bytes(),
          write_controller->delayed_write_rate());
    } else {
      assert(write_stall_condition == WriteStallCondition::kNormal);
      if (vstorage->l0_delay_trigger_count() >=
          GetL0ThresholdSpeedupCompaction(
              mutable_cf_options.level0_file_num_compaction_trigger,
              mutable_cf_options.level0_slowdown_writes_trigger)) {
        write_controller_token_ =
            write_controller->GetCompactionPressureToken();
        ROCKS_LOG_INFO(
            ioptions_.logger,
            "[%s] Increasing compaction threads because we have %d level-0 "
            "files ",
            name_.c_str(), vstorage->l0_delay_trigger_count());
      } else if (vstorage->estimated_compaction_needed_bytes() >=
                 mutable_cf_options.soft_pending_compaction_bytes_limit / 4) {
        // Increase compaction threads if bytes needed for compaction exceeds
        // 1/4 of threshold for slowing down.
        // If soft pending compaction byte limit is not set, always speed up
        // compaction.
        write_controller_token_ =
            write_controller->GetCompactionPressureToken();
        if (mutable_cf_options.soft_pending_compaction_bytes_limit > 0) {
          ROCKS_LOG_INFO(
              ioptions_.logger,
              "[%s] Increasing compaction threads because of estimated pending "
              "compaction "
              "bytes %" PRIu64,
              name_.c_str(), vstorage->estimated_compaction_needed_bytes());
        }
      } else {
        write_controller_token_.reset();
      }
      // If the DB recovers from delay conditions, we reward with reducing
      // double the slowdown ratio. This is to balance the long term slowdown
      // increase signal.
      if (needed_delay) {
        uint64_t write_rate = write_controller->delayed_write_rate();
        write_controller->set_delayed_write_rate(static_cast<uint64_t>(
            static_cast<double>(write_rate) * kDelayRecoverSlowdownRatio));
        // Set the low pri limit to be 1/4 the delayed write rate.
        // Note we don't reset this value even after delay condition is relased.
        // Low-pri rate will continue to apply if there is a compaction
        // pressure.
        write_controller->low_pri_rate_limiter()->SetBytesPerSecond(write_rate /
                                                                    4);
      }
    }
    prev_compaction_needed_bytes_ = compaction_needed_bytes;
  }
  return write_stall_condition;
}

const FileOptions* ColumnFamilyData::soptions() const {
  return &(column_family_set_->file_options_);
}

void ColumnFamilyData::SetCurrent(Version* current_version) {
  current_ = current_version;
}

uint64_t ColumnFamilyData::GetNumLiveVersions() const {
  return VersionSet::GetNumLiveVersions(dummy_versions_);
}

uint64_t ColumnFamilyData::GetTotalSstFilesSize() const {
  return VersionSet::GetTotalSstFilesSize(dummy_versions_);
}

uint64_t ColumnFamilyData::GetTotalBlobFileSize() const {
  return VersionSet::GetTotalBlobFileSize(dummy_versions_);
}

uint64_t ColumnFamilyData::GetLiveSstFilesSize() const {
  return current_->GetSstFilesSize();
}

MemTable* ColumnFamilyData::ConstructNewMemtable(
    const MutableCFOptions& mutable_cf_options, SequenceNumber earliest_seq) {
  return new MemTable(internal_comparator_, ioptions_, mutable_cf_options,
                      write_buffer_manager_, earliest_seq, id_);
}

void ColumnFamilyData::CreateNewMemtable(
    const MutableCFOptions& mutable_cf_options, SequenceNumber earliest_seq) {
  if (mem_ != nullptr) {
    delete mem_->Unref();
  }
  SetMemtable(ConstructNewMemtable(mutable_cf_options, earliest_seq));
  mem_->Ref();
}

bool ColumnFamilyData::NeedsCompaction() const {
  return !mutable_cf_options_.disable_auto_compactions &&
         compaction_picker_->NeedsCompaction(current_->storage_info());
}

Compaction* ColumnFamilyData::PickCompaction(
    const MutableCFOptions& mutable_options,
    const MutableDBOptions& mutable_db_options, LogBuffer* log_buffer) {
  auto* result = compaction_picker_->PickCompaction(
      GetName(), mutable_options, mutable_db_options, current_->storage_info(),
      log_buffer);
  if (result != nullptr) {
    result->SetInputVersion(current_);
  }
  return result;
}

bool ColumnFamilyData::RangeOverlapWithCompaction(
    const Slice& smallest_user_key, const Slice& largest_user_key,
    int level) const {
  return compaction_picker_->RangeOverlapWithCompaction(
      smallest_user_key, largest_user_key, level);
}

Status ColumnFamilyData::RangesOverlapWithMemtables(
    const autovector<Range>& ranges, SuperVersion* super_version,
    bool allow_data_in_errors, bool* overlap) {
  assert(overlap != nullptr);
  *overlap = false;
  // Create an InternalIterator over all unflushed memtables
  Arena arena;
  // TODO: plumb Env::IOActivity
  ReadOptions read_opts;
  read_opts.total_order_seek = true;
  MergeIteratorBuilder merge_iter_builder(&internal_comparator_, &arena);
  merge_iter_builder.AddIterator(
      super_version->mem->NewIterator(read_opts, &arena));
  super_version->imm->AddIterators(read_opts, &merge_iter_builder,
                                   false /* add_range_tombstone_iter */);
  ScopedArenaIterator memtable_iter(merge_iter_builder.Finish());

  auto read_seq = super_version->current->version_set()->LastSequence();
  ReadRangeDelAggregator range_del_agg(&internal_comparator_, read_seq);
  auto* active_range_del_iter = super_version->mem->NewRangeTombstoneIterator(
      read_opts, read_seq, false /* immutable_memtable */);
  range_del_agg.AddTombstones(
      std::unique_ptr<FragmentedRangeTombstoneIterator>(active_range_del_iter));
  Status status;
  status = super_version->imm->AddRangeTombstoneIterators(
      read_opts, nullptr /* arena */, &range_del_agg);
  // AddRangeTombstoneIterators always return Status::OK.
  assert(status.ok());

  for (size_t i = 0; i < ranges.size() && status.ok() && !*overlap; ++i) {
    auto* vstorage = super_version->current->storage_info();
    auto* ucmp = vstorage->InternalComparator()->user_comparator();
    InternalKey range_start(ranges[i].start, kMaxSequenceNumber,
                            kValueTypeForSeek);
    memtable_iter->Seek(range_start.Encode());
    status = memtable_iter->status();
    ParsedInternalKey seek_result;

    if (status.ok() && memtable_iter->Valid()) {
      status = ParseInternalKey(memtable_iter->key(), &seek_result,
                                allow_data_in_errors);
    }

    if (status.ok()) {
      if (memtable_iter->Valid() &&
          ucmp->Compare(seek_result.user_key, ranges[i].limit) <= 0) {
        *overlap = true;
      } else if (range_del_agg.IsRangeOverlapped(ranges[i].start,
                                                 ranges[i].limit)) {
        *overlap = true;
      }
    }
  }
  return status;
}

const int ColumnFamilyData::kCompactAllLevels = -1;
const int ColumnFamilyData::kCompactToBaseLevel = -2;

Compaction* ColumnFamilyData::CompactRange(
    const MutableCFOptions& mutable_cf_options,
    const MutableDBOptions& mutable_db_options, int input_level,
    int output_level, const CompactRangeOptions& compact_range_options,
    const InternalKey* begin, const InternalKey* end,
    InternalKey** compaction_end, bool* conflict,
    uint64_t max_file_num_to_ignore, const std::string& trim_ts) {
  auto* result = compaction_picker_->CompactRange(
      GetName(), mutable_cf_options, mutable_db_options,
      current_->storage_info(), input_level, output_level,
      compact_range_options, begin, end, compaction_end, conflict,
      max_file_num_to_ignore, trim_ts);
  if (result != nullptr) {
    result->SetInputVersion(current_);
  }
  TEST_SYNC_POINT("ColumnFamilyData::CompactRange:Return");
  return result;
}

SuperVersion* ColumnFamilyData::GetReferencedSuperVersion(DBImpl* db) {
  SuperVersion* sv = GetThreadLocalSuperVersion(db);
  sv->Ref();
  if (!ReturnThreadLocalSuperVersion(sv)) {
    // This Unref() corresponds to the Ref() in GetThreadLocalSuperVersion()
    // when the thread-local pointer was populated. So, the Ref() earlier in
    // this function still prevents the returned SuperVersion* from being
    // deleted out from under the caller.
    sv->Unref();
  }
  return sv;
}

SuperVersion* ColumnFamilyData::GetThreadLocalSuperVersion(DBImpl* db) {
  // The SuperVersion is cached in thread local storage to avoid acquiring
  // mutex when SuperVersion does not change since the last use. When a new
  // SuperVersion is installed, the compaction or flush thread cleans up
  // cached SuperVersion in all existing thread local storage. To avoid
  // acquiring mutex for this operation, we use atomic Swap() on the thread
  // local pointer to guarantee exclusive access. If the thread local pointer
  // is being used while a new SuperVersion is installed, the cached
  // SuperVersion can become stale. In that case, the background thread would
  // have swapped in kSVObsolete. We re-check the value at when returning
  // SuperVersion back to thread local, with an atomic compare and swap.
  // The superversion will need to be released if detected to be stale.
  void* ptr = local_sv_->Swap(SuperVersion::kSVInUse);
  // Invariant:
  // (1) Scrape (always) installs kSVObsolete in ThreadLocal storage
  // (2) the Swap above (always) installs kSVInUse, ThreadLocal storage
  // should only keep kSVInUse before ReturnThreadLocalSuperVersion call
  // (if no Scrape happens).
  assert(ptr != SuperVersion::kSVInUse);
  SuperVersion* sv = static_cast<SuperVersion*>(ptr);
  if (sv == SuperVersion::kSVObsolete ||
      sv->version_number != super_version_number_.load()) {
    RecordTick(ioptions_.stats, NUMBER_SUPERVERSION_ACQUIRES);
    SuperVersion* sv_to_delete = nullptr;

    if (sv && sv->Unref()) {
      RecordTick(ioptions_.stats, NUMBER_SUPERVERSION_CLEANUPS);
      db->mutex()->Lock();
      // NOTE: underlying resources held by superversion (sst files) might
      // not be released until the next background job.
      sv->Cleanup();
      if (db->immutable_db_options().avoid_unnecessary_blocking_io) {
        db->AddSuperVersionsToFreeQueue(sv);
        db->SchedulePurge();
      } else {
        sv_to_delete = sv;
      }
    } else {
      db->mutex()->Lock();
    }
    sv = super_version_->Ref();
    db->mutex()->Unlock();

    delete sv_to_delete;
  }
  assert(sv != nullptr);
  return sv;
}

bool ColumnFamilyData::ReturnThreadLocalSuperVersion(SuperVersion* sv) {
  assert(sv != nullptr);
  // Put the SuperVersion back
  void* expected = SuperVersion::kSVInUse;
  if (local_sv_->CompareAndSwap(static_cast<void*>(sv), expected)) {
    // When we see kSVInUse in the ThreadLocal, we are sure ThreadLocal
    // storage has not been altered and no Scrape has happened. The
    // SuperVersion is still current.
    return true;
  } else {
    // ThreadLocal scrape happened in the process of this GetImpl call (after
    // thread local Swap() at the beginning and before CompareAndSwap()).
    // This means the SuperVersion it holds is obsolete.
    assert(expected == SuperVersion::kSVObsolete);
  }
  return false;
}

//Self Added
//shall be called before InstallSuperVersion for flush and compaction
//old version: old_superversion->current, new version: current_
//opt: 1 for flush, 2 for compaction, 3: for compaction direcly deleted flie
void ColumnFamilyData::updateRDF2NewVersion(int opt, bool split_flag){
  SuperVersion* old_superversion = super_version_;


  // if(old_superversion != NULL){
  //   if(old_superversion->current->get_flush_install_count() 
  //     + old_superversion->current->get_compaction_install_count() >= 1){
  //     std::cout << "Old version (old_superversion->current) flush+compact sum >= 1. times = " 
  //               << old_superversion->current->get_installSuperversion_count() << __FILE__ << ":" << __LINE__ << std::endl;
  //     std::cout << "flush = " << old_superversion->current->get_flush_install_count() << std::endl;
  //     std::cout << "compact = " << old_superversion->current->get_compaction_install_count() << std::endl;
  //     std::cout << "installSuperversion = " << old_superversion->current->get_installSuperversion_count() << std::endl;
  //   }
  // }

  if(current_->get_installSuperversion_count() > 0){
    std::cerr << "New version (current_) has been installed to New Superversion more than once. times = " 
              << current_->get_installSuperversion_count() << __FILE__ << ":" << __LINE__ << std::endl;
    std::cerr << "flush = " << current_->get_flush_install_count() << std::endl
              << "compact = " << current_->get_compaction_install_count() << std::endl
              << "installSuperversion = " << current_->get_installSuperversion_count() << std::endl;
  }
  current_->inc_installSuperversion_count();

  if(old_superversion != NULL){
    if(old_superversion->current->get_flush_install_count() 
      + old_superversion->current->get_compaction_install_count()
      + current_->get_flush_install_count()
      + current_->get_compaction_install_count() >= 0){
        
      // std::cout << "@@@ old version: " << std::endl  
      //           << "flush = " << old_superversion->current->get_flush_install_count() << std::endl
      //           << "compact = " << old_superversion->current->get_compaction_install_count() << std::endl
      //           << "installSuperversion = " << old_superversion->current->get_installSuperversion_count() << std::endl
      //           << "new version: " << std::endl
      //           << "flush = " << current_->get_flush_install_count() << std::endl
      //           << "compact = " << current_->get_compaction_install_count() << std::endl
      //           << "installSuperversion = " << current_->get_installSuperversion_count() << std::endl;

      // std::cout << "@@@@ (clr) old version: " << std::endl  
      //       << "flush_clr = " << old_superversion->current->get_flush_install_count_clr() << std::endl
      //       << "compact_clr = " << old_superversion->current->get_compaction_install_count_clr() << std::endl
      //       << "new version: " << std::endl
      //       << "flush_clr = " << current_->get_flush_install_count_clr() << std::endl
      //       << "compact_clr = " << current_->get_compaction_install_count_clr() << std::endl;
        
      // std::cout << "@@@@ opt = " << opt << std::endl;

      
      // std::cout << "@@@@ (cfd): " << std::endl  
      //       << "cfd->flush_clr = " << this->get_flush_install_count_clr() << std::endl
      //       << "cfd->compact_clr = " << this->get_compaction_install_count_clr() << std::endl;

      // std::cout << "@@@@@@ (addr) old version: " << old_superversion->current << " , new version = " << current_ << std::endl; 
    }
  }
  if(old_superversion != NULL && old_superversion->current != current_){
    current_->inc_update_at_installSuperversion_count();
    if(current_->get_update_at_installSuperversion_count() > 1){
        std::cerr << "Error: update version (current_) more than once" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
  }

  if(old_superversion != NULL && old_superversion->current != current_){
    //check each version has at most 1 flush and 1 compact
    // if( (old_superversion != NULL && old_superversion->current->get_flush_install_count() >= 2)
    //     || (old_superversion != NULL && old_superversion->current->get_compaction_install_count() >= 2)
    //     || current_->get_flush_install_count() >= 2
    //     || current_->get_compaction_install_count() >= 2){
    if( current_->get_flush_install_count() >= 2 || current_->get_compaction_install_count() >= 2){
          std::cerr << "Error: flush or compact count >= 2" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
          
          std::cerr << "@@@ old version: " << std::endl  
                  << "flush = " << old_superversion->current->get_flush_install_count() << std::endl
                  << "compact = " << old_superversion->current->get_compaction_install_count() << std::endl
                  << "installSuperversion = " << old_superversion->current->get_installSuperversion_count() << std::endl
                  << "new version: " << std::endl
                  << "flush = " << current_->get_flush_install_count() << std::endl
                  << "compact = " << current_->get_compaction_install_count() << std::endl
                  << "installSuperversion = " << current_->get_installSuperversion_count() << std::endl;

          std::cerr << "@@@@ (clr) old version: " << std::endl  
                << "flush_clr = " << old_superversion->current->get_flush_install_count_clr() << std::endl
                << "compact_clr = " << old_superversion->current->get_compaction_install_count_clr() << std::endl
                << "new version: " << std::endl
                << "flush_clr = " << current_->get_flush_install_count_clr() << std::endl
                << "compact_clr = " << current_->get_compaction_install_count_clr() << std::endl;
            
          exit(1);
    }
  }

  // if(old_superversion != NULL && old_superversion->current != current_){
    // if( (old_superversion->current->get_flush_install_count() 
    //     + current_->get_flush_install_count()) >= 2
    //     || (old_superversion->current->get_compaction_install_count() 
    //     + current_->get_compaction_install_count()) >= 2){
    //       std::cerr << "Error: (old flush + new flush) >= 2 or (old compact + new compact) >= 2" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
          
    //       std::cerr << "@@@ old version: " << std::endl  
    //             << "flush = " << old_superversion->current->get_flush_install_count() << std::endl
    //             << "compact = " << old_superversion->current->get_compaction_install_count() << std::endl
    //             << "installSuperversion = " << old_superversion->current->get_installSuperversion_count() << std::endl
    //             << "new version: " << std::endl
    //             << "flush = " << current_->get_flush_install_count() << std::endl
    //             << "compact = " << current_->get_compaction_install_count() << std::endl
    //             << "installSuperversion = " << current_->get_installSuperversion_count() << std::endl;
                
    //       std::cerr << "@@@@ (clr) old version: " << std::endl  
    //             << "flush_clr = " << old_superversion->current->get_flush_install_count_clr() << std::endl
    //             << "compact_clr = " << old_superversion->current->get_compaction_install_count_clr() << std::endl
    //             << "new version: " << std::endl
    //             << "flush_clr = " << current_->get_flush_install_count_clr() << std::endl
    //             << "compact_clr = " << current_->get_compaction_install_count_clr() << std::endl;
    //       exit(1);
    // }
    
    // if( (old_superversion->current->get_flush_install_count_clr() 
    //     + old_superversion->current->get_compaction_install_count_clr())  >= 2
    //     || (current_->get_flush_install_count_clr()
    //     + current_->get_compaction_install_count_clr()) >= 2){
    //       std::cerr << "Error: (old flush clr + old compact clr) >= 2 or (new flush clr + new compact clr) >= 2 " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
          
    //       std::cerr << "@@@ old version: " << std::endl  
    //             << "flush = " << old_superversion->current->get_flush_install_count() << std::endl
    //             << "compact = " << old_superversion->current->get_compaction_install_count() << std::endl
    //             << "installSuperversion = " << old_superversion->current->get_installSuperversion_count() << std::endl
    //             << "new version: " << std::endl
    //             << "flush = " << current_->get_flush_install_count() << std::endl
    //             << "compact = " << current_->get_compaction_install_count() << std::endl
    //             << "installSuperversion = " << current_->get_installSuperversion_count() << std::endl;
                
    //       std::cerr << "@@@@ (clr) old version: " << std::endl  
    //             << "flush_clr = " << old_superversion->current->get_flush_install_count_clr() << std::endl
    //             << "compact_clr = " << old_superversion->current->get_compaction_install_count_clr() << std::endl
    //             << "(clr) new version: " << std::endl
    //             << "flush_clr = " << current_->get_flush_install_count_clr() << std::endl
    //             << "compact_clr = " << current_->get_compaction_install_count_clr() << std::endl;
    //       std::cerr << "@@@@ opt = " << opt << std::endl;
    //       exit(1);
    // }

    // if(update_RDF_version_pre != NULL){
    //   if(old_superversion != NULL && old_superversion->current != update_RDF_version_pre){
    //     std::cerr << "Error: versions are not contiguously changing. Some versions might have been skipped." << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    //     std::cerr << "old_superversion->current = " << old_superversion->current << std::endl
    //               << "current_ = " << current_ << std::endl
    //               << "update_RDF_version_pre = " << update_RDF_version_pre << std::endl;
    //   }
    //   if(old_superversion != NULL && current_ == update_RDF_version_pre){
    //     std::cerr << "Error: versions is not moving forward." << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    //     std::cerr << "old_superversion->current = " << old_superversion->current << std::endl
    //               << "current_ = " << current_ << std::endl
    //               << "update_RDF_version_pre = " << update_RDF_version_pre << std::endl;
    //   }
    // }
    // update_RDF_version_pre = current_;
  // }


  // if(old_superversion != NULL && old_superversion->current != current_){
  //   old_superversion->current->clear_flush_install_count_clr();
  //   old_superversion->current->clear_compaction_install_count_clr();
  // }
  // current_->clear_flush_install_count_clr();
  // current_->clear_compaction_install_count_clr();

  if(this->get_flush_install_count_clr() >= 2 || this->get_compaction_install_count_clr() >= 2){
    std::cerr << "numbers of flush or compaction accumulated are more than 2" << std::endl;
    std::cerr << "@@@@ (cfd): " << std::endl  
          << "cfd->flush_clr = " << this->get_flush_install_count_clr() << std::endl
          << "cfd->compact_clr = " << this->get_compaction_install_count_clr() << std::endl;
  }


  if(this->get_split__flush_install_count_clr() >= 2 || this->get_split__compaction_install_count_clr() >= 2){
    std::cerr << "(split RDF) numbers of flush or compaction accumulated are more than 2" << std::endl;
    std::cerr << "@@@@ (cfd): " << std::endl  
          << "cfd->flush_clr = " << this->get_split__flush_install_count_clr() << std::endl
          << "cfd->compact_clr = " << this->get_split__compaction_install_count_clr() << std::endl;
  }


// std::cout << "opt = " << opt << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
// std::cout << "split_flag = " << split_flag << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  if(opt == 1){ //flush
    //Skyline RDF
    auto RDs = std::get<1>(*this->fd_RD_in_ptr);
    this->addRangeToSkylineRDFPrime(RDs);


    //fd_RDs_map
    if(this->fd_RD_in_ptr == NULL){
      std::cerr << "Error: fd_RD_in_ptr is NULL " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      exit(1);
    }
    auto fd = std::get<0>(*this->fd_RD_in_ptr);
    // auto RDs = std::get<1>(*this->fd_RD_in_ptr);
    RDs = std::get<1>(*this->fd_RD_in_ptr);
    this->fd_RDs_map[fd] = RDs;
    this->reset_fd_RD_in_ptr();
// std::cout << " save RD of fd = " << fd << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl; //xxx 



    //PLRDF
    auto &file_num = std::get<0>(this->flush_to_level0_RD_vector);
    auto &range_delete_list_in = std::get<1>(this->flush_to_level0_RD_vector);
    auto &exist_level0_file_nums = std::get<2>(this->flush_to_level0_RD_vector);
    (this->plrdf_prime).insertRangeDeleteToLevel0(file_num, 
                                                  range_delete_list_in, 
                                                  exist_level0_file_nums);

    
    this->flush_to_level0_RD_vector = make_tuple(-1, std::vector<pll>(), std::vector<uint64_t>());



    //Split PLRDF
    auto &file_num2 = std::get<0>(this->split__flush_to_level0_RD_vector);
    auto &range_delete_list_in2 = std::get<1>(this->split__flush_to_level0_RD_vector);
    auto &exist_level0_file_nums2 = std::get<2>(this->split__flush_to_level0_RD_vector);
    (this->split_plrdf_prime).insertRangeDeleteToLevel0(file_num2, 
                                                  range_delete_list_in2, 
                                                  exist_level0_file_nums2);

    
    //Top Level RDF
    (this->top_level_rdf_prime).insertRangeDeleteToLevel0(file_num2, 
                                                  range_delete_list_in2, 
                                                  exist_level0_file_nums2);


    //Split PLRDF
    this->split__flush_to_level0_RD_vector = make_tuple(-1, std::vector<pll>(), std::vector<uint64_t>());


    // if(old_superversion != NULL){
    //   old_superversion->current->clear_flush_install_count_clr();
    // }
    current_->clear_flush_install_count_clr();

    this->clear_flush_install_count_clr();
    this->clear_split__flush_install_count_clr();
  }else if(opt == 2 || opt == 3){ //compaction
    //fd_RDs_map
    const std::vector<uint64_t> &fd_in = file_in_out_ptr->fd_in;
    const std::vector<std::tuple<uint64_t, long long, long long>> &file_out = file_in_out_ptr->file_out;
    std::vector<t3ll> RD_seq_vector;
    for (auto fd : fd_in){
      for( auto &RD_seq : this->fd_RDs_map[fd]){
        RD_seq_vector.push_back(RD_seq);
      }
    }
    for(auto &file : file_out){
      auto fd = std::get<0>(file);
      auto min_range = std::get<1>(file);
      auto max_range = std::get<2>(file);
      std::vector<t3ll> RD_seq_vector2;
      for(auto &RD_seq : RD_seq_vector){
        auto new_min_range = std::max(std::get<0>(RD_seq), min_range);
        auto new_max_range = std::min(std::get<1>(RD_seq), max_range+1);
        if(new_min_range >= new_max_range){
          continue;
        }
        RD_seq_vector2.push_back( std::make_tuple(
            new_min_range, 
            new_max_range,
            std::get<2>(RD_seq)
          )
        );
      }
      std::sort(RD_seq_vector2.begin(), RD_seq_vector2.end());
      this->fd_RDs_map[fd] = RD_seq_vector2;
// std::cout << " save RD of fd = " << fd << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl; //xxx
    }
    this->reset_file_in_out_ptr();


    if(opt == 2){ //complete compaction or trivial move compaction
      //PLRDF
      (this->plrdf_prime).shiftRDFToOutputLevel(&this->compaction_moving_RD_vector);
      this->compaction_moving_RD_vector.clear();
      
      //Split PLRDF
      if(split_flag == true){
        if(this->get_split__out_level() != std::get<1>(this->split__compaction_moving_RD_vector[0])){
            std::cerr << "Error: split RDF out level is not consistent with the out level of the compaction moving RD vector" << std::endl
                      << "split RDF out level = " << this->get_split__out_level() << std::endl
                      << "compaction moving RD vector out level = " << std::get<1>(this->split__compaction_moving_RD_vector[0]) 
                      << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        }


        // (this->split_plrdf_prime).setLevelRanges(this->split__level_ranges_updated, this->get_split__out_level());
        
        // // this->split_end();
        
        if(this->get_split__count() > 1){
          std::cerr << "Error: split RDF count is more than 1" << std::endl
                    << "split RDF count = " << this->get_split__count() << std::endl
                    << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        }
      }

      //Top Level RDF
      if(split_flag == true){ //complete compaction
        int in_lvl = std::get<0>(this->split__compaction_moving_RD_vector[0]);
        // if(in_lvl == 0 || in_lvl == 1){
        if(in_lvl == 0){
          (this->top_level_rdf_prime).shiftRDFToOutputLevel(&this->split__compaction_moving_RD_vector);
          if(split_flag == true && in_lvl == 0){
            (this->top_level_rdf_prime).splitRangesOnLevel((uint)1, this->top_level__level_points);
          }
        }
      }else{ //trivial move
        // not the way we want, so --> set trivial move to false --> never do trivial move only 
        // for top level RDF (just for an easier way to simulate top level RDF's behavior)
        // int in_lvl = std::get<0>(this->top_level__trivial_move__delete_RD_vector);
        // if(in_lvl == 1){
        //   (this->top_level_rdf_prime).deleteRDFAssociatedWithFilesAtCurrentLevel(&this->top_level__trivial_move__delete_RD_vector);
        // }
        this->top_level__trivial_move__delete_RD_vector = make_tuple(-1, std::vector<pll>(), std::vector<uint64_t>());
      }


      //Split PLRDF
      (this->split_plrdf_prime).shiftRDFToOutputLevel(&this->split__compaction_moving_RD_vector);
      this->split__compaction_moving_RD_vector.clear();

      //Split PLRDF
      if(split_flag == true){ //complete compaction
        // // std::cerr << "Logging: split RDF out level = " << this->get_split__out_level() << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        (this->split_plrdf_prime).splitRangesOnLevel((uint)this->get_split__out_level(), this->split__level_points);
// std::cout << "split__level_points.size() = " << this->split__level_points.size() << std::endl;
// for (auto &i : this->split__level_points){
//   std::cout <<  i << " ";
// }
// std::cout << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl; //xxx
        this->split__level_points.clear();

        this->clear_split__count();
        this->clear_split__fin_flag();
        this->clear_split__out_level();
        this->clear_split__level_ranges_updated();
      }


    }else if(opt == 3){
      //PLRDF
      (this->plrdf_prime).deleteRDFAssociatedWithFilesAtCurrentLevel(&this->compaction_direct_delete_RD_vector);
      this->compaction_direct_delete_RD_vector = make_tuple(-1, std::vector<pll>(), std::vector<uint64_t>());
      
      //Split PLRDF
      // if(this->get_split__out_level() != std::get<0>(this->split__compaction_direct_delete_RD_vector)){
      //   std::cerr << "Error: split RDF out level is not consistent with the out level of the compaction moving RD vector" << std::endl
      //             << "split RDF out level = " << this->get_split__out_level() << std::endl
      //             << "compaction moving RD vector out level = " << std::get<0>(this->split__compaction_direct_delete_RD_vector) 
      //             << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      // }

      // // (this->split_plrdf_prime).setLevelRanges(this->split__level_ranges_updated, this->get_split__out_level());

      // if(this->get_split__count() > 1){
      //   std::cerr << "Error: split RDF count is more than 1" << std::endl
      //             << "split RDF count = " << this->get_split__count() << std::endl
      //             << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      // }
      // this->clear_split__count();
      // this->clear_split__fin_flag();
      // this->clear_split__out_level();
      // this->clear_split__level_ranges_updated();


      // //Top Level RDF
      // int in_lvl = std::get<0>(this->split__compaction_direct_delete_RD_vector);
      // if(in_lvl == 1){
      //   (this->top_level_rdf_prime).deleteRDFAssociatedWithFilesAtCurrentLevel(&this->split__compaction_direct_delete_RD_vector);
      // }

      //Split PLRDF
      (this->split_plrdf_prime).deleteRDFAssociatedWithFilesAtCurrentLevel(&this->split__compaction_direct_delete_RD_vector);
      this->split__compaction_direct_delete_RD_vector = make_tuple(-1, std::vector<pll>(), std::vector<uint64_t>());
    }

    
    if(old_superversion != NULL){
      old_superversion->current->clear_compaction_install_count_clr();
    }
    // current_->clear_compaction_install_count_clr();
    this->clear_compaction_install_count_clr();
    this->clear_split__compaction_install_count_clr();
  }else{
    std::cerr << "Error: option not implemented. opt is not 1 or 2, opt = " << opt << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
  }

  //checking version update logic
  if(old_superversion != NULL && old_superversion->current == current_){
    std::cerr << "old and new version are both the same, " << "old_superversion->current == current_" << __FILE__ << ":" << __LINE__  << std::endl;
  }


  //PLRDF
  if(this->get_call_before_install_superversion_count() != 0){
    std::cerr << "Error: call_before_install_superversion_count is not 0 @updateRDF2NewVersion, call_before_install_superversion_count = " 
              << this->get_call_before_install_superversion_count() << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
  }
  this->inc_call_before_install_superversion_count();

  //Split PLRDF
  if(this->get_split__call_before_install_superversion_count() != 0){
    std::cerr << "Error: split__call_before_install_superversion_count is not 0 @updateRDF2NewVersion, split__call_before_install_superversion_count = " 
              << this->get_split__call_before_install_superversion_count() << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
  }
  this->inc_split__call_before_install_superversion_count();


  this->logCurrentTotalNumbersOfRangesInEachRDF();
}
//Self Added End







void ColumnFamilyData::InstallSuperVersion(SuperVersionContext* sv_context,
                                           InstrumentedMutex* db_mutex) {
  db_mutex->AssertHeld();
  return InstallSuperVersion(sv_context, mutable_cf_options_);
}

void ColumnFamilyData::InstallSuperVersion(
    SuperVersionContext* sv_context,
    const MutableCFOptions& mutable_cf_options) {
  
  //self added
  // std::cout << std::endl << "(cfd) this->current()->printRDFTest()" << std::endl;
  // this->current()->printRDFTest();
  // std::cout << std::endl << "(cfd) current_->printRDFTest()" << std::endl;
  // current_->printRDFTest();
  // std::cout << std::endl << "(cfd) current_->printRDFTest2()" << std::endl;
  // current_->printRDFTest2();
  SuperVersion* old_superversion = super_version_;

  
  
  //self added 
  // std::cout << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
  // old_superversion->getRDFTest();
  std::vector<std::pair<long long, long long>> RDF_test_old;
  // std::vector<std::pair<long long, long long>> RDF_test_tmp = old_superversion->getRDFTest();
  // std::cout << "(cfd) super_version_->current->printRDFTest2() (version) " << std::endl;
  // super_version_->current->printRDFTest2();


  if(old_superversion == NULL){
    // std::cout << "(cfd) old_superversion == NULL" << std::endl;
  }else{
    // std::cout << "(cfd) old_superversion != NULL" << std::endl;
    
    // // std::cout << "(cfd) old_superversion->current->printRDFTest() (version) " << std::endl;
    // // old_superversion->current->printRDFTest();
    // // std::cout << "(cfd) old_superversion->current->printRDFTest2() (version) " << std::endl;
    // // old_superversion->current->printRDFTest2();

    // // if(old_superversion->current->getIsRDFTest2Set() == true){
    // //   RDF_test_old = old_superversion->current->getRDFTest2();
    // // }else{
    // //   RDF_test_old = old_superversion->current->getRDFTest();
    // // }
    // // // (new_superversion->current)->setRDFTest(RDF_test_old);
    // // current_->setRDFTest(RDF_test_old);    

    // // PL_RDF per_level_RDF_old;
    // // if(old_superversion->current->getIsRDFUpdated() == true){
    //   // per_level_RDF_old = old_superversion->current->getPerLevelRDFUpdated();
    //   // per_level_RDF_old.deleteLastLevelIfEqualsBottomLevel((uint)current_->storage_info()->num_levels());

    // // rdfilter::PLRDF::getRDFilter()->deleteLastLevelIfEqualsBottomLevel((uint)current_->storage_info()->num_levels());

    // // }else{
    // //   per_level_RDF_old = old_superversion->current->getPerLevelRDF();
    // // }
    // // current_->setPerLevelRDF(per_level_RDF_old);
  }

  if(old_superversion != NULL){
    // std::cout << "old_superversion->current->printAllLevelSize()" << std::endl;
    // old_superversion->current->printAllLevelSize();
    // // std::cout << "old_superversion->current->printAllFileRanges()" << std::endl;
    // // old_superversion->current->printAllFileRanges();
  }


  // if(old_superversion == NULL && old_superversion->current != current_){
  if(old_superversion == NULL){
    //PLRDF
    current_->setPLRDF(this->plrdf_prime);
    if(this->get_call_before_install_superversion_count() > 1){
      // std::cerr << "Error: call_before_install_superversion_count is not 1 @installSuperversion, call_before_install_superversion_count = " 
      //           << this->get_call_before_install_superversion_count() << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      std::cerr << "Error: call_before_install_superversion_count > 1 @installSuperversion, call_before_install_superversion_count = " 
                << this->get_call_before_install_superversion_count() << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    this->clear_call_before_install_superversion_count();

    //Split PLRDF
    current_->setSplitPLRDF(this->split_plrdf_prime);
    if(this->get_split__call_before_install_superversion_count() > 1){
      std::cerr << "Error: split__call_before_install_superversion_count > 1 @installSuperversion, split__call_before_install_superversion_count = " 
                << this->get_split__call_before_install_superversion_count() << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    this->clear_split__call_before_install_superversion_count();

    //Top Level RDF
    current_->setTopLevelRDF(this->top_level_rdf_prime);

    //Skyline RDF
    current_->setSkylineRDF(this->skyline_rdf_prime);
    current_->setSkylineNumbersOfRangesInRDFLog(this->skyline__numbers_of_ranges_in_rdf_log);
  }

  if(old_superversion != NULL && old_superversion->current != current_){
    //PLRDF
    (this->plrdf_prime).deleteLastLevelIfEqualsBottomLevel((uint)current_->storage_info()->num_levels());
    current_->setPLRDF(this->plrdf_prime);
    // if(this->get_call_before_install_superversion_count() != 1){
    if(this->get_call_before_install_superversion_count() > 1){
      // std::cerr << "Error: call_before_install_superversion_count is not 1 @installSuperversion, call_before_install_superversion_count = " 
      //           << this->get_call_before_install_superversion_count() << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      std::cerr << "Error: call_before_install_superversion_count > 1 @installSuperversion, call_before_install_superversion_count = " 
                << this->get_call_before_install_superversion_count() << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    this->clear_call_before_install_superversion_count();
    // old_superversion->current->setPLRDF(this->plrdf_prime);

    //Split PLRDF
    (this->split_plrdf_prime).deleteLastLevelIfEqualsBottomLevel((uint)current_->storage_info()->num_levels());
    current_->setSplitPLRDF(this->split_plrdf_prime);
    if(this->get_split__call_before_install_superversion_count() > 1){
      std::cerr << "Error: split__call_before_install_superversion_count > 1 @installSuperversion, split__call_before_install_superversion_count = " 
                << this->get_split__call_before_install_superversion_count() << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    this->clear_split__call_before_install_superversion_count();

    //Top Level RDF
    current_->setTopLevelRDF(this->top_level_rdf_prime);

    //Skyline RDF
    current_->setSkylineRDF(this->skyline_rdf_prime);
    current_->setSkylineNumbersOfRangesInRDFLog(this->skyline__numbers_of_ranges_in_rdf_log);



    if(install_version_pre != NULL){
      if(old_superversion != NULL && old_superversion->current != install_version_pre){
        std::cerr << "Error: versions are not contiguously changing. Some versions might have been skipped." << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        std::cerr << "old_superversion->current = " << old_superversion->current << std::endl
                  << "current_ = " << current_ << std::endl
                  << "install_version_pre = " << install_version_pre << std::endl;
      }
      if(old_superversion != NULL && current_ == install_version_pre){
        std::cerr << "Error: versions is not moving forward." << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        std::cerr << "old_superversion->current = " << old_superversion->current << std::endl
                  << "current_ = " << current_ << std::endl
                  << "install_version_pre = " << install_version_pre << std::endl;
      }
    }
    install_version_pre = current_;
  }

  // if(old_superversion != NULL){
  //   for(auto &x: old_superversion->current->getRDFTestCompact()){
  //       current_->storeRange2RDFTest(x.first, x.second);
  //   }
  // }

  // if(old_superversion != NULL){
  //   std::cout << "(cfd) old_superversion->current->printRDFTestCompact()  (version) " << std::endl;
  //   // new_superversion->current->printRDFTest();
  //   old_superversion->current->printRDFTestCompact();
  // }

  // std::cout << "(cfd) new_superversion->current->printRDFTestCompact()  (version) " << std::endl;
  // // new_superversion->current->printRDFTest();
  // current_->printRDFTestCompact();
  // if(current_->getRDFTestCompact().size() != 0){
  //   for(auto &x: current_->getRDFTestCompact()){
  //     current_->storeRange2RDFTest(x.first, x.second);
  //   }
  // }
  // std::cout << "(cfd) old_superversion->current->printRDFTest2() (version) " << std::endl;
  // old_superversion->current->printRDFTest2();

  // std::cout << "(cfd) current_->printRDFTest2()  (version) " << std::endl;
  // current_->printRDFTest2();

  
  // std::cout << "(cfd) new_superversion->current->printRDFTest()  (version) " << std::endl;
  // // new_superversion->current->printRDFTest();
  // current_->printRDFTest();
  // std::cout << "(cfd) new_superversion->current->printRDFTest2()  (version) " << std::endl;
  // // new_superversion->current->printRDFTest2();
  // current_->printRDFTest2();
  
  
  // std::cout << "(cfd) this->printRDFTest " << std::endl;
  // this->printRDFTest();
  
  
  // std::cout << "(cfd) this->printRDFTest2 " << std::endl;
  // this->printRDFTest2();

  // std::cout << std::endl << std::endl;
  // // (new_superversion->current)->setRDFTest(RDF_test_tmp);
  // // (new_superversion->current)->setRDFTest2(RDF_test2);




// /*
//   //checking version update logic
//   if(old_superversion != NULL && old_superversion->current == current_){
//     std::cout << "old and new version are both the same, " << "old_superversion->current == current_" << __FILE__ << ":" << __LINE__  << std::endl;
//   }

//   if(old_superversion != NULL){
//     if(old_superversion->current->get_flush_install_count() 
//       + old_superversion->current->get_compaction_install_count() >= 1){
//       std::cout << "Old version (old_superversion->current) flush+compact sum >= 1. times = " 
//                 << old_superversion->current->get_installSuperversion_count() << __FILE__ << ":" << __LINE__ << std::endl;
//       std::cout << "flush = " << old_superversion->current->get_flush_install_count() << std::endl;
//       std::cout << "compact = " << old_superversion->current->get_compaction_install_count() << std::endl;
//       std::cout << "installSuperversion = " << old_superversion->current->get_installSuperversion_count() << std::endl;
//     }
//   }

//   if(current_->get_installSuperversion_count() > 0){
//     std::cout << "New version (current_) has been installed to New Superversion more than once. times = " 
//               << current_->get_installSuperversion_count() << __FILE__ << ":" << __LINE__ << std::endl;
//     std::cout << "flush = " << current_->get_flush_install_count() << std::endl;
//     std::cout << "compact = " << current_->get_compaction_install_count() << std::endl;
//     std::cout << "installSuperversion = " << current_->get_installSuperversion_count() << std::endl;
//   }
//   current_->inc_installSuperversion_count();

//   if(old_superversion != NULL){
//     if(old_superversion->current->get_flush_install_count() 
//       + old_superversion->current->get_compaction_install_count()
//       + current_->get_flush_install_count()
//       + current_->get_compaction_install_count() >= 0){
        
//       std::cout << "@@@ old version: " << std::endl  
//                 << "flush = " << old_superversion->current->get_flush_install_count() << std::endl
//                 << "compact = " << old_superversion->current->get_compaction_install_count() << std::endl
//                 << "installSuperversion = " << old_superversion->current->get_installSuperversion_count() << std::endl
//                 << "new version: " << std::endl
//                 << "flush = " << current_->get_flush_install_count() << std::endl
//                 << "compact = " << current_->get_compaction_install_count() << std::endl
//                 << "installSuperversion = " << current_->get_installSuperversion_count() << std::endl;

//       std::cout << "@@@@ (clr) old version: " << std::endl  
//             << "flush_clr = " << old_superversion->current->get_flush_install_count_clr() << std::endl
//             << "compact_clr = " << old_superversion->current->get_compaction_install_count_clr() << std::endl
//             << "new version: " << std::endl
//             << "flush_clr = " << current_->get_flush_install_count_clr() << std::endl
//             << "compact_clr = " << current_->get_compaction_install_count_clr() << std::endl;

//       std::cout << "@@@@@@ (addr) old version: " << old_superversion->current << " , new version = " << current_ << std::endl; 
//     }
//   }
//   if(old_superversion != NULL && old_superversion->current != current_){
//     current_->inc_update_at_installSuperversion_count();
//     if(current_->get_update_at_installSuperversion_count() > 1){
//         std::cerr << "Error: update version (current_) more than once" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//     }
//   }

//   if(old_superversion != NULL && old_superversion->current != current_){
//     //check each version has at most 1 flush and 1 compact
//     if( (old_superversion != NULL && old_superversion->current->get_flush_install_count() >= 2)
//         || (old_superversion != NULL && old_superversion->current->get_compaction_install_count() >= 2)
//         || current_->get_flush_install_count() >= 2
//         || current_->get_compaction_install_count() >= 2){
//           std::cerr << "Error: flush or compact count >= 2" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
          
//           std::cerr << "@@@ old version: " << std::endl  
//                   << "flush = " << old_superversion->current->get_flush_install_count() << std::endl
//                   << "compact = " << old_superversion->current->get_compaction_install_count() << std::endl
//                   << "installSuperversion = " << old_superversion->current->get_installSuperversion_count() << std::endl
//                   << "new version: " << std::endl
//                   << "flush = " << current_->get_flush_install_count() << std::endl
//                   << "compact = " << current_->get_compaction_install_count() << std::endl
//                   << "installSuperversion = " << current_->get_installSuperversion_count() << std::endl;

//           std::cerr << "@@@@ (clr) old version: " << std::endl  
//                 << "flush_clr = " << old_superversion->current->get_flush_install_count_clr() << std::endl
//                 << "compact_clr = " << old_superversion->current->get_compaction_install_count_clr() << std::endl
//                 << "new version: " << std::endl
//                 << "flush_clr = " << current_->get_flush_install_count_clr() << std::endl
//                 << "compact_clr = " << current_->get_compaction_install_count_clr() << std::endl;
            
//           exit(1);
//     }
//   }

//   if(old_superversion != NULL && old_superversion->current != current_){
//     if( (old_superversion->current->get_flush_install_count() 
//         + current_->get_flush_install_count()) >= 2
//         || (old_superversion->current->get_compaction_install_count() 
//         + current_->get_compaction_install_count()) >= 2){
//           std::cerr << "Error: (old flush + new flush) >= 2 or (old compact + new compact) >= 2" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
          
//           std::cerr << "@@@ old version: " << std::endl  
//                 << "flush = " << old_superversion->current->get_flush_install_count() << std::endl
//                 << "compact = " << old_superversion->current->get_compaction_install_count() << std::endl
//                 << "installSuperversion = " << old_superversion->current->get_installSuperversion_count() << std::endl
//                 << "new version: " << std::endl
//                 << "flush = " << current_->get_flush_install_count() << std::endl
//                 << "compact = " << current_->get_compaction_install_count() << std::endl
//                 << "installSuperversion = " << current_->get_installSuperversion_count() << std::endl;
                
//           std::cerr << "@@@@ (clr) old version: " << std::endl  
//                 << "flush_clr = " << old_superversion->current->get_flush_install_count_clr() << std::endl
//                 << "compact_clr = " << old_superversion->current->get_compaction_install_count_clr() << std::endl
//                 << "new version: " << std::endl
//                 << "flush_clr = " << current_->get_flush_install_count_clr() << std::endl
//                 << "compact_clr = " << current_->get_compaction_install_count_clr() << std::endl;
//           exit(1);
//     }
    
//     if( (old_superversion->current->get_flush_install_count() 
//         + old_superversion->current->get_compaction_install_count())  >= 2
//         || (current_->get_flush_install_count()
//         + current_->get_compaction_install_count()) >= 2){
//           std::cerr << "Error: (old flush + old compact) >= 2 or (new flush + new compact) >= 2" << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
          
//           std::cerr << "@@@ old version: " << std::endl  
//                 << "flush = " << old_superversion->current->get_flush_install_count() << std::endl
//                 << "compact = " << old_superversion->current->get_compaction_install_count() << std::endl
//                 << "installSuperversion = " << old_superversion->current->get_installSuperversion_count() << std::endl
//                 << "new version: " << std::endl
//                 << "flush = " << current_->get_flush_install_count() << std::endl
//                 << "compact = " << current_->get_compaction_install_count() << std::endl
//                 << "installSuperversion = " << current_->get_installSuperversion_count() << std::endl;
                
//           std::cerr << "@@@@ (clr) old version: " << std::endl  
//                 << "flush_clr = " << old_superversion->current->get_flush_install_count_clr() << std::endl
//                 << "compact_clr = " << old_superversion->current->get_compaction_install_count_clr() << std::endl
//                 << "new version: " << std::endl
//                 << "flush_clr = " << current_->get_flush_install_count_clr() << std::endl
//                 << "compact_clr = " << current_->get_compaction_install_count_clr() << std::endl;
//           exit(1);
//     }
//   }


//   if(old_superversion != NULL && old_superversion->current != current_){
//     old_superversion->current->clear_flush_install_count_clr();
//     old_superversion->current->clear_compaction_install_count_clr();
//   }
//   // current_->clear_flush_install_count_clr();
//   // current_->clear_compaction_install_count_clr();
//   */
  //Self Added End






  SuperVersion* new_superversion = sv_context->new_superversion.release();
  new_superversion->mutable_cf_options = mutable_cf_options;

  new_superversion->Init(this, mem_, imm_.current(), current_);
  
  




  super_version_ = new_superversion;
  ++super_version_number_;
  super_version_->version_number = super_version_number_;
  if (old_superversion == nullptr || old_superversion->current != current() ||
      old_superversion->mem != mem_ ||
      old_superversion->imm != imm_.current()) {
    // Should not recalculate slow down condition if nothing has changed, since
    // currently RecalculateWriteStallConditions() treats it as further slowing
    // down is needed.
    super_version_->write_stall_condition =
        RecalculateWriteStallConditions(mutable_cf_options);
  } else {
    super_version_->write_stall_condition =
        old_superversion->write_stall_condition;
  }
  if (old_superversion != nullptr) {
    // Reset SuperVersions cached in thread local storage.
    // This should be done before old_superversion->Unref(). That's to ensure
    // that local_sv_ never holds the last reference to SuperVersion, since
    // it has no means to safely do SuperVersion cleanup.
    ResetThreadLocalSuperVersions();

    if (old_superversion->mutable_cf_options.write_buffer_size !=
        mutable_cf_options.write_buffer_size) {
      mem_->UpdateWriteBufferSize(mutable_cf_options.write_buffer_size);
    }
    if (old_superversion->write_stall_condition !=
        new_superversion->write_stall_condition) {
      sv_context->PushWriteStallNotification(
          old_superversion->write_stall_condition,
          new_superversion->write_stall_condition, GetName(), ioptions());
    }
    if (old_superversion->Unref()) {
      old_superversion->Cleanup();
      sv_context->superversions_to_free.push_back(old_superversion);
    }
  }
}

void ColumnFamilyData::ResetThreadLocalSuperVersions() {
  autovector<void*> sv_ptrs;
  local_sv_->Scrape(&sv_ptrs, SuperVersion::kSVObsolete);
  for (auto ptr : sv_ptrs) {
    assert(ptr);
    if (ptr == SuperVersion::kSVInUse) {
      continue;
    }
    auto sv = static_cast<SuperVersion*>(ptr);
    bool was_last_ref __attribute__((__unused__));
    was_last_ref = sv->Unref();
    // sv couldn't have been the last reference because
    // ResetThreadLocalSuperVersions() is called before
    // unref'ing super_version_.
    assert(!was_last_ref);
  }
}

Status ColumnFamilyData::ValidateOptions(
    const DBOptions& db_options, const ColumnFamilyOptions& cf_options) {
  Status s;
  s = CheckCompressionSupported(cf_options);
  if (s.ok() && db_options.allow_concurrent_memtable_write) {
    s = CheckConcurrentWritesSupported(cf_options);
  }
  if (s.ok() && db_options.unordered_write &&
      cf_options.max_successive_merges != 0) {
    s = Status::InvalidArgument(
        "max_successive_merges > 0 is incompatible with unordered_write");
  }
  if (s.ok()) {
    s = CheckCFPathsSupported(db_options, cf_options);
  }
  if (!s.ok()) {
    return s;
  }

  if (cf_options.ttl > 0 && cf_options.ttl != kDefaultTtl) {
    if (!cf_options.table_factory->IsInstanceOf(
            TableFactory::kBlockBasedTableName())) {
      return Status::NotSupported(
          "TTL is only supported in Block-Based Table format. ");
    }
  }

  if (cf_options.periodic_compaction_seconds > 0 &&
      cf_options.periodic_compaction_seconds != kDefaultPeriodicCompSecs) {
    if (!cf_options.table_factory->IsInstanceOf(
            TableFactory::kBlockBasedTableName())) {
      return Status::NotSupported(
          "Periodic Compaction is only supported in "
          "Block-Based Table format. ");
    }
  }

  if (cf_options.enable_blob_garbage_collection) {
    if (cf_options.blob_garbage_collection_age_cutoff < 0.0 ||
        cf_options.blob_garbage_collection_age_cutoff > 1.0) {
      return Status::InvalidArgument(
          "The age cutoff for blob garbage collection should be in the range "
          "[0.0, 1.0].");
    }
    if (cf_options.blob_garbage_collection_force_threshold < 0.0 ||
        cf_options.blob_garbage_collection_force_threshold > 1.0) {
      return Status::InvalidArgument(
          "The garbage ratio threshold for forcing blob garbage collection "
          "should be in the range [0.0, 1.0].");
    }
  }

  if (cf_options.compaction_style == kCompactionStyleFIFO &&
      db_options.max_open_files != -1 && cf_options.ttl > 0) {
    return Status::NotSupported(
        "FIFO compaction only supported with max_open_files = -1.");
  }

  std::vector<uint32_t> supported{0, 1, 2, 4, 8};
  if (std::find(supported.begin(), supported.end(),
                cf_options.memtable_protection_bytes_per_key) ==
      supported.end()) {
    return Status::NotSupported(
        "Memtable per key-value checksum protection only supports 0, 1, 2, 4 "
        "or 8 bytes per key.");
  }
  if (std::find(supported.begin(), supported.end(),
                cf_options.block_protection_bytes_per_key) == supported.end()) {
    return Status::NotSupported(
        "Block per key-value checksum protection only supports 0, 1, 2, 4 "
        "or 8 bytes per key.");
  }

  if (!cf_options.compaction_options_fifo.file_temperature_age_thresholds
           .empty()) {
    if (cf_options.compaction_style != kCompactionStyleFIFO) {
      return Status::NotSupported(
          "Option file_temperature_age_thresholds only supports FIFO "
          "compaction.");
    } else if (cf_options.num_levels > 1) {
      return Status::NotSupported(
          "Option file_temperature_age_thresholds is only supported when "
          "num_levels = 1.");
    } else {
      const auto& ages =
          cf_options.compaction_options_fifo.file_temperature_age_thresholds;
      assert(ages.size() >= 1);
      // check that age is sorted
      for (size_t i = 0; i < ages.size() - 1; ++i) {
        if (ages[i].age >= ages[i + 1].age) {
          return Status::NotSupported(
              "Option file_temperature_age_thresholds requires elements to be "
              "sorted in increasing order with respect to `age` field.");
        }
      }
    }
  }
  return s;
}

Status ColumnFamilyData::SetOptions(
    const DBOptions& db_opts,
    const std::unordered_map<std::string, std::string>& options_map) {
  ColumnFamilyOptions cf_opts =
      BuildColumnFamilyOptions(initial_cf_options_, mutable_cf_options_);
  ConfigOptions config_opts;
  config_opts.mutable_options_only = true;
  Status s = GetColumnFamilyOptionsFromMap(config_opts, cf_opts, options_map,
                                           &cf_opts);
  if (s.ok()) {
    s = ValidateOptions(db_opts, cf_opts);
  }
  if (s.ok()) {
    mutable_cf_options_ = MutableCFOptions(cf_opts);
    mutable_cf_options_.RefreshDerivedOptions(ioptions_);
  }
  return s;
}

// REQUIRES: DB mutex held
Env::WriteLifeTimeHint ColumnFamilyData::CalculateSSTWriteHint(int level) {
  if (initial_cf_options_.compaction_style != kCompactionStyleLevel) {
    return Env::WLTH_NOT_SET;
  }
  if (level == 0) {
    return Env::WLTH_MEDIUM;
  }
  int base_level = current_->storage_info()->base_level();

  // L1: medium, L2: long, ...
  if (level - base_level >= 2) {
    return Env::WLTH_EXTREME;
  } else if (level < base_level) {
    // There is no restriction which prevents level passed in to be smaller
    // than base_level.
    return Env::WLTH_MEDIUM;
  }
  return static_cast<Env::WriteLifeTimeHint>(
      level - base_level + static_cast<int>(Env::WLTH_MEDIUM));
}

Status ColumnFamilyData::AddDirectories(
    std::map<std::string, std::shared_ptr<FSDirectory>>* created_dirs) {
  Status s;
  assert(created_dirs != nullptr);
  assert(data_dirs_.empty());
  for (auto& p : ioptions_.cf_paths) {
    auto existing_dir = created_dirs->find(p.path);

    if (existing_dir == created_dirs->end()) {
      std::unique_ptr<FSDirectory> path_directory;
      s = DBImpl::CreateAndNewDirectory(ioptions_.fs.get(), p.path,
                                        &path_directory);
      if (!s.ok()) {
        return s;
      }
      assert(path_directory != nullptr);
      data_dirs_.emplace_back(path_directory.release());
      (*created_dirs)[p.path] = data_dirs_.back();
    } else {
      data_dirs_.emplace_back(existing_dir->second);
    }
  }
  assert(data_dirs_.size() == ioptions_.cf_paths.size());
  return s;
}

FSDirectory* ColumnFamilyData::GetDataDir(size_t path_id) const {
  if (data_dirs_.empty()) {
    return nullptr;
  }

  assert(path_id < data_dirs_.size());
  return data_dirs_[path_id].get();
}

void ColumnFamilyData::RecoverEpochNumbers() {
  assert(current_);
  auto* vstorage = current_->storage_info();
  assert(vstorage);
  vstorage->RecoverEpochNumbers(this);
}

ColumnFamilySet::ColumnFamilySet(const std::string& dbname,
                                 const ImmutableDBOptions* db_options,
                                 const FileOptions& file_options,
                                 Cache* table_cache,
                                 WriteBufferManager* _write_buffer_manager,
                                 WriteController* _write_controller,
                                 BlockCacheTracer* const block_cache_tracer,
                                 const std::shared_ptr<IOTracer>& io_tracer,
                                 const std::string& db_id,
                                 const std::string& db_session_id)
    : max_column_family_(0),
      file_options_(file_options),
      dummy_cfd_(new ColumnFamilyData(
          ColumnFamilyData::kDummyColumnFamilyDataId, "", nullptr, nullptr,
          nullptr, ColumnFamilyOptions(), *db_options, &file_options_, nullptr,
          block_cache_tracer, io_tracer, db_id, db_session_id)),
      default_cfd_cache_(nullptr),
      db_name_(dbname),
      db_options_(db_options),
      table_cache_(table_cache),
      write_buffer_manager_(_write_buffer_manager),
      write_controller_(_write_controller),
      block_cache_tracer_(block_cache_tracer),
      io_tracer_(io_tracer),
      db_id_(db_id),
      db_session_id_(db_session_id) {
  // initialize linked list
  dummy_cfd_->prev_ = dummy_cfd_;
  dummy_cfd_->next_ = dummy_cfd_;
}

ColumnFamilySet::~ColumnFamilySet() {
  while (column_family_data_.size() > 0) {
    // cfd destructor will delete itself from column_family_data_
    auto cfd = column_family_data_.begin()->second;
    bool last_ref __attribute__((__unused__));
    last_ref = cfd->UnrefAndTryDelete();
    assert(last_ref);
  }
  bool dummy_last_ref __attribute__((__unused__));
  dummy_last_ref = dummy_cfd_->UnrefAndTryDelete();
  assert(dummy_last_ref);
}

ColumnFamilyData* ColumnFamilySet::GetDefault() const {
  assert(default_cfd_cache_ != nullptr);
  return default_cfd_cache_;
}

ColumnFamilyData* ColumnFamilySet::GetColumnFamily(uint32_t id) const {
  auto cfd_iter = column_family_data_.find(id);
  if (cfd_iter != column_family_data_.end()) {
    return cfd_iter->second;
  } else {
    return nullptr;
  }
}

ColumnFamilyData* ColumnFamilySet::GetColumnFamily(
    const std::string& name) const {
  auto cfd_iter = column_families_.find(name);
  if (cfd_iter != column_families_.end()) {
    auto cfd = GetColumnFamily(cfd_iter->second);
    assert(cfd != nullptr);
    return cfd;
  } else {
    return nullptr;
  }
}

uint32_t ColumnFamilySet::GetNextColumnFamilyID() {
  return ++max_column_family_;
}

uint32_t ColumnFamilySet::GetMaxColumnFamily() { return max_column_family_; }

void ColumnFamilySet::UpdateMaxColumnFamily(uint32_t new_max_column_family) {
  max_column_family_ = std::max(new_max_column_family, max_column_family_);
}

size_t ColumnFamilySet::NumberOfColumnFamilies() const {
  return column_families_.size();
}

// under a DB mutex AND write thread
ColumnFamilyData* ColumnFamilySet::CreateColumnFamily(
    const std::string& name, uint32_t id, Version* dummy_versions,
    const ColumnFamilyOptions& options) {
  assert(column_families_.find(name) == column_families_.end());
  ColumnFamilyData* new_cfd = new ColumnFamilyData(
      id, name, dummy_versions, table_cache_, write_buffer_manager_, options,
      *db_options_, &file_options_, this, block_cache_tracer_, io_tracer_,
      db_id_, db_session_id_);
  column_families_.insert({name, id});
  column_family_data_.insert({id, new_cfd});
  max_column_family_ = std::max(max_column_family_, id);
  // add to linked list
  new_cfd->next_ = dummy_cfd_;
  auto prev = dummy_cfd_->prev_;
  new_cfd->prev_ = prev;
  prev->next_ = new_cfd;
  dummy_cfd_->prev_ = new_cfd;
  if (id == 0) {
    default_cfd_cache_ = new_cfd;
  }
  return new_cfd;
}

// under a DB mutex AND from a write thread
void ColumnFamilySet::RemoveColumnFamily(ColumnFamilyData* cfd) {
  auto cfd_iter = column_family_data_.find(cfd->GetID());
  assert(cfd_iter != column_family_data_.end());
  column_family_data_.erase(cfd_iter);
  column_families_.erase(cfd->GetName());
}

// under a DB mutex OR from a write thread
bool ColumnFamilyMemTablesImpl::Seek(uint32_t column_family_id) {
  if (column_family_id == 0) {
    // optimization for common case
    current_ = column_family_set_->GetDefault();
  } else {
    current_ = column_family_set_->GetColumnFamily(column_family_id);
  }
  handle_.SetCFD(current_);
  return current_ != nullptr;
}

uint64_t ColumnFamilyMemTablesImpl::GetLogNumber() const {
  assert(current_ != nullptr);
  return current_->GetLogNumber();
}

MemTable* ColumnFamilyMemTablesImpl::GetMemTable() const {
  assert(current_ != nullptr);
  return current_->mem();
}

ColumnFamilyHandle* ColumnFamilyMemTablesImpl::GetColumnFamilyHandle() {
  assert(current_ != nullptr);
  return &handle_;
}

uint32_t GetColumnFamilyID(ColumnFamilyHandle* column_family) {
  uint32_t column_family_id = 0;
  if (column_family != nullptr) {
    auto cfh = static_cast_with_check<ColumnFamilyHandleImpl>(column_family);
    column_family_id = cfh->GetID();
  }
  return column_family_id;
}

const Comparator* GetColumnFamilyUserComparator(
    ColumnFamilyHandle* column_family) {
  if (column_family != nullptr) {
    return column_family->GetComparator();
  }
  return nullptr;
}

}  // namespace ROCKSDB_NAMESPACE
