/*
 *  Created on: April 23, 2023
 *  Author: yucheng
 */

/*Notice: anytime only one lock is locked in this class*/

// #define CHECK_PLRDF_TRACING_CONSISTENCY

#ifndef SYS_RDFILTER_H_
#define SYS_RDFILTER_H_

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "SuRF/include/surf.hpp"

// namespace rdfilter {
//   // class PerlevelRangeDeleteFilterByVector {
//   class PLRDF;
// }

using pll = std::pair<long long, long long>;      //[start, end)
using pss = std::pair<std::string, std::string>;  //[start, end)
using t3ll =
    std::tuple<long long, long long, long long>;  //([start, end), time)
// using t2str1l = std::tuple<std::string, std::string, long long>; //([start,
// end), time)

struct FileRDs {
  // std::vector<uint64_t> fd_in;
  std::vector<t3ll> rds;
  std::vector<std::tuple<std::string, std::string, long long>> string_rds;

  void set_rds(const std::vector<t3ll>& rds_in) { this->rds = rds_in; }

  void set_string_rds(
      const std::vector<std::tuple<std::string, std::string, long long>>&
          rds_in) {
    this->string_rds = rds_in;
  }

  void print() {
    std::cout << "file_out: ";
    for (auto rd : rds) {
      std::cout << " (" << std::get<0>(rd) << ", " << std::get<1>(rd) << ") "
                << std::get<2>(rd);
    }
    std::cout << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
              << std::endl;
  }

  void print_string_rds() {
    std::cout << "file_out: ";
    for (auto rd : string_rds) {
      std::cout << " (" << std::get<0>(rd) << ", " << std::get<1>(rd) << ") "
                << std::get<2>(rd);
    }
    std::cout << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
              << std::endl;
  }

  size_t get_rds_size() {
    size_t size = 0;
    for (auto& rd : rds) {
      long long s = std::get<0>(rd);
      long long e = std::get<1>(rd);
      long long time = std::get<2>(rd);
      size += sizeof(s) + sizeof(e) + sizeof(time);
    }
    return size;
  }

  size_t get_string_rds_size() {
    size_t size = 0;
    for (auto& rd : string_rds) {
      auto& s = std::get<0>(rd);
      auto& e = std::get<1>(rd);
      auto& time = std::get<2>(rd);
      size += s.size() + e.size() + sizeof(time);
    }
    return size;
  }
};

struct FileInOut {
  std::vector<uint64_t> fd_in;
  std::vector<std::tuple<uint64_t, long long, long long>> file_out;
  std::vector<
      std::tuple<uint64_t, std::string, std::string, std::string, std::string>>
      string_file_out;

  void print() {
    std::cout << "fd_in: ";
    for (auto i : fd_in) {
      std::cout << i << " ";
    }
    std::cout << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
              << std::endl;
    std::cout << "file_out: ";
    for (auto i : file_out) {
      std::cout << std::get<0>(i) << " (" << std::get<1>(i) << ", "
                << std::get<2>(i) << ") ";
    }
    std::cout << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
              << std::endl;
  }

  void print_string_file_out() {
    std::cout << "string_file_out: ";
    for (auto i : string_file_out) {
      std::cout << std::get<0>(i) << " (P: " << std::get<1>(i) << " - "
                << std::get<2>(i) << ", RT: " << std::get<3>(i) << " - "
                << std::get<4>(i) << ") ";
    }
    std::cout << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
              << std::endl;
  }
};

// using namespace ROCKSDB_NAMESPACE;
// namespace ROCKSDB_NAMESPACE{
class PLRDF;
class SkyLineRDF;
// }

#include <assert.h>
#include <sys/time.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "include/rocksdb/SuRF/include/surf.hpp"

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
//     std::unordered_map<uint64_t, std::vector<pll>> rd_filter_level0; //for
//     level 0, (file_num, RD_list), FileMetaData* -> fd .GetNumber();

//     std::vector<std::vector<pll>> rd_filter; //for level > 0, list of range
//     delete (start, end), all entries are non-overlapping std::vector<int>
//     numbers_of_ranges_in_RDF_log; //for level > 0, number of ranges in RDF

//     void addRangeDelete_internal(uint level, std::vector<pll>
//     &range_delete_list_in); std::vector<pll> sortAndMerge(std::vector<pll>
//     &range_delete_list_in); void addRangeDelete(std::vector<pll>
//     &range_delete_list, std::vector<pll> &range_delete_list_in); void
//     addRangeDelete(std::vector<pll> &range_delete_list, long long start, long
//     long end); void print_internal();

//     /*
//       * adjust range deletes as per the compaction
//       */
//     void adjustRangeDeletesForLevel0Input(uint olevel, std::vector<uint64_t>
//     file_numbers); void adjustRangeDeletes(uint clevel, uint olevel,
//     std::vector<std::pair<long long, long long>>
//     one_level_compaction_file_boundaries);

//   public:
//     // std::vector<pll> getRangeDeleteList();
//     void insertRangeDeleteToLevel0(uint64_t file_num, std::vector<pll>
//     &range_delete_list_in, std::vector<uint64_t> exist_level0_file_nums);

//     void addRangeDelete(uint level, long long start, long long end);
//     void addRangeDelete(uint level, std::vector<pll> &range_delete_list_in);
//     void shiftRDFToOutputLevel(std::vector<std::tuple<int, int,
//     std::vector<pll>, std::vector<uint64_t>>> *file_meta_data_vectors); void
//     deleteLastLevelIfEqualsBottomLevel(uint bottom_level);
//     // void deleteRDFAssociatedWithFilesAtCurrentLevel(std::tuple<int, const
//     std::vector<FileMetaData*>*> *file_meta_data); void
//     deleteRDFAssociatedWithFilesAtCurrentLevel(std::tuple<int,
//     std::vector<pll>, std::vector<uint64_t>> *file_meta_data);

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
  static PLRDF_Env* plrdf_Env_ptr;
  bool flag_key_may_deleted = false;  // used during get()
  bool flag_using_partial_string_key = false;

 public:
  static PLRDF_Env* getInstance();

  // Start: used during Get()
  void setFlagKeyMayDeleted(bool flag) { flag_key_may_deleted = flag; }
  bool getFlagKeyMayDeleted() { return flag_key_may_deleted; }
  void clearFlagKeyMayDeleted() {
    flag_key_may_deleted = false;
    // key_search_count_kmd = 0;
  }
  void setUsingPartialStringKey(bool flag) {
    flag_using_partial_string_key = flag;
  }
  bool getUsingPartialStringKey() { return flag_using_partial_string_key; }
  // End: used during Get()
};

class SkyLineRDF {
 private:
  std::vector<t3ll> rd_list;
  std::vector<int> numbers_of_ranges_in_RDF_log;
  std::vector<int> memory_usage_in_RDF_log;

  uint32_t key_search_count = 0, key_may_deleted_count = 0;
  uint32_t key_search_count_kmd = 0;
  bool flag_key_may_deleted = false;

