/*
 *  Created on: May 14, 2019
 *  Author: Subhadeep, Papon
 */

#ifndef PERLEVEL_RANGE_DELETE_FILTER_H_
#define PERLEVEL_RANGE_DELETE_FILTER_H_


namespace range_delete_filter {
  class PerlevelRangeDeleteFilterBySet;
  class PerlevelRangeDeleteFilterByVector;
  

  // using PerlevelRangeDeleteFilter = PerlevelRangeDeleteFilterBySet; // choose the implementation here
  using PerlevelRangeDeleteFilter = PerlevelRangeDeleteFilterByVector; // choose the implementation here
  // typedef PerlevelRangeDeleteFilterByVector PerlevelRangeDeleteFilter;
} 

#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <set>
#include "../emu_environment.h"
#include "../workload_executor.h"
#include "../tree_builder/entry.h"




using namespace std;
using namespace workload_exec;
using namespace tree_builder;

namespace range_delete_filter {

  using pll = pair<long, long>;

  class PerlevelRangeDeleteFilterBySet {  
      
    private:
      set<pll> range_delete_list; //list of range delete (start, end), all entries are non-overlapping
      
    public:
      set<pll> getRangeDeleteList();
      void addRangeDelete(long start, long end);
      void addRangeDelete(vector<RangeEntry> &inserted_range_delete_list);
      void removeRangeDelete(vector<RangeEntry> &range_delete_discard_list);
      void removeRangeDelete(vector<pll> &range_delete_discard_list);
      //TODO: implement ----------
      // long getIthRangeDeleteStartKey(int i);
      // long getIthRangeDeleteEndKey(int i);
      void splitRangeByPointEntries(vector<PointEntry> &point_list);
      //--------------------------

      void printRangeDeleteList();

      EntryList* applyRangeFilter(long start, long end);
      bool isEntryAlive(long start);

      int getRangeDeleteCount();
  };


  class PerlevelRangeDeleteFilterByVector {  
    private:
      vector<pll> range_delete_list; //list of range delete (start, end), all entries are non-overlapping
      
    public:
      vector<pll> getRangeDeleteList();
      void addRangeDelete(long start, long end);
      void addRangeDelete(vector<RangeEntry> &inserted_range_delete_list);
      void removeRangeDelete(vector<RangeEntry> &range_delete_discard_list);
      void removeRangeDelete(vector<pll> &range_delete_discard_list);
      // long getIthRangeDeleteStartKey(int i);
      // long getIthRangeDeleteEndKey(int i);
      void splitRangeByPointEntries(vector<PointEntry> &point_list);


      void printRangeDeleteList();

      EntryList* applyRangeFilter(long start, long end);  //TODO: implement this, return shall be vector<RangeEntry>
      bool isEntryAlive(long start);

      int getRangeDeleteCount();

  };
} // namespace

#endif /* PERLEVEL_RANGE_DELETE_FILTER_H_ */
