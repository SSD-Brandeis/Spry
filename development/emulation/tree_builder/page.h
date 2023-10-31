/*
 *  Created on: May 14, 2019
 *  Author: Subhadeep, Papon
 */


#ifndef LSM___PAGE_H_
#define LSM___PAGE_H_


namespace tree_builder {
  class Page;
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
#include "entry.h"



using namespace std;
using namespace workload_exec;

namespace tree_builder {

  class Page {
    private:
      vector<PointEntry> point_entry_vector;   //ADDED
      vector<RangeEntry> range_entry_vector;   //ADDED 


      long min_sort_key;
      long max_sort_key;
      long min_delete_key;
      long max_delete_key;

      long min_range_key;
      long max_range_key;
      
    public:
      enum PageType {NOT_SET_YET, POINT, RANGE};
 
    // long min_sort_key;
    // long max_sort_key;
    // long min_delete_key;
    // long max_delete_key;
    // vector <pair < pair < long, long> , string>> kv_vector;

    
    // vector <Entry> entry_vector;
    
    // static struct vector<Page> createNewPages(int page_count);


    
      PageType page_type;

      Page();
      Page& setPageType(PageType page_type);
      PageType getPageType();

      Page& setMinSortKey(long min_sort_key);
      Page& setMaxSortKey(long max_sort_key);
      Page& setMinDeleteKey(long min_delete_key);
      Page& setMaxDeleteKey(long max_delete_key);
      Page& setMinRangeKey(long min_range_key);
      Page& setMaxRangeKey(long max_range_key);

      long getMinSortKey();
      long getMaxSortKey();
      long getMinDeleteKey();
      long getMaxDeleteKey();
      long getMinRangeKey();
      long getMaxRangeKey();

      Page& addPointEntry(PointEntry point_entry);
      Page& addRangeEntry(RangeEntry range_entry);
      vector<PointEntry> getPointEntryVector();
      vector<RangeEntry> getRangeEntryVector();
      long getPointEntryVectorSize(); 
      long getRangeEntryVectorSize();
      PointEntry& getIthPointEntry(int i);
      RangeEntry& getIthRangeEntry(int i);
      void clearPointEntryVector();
      void clearRangeEntryVector();
      void clearAllVectors();
      void setAllKeysToDefaultNegativeOne();

      void sortPointEntryVectorOnSortKeyInAscendingOrder();
      void sortPointEntryVectorOnDeleteKeyInAscendingOrder();
      void sortRangeEntryVectorOnStartKeyInAscendingOrder();

      static struct vector<Page> createNewPointPages(int page_count);
      static struct vector<Page> createNewRangePages(int page_count);
  };

} // namespace

#endif /* LSM___PAGE_H_ */