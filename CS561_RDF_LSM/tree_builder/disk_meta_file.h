/*
 *  Created on: May 14, 2019
 *  Author: Subhadeep, Papon
 */

#ifndef LSM___DISK_META_FILE_H_
#define LSM___DISK_META_FILE_H_


namespace tree_builder {
  class DiskMetaFile;
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
#include "../range_delete_filter/range_delete_filter.h"




using namespace std;
using namespace workload_exec;
using namespace range_delete_filter;
using PL_RDF = PerlevelRangeDeleteFilter;
using Skyline_RDF = SkylineRangeDeleteFilter;

namespace tree_builder {

  class DiskMetaFile {
    private: 
    static pair <long, long> getMatchingKeyFile(long min_key, long max_key, int key_level); //??? Definition change hobe
    static vector<PL_RDF> per_level_range_delete_filter; //ranges don't split when inserts come//added by ychaung
    static vector<PL_RDF> per_level_range_delete_filter_split; //ranges split when inserts come //added by ychaung
    static Skyline_RDF skyline_range_delete_filter; //added by ychaung

    public:
    // static SSTFile* head_level_1;


    static int checkAndAdjustLevelSaturation(int level);
    static int checkAndAdjustLevelSaturation_RDF(int level);
    static long getLevelPointEntryCount(int level);
    static long getLevelRangeEntryCount(int level);
    static int getLevelFileCount(int level);
    static int getTotalLevelCount();
    static int checkOverlapping(SSTFile *file, int level);

    static int setSSTFileHead(SSTFile* arg, int level);
    static SSTFile* getSSTFileHead(int level);

    static int getTotalRangeDeleteCountOfPerLevelRangeDeleteFilter();
    static int getTotalRangeDeleteWithoutLastLevelCountOfPerLevelRangeDeleteFilter();
    static int getTotalRangeDeleteCountOfSplitPerLevelRangeDeleteFilter();
    static int getTotalRangeDeleteWithoutLastLevelCountOfSplitPerLevelRangeDeleteFilter();
    static int getTotalRangeDeleteCountOfSkylineRangeDeleteFilter();


    static long getTotalPointEntryCount();
    static long getTotalRangeEntryCount();
    static void getMetaStatistics();
    static int printAllEntries(int only_file_meta_data);
    static int printAllEntriesStyle2(int only_file_meta_data);
    static int printAllLevelRangeDeleteFilter();
    static int printAllLevelSplitRangeDeleteFilter();
    static int printSkylineRangeDeleteFilter();
    // static int getTotalPageCount();
    static int getTotalPointPageCount();
    static int getTotalRangePageCount();
    static int clearAllEntries();

    //for per_level_range_delete_filter
    static void addRangeDeleteToRangeDeleteFilterAtLevel(int level, vector<RangeEntry> range_delete_list);
    static void removeRangeDeleteFromRangeDeleteFilterAtLevel(int level, vector<RangeEntry> range_delete_discard_list);
    static bool isAliveThroughRangeDeleteFilterAtLevel(int level, long key);

    //for per_level_range_delete_filter_split
    static void addRangeDeleteToSplitRangeDeleteFilterAtLevel(int level, vector<RangeEntry> range_delete_list);
    static void removeRangeDeleteFromSplitRangeDeleteFilterAtLevel(int level, vector<RangeEntry> range_delete_discard_list);
    static bool isAliveThroughSplitRangeDeleteFilterAtLevel(int level, long key);
    static void splitRangesInSplitRangeDeleteFilterAtLevelByEntryList(int level, EntryList &entrylist);


    static void addRangeDeleteToSkylineRangeDeleteFilter(long start_key, long end_key, long timestamp);
    static bool isAliveThroughSkylineRangeDeleteFilter(long key, long timestamp);


    static SSTFile* level_head[32];

    static long level_min_key[32];
    static long level_max_key[32];
    static long level_file_count[32];
    static long level_entry_count[32];
    static long level_current_size[32];

    static long level_max_size[32];
    static long level_max_file_count[32];
    static long global_level_file_counter[32];
    static float disk_run_flush_threshold[32];

    static int compaction_counter[32];
    static int compaction_through_sortmerge_counter[32];
    static int compaction_through_point_manipulation_counter[32];
    static int compaction_file_counter[32];

    ////static disk_run_flush_threshold;
  };

} // namespace

#endif /* LSM___DISK_META_FILE_H_ */
