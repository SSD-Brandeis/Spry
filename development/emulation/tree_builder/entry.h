/*
 *  Created on: May 14, 2019
 *  Author: Subhadeep, Papon
 */


#ifndef LSM___ENTRY_H_
#define LSM___ENTRY_H_

namespace tree_builder {
  class PointEntry;
  class RangeEntry;
  class EntryList;
} 

#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include "../emu_environment.h"
#include "../workload_executor.h"


#include "delete_tile.h"
#include "disk_meta_file.h"
#include "memory_buffer.h"
#include "page.h"
#include "sst_file.h"



using namespace std;
using namespace workload_exec;

using KV = pair < pair < long, long> , string>;
using pll = pair<long, long>;

namespace tree_builder {
  class PointEntry{
    private:
      long sort_key;
      long delete_key;
      string value;
      long timetag;
    public:
      // Entry(long key, long delete_key, string value, long timetag);
      PointEntry(KV kv, long timetag);
      KV get_keyvalue_pair();
      long getSortKey();
      long getDeleteKey();
      string getValue();
      void setValue(string value);
      long getTimetag();
  };

  class RangeEntry{
    private:
      long start_key, end_key; //on primary key
      long timetag;
    public:
      // Range_Entry(long start, long end, long timetag);
      RangeEntry(pll range, long timetag);
      pll get_pll_pair();
      long getStartKey();
      long getEndKey();
      long getTimetag();
      void setStartKey(long start_key);
      void setEndKey(long end_key);
  };


  //for migration phase
  class EntryList {
    private:
      vector<PointEntry> point_entry_vector;
      vector<RangeEntry> range_entry_vector;

    public: 
    vector <KV> kv_vector;
    vector<pll> range_delete_tombstone_vector;

    //TODO
    void add_entry(KV kv);
    void add_range_delete_tombstone(pll range_delete_tombstone);
    vector<KV> find_overlap_kv_pairs(vector<KV> other_kv_vector);
    vector<pll> find_overlap_range_delete_tombstones(vector<pll> other_range_delete_tombstone_vector);


    //Really used ones
    EntryList& add_point_entry(PointEntry point_entry);
    EntryList& add_range_entry(RangeEntry range_entry);

    vector<PointEntry> get_point_entry_vector();
    vector<RangeEntry> get_range_entry_vector();

    void remove_point_entries_in_range(long start_key, long end_key);

    vector<PointEntry> extract_point_entry_from_overlapping_file(SSTFile *sst_file_level_head);   // only file points that at least a point falls into
    vector<PointEntry> apply_range_delete_on_point_entry_vector(vector<PointEntry> &next_level_point_entry_vector);

    void merge_its_own_range_entry();
    void merge_with_another_point_entry_vector(vector<PointEntry> &next_level_point_entry_vector);
    vector<RangeEntry> extract_range_entry_from_overlapping_file(SSTFile *sst_file_level_head); // only file ranges that at least a range has a overlap with it
    void merge_with_another_range_entry_vector(vector<RangeEntry> &next_level_range_entry_vector);

    // files (ranges + points) that the full min-max range (min(point_min, range_min), max(point_max, range_max)) has overlap with it
    EntryList get_minmax_range_overlapped_entries_from_files(SSTFile *sst_file_level_head); 
    //********************************************************************************************************************************

    EntryList split_out_by_size_inByte(int split_size);
    bool empty();
    bool PointEntry_empty();
    bool RangeEntry_empty();
    void clear();
    void print();
    int get_PointEntry_size();
    int get_RangeEntry_size();
    int get_entrylist_size_inByte();
    int get_point_entry_vector_size_inByte();
    int get_range_entry_vector_size_inByte();
    int get_min_key();  //min(range_start_key, point_sort_key) default: -1 
    int get_max_key();  //max(range_start_key, point_sort_key) default: -1
    void sortRangeEntryVectorOnStartKeyInAscendingOrder();
    void sortRangeEntryVectorOnEndKeyInAscendingOrder();
    void sortPointEntryVectorOnSortKseyInAscendingOrder();

    int get_point_entry_vector_size();
    int get_range_entry_vector_size();
    PointEntry &get_ith_point_entry(int i);
    RangeEntry &get_ith_range_entry(int i);

    void pop_point_entry_vector_back();
    
  };

} // namespace

#endif /* LSM___ENTRY_H_ */