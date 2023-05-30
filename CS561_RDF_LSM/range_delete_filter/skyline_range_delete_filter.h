/*
 *  Created on: May 14, 2019
 *  Author: Subhadeep, Papon
 */

#ifndef SKYLINE_RANGE_DELETE_FILTER_H_
#define SKYLINE_RANGE_DELETE_FILTER_H_


namespace range_delete_filter {
  class SkylineRangeDeleteFilter;
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

  struct range_delete_entry {
    long start;
    long end;
    long timetag;

    bool operator<(const range_delete_entry& other) const {
      if(start < other.start){return true;}
      if(end < other.end){return true;}
      // return timetag <= other.timetag;
      return false;
    }
  };

  class SkylineRangeDeleteFilter {  
      
    private:
      // set<pll> range_delete_list; //list of range delete (start, end), all entries are non-overlapping
      set<range_delete_entry> range_delete_with_timetag_list; //list of range delete (start, end, timetag), all entries are non-overlapping
      
    public:
      // SkylineRangeDeleteFilter(){
      //   range_delete_list = new set<pll>();
      //   range_delete_with_timetag_list = new set<range_delete_entry>();
      // }
      // set<pll> getRangeDeleteList();
      set<range_delete_entry> getRangeDeleteWithTimetagList();
      // void addRangeDelete(long start, long end);
      void addRangeDeleteWithTimetag(long start, long end, long timetag);


      // void printRangeDeleteList();
      void printRangeDeleteWithTimetagList();


            
      // EntryList* applyRangeFilterWithoutTimetag(long start, long end);
      EntryList* applyRangeFilterWithTimetag(long start, long end, long timetag);
      
      // bool isEntryAliveWithoutTimetag(long start);
      bool isEntryAliveWithTimetag(long start, long timetag);

      int getRangeDeleteWithTimetagCount();

  };
} // namespace

#endif /* SKYLINE_RANGE_DELETE_FILTER_H_ */
