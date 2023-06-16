/*
 *  Created on: May 14, 2019
 *  Author: Subhadeep, Papon
 */


#ifndef Memory_Buffer_H_
#define Memory_Buffer_H_

namespace tree_builder {
  class MemoryBuffer;
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

namespace tree_builder {

  class MemoryBuffer {
    private:
    MemoryBuffer(EmuEnv* _env);
    static MemoryBuffer* buffer_instance;
    static long max_buffer_size;
    static EntryList* buffer;

    public:
    static float buffer_flush_threshold; 
    static long current_buffer_entry_count;
    static long current_buffer_point_entry_count;
    static long current_buffer_range_entry_count;
    static long current_buffer_size;
    static float current_buffer_saturation;
    static int buffer_flush_count;
    static int verbosity;

    static MemoryBuffer* getBufferInstance(EmuEnv* _env);
    static int getCurrentBufferStatistics();
    static int setCurrentBufferStatistics(int point_entry_count_change, int range_entry_count_change, int buffer_flush_threshold);
    static int initiateBufferFlush(int level_to_flush_in);     
    static int printBufferEntries();

    static EntryList& getBuffer();


    
  };


} // namespace

#endif /* Memory_Buffer_H_ */