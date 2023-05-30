#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <set>
#include "../tree_builder/delete_tile.h"
#include "../tree_builder/delete_tile.cc"
#include "../tree_builder/disk_meta_file.h"
#include "../tree_builder/disk_meta_file.cc"
#include "../tree_builder/sst_file.h"
#include "../tree_builder/sst_file.cc"
#include "../tree_builder/page.h"
#include "../tree_builder/page.cc"
#include "../tree_builder/tree_builder.h"
#include "../tree_builder/entry.h"
#include "../tree_builder/entry.cc"
#include "../tree_builder/memory_buffer.h"
#include "../tree_builder/memory_buffer.cc"
#include "../emu_environment.h"
#include "../emu_environment.cc"
#include "../workload_executor.h"
#include "../workload_executor.cc"



using namespace std;
using namespace tree_builder;

using KV = pair < pair < long, long> , string>;
using pll = pair<long, long>;

void insert(EntryList& entrylist);
void sortAndWrite_RD(EntryList &entries_to_compact,int level_to_flush_in);
void mergeRangeDeleteTombstones(SSTFile *moving_head, EntryList &entries_to_compact);
void setupTestEnvironment(EntryList &entries_to_compact);
void printRangeDeleteTombstones(const std::vector<pll> &range_delete_tombstone_vector);

int main(){
    //EntryList entry_list;
    //insert(entry_list);
    //entry_list.print();
    //sortAndWrite_RD(entry_list,1);
    //entry_list.print();

    EntryList entries_to_compact;
    setupTestEnvironment(entries_to_compact);

    std::cout << "Before merging:" << std::endl;
    printRangeDeleteTombstones(entries_to_compact.range_delete_tombstone_vector);
    

        // Get head_level_1 (SSTFile head of the level to flush in)
    SSTFile *head_level_1 = DiskMetaFile::getSSTFileHead(1);
    SSTFile *moving_head = head_level_1;
    mergeRangeDeleteTombstones(moving_head, entries_to_compact);

    std::cout << "After merging:" << std::endl;
    printRangeDeleteTombstones(entries_to_compact.range_delete_tombstone_vector);


    return 0;
}

void mergeRangeDeleteTombstones(SSTFile *moving_head, EntryList &entries_to_compact)
{
    std::vector<pll> merged_tombstones;
    //std::sort(entries_to_compact.range_delete_tombstone_vector.begin(), entries_to_compact.range_delete_tombstone_vector.end());
    sort(entries_to_compact.range_delete_tombstone_vector.begin(), entries_to_compact.range_delete_tombstone_vector.end(), [](const pll& a, const pll& b) {
            return a.first < b.first;
        });
    for (const auto &current_tombstone : entries_to_compact.range_delete_tombstone_vector)
    {
        bool merged = false;

        for (auto &merged_tombstone : merged_tombstones)
        {
            //check if tombstones overlap
            if (current_tombstone.first <= merged_tombstone.second +1 &&
                current_tombstone.second >= merged_tombstone.first -1)
            {
                //merge overlapping tombstones
                merged_tombstone.first = min(merged_tombstone.first, current_tombstone.first);
                merged_tombstone.second = max(merged_tombstone.second, current_tombstone.second);
                merged = true;
                break;
            }
        }

        if (!merged)
        {
            merged_tombstones.push_back(current_tombstone);
        }
    }

    entries_to_compact.range_delete_tombstone_vector = merged_tombstones;
} 

void insert(EntryList& entrylist){
    entrylist.add_entry(KV {{4,1},"A"});
    entrylist.add_entry(KV {{2,0},"A"});
    entrylist.add_entry(KV {{5,1},"A"});
   
    entrylist.add_range_delete_tombstone(pll {1,3});
    entrylist.add_range_delete_tombstone(pll {2,4});
    entrylist.add_range_delete_tombstone(pll {1,6});
    std::cout << "insert done" << std::endl;
}
void sortAndWrite_RD(EntryList& entries_to_compact, int level_to_flush_in){
    std::cout << "sortAndWrite done" << std::endl;
}

void setupTestEnvironment(EntryList &entries_to_compact)
{
    // Initialize entries_to_compact.range_delete_tombstone_vector with some initial sorted range delete tombstones
    entries_to_compact.add_range_delete_tombstone(pll {9,10});
    entries_to_compact.add_range_delete_tombstone(pll {11,15});
    entries_to_compact.add_range_delete_tombstone(pll {1,5});
    entries_to_compact.add_range_delete_tombstone(pll {1,3});
    entries_to_compact.add_range_delete_tombstone(pll {2,4});
    entries_to_compact.add_range_delete_tombstone(pll {1,5});

    
}

void printRangeDeleteTombstones(const std::vector<pll> &range_delete_tombstone_vector)
{
    std::cout << "Range delete tombstones:" << std::endl;
    for (const auto &tombstone : range_delete_tombstone_vector)
    {
        std::cout << "[" << tombstone.first << ", " << tombstone.second << "]" << std::endl;
    }
}