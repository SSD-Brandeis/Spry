/*
 *  Created on: May 14, 2019
 *  Author: Subhadeep, Papon
 */
 
#ifndef AWESOME_H_
#define AWESOME_H_
namespace workload_exec {
    class WorkloadExecutor;
}


#include "emu_environment.h"
#include "tree_builder/entry.h"

#include <iostream>
#include <vector>

using namespace std;

namespace workload_exec {

  class WorkloadExecutor {
    private:
      static void checkBufferFlush();

    public:
    static long total_insert_count;
    static long buffer_update_count;
    static long buffer_insert_count;
    static long total_buffer_range_tombstone_deleted_point_entry_count;
    static uint32_t counter;

    //added by RD
    static long total_range_tombstone_count;
    static long buffer_range_tombstone_count;

    static long total_merge_deleted_entry_count;
    static long total_merge_deleted_point_entry_count;
    static long total_merge_deleted_range_entry_count;
    static long total_range_deleted_point_entry_count;

    static long total_file_boundary_split_range_entry_count;


    // static int insert(long sortkey, long deletekey, string value);
    static int insert(long sortkey, long deletekey, string value, long timetag);
    static int rangeDeleteOnPrimaryKey(long startkey, long endkey, long timetag);
    static int pointGet(long key);
    static int search(long key, int possible_level_of_occurrence);
    static int getWorkloadStatictics(EmuEnv* _env);

    static int getExecutionStatistics();
  };

  class Utility { 
      public:
      static void sortAndWrite(vector < pair < pair < long, long >, string > > vector_to_compact, int level_to_flush_in);
      static void compactAndFlush(vector < pair < pair < long, long >, string > > vector_to_compact, int level_to_flush_in);
      static bool sortbysortkey(const pair<pair<long, long>, string> &a, const pair<pair<long, long>, string> &b);
      static bool sortbydeletekey(const pair<pair<long, long>, string> &a, const pair<pair<long, long>, string> &b);
      static int minInt(int a, int b);
      static void compactAndFlush_RDF(tree_builder::EntryList entries_to_compact, int level_to_flush_in);
      static void sortAndWrite_RD(tree_builder::EntryList entries_to_compact, int level_to_flush_in);
      static void sortAndWrite_RDF(tree_builder::EntryList entries_to_compact, int level_to_flush_in);

  };


} // namespace awesome

#endif /*EMU_ENVIRONMENT_H_*/
