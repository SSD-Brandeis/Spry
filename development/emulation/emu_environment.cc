#include <iostream>
#include <cmath>
#include <sys/time.h>
#include "emu_environment.h"

 
/*Set up the singleton object with the experiment wide options*/
EmuEnv* EmuEnv::instance = 0;


EmuEnv::EmuEnv() 
{
  size_ratio = 2;

  buffer_size_in_pages = 128;
  entries_per_page = 128;
  entry_size = 128; // in Bytes 
  Pointentries_per_page = entries_per_page;
  buffer_size = buffer_size_in_pages * Pointentries_per_page * entry_size; // M = P*B*E = 128 * 128 * 128 B = 2 MB 

  per_kv_size = 128;
  per_range_delete_size = 16;

  delete_tile_size_in_pages = 2; // h
  file_size = buffer_size; // file_size = M/s = 2 MB / 8 = 256 KB   read by Compact and Flush
  page_size = Pointentries_per_page * per_kv_size; // read by PopulateFile
  delete_tile_size = delete_tile_size_in_pages * page_size;

  buffer_flush_threshold = disk_run_flush_threshold = 1;

  num_inserts = 0;

  verbosity = 0;
  lethe_new = 0;  // 0 for classical lethe, 1 for optimal Kiwi, 2 for Kiwi++
  srd_count = 1;
  epq_count = 1;
  pq_count = 1;
  srq_count = 1;
  level_count = 1;

  rd_count = 1;
  selectivity = 0.001;
  workloadFilename = "workload.txt";
  insertBeforeRangeDelete = 0.5;
}
int EmuEnv::getDeleteTileSize(int level)
{
  if (lethe_new == 2)
  {
    return variable_delete_tile_size_in_pages[level];
  }
  else
  {
    return delete_tile_size_in_pages;
  }
}

EmuEnv* EmuEnv::getInstance()
{
  if (instance == 0)
    instance = new EmuEnv();

  return instance;
}
void EmuEnv::read_variables(){
  per_kv_size = entry_size;
  per_range_delete_size = 2*4; // 2 long

  Pointentries_per_page = entries_per_page;
  buffer_size = buffer_size_in_pages * Pointentries_per_page * entry_size;

  file_size = buffer_size; // file_size = M/s = 2 MB / 8 = 256 KB   read by Compact and Flush
  page_size = Pointentries_per_page * per_kv_size; // read by PopulateFile
  delete_tile_size = delete_tile_size_in_pages * page_size;
}