 public:
  // void addRangeDelete(std::vector<t3ll> &range_delete_list_in);
  void addRangeTombstones(std::vector<t3ll> range) {
    if (range.size() == 0) {
      return;
    }

    std::vector<t3ll> tmp_v;  // start, end, seq
    for (auto& r : range) {
      tmp_v.push_back(r);
    }
    for (auto& r : rd_list) {
      tmp_v.push_back(r);
    }

    std::sort(tmp_v.begin(), tmp_v.end());
    std::priority_queue<pll> pq;  // seq, end
    std::vector<t3ll> out_v;      // start, end, seq

    auto t_cur = std::get<0>(tmp_v[0]);
    for (auto& x : tmp_v) {
      auto start = std::get<0>(x);
      auto end = std::get<1>(x);
      auto seq = std::get<2>(x);
      // if(!pq.empty() && +pq.top().second <= start){
      while (!pq.empty() && +pq.top().second <= start) {
        pll p = pq.top();
        pq.pop();
        auto seq2 = +p.first;
        auto end2 = +p.second;
        if (end2 <= t_cur) {
          continue;
        }
        // std::cout << " t_cur = " << t_cur << " end2 = " << end2 << " seq2 = "
        // << seq2 << " start = " << start << " "
        //           << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<
        //           std::endl;
        out_v.push_back(std::make_tuple(t_cur, end2, seq2));
        t_cur = end2;
      }
      // }

      if (pq.empty()) {
        t_cur = start;
      } else {
        auto seq2 = +pq.top().first;
        out_v.push_back(std::make_tuple(t_cur, start, seq2));
        t_cur = start;
      }

      pq.push(std::make_pair(+seq, +end));
    }
    while (!pq.empty()) {
      pll p = pq.top();
      pq.pop();
      auto seq2 = +p.first;
      auto end2 = +p.second;
      if (end2 <= t_cur) {
        continue;
      }

      out_v.push_back(std::make_tuple(t_cur, end2, seq2));
      t_cur = end2;
    }

    std::vector<t3ll> out_v2;
    int len_out_v = out_v.size();
    auto start = std::get<0>(out_v[0]);
    auto end = std::get<1>(out_v[0]);
    auto seq = std::get<2>(out_v[0]);
    for (int i = 1; i < len_out_v; i++) {
      if (std::get<1>(out_v[i - 1]) == std::get<0>(out_v[i]) &&
          std::get<2>(out_v[i - 1]) == std::get<2>(out_v[i])) {
        end = std::get<1>(out_v[i]);
      } else {
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

  bool isEntryAlive(long long key, long long seq) {
    clearFlagKeyMayDeleted();
    incKeySearchCountKMD();
    incKeySearchCount();

    long long skyline__max_seq = getMaxSeq(key);
    bool result = seq >= skyline__max_seq;
    // if (!result) {
    //   setFlagKeyMayDeleted();
    // }
    return result;
  }

  void clearFilterFalsePositiveRate() {
    clearKeySearchCount();
    clearKeyMayDeletedCount();
  }
  double getFilterFalsePositiveRate() {
    uint32_t total_key_search_count = getKeySearchCount();
    if (total_key_search_count == 0) {
      return -1;
    }
    return 1.0 * getKeyMayDeletedCount() / total_key_search_count;
  }
  // use clearFilterFalsePositiveRate for public called
  void clearKeySearchCount() { key_search_count = 0; }
  void incKeySearchCount() {
    if (key_search_count ==
        std::numeric_limits<decltype(key_search_count)>::max()) {
      std::cerr << "Error: overflow of key_search_count " << __FILE__ << ":"
                << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    key_search_count += 1;
  }
  uint32_t getKeySearchCount() { return key_search_count; }
  // use clearFilterFalsePositiveRate for public called
  void clearKeyMayDeletedCount() { key_may_deleted_count = 0; }
  uint32_t getKeyMayDeletedCount() { return key_may_deleted_count; }
  void setFlagKeyMayDeleted() {
    if (key_may_deleted_count ==
        std::numeric_limits<decltype(key_may_deleted_count)>::max()) {
      std::cerr << "Error: overflow of key_may_deleted_count " << __FILE__
                << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    key_may_deleted_count += 1;
    flag_key_may_deleted = true;
  }
  bool getFlagKeyMayDeleted() {
    checkProperUsageOfFlagKeyMayDeleted();
    return flag_key_may_deleted;
  }

  void clearFlagKeyMayDeleted() {
    flag_key_may_deleted = false;
    key_search_count_kmd = 0;
  }
  void incKeySearchCountKMD() { key_search_count_kmd += 1; }
  void checkProperUsageOfFlagKeyMayDeleted() {
    if (key_search_count_kmd > 1) {
      std::cout << "Error: key_may_deleted_count shall be 0 or 1."
                << " Make sure clearFlagKeyMayDeleted is called in the "
                   "beginning of all the series of isEntryAlive functions"
                << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                << std::endl;
      std::cerr << "Error: key_may_deleted_count shall be 0 or 1."
                << " Make sure clearFlagKeyMayDeleted is called in the "
                   "beginning of all the series of isEntryAlive functions"
                << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                << std::endl;
      exit(-1);
    }
  }
  // void print();
  void logCurrentTotalNumbersOfRanges() {
    numbers_of_ranges_in_RDF_log.push_back(getNumberOfTotalRanges());
  }
  std::vector<int> getNumbersOfRangesInRDFLog() {
    return numbers_of_ranges_in_RDF_log;
  }
  void logCurrentTotalMemoryUsage() {
    memory_usage_in_RDF_log.push_back(getNumberOfTotalMemoryUsage());
  }
  std::vector<int> getMemoryUsageInRDFLog() { return memory_usage_in_RDF_log; }

  int getNumberOfTotalRanges() { return rd_list.size(); }

  int getNumberOfTotalMemoryUsage() { return rd_list.size() * sizeof(t3ll); }

  long long getMaxSeq(long long key) {
    auto it =
        std::lower_bound(rd_list.begin(), rd_list.end(), key,
                         [](auto& a, long long b) { return get<1>(a) <= b; });
    if (it == rd_list.end()) {
      return 0;
    }
    // std::cout << " min = " << std::get<0>(*it) << " max = " <<
    // std::get<1>(*it) << " seq = " << std::get<2>(*it) << " " << __FILE__ <<
    // ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

    if (std::get<0>(*it) <= key && key < std::get<1>(*it)) {
      return std::get<2>(*it);
    }
    return 0;
  }

  void print() {
    std::cout << "Skyline RDF" << " " << __FILE__ << ":" << __LINE__ << " "
              << __FUNCTION__ << std::endl;
    // for(auto it = rd_list.begin(); it != rd_list.end(); it++){
    for (auto& x : rd_list) {
      auto start = std::get<0>(x);
      auto end = std::get<1>(x);
      auto seq = std::get<2>(x);
      std::cout << " [" << start << ", " << end << "] --(" << seq << ") ";
    }
    std::cout << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
              << std::endl;
  }
};
// }

// template<typename RangeType>
template <typename KeyType>
class PLRDF_t {
  using Pair = std::pair<KeyType, KeyType>;

 private:
  std::unordered_map<uint64_t, std::vector<Pair>>
      rd_filter_level0;  // for level 0, (file_num, RD_list), FileMetaData* ->
                         // fd .GetNumber();
  // Tracing mechanism: level -> fd -> vector<Pair>
  std::unordered_map<uint64_t, std::vector<Pair>> tracing_rd_filter_level0;
  std::vector<std::map<uint64_t, std::vector<Pair>>>
      tracing_level_file_rd_filter;

  std::vector<std::vector<Pair>>
      rd_filter;  // for level > 0, list of range delete (start, end), all
                  // entries are non-overlapping
  std::vector<std::vector<KeyType>> split_keys;  // for level > 0,
  std::vector<int>
      numbers_of_ranges_in_RDF_log;  // for level > 0, number of ranges in RDF
  std::vector<int> numbers_of_split_keys_in_RDF_log;  // for level > 0, number
                                                      // of split keys in RDF
  std::vector<int> memory_usage_in_RDF_log;

  // int may_be_deleted_count = 0;
  uint32_t key_search_count = 0, key_may_deleted_count = 0;
  uint32_t key_search_count_kmd = 0;
  bool flag_key_may_deleted = false;

  void addRangeDelete_internal(uint level,
                               std::vector<Pair>& range_delete_list_in) {
    // init();
    // update_mutex.lock();
    // std::lock_guard<std::mutex> guard(update_mutex);

    assert(rd_filter.size() >= level);
    while (rd_filter.size() <= level) {
      rd_filter.push_back(std::vector<Pair>());
      split_keys.push_back(std::vector<KeyType>());
    }

    purge_key_on_level(level, range_delete_list_in);
    addRangeDelete(rd_filter[level], range_delete_list_in);

    // update_mutex.unlock();
  }

 public:
  std::vector<Pair> sortAndMerge(std::vector<Pair>& range_delete_list_in) {
    if (range_delete_list_in.size() == 0) {
      return {};
    }

    std::sort(range_delete_list_in.begin(), range_delete_list_in.end(),
              [](Pair a, Pair b) { return a.first < b.first; });

    std::vector<Pair> range_delete_list;
    range_delete_list.reserve(range_delete_list_in.size());
    auto itA = range_delete_list_in.begin();
    auto iteA = range_delete_list_in.end();
    Pair tmp_range = *itA;
    for (; itA != iteA; itA++) {
      if (tmp_range.second >= itA->first) {
        tmp_range.second = std::max(tmp_range.second, itA->second);
      } else {
        range_delete_list.push_back(tmp_range);
        tmp_range = *itA;
      }
    }

    range_delete_list.push_back(tmp_range);
    return range_delete_list;
  }

 private:
  void purge_key_on_level(int level, std::vector<Pair>& range_delete_list_in) {
    // range_delete_list_in is sorted and merged
    if (split_keys.size() <= static_cast<size_t>(level) ||
        split_keys[level].size() == 0) {
      return;
    }
    auto& split_keys_list = split_keys[level];
    std::vector<KeyType> new_split_keys_list;
    new_split_keys_list.reserve(split_keys_list.size());

    auto it_sk = split_keys_list.begin();
    auto it_ske = split_keys_list.end();

    for (auto& range : range_delete_list_in) {
      while (it_sk != it_ske && *it_sk < range.first) {
        new_split_keys_list.push_back(*it_sk);
        it_sk++;
      }
      while (it_sk != it_ske && *it_sk < range.second) {
        it_sk++;
      }
      if (it_sk == it_ske) {
        break;
      }
    }
    while (it_sk != it_ske) {
      new_split_keys_list.push_back(*it_sk);
      it_sk++;
    }
    split_keys[level] = new_split_keys_list;
  }

  void addRangeDelete(std::vector<Pair>& range_delete_list,
                      std::vector<Pair>& range_delete_list_in) {
    // init();
    // std::lock_guard<std::mutex> guard(init_mutex);

    auto& rdList = range_delete_list;
    auto& rdList_in = range_delete_list_in;

    if (rdList_in.size() == 0) {
      return;
    }

    for (uint i = 1; i < rdList_in.size(); i++) {
      if (rdList_in[i - 1].first > rdList_in[i].first) {
        std::cerr << "Error: rdList_in to be flushed has not been sorted in "
                     "ascending order yet"
                  << "\t" << __FILE__ << " " << __LINE__ << " " << __func__
                  << std::endl;
        exit(1);
      }
    }

    std::vector<Pair> rdList_new;
    rdList_new.reserve(rdList.size() + rdList_in.size());

    auto itA = rdList.begin();
    auto iteA = rdList.end();
    auto itB = rdList_in.begin();
    auto iteB = rdList_in.end();
    auto take_next = [&]() -> Pair {
      if (itA == iteA) return *itB++;
      if (itB == iteB) return *itA++;
      if (itA->first <= itB->first) return *itA++;
      return *itB++;
    };

    if (itA == iteA && itB == iteB) return;

    Pair tmp_range = take_next();  // IMPORTANT: advances the chosen iterator

    while (itA != iteA || itB != iteB) {
      Pair nxt = take_next();  // always smallest remaining start

      if (nxt.first <= tmp_range.second) {  // use < if half-open [l,r)
        tmp_range.second = std::max(tmp_range.second, nxt.second);
      } else {
        rdList_new.push_back(tmp_range);
        tmp_range = nxt;
      }
    }

    rdList_new.push_back(tmp_range);
    rdList = std::move(rdList_new);
  }

  void addRangeDelete(std::vector<Pair>& range_delete_list, Pair rd) {
    // init();
    // std::lock_guard<std::mutex> guard(init_mutex);

    auto start = rd.first;
    auto end = rd.second;

    auto& rdList = range_delete_list;
#ifdef DEBUG
    cout << "Adding range delete: " << start << " " << end << endl;
#endif
    std::vector<Pair> rdList_new;
    rdList_new.reserve(rdList.size() + 1);

    auto minK = start;
    auto maxK = end;

    auto it = rdList.begin();
    while (it != rdList.end()) {
      // [a,b], [c,d]

      //[a, b), [c,d)
      // if (it->second < start){ rdList_new.push_back(*it); it++; continue;}
      // if (it->first > end){ break;}
      if (it->second < start) {
        rdList_new.push_back(*it);
        it++;
        continue;
      }
      if (it->first > end) {
        break;
      }

      minK = std::min(minK, it->first);
      maxK = std::max(maxK, it->second);
      it++;
    }
    rdList_new.push_back(Pair({minK, maxK}));
    while (it != rdList.end()) {
      rdList_new.push_back(*it);
      it++;
    }

    rdList.clear();
    rdList.reserve(rdList_new.size());
    for (auto& p : rdList_new) {
      rdList.push_back(p);
    }
  }

  void print_internal() {
    // init();
    // std::lock_guard<std::mutex> guard(update_mutex);

    std::cout << std::setfill('-') << std::setw(60) << " START: Print PL RDF "
              << std::setfill('-') << "" << std::endl;
    for (uint l = 0; l < rd_filter.size(); l++) {
      std::cout << "Level: " << l << std::endl;
      auto& rdList = rd_filter[l];
      for (auto it = rdList.begin(); it != rdList.end(); it++) {
        std::cout << "(" << it->first << " " << it->second << ") ";
      }
      std::cout << std::endl;

      auto& splitKeyList = split_keys[l];
      for (auto it = splitKeyList.begin(); it != splitKeyList.end(); it++) {
        std::cout << (*it) << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::setfill('-') << std::setw(60) << " END: Print PL RDF "
              << std::setfill('-') << "" << std::endl;
  }

  void adjustRangeDeletesForLevel0Input(
      uint olevel, std::vector<uint64_t> file_numbers,
      std::vector<Pair> collected_tombstones) {
    for (uint64_t& file_num : file_numbers) {
      auto it = rd_filter_level0.find(file_num);
      if (it != rd_filter_level0.end()) {
        rd_filter_level0.erase(it);
      }
    }
    addRangeDelete_internal(olevel, collected_tombstones);
  }

  void removeRangeDeletesFromLevel0(std::vector<uint64_t> file_numbers) {
    for (uint64_t& file_num : file_numbers) {
      auto it = rd_filter_level0.find(file_num);
      if (it != rd_filter_level0.end()) {
        rd_filter_level0.erase(it);
      }
    }
  }

  // This would be used for trivial compaction and normal compaction
  // input_level, output_level, file_boundaries
  //   void adjustRangeDeletes(
  //       uint clevel, uint olevel,
  //       std::vector<Pair> one_level_compaction_file_boundaries) {
  //     // // init();
  //     // std::lock_guard<std::mutex> guard(update_mutex);

  //     std::vector<Pair> new_current_level_rdf;
  //     std::vector<Pair> to_be_added_in_next_level_rdf;

  //     if (rd_filter.size() <= clevel) {
  // #ifdef DEBUG
  //       std::cout << "AdjustRangeDeletes: return by rd_filter.size() <=
  //       clevel "
  //                 << "rd_filter.size(): " << rd_filter.size()
  //                 << " clevel: " << clevel << " " << __FILE__ << ":" <<
  //                 __LINE__
  //                 << " " << __FUNCTION__ << std::endl;
  // #endif
  //       return;
  //     }

  //     auto old_current_level_rdf = rd_filter[clevel];

  //     // FIXME: (Shubham) This might not be required
  //     if (one_level_compaction_file_boundaries.size() == 0) {
  //       return;
  //     }

  //     auto it = old_current_level_rdf.begin();
  //     auto itf = one_level_compaction_file_boundaries.begin();

  //     while (it != old_current_level_rdf.end()) {
  //       Pair val = *it;

  //       /*
  //        *    |--|
  //        *         -----
  //        *         |   |
  //        *         -----
  //        */
  //       //  if (itf == one_level_compaction_file_boundaries.end() ||
  //       (val.second
  //       //  <= file_boundry.first))
  //       if (itf == one_level_compaction_file_boundaries.end() ||
  //           (val.second <= itf->first)) {
  //         // new_current_level_rdf.push_back(val);
  //         if (val.first != val.second) {
  //           new_current_level_rdf.push_back(val);
  //         }
  //         it++;
  //         continue;
  //       }

  //       auto file_boundries = *itf;
  //       Pair file_boundry =
  //           std::make_pair(file_boundries.first, file_boundries.second);
  //       /*
  //        *             |--|
  //        *     ------
  //        *     |    |
  //        *     ------
  //        */
  //       if (val.first >= file_boundry.second) {
  //         itf++;
  //       }
  //       /*
  //        *    |------||||
  //        *         ------
  //        *         |    |
  //        *         ------
  //        */
  //       else if (val.first < file_boundry.first &&
  //                val.second > file_boundry.first &&
  //                val.second <= file_boundry.second) {
  //         // new_current_level_rdf.push_back(std::make_pair(val.first,
  //         // file_boundry.first));
  //         //
  //         to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first,
  //         // val.second));
  //         if (val.first != file_boundry.first) {
  //           new_current_level_rdf.push_back(
  //               std::make_pair(val.first, file_boundry.first));
  //         }
  //         if (file_boundry.first != val.second) {
  //           to_be_added_in_next_level_rdf.push_back(
  //               std::make_pair(file_boundry.first, val.second));
  //         }
  //         it++;
  //       }
  //       /*
  //        *    |||--|||
  //        *    --------
  //        *    |      |
  //        *    --------
  //        */
  //       else if (val.first >= file_boundry.first &&
  //                val.second <= file_boundry.second) {
  //         // to_be_added_in_next_level_rdf.push_back(val);
  //         if (val.first != val.second) {
  //           to_be_added_in_next_level_rdf.push_back(val);
  //         }
  //         it++;
  //       }
  //       /*
  //        *     ||||-------|
  //        *     --------
  //        *     |      |
  //        *     --------
  //        */
  //       else if (val.first >= file_boundry.first &&
  //                val.first < file_boundry.second &&
  //                val.second > file_boundry.second) {
  //         //
  //         to_be_added_in_next_level_rdf.push_back(std::make_pair(val.first,
  //         // file_boundry.second + 1));
  //         // (*it).first = file_boundry.second + 1;
  //         to_be_added_in_next_level_rdf.push_back(
  //             std::make_pair(val.first, file_boundry.second));
  //         (*it).first = file_boundry.second;
  //         if ((*it).first >= (*it).second) {  // won't happen here
  //           it++;
  //         }  // <------------------------
  //         itf++;
  //       }
  //       /*
  //        *  |------------|
  //        *     --------
  //        *     |      |
  //        *     --------
  //        */
  //       else if (val.first < file_boundry.first &&
  //                val.second > file_boundry.second) {
  //         // new_current_level_rdf.push_back(std::make_pair(val.first,
  //         // file_boundry.first));
  //         if (val.first != file_boundry.first) {
  //           new_current_level_rdf.push_back(
  //               std::make_pair(val.first, file_boundry.first));
  //         }
  //         //
  //         to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first,
  //         // file_boundry.second + 1));
  //         // (*it).first = file_boundry.second + 1;
  //         to_be_added_in_next_level_rdf.push_back(
  //             std::make_pair(file_boundry.first, file_boundry.second));
  //         (*it).first = file_boundry.second;
  //         if ((*it).first >= (*it).second) {  // won't happen here
  //           it++;
  //         }  // <------------------------
  //         itf++;
  //       } else {
  //         std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__
  //                   << " " << __FUNCTION__ << std::endl;
  //         std::cerr << "val.first: " << val.first << " val.second: " <<
  //         val.second
  //                   << " file_boundry.first: " << file_boundry.first
  //                   << " file_boundry.second: " << file_boundry.second
  //                   << std::endl;
  //         assert(false);
  //         exit(1);
  //       }
  //     }

  //     rd_filter[clevel] = new_current_level_rdf;
  //     std::sort(to_be_added_in_next_level_rdf.begin(),
  //               to_be_added_in_next_level_rdf.end(),
  //               [](const Pair a, const Pair b) { return a.first < b.first;
  //               });

  //     std::cout << "*** clevel = " << clevel << " olevel = " << olevel
  //               << std::endl;
  //     std::cout << "one_level_compaction_file_boundaries.size(): "
  //               << one_level_compaction_file_boundaries.size() << std::endl;
  //     std::cout << "to_be_added_in_next_level_rdf.size(): "
  //               << to_be_added_in_next_level_rdf.size() << std::endl;
  //     std::cout << "one_level_compaction_file_boundaries: " << std::endl;
  //     for (auto& p : one_level_compaction_file_boundaries) {
  //       std::cout << "( " << p.first << " , " << p.second << ") ";
  //     }
  //     std::cout << std::endl;
  //     std::cout << "to_be_added_in_next_level_rdf: " << std::endl;
  //     for (auto& p : to_be_added_in_next_level_rdf) {
  //       std::cout << "( " << p.first << " , " << p.second << ") ";
  //     }
  //     std::cout << std::endl;

  //     assert(one_level_compaction_file_boundaries.size() ==
  //            to_be_added_in_next_level_rdf.size());
  //     for (size_t ic = 0; ic < one_level_compaction_file_boundaries.size();
  //          ic++) {
  //       auto& a = to_be_added_in_next_level_rdf[ic];
  //       auto& b = one_level_compaction_file_boundaries[ic];
  //       if (a.first == b.first && a.second == b.second) {
  //         continue;
  //       }
  //       std::cout << "a = ( " << a.first << " , " << a.second << ") "
  //                 << std::endl;
  //       std::cout << "b = ( " << b.first << " , " << b.second << ") "
  //                 << std::endl;
  //       assert(a.first == b.first && a.second == b.second);
  //     }

  //     addRangeDelete_internal(olevel, to_be_added_in_next_level_rdf);

  //     auto old_current_level_split_keys = split_keys[clevel];
  //     std::vector<KeyType> new_current_level_split_keys;
  //     std::vector<KeyType> to_be_added_in_next_level_split_keys;
  //     auto it_p = old_current_level_split_keys.begin();
  //     auto it_pe = old_current_level_split_keys.end();
  //     auto it_r = new_current_level_rdf.begin();
  //     auto it_re = new_current_level_rdf.end();
  //     // it_p has keys sorted in ascending order
  //     // it_r has ranges sorted in ascending order
  //     // The goal is to populate new_current_level_split_keys with only those
  //     keys
  //     // from old_current_level_split_keys that fall within the given ranges
  //     in
  //     // new_current_level_rdf.
  //     while (it_p != it_pe && it_r != it_re) {
  //       if (*it_p < it_r->first) {
  //         // Key is in a gap (behind the current range), DISCARD it
  //         // to_be_added_in_next_level_split_keys.push_back(*it_p);
  //         ++it_p;
  //       } else if (*it_p >= it_r->second) {
  //         // Range is behind the current key, advance range
  //         ++it_r;
  //       } else {
  //         // *it_p is within [it_r->first, it_r->second), KEEP it
  //         new_current_level_split_keys.push_back(*it_p);
  //         ++it_p;
  //       }
  //     }
  //     // while(it_p != it_pe){
  //     //   to_be_added_in_next_level_split_keys.push_back(*it_p);
  //     //   ++it_p;
  //     // }

  //     split_keys[clevel] = new_current_level_split_keys;
  //   }

  void RemoveRangeDeletesAndSplitKeyFromLevel(
      uint clevel, std::vector<Pair> one_level_compaction_file_boundaries) {
    // // init();
    // std::lock_guard<std::mutex> guard(update_mutex);

    std::vector<Pair> new_current_level_rdf;
    std::vector<Pair> to_be_added_in_next_level_rdf;

    if (rd_filter.size() <= clevel) {
#ifdef DEBUG
      std::cout << "AdjustRangeDeletes: return by rd_filter.size() <= clevel "
                << "rd_filter.size(): " << rd_filter.size()
                << " clevel: " << clevel << " " << __FILE__ << ":" << __LINE__
                << " " << __FUNCTION__ << std::endl;
#endif
      return;
    }

    auto old_current_level_rdf = rd_filter[clevel];

    // FIXME: (Shubham) This might not be required
    if (one_level_compaction_file_boundaries.size() == 0) {
      return;
    }

    auto it = old_current_level_rdf.begin();
    auto itf = one_level_compaction_file_boundaries.begin();

    while (it != old_current_level_rdf.end()) {
      Pair val = *it;

      /*
       *    |--|
       *         -----
       *         |   |
       *         -----
       */
      //  if (itf == one_level_compaction_file_boundaries.end() || (val.second
      //  <= file_boundry.first))
      if (itf == one_level_compaction_file_boundaries.end() ||
          (val.second <= itf->first)) {
        // new_current_level_rdf.push_back(val);
        if (val.first != val.second) {
          new_current_level_rdf.push_back(val);
        }
        it++;
        continue;
      }

      auto file_boundries = *itf;
      Pair file_boundry =
          std::make_pair(file_boundries.first, file_boundries.second);
      /*
       *             |--|
       *     ------
       *     |    |
       *     ------
       */
      if (val.first >= file_boundry.second) {
        itf++;
      }
      /*
       *    |------||||
       *         ------
       *         |    |
       *         ------
       */
      else if (val.first < file_boundry.first &&
               val.second > file_boundry.first &&
               val.second <= file_boundry.second) {
        // new_current_level_rdf.push_back(std::make_pair(val.first,
        // file_boundry.first));
        // to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first,
        // val.second));
        if (val.first != file_boundry.first) {
          new_current_level_rdf.push_back(
              std::make_pair(val.first, file_boundry.first));
        }
        if (file_boundry.first != val.second) {
          to_be_added_in_next_level_rdf.push_back(
              std::make_pair(file_boundry.first, val.second));
        }
        it++;
      }
      /*
       *    |||--|||
       *    --------
       *    |      |
       *    --------
       */
      else if (val.first >= file_boundry.first &&
               val.second <= file_boundry.second) {
        // to_be_added_in_next_level_rdf.push_back(val);
        if (val.first != val.second) {
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
      else if (val.first >= file_boundry.first &&
               val.first < file_boundry.second &&
               val.second > file_boundry.second) {
        // to_be_added_in_next_level_rdf.push_back(std::make_pair(val.first,
        // file_boundry.second + 1));
        // (*it).first = file_boundry.second + 1;
        to_be_added_in_next_level_rdf.push_back(
            std::make_pair(val.first, file_boundry.second));
        (*it).first = file_boundry.second;
        if ((*it).first >= (*it).second) {  // won't happen here
          it++;
        }  // <------------------------
        itf++;
      }
      /*
       *  |------------|
       *     --------
       *     |      |
       *     --------
       */
      else if (val.first < file_boundry.first &&
               val.second > file_boundry.second) {
        // new_current_level_rdf.push_back(std::make_pair(val.first,
        // file_boundry.first));
        if (val.first != file_boundry.first) {
          new_current_level_rdf.push_back(
              std::make_pair(val.first, file_boundry.first));
        }
        // to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first,
        // file_boundry.second + 1));
        // (*it).first = file_boundry.second + 1;
        to_be_added_in_next_level_rdf.push_back(
            std::make_pair(file_boundry.first, file_boundry.second));
        (*it).first = file_boundry.second;
        if ((*it).first >= (*it).second) {  // won't happen here
          it++;
        }  // <------------------------
        itf++;
      } else {
        std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__
                  << " " << __FUNCTION__ << std::endl;
        std::cerr << "val.first: " << val.first << " val.second: " << val.second
                  << " file_boundry.first: " << file_boundry.first
                  << " file_boundry.second: " << file_boundry.second
                  << std::endl;
        assert(false);
        exit(1);
      }
    }

    rd_filter[clevel] = new_current_level_rdf;
    // std::sort(to_be_added_in_next_level_rdf.begin(),
    //           to_be_added_in_next_level_rdf.end(),
    //           [](const Pair a, const Pair b) { return a.first < b.first; });

    // std::cout << "*** clevel = " << clevel << " olevel = " << olevel
    //           << std::endl;
    // std::cout << "one_level_compaction_file_boundaries.size(): "
    //           << one_level_compaction_file_boundaries.size() << std::endl;
    // std::cout << "to_be_added_in_next_level_rdf.size(): "
    //           << to_be_added_in_next_level_rdf.size() << std::endl;
    // std::cout << "one_level_compaction_file_boundaries: " << std::endl;
    // for (auto& p : one_level_compaction_file_boundaries) {
    //   std::cout << "( " << p.first << " , " << p.second << ") ";
    // }
    // std::cout << std::endl;
    // std::cout << "to_be_added_in_next_level_rdf: " << std::endl;
    // for (auto& p : to_be_added_in_next_level_rdf) {
    //   std::cout << "( " << p.first << " , " << p.second << ") ";
    // }
    // std::cout << std::endl;

    // assert(one_level_compaction_file_boundaries.size() ==
    //        to_be_added_in_next_level_rdf.size());
    // for (size_t ic = 0; ic < one_level_compaction_file_boundaries.size();
    //      ic++) {
    //   auto& a = to_be_added_in_next_level_rdf[ic];
    //   auto& b = one_level_compaction_file_boundaries[ic];
    //   if (a.first == b.first && a.second == b.second) {
    //     continue;
    //   }
    //   std::cout << "a = ( " << a.first << " , " << a.second << ") "
    //             << std::endl;
    //   std::cout << "b = ( " << b.first << " , " << b.second << ") "
    //             << std::endl;
    //   assert(a.first == b.first && a.second == b.second);
    // }

    // addRangeDelete_internal(olevel, to_be_added_in_next_level_rdf);

    auto old_current_level_split_keys = split_keys[clevel];
    std::vector<KeyType> new_current_level_split_keys;
    std::vector<KeyType> to_be_added_in_next_level_split_keys;
    auto it_p = old_current_level_split_keys.begin();
    auto it_pe = old_current_level_split_keys.end();
    auto it_r = new_current_level_rdf.begin();
    auto it_re = new_current_level_rdf.end();
    // it_p has keys sorted in ascending order
    // it_r has ranges sorted in ascending order
    // The goal is to populate new_current_level_split_keys with only those keys
    // from old_current_level_split_keys that fall within the given ranges in
    // new_current_level_rdf.
    while (it_p != it_pe && it_r != it_re) {
      if (*it_p < it_r->first) {
        // Key is in a gap (behind the current range), DISCARD it
        // to_be_added_in_next_level_split_keys.push_back(*it_p);
        ++it_p;
      } else if (*it_p >= it_r->second) {
        // Range is behind the current key, advance range
        ++it_r;
      } else {
        // *it_p is within [it_r->first, it_r->second), KEEP it
        new_current_level_split_keys.push_back(*it_p);
        ++it_p;
      }
    }
    // while(it_p != it_pe){
    //   to_be_added_in_next_level_split_keys.push_back(*it_p);
    //   ++it_p;
    // }

    split_keys[clevel] = new_current_level_split_keys;
  }

 public:
  std::vector<Pair> getOverlappingRanges(uint32_t level, const KeyType& start,
                                         const KeyType& end) const {
    std::vector<Pair> result;
    if (level >= rd_filter.size()) return result;

    const auto& ranges = rd_filter[level];

    Pair search_val;
    search_val.first = start;

    auto it = std::lower_bound(
        ranges.begin(), ranges.end(), search_val,
        [](const Pair& a, const Pair& b) { return a.first < b.first; });

    if (it != ranges.begin()) {
      auto prev = std::prev(it);
      if (prev->second > start) {
        result.push_back(*prev);
      }
    }

    while (it != ranges.end() && it->first < end) {
      result.push_back(*it);
      it++;
    }

    return result;
  }

  // Functional retrieval method
  std::vector<Pair> getRangesForFile(uint32_t level, uint64_t fd,
                                     const KeyType& start,
                                     const KeyType& end) const {
    if (level == 0) {
      if (rd_filter_level0.count(fd)) {
        return rd_filter_level0.at(fd);
      }
      return {};
    } else {
      return getOverlappingRanges(level, start, end);
    }
  }

  void insertRangeDeleteToLevel0(uint64_t file_num,
                                 std::vector<Pair>& range_delete_list_in,
                                 std::vector<uint64_t> exist_level0_file_nums) {
    std::vector<Pair> sorted_merged_rdlist = sortAndMerge(range_delete_list_in);

    if (std::binary_search(exist_level0_file_nums.begin(),
                           exist_level0_file_nums.end(), file_num) == true) {
      std::cerr << "Error: file_num already exists in rd_filter_level0 "
                << "file_num = " << file_num << "\t" << __FILE__ << ":"
                << __LINE__ << " " << __func__ << std::endl;
      exit(1);
    }
    rd_filter_level0[file_num] = sorted_merged_rdlist;
  }

  void printLevel0() {
    // init();
    // std::lock_guard<std::mutex> guard(rd_filter_level0_mutex);

    std::cout << "rd_filter_level0" << " " << __FILE__ << ":" << __LINE__ << " "
              << __FUNCTION__ << std::endl
              << std::endl;
    for (auto it = rd_filter_level0.begin(); it != rd_filter_level0.end();
         it++) {
      std::cout << "rd_filter_level0 file number: " << it->first
                << " number of RD: " << it->second.size() << std::endl;
      // print all ranges
      for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
        std::cout << "rd_filter_level0" << " " << it2->first << " "
                  << it2->second << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl << std::endl;
  }

  void addRangeDelete(uint level, std::vector<Pair>& range_delete_list_in) {
    // init();
    // // update_mutex.lock();
    // std::lock_guard<std::mutex> guard(update_mutex);

    while (rd_filter.size() <= level) {
      rd_filter.push_back(std::vector<Pair>());
    }

    addRangeDelete(rd_filter[level], range_delete_list_in);

    // update_mutex.unlock();
  }

  /*
   *Do insertion, even if the vector is empty, because we need to set
   * condition_variable of mutex (semaphore) for compaction
   */
  // input_level, output_level, file_boundries, file_numbers
  void shiftRDFToOutputLevel(
      const std::vector<
          std::tuple<int, int, std::vector<Pair>, std::vector<uint64_t>>>*
          file_meta_data_vectors,
      std::vector<Pair>& sorted_merged_rdlist_outlevel) {
    // init();
    // std::lock_guard<std::mutex> guard(update_mutex);

    if (file_meta_data_vectors->empty()) {
      return;
    }

    for (auto file_meta_data : *file_meta_data_vectors) {
      int clevel = std::get<0>(file_meta_data);

      std::vector<uint64_t>& src_fd_list = std::get<3>(file_meta_data);

      std::vector<Pair>& range_list = std::get<2>(file_meta_data);
      std::vector<Pair> sorted_merged_rdlist = sortAndMerge(range_list);
      if (clevel == 0) {
        // adjustRangeDeletesForLevel0Input(olevel, src_fd_list,
        //                                  sorted_merged_rdlist);
        removeRangeDeletesFromLevel0(src_fd_list);
      } else {
        // if (sorted_merged_rdlist.size()) {
        //   std::cout << "(compaction) remove rt @" << clevel << " " <<
        //   __FILE__
        //             << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        //   std::cout << "-";
        //   for (auto& rt : sorted_merged_rdlist) {
        //     std::cout << "[" << rt.first << "," << rt.second << ") ";
        //   }
        //   std::cout << std::endl;
        // }
        // adjustRangeDeletes(clevel, olevel, sorted_merged_rdlist);
        RemoveRangeDeletesAndSplitKeyFromLevel(clevel, sorted_merged_rdlist);
      }
    }
    if (sorted_merged_rdlist_outlevel.size()) {
      int olevel = std::get<1>((*file_meta_data_vectors)[0]);
      // std::cout << "(compaction) add rt @" << olevel << " " << __FILE__ <<
      // ":"
      //           << __LINE__ << " " << __FUNCTION__ << std::endl;
      // std::cout << "+";
      // for (auto& rt : sorted_merged_rdlist_outlevel) {
      //   std::cout << "[" << rt.first << "," << rt.second << ") ";
      // }
      // std::cout << std::endl;
      addRangeDelete_internal(olevel, sorted_merged_rdlist_outlevel);
    }

    // // // FIXME: FOR TESTING (next 2 lines)
    // // std::cout << "After Comapction" << " " << __FILE__ << ":" << __LINE__
    // << " " << __FUNCTION__ << std::endl;
    // // // update_mutex.lock();
    // print_internal();
    // // update_mutex.unlock();
  }

  // this is only used for direct compaction //
  // input_level, file_boundries, file_numbers
  void deleteRDFAssociatedWithFilesAtCurrentLevel(
      std::tuple<int, std::vector<Pair>, std::vector<uint64_t>>*
          file_meta_data) {
    // init();
    // std::lock_guard<std::mutex> guard(update_mutex);

    // std::cout << "Before Deletion Comapction" << " " << __FILE__ << ":" <<
    // __LINE__ << " " << __FUNCTION__ << std::endl; print_internal();

    std::vector<Pair> one_level_file_boundries;
    auto level = std::get<0>(*file_meta_data);

    if (level == 0) {
      std::vector<uint64_t> fd_list = std::get<2>(*file_meta_data);
      for (auto fd : fd_list) {
        auto it = rd_filter_level0.find(fd);
        if (it == rd_filter_level0.end()) {
          assert(it != rd_filter_level0.end());
          std::cerr << "Error: file_num does not exist in rd_filter_level0"
                    << "\t" << __FILE__ << " " << __LINE__ << " " << __func__
                    << std::endl;
          exit(1);
        }
        rd_filter_level0.erase(it);
      }
      return;
    }

    if (rd_filter.size() <= (uint)level) {
      return;
    }

    auto meta_data = std::get<1>(*file_meta_data);

    for (auto meta : meta_data) {
      one_level_file_boundries.push_back(
          std::make_pair(meta.first, meta.second));
    }
    one_level_file_boundries = sortAndMerge(one_level_file_boundries);

    // std::cout << "!!! delete RDF Accociate RangeTombstones on level " <<
    // level
    //           << " " << __FILE__ << ":" << __LINE__ << " " << __func__
    //           << std::endl;
    // for (auto it : one_level_file_boundries) {
    //   std::cout << "[ " << it.first << " " << it.second << " ) ";
    // }
    // std::cout << std::endl;

    std::vector<Pair> new_current_level_rdf;
    auto old_current_level_rdf = rd_filter[level];
    auto it = old_current_level_rdf.begin();
    auto itf = one_level_file_boundries.begin();

    while (it != old_current_level_rdf.end()) {
      Pair val = *it;
      if (itf == one_level_file_boundries.end()) {
        new_current_level_rdf.push_back(val);
        it++;
        continue;
      }
      auto file_boundries = *itf;
      Pair file_boundry =
          std::make_pair(file_boundries.first, file_boundries.second);

      /*
       *    |--|
       *         -----
       *         |   |
       *         -----
       */
      if (val.second <= file_boundry.first) {
        new_current_level_rdf.push_back(val);
        it++;
      }
      /*
       *             |--|
       *     ------
       *     |    |
       *     ------
       */
      else if (val.first >= file_boundry.second) {
        itf++;
      }
      /*
       *    |------||||
       *         ------
       *         |    |
       *         ------
       */
      else if (val.first < file_boundry.first &&
               val.second > file_boundry.first &&
               val.second <= file_boundry.second) {
        new_current_level_rdf.push_back(
            std::make_pair(val.first, file_boundry.first));
        it++;
      }
      /*
       *    |||--|||
       *    --------
       *    |      |
       *    --------
       */
      else if (val.first >= file_boundry.first &&
               val.second <= file_boundry.second) {
        it++;
      }
      /*
       *     ||||-------|
       *     --------
       *     |      |
       *     --------
       */
      else if (val.first >= file_boundry.first &&
               val.first < file_boundry.second &&
               val.second > file_boundry.second) {
        // (*it).first = file_boundry.second + 1;
        (*it).first = file_boundry.second;
        if ((*it).first >= (*it).second) {  // won't happen here
          it++;
        }  // <------------------------
        itf++;
      }
      /*
       *  |------------|
       *     --------
       *     |      |
       *     --------
       */
      else if (val.first < file_boundry.first &&
               val.second > file_boundry.second) {
        new_current_level_rdf.push_back(
            std::make_pair(val.first, file_boundry.first));
        // (*it).first = file_boundry.second + 1;
        (*it).first = file_boundry.second;
        if ((*it).first >= (*it).second) {  // won't happen here
          it++;
        }  // <------------------------
        itf++;
      } else {
        std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__
                  << " " << __FUNCTION__ << std::endl;
        std::cerr << "val.first: " << val.first << " val.second: " << val.second
                  << " file_boundry.first: " << file_boundry.first
                  << " file_boundry.second: " << file_boundry.second
                  << std::endl;
        assert(false);
        exit(1);
      }
    }

    rd_filter[level] = new_current_level_rdf;

    // std::cout << "After Deletion Comapction" << " " << __FILE__ << ":" <<
    // __LINE__ << " " << __FUNCTION__ << std::endl; print_internal();

    auto old_current_level_split_keys = split_keys[level];
    std::vector<KeyType> new_current_level_split_keys;
    auto it_p = old_current_level_split_keys.begin();
    auto it_pe = old_current_level_split_keys.end();
    auto it_r = new_current_level_rdf.begin();
    auto it_re = new_current_level_rdf.end();
    // it_p has keys sorted in ascending order
    // it_r has ranges sorted in ascending order
    // The goal is to populate new_current_level_split_keys with only those keys
    // from old_current_level_split_keys that fall within the given ranges in
    // new_current_level_rdf.
    while (it_p != it_pe && it_r != it_re) {
      // if (*it_p < it_r->first) {
      if (*it_p < it_r->first) {
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

  vector<Pair> getLevelRanges(int outlevel) {
    // init();
    // std::lock_guard<std::mutex> guard(update_mutex);
    if ((uint)outlevel >= rd_filter.size()) {
      return {};
    }
    return rd_filter[outlevel];
  }
  void setLevelRanges(vector<Pair> level_ranges_in, int outlevel) {
    // init();
    // std::lock_guard<std::mutex> guard(update_mutex);
    while (rd_filter.size() <= (uint)outlevel) {
      rd_filter.push_back(vector<Pair>());
    }

    if (level_ranges_in != rd_filter[outlevel]) {
      std::cout << "^^^ outlevel: " << outlevel
                << " level_ranges_in.size(): " << level_ranges_in.size()
                << " rd_filter[outlevel].size() " << rd_filter[outlevel].size()
                << std::endl;
      std::cout << "^^^ level_ranges_in: " << std::endl;
      for (auto it = level_ranges_in.begin(); it != level_ranges_in.end();
           it++) {
        std::cout << "(" << it->first << " " << it->second << ") ";
      }
      std::cout << std::endl;
      std::cout << "^^^ rd_filter[outlevel]: " << std::endl;
      for (auto it = rd_filter[outlevel].begin();
           it != rd_filter[outlevel].end(); it++) {
        std::cout << "(" << it->first << " " << it->second << ") ";
      }
      std::cout << std::endl;
    }

    rd_filter[outlevel] = level_ranges_in;
  }

  int getNumberOfTotalLevels() {
    int num = 0;
    int len = rd_filter.size();
    for (int i = 0; i < len; i++) {
      if (rd_filter[i].size() > 0) {
        num = i + 1;
      }
    }
    return num;
  }

  int getNumberOfTotalRanges() {
    int num = 0;
    for (auto it = rd_filter.begin(); it != rd_filter.end(); it++) {
      num += it->size();
    }
    return num;
  }

  int getNumberOfTotalSplitKeys() {
    int num = 0;
    for (auto it = split_keys.begin(); it != split_keys.end(); it++) {
      num += it->size();
    }
    return num;
  }

  int getNumberOfTotalMemoryUsage() {
    int num = 0;

    if constexpr (std::is_same<KeyType, std::string>::value) {
      // std::cout << "KeyType is std::string\n";
      for (auto it = rd_filter.begin(); it != rd_filter.end(); it++) {
        for (auto it2 : *it) {
          num += (it2.first).size() +
                 (it2.second).size();  // size of a range: sizeof(Pair)
        }
      }
      for (auto it = split_keys.begin(); it != split_keys.end(); it++) {
        for (auto it2 : *it) {
          num += it2.size();  // size of a key: sizeof(KeyType)
        }
      }
    } else {
      // std::cout << "KeyType is not std::string\n";
      for (auto it = rd_filter.begin(); it != rd_filter.end(); it++) {
        num += sizeof(it->first) +
               sizeof(it->second);  // size of a range: sizeof(Pair)
      }
      for (auto it = split_keys.begin(); it != split_keys.end(); it++) {
        num += sizeof(*it);  // size of a key: sizeof(KeyType)
      }
    }

    return num;
  }

  void print() {
    // init();
    // std::lock_guard<std::mutex> guard(update_mutex);

    std::cout << std::setfill('-') << std::setw(60) << " START: Print  RDF "
              << std::setfill('-') << "" << std::endl;
    for (uint l = 0; l < rd_filter.size(); l++) {
      std::cout << "Level: " << l << std::endl;
      auto& rdList = rd_filter[l];
      for (auto it = rdList.begin(); it != rdList.end(); it++) {
        std::cout << "(" << it->first << " " << it->second << ") ";
      }
      std::cout << std::endl;
    }
    std::cout << std::setfill('-') << std::setw(60) << " END: Print  RDF "
              << std::setfill('-') << "" << std::endl;
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

  void clearFilterFalsePositiveRate() {
    clearKeySearchCount();
    clearKeyMayDeletedCount();
  }
  double getFilterFalsePositiveRate() {
    uint32_t total_key_search_count = getKeySearchCount();
    if (total_key_search_count == 0) {
      return -1;
    }
    return 1.0 * getKeyMayDeletedCount() / total_key_search_count;
  }
  // use clearFilterFalsePositiveRate for public called
  void clearKeySearchCount() { key_search_count = 0; }
  void incKeySearchCount() {
    if (key_search_count ==
        std::numeric_limits<decltype(key_search_count)>::max()) {
      std::cerr << "Error: overflow of key_search_count " << __FILE__ << ":"
                << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    key_search_count += 1;
  }
  uint32_t getKeySearchCount() { return key_search_count; }
  // use clearFilterFalsePositiveRate for public called
  void clearKeyMayDeletedCount() { key_may_deleted_count = 0; }
  uint32_t getKeyMayDeletedCount() { return key_may_deleted_count; }
  void setFlagKeyMayDeleted() {
    if (key_may_deleted_count ==
        std::numeric_limits<decltype(key_may_deleted_count)>::max()) {
      std::cerr << "Error: overflow of key_may_deleted_count " << __FILE__
                << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    key_may_deleted_count += 1;
    flag_key_may_deleted = true;
  }
  bool getFlagKeyMayDeleted() {
    checkProperUsageOfFlagKeyMayDeleted();
    return flag_key_may_deleted;
  }

  void clearFlagKeyMayDeleted() {
    flag_key_may_deleted = false;
    key_search_count_kmd = 0;
  }
  void incKeySearchCountKMD() { key_search_count_kmd += 1; }
  void checkProperUsageOfFlagKeyMayDeleted() {
    if (key_search_count_kmd > 1) {
      std::cout << "Error: key_may_deleted_count shall be 0 or 1."
                << " Make sure clearFlagKeyMayDeleted is called in the "
                   "beginning of all the series of isEntryAlive functions"
                << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                << std::endl;
      std::cerr << "Error: key_may_deleted_count shall be 0 or 1."
                << " Make sure clearFlagKeyMayDeleted is called in the "
                   "beginning of all the series of isEntryAlive functions"
                << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                << std::endl;
      exit(-1);
    }
  }

  bool isEntryAlive(uint level, KeyType key) {
    // init();
    // std::lock_guard<std::mutex> guard(update_mutex);
    clearFlagKeyMayDeleted();
    incKeySearchCountKMD();
    incKeySearchCount();

    assert(rd_filter.size() > level);

    if (level >= rd_filter.size()) {
      return true;
    }

    // range tombstones
    auto& rdList = rd_filter[level];
    if (rdList.size() == 0) {
      return true;
    }

    // split keys
    auto& splitKeyList = split_keys[level];
    auto it_p = lower_bound(splitKeyList.begin(), splitKeyList.end(), key);
    // if key exact the same return true
    if (it_p != splitKeyList.end() && *it_p == key) {
      return true;
    }

    // range tombstones
    auto it = upper_bound(
        rdList.begin(), rdList.end(), Pair(key, key),
        [](const Pair& a, const Pair& b) { return a.first < b.first; });
    if (it != rdList.begin()) {
      it--;
    }

    if (PLRDF_Env::getInstance()->getUsingPartialStringKey()) {
      if (key == it->first || key == it->second) {
        setFlagKeyMayDeleted();
      }
      if (key >= it->first && key < it->second) {
        return false;
      }
      return true;
    } else {
      if (key >= it->first && key < it->second) {
        return false;
      }
    }
    return true;
  }

  std::vector<Pair> getAllFileRangeTombstones(uint32_t level) {
    if (level >= tracing_level_file_rd_filter.size()) return {};
    std::vector<Pair> all_ranges;
    for (auto const& entry : tracing_level_file_rd_filter[level]) {
      all_ranges.insert(all_ranges.end(), entry.second.begin(),
                        entry.second.end());
    }
    return sortAndMerge(all_ranges);
  }

  void gatherSortedRangeTombstonesAndRemoveRDF(
      uint clevel, std::vector<uint64_t> file_numbers,
      std::vector<Pair> file_boundaries, std::vector<Pair>& gathered_rd_list) {
    if (clevel == 0) {
      for (uint64_t& file_num : file_numbers) {
        auto it = rd_filter_level0.find(file_num);
        if (it != rd_filter_level0.end()) {
          gathered_rd_list.insert(gathered_rd_list.end(), it->second.begin(),
                                  it->second.end());
          rd_filter_level0.erase(it);
        }
      }
    } else {
      if (rd_filter.size() <= clevel) {
        return;
      }
      std::vector<Pair> new_current_level_rdf;
      auto& old_current_level_rdf = rd_filter[clevel];

      for (auto& range : old_current_level_rdf) {
        bool overlapped = false;
        for (auto& boundary : file_boundaries) {
          if (range.second > boundary.first && range.first < boundary.second) {
            overlapped = true;
            break;
          }
        }

        if (overlapped) {
          // Fragment the range if it spans outside file boundaries
          for (auto& boundary : file_boundaries) {
            KeyType start = std::max(range.first, boundary.first);
            KeyType end = std::min(range.second, boundary.second);
            if (start < end) {
              gathered_rd_list.push_back({start, end});
            }
          }
        } else {
          new_current_level_rdf.push_back(range);
        }
      }
      rd_filter[clevel] = new_current_level_rdf;
    }
  }

  // void shiftRDFToOutputLevel(uint olevel, std::vector<Pair> gathered_rd_list,
  //                            std::vector<Pair> output_file_boundaries,
  //                            std::vector<uint64_t> output_file_numbers) {
  //   if (gathered_rd_list.empty()) {
  //     return;
  //   }

  //   std::vector<Pair> sorted_merged_rdlist = sortAndMerge(gathered_rd_list);

  //   if (olevel == 0) {
  //     assert(output_file_numbers.size() == 1);
  //     uint64_t file_num = output_file_numbers[0];
  //     // Clip to output file boundary if provided
  //     if (!output_file_boundaries.empty()) {
  //       std::vector<Pair> clipped_rd_list;
  //       Pair boundary = output_file_boundaries[0];
  //       for (auto& range : sorted_merged_rdlist) {
  //         KeyType start = std::max(range.first, boundary.first);
  //         KeyType end = std::min(range.second, boundary.second);
  //         if (start < end) {
  //           clipped_rd_list.push_back({start, end});
  //         }
  //       }
  //       rd_filter_level0[file_num] = clipped_rd_list;
  //     } else {
  //       rd_filter_level0[file_num] = sorted_merged_rdlist;
  //     }
  //   } else {
  //     std::vector<Pair> to_be_added;
  //     if (!output_file_boundaries.empty()) {
  //       for (auto& boundary : output_file_boundaries) {
  //         for (auto& range : sorted_merged_rdlist) {
  //           KeyType start = std::max(range.first, boundary.first);
  //           KeyType end = std::min(range.second, boundary.second);
  //           if (start < end) {
  //             to_be_added.push_back({start, end});
  //           }
  //         }
  //       }
  //     } else {
  //       to_be_added = sorted_merged_rdlist;
  //     }
  //     addRangeDelete_internal(olevel, to_be_added);
  //   }
  // }

  void deleteLastLevelIfEqualsBottomLevel(uint bottom_level) {
    // init();
    // std::lock_guard<std::mutex> guard(update_mutex);

    if (rd_filter.size() - 1 == bottom_level) {
      rd_filter[bottom_level].clear();
      split_keys[bottom_level].clear();
    }
  }

  // TODO:
  void splitRangesOnLevel(uint level, std::vector<KeyType> keys) {
    if (rd_filter.size() <= level) {
      // std::cerr << "Error: splitRangesOnLevel: level: " << level
      //           << " is not present in PLRDF" << " " << __FILE__ << ":"
      //           << __LINE__ << " " << __FUNCTION__ << std::endl;
      // exit(1);
      return;
    }

    // auto &rdList = rd_filter[level];
    //   std::vector<Pair> rdList_new;
    auto& splitKeyList = split_keys[level];
    std::vector<KeyType> splitKeysList_new;

    auto& rangeList = rd_filter[level];
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
        // Equal keys ??advance both
        candidate = *it1;
        ++it1;
        ++it2;
      }

      // only storing split keys covered by some ranges
      {
        while (it_r != it_re && it_r->second <= candidate) {
          it_r++;
        }
        if (it_r == it_re) {
          break;
        }
        if (candidate < it_r->first) {
          continue;
        }
      }

      // Avoid inserting duplicate consecutive values
      if (splitKeysList_new.empty() ||
          (candidate != splitKeysList_new.back())) {
        splitKeysList_new.push_back(candidate);
      }
    }
    split_keys[level] = splitKeysList_new;
  }

  void logCurrentTotalNumbersOfRanges() {
    numbers_of_ranges_in_RDF_log.push_back(getNumberOfTotalRanges());
  }

  std::vector<int> getNumbersOfRangesInRDFLog() {
    if (numbers_of_ranges_in_RDF_log.size() == 0) {
      return {};
    } else {
      return numbers_of_ranges_in_RDF_log;
    }
  }

  void logCurrentTotalNumbersOfSplitKeys() {
    numbers_of_split_keys_in_RDF_log.push_back(getNumberOfTotalSplitKeys());
  }

  std::vector<int> getNumbersOfSplitKeysInRDFLog() {
    if (numbers_of_split_keys_in_RDF_log.size() == 0) {
      return {};
    } else {
      return numbers_of_split_keys_in_RDF_log;
    }
  }

  void logCurrentTotalMemoryUsage() {
    memory_usage_in_RDF_log.push_back(getNumberOfTotalMemoryUsage());
  }
  std::vector<int> getMemoryUsageInRDFLog() { return memory_usage_in_RDF_log; }

  // Tracing logic
  // don't use any tracing logic members
  // std::vector<Pair> getTracingRangesForFile(uint32_t level, uint64_t fd)
  // const {
  //   if (level == 0) {
  //     if (tracing_rd_filter_level0.count(fd)) {
  //       return tracing_rd_filter_level0.at(fd);
  //     }
  //   } else {
  //     if (level < tracing_level_file_rd_filter.size() &&
  //         tracing_level_file_rd_filter[level].count(fd)) {
  //       return tracing_level_file_rd_filter[level].at(fd);
  //     }
  //   }
  //   return {};
  // }

  void insertTracingRangesAtLevelOfFd(uint32_t level, uint64_t fd,
                                      std::vector<Pair>& ranges) {
    if (ranges.size() == 0) {
      return;
    }
    if (level == 0) {
      tracing_rd_filter_level0[fd] = ranges;
    } else {
      while (level >= tracing_level_file_rd_filter.size()) {
        tracing_level_file_rd_filter.push_back(
            std::map<uint64_t, std::vector<Pair>>());
      }
      // std::cout << "(Tracing) insertRT @" << level << " fd = " << fd <<
      // __FILE__
      //           << ":" << __LINE__ << std::endl;
      // std::cout << "(Tracing) +" << std::endl;
      // auto& ranges_to_insert = ranges;
      // for (auto& rt : ranges_to_insert) {
      //   std::cout << "[" << rt.first << ", " << rt.second << ") ";
      // }
      // std::cout << std::endl;
      tracing_level_file_rd_filter[level][fd] = ranges;
    }
  }

  void removeTracingRangesAtLevelOfFd(uint32_t level, uint64_t fd) {
    if (level == 0) {
      tracing_rd_filter_level0.erase(fd);
    } else if (level < tracing_level_file_rd_filter.size()) {
      // std::cout << "(Tracing) removeRT @" << level << " fd = " << fd <<
      // __FILE__
      //           << ":" << __LINE__ << std::endl;
      // std::cout << "(Tracing) -" << std::endl;
      // auto& ranges_to_remove = tracing_level_file_rd_filter[level][fd];
      // for (auto& rt : ranges_to_remove) {
      //   std::cout << "[" << rt.first << ", " << rt.second << ") ";
      // }
      // std::cout << std::endl;
      tracing_level_file_rd_filter[level].erase(fd);
    }
  }

  std::vector<Pair> gatherTracingRangesAndRemove(
      std::vector<uint32_t>& src_level_list,
      std::vector<std::vector<uint64_t>>& src_fd_list2d) {
    assert(src_level_list.size() == src_fd_list2d.size());
    std::vector<Pair> range_tombstone_list_agg;

    for (size_t i = 0; i < src_level_list.size(); i++) {
      uint32_t level = src_level_list[i];
      for (uint64_t fd : src_fd_list2d[i]) {
        if (level == 0) {
          if (tracing_rd_filter_level0.count(fd)) {
            auto& rds = tracing_rd_filter_level0[fd];
            range_tombstone_list_agg.insert(range_tombstone_list_agg.end(),
                                            rds.begin(), rds.end());
            tracing_rd_filter_level0.erase(fd);
          }
        }
        if (level < tracing_level_file_rd_filter.size()) {
          auto it = tracing_level_file_rd_filter[level].find(fd);
          if (it != tracing_level_file_rd_filter[level].end()) {
            range_tombstone_list_agg.insert(range_tombstone_list_agg.end(),
                                            it->second.begin(),
                                            it->second.end());
            // std::cout << "(Tracing) removeRT @" << level << " fd = " << fd
            //           << __FILE__ << ":" << __LINE__ << std::endl;
            // std::cout << "(Tracing) -" << std::endl;
            // auto& ranges_to_remove = tracing_level_file_rd_filter[level][fd];
            // for (auto& rt : ranges_to_remove) {
            //   std::cout << "[" << rt.first << ", " << rt.second << ") ";
            // }
            // std::cout << std::endl;
            tracing_level_file_rd_filter[level].erase(it);
          }
        }
      }
    }

    return sortAndMerge(range_tombstone_list_agg);
  }

  void shiftTracingToOutputLevel(std::vector<Pair>& rd_merged,
                                 uint32_t dst_level,
                                 std::vector<uint64_t>& dst_fd_list,
                                 std::vector<Pair>& file_boundary_list) {
    if (rd_merged.size() == 0) return;
    assert(dst_fd_list.size() == file_boundary_list.size());

    size_t i_rd = 0;
    size_t len_rd = rd_merged.size();
    size_t len_dst = dst_fd_list.size();

    for (size_t i_dst = 0; i_dst < len_dst; i_dst++) {
      if (i_rd >= len_rd) break;

      uint64_t dst_fd = dst_fd_list[i_dst];
      Pair file_boundary = file_boundary_list[i_dst];
      if (file_boundary.first == file_boundary.second) continue;

      std::vector<Pair> ranges_to_insert;
      // Skip ranges that are strictly before the file boundary
      while (i_rd < len_rd && rd_merged[i_rd].second <= file_boundary.first) {
        i_rd++;
      }

      // Collect/clip ranges that overlap with the file boundary
      while (i_rd < len_rd && rd_merged[i_rd].first < file_boundary.second) {
        Pair range_in = std::make_pair(
            std::max(rd_merged[i_rd].first, file_boundary.first),
            std::min(rd_merged[i_rd].second, file_boundary.second));
        ranges_to_insert.push_back(range_in);

        // If the current gathered range extends beyond the current output
        // file, don't advance i_rd yet because it might overlap with the next
        // output file.
        if (rd_merged[i_rd].second > file_boundary.second) {
          break;
        }
        i_rd++;
      }

      if (ranges_to_insert.size() > 0) {
        this->insertTracingRangesAtLevelOfFd(dst_level, dst_fd,
                                             ranges_to_insert);
      }
    }
  }

  void setTracingForFile(uint32_t level, uint64_t fd,
                         const std::vector<Pair>& ranges) {
    if (level >= tracing_level_file_rd_filter.size()) {
      tracing_level_file_rd_filter.resize(level + 1);
    }
    tracing_level_file_rd_filter[level][fd] = ranges;
  }

  void removeTracingForFile(uint32_t level, uint64_t fd) {
    if (level < tracing_level_file_rd_filter.size()) {
      tracing_level_file_rd_filter[level].erase(fd);
    }
  }

  void checkTracingConsistency(uint32_t level) {
    if (level >= rd_filter.size()) return;
    auto actual = rd_filter[level];
    auto traced = getAllFileRangeTombstones(level);
    size_t len_actual = actual.size();
    size_t len_traced = traced.size();
    bool flag_inconsistency = false;

    if (len_actual != len_traced) {
      flag_inconsistency = true;
      std::cerr << "\t\t\t len_actual " << len_actual << " != " << "len_traced "
                << len_traced << std::endl;
      // rd_filter[level] = traced;
    }

    for (size_t i = 0; i < min(len_actual, len_traced); i++) {
      if (actual[i] != traced[i]) {
        flag_inconsistency = true;
        auto& r1 = actual[i];
        auto& r2 = traced[i];
        std::cerr << "\t\t\tactual " << i << ": " << "[" << r1.first << ","
                  << r1.second << ")"
                  << " != traced " << i << ": " << "[" << r2.first << ","
                  << r2.second << ")" << std::endl;
      }
    }
    if (flag_inconsistency) {
      std::cerr << "\t\t\t*** >>> Consistency Check FAILED for level " << level
                << std::endl;
      std::cerr << "\t\t\tActual size: " << actual.size()
                << ", Traced size: " << traced.size() << std::endl;
      std::cerr << "\t\t\tActual: ";
      for (auto const& r : actual)
        std::cerr << "[" << r.first << "," << r.second << ") ";
      std::cerr << std::endl << "\t\t\tTraced: ";
      for (auto const& r : traced)
        std::cerr << "[" << r.first << "," << r.second << ") ";
      std::cerr << std::endl;

      // std::cerr << "\t\t\t List All Level" << std::endl;
      // for (size_t lvl = 0; lvl < rd_filter.size(); lvl++) {
      //   auto actual2 = rd_filter[lvl];
      //   auto traced2 = getAllFileRangeTombstones(lvl);
      //   std::cerr << "\t\t\t\t lvl = " << lvl << std::endl;
      //   std::cerr << "\t\t\t\tActual size: " << actual2.size()
      //             << ", Traced size: " << traced2.size() << std::endl;
      //   std::cerr << "\t\t\t\tActual: ";
      //   for (auto const& r : actual2)
      //     std::cerr << "[" << r.first << "," << r.second << ") ";
      //   std::cerr << std::endl << "\t\t\t\tTraced: ";
      //   for (auto const& r : traced2)
      //     std::cerr << "[" << r.first << "," << r.second << ") ";
      //   std::cerr << std::endl;
      // }
    }
  }

  bool isEntryAliveWithTracingAtLevelOfFd(uint level, uint64_t fd,
                                          KeyType key) {
    if (level == 0) {
      if (tracing_rd_filter_level0.count(fd)) {
        auto& rdList = tracing_rd_filter_level0[fd];
        auto it = upper_bound(
            rdList.begin(), rdList.end(), Pair(key, key),
            [](const Pair& a, const Pair& b) { return a.first < b.first; });
        if (it != rdList.begin()) it--;
        if (it != rdList.end() && key >= it->first && key < it->second) {
          return false;
        }
      }
      return true;
    }

    if (level < tracing_level_file_rd_filter.size()) {
      auto it_map = tracing_level_file_rd_filter[level].find(fd);
      if (it_map != tracing_level_file_rd_filter[level].end()) {
        auto& rdList = it_map->second;
        auto it = upper_bound(
            rdList.begin(), rdList.end(), Pair(key, key),
            [](const Pair& a, const Pair& b) { return a.first < b.first; });
        if (it != rdList.begin()) it--;
        if (it != rdList.end() && key >= it->first && key < it->second) {
          return false;
        }
      }
    }
    return true;
  }
};

class PLRDF {
 private:
  std::unordered_map<uint64_t, std::vector<pll>>
      rd_filter_level0;  // for level 0, (file_num, RD_list), FileMetaData* ->
                         // fd .GetNumber();

  std::vector<std::vector<pll>>
      rd_filter;  // for level > 0, list of range delete (start, end), all
                  // entries are non-overlapping
  std::vector<std::vector<long long>> split_keys;
  std::unordered_map<uint64_t, std::vector<pll>> tracing_rd_filter_level0;
  std::vector<std::map<uint64_t, std::vector<pll>>>
      tracing_level_file_rd_filter;
  std::vector<int>
      numbers_of_ranges_in_RDF_log;  // for level > 0, number of ranges in RDF
  std::vector<int> memory_usage_in_RDF_log;

  uint32_t key_search_count = 0, key_may_deleted_count = 0;
  uint32_t key_search_count_kmd = 0;
  bool flag_key_may_deleted = false;

  void addRangeDelete_internal(uint level,
                               std::vector<pll>& range_delete_list_in) {
    // init();
    // update_mutex.lock();
    // std::lock_guard<std::mutex> guard(update_mutex);

    assert(rd_filter.size() >= level);
    assert(split_keys.size() >= level);
    while (rd_filter.size() <= level) {
      rd_filter.push_back(std::vector<pll>());
    }
    while (split_keys.size() <= level) {
      split_keys.push_back(std::vector<long long>());
    }

    purge_key_on_level(level, range_delete_list_in);
    addRangeDelete(rd_filter[level], range_delete_list_in);

    // update_mutex.unlock();
  }

  // ToDO: string cannot be merged on the boundary overlap
 public:
  std::vector<pll> sortAndMerge(std::vector<pll>& range_delete_list_in) {
    if (range_delete_list_in.size() == 0) {
      return {};
    }

    std::sort(range_delete_list_in.begin(), range_delete_list_in.end(),
              [](pll a, pll b) { return a.first < b.first; });

    std::vector<pll> range_delete_list;
    range_delete_list.reserve(range_delete_list_in.size());
    auto itA = range_delete_list_in.begin();
    auto iteA = range_delete_list_in.end();
    pll tmp_range = *itA;
    for (; itA != iteA; itA++) {
      if (tmp_range.second >= itA->first) {
        tmp_range.second = std::max(tmp_range.second, itA->second);
      } else {
        range_delete_list.push_back(tmp_range);
        tmp_range = *itA;
      }
    }

    range_delete_list.push_back(tmp_range);
    return range_delete_list;
  }

 private:
  void purge_key_on_level(int level, std::vector<pll>& range_delete_list_in) {
    // range_delete_list_in is sorted and merged
    if (split_keys.size() <= static_cast<size_t>(level) ||
        split_keys[level].size() == 0) {
      return;
    }
    auto& split_keys_list = split_keys[level];
    std::vector<long long> new_split_keys_list;
    new_split_keys_list.reserve(split_keys_list.size());

    auto it_sk = split_keys_list.begin();
    auto it_ske = split_keys_list.end();

    for (auto& range : range_delete_list_in) {
      while (it_sk != it_ske && *it_sk < range.first) {
        new_split_keys_list.push_back(*it_sk);
        it_sk++;
      }
      while (it_sk != it_ske && *it_sk < range.second) {
        it_sk++;
      }
      if (it_sk == it_ske) {
        break;
      }
    }
    while (it_sk != it_ske) {
      new_split_keys_list.push_back(*it_sk);
      it_sk++;
    }
    split_keys[level] = new_split_keys_list;
  }

  void addRangeDelete(std::vector<pll>& range_delete_list,
                      std::vector<pll>& range_delete_list_in) {
    // init();
    // std::lock_guard<std::mutex> guard(init_mutex);

    auto& rdList = range_delete_list;
    auto& rdList_in = range_delete_list_in;

    if (rdList_in.size() == 0) {
      return;
    }

    for (uint i = 1; i < rdList_in.size(); i++) {
      if (rdList_in[i - 1].first > rdList_in[i].first) {
        std::cerr << "Error: rdList_in to be flushed has not been sorted in "
                     "ascending order yet"
                  << "\t" << __FILE__ << " " << __LINE__ << " " << __func__
                  << std::endl;
        exit(1);
      }
    }

    std::vector<pll> rdList_new;
    rdList_new.reserve(rdList.size() + rdList_in.size());

    auto itA = rdList.begin();
    auto iteA = rdList.end();
    auto itB = rdList_in.begin();
    auto iteB = rdList_in.end();

    auto take_next = [&]() -> pll {
      if (itA == iteA) return *itB++;
      if (itB == iteB) return *itA++;
      if (itA->first <= itB->first) return *itA++;
      return *itB++;
    };

    if (itA == iteA && itB == iteB) return;

    pll tmp_range = take_next();  // IMPORTANT: advances the chosen iterator

    while (itA != iteA || itB != iteB) {
      pll nxt = take_next();  // always smallest remaining start

      if (nxt.first <= tmp_range.second) {  // use < if half-open [l,r)
        tmp_range.second = std::max(tmp_range.second, nxt.second);
      } else {
        rdList_new.push_back(tmp_range);
        tmp_range = nxt;
      }
    }

    rdList_new.push_back(tmp_range);
    rdList = std::move(rdList_new);
  }

  // // void addRangeDelete(std::vector<pll> &range_delete_list, long long
  // start, long long end){ void addRangeDelete(std::vector<pll>
  // &range_delete_list, pll rd){
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

  void print_internal() {
    // init();
    // std::lock_guard<std::mutex> guard(update_mutex);

    std::cout << std::setfill('-') << std::setw(60) << " START: Print PL RDF "
              << std::setfill('-') << "" << std::endl;
    for (uint l = 0; l < rd_filter.size(); l++) {
      std::cout << "Level: " << l << std::endl;
      auto& rdList = rd_filter[l];
      for (auto it = rdList.begin(); it != rdList.end(); it++) {
        std::cout << "(" << it->first << " " << it->second << ") ";
      }
      std::cout << std::endl;
    }
    std::cout << std::setfill('-') << std::setw(60) << " END: Print PL RDF "
              << std::setfill('-') << "" << std::endl;
  }

  void adjustRangeDeletesForLevel0Input(uint olevel,
                                        std::vector<uint64_t> file_numbers,
                                        std::vector<pll> collected_tombstones) {
    for (uint64_t& file_num : file_numbers) {
      auto it = rd_filter_level0.find(file_num);
      if (it != rd_filter_level0.end()) {
        rd_filter_level0.erase(it);
      }
    }
    addRangeDelete_internal(olevel, collected_tombstones);
  }

  void removeRangeDeletesFromLevel0(std::vector<uint64_t> file_numbers) {
    for (uint64_t& file_num : file_numbers) {
      auto it = rd_filter_level0.find(file_num);
      if (it != rd_filter_level0.end()) {
        rd_filter_level0.erase(it);
      }
    }
  }

  // This would be used for trivial compaction and normal compaction
  // input_level, output_level, file_boundaries
  void adjustRangeDeletes(
      uint clevel, uint olevel,
      std::vector<pll> one_level_compaction_file_boundaries) {
    // // init();
    // std::lock_guard<std::mutex> guard(update_mutex);

    std::vector<pll> new_current_level_rdf;
    std::vector<pll> to_be_added_in_next_level_rdf;

    if (rd_filter.size() <= clevel) {
#ifdef DEBUG
      std::cout << "AdjustRangeDeletes: return by rd_filter.size() <= clevel "
                << "rd_filter.size(): " << rd_filter.size()
                << " clevel: " << clevel << " " << __FILE__ << ":" << __LINE__
                << " " << __FUNCTION__ << std::endl;
#endif
      return;
    }

    auto old_current_level_rdf = rd_filter[clevel];

    // FIXME: (Shubham) This might not be required
    if (one_level_compaction_file_boundaries.size() == 0) {
      return;
    }

    auto it = old_current_level_rdf.begin();
    auto itf = one_level_compaction_file_boundaries.begin();

    while (it != old_current_level_rdf.end()) {
      pll val = *it;
      /*
       *    |--|
       *         -----
       *         |   |
       *         -----
       */
      //  if (itf == one_level_compaction_file_boundaries.end() || (val.second
      //  <= file_boundry.first))
      if (itf == one_level_compaction_file_boundaries.end() ||
          (val.second <= itf->first)) {
        // new_current_level_rdf.push_back(val);
        if (val.first != val.second) {
          new_current_level_rdf.push_back(val);
        }
        it++;
        continue;
      }

      auto file_boundries = *itf;
      pll file_boundry =
          std::make_pair(file_boundries.first, file_boundries.second);

      /*
       *             |--|
       *     ------
       *     |    |
       *     ------
       */
      if (val.first >= file_boundry.second) {
        itf++;
      }
      /*
       *    |------||||
       *         ------
       *         |    |
       *         ------
       */
      else if (val.first < file_boundry.first &&
               val.second > file_boundry.first &&
               val.second <= file_boundry.second) {
        // new_current_level_rdf.push_back(std::make_pair(val.first,
        // file_boundry.first));
        // to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first,
        // val.second));
        if (val.first != file_boundry.first) {
          new_current_level_rdf.push_back(
              std::make_pair(val.first, file_boundry.first));
        }
        if (file_boundry.first != val.second) {
          to_be_added_in_next_level_rdf.push_back(
              std::make_pair(file_boundry.first, val.second));
        }
        it++;
      }
      /*
       *    |||--|||
       *    --------
       *    |      |
       *    --------
       */
      else if (val.first >= file_boundry.first &&
               val.second <= file_boundry.second) {
        // to_be_added_in_next_level_rdf.push_back(val);
        if (val.first != val.second) {
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
      else if (val.first >= file_boundry.first &&
               val.first <= file_boundry.second &&
               val.second > file_boundry.second) {
        to_be_added_in_next_level_rdf.push_back(
            std::make_pair(val.first, file_boundry.second));
        (*it).first = file_boundry.second;
        if ((*it).first >= (*it).second) {
          it++;
        }  // <------------------------
        itf++;
      }
      /*
       *  |------------|
       *     --------
       *     |      |
       *     --------
       */
      else if (val.first < file_boundry.first &&
               val.second > file_boundry.second) {
        // new_current_level_rdf.push_back(std::make_pair(val.first,
        // file_boundry.first));
        if (val.first != file_boundry.first) {
          new_current_level_rdf.push_back(
              std::make_pair(val.first, file_boundry.first));
        }
        to_be_added_in_next_level_rdf.push_back(
            std::make_pair(file_boundry.first, file_boundry.second));
        (*it).first = file_boundry.second;
        if ((*it).first >= (*it).second) {
          it++;
        }  // <------------------------
        itf++;
      } else {
        std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__
                  << " " << __FUNCTION__ << std::endl;
        std::cerr << "val.first: " << val.first << " val.second: " << val.second
                  << " file_boundry.first: " << file_boundry.first
                  << " file_boundry.second: " << file_boundry.second
                  << std::endl;
        assert(false);
        exit(1);
      }
    }

    rd_filter[clevel] = new_current_level_rdf;
    std::sort(to_be_added_in_next_level_rdf.begin(),
              to_be_added_in_next_level_rdf.end(),
              [](const pll a, const pll b) { return a.first < b.first; });

    addRangeDelete_internal(olevel, to_be_added_in_next_level_rdf);

    auto old_current_level_split_keys = split_keys[clevel];
    std::vector<long long> new_current_level_split_keys;
    std::vector<long long> to_be_added_in_next_level_split_keys;
    auto it_p = old_current_level_split_keys.begin();
    auto it_pe = old_current_level_split_keys.end();
    auto it_r = new_current_level_rdf.begin();
    auto it_re = new_current_level_rdf.end();
    // it_p has keys sorted in ascending order
    // it_r has ranges sorted in ascending order
    // The goal is to populate new_current_level_split_keys with only those
    // keys from old_current_level_split_keys that fall within the given
    // ranges in new_current_level_rdf.
    while (it_p != it_pe && it_r != it_re) {
      if (*it_p < it_r->first) {
        // Key is in a gap (behind the current range), DISCARD it
        // to_be_added_in_next_level_split_keys.push_back(*it_p);
        ++it_p;
      } else if (*it_p >= it_r->second) {
        // Range is behind the current key, advance range
        ++it_r;
      } else {
        // *it_p is within [it_r->first, it_r->second), KEEP it
        new_current_level_split_keys.push_back(*it_p);
        ++it_p;
      }
    }
    // while(it_p != it_pe){
    //   to_be_added_in_next_level_split_keys.push_back(*it_p);
    //   ++it_p;
    // }

    split_keys[clevel] = new_current_level_split_keys;
  }

  std::vector<pll> getAllFileRangeTombstones(uint32_t level) {
    if (level >= tracing_level_file_rd_filter.size()) return {};
    std::vector<pll> all_ranges;
    for (auto const& entry : tracing_level_file_rd_filter[level]) {
      all_ranges.insert(all_ranges.end(), entry.second.begin(),
                        entry.second.end());
    }
    return sortAndMerge(all_ranges);
  }

 public:
  std::vector<pll> getOverlappingRanges(uint32_t level, long long start,
                                        long long end) const {
    std::vector<pll> result;
    if (level >= rd_filter.size()) return result;

    const auto& ranges = rd_filter[level];

    auto it = std::lower_bound(
        ranges.begin(), ranges.end(), std::make_pair(start, (long long)0),
        [](const pll& a, const pll& b) { return a.first < b.first; });

    if (it != ranges.begin()) {
      auto prev = std::prev(it);
      if (prev->second > start) {
        result.push_back(*prev);
      }
    }

    while (it != ranges.end() && it->first < end) {
      result.push_back(*it);
      it++;
    }

    return result;
  }

  std::vector<pll> getRangesForFile(uint32_t level, uint64_t fd,
                                    long long start, long long end) const {
    if (level == 0) {
      if (rd_filter_level0.count(fd)) {
        return rd_filter_level0.at(fd);
      }
      return {};
    } else {
      return getOverlappingRanges(level, start, end);
    }
  }

  void insertRangeDeleteToLevel0(uint64_t file_num,
                                 std::vector<pll>& range_delete_list_in,
                                 std::vector<uint64_t> exist_level0_file_nums) {
    std::vector<pll> sorted_merged_rdlist = sortAndMerge(range_delete_list_in);
    // init();

    // -- updating rd_filter_level0 --
    // std::lock_guard<std::mutex> guard(rd_filter_level0_mutex);
    // rd_filter_level0_mutex.lock();

    // if(rd_filter_level0.count(file_num) > 0){
    if (std::binary_search(exist_level0_file_nums.begin(),
                           exist_level0_file_nums.end(), file_num) == true) {
      std::cerr << "Error: file_num already exists in rd_filter_level0 "
                << "file_num = " << file_num << "\t" << __FILE__ << ":"
                << __LINE__ << " " << __func__ << std::endl;
      exit(1);
    }
    rd_filter_level0[file_num] = sorted_merged_rdlist;

    // rd_filter_level0_mutex.unlock();
    // -- updating rd_filter_level0 --
  }

  void printLevel0() {
    // init();
    // std::lock_guard<std::mutex> guard(rd_filter_level0_mutex);

    std::cout << "rd_filter_level0" << " " << __FILE__ << ":" << __LINE__ << " "
              << __FUNCTION__ << std::endl
              << std::endl;
    for (auto it = rd_filter_level0.begin(); it != rd_filter_level0.end();
         it++) {
      std::cout << "rd_filter_level0 file number: " << it->first
                << " number of RD: " << it->second.size() << std::endl;
      // print all ranges
      for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
        std::cout << "rd_filter_level0" << " " << it2->first << " "
                  << it2->second << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl << std::endl;
  }

  void addRangeDelete(uint level, std::vector<pll>& range_delete_list_in) {
    // init();
    // // update_mutex.lock();
    // std::lock_guard<std::mutex> guard(update_mutex);

    while (rd_filter.size() <= level) {
      rd_filter.push_back(std::vector<pll>());
    }

    addRangeDelete(rd_filter[level], range_delete_list_in);

    // update_mutex.unlock();
  }

  // void shiftRDFToOutputLevel(std::vector<pll>& rd_merged, uint32_t dst_level,
  //                            std::vector<uint64_t>& dst_fd_list,
  //                            std::vector<pll>& file_boundary_list) {
  //   if (rd_merged.size() == 0) return;
  //   assert(dst_fd_list.size() == file_boundary_list.size());

  //   size_t i_rd = 0;
  //   size_t len_rd = rd_merged.size();
  //   size_t len_dst = dst_fd_list.size();

  //   for (size_t i_dst = 0; i_dst < len_dst; i_dst++) {
  //     if (i_rd >= len_rd) break;

  //     uint64_t dst_fd = dst_fd_list[i_dst];
  //     pll file_boundary = file_boundary_list[i_dst];
  //     if (file_boundary.first == file_boundary.second) continue;

  //     std::vector<pll> ranges_to_insert;
  //     // Skip ranges that are strictly before the file boundary
  //     while (i_rd < len_rd && rd_merged[i_rd].second <= file_boundary.first)
  //     {
  //       i_rd++;
  //     }

  //     // Collect/clip ranges that overlap with the file boundary
  //     while (i_rd < len_rd && rd_merged[i_rd].first < file_boundary.second) {
  //       pll range_in = std::make_pair(
  //           std::max(rd_merged[i_rd].first, file_boundary.first),
  //           std::min(rd_merged[i_rd].second, file_boundary.second));
  //       ranges_to_insert.push_back(range_in);

  //       // If the current gathered range extends beyond the current output
  //       file,
  //       // don't advance i_rd yet because it might overlap with the next
  //       output
  //       // file.
  //       if (rd_merged[i_rd].second > file_boundary.second) {
  //         break;
  //       }
  //       i_rd++;
  //     }

  //     if (ranges_to_insert.size() > 0) {
  //       this->insertRangesAtLevelOfFd(dst_level, dst_fd, ranges_to_insert);
  //       // Also update the aggregate filter to maintain compatibility for now
  //       this->addRangeDelete_internal(dst_level, ranges_to_insert);
  //     }
  //   }
  // }

  void RemoveRangeDeletesAndSplitKeyFromLevel(
      uint clevel, std::vector<pll> one_level_compaction_file_boundaries) {
    // // init();
    // std::lock_guard<std::mutex> guard(update_mutex);

    std::vector<pll> new_current_level_rdf;
    std::vector<pll> to_be_added_in_next_level_rdf;

    if (rd_filter.size() <= clevel) {
#ifdef DEBUG
      std::cout << "AdjustRangeDeletes: return by rd_filter.size() <= clevel "
                << "rd_filter.size(): " << rd_filter.size()
                << " clevel: " << clevel << " " << __FILE__ << ":" << __LINE__
                << " " << __FUNCTION__ << std::endl;
#endif
      return;
    }

    auto old_current_level_rdf = rd_filter[clevel];

    // FIXME: (Shubham) This might not be required
    if (one_level_compaction_file_boundaries.size() == 0) {
      return;
    }

    auto it = old_current_level_rdf.begin();
    auto itf = one_level_compaction_file_boundaries.begin();

    while (it != old_current_level_rdf.end()) {
      pll val = *it;

      /*
       *    |--|
       *         -----
       *         |   |
       *         -----
       */
      //  if (itf == one_level_compaction_file_boundaries.end() || (val.second
      //  <= file_boundry.first))
      if (itf == one_level_compaction_file_boundaries.end() ||
          (val.second <= itf->first)) {
        // new_current_level_rdf.push_back(val);
        if (val.first != val.second) {
          new_current_level_rdf.push_back(val);
        }
        it++;
        continue;
      }

      auto file_boundries = *itf;
      pll file_boundry =
          std::make_pair(file_boundries.first, file_boundries.second);
      /*
       *             |--|
       *     ------
       *     |    |
       *     ------
       */
      if (val.first >= file_boundry.second) {
        itf++;
      }
      /*
       *    |------||||
       *         ------
       *         |    |
       *         ------
       */
      else if (val.first < file_boundry.first &&
               val.second > file_boundry.first &&
               val.second <= file_boundry.second) {
        // new_current_level_rdf.push_back(std::make_pair(val.first,
        // file_boundry.first));
        // to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first,
        // val.second));
        if (val.first != file_boundry.first) {
          new_current_level_rdf.push_back(
              std::make_pair(val.first, file_boundry.first));
        }
        if (file_boundry.first != val.second) {
          to_be_added_in_next_level_rdf.push_back(
              std::make_pair(file_boundry.first, val.second));
        }
        it++;
      }
      /*
       *    |||--|||
       *    --------
       *    |      |
       *    --------
       */
      else if (val.first >= file_boundry.first &&
               val.second <= file_boundry.second) {
        // to_be_added_in_next_level_rdf.push_back(val);
        if (val.first != val.second) {
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
      else if (val.first >= file_boundry.first &&
               val.first < file_boundry.second &&
               val.second > file_boundry.second) {
        // to_be_added_in_next_level_rdf.push_back(std::make_pair(val.first,
        // file_boundry.second + 1));
        // (*it).first = file_boundry.second + 1;
        to_be_added_in_next_level_rdf.push_back(
            std::make_pair(val.first, file_boundry.second));
        (*it).first = file_boundry.second;
        if ((*it).first >= (*it).second) {  // won't happen here
          it++;
        }  // <------------------------
        itf++;
      }
      /*
       *  |------------|
       *     --------
       *     |      |
       *     --------
       */
      else if (val.first < file_boundry.first &&
               val.second > file_boundry.second) {
        // new_current_level_rdf.push_back(std::make_pair(val.first,
        // file_boundry.first));
        if (val.first != file_boundry.first) {
          new_current_level_rdf.push_back(
              std::make_pair(val.first, file_boundry.first));
        }
        // to_be_added_in_next_level_rdf.push_back(std::make_pair(file_boundry.first,
        // file_boundry.second + 1));
        // (*it).first = file_boundry.second + 1;
        to_be_added_in_next_level_rdf.push_back(
            std::make_pair(file_boundry.first, file_boundry.second));
        (*it).first = file_boundry.second;
        if ((*it).first >= (*it).second) {  // won't happen here
          it++;
        }  // <------------------------
        itf++;
      } else {
        std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__
                  << " " << __FUNCTION__ << std::endl;
        std::cerr << "val.first: " << val.first << " val.second: " << val.second
                  << " file_boundry.first: " << file_boundry.first
                  << " file_boundry.second: " << file_boundry.second
                  << std::endl;
        assert(false);
        exit(1);
      }
    }

    rd_filter[clevel] = new_current_level_rdf;

    auto old_current_level_split_keys = split_keys[clevel];
    std::vector<long long> new_current_level_split_keys;
    auto it_p = old_current_level_split_keys.begin();
    auto it_pe = old_current_level_split_keys.end();
    auto it_r = new_current_level_rdf.begin();
    auto it_re = new_current_level_rdf.end();
    // it_p has keys sorted in ascending order
    // it_r has ranges sorted in ascending order
    // The goal is to populate new_current_level_split_keys with only those keys
    // from old_current_level_split_keys that fall within the given ranges in
    // new_current_level_rdf.
    while (it_p != it_pe && it_r != it_re) {
      if (*it_p < it_r->first) {
        // Key is in a gap (behind the current range), DISCARD it
        // to_be_added_in_next_level_split_keys.push_back(*it_p);
        ++it_p;
      } else if (*it_p >= it_r->second) {
        // Range is behind the current key, advance range
        ++it_r;
      } else {
        // *it_p is within [it_r->first, it_r->second), KEEP it
        new_current_level_split_keys.push_back(*it_p);
        ++it_p;
      }
    }

    split_keys[clevel] = new_current_level_split_keys;
  }

  // input_level, output_level, file_boundries, file_numbers
  void shiftRDFToOutputLevel(
      const std::vector<
          std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>>*
          file_meta_data_vectors,
      std::vector<pll>& sorted_merged_rdlist_outlevel) {
    // init();
    // std::lock_guard<std::mutex> guard(update_mutex);

    // // // FIXME: FOR TESTING (next 2 lines)
    if (file_meta_data_vectors->empty()) {
      return;
    }

    for (auto file_meta_data : *file_meta_data_vectors) {
      int clevel = std::get<0>(file_meta_data);
      std::vector<pll>& range_list = std::get<2>(file_meta_data);
      std::vector<pll> sorted_merged_rdlist = sortAndMerge(range_list);
      if (clevel == 0) {
        // adjustRangeDeletesForLevel0Input(olevel, std::get<3>(file_meta_data),
        //                                  sorted_merged_rdlist);
        removeRangeDeletesFromLevel0(std::get<3>(file_meta_data));

      } else {
        // adjustRangeDeletes(std::get<0>(file_meta_data),
        //                    std::get<1>(file_meta_data),
        //                    sorted_merged_rdlist);
        // adjustRangeDeletes(clevel, olevel, sorted_merged_rdlist);
        // if (sorted_merged_rdlist.size()) {
        //   std::cout << "(compaction) remove rt @" << clevel << " " <<
        //   __FILE__
        //             << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        //   std::cout << "-";
        //   for (auto& rt : sorted_merged_rdlist) {
        //     std::cout << "[" << rt.first << "," << rt.second << ") ";
        //   }
        //   std::cout << std::endl;
        // }
        RemoveRangeDeletesAndSplitKeyFromLevel(clevel, sorted_merged_rdlist);
      }
    }
    if (sorted_merged_rdlist_outlevel.size()) {
      int olevel = std::get<1>((*file_meta_data_vectors)[0]);
      // std::cout << "(compaction) add rt @" << olevel << " " << __FILE__ <<
      // ":"
      //           << __LINE__ << " " << __FUNCTION__ << std::endl;
      // std::cout << "+";
      // for (auto& rt : sorted_merged_rdlist_outlevel) {
      //   std::cout << "[" << rt.first << "," << rt.second << ") ";
      // }
      // std::cout << std::endl;
      addRangeDelete_internal(olevel, sorted_merged_rdlist_outlevel);
    }
  }

  // this is only used for direct compaction //
  // input_level, file_boundries, file_numbers
  void deleteRDFAssociatedWithFilesAtCurrentLevel(
      std::tuple<int, std::vector<pll>, std::vector<uint64_t>>*
          file_meta_data) {
    // init();
    // std::lock_guard<std::mutex> guard(update_mutex);

    // std::cout << "Before Deletion Comapction" << " " << __FILE__ << ":" <<
    // __LINE__ << " " << __FUNCTION__ << std::endl; print_internal();

    std::vector<std::pair<long long, long long>> one_level_file_boundries;
    auto level = std::get<0>(*file_meta_data);

    if (level == 0) {
      std::vector<uint64_t> fd_list = std::get<2>(*file_meta_data);
      for (auto fd : fd_list) {
        auto it = rd_filter_level0.find(fd);
        if (it == rd_filter_level0.end()) {
          assert(it != rd_filter_level0.end());
          std::cerr << "Error: file_num does not exist in rd_filter_level0"
                    << "\t" << __FILE__ << " " << __LINE__ << " " << __func__
                    << std::endl;
          exit(1);
        }
        rd_filter_level0.erase(it);
      }
      return;
    }

    if (rd_filter.size() <= (uint)level) {
      return;
    }

    auto meta_data = std::get<1>(*file_meta_data);

    for (auto meta : meta_data) {
      one_level_file_boundries.push_back(
          std::make_pair(meta.first, meta.second));
    }
    one_level_file_boundries = sortAndMerge(one_level_file_boundries);

    std::vector<pll> new_current_level_rdf;
    auto old_current_level_rdf = rd_filter[level];
    auto it = old_current_level_rdf.begin();
    auto itf = one_level_file_boundries.begin();

    while (it != old_current_level_rdf.end()) {
      pll val = *it;
      if (itf == one_level_file_boundries.end()) {
        new_current_level_rdf.push_back(val);
        it++;
        continue;
      }
      auto file_boundries = *itf;
      pll file_boundry =
          std::make_pair(file_boundries.first, file_boundries.second);

      /*
       *    |--|
       *         -----
       *         |   |
       *         -----
       */
      if (val.second <= file_boundry.first) {
        new_current_level_rdf.push_back(val);
        it++;
      }
      /*
       *             |--|
       *     ------
       *     |    |
       *     ------
       */
      else if (val.first >= file_boundry.second) {
        itf++;
      }
      /*
       *    |------||||
       *         ------
       *         |    |
       *         ------
       */
      else if (val.first < file_boundry.first &&
               val.second > file_boundry.first &&
               val.second <= file_boundry.second) {
        new_current_level_rdf.push_back(
            std::make_pair(val.first, file_boundry.first));
        it++;
      }
      /*
       *    |||--|||
       *    --------
       *    |      |
       *    --------
       */
      else if (val.first >= file_boundry.first &&
               val.second <= file_boundry.second) {
        it++;
      }
      /*
       *     ||||-------|
       *     --------
       *     |      |
       *     --------
       */
      else if (val.first >= file_boundry.first &&
               val.first <= file_boundry.second &&
               val.second > file_boundry.second) {
        (*it).first = file_boundry.second;
        if ((*it).first >= (*it).second) {
          it++;
        }  // <------------------------
        itf++;
      }
      /*
       *  |------------|
       *     --------
       *     |      |
       *     --------
       */
      else if (val.first < file_boundry.first &&
               val.second > file_boundry.second) {
        new_current_level_rdf.push_back(
            std::make_pair(val.first, file_boundry.first));
        (*it).first = file_boundry.second;
        if ((*it).first >= (*it).second) {
          it++;
        }  // <------------------------
        itf++;
      } else {
        std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__
                  << " " << __FUNCTION__ << std::endl;
        std::cerr << "val.first: " << val.first << " val.second: " << val.second
                  << " file_boundry.first: " << file_boundry.first
                  << " file_boundry.second: " << file_boundry.second
                  << std::endl;
        assert(false);
        exit(1);
      }
    }

    rd_filter[level] = new_current_level_rdf;

    auto old_current_level_split_keys = split_keys[level];
    std::vector<long long> new_current_level_split_keys;
    auto it_p = old_current_level_split_keys.begin();
    auto it_pe = old_current_level_split_keys.end();
    auto it_r = new_current_level_rdf.begin();
    auto it_re = new_current_level_rdf.end();
    // it_p has keys sorted in ascending order
    // it_r has ranges sorted in ascending order
    // The goal is to populate new_current_level_split_keys with only those
    // keys from old_current_level_split_keys that fall within the given
    // ranges in new_current_level_rdf.
    while (it_p != it_pe && it_r != it_re) {
      if (*it_p < it_r->first) {
        // Key is in a gap (behind the current range), DISCARD it
        ++it_p;
      } else if (*it_p >= it_r->second) {
        // Range is behind the current key, advance range
        ++it_r;
      } else {
        // *it_p is within [it_r->first, it_r->second), KEEP it
        new_current_level_split_keys.push_back(*it_p);
        ++it_p;
      }
    }
    split_keys[level] = new_current_level_split_keys;
  }

  vector<pll> getLevelRanges(int outlevel) {
    // init();
    // std::lock_guard<std::mutex> guard(update_mutex);
    if ((uint)outlevel >= rd_filter.size()) {
      return {};
    }
    return rd_filter[outlevel];
  }

  void setLevelRanges(vector<pll> level_ranges_in, int outlevel) {
    // init();
    // std::lock_guard<std::mutex> guard(update_mutex);
    while (rd_filter.size() <= (uint)outlevel) {
      rd_filter.push_back(vector<pll>());
    }

    if (level_ranges_in != rd_filter[outlevel]) {
      std::cout << "^^^ outlevel: " << outlevel
                << " level_ranges_in.size(): " << level_ranges_in.size()
                << " rd_filter[outlevel].size() " << rd_filter[outlevel].size()
                << std::endl;
      std::cout << "^^^ level_ranges_in: " << std::endl;
      for (auto it = level_ranges_in.begin(); it != level_ranges_in.end();
           it++) {
        std::cout << "(" << it->first << " " << it->second << ") ";
      }
      std::cout << std::endl;
      std::cout << "^^^ rd_filter[outlevel]: " << std::endl;
      for (auto it = rd_filter[outlevel].begin();
           it != rd_filter[outlevel].end(); it++) {
        std::cout << "(" << it->first << " " << it->second << ") ";
      }
      std::cout << std::endl;
    }

    rd_filter[outlevel] = level_ranges_in;
  }

  int getNumberOfTotalLevels() {
    int num = 0;
    int len = rd_filter.size();
    for (int i = 0; i < len; i++) {
      if (rd_filter[i].size() > 0) {
        num = i + 1;
      }
    }
    return num;
  }

  int getNumberOfTotalRanges() {
    int num = 0;
    for (auto it = rd_filter.begin(); it != rd_filter.end(); it++) {
      num += it->size();
    }
    return num;
  }

  int getNumberOfTotalMemoryUsage() {
    int num = 0;
    for (auto it = rd_filter.begin(); it != rd_filter.end(); it++) {
      num += it->size();
    }
    return num * sizeof(pll);  // size of a ranges is the size of pair<long
                               // long, long long> = 16 bytes
  }

  void print() {
    // init();
    // std::lock_guard<std::mutex> guard(update_mutex);

    std::cout << std::setfill('-') << std::setw(60) << " START: Print  RDF "
              << std::setfill('-') << "" << std::endl;
    for (uint l = 0; l < rd_filter.size(); l++) {
      std::cout << "Level: " << l << std::endl;
      auto& rdList = rd_filter[l];
      for (auto it = rdList.begin(); it != rdList.end(); it++) {
        std::cout << "(" << it->first << " " << it->second << ") ";
      }
      std::cout << std::endl;
    }
    std::cout << std::setfill('-') << std::setw(60) << " END: Print  RDF "
              << std::setfill('-') << "" << std::endl;
  }

  void clearFilterFalsePositiveRate() {
    clearKeySearchCount();
    clearKeyMayDeletedCount();
  }
  double getFilterFalsePositiveRate() {
    uint32_t total_key_search_count = getKeySearchCount();
    if (total_key_search_count == 0) {
      return -1;
    }
    return 1.0 * getKeyMayDeletedCount() / total_key_search_count;
  }
  // use clearFilterFalsePositiveRate for public called
  void clearKeySearchCount() { key_search_count = 0; }
  void incKeySearchCount() {
    if (key_search_count ==
        std::numeric_limits<decltype(key_search_count)>::max()) {
      std::cerr << "Error: overflow of key_search_count " << __FILE__ << ":"
                << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    key_search_count += 1;
  }
  uint32_t getKeySearchCount() { return key_search_count; }
  // use clearFilterFalsePositiveRate for public called
  void clearKeyMayDeletedCount() { key_may_deleted_count = 0; }
  uint32_t getKeyMayDeletedCount() { return key_may_deleted_count; }
  void setFlagKeyMayDeleted() {
    if (key_may_deleted_count ==
        std::numeric_limits<decltype(key_may_deleted_count)>::max()) {
      std::cerr << "Error: overflow of key_may_deleted_count " << __FILE__
                << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    key_may_deleted_count += 1;
    flag_key_may_deleted = true;
  }
  bool getFlagKeyMayDeleted() {
    checkProperUsageOfFlagKeyMayDeleted();
    return flag_key_may_deleted;
  }

  void clearFlagKeyMayDeleted() {
    flag_key_may_deleted = false;
    key_search_count_kmd = 0;
  }
  void incKeySearchCountKMD() { key_search_count_kmd += 1; }
  void checkProperUsageOfFlagKeyMayDeleted() {
    if (key_search_count_kmd > 1) {
      std::cout << "Error: key_may_deleted_count shall be 0 or 1."
                << " Make sure clearFlagKeyMayDeleted is called in the "
                   "beginning of all the series of isEntryAlive functions"
                << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                << std::endl;
      std::cerr << "Error: key_may_deleted_count shall be 0 or 1."
                << " Make sure clearFlagKeyMayDeleted is called in the "
                   "beginning of all the series of isEntryAlive functions"
                << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                << std::endl;
      exit(-1);
    }
  }

  bool isEntryAlive(uint level, long long key) {
    // init();
    // std::lock_guard<std::mutex> guard(update_mutex);

    assert(rd_filter.size() > level);

    if (level >= rd_filter.size()) {
      return true;
    }

    auto& rdList = rd_filter[level];
    if (rdList.size() == 0) {
      return true;
    }

    // split keys
    auto& splitKeyList = split_keys[level];
    auto it_p = lower_bound(splitKeyList.begin(), splitKeyList.end(), key);
    // if key exact the same return true
    if (it_p != splitKeyList.end() && *it_p == key) {
      return true;
    }

    auto it = upper_bound(
        rdList.begin(), rdList.end(), pll(key, key),
        [](const pll& a, const pll& b) { return a.first < b.first; });
    if (it != rdList.begin()) {
      it--;
    }

    if (key >= it->first && key < it->second) {
      return false;
    }

    return true;
  }

  void gatherSortedRangeTombstonesAndRemoveRDF(
      uint clevel, std::vector<uint64_t> file_numbers,
      std::vector<pll> file_boundaries, std::vector<pll>& gathered_rd_list) {
    if (clevel == 0) {
      for (uint64_t& file_num : file_numbers) {
        auto it = rd_filter_level0.find(file_num);
        if (it != rd_filter_level0.end()) {
          gathered_rd_list.insert(gathered_rd_list.end(), it->second.begin(),
                                  it->second.end());
          rd_filter_level0.erase(it);
        }
      }
    } else {
      if (rd_filter.size() <= clevel) {
        return;
      }
      std::vector<pll> new_current_level_rdf;
      auto& old_current_level_rdf = rd_filter[clevel];

      for (auto& range : old_current_level_rdf) {
        bool overlapped = false;
        for (auto& boundary : file_boundaries) {
          if (range.second > boundary.first && range.first < boundary.second) {
            overlapped = true;
            break;
          }
        }

        if (overlapped) {
          for (auto& boundary : file_boundaries) {
            long long start = std::max(range.first, boundary.first);
            long long end = std::min(range.second, boundary.second);
            if (start < end) {
              gathered_rd_list.push_back({start, end});
            }
          }
        } else {
          new_current_level_rdf.push_back(range);
        }
      }
      rd_filter[clevel] = new_current_level_rdf;
    }
  }

  // void shiftRDFToOutputLevel(uint olevel, std::vector<pll> gathered_rd_list,
  //                            std::vector<pll> output_file_boundaries,
  //                            std::vector<uint64_t> output_file_numbers) {
  //   if (gathered_rd_list.empty()) {
  //     return;
  //   }

  //   std::vector<pll> sorted_merged_rdlist = sortAndMerge(gathered_rd_list);

  //   if (olevel == 0) {
  //     assert(output_file_numbers.size() == 1);
  //     uint64_t file_num = output_file_numbers[0];
  //     if (!output_file_boundaries.empty()) {
  //       std::vector<pll> clipped_rd_list;
  //       pll boundary = output_file_boundaries[0];
  //       for (auto& range : sorted_merged_rdlist) {
  //         long long start = std::max(range.first, boundary.first);
  //         long long end = std::min(range.second, boundary.second);
  //         if (start < end) {
  //           clipped_rd_list.push_back({start, end});
  //         }
  //       }
  //       rd_filter_level0[file_num] = clipped_rd_list;
  //     } else {
  //       rd_filter_level0[file_num] = sorted_merged_rdlist;
  //     }
  //   } else {
  //     std::vector<pll> to_be_added;
  //     if (!output_file_boundaries.empty()) {
  //       for (auto& boundary : output_file_boundaries) {
  //         for (auto& range : sorted_merged_rdlist) {
  //           long long start = std::max(range.first, boundary.first);
  //           long long end = std::min(range.second, boundary.second);
  //           if (start < end) {
  //             to_be_added.push_back({start, end});
  //           }
  //         }
  //       }
  //     } else {
  //       to_be_added = sorted_merged_rdlist;
  //     }
  //     addRangeDelete_internal(olevel, to_be_added);
  //   }
  // }

  void deleteLastLevelIfEqualsBottomLevel(uint bottom_level) {
    // init();
    // std::lock_guard<std::mutex> guard(update_mutex);

    if (rd_filter.size() - 1 == bottom_level) {
      rd_filter[bottom_level].clear();
      split_keys[bottom_level].clear();
    }
  }

  void splitRangesOnLevel(uint level, std::vector<long long> keys) {
    if (rd_filter.size() <= level) {
      // std::cerr << "Error: splitRangesOnLevel: level: " << level
      //           << " is not present in PLRDF" << " " << __FILE__ << ":"
      //           << __LINE__ << " " << __FUNCTION__ << std::endl;
      // exit(1);
      return;
    }

    // auto rdList = rd_filter[level];
    // std::vector<pll> rdList_new;
    // auto &rdList = rd_filter[level];
    //   std::vector<Pair> rdList_new;
    auto& splitKeyList = split_keys[level];
    std::vector<long long> splitKeysList_new;

    auto& rangeList = rd_filter[level];
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
      long long candidate;

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
        // Equal keys ??advance both
        candidate = *it1;
        ++it1;
        ++it2;
      }

      // only storing split keys covered by some ranges
      {
        while (it_r != it_re && it_r->second <= candidate) {
          it_r++;
        }
        if (it_r == it_re) {
          break;
        }
        if (candidate < it_r->first) {
          continue;
        }
      }

      // Avoid inserting duplicate consecutive values
      if (splitKeysList_new.empty() ||
          (candidate != splitKeysList_new.back())) {
        splitKeysList_new.push_back(candidate);
      }
    }
    split_keys[level] = splitKeysList_new;
  }

  void logCurrentTotalNumbersOfRanges() {
    numbers_of_ranges_in_RDF_log.push_back(getNumberOfTotalRanges());
  }

  std::vector<int> getNumbersOfRangesInRDFLog() {
    if (numbers_of_ranges_in_RDF_log.size() == 0) {
      return {};
    } else {
      return numbers_of_ranges_in_RDF_log;
    }
  }

  void logCurrentTotalMemoryUsage() {
    memory_usage_in_RDF_log.push_back(getNumberOfTotalMemoryUsage());
  }
  std::vector<int> getMemoryUsageInRDFLog() { return memory_usage_in_RDF_log; }

  // Tracing logic
  void checkTracingConsistency(uint32_t level) {
    if (level >= rd_filter.size()) return;
    auto actual = rd_filter[level];
    auto traced = getAllFileRangeTombstones(level);
    size_t len_actual = actual.size();
    size_t len_traced = traced.size();
    bool flag_inconsistency = false;

    if (len_actual != len_traced) {
      flag_inconsistency = true;
      std::cerr << "len_actual " << len_actual << " != " << "len_traced "
                << len_traced << std::endl;
    }

    for (size_t i = 0; i < len_actual; i++) {
      if (actual[i] != traced[i]) {
        flag_inconsistency = true;
        auto& r1 = actual[i];
        auto& r2 = traced[i];
        std::cerr << "actual " << i << ": " << "[" << r1.first << ","
                  << r1.second << ")"
                  << " != traced " << i << ": " << "[" << r2.first << ","
                  << r2.second << ")" << std::endl;
      }
    }
    if (flag_inconsistency) {
      std::cerr << "Consistency Check FAILED for level " << level << std::endl;
      std::cerr << "Actual size: " << actual.size()
                << ", Traced size: " << traced.size() << std::endl;
      std::cerr << "Actual: ";
      for (auto const& r : actual)
        std::cerr << "[" << r.first << "," << r.second << ") ";
      std::cerr << std::endl << "Traced: ";
      for (auto const& r : traced)
        std::cerr << "[" << r.first << "," << r.second << ") ";
      std::cerr << std::endl;
    }
  }

  void insertTracingRangesAtLevelOfFd(uint32_t level, uint64_t fd,
                                      std::vector<pll>& ranges) {
    if (ranges.empty()) return;
    if (level == 0) {
      tracing_rd_filter_level0[fd] = ranges;
    } else {
      while (level >= tracing_level_file_rd_filter.size()) {
        tracing_level_file_rd_filter.push_back(
            std::map<uint64_t, std::vector<pll>>());
      }
      tracing_level_file_rd_filter[level][fd] = ranges;
    }
  }

  void removeTracingRangesAtLevelOfFd(uint32_t level, uint64_t fd) {
    if (level == 0) {
      tracing_rd_filter_level0.erase(fd);
    } else if (level < tracing_level_file_rd_filter.size()) {
      tracing_level_file_rd_filter[level].erase(fd);
    }
  }

  void setTracingForFile(uint32_t level, uint64_t fd,
                         const std::vector<pll>& ranges) {
    insertTracingRangesAtLevelOfFd(level, fd,
                                   const_cast<std::vector<pll>&>(ranges));
  }

  void removeTracingForFile(uint32_t level, uint64_t fd) {
    removeTracingRangesAtLevelOfFd(level, fd);
  }

  std::vector<pll> getTracingRangesForFile(uint32_t level, uint64_t fd) {
    if (level == 0) {
      if (tracing_rd_filter_level0.count(fd)) {
        return tracing_rd_filter_level0.at(fd);
      }
    } else {
      if (level < tracing_level_file_rd_filter.size() &&
          tracing_level_file_rd_filter[level].count(fd)) {
        return tracing_level_file_rd_filter[level].at(fd);
      }
    }
    return {};
  }

  // void insertTracingRangesAtLevelOfFd(uint32_t level, uint64_t fd,
  //                                     std::vector<pll>& ranges) {
  //   if (ranges.size() == 0) {
  //     return;
  //   }
  //   if (level == 0) {
  //     tracing_rd_filter_level0[fd] = ranges;
  //   } else {
  //     while (level >= tracing_level_file_rd_filter.size()) {
  //       tracing_level_file_rd_filter.push_back(
  //           std::map<uint64_t, std::vector<pll>>());
  //     }
  //     tracing_level_file_rd_filter[level][fd] = ranges;
  //   }
  // }

  // void removeTracingRangesAtLevelOfFd(uint32_t level, uint64_t fd) {
  //   if (level == 0) {
  //     tracing_rd_filter_level0.erase(fd);
  //   } else if (level < tracing_level_file_rd_filter.size()) {
  //     tracing_level_file_rd_filter[level].erase(fd);
  //   }
  // }

  std::vector<pll> gatherTracingRangesAndRemove(
      std::vector<uint32_t>& src_level_list,
      std::vector<std::vector<uint64_t>>& src_fd_list2d) {
    assert(src_level_list.size() == src_fd_list2d.size());
    std::vector<pll> range_tombstone_list_agg;

    for (size_t i = 0; i < src_level_list.size(); i++) {
      uint32_t level = src_level_list[i];
      for (uint64_t fd : src_fd_list2d[i]) {
        if (level == 0) {
          if (tracing_rd_filter_level0.count(fd)) {
            auto& rds = tracing_rd_filter_level0[fd];
            range_tombstone_list_agg.insert(range_tombstone_list_agg.end(),
                                            rds.begin(), rds.end());
            tracing_rd_filter_level0.erase(fd);
          }
        }
        if (level < tracing_level_file_rd_filter.size()) {
          auto it = tracing_level_file_rd_filter[level].find(fd);
          if (it != tracing_level_file_rd_filter[level].end()) {
            range_tombstone_list_agg.insert(range_tombstone_list_agg.end(),
                                            it->second.begin(),
                                            it->second.end());
            tracing_level_file_rd_filter[level].erase(it);
          }
        }
      }
    }

    return sortAndMerge(range_tombstone_list_agg);
  }

  void shiftTracingToOutputLevel(std::vector<pll>& rd_merged,
                                 uint32_t dst_level,
                                 std::vector<uint64_t>& dst_fd_list,
                                 std::vector<pll>& file_boundary_list) {
    if (rd_merged.size() == 0) return;
    assert(dst_fd_list.size() == file_boundary_list.size());

    size_t i_rd = 0;
    size_t len_rd = rd_merged.size();
    size_t len_dst = dst_fd_list.size();

    for (size_t i_dst = 0; i_dst < len_dst; i_dst++) {
      if (i_rd >= len_rd) break;

      uint64_t dst_fd = dst_fd_list[i_dst];
      pll file_boundary = file_boundary_list[i_dst];
      if (file_boundary.first == file_boundary.second) continue;

      std::vector<pll> ranges_to_insert;
      while (i_rd < len_rd && rd_merged[i_rd].second <= file_boundary.first) {
        i_rd++;
      }
      while (i_rd < len_rd && rd_merged[i_rd].first < file_boundary.second) {
        pll range_in = std::make_pair(
            std::max(rd_merged[i_rd].first, file_boundary.first),
            std::min(rd_merged[i_rd].second, file_boundary.second));
        ranges_to_insert.push_back(range_in);
        if (rd_merged[i_rd].second > file_boundary.second) {
          break;
        }
        i_rd++;
      }

      if (ranges_to_insert.size() > 0) {
        this->insertTracingRangesAtLevelOfFd(dst_level, dst_fd,
                                             ranges_to_insert);
      }
    }
  }

  std::vector<pll> getTracingRangesForFile(uint32_t level, uint64_t fd) const {
    if (level == 0) {
      if (tracing_rd_filter_level0.count(fd)) {
        return tracing_rd_filter_level0.at(fd);
      }
    } else {
      if (level < tracing_level_file_rd_filter.size() &&
          tracing_level_file_rd_filter[level].count(fd)) {
        return tracing_level_file_rd_filter[level].at(fd);
      }
    }
    return {};
  }

  bool isEntryAliveWithTracingAtLevelOfFd(uint level, uint64_t fd,
                                          long long key) {
    if (level == 0) {
      if (tracing_rd_filter_level0.count(fd)) {
        auto& rdList = tracing_rd_filter_level0[fd];
        auto it = upper_bound(
            rdList.begin(), rdList.end(), pll(key, key),
            [](const pll& a, const pll& b) { return a.first < b.first; });
        if (it != rdList.begin()) it--;
        if (it != rdList.end() && key >= it->first && key < it->second) {
          return false;
        }
      }
      return true;
    }

    if (level < tracing_level_file_rd_filter.size()) {
      auto it_map = tracing_level_file_rd_filter[level].find(fd);
      if (it_map != tracing_level_file_rd_filter[level].end()) {
        auto& rdList = it_map->second;
        auto it = upper_bound(
            rdList.begin(), rdList.end(), pll(key, key),
            [](const pll& a, const pll& b) { return a.first < b.first; });
        if (it != rdList.begin()) it--;
        if (it != rdList.end() && key >= it->first && key < it->second) {
          return false;
        }
      }
    }
    return true;
  }
};

// Self Added --- END PL-RDF ---

struct SuRFFlushToLevel0Info {
  uint64_t dst_fd = 0;
  std::vector<pss> rd_list;

  void check_filled() { assert(rd_list.size() > 0); }
};

struct SuRFCompactionSourceLevelInfo {
  uint32_t src_level;
  std::vector<uint64_t> src_fd_list;
  bool is_on_output_level;
  std::vector<pss> range_tombstones_str;

  void check_filled() { assert(src_fd_list.size() > 0); }
};
struct SuRFCompactionDstinationLevelInfo {
  uint64_t fd;
  pss file_boundary;
};
struct SuRFCompactionMovingRDInfo {
  std::vector<SuRFCompactionSourceLevelInfo> src_level_info_list;
  uint32_t dst_level;
  std::vector<SuRFCompactionDstinationLevelInfo> dst_level_info_list;
  bool flag_direct_move_to_dst_level = false;

  // for split
  std::vector<pss> in_comming_point_keys;

  void check_filled() {
    assert(src_level_info_list.size() > 0);
    if (flag_direct_move_to_dst_level == false) {
      assert(dst_level_info_list.size() > 0);
    }
  }
};

struct SuRFCompactionDirectRemovalInfo {
  uint32_t src_level;
  std::vector<uint64_t> src_fd_list;

  void check_filled() { assert(src_fd_list.size() > 0); }
};

struct OriginInfo {
  std::vector<int> numbers_of_ranges, memory_usage_on_ranges;

  void logCurrentTotalNumbersOfRanges(int count) {
    numbers_of_ranges.push_back(count);
  }

  void logCurrentTotalMemoryUsage(int bytes) {
    memory_usage_on_ranges.push_back(bytes);
  }

  std::vector<int> getNumbersOfRanges() { return numbers_of_ranges; }

  std::vector<int> getMemoryUsageOnRanges() { return memory_usage_on_ranges; }
};

struct RDFUpdateMetadata;

// VersionRDFBundle encapsulates the complete RDF state for a Version.
// In a Pure MVCC model, each Version holds a shared_ptr to this bundle.
struct VersionRDFBundle {
  PLRDF plrdf;
  PLRDF split_plrdf;
  PLRDF_t<std::string> plrdf_stringkey;
  PLRDF_t<std::string> split_plrdf_stringkey;
  PLRDF top_level_rdf;
  PLRDF_t<std::string> top_level_rdf_stringkey;
  SkyLineRDF skyline_rdf;

  // Using shared_ptr for SuRF components to manage their lifetime
  // within the bundle.
  std::shared_ptr<surf::SuRF_RDF> surf_level_file_rdf;
  std::shared_ptr<surf::SuRF_RDF> surf_level_file_split_rdf;

  OriginInfo origin_info;

  VersionRDFBundle() = default;

  // Custom Copy Constructor for Deep Copy
  VersionRDFBundle(const VersionRDFBundle& other)
      : plrdf(other.plrdf),
        split_plrdf(other.split_plrdf),
        plrdf_stringkey(other.plrdf_stringkey),
        split_plrdf_stringkey(other.split_plrdf_stringkey),
        top_level_rdf(other.top_level_rdf),
        top_level_rdf_stringkey(other.top_level_rdf_stringkey),
        skyline_rdf(other.skyline_rdf),
        origin_info(other.origin_info) {
    // std::cout << "VersionRDFBundle DEEP Copy Constructor " << __FILE__ << ":"
    // << __LINE__ << " " << __FUNCTION__ << std::endl;
    if (other.surf_level_file_rdf) {
      surf_level_file_rdf =
          std::make_shared<surf::SuRF_RDF>(*other.surf_level_file_rdf);
    }
    if (other.surf_level_file_split_rdf) {
      surf_level_file_split_rdf =
          std::make_shared<surf::SuRF_RDF>(*other.surf_level_file_split_rdf);
    }
  }

  // Deep copy / Clone support for induction (if needed, otherwise induction
  // can just modify a new bundle created from the previous one)

  // Apply changes from RDFUpdateMetadata to this bundle.
  // This method modifies the bundle in-place.
  // When applying to a new Version, ensure this bundle is a clone/copy first.
  void ApplyDelta(const RDFUpdateMetadata& delta);

 private:
  // Helper methods for job-type-specific RDF updates
  void ApplyFlushDelta(const RDFUpdateMetadata& delta);
  void ApplyCompactionDelta(const RDFUpdateMetadata& delta);
  void ApplyDirectRemovalDelta(const RDFUpdateMetadata& delta);
};

// RDFUpdateMetadata encapsulates all job-specific deltas for all 9 RDF
// variants.
struct RDFUpdateMetadata {
  enum class JobType {
    kFlush,
    kCompaction,
    kCompactionDirectRemoved,
    kCompactionDirectMoved
  };
  JobType type;
  bool split_flag = false;

  // PLRDF variants deltas (Level0 inserts)
  // tuple: (file_num, range_delete_list, exist_level0_file_nums)
  std::tuple<uint64_t, std::vector<pll>, std::vector<uint64_t>>
      plrdf_flush_delta;
  std::tuple<uint64_t, std::vector<pll>, std::vector<uint64_t>>
      split_plrdf_flush_delta;
  std::tuple<uint64_t, std::vector<pss>, std::vector<uint64_t>>
      plrdf_stringkey_flush_delta;
  std::tuple<uint64_t, std::vector<pss>, std::vector<uint64_t>>
      split_plrdf_stringkey_flush_delta;
  std::tuple<uint64_t, std::vector<pll>, std::vector<uint64_t>>
      top_level_plrdf_flush_delta;
  std::tuple<uint64_t, std::vector<pss>, std::vector<uint64_t>>
      top_level_plrdf_stringkey_flush_delta;

  // Compaction deltas (Moving ranges)
  std::vector<std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>>
      plrdf_compaction_delta;
  std::vector<std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>>
      split_plrdf_compaction_delta;
  std::vector<std::tuple<int, int, std::vector<pss>, std::vector<uint64_t>>>
      plrdf_stringkey_compaction_delta;
  std::vector<std::tuple<int, int, std::vector<pss>, std::vector<uint64_t>>>
      split_plrdf_stringkey_compaction_delta;

  // TopLevel RDF variants deltas (Compaction / Move)
  std::vector<std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>>
      top_level_plrdf_compaction_delta;
  std::vector<std::tuple<int, int, std::vector<pss>, std::vector<uint64_t>>>
      top_level_plrdf_stringkey_compaction_delta;

  // SuRF variants deltas
  std::unique_ptr<SuRFFlushToLevel0Info> surf_flush_info;
  std::unique_ptr<SuRFFlushToLevel0Info> split_surf_flush_info;
  std::unique_ptr<SuRFCompactionMovingRDInfo> surf_compaction_info;
  std::unique_ptr<SuRFCompactionMovingRDInfo> split_surf_compaction_info;
  std::unique_ptr<SuRFCompactionDirectRemovalInfo> surf_direct_removal_info;
  std::unique_ptr<SuRFCompactionDirectRemovalInfo>
      split_surf_direct_removal_info;

  // Skyline
  std::vector<t3ll> skyline_delta;

  // Other transient states
  uint64_t flush_in_file_num = 0;
  int out_level = -1;

  // Split sampled keys collected during compaction
  std::vector<long long> split_sampled_keys;
  std::vector<std::string> split_sampled_keys_str;

  uint32_t src_level;
  std::vector<uint64_t> src_fd_list;
  std::vector<pss> range_tombstones_str;

  // Tracing Deltas
  // For compaction/removal (reusing SuRFCompactionMovingRDInfo structure for
  // gathering)
  std::unique_ptr<SuRFCompactionMovingRDInfo> fd_rangetombstones_info;

  // For flush (explicit Level 0 additions)
  std::vector<std::tuple<uint32_t, uint64_t, std::vector<pll>>>
      plrdf_tracing_add;
  std::vector<std::tuple<uint32_t, uint64_t, std::vector<pll>>>
      split_plrdf_tracing_add;
  std::vector<std::tuple<uint32_t, uint64_t, std::vector<pss>>>
      plrdf_stringkey_tracing_add;
  std::vector<std::tuple<uint32_t, uint64_t, std::vector<pss>>>
      split_plrdf_stringkey_tracing_add;

  // Explicit tracing data (level, fd) for removal
  std::vector<std::pair<int, uint64_t>> plrdf_tracing_remove;
  std::vector<std::pair<int, uint64_t>> split_plrdf_tracing_remove;
  std::vector<std::pair<int, uint64_t>> plrdf_stringkey_tracing_remove;
  std::vector<std::pair<int, uint64_t>> split_plrdf_stringkey_tracing_remove;

  vector<pss> getOutputLevelRangeTombstonesFilteredByFileRanges(
      std::vector<pss>& rd_merged, uint32_t /*dst_level*/,
      std::vector<pss>& file_boundary_list) const;
};

inline vector<pss>
RDFUpdateMetadata::getOutputLevelRangeTombstonesFilteredByFileRanges(
    std::vector<pss>& rd_merged, uint32_t /*dst_level*/,
    std::vector<pss>& file_boundary_list) const {
  if (rd_merged.size() == 0) return {};

  size_t start_i_rd = 0;
  size_t len_rd = rd_merged.size();
  std::vector<pss> ranges_to_insert;
  for (size_t i_dst = 0; i_dst < file_boundary_list.size(); i_dst++) {
    pss file_boundary = file_boundary_list[i_dst];
    if (file_boundary.first == file_boundary.second) continue;

    size_t i_rd = start_i_rd;
    while (i_rd < len_rd && rd_merged[i_rd].second <= file_boundary.first) {
      i_rd++;
    }
    start_i_rd = i_rd;

    while (i_rd < len_rd && rd_merged[i_rd].first < file_boundary.second) {
      pss range_in = std::make_pair(
          std::max(rd_merged[i_rd].first, file_boundary.first),
          std::min(rd_merged[i_rd].second, file_boundary.second));
      ranges_to_insert.push_back(range_in);
      // if (rd_merged[i_rd].second >= file_boundary.second) {
      //   break;
      // }
      i_rd++;
    }
  }
  return ranges_to_insert;
}

inline void VersionRDFBundle::ApplyDelta(const RDFUpdateMetadata& delta) {
  switch (delta.type) {
    case RDFUpdateMetadata::JobType::kFlush:
      // std::cout << "kFlush ApplyFlushDelta " << " " << __FILE__ << ":"
      //           << __LINE__ << " " << __FUNCTION__ << std::endl;
      ApplyFlushDelta(delta);
      break;
    case RDFUpdateMetadata::JobType::kCompaction:
      // std::cout << "kCompaction ApplyCompactionDelta " << " " << __FILE__ <<
      // ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      ApplyCompactionDelta(delta);
      break;
    case RDFUpdateMetadata::JobType::kCompactionDirectRemoved:
      // std::cout << "kCompactionDirectRemoved ApplyDirectRemovalDelta " << " "
      //           << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
      //           << std::endl;
      ApplyDirectRemovalDelta(delta);
      break;
    case RDFUpdateMetadata::JobType::kCompactionDirectMoved:
      // std::cout << "kCompactionDirectMoved ApplyCompactionDelta " << " "
      //           << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
      //           << std::endl;
      ApplyCompactionDelta(delta);
      break;
  }

  logCurrentTotalNumbersOfRangesInEachRDF();
  logCurrentTotalMemoryUsageInEachRDF();
}

inline void VersionRDFBundle::ApplyFlushDelta(const RDFUpdateMetadata& delta) {
  // Tracing Updates for Flush
  if (!delta.plrdf_tracing_add.empty()) {
    for (const auto& tup : delta.plrdf_tracing_add) {
      plrdf.insertTracingRangesAtLevelOfFd(
          std::get<0>(tup), std::get<1>(tup),
          const_cast<std::vector<pll>&>(std::get<2>(tup)));
    }
  }
  if (!delta.split_plrdf_tracing_add.empty()) {
    for (const auto& tup : delta.split_plrdf_tracing_add) {
      split_plrdf.insertTracingRangesAtLevelOfFd(
          std::get<0>(tup), std::get<1>(tup),
          const_cast<std::vector<pll>&>(std::get<2>(tup)));
    }
  }
  if (!delta.plrdf_stringkey_tracing_add.empty()) {
    for (const auto& tup : delta.plrdf_stringkey_tracing_add) {
      plrdf_stringkey.insertTracingRangesAtLevelOfFd(
          std::get<0>(tup), std::get<1>(tup),
          const_cast<std::vector<pss>&>(std::get<2>(tup)));
    }
  }
  if (!delta.split_plrdf_stringkey_tracing_add.empty()) {
    for (const auto& tup : delta.split_plrdf_stringkey_tracing_add) {
      split_plrdf_stringkey.insertTracingRangesAtLevelOfFd(
          std::get<0>(tup), std::get<1>(tup),
          const_cast<std::vector<pss>&>(std::get<2>(tup)));
    }
  }

  // 1. PLRDF Flush
  if (std::get<0>(delta.plrdf_flush_delta) != 0) {
    plrdf.insertRangeDeleteToLevel0(
        std::get<0>(delta.plrdf_flush_delta),
        const_cast<std::vector<pll>&>(std::get<1>(delta.plrdf_flush_delta)),
        std::get<2>(delta.plrdf_flush_delta));
#ifdef CHECK_PLRDF_TRACING_CONSISTENCY
    plrdf.checkTracingConsistency(0);
#endif
  }
  if (std::get<0>(delta.plrdf_stringkey_flush_delta) != 0) {
    plrdf_stringkey.insertRangeDeleteToLevel0(
        std::get<0>(delta.plrdf_stringkey_flush_delta),
        const_cast<std::vector<pss>&>(
            std::get<1>(delta.plrdf_stringkey_flush_delta)),
        std::get<2>(delta.plrdf_stringkey_flush_delta));
#ifdef CHECK_PLRDF_TRACING_CONSISTENCY
    plrdf_stringkey.checkTracingConsistency(0);
#endif
  }

  // 2. PLRDF Flush
  if (std::get<0>(delta.split_plrdf_flush_delta) != 0) {
    split_plrdf.insertRangeDeleteToLevel0(
        std::get<0>(delta.split_plrdf_flush_delta),
        const_cast<std::vector<pll>&>(
            std::get<1>(delta.split_plrdf_flush_delta)),
        std::get<2>(delta.split_plrdf_flush_delta));
#ifdef CHECK_PLRDF_TRACING_CONSISTENCY
    split_plrdf.checkTracingConsistency(0);
#endif
  }
  if (std::get<0>(delta.split_plrdf_stringkey_flush_delta) != 0) {
    split_plrdf_stringkey.insertRangeDeleteToLevel0(
        std::get<0>(delta.split_plrdf_stringkey_flush_delta),
        const_cast<std::vector<pss>&>(
            std::get<1>(delta.split_plrdf_stringkey_flush_delta)),
        std::get<2>(delta.split_plrdf_stringkey_flush_delta));
#ifdef CHECK_PLRDF_TRACING_CONSISTENCY
    split_plrdf_stringkey.checkTracingConsistency(0);
#endif
  }

  // 3. TopLevelRDF Flush
  if (std::get<0>(delta.top_level_plrdf_flush_delta) != 0) {
    top_level_rdf.insertRangeDeleteToLevel0(
        std::get<0>(delta.top_level_plrdf_flush_delta),
        const_cast<std::vector<pll>&>(
            std::get<1>(delta.top_level_plrdf_flush_delta)),
        std::get<2>(delta.top_level_plrdf_flush_delta));
  }
  if (std::get<0>(delta.top_level_plrdf_stringkey_flush_delta) != 0) {
    top_level_rdf_stringkey.insertRangeDeleteToLevel0(
        std::get<0>(delta.top_level_plrdf_stringkey_flush_delta),
        const_cast<std::vector<pss>&>(
            std::get<1>(delta.top_level_plrdf_stringkey_flush_delta)),
        std::get<2>(delta.top_level_plrdf_stringkey_flush_delta));
  }

  // SuRF Flush
  if (delta.surf_flush_info) {
    if (!surf_level_file_rdf) {
      surf_level_file_rdf =
          std::make_shared<surf::SuRF_RDF>(surf::SuRF_RDF::PER_FILE);
    } else {
      surf_level_file_rdf =
          std::make_shared<surf::SuRF_RDF>(*surf_level_file_rdf);
    }

    std::vector<pss> rd_list = delta.surf_flush_info->rd_list;
    if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
        true) {
      for (auto& x : rd_list) {
        uint32_t len_condensed_key =
            surf::SuRF_Env::getInstance()->getLengthOfCondensedDigitKey();
        x.first = surf::SuRF_Utils::encode_digit_string_to_byte_string(x.first);
        x.first = surf::SuRF_Utils::extend_string_to_length(
            x.first, len_condensed_key, (char)0);
        x.second =
            surf::SuRF_Utils::encode_digit_string_to_byte_string(x.second);
        x.second = surf::SuRF_Utils::extend_string_to_length(
            x.second, len_condensed_key, (char)0);
      }
    }

    if (rd_list.size() != 0) {
      surf_level_file_rdf->insertRangeDeleteToLevel0(
          delta.surf_flush_info->dst_fd, const_cast<std::vector<pss>&>(rd_list),
          surf::SuRF_Env::getInstance()->getFlagAllowRangeBoundaryOverlapped());
    }
  }

  if (delta.split_surf_flush_info) {
    if (!surf_level_file_split_rdf) {
      surf_level_file_split_rdf =
          std::make_shared<surf::SuRF_RDF>(surf::SuRF_RDF::PER_FILE);
    } else {
      surf_level_file_split_rdf =
          std::make_shared<surf::SuRF_RDF>(*surf_level_file_split_rdf);
    }

    std::vector<pss> rd_list = delta.split_surf_flush_info->rd_list;
    if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
        true) {
      for (auto& x : rd_list) {
        uint32_t len_condensed_key =
            surf::SuRF_Env::getInstance()->getLengthOfCondensedDigitKey();
        x.first = surf::SuRF_Utils::encode_digit_string_to_byte_string(x.first);
        x.first = surf::SuRF_Utils::extend_string_to_length(
            x.first, len_condensed_key, (char)0);
        x.second =
            surf::SuRF_Utils::encode_digit_string_to_byte_string(x.second);
        x.second = surf::SuRF_Utils::extend_string_to_length(
            x.second, len_condensed_key, (char)0);
      }
    }

    if (rd_list.size() != 0) {
      surf_level_file_split_rdf->insertRangeDeleteToLevel0(
          delta.split_surf_flush_info->dst_fd,
          const_cast<std::vector<pss>&>(rd_list),
          surf::SuRF_Env::getInstance()->getFlagAllowRangeBoundaryOverlapped());
    }
  }

  // Skyline RDF
  // Skyline might be updated in flush as well, although usually compaction.
  // Keeping it here if populated in flush delta.
  if (!delta.skyline_delta.empty()) {
    skyline_rdf.addRangeTombstones(delta.skyline_delta);
  }
}

inline void VersionRDFBundle::ApplyCompactionDelta(
    const RDFUpdateMetadata& delta) {
  // Tracing Updates
  // If explicit tracing is provided, use it. Otherwise fall back to flush_delta
  // for L0.
  // Handle Tracing for Compaction (Per-File Redesign)
  std::vector<pss> rangetombstones_on_ouput_level;
  // std::vector<pll> rangetombstones_on_ouput_level_pll;
  if (delta.fd_rangetombstones_info) {
    auto& info = *delta.fd_rangetombstones_info;
    std::vector<uint32_t> src_level_list;
    std::vector<std::vector<uint64_t>> src_fd_list2d;
    std::vector<pss> sorted_range_tombstones_str;
    std::vector<pss> merged_sorted_range_tombstones_str;
    for (auto& src_info : info.src_level_info_list) {
      src_level_list.push_back(src_info.src_level);
      src_fd_list2d.push_back(src_info.src_fd_list);
      std::vector<pss>& range_tombstones_str = src_info.range_tombstones_str;
      sorted_range_tombstones_str.insert(sorted_range_tombstones_str.end(),
                                         range_tombstones_str.begin(),
                                         range_tombstones_str.end());
    }

    std::vector<uint64_t> dst_fd_list;
    for (auto& dst_info : info.dst_level_info_list) {
      dst_fd_list.push_back(dst_info.fd);
    }

    // #ifdef CHECK_PLRDF_TRACING_CONSISTENCY
    //     std::cout << "apply compaction " << "in level = ";
    //     for (auto& src_level : src_level_list) {
    //       std::cout << src_level << " ";
    //     }
    //     std::cout << "to level = " << info.dst_level;
    //     std::cout << std::endl;
    // #endif
    // if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
    //     true) {
    //   for (auto& x : sorted_range_tombstones_str) {
    //     uint32_t len_condensed_key =
    //         surf::SuRF_Env::getInstance()->getLengthOfCondensedDigitKey();
    //     x.first =
    //         surf::SuRF_Utils::encode_digit_string_to_byte_string(x.first);
    //     x.first = surf::SuRF_Utils::extend_string_to_length(
    //         x.first, len_condensed_key, (char)0);
    //     x.second =
    //         surf::SuRF_Utils::encode_digit_string_to_byte_string(x.second);
    //     x.second = surf::SuRF_Utils::extend_string_to_length(
    //         x.second, len_condensed_key, (char)0);
    //   }
    // }
    std::sort(sorted_range_tombstones_str.begin(),
              sorted_range_tombstones_str.end());
    bool flag_first = true;
    for (auto& x_range : sorted_range_tombstones_str) {
      if (flag_first == true) {
        merged_sorted_range_tombstones_str.push_back(x_range);
        flag_first = false;
        continue;
      }

      if (x_range.first <= merged_sorted_range_tombstones_str.back().second) {
        merged_sorted_range_tombstones_str.back().second = std::max(
            x_range.second, merged_sorted_range_tombstones_str.back().second);
      } else {
        merged_sorted_range_tombstones_str.push_back(x_range);
      }
    }
    std::vector<pss> range_tombstone_merged;
    range_tombstone_merged = merged_sorted_range_tombstones_str;
    std::vector<pss> dst_boundaries_str;
    for (auto& dst_info : info.dst_level_info_list) {
      dst_boundaries_str.push_back(dst_info.file_boundary);
    }
    rangetombstones_on_ouput_level =
        delta.getOutputLevelRangeTombstonesFilteredByFileRanges(
            range_tombstone_merged, info.dst_level, dst_boundaries_str);

    // std::vector<pll> rangetombstones_on_ouput_level_pll;
    // for (auto& x : rangetombstones_on_ouput_level) {
    //   rangetombstones_on_ouput_level_pll.push_back(
    //       std::make_pair(std::stoll(x.first), std::stoll(x.second)));
    // }

#ifdef CHECK_PLRDF_TRACING_CONSISTENCY
    // 1. PLRDF
    {
      if (!delta.plrdf_compaction_delta.empty()) {
        std::vector<pll> gathered =
            plrdf.gatherTracingRangesAndRemove(src_level_list, src_fd_list2d);
        gathered.clear();
        for (auto& rt : range_tombstone_merged) {
          gathered.push_back(
              std::make_pair(std::stoll(rt.first), std::stoll(rt.second)));
        }
        if (!gathered.empty()) {
          std::vector<pll> dst_boundaries_pll;
          for (auto& dst_info : info.dst_level_info_list) {
            dst_boundaries_pll.push_back(
                std::make_pair(std::stoll(dst_info.file_boundary.first),
                               std::stoll(dst_info.file_boundary.second)));
          }
          plrdf.shiftTracingToOutputLevel(gathered, info.dst_level, dst_fd_list,
                                          dst_boundaries_pll);
        }
      }
    }

    // 2. SPLIT_PLRDF
    {
      if (!delta.split_plrdf_compaction_delta.empty()) {
        std::vector<pll> gathered = split_plrdf.gatherTracingRangesAndRemove(
            src_level_list, src_fd_list2d);
        gathered.clear();
        for (auto& rt : range_tombstone_merged) {
          gathered.push_back(
              std::make_pair(std::stoll(rt.first), std::stoll(rt.second)));
        }
        if (!gathered.empty()) {
          std::vector<pll> dst_boundaries_pll;
          for (auto& dst_info : info.dst_level_info_list) {
            dst_boundaries_pll.push_back(
                std::make_pair(std::stoll(dst_info.file_boundary.first),
                               std::stoll(dst_info.file_boundary.second)));
          }
          split_plrdf.shiftTracingToOutputLevel(
              gathered, info.dst_level, dst_fd_list, dst_boundaries_pll);
        }
      }
    }

    // 3. PLRDF String Key
    {
      if (!delta.plrdf_stringkey_compaction_delta.empty()) {
        std::vector<pss> gathered =
            plrdf_stringkey.gatherTracingRangesAndRemove(src_level_list,
                                                         src_fd_list2d);
        // gathered.clear(); // Removed
        gathered = range_tombstone_merged;
        if (!gathered.empty()) {
          std::vector<pss> dst_boundaries_pss;
          for (auto& dst_info : info.dst_level_info_list) {
            dst_boundaries_pss.push_back(dst_info.file_boundary);
          }
          plrdf_stringkey.shiftTracingToOutputLevel(
              gathered, info.dst_level, dst_fd_list, dst_boundaries_pss);
        }
      }
    }

    // 4. SPLIT_PLRDF String Key
    {
      if (!delta.split_plrdf_stringkey_compaction_delta.empty()) {
        std::vector<pss> gathered =
            split_plrdf_stringkey.gatherTracingRangesAndRemove(src_level_list,
                                                               src_fd_list2d);
        gathered.clear();
        gathered = range_tombstone_merged;
        if (!gathered.empty()) {
          std::vector<pss> dst_boundaries_pss;
          for (auto& dst_info : info.dst_level_info_list) {
            dst_boundaries_pss.push_back(dst_info.file_boundary);
          }
          split_plrdf_stringkey.shiftTracingToOutputLevel(
              gathered, info.dst_level, dst_fd_list, dst_boundaries_pss);
        }
      }
    }
#endif
  }

  // 1. PLRDF Compaction
  if (!delta.plrdf_compaction_delta.empty()) {
    // std::vector<std::tuple<int, int, std::vector<pll>,
    // std::vector<uint64_t>>>*
    //     rangetombstones_to_remove_on_levels = new std::vector<
    //         std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>>();

    // auto& info = *delta.fd_rangetombstones_info;
    // for (auto& src_info : info.src_level_info_list) {
    //   std::vector<pll> rangetombstones_str_pll;
    //   for (auto& x : src_info.range_tombstones_str) {
    //     rangetombstones_str_pll.push_back(
    //         std::make_pair(std::stoll(x.first), std::stoll(x.second)));
    //   }
    //   rangetombstones_to_remove_on_levels->push_back(
    //       std::make_tuple(src_info.src_level, info.dst_level,
    //                       rangetombstones_str_pll, std::vector<uint64_t>{}));
    // }

    std::vector<pll> rangetombstones_on_ouput_level_pll;
    for (auto& x : rangetombstones_on_ouput_level) {
      rangetombstones_on_ouput_level_pll.push_back(
          std::make_pair(std::stoll(x.first), std::stoll(x.second)));
    }

    // plrdf.shiftRDFToOutputLevel(rangetombstones_to_remove_on_levels,
    //                             rangetombstones_on_ouput_level_pll);
    plrdf.shiftRDFToOutputLevel(&delta.plrdf_compaction_delta,
                                rangetombstones_on_ouput_level_pll);
#ifdef CHECK_PLRDF_TRACING_CONSISTENCY
    plrdf.checkTracingConsistency(
        std::get<0>(delta.plrdf_compaction_delta.front()));
    plrdf.checkTracingConsistency(
        std::get<1>(delta.plrdf_compaction_delta.front()));
#endif
  }
  if (!delta.plrdf_stringkey_compaction_delta.empty()) {
    // std::vector<std::tuple<int, int, std::vector<pss>,
    // std::vector<uint64_t>>>*
    //     rangetombstones_to_remove_on_levels = new std::vector<
    //         std::tuple<int, int, std::vector<pss>, std::vector<uint64_t>>>();

    // auto& info = *delta.fd_rangetombstones_info;
    // for (auto& src_info : info.src_level_info_list) {
    //   std::cout << "src_info lvl: " << src_info.src_level << " RT: ";
    //   for (auto& rt :)
    //     rangetombstones_to_remove_on_levels->push_back(std::make_tuple(
    //         src_info.src_level, info.dst_level,
    //         src_info.range_tombstones_str, std::vector<uint64_t>{}));
    // }

    // plrdf_stringkey.shiftRDFToOutputLevel(rangetombstones_to_remove_on_levels,
    //                                       rangetombstones_on_ouput_level);
    plrdf_stringkey.shiftRDFToOutputLevel(
        &delta.plrdf_stringkey_compaction_delta,
        rangetombstones_on_ouput_level);
#ifdef CHECK_PLRDF_TRACING_CONSISTENCY
    plrdf_stringkey.checkTracingConsistency(
        std::get<0>(delta.plrdf_stringkey_compaction_delta.front()));
    plrdf_stringkey.checkTracingConsistency(
        std::get<1>(delta.plrdf_stringkey_compaction_delta.front()));
#endif
  }

  // 2. Split PLRDF Compaction
  if (!delta.split_plrdf_compaction_delta.empty()) {
    // std::vector<std::tuple<int, int, std::vector<pll>,
    // std::vector<uint64_t>>>*
    //     rangetombstones_to_remove_on_levels = new std::vector<
    //         std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>>();

    // auto& info = *delta.fd_rangetombstones_info;
    // for (auto& src_info : info.src_level_info_list) {
    //   std::vector<pll> rangetombstones_str_pll;
    //   for (auto& x : src_info.range_tombstones_str) {
    //     rangetombstones_str_pll.push_back(
    //         std::make_pair(std::stoll(x.first), std::stoll(x.second)));
    //   }
    //   rangetombstones_to_remove_on_levels->push_back(
    //       std::make_tuple(src_info.src_level, info.dst_level,
    //                       rangetombstones_str_pll, std::vector<uint64_t>{}));
    // }

    std::vector<pll> rangetombstones_on_ouput_level_pll;
    for (auto& x : rangetombstones_on_ouput_level) {
      rangetombstones_on_ouput_level_pll.push_back(
          std::make_pair(std::stoll(x.first), std::stoll(x.second)));
    }

    // split_plrdf.shiftRDFToOutputLevel(rangetombstones_to_remove_on_levels,
    //                                   rangetombstones_on_ouput_level_pll);
    split_plrdf.shiftRDFToOutputLevel(&delta.split_plrdf_compaction_delta,
                                      rangetombstones_on_ouput_level_pll);
#ifdef CHECK_PLRDF_TRACING_CONSISTENCY
    split_plrdf.checkTracingConsistency(
        std::get<0>(delta.split_plrdf_compaction_delta.front()));
    split_plrdf.checkTracingConsistency(
        std::get<1>(delta.split_plrdf_compaction_delta.front()));
#endif

    int out_level_for_split =
        std::get<1>(delta.split_plrdf_compaction_delta.front());
    if (delta.split_flag && out_level_for_split >= 0 &&
        !delta.split_sampled_keys.empty()) {
      split_plrdf.splitRangesOnLevel(
          static_cast<uint>(out_level_for_split),
          const_cast<std::vector<long long>&>(delta.split_sampled_keys));
    }
  }
  if (!delta.split_plrdf_stringkey_compaction_delta.empty()) {
    // std::vector<std::tuple<int, int, std::vector<pss>,
    // std::vector<uint64_t>>>*
    //     rangetombstones_to_remove_on_levels = new std::vector<
    //         std::tuple<int, int, std::vector<pss>, std::vector<uint64_t>>>();

    // auto& info = *delta.fd_rangetombstones_info;
    // for (auto& src_info : info.src_level_info_list) {
    //   rangetombstones_to_remove_on_levels->push_back(std::make_tuple(
    //       src_info.src_level, info.dst_level, src_info.range_tombstones_str,
    //       std::vector<uint64_t>{}));
    // }

    // split_plrdf_stringkey.shiftRDFToOutputLevel(
    //     &delta.split_plrdf_stringkey_compaction_delta,
    //     rangetombstones_on_ouput_level);
    split_plrdf_stringkey.shiftRDFToOutputLevel(
        &delta.split_plrdf_stringkey_compaction_delta,
        rangetombstones_on_ouput_level);
#ifdef CHECK_PLRDF_TRACING_CONSISTENCY
    split_plrdf_stringkey.checkTracingConsistency(
        std::get<0>(delta.split_plrdf_stringkey_compaction_delta.front()));
    split_plrdf_stringkey.checkTracingConsistency(
        std::get<1>(delta.split_plrdf_stringkey_compaction_delta.front()));
#endif

    int out_level_for_split_str =
        std::get<1>(delta.split_plrdf_stringkey_compaction_delta.front());
    if (delta.split_flag && out_level_for_split_str >= 0 &&
        !delta.split_sampled_keys_str.empty()) {
      split_plrdf_stringkey.splitRangesOnLevel(
          static_cast<uint>(out_level_for_split_str),
          const_cast<std::vector<std::string>&>(delta.split_sampled_keys_str));
    }
  }

  // However, `RDFUpdateMetadata` might need an explicit `out_level` field if
  // not easily inferred. In `ych16`, `splitRangesOnLevel` uses
  // `this->get_split__out_level()`. Here we'll iterate through
  // `split_plrdf_compaction_delta` to find destination level or use a
  // convention. But wait, `splitRangesOnLevel` acts on the *resulting* level.
  // Let's assume for now we use the level from the compaction info.

  // Note: We need `split_sampled_keys` to perform the split.
  // If `split_flag` is set, we attempt simply calling split on the output
  // level. We need to look at `split_plrdf_compaction_delta` to find the
  // out_level (dest level). The tuple is (in_level, out_level, rds, ...).

  // 3. TopLevel RDF Compaction
  if (!delta.top_level_plrdf_compaction_delta.empty()) {
    // TopLevel might also need split logic if enabled
    if (delta.split_flag && !delta.split_sampled_keys.empty() &&
        !delta.top_level_plrdf_compaction_delta.empty() &&
        std::get<0>(delta.top_level_plrdf_compaction_delta.front()) == 0) {
      std::vector<pll> rangetombstones_on_ouput_level_pll;
      for (auto& x : rangetombstones_on_ouput_level) {
        rangetombstones_on_ouput_level_pll.push_back(
            std::make_pair(std::stoll(x.first), std::stoll(x.second)));
      }
      top_level_rdf.shiftRDFToOutputLevel(
          &delta.top_level_plrdf_compaction_delta,
          rangetombstones_on_ouput_level_pll);

      // Logic from ych16: if in_lvl == 0 and split_flag, split on level 1
      top_level_rdf.splitRangesOnLevel(
          1, const_cast<std::vector<long long>&>(delta.split_sampled_keys));
    }
  }

  if (!delta.top_level_plrdf_stringkey_compaction_delta.empty()) {
    if (delta.split_flag && !delta.split_sampled_keys_str.empty() &&
        !delta.top_level_plrdf_stringkey_compaction_delta.empty() &&
        std::get<0>(delta.top_level_plrdf_stringkey_compaction_delta.front()) ==
            0) {
      // std::cout << "top_level_plrdf_stringkey_compaction_delta " << " "
      //           << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
      //           << std::endl;
      // std::cout << "in_level = "
      //           << std::get<0>(
      //                  delta.top_level_plrdf_stringkey_compaction_delta.front())
      //           << std::endl;
      // std::cout << "out_level = "
      //           << std::get<1>(
      //                  delta.top_level_plrdf_stringkey_compaction_delta.front())
      //           << std::endl;
      // std::cout << "rd_list = ";
      // for (const auto& rd : std::get<2>(
      //          delta.top_level_plrdf_stringkey_compaction_delta.front())) {
      //   std::cout << "(" << rd.first << ", " << rd.second << ") ";
      // }
      // std::cout << std::endl;

      top_level_rdf_stringkey.shiftRDFToOutputLevel(
          &delta.top_level_plrdf_stringkey_compaction_delta,
          rangetombstones_on_ouput_level);

      top_level_rdf_stringkey.splitRangesOnLevel(
          1,
          const_cast<std::vector<std::string>&>(delta.split_sampled_keys_str));
    }
  }

  // 6. SuRF Compaction (Move)
  if (delta.surf_compaction_info) {
    if (!surf_level_file_rdf) {
      surf_level_file_rdf =
          std::make_shared<surf::SuRF_RDF>(surf::SuRF_RDF::PER_FILE);
    } else {
      surf_level_file_rdf =
          std::make_shared<surf::SuRF_RDF>(*surf_level_file_rdf);
    }

    bool surf_flag__allow_range_boundary_overlapped =
        surf::SuRF_Env::getInstance()->getFlagAllowRangeBoundaryOverlapped();

    std::vector<SuRFCompactionSourceLevelInfo>& src_level_info_list =
        delta.surf_compaction_info->src_level_info_list;
    uint32_t dst_level = delta.surf_compaction_info->dst_level;
    std::vector<SuRFCompactionDstinationLevelInfo>& dst_level_info_list =
        delta.surf_compaction_info->dst_level_info_list;
    bool flag_direct_move_to_dst_level =
        delta.surf_compaction_info->flag_direct_move_to_dst_level;

    if (flag_direct_move_to_dst_level == true) {
      for (auto& src_level_info : src_level_info_list) {
        uint32_t src_level = src_level_info.src_level;
        std::vector<uint64_t> src_fd_list = src_level_info.src_fd_list;
        for (auto& fd : src_fd_list) {
          surf_level_file_rdf->directMoveFileToLevel(fd, src_level, dst_level);
        }
      }
    } else {
      std::vector<uint32_t> src_level_list;
      std::vector<std::vector<uint64_t>> src_fd_list2d;
      std::vector<pss> sorted_range_tombstones_str;
      std::vector<pss> merged_sorted_range_tombstones_str;
      for (auto& src_level_info : src_level_info_list) {
        uint32_t& src_level = src_level_info.src_level;
        std::vector<uint64_t>& src_fd_list = src_level_info.src_fd_list;
        std::vector<pss>& range_tombstones_str =
            src_level_info.range_tombstones_str;
        src_level_list.push_back(src_level);
        src_fd_list2d.push_back(src_fd_list);
        sorted_range_tombstones_str.insert(sorted_range_tombstones_str.end(),
                                           range_tombstones_str.begin(),
                                           range_tombstones_str.end());
      }

      if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
          true) {
        for (auto& x : sorted_range_tombstones_str) {
          uint32_t len_condensed_key =
              surf::SuRF_Env::getInstance()->getLengthOfCondensedDigitKey();
          x.first =
              surf::SuRF_Utils::encode_digit_string_to_byte_string(x.first);
          x.first = surf::SuRF_Utils::extend_string_to_length(
              x.first, len_condensed_key, (char)0);
          x.second =
              surf::SuRF_Utils::encode_digit_string_to_byte_string(x.second);
          x.second = surf::SuRF_Utils::extend_string_to_length(
              x.second, len_condensed_key, (char)0);
        }
      }
      std::sort(sorted_range_tombstones_str.begin(),
                sorted_range_tombstones_str.end());
      bool flag_first = true;
      for (auto& x_range : sorted_range_tombstones_str) {
        if (flag_first == true) {
          merged_sorted_range_tombstones_str.push_back(x_range);
          flag_first = false;
          continue;
        }

        if (x_range.first <= merged_sorted_range_tombstones_str.back().second) {
          merged_sorted_range_tombstones_str.back().second = std::max(
              x_range.second, merged_sorted_range_tombstones_str.back().second);
        } else {
          merged_sorted_range_tombstones_str.push_back(x_range);
        }
      }
      surf_level_file_rdf->RemoveSuRF(src_level_list, src_fd_list2d);
      std::vector<pss> range_tombstone_merged;

      range_tombstone_merged = merged_sorted_range_tombstones_str;

      size_t len_rd = range_tombstone_merged.size();
      if (len_rd > 0) {
        std::vector<uint64_t> dst_fd_list;
        std::vector<pss> file_boundary_list;
        for (auto& dst_level_info : dst_level_info_list) {
          uint64_t& dst_fd = dst_level_info.fd;
          pss& file_boundary = dst_level_info.file_boundary;
          dst_fd_list.push_back(dst_fd);
          file_boundary_list.push_back(file_boundary);
        }

        if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
            true) {
          for (auto& x : file_boundary_list) {
            uint32_t len_condensed_key =
                surf::SuRF_Env::getInstance()->getLengthOfCondensedDigitKey();
            x.first =
                surf::SuRF_Utils::encode_digit_string_to_byte_string(x.first);
            x.first = surf::SuRF_Utils::extend_string_to_length(
                x.first, len_condensed_key, (char)0);
            x.second =
                surf::SuRF_Utils::encode_digit_string_to_byte_string(x.second);
            x.second = surf::SuRF_Utils::extend_string_to_length(
                x.second, len_condensed_key, (char)0);
          }
        }

        surf_level_file_rdf->shiftRDFToOutputLevel(
            range_tombstone_merged, dst_level, dst_fd_list, file_boundary_list,
            surf_flag__allow_range_boundary_overlapped);
      }
    }
  }

  // SuRF Split Compaction
  if (delta.split_surf_compaction_info) {
    if (!surf_level_file_split_rdf) {
      surf_level_file_split_rdf =
          std::make_shared<surf::SuRF_RDF>(surf::SuRF_RDF::PER_FILE);
    } else {
      surf_level_file_split_rdf =
          std::make_shared<surf::SuRF_RDF>(*surf_level_file_split_rdf);
    }

    bool surf_flag__allow_range_boundary_overlapped =
        surf::SuRF_Env::getInstance()->getFlagAllowRangeBoundaryOverlapped();
    std::vector<SuRFCompactionSourceLevelInfo>& src_level_info_list =
        delta.split_surf_compaction_info->src_level_info_list;
    uint32_t dst_level = delta.split_surf_compaction_info->dst_level;
    std::vector<SuRFCompactionDstinationLevelInfo>& dst_level_info_list =
        delta.split_surf_compaction_info->dst_level_info_list;
    bool flag_direct_move_to_dst_level =
        delta.split_surf_compaction_info->flag_direct_move_to_dst_level;

    if (flag_direct_move_to_dst_level == true) {
      for (auto& src_level_info : src_level_info_list) {
        uint32_t src_level = src_level_info.src_level;
        std::vector<uint64_t> src_fd_list = src_level_info.src_fd_list;
        for (auto& fd : src_fd_list) {
          surf_level_file_split_rdf->directMoveFileToLevel(fd, src_level,
                                                           dst_level);
        }
      }
    } else {
      std::vector<uint32_t> src_level_list;
      std::vector<std::vector<uint64_t>> src_fd_list2d;
      std::vector<pss> sorted_range_tombstones_str;
      std::vector<pss> merged_sorted_range_tombstones_str;
      for (auto& src_level_info : src_level_info_list) {
        uint32_t& src_level = src_level_info.src_level;
        std::vector<uint64_t>& src_fd_list = src_level_info.src_fd_list;

        std::vector<pss>& range_tombstones_str =
            src_level_info.range_tombstones_str;
        src_level_list.push_back(src_level);
        src_fd_list2d.push_back(src_fd_list);

        sorted_range_tombstones_str.insert(sorted_range_tombstones_str.end(),
                                           range_tombstones_str.begin(),
                                           range_tombstones_str.end());
      }

      if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
          true) {
        for (auto& x : sorted_range_tombstones_str) {
          uint32_t len_condensed_key =
              surf::SuRF_Env::getInstance()->getLengthOfCondensedDigitKey();
          x.first =
              surf::SuRF_Utils::encode_digit_string_to_byte_string(x.first);
          x.first = surf::SuRF_Utils::extend_string_to_length(
              x.first, len_condensed_key, (char)0);
          x.second =
              surf::SuRF_Utils::encode_digit_string_to_byte_string(x.second);
          x.second = surf::SuRF_Utils::extend_string_to_length(
              x.second, len_condensed_key, (char)0);
        }
      }
      std::sort(sorted_range_tombstones_str.begin(),
                sorted_range_tombstones_str.end());
      bool flag_first = true;
      for (auto& x_range : sorted_range_tombstones_str) {
        if (flag_first == true) {
          merged_sorted_range_tombstones_str.push_back(x_range);
          flag_first = false;
          continue;
        }

        if (x_range.first <= merged_sorted_range_tombstones_str.back().second) {
          merged_sorted_range_tombstones_str.back().second = std::max(
              x_range.second, merged_sorted_range_tombstones_str.back().second);
        } else {
          merged_sorted_range_tombstones_str.push_back(x_range);
        }
      }

      surf_level_file_split_rdf->RemoveSuRF(src_level_list, src_fd_list2d);
      std::vector<pss> range_tombstone_merged;

      range_tombstone_merged = merged_sorted_range_tombstones_str;

      auto in_coming_point_keys_str =
          const_cast<std::vector<std::string>&>(delta.split_sampled_keys_str);

      size_t len_rd = range_tombstone_merged.size();
      if (len_rd > 0) {
        std::vector<uint64_t> dst_fd_list;
        std::vector<pss> file_boundary_list;
        for (auto& dst_level_info : dst_level_info_list) {
          uint64_t& dst_fd = dst_level_info.fd;
          pss& file_boundary = dst_level_info.file_boundary;
          dst_fd_list.push_back(dst_fd);
          file_boundary_list.push_back(file_boundary);
        }
        if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
            true) {
          uint32_t len_condensed_key =
              surf::SuRF_Env::getInstance()->getLengthOfCondensedDigitKey();
          for (uint32_t i_pk = 0; i_pk < in_coming_point_keys_str.size();
               i_pk++) {
            auto x = in_coming_point_keys_str[i_pk];
            x = surf::SuRF_Utils::encode_digit_string_to_byte_string(x);
            x = surf::SuRF_Utils::extend_string_to_length(x, len_condensed_key,
                                                          (char)0);
            in_coming_point_keys_str[i_pk] = x;
          }
        }
        if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
            true) {
          for (auto& x : file_boundary_list) {
            uint32_t len_condensed_key =
                surf::SuRF_Env::getInstance()->getLengthOfCondensedDigitKey();
            x.first =
                surf::SuRF_Utils::encode_digit_string_to_byte_string(x.first);
            x.first = surf::SuRF_Utils::extend_string_to_length(
                x.first, len_condensed_key, (char)0);
            x.second =
                surf::SuRF_Utils::encode_digit_string_to_byte_string(x.second);
            x.second = surf::SuRF_Utils::extend_string_to_length(
                x.second, len_condensed_key, (char)0);
          }
        }

        surf_level_file_split_rdf->shiftRDFWithPointKeysToOutputLevel(
            range_tombstone_merged, in_coming_point_keys_str, dst_level,
            dst_fd_list, file_boundary_list,
            surf_flag__allow_range_boundary_overlapped);
      }
    }
  }

  // Skyline RDF
  if (!delta.skyline_delta.empty()) {
    skyline_rdf.addRangeTombstones(delta.skyline_delta);
  }
}

inline void VersionRDFBundle::ApplyDirectRemovalDelta(
    const RDFUpdateMetadata& delta) {
  // Option 1: Explicit removal vectors (requested by user)
  if (!delta.plrdf_tracing_remove.empty()) {
    for (const auto& pair : delta.plrdf_tracing_remove) {
      plrdf.removeTracingRangesAtLevelOfFd(pair.first, pair.second);
    }
  }
  if (!delta.split_plrdf_tracing_remove.empty()) {
    for (const auto& pair : delta.split_plrdf_tracing_remove) {
      split_plrdf.removeTracingRangesAtLevelOfFd(pair.first, pair.second);
    }
  }
  if (!delta.plrdf_stringkey_tracing_remove.empty()) {
    for (const auto& pair : delta.plrdf_stringkey_tracing_remove) {
      plrdf_stringkey.removeTracingRangesAtLevelOfFd(pair.first, pair.second);
    }
  }
  if (!delta.split_plrdf_stringkey_tracing_remove.empty()) {
    for (const auto& pair : delta.split_plrdf_stringkey_tracing_remove) {
      split_plrdf_stringkey.removeTracingRangesAtLevelOfFd(pair.first,
                                                           pair.second);
    }
  }

  // 3. Direct Removal (opt 3) - FIFO Deletion
  // PLRDF variants
  for (const auto& tup : delta.plrdf_compaction_delta) {
    // tuple format: (in_lvl, out_lvl(-1), rds{}, fd_list{only 1 fd})
    // deleteRDFAssociatedWithFilesAtCurrentLevel expects tuple pointer
    auto file_val =
        std::make_tuple(std::get<0>(tup), std::get<2>(tup), std::get<3>(tup));
    plrdf.deleteRDFAssociatedWithFilesAtCurrentLevel(&file_val);
#ifdef CHECK_PLRDF_TRACING_CONSISTENCY
    plrdf.checkTracingConsistency(std::get<0>(tup));
#endif
  }
  for (const auto& tup : delta.plrdf_stringkey_compaction_delta) {
    auto file_val =
        std::make_tuple(std::get<0>(tup), std::get<2>(tup), std::get<3>(tup));
    plrdf_stringkey.deleteRDFAssociatedWithFilesAtCurrentLevel(&file_val);
#ifdef CHECK_PLRDF_TRACING_CONSISTENCY
    plrdf_stringkey.checkTracingConsistency(std::get<0>(tup));
#endif
  }

  // Split PLRDF variants
  for (const auto& tup : delta.split_plrdf_compaction_delta) {
    auto file_val =
        std::make_tuple(std::get<0>(tup), std::get<2>(tup), std::get<3>(tup));
    split_plrdf.deleteRDFAssociatedWithFilesAtCurrentLevel(&file_val);
#ifdef CHECK_PLRDF_TRACING_CONSISTENCY
    split_plrdf.checkTracingConsistency(std::get<0>(tup));
#endif
  }
  for (const auto& tup : delta.split_plrdf_stringkey_compaction_delta) {
    auto file_val =
        std::make_tuple(std::get<0>(tup), std::get<2>(tup), std::get<3>(tup));
    split_plrdf_stringkey.deleteRDFAssociatedWithFilesAtCurrentLevel(&file_val);
#ifdef CHECK_PLRDF_TRACING_CONSISTENCY
    split_plrdf_stringkey.checkTracingConsistency(std::get<0>(tup));
#endif
  }

  // TopLevel RDF variants
  // for (const auto& tup : delta.top_level_plrdf_compaction_delta) {
  //   auto file_val =
  //       std::make_tuple(std::get<0>(tup), std::get<2>(tup),
  //       std::get<3>(tup));
  //   top_level_rdf.deleteRDFAssociatedWithFilesAtCurrentLevel(&file_val);
  // }
  // for (const auto& tup : delta.top_level_plrdf_stringkey_compaction_delta)
  // {
  //   auto file_val =
  //       std::make_tuple(std::get<0>(tup), std::get<2>(tup),
  //       std::get<3>(tup));
  //   top_level_rdf_stringkey.deleteRDFAssociatedWithFilesAtCurrentLevel(
  //       &file_val);
  // }

  // SuRF variants
  if (delta.surf_direct_removal_info) {
    if (!surf_level_file_rdf) {
      // Should exist if we are deleting from it, but consistency check
      surf_level_file_rdf =
          std::make_shared<surf::SuRF_RDF>(surf::SuRF_RDF::PER_FILE);
    } else {
      surf_level_file_rdf =
          std::make_shared<surf::SuRF_RDF>(*surf_level_file_rdf);
    }
    uint32_t level = delta.surf_direct_removal_info->src_level;
    for (auto fd : delta.surf_direct_removal_info->src_fd_list) {
      surf_level_file_rdf->removeSuRFAtLevelOfFd(level, fd);
    }
  }

  // Split SuRF variants
  if (delta.split_surf_direct_removal_info) {
    if (!surf_level_file_split_rdf) {
      surf_level_file_split_rdf =
          std::make_shared<surf::SuRF_RDF>(surf::SuRF_RDF::PER_FILE);
    } else {
      surf_level_file_split_rdf =
          std::make_shared<surf::SuRF_RDF>(*surf_level_file_split_rdf);
    }
    uint32_t level = delta.split_surf_direct_removal_info->src_level;
    for (auto fd : delta.split_surf_direct_removal_info->src_fd_list) {
      surf_level_file_split_rdf->removeSuRFAtLevelOfFd(level, fd);
    }
  }
}

void VersionRDFBundle::logCurrentTotalNumbersOfRangesInEachRDF() {
  // uint32_t origin_count =
  // current_->getNumberOfTablesRangeTombstonesInCache();

  // origin_info.logCurrentTotalNumbersOfRanges(origin_count);

  plrdf.logCurrentTotalNumbersOfRanges();
  split_plrdf.logCurrentTotalNumbersOfRanges();
  plrdf_stringkey.logCurrentTotalNumbersOfRanges();
  split_plrdf_stringkey.logCurrentTotalNumbersOfRanges();
  top_level_rdf.logCurrentTotalNumbersOfRanges();
  top_level_rdf_stringkey.logCurrentTotalNumbersOfRanges();
  skyline_rdf.logCurrentTotalNumbersOfRanges();

  if (surf_level_file_rdf) {
    surf_level_file_rdf->logCurrentTotalNumbersOfRanges();
  }
  if (surf_level_file_split_rdf) {
    surf_level_file_split_rdf->logCurrentTotalNumbersOfRanges();
  }
}

void VersionRDFBundle::logCurrentTotalMemoryUsageInEachRDF() {
  // uint32_t origin_bytes = current_->getSizeOfTablesRangeTombstonesInCache();

  // origin_info.logCurrentTotalMemoryUsage(origin_bytes);

  plrdf.logCurrentTotalMemoryUsage();
  split_plrdf.logCurrentTotalMemoryUsage();
  plrdf_stringkey.logCurrentTotalMemoryUsage();
  split_plrdf_stringkey.logCurrentTotalMemoryUsage();
  top_level_rdf.logCurrentTotalMemoryUsage();
  top_level_rdf_stringkey.logCurrentTotalMemoryUsage();
  skyline_rdf.logCurrentTotalMemoryUsage();

  if (surf_level_file_rdf) {
    surf_level_file_rdf->logCurrentTotalMemoryUsage();
  }
  if (surf_level_file_split_rdf) {
    surf_level_file_split_rdf->logCurrentTotalMemoryUsage();
  }
}

#endif  // SYS_RDFILTER_H
