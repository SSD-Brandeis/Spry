#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <set>
#include "../tree_builder/tree_builder.h"
#include "../tree_builder/entry.h"
#include "../tree_builder/entry.cc"
#include "../workload_executor.h"


using namespace std;
using namespace tree_builder;

using KV = pair < pair < long, long> , string>;
using pll = pair<long, long>;

void insert(EntryList& entrylist);
EntryList split_out_for_file(EntryList &entries_to_compact);



int main(){
    EntryList entry_list;
    insert(entry_list);
    entry_list.print();
    vector<RangeEntry> range_entry_vector;
    range_entry_vector.push_back(RangeEntry(pll(2,50), -1));
    range_entry_vector.push_back(RangeEntry(pll(53,70), -1));
    range_entry_vector.push_back(RangeEntry(pll(80,150), -1));
    DiskMetaFile::addRangeDeleteToRangeDeleteFilterAtLevel(0, range_entry_vector);
    cout<< "insert range" <<endl;
    DiskMetaFile::split_level_range_delete_filter_by_entry_list(0, entry_list);
    DiskMetaFile::printAllLevelRangeDeleteFilter();
    




    // Utility::compactAndFlush_RDF(entry_list, 1);
    // DiskMetaFile::getMetaStatistics();
    // DiskMetaFile::printAllEntries(0);

    return 0;
}


void insert(EntryList& entrylist){
    PointEntry point_entry = PointEntry(KV(pll(5,6), "hello"), -1);
    PointEntry point_entry2 = PointEntry(KV(pll(10,7), "world"), -1);
    PointEntry point_entry3 = PointEntry(KV(pll(30,8), "hello"), -1);
    PointEntry point_entry4 = PointEntry(KV(pll(43,9), "world"), -1);
    PointEntry point_entry5 = PointEntry(KV(pll(53,10), "hello"), -1);
    PointEntry point_entry6 = PointEntry(KV(pll(67,11), "world"), -1);
    PointEntry point_entry7 = PointEntry(KV(pll(100,12), "hello"), -1);
    PointEntry point_entry8 = PointEntry(KV(pll(110,13), "world"), -1);
    entrylist.add_point_entry(point_entry).add_point_entry(point_entry2).add_point_entry(point_entry3).add_point_entry(point_entry4);
    entrylist.add_point_entry(point_entry5).add_point_entry(point_entry6).add_point_entry(point_entry7).add_point_entry(point_entry8);
    
    // RangeEntry range_entry = RangeEntry(pll(5,6), -1);
    // RangeEntry range_entry2 = RangeEntry(pll(6,7), -1);
    // RangeEntry range_entry3 = RangeEntry(pll(7,8), -1);
    // RangeEntry range_entry4 = RangeEntry(pll(8,9), -1);
    // RangeEntry range_entry5 = RangeEntry(pll(9,10), -1);
    // RangeEntry range_entry6 = RangeEntry(pll(10,11), -1);
    // RangeEntry range_entry7 = RangeEntry(pll(11,12), -1);
    // RangeEntry range_entry8 = RangeEntry(pll(12,13), -1);
    // entrylist.add_range_entry(range_entry).add_range_entry(range_entry2).add_range_entry(range_entry3).add_range_entry(range_entry4);
    // entrylist.add_range_entry(range_entry5).add_range_entry(range_entry6).add_range_entry(range_entry7).add_range_entry(range_entry8);
}


EntryList split_out_for_file(EntryList &entries_to_compact)
{
    int file_size = 128*8;
    vector<KV>::iterator kv_iter = entries_to_compact.kv_vector.begin(); 
    vector<KV>::iterator kv_end = entries_to_compact.kv_vector.end();
    vector<pll>::iterator pll_iter = entries_to_compact.range_delete_tombstone_vector.begin();
    vector<pll>::iterator pll_end = entries_to_compact.range_delete_tombstone_vector.end();
    // using 2 pointer points to the 2 first elements of vector

    EntryList entries_to_populate_file; // entrylist that gonna be output 
    int current_file_size = 0;

    while(current_file_size < file_size)
    {  // the file is not full
        kv_end = entries_to_compact.kv_vector.end(); //  end
        pll_end = entries_to_compact.range_delete_tombstone_vector.end(); // end

        if(kv_iter == kv_end && pll_iter == pll_end) break; // if we iterate all the element, break
        
        if(pll_iter == pll_end)
        { // if range delete ends, push kv
            entries_to_populate_file.kv_vector.push_back(*kv_iter);
            current_file_size += 128; // may change
            kv_iter = entries_to_compact.kv_vector.erase(kv_iter); // erase the element and move the iterator
            continue;
        }

        if(kv_iter == kv_end)
        { // if kv ends, push range deletes
            entries_to_populate_file.range_delete_tombstone_vector.push_back(*pll_iter);
            current_file_size += (8*2); // may change
            pll_iter = entries_to_compact.range_delete_tombstone_vector.erase(pll_iter); // erase the element and move the iterator
            continue;
        }

        if(kv_iter->first.first <= pll_iter->second)
        {
            entries_to_populate_file.kv_vector.push_back(*kv_iter);
            current_file_size += 128;
            kv_iter = entries_to_compact.kv_vector.erase(kv_iter); // erase the element and move the iterator
        }
        else{
            entries_to_populate_file.range_delete_tombstone_vector.push_back(*pll_iter);
            current_file_size += (8*2); // may change
            pll_iter = entries_to_compact.range_delete_tombstone_vector.erase(pll_iter); // erase the element and move the iterator
        }
cout << "_" <<endl;
entries_to_populate_file.print();
    } 

    return entries_to_populate_file;  
}     