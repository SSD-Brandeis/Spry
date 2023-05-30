/*
 *  Created on: Jan 28, 2019
 *  Author: Papon
 */


#ifndef QUERY_H_
#define QUERY_H_


class Query;

#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <fstream>

#include "emu_environment.h"
#include "tree_builder/tree_builder.h"
#include "workload_generator.h"

using namespace std;


class Query
{
private:
    static long disk_sstfile_access_count;
  
public:
    static void incDiskSSTFileAccessCount();
    static long getDiskSSTFileAccessCount();
    static void resetDiskSSTFileAccessCount();


    static int delete_key;
    static int range_start_key;
    static int range_end_key;
    static int sec_range_start_key;
    static int sec_range_end_key;
    static int iterations_point_query;
    

    static int complete_delete_count;
    static int not_possible_delete_count;
    static int partial_delete_count;

    static int range_occurances;
    static int secondary_range_occurances;

    static long sum_page_id;
    static long found_count;
    static long not_found_count;

    static void checkDeleteCount (int deletekey);
    static int pointQuery (int key);
    static pair<bool, string> pointQuery_RDF (long key);
    static pair<bool, string> pointQuery_SplitRDF (long key);
    static pair<bool, string> pointQuery_RDF_SkyLine (long key, long timetag);
    static void pointQueryRunner (int iterations);
    static void rangeQuery (int lowerlimit, int upperlimit);
    static void secondaryRangeQuery (int lowerlimit, int upperlimit);

    static void range_query_experiment();
    static void sec_range_query_experiment();
    static void delete_query_experiment();
    static void point_query_experiment();
    static void new_point_query_experiment();

    static void interactive_point_query();

};

#endif /* QUERY_H_ */