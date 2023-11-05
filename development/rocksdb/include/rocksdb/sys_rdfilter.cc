/*
 *  Created on: April 23, 2023
 *  Author: yucheng
 */

/*Notice: anytime only one lock is locked in this class*/

#ifndef SYS_RDFILTER_CC_
#define SYS_RDFILTER_CC_

#include "sys_rdfilter.h"

using namespace std;
using namespace ROCKSDB_NAMESPACE;

std::vector<pll> PLRDF::sortAndMerge(std::vector<pll> &range_delete_list_in) {
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

void PLRDF::addRangeDelete(std::vector<pll> &range_delete_list,
                           std::vector<pll> &range_delete_list_in) {
  auto &rdList = range_delete_list;
  auto &rdList_in = range_delete_list_in;

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

  if (rdList.size() == 0) {
    // do the merging first before adding to rdList
    rdList.reserve(rdList_in.size());
    auto itA = rdList_in.begin();
    auto iteA = rdList_in.end();
    pll tmp_range = *itA;
    for (; itA != iteA; itA++) {
      if (tmp_range.second >= itA->first) {
        tmp_range.second = std::max(tmp_range.second, itA->second);
      } else {
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
  if (itA->first < itB->first) {
    tmp_range = *itA;
  } else {
    tmp_range = *itB;
  }

  while (itA != iteA || itB != iteB) {
    if (itA != iteA && itA->first <= tmp_range.second) {
      tmp_range.second = std::max(tmp_range.second, itA->second);
      itA++;
      continue;
    }
    if (itB != iteB && itB->first <= tmp_range.second) {
      tmp_range.second = std::max(tmp_range.second, itB->second);
      itB++;
      continue;
    }
    rdList_new.push_back(tmp_range);

    if (itA == iteA) {
      tmp_range = *itB;
      itB++;
      continue;
    }
    if (itB == iteB) {
      tmp_range = *itA;
      itA++;
      continue;
    }

    if (itA->first <= itB->first) {
      tmp_range = *itA;
      itA++;
    } else {
      tmp_range = *itB;
      itB++;
    }
  }
  rdList_new.push_back(tmp_range);

  rdList.clear();
  rdList.reserve(rdList_new.size());
  for (auto &p : rdList_new) {
    rdList.push_back(p);
  }
}

void PLRDF::addRangeDelete(std::vector<pll> &range_delete_list, long long start,
                           long long end) {
  auto &rdList = range_delete_list;
#ifdef DEBUG
  cout << "Adding range delete: " << start << " " << end << endl;
#endif
  std::vector<pll> rdList_new;
  rdList_new.reserve(rdList.size() + 1);

  long long minK = start;
  long long maxK = end;

  auto it = rdList.begin();
  while (it != rdList.end()) {
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
  rdList_new.push_back(pll({minK, maxK}));
  while (it != rdList.end()) {
    rdList_new.push_back(*it);
    it++;
  }

  rdList.clear();
  rdList.reserve(rdList_new.size());
  for (auto &p : rdList_new) {
    rdList.push_back(p);
  }
}

void PLRDF::print_internal() {
  std::cout << std::setfill('-') << std::setw(60) << " START: Print PL RDF "
            << std::setfill('-') << "" << std::endl;
  for (uint l = 0; l < rd_filter.size(); l++) {
    std::cout << "Level: " << l << std::endl;
    auto &rdList = rd_filter[l];
    for (auto it = rdList.begin(); it != rdList.end(); it++) {
      std::cout << "(" << it->first << " " << it->second << ") ";
    }
    std::cout << std::endl;
  }
  std::cout << std::setfill('-') << std::setw(60) << " END: Print PL RDF "
            << std::setfill('-') << "" << std::endl;
}

void PLRDF::adjustRangeDeletesForLevel0Input(
    uint olevel, std::vector<uint64_t> file_numbers) {
  std::vector<pll> to_be_added_in_next_level_rdf;
  for (uint64_t &file_num : file_numbers) {
    {
      auto it = rd_filter_level0.find(file_num);
      if (it == rd_filter_level0.end()) {
        assert(it != rd_filter_level0.end());
        std::cerr << "File number not found in level 0 "
                  << "File number " << file_num << " " << __FILE__ << ":"
                  << __LINE__ << std::endl;
        std::cerr
            << "Remindation: Do the manually flush after all the insert "
               "workload are done. So no entries lie inside memtable anymore. "
               "In case those entries will go through the track of bulk "
               "buiding from WAL and no going through the path of flushJob."
            << "File number " << file_num << " " << __FILE__ << ":" << __LINE__
            << std::endl;
        exit(1);
      }
      auto val = it->second;
      to_be_added_in_next_level_rdf.insert(to_be_added_in_next_level_rdf.end(),
                                           val.begin(), val.end());
      rd_filter_level0.erase(it);
    }
  }
  std::sort(to_be_added_in_next_level_rdf.begin(),
            to_be_added_in_next_level_rdf.end(),
            [](const pll a, const pll b) { return a.first < b.first; });
  addRangeDelete_internal(olevel, to_be_added_in_next_level_rdf);
}

// This would be used for trivial compaction and normal compaction
// input_level, output_level, file_boundaries
void PLRDF::adjustRangeDeletes(uint clevel, uint olevel,
                               std::vector<std::pair<long long, long long>>
                                   one_level_compaction_file_boundaries) {
  std::vector<pll> new_current_level_rdf;
  std::vector<pll> to_be_added_in_next_level_rdf;

  if (rd_filter.size() <= clevel) {
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
    auto file_boundries = *itf;
    pll file_boundry =
        std::make_pair(file_boundries.first, file_boundries.second);

    /*
     *    |--|
     *         -----
     *         |   |
     *         -----
     */
    if (itf == one_level_compaction_file_boundaries.end() ||
        (val.second <= file_boundry.first)) {
      new_current_level_rdf.push_back(val);
      it++;
    }
    /*
     *             |--|
     *     ------
     *     |    |
     *     ------
     */
    else if (val.first > file_boundry.second) {
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
      to_be_added_in_next_level_rdf.push_back(
          std::make_pair(file_boundry.first, val.second));
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
      to_be_added_in_next_level_rdf.push_back(val);
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
          std::make_pair(val.first, file_boundry.second + 1));
      (*it).first = file_boundry.second + 1;
      if ((*it).first >= (*it).second) {
        it++;
      }
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
      to_be_added_in_next_level_rdf.push_back(
          std::make_pair(file_boundry.first, file_boundry.second + 1));
      (*it).first = file_boundry.second + 1;
      if ((*it).first >= (*it).second) {
        it++;
      }
      itf++;
    } else {
      std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__ << " "
                << __FUNCTION__ << std::endl;
      std::cerr << "val.first: " << val.first << " val.second: " << val.second
                << " file_boundry.first: " << file_boundry.first
                << " file_boundry.second: " << file_boundry.second << std::endl;
      assert(false);
      exit(1);
    }
  }

  rd_filter[clevel] = new_current_level_rdf;
  std::sort(to_be_added_in_next_level_rdf.begin(),
            to_be_added_in_next_level_rdf.end(),
            [](const pll a, const pll b) { return a.first < b.first; });

  addRangeDelete_internal(olevel, to_be_added_in_next_level_rdf);
}

void PLRDF::insertRangeDeleteToLevel0(
    uint64_t file_num, std::vector<pll> &range_delete_list_in,
    std::vector<uint64_t> exist_level0_file_nums) {
  std::vector<pll> sorted_merged_rdlist = sortAndMerge(range_delete_list_in);
  if (std::binary_search(exist_level0_file_nums.begin(),
                         exist_level0_file_nums.end(), file_num) == true) {
    std::cerr << "Error: file_num already exists in rd_filter_level0 "
              << "file_num = " << file_num << "\t" << __FILE__ << ":"
              << __LINE__ << " " << __func__ << std::endl;
    exit(1);
  }
  rd_filter_level0[file_num] = sorted_merged_rdlist;
}

void PLRDF::printLevel0() {
  std::cout << "rd_filter_level0"
            << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
            << std::endl
            << std::endl;
  for (auto it = rd_filter_level0.begin(); it != rd_filter_level0.end(); it++) {
    std::cout << "rd_filter_level0 file number: " << it->first
              << " number of RD: " << it->second.size() << std::endl;
    // print all ranges
    for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
      std::cout << "rd_filter_level0"
                << " " << it2->first << " " << it2->second << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl << std::endl;
}

void PLRDF::addRangeDelete(uint level, std::vector<pll> &range_delete_list_in) {
  while (rd_filter.size() <= level) {
    rd_filter.push_back(std::vector<pll>());
  }
  addRangeDelete(rd_filter[level], range_delete_list_in);
}

/*
 *Do insertion, even if the vector is empty, because we need to set
 *condition_variable of mutex (semaphore) for compaction
 */
// input_level, output_level, file_boundries, file_numbers
void PLRDF::shiftRDFToOutputLevel(
    std::vector<std::tuple<int, int, std::vector<pll>, std::vector<uint64_t>>>
        *file_meta_data_vectors) {
  for (auto file_meta_data : *file_meta_data_vectors) {
    int clevel = std::get<0>(file_meta_data);
    if (clevel == 0) {
      adjustRangeDeletesForLevel0Input(std::get<1>(file_meta_data),
                                       std::get<3>(file_meta_data));
    } else {
      // file ranges
      std::vector<std::pair<long long, long long>> one_level_file_boundries;
      auto meta_data = std::get<2>(file_meta_data);

      for (auto meta : meta_data) {
        one_level_file_boundries.push_back(
            std::make_pair(meta.first, meta.second));
      }

      adjustRangeDeletes(std::get<0>(file_meta_data),
                         std::get<1>(file_meta_data), one_level_file_boundries);
    }
  }
}

// this is only used for direct compaction //
// input_level, file_boundries, file_numbers
void PLRDF::deleteRDFAssociatedWithFilesAtCurrentLevel(
    std::tuple<int, std::vector<pll>, std::vector<uint64_t>> *file_meta_data) {
  std::vector<std::pair<long long, long long>> one_level_file_boundries;
  auto level = std::get<0>(*file_meta_data);

  if (level == 0) {
    auto it = rd_filter_level0.find(level);
    if (it == rd_filter_level0.end()) {
      assert(it != rd_filter_level0.end());
      std::cerr << "Error: file_num does not exist in rd_filter_level0"
                << "\t" << __FILE__ << " " << __LINE__ << " " << __func__
                << std::endl;
      exit(1);
    }
    rd_filter_level0.erase(it);
    return;
  }

  if (rd_filter.size() <= (uint)level) {
    return;
  }

  auto meta_data = std::get<1>(*file_meta_data);

  for (auto meta : meta_data) {
    one_level_file_boundries.push_back(std::make_pair(meta.first, meta.second));
  }

  std::vector<pll> new_current_level_rdf;
  auto old_current_level_rdf = rd_filter[level];
  auto it = old_current_level_rdf.begin();
  auto itf = one_level_file_boundries.begin();

  while (it != old_current_level_rdf.end()) {
    pll val = *it;
    auto file_boundries = *itf;
    pll file_boundry =
        std::make_pair(file_boundries.first, file_boundries.second);

    /*
     *    |--|
     *         -----
     *         |   |
     *         -----
     */
    if (itf == one_level_file_boundries.end() ||
        (val.second <= file_boundry.first)) {
      new_current_level_rdf.push_back(val);
      it++;
    }
    /*
     *             |--|
     *     ------
     *     |    |
     *     ------
     */
    else if (val.first > file_boundry.second) {
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
      (*it).first = file_boundry.second + 1;
      if ((*it).first >= (*it).second) {
        it++;
      }
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
      (*it).first = file_boundry.second + 1;
      if ((*it).first >= (*it).second) {
        it++;
      }
      itf++;
    } else {
      std::cerr << "Condition Unchecked " << __FILE__ << ":" << __LINE__ << " "
                << __FUNCTION__ << std::endl;
      std::cerr << "val.first: " << val.first << " val.second: " << val.second
                << " file_boundry.first: " << file_boundry.first
                << " file_boundry.second: " << file_boundry.second << std::endl;
      assert(false);
      exit(1);
    }
  }
  rd_filter[level] = new_current_level_rdf;
}

vector<pll> PLRDF::getLevelRanges(int outlevel) {
  if ((uint)outlevel >= rd_filter.size()) {
    return {};
  }
  return rd_filter[outlevel];
}

void PLRDF::setLevelRanges(vector<pll> level_ranges_in, int outlevel) {
  while (rd_filter.size() <= (uint)outlevel) {
    rd_filter.push_back(vector<pll>());
  }

  if (level_ranges_in != rd_filter[outlevel]) {
    std::cout << "^^^ outlevel: " << outlevel
              << " level_ranges_in.size(): " << level_ranges_in.size()
              << " rd_filter[outlevel].size() " << rd_filter[outlevel].size()
              << std::endl;
    std::cout << "^^^ level_ranges_in: " << std::endl;
    for (auto it = level_ranges_in.begin(); it != level_ranges_in.end(); it++) {
      std::cout << "(" << it->first << " " << it->second << ") ";
    }
    std::cout << std::endl;
    std::cout << "^^^ rd_filter[outlevel]: " << std::endl;
    for (auto it = rd_filter[outlevel].begin(); it != rd_filter[outlevel].end();
         it++) {
      std::cout << "(" << it->first << " " << it->second << ") ";
    }
    std::cout << std::endl;
  }
  rd_filter[outlevel] = level_ranges_in;
}

int PLRDF::getNumberOfTotalLevels() {
  int num = 0;
  int len = rd_filter.size();
  for (int i = 1; i < len; i++) {
    if (rd_filter[i].size() > 0) {
      num = i + 1;
    }
  }
  return num;
}

int PLRDF::getNumberOfTotalRanges() {
  int num = 0;
  for (auto it = rd_filter.begin(); it != rd_filter.end(); it++) {
    num += it->size();
  }
  return num;
}

void PLRDF::print() {
  std::cout << std::setfill('-') << std::setw(60) << " START: Print  RDF "
            << std::setfill('-') << "" << std::endl;
  for (uint l = 0; l < rd_filter.size(); l++) {
    std::cout << "Level: " << l << std::endl;
    auto &rdList = rd_filter[l];
    for (auto it = rdList.begin(); it != rdList.end(); it++) {
      std::cout << "(" << it->first << " " << it->second << ") ";
    }
    std::cout << std::endl;
  }
  std::cout << std::setfill('-') << std::setw(60) << " END: Print  RDF "
            << std::setfill('-') << "" << std::endl;
}

bool PLRDF::isEntryAlive(uint level, long long key) {
  assert(rd_filter.size() > level);

  if (level >= rd_filter.size()) {
    return true;
  }

  auto &rdList = rd_filter[level];
  if (rdList.size() == 0) {
    return true;
  }

  auto it =
      upper_bound(rdList.begin(), rdList.end(), pll(key, key),
                  [](const pll &a, const pll &b) { return a.first < b.first; });
  if (it != rdList.begin()) {
    it--;
  }
  if (key >= it->first && key < it->second) {
    return false;
  }
  return true;
}

void PLRDF::deleteLastLevelIfEqualsBottomLevel(uint bottom_level) {
  if (rd_filter.size() - 1 == bottom_level) {
    rd_filter[bottom_level].clear();
  }
}

void PLRDF::splitRangesOnLevel(uint level, std::vector<long long> keys) {
  if (rd_filter.size() <= level) {
    std::cerr << "Error: splitRangesOnLevel: level: " << level
              << " is not present in PLRDF"
              << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
              << std::endl;
    exit(1);
  }

  auto rdList = rd_filter[level];
  std::vector<pll> rdList_new;

  int idx = 0;
  int len = rdList.size();

  for (auto &key_in : keys) {
    while (idx < len && rdList[idx].second <= key_in) {
      rdList_new.push_back(rdList[idx]);
      idx += 1;
    }
    if (idx < len && rdList[idx].first > key_in) {
      continue;
    }

    if (idx < len && rdList[idx].first == key_in) {
      if (key_in + 1 >= rdList[idx].second) {
        idx += 1;
      } else {
        rdList[idx].first = key_in + 1;
      }
      continue;
    }

    if (idx < len && rdList[idx].first < key_in &&
        rdList[idx].second > key_in) {
      auto tmp = rdList[idx];
      tmp.second = key_in;
      rdList_new.push_back(tmp);
      if (key_in + 1 >= rdList[idx].second) {
        idx += 1;
      } else {
        rdList[idx].first = key_in + 1;
      }
      continue;
    }
  }

  while (idx < len) {
    rdList_new.push_back(rdList[idx]);
    idx += 1;
  }

  std::cout << std::endl;

  rd_filter[level] = rdList_new;
}

void PLRDF::logCurrentTotalNumbersOfRanges() {
  numbers_of_ranges_in_RDF_log.push_back(getNumberOfTotalRanges());
}

std::vector<int> PLRDF::getNumbersOfRangesInRDFLog() {
  return numbers_of_ranges_in_RDF_log;
}

#endif /* SYS_RDFILTER_CC_ */