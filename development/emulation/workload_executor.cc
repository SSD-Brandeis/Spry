/*
 *  Created on: May 14, 2019
 *  Author: Subhadeep, Papon
 */

#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <iomanip>

#include "emu_environment.h"
#include "tree_builder/tree_builder.h"
#include "tree_builder/entry.h"
#include "workload_executor.h"



using namespace std;
using namespace tree_builder;
using namespace workload_exec;

long WorkloadExecutor::buffer_insert_count = 0;
long WorkloadExecutor::buffer_update_count = 0;
long WorkloadExecutor::total_insert_count = 0;
long WorkloadExecutor::total_buffer_range_tombstone_deleted_point_entry_count = 0;
uint32_t WorkloadExecutor::counter = 0;

long WorkloadExecutor::total_range_tombstone_count = 0;
long WorkloadExecutor::buffer_range_tombstone_count = 0;
long WorkloadExecutor::total_merge_deleted_entry_count = 0;
long WorkloadExecutor::total_merge_deleted_point_entry_count = 0;
long WorkloadExecutor::total_merge_deleted_range_entry_count = 0;
long WorkloadExecutor::total_range_deleted_point_entry_count = 0;

long WorkloadExecutor::total_file_boundary_split_range_entry_count;


inline void showProgress(const uint32_t &workload_size, const uint32_t &counter) {
  
    if (counter / (workload_size/100) >= 1) {
      for (int i = 0; i<104; i++){
        std::cout << "\b";
        fflush(stdout);
      }
    }
    for (int i = 0; i<counter / (workload_size/100); i++){
      std::cout << "=" ;
      fflush(stdout);
    }
    std::cout << std::setfill(' ') << std::setw(101 - counter / (workload_size/100));
    std::cout << counter*100/workload_size << "%";
      fflush(stdout);

  if (counter == workload_size) {
    std::cout << "\n";
    return;
  }
}

//This is used to sort the whole file based on sort key
bool Utility::sortbysortkey(const pair<pair<long, long>, string> &a, const pair<pair<long, long>, string> &b)
{
  return (a.first.first < b.first.first);
}
//This is used to sort the whole file based on delete key
bool Utility::sortbydeletekey(const pair<pair<long, long>, string> &a, const pair<pair<long, long>, string> &b)
{
  return (a.first.second < b.first.second);
}

int Utility::minInt(int a, int b)
{
  if (a <= b)
    return a;
  else
  {
    return b;
  }
}


// //TODO: sortAndWriteForRDF()  --> sortAndWriteFactory() ???  
// void Utility::sortAndWrite(vector < pair < pair < long, long >, string > > vector_to_compact, int level_to_flush_in)
// {
//   EmuEnv *_env = EmuEnv::getInstance();
//   SSTFile *head_level_1 = DiskMetaFile::getSSTFileHead(level_to_flush_in);
//   int entries_per_file = _env->entries_per_page * _env->buffer_size_in_pages;

//   std::sort(vector_to_compact.begin(), vector_to_compact.end(), Utility::sortbysortkey);
//   long startval =  vector_to_compact[0].first.first;
//   long endval =  vector_to_compact[vector_to_compact.size()-1].first.first;

//   //level not exists, compact to a new level
//   if (!head_level_1)
//   {
//     if (MemoryBuffer::verbosity == 2)
//       std::cout << "NULL" << std::endl;

//     if (vector_to_compact.size() % _env->getDeleteTileSize(level_to_flush_in) != 0 && vector_to_compact.size() / _env->getDeleteTileSize(level_to_flush_in) < 1)
//     {
//       std::cout << " ERROR " << std::endl;
//       exit(1);
//     }
//     else
//     {
// std::cout << "Tracking pre compactAndFlush @ " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;
//       compactAndFlush(vector_to_compact, level_to_flush_in);
// std::cout << "Tracking post compactAndFlush @ " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;

//     }
//   }
//   // level exists, gather all the different entries from overlapped next leval them flush it down
//   // where did all those files go after entries are gathered??
//   else
//   {
//     if (MemoryBuffer::verbosity == 2)
//       std::cout << "head not null anymore" << std::endl;
//     SSTFile *head_level_1 = DiskMetaFile::getSSTFileHead(level_to_flush_in);
//     SSTFile *moving_head = head_level_1;
//     int match = 0;

//     if (MemoryBuffer::verbosity == 2)
//       std::cout << "Vector size before merging : " << vector_to_compact.size() << std::endl;
    
//     while (moving_head)
//     {
//       //find the current level file that overlaps with the file from the previous level 
//       if (moving_head->getMinSortKey() >= endval || moving_head->getMaxSortKey() <= startval )
//       {
//       // moving_head = moving_head->next_file_ptr;
//       moving_head = moving_head->getNextFilePtr();
//         continue;
//       }
//       //cout << "Performed Optimization :: Overlap FOUND" << endl;
      
//       // for (int k = 0; k < moving_head->tile_vector.size(); k++)
//       for (int k = 0; k < moving_head->getPointDeleteTileCount(); k++)
//       {
//         // DeleteTile delete_tile = moving_head->tile_vector[k];
//         DeleteTile delete_tile = moving_head->getIthPointDeleteTile(k);
//         // for (int l = 0; l < delete_tile.page_vector.size(); l++)
//         for (int l = 0; l < delete_tile.getPointPageVectorSize(); l++)
//         {
//           // Page page = delete_tile.page_vector[l];
//           Page page = delete_tile.getIthPointPage(l);
//           for (int m = 0; m < page.getPointEntryVectorSize(); m++)
//           {
//             for(int p = 0; p < vector_to_compact.size(); p++) {
//               // if (vector_to_compact[p].first.first == page.kv_vector[m].first.first) {
//               if (vector_to_compact[p].first.first == page.getIthPointEntry(m).getSortKey()) {
//                 match++;
//               }
//             }
//             if (match == 0){
//               PointEntry entry = page.getIthPointEntry(m);
//               vector_to_compact.push_back( make_pair( make_pair(entry.getSortKey(), entry.getDeleteKey()), entry.getValue() ) );
//             }
//             else {
//               match = 0;
//             }
//           }
//         }
//       }
//       // moving_head = moving_head->next_file_ptr;
//       moving_head = moving_head->getNextFilePtr();
//     }

//     if (MemoryBuffer::verbosity == 2)
//       std::cout << "Vector size after merging : " << vector_to_compact.size() << std::endl;

//     std::sort(vector_to_compact.begin(), vector_to_compact.end(), Utility::sortbysortkey);
    
//     if (MemoryBuffer::verbosity == 1)   //UNCOMMENT
//     {
//       std::cout << "\nprinting before compacting " << std::endl;
//       for (int j = 0; j < vector_to_compact.size(); ++j)
//       {
//         std::cout << "< " << vector_to_compact[j].first.first << ",  " << vector_to_compact[j].first.second << " >"
//                   << "\t" << std::endl;
//         if (j%8 == 7) cout << std::endl;
//       }
//     }
    
//     compactAndFlush(vector_to_compact, level_to_flush_in);
//   }
//   int saturation = DiskMetaFile::checkAndAdjustLevelSaturation(level_to_flush_in);
// }


int WorkloadExecutor::insert(long sortkey, long deletekey, string value, long timetag)
{
  EmuEnv* _env = EmuEnv::getInstance();
  bool found = false;
  //For UPDATES in inserts
  for (int i = 0; i < MemoryBuffer::getBuffer().get_point_entry_vector_size() ; ++i)
  {
    if (MemoryBuffer::getBuffer().get_ith_point_entry(i).getSortKey() == sortkey)
    {
      MemoryBuffer::setCurrentBufferStatistics(0, 0, (value.size() - MemoryBuffer::getBuffer().get_ith_point_entry(i).getValue().size()));
      MemoryBuffer::getBuffer().get_ith_point_entry(i).setValue(value);
      found = true;

      total_insert_count++;
      buffer_update_count++;
      break;
    }
  }

  //For INSERTS in inserts
  if (!found)
  {
    MemoryBuffer::setCurrentBufferStatistics(1, 0, (sizeof(sortkey) + sizeof(deletekey) + value.size()));
    MemoryBuffer::getBuffer().add_point_entry(PointEntry({(make_pair(make_pair(sortkey, deletekey), value)), timetag}));
    total_insert_count++;
    buffer_insert_count++;
  }

  checkBufferFlush();


  counter++;


  return 1;
}

void WorkloadExecutor::checkBufferFlush(){
  if (MemoryBuffer::current_buffer_saturation >= MemoryBuffer::buffer_flush_threshold)
  {
    if(MemoryBuffer::verbosity == 2){
      std::cout << "Buffer full :: Sorting buffer " ;
    }


    if (MemoryBuffer::verbosity == 1)
    {
      std::cout << ":::: Buffer full :: Flushing buffer to Level 1 " << std::endl;
      MemoryBuffer::printBufferEntries();
    }

    int status = MemoryBuffer::initiateBufferFlush(1);
    if (status)
    {
      if (MemoryBuffer::verbosity == 2)
      MemoryBuffer::getBuffer().clear();
      if (MemoryBuffer::verbosity == 2){
        std::cout << ":::: Buffer point resized ( size = " << MemoryBuffer::getBuffer().get_point_entry_vector_size() << " ) " << std::endl;
        std::cout << ":::: Buffer range resized ( size = " << MemoryBuffer::getBuffer().get_range_entry_vector_size() << " ) " << std::endl;
      }
      MemoryBuffer::current_buffer_entry_count = 0;
      MemoryBuffer::current_buffer_point_entry_count = 0;
      MemoryBuffer::current_buffer_range_entry_count = 0;
      MemoryBuffer::current_buffer_saturation = 0;
      MemoryBuffer::current_buffer_size = 0;
    }
  }
}


int WorkloadExecutor::rangeDeleteOnPrimaryKey(long startKey, long endKey, long timetag){
  DiskMetaFile::addRangeDeleteToSkylineRangeDeleteFilter(startKey, endKey, timetag);

  MemoryBuffer::setCurrentBufferStatistics(0, 1, (sizeof(startKey) + sizeof(endKey)));
  MemoryBuffer::getBuffer().add_range_entry(RangeEntry({make_pair(startKey, endKey), timetag}));
  total_range_tombstone_count++;
  buffer_range_tombstone_count++;
  
  MemoryBuffer::getBuffer().remove_point_entries_in_range(startKey, endKey);

  checkBufferFlush();
  counter++;

  return 1;
}



int WorkloadExecutor::search(long key, int possible_level_of_occurrence)
{
  SSTFile *level_head_reference = DiskMetaFile::getSSTFileHead(possible_level_of_occurrence);
  // while (level_head_reference != NULL) {
  //   for(int i = 0; i<level_head_reference->file_instance.size(); ++i) {
  //     if(level_head_reference->file_instance[i].first == key) {
  //       //std::cout << "Match found for key " << key << std::endl;
  //       return 1;
  //     }
  //   }
  //   level_head_reference = level_head_reference->next_file_ptr;
  // }
  return 0;
}


int WorkloadExecutor::getWorkloadStatictics(EmuEnv *_env)
{
  std::cout << "************* PRINTING WORKLOAD STATISTICS *************" << std::endl;
  std::cout << "Total inserts in buffer = " << total_insert_count << " (unique inserts = " << buffer_insert_count << "; in-place updates = " << buffer_update_count << ")" << std::endl;

  std::cout << "Insert stats: ";
  int total_compactions = 0;
  long total_files_in_compcations = 0;
  for (int i = 0; i < 32; i++)
  {
    total_compactions += DiskMetaFile::compaction_through_sortmerge_counter[i];
    total_files_in_compcations += DiskMetaFile::compaction_file_counter[i];
  }
  //int disk_pages_per_file = _env->buffer_size_in_pages/_env->buffer_split_factor;
  int disk_pages_per_file = _env->buffer_size_in_pages; //Doubt
  std::cout << "#compactions = " << total_compactions << ", #files_in_compactions = " << total_files_in_compcations << ", #IOs = " << 2 * total_files_in_compcations * disk_pages_per_file
            << " (#IOs does not include IOs for only writing or pointer manipulation)" << std::endl;

  std::cout << "Disk space amplification = ";
  SSTFile *level_1_head = DiskMetaFile::getSSTFileHead(1);
  long total_entries_in_L1 = 0;
  long duplicate_key_count = 0;

  float space_amplification = (float)duplicate_key_count / total_entries_in_L1;
  std::cout << space_amplification << " (entries in L1 = " << total_entries_in_L1 << " / duplicated entries = " << duplicate_key_count << ")" << std::endl;

  std::cout << "********************************************************" << std::endl;

  return 1;
}


void Utility::compactAndFlush_RDF(EntryList entrylist_to_compact, int level_to_flush_in){
    if(entrylist_to_compact.empty()){
      cout << "Empty entrylist to compact " <<  __FILE__ << ":" << __LINE__ << endl;
      return;
    }

    EmuEnv *_env = EmuEnv::getInstance();

    SSTFile *head_level_1 = DiskMetaFile::getSSTFileHead(level_to_flush_in);
    SSTFile *moving_ptr;

    //if current level is empty, then create new files and populate them
    if(!head_level_1){
      
      //write head file of the level
      EntryList entries_to_populate_file = entrylist_to_compact.split_out_by_size_inByte(_env->file_size); 

      int point_deletetile_count = ceil((entries_to_populate_file.get_point_entry_vector_size_inByte() * 1.0)/(_env->delete_tile_size * 1.0)); 
      int range_deletetile_count = ceil((entries_to_populate_file.get_range_entry_vector_size_inByte() * 1.0)/(_env->delete_tile_size * 1.0));
      
      SSTFile *new_file = SSTFile::createNewSSTFile_RDF(level_to_flush_in, point_deletetile_count, range_deletetile_count);
      head_level_1 = new_file;

      DiskMetaFile::setSSTFileHead(head_level_1, level_to_flush_in);
      int status = SSTFile::PopulateFile_RDF(new_file, entries_to_populate_file, level_to_flush_in);
      
      moving_ptr = head_level_1; // set the moving ptr to the front 
      while(!entrylist_to_compact.empty()){
        EntryList entries_to_populate_file = entrylist_to_compact.split_out_by_size_inByte(_env->file_size); 

        int point_deletetile_count = ceil((entries_to_populate_file.get_point_entry_vector_size_inByte() * 1.0)/(_env->delete_tile_size * 1.0)); 
        int range_deletetile_count = ceil((entries_to_populate_file.get_range_entry_vector_size_inByte() * 1.0)/(_env->delete_tile_size * 1.0));
        
        SSTFile *new_file = SSTFile::createNewSSTFile_RDF(level_to_flush_in, point_deletetile_count, range_deletetile_count);
        
        int status = SSTFile::PopulateFile_RDF(new_file, entries_to_populate_file, level_to_flush_in);
        moving_ptr->setNextFilePtr(new_file);
        moving_ptr = moving_ptr->getNextFilePtr();
      }
      moving_ptr->setNextFilePtr(NULL); //just make sure the end is NULL
    }else{   // there are already some files in the level
      moving_ptr = head_level_1; // set pointer to the head 
      SSTFile *moving_head = head_level_1, *moving_end = head_level_1;
      while( (moving_end != NULL) && (moving_end->getSSTMinKey() <= entrylist_to_compact.get_max_key()) ){
        if(moving_end->getSSTMaxKey() < entrylist_to_compact.get_min_key()){
          moving_head = moving_end;
        }
        moving_end = moving_end->getNextFilePtr();
      }


      SSTFile *tmp_head_ptr, *ptr;
      EntryList entries_to_populate_file = entrylist_to_compact.split_out_by_size_inByte(_env->file_size); 


      int point_deletetile_count = ceil((entries_to_populate_file.get_point_entry_vector_size_inByte() * 1.0)/(_env->delete_tile_size * 1.0)); 
      int range_deletetile_count = ceil((entries_to_populate_file.get_range_entry_vector_size_inByte() * 1.0)/(_env->delete_tile_size * 1.0));
      
      SSTFile *new_file = SSTFile::createNewSSTFile_RDF(level_to_flush_in, point_deletetile_count, range_deletetile_count);
      tmp_head_ptr = new_file;

      int status = SSTFile::PopulateFile_RDF(new_file, entries_to_populate_file, level_to_flush_in);
      
      ptr = tmp_head_ptr; // set the moving ptr to the front 
      while(!entrylist_to_compact.empty()){
        EntryList entries_to_populate_file = entrylist_to_compact.split_out_by_size_inByte(_env->file_size); 



        int point_deletetile_count = ceil((entries_to_populate_file.get_point_entry_vector_size_inByte() * 1.0)/(_env->delete_tile_size * 1.0)); 
        int range_deletetile_count = ceil((entries_to_populate_file.get_range_entry_vector_size_inByte() * 1.0)/(_env->delete_tile_size * 1.0));
        
        SSTFile *new_file = SSTFile::createNewSSTFile_RDF(level_to_flush_in, point_deletetile_count, range_deletetile_count);
        
        int status = SSTFile::PopulateFile_RDF(new_file, entries_to_populate_file, level_to_flush_in);
        ptr->setNextFilePtr(new_file);
        ptr = ptr->getNextFilePtr();
      }
      ptr->setNextFilePtr(moving_end); //here connect back to the remaining nonverlaapigng files
    
      //if the new file is the new head, then set the level head to the new file
      SSTFile *delete_start_ptr;
      if(head_level_1->getSSTMaxKey() >= tmp_head_ptr->getSSTMinKey()){ 
        delete_start_ptr = moving_head;
        DiskMetaFile::setSSTFileHead(tmp_head_ptr, level_to_flush_in);
      }else{
        delete_start_ptr = moving_head->getNextFilePtr();
        moving_head->setNextFilePtr(tmp_head_ptr);
      }

      //memory leakage?   --> Delete the files in the range [delete_start_ptr, moving_end)
    }

}

// //
// void Utility::compactAndFlush_RDF(EntryList entrylist_to_compact, int level_to_flush_in){
//   EmuEnv *_env = EmuEnv::getInstance();

//   SSTFile *head_level_1 = DiskMetaFile::getSSTFileHead(level_to_flush_in);
//   SSTFile *moving_ptr;
//   SSTFile *moving_head = head_level_1;
//   SSTFile *moving_head_prev = head_level_1;   //Need this to overwrite current overlapping file
//   SSTFile *end_ptr = moving_head;

//   //if current level is empty, then create new files and populate them
//   if (!head_level_1)
//   { 
//     bool head_flag = 1; // 1 means we are cratting the haed of SST
//     while(!entrylist_to_compact.empty())
//     {
// std::cout << "Tracking C1 @ " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;

//       // split the Entrylist for 1 file size
//       EntryList entries_to_populate_file = entrylist_to_compact.split_out_by_size_inByte(_env->file_size); 
// std::cout << "entries_to_populate_file " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;
// entries_to_populate_file.print();
// std::cout << "entrylist_to_compact " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;
// entrylist_to_compact.print();

//       int point_deletetile_count = ceil((entries_to_populate_file.get_point_entry_vector_size_inByte() * 1.0)/(_env->delete_tile_size * 1.0)); 
//       int range_deletetile_count = ceil((entries_to_populate_file.get_range_entry_vector_size_inByte() * 1.0)/(_env->delete_tile_size * 1.0));
//       // create new SST file 
//       SSTFile *new_file = SSTFile::createNewSSTFile_RDF(level_to_flush_in, point_deletetile_count, range_deletetile_count);
//       if (head_flag) // asign head SST 
//       {
//         head_flag = 0; // change head flag
//         head_level_1 = new_file;
//         DiskMetaFile::setSSTFileHead(head_level_1, level_to_flush_in);
//         int status = SSTFile::PopulateFile_RDF(new_file, entries_to_populate_file, level_to_flush_in);
//         moving_ptr = head_level_1; // set the moving ptr to the front 
//       }
//       else
//       {
//         int status = SSTFile::PopulateFile_RDF(new_file, entries_to_populate_file, level_to_flush_in);
//         moving_ptr->setNextFilePtr(new_file);
//         moving_ptr = moving_ptr->getNextFilePtr();
//       }
//       entries_to_populate_file.clear();
//     }
// std::cout << "Tracking C2 @ " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;

//   }
//   // there is files in the level
//   else
//   {
// std::cout << "Tracking C3 @ " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;

//     moving_ptr = head_level_1; // set pointer to the head 
//     // Caluculate end pointer
//     while(moving_ptr){
//       std::cout<< "get SST Max KEY: "<< moving_ptr->getSSTMaxKey()<<endl;
//       std::cout<<"get SST Min key:"<<moving_ptr->getSSTMinKey()<<endl;
//       if(moving_ptr->getSSTMaxKey() <= entrylist_to_compact.get_min_key())
//       {
//         end_ptr = NULL;
//         moving_ptr = moving_ptr->getNextFilePtr();
//         continue;
//       }
//       end_ptr = moving_ptr;  // Need to assign this in case vector gets inserted in between with no overlapping
//       if (moving_ptr->getSSTMinKey() >= entrylist_to_compact.get_max_key())
//       {
//         break;  //safe to say we have calculated end pointer
//       }
//       end_ptr = moving_ptr->getNextFilePtr();
//       moving_ptr = moving_ptr->getNextFilePtr();
//     }

// std::cout << "Tracking C4 @ " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;
//     // After the code above end_ptr points to the SST file that is not ovelaping 
//     int flag = 0; //0 means overlapping
//                   //1 means vector to be appended at the begining or vector to be inserted in between two files (NO OVERLAPPING); 
//                   //2 means the current inserting (vector_to_compact) is not possible to be the head of the level;
//     bool hasHeadChanged = 0; 
//     moving_ptr = head_level_1; // set pointer to the head

//     while(moving_ptr)
//     {
//       if(moving_ptr->getSSTMaxKey() <= entrylist_to_compact.get_min_key()) {
//         flag = 2; //Not possible to be a head anymore
//         moving_head = moving_ptr; // moving head would point to the non overlaping SST
//         if(moving_ptr->getNextFilePtr()) {
//           moving_ptr = moving_ptr->getNextFilePtr();
//           continue;
//         }
//       }
//       if (moving_ptr->getSSTMinKey() >= entrylist_to_compact.get_max_key()) {
//         flag = 1; //vector to be appended very begining or vector to be inserted in between with no overlapping
//       }
//       bool first_iteration = 1; // 1 means we are in the first iteration;
//       // start inserting SSTs
// std::cout << "Tracking C4_1 @ " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;
//       while(!entrylist_to_compact.empty()) {
//         // split the Entrylist for 1 file size
//         EntryList entries_to_populate_file = entrylist_to_compact.split_out_by_size_inByte(_env->file_size);  
// std::cout << "entries_to_populate_file " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;
// entries_to_populate_file.print();
// std::cout << "entrylist_to_compact " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;
// entrylist_to_compact.print();

//         int point_deletetile_count = ceil((entries_to_populate_file.get_point_entry_vector_size_inByte() * 1.0)/(_env->delete_tile_size * 1.0)); 
//         int range_deletetile_count = ceil((entries_to_populate_file.get_range_entry_vector_size_inByte() * 1.0)/(_env->delete_tile_size * 1.0));

//         // create new SST file 
//         SSTFile *new_file = SSTFile::createNewSSTFile_RDF(level_to_flush_in, point_deletetile_count, range_deletetile_count);
//         int status = SSTFile::PopulateFile_RDF(new_file, entries_to_populate_file, level_to_flush_in); // POPULATE 
//         new_file->setNextFilePtr(end_ptr);  //set the next file of the new_file to the first non-overlapping file 
        
//         //if the data is going to be inserted at the begining of the level, then update the head of the level
//         if (moving_ptr == DiskMetaFile::getSSTFileHead(level_to_flush_in) && first_iteration && flag != 2) {
//           DiskMetaFile::setSSTFileHead(new_file, level_to_flush_in);
//           hasHeadChanged = 1;
//         }

//         //if not overlapping and won't inserted at the begining of the level, then insert the data in between two files
//         if (flag == 1 && hasHeadChanged == 0) {  //This means file to be inserted in between with no overlapping 
//           moving_head->setNextFilePtr(new_file);
//         }
        
//         //Either overlapping or is inserted at the begining of the level, 
//         if (flag != 1) {
//           moving_head->setNextFilePtr(new_file);
//           moving_head = new_file;
//         }

//         first_iteration = 0;
//         entries_to_populate_file.clear();
//       }
//       break;
//     }
// std::cout << "Tracking C5 @ " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;

//   }
// }


// // Jing Yi version
// void Utility::sortAndWrite_RD(EntryList entries_to_compact, int level_to_flush_in){

//     EmuEnv *_env = EmuEnv::getInstance();
//     SSTFile *head_level_1 = DiskMetaFile::getSSTFileHead(level_to_flush_in);
//     SSTFile *head_level_1_RD = DiskMetaFile::getSSTFileHead(level_to_flush_in);


//     sort(entries_to_compact.kv_vector.begin(), entries_to_compact.kv_vector.end(), [](const KV& a, const KV& b) {
//             return a.first.first < b.first.first;
//         });
//     sort(entries_to_compact.range_delete_tombstone_vector.begin(), entries_to_compact.range_delete_tombstone_vector.end(), [](const pll& a, const pll& b) {
//             return a.first < b.first;
//         });
//     //int i = entries_to_compact.get_entry_size();

//     long startval = entries_to_compact.kv_vector[0].first.first;
//     long endval = entries_to_compact.kv_vector[entries_to_compact.kv_vector.size() - 1].first.first;

//     long startval_RD = entries_to_compact.range_delete_tombstone_vector[0].first;
//     long endval_RD = entries_to_compact.range_delete_tombstone_vector[entries_to_compact.range_delete_tombstone_vector.size() - 1].first;


//       //level not exists, compact to a new level
//     if (!head_level_1)
//     {
//         if (MemoryBuffer::verbosity == 2)
//             std::cout << "NULL" << std::endl;

//         if (entries_to_compact.get_PointEntry_size() % _env->getDeleteTileSize(level_to_flush_in) != 0 && entries_to_compact.get_PointEntry_size() / _env->getDeleteTileSize(level_to_flush_in) < 1)
//         {
//             std::cout << " ERROR " << std::endl;
//             exit(1);
//         }
//         else
//         {
//             std::cout << "Tracking pre compactAndFlush_RD @ " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;
//             compactAndFlush_RDF(entries_to_compact, level_to_flush_in);
//             std::cout << "Tracking post compactAndFlush_RD @ " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;

//         }
//     }
//     else
//     {
//         if (MemoryBuffer::verbosity == 2)
//             std::cout << "head not null anymore" << std::endl;
//         SSTFile *head_level_1 = DiskMetaFile::getSSTFileHead(level_to_flush_in);
//         SSTFile *moving_head = head_level_1;
//         int match = 0;

//         if (MemoryBuffer::verbosity == 2)
//             std::cout << "EntryList size before merging : " << entries_to_compact.get_PointEntry_size() << std::endl;
    
//         while (moving_head)
//         {
//     // find the current level file that overlaps with the file from the previous level 
//             if (moving_head->getMinSortKey() >= endval || moving_head->getMaxSortKey() <= startval)
//             {
//                 moving_head = moving_head->getNextFilePtr();
//                 continue;
//             }

//     // Iterate through the DeleteTiles, Pages, and PointEntries to merge overlapping entries
//             for (int k = 0; k < moving_head->getPointDeleteTileCount(); k++)
//             {
//                 DeleteTile delete_tile = moving_head->getIthPointDeleteTile(k);
//                 for (int l = 0; l < delete_tile.getPointPageVectorSize(); l++)
//                 {
//                     Page page = delete_tile.getIthPointPage(l);
//                     for (int m = 0; m < page.getPointEntryVectorSize(); m++)
//                     {
//                         for(int p = 0; p < entries_to_compact.kv_vector.size(); p++) {
//               // if (vector_to_compact[p].first.first == page.kv_vector[m].first.first) {
//                             if (entries_to_compact.kv_vector[p].first.first == page.getIthPointEntry(m).getSortKey()) {
//                             match++;
//                             }
//                         }
//                         if (match == 0){
//                             PointEntry entry = page.getIthPointEntry(m);
//                             entries_to_compact.kv_vector.push_back( make_pair( make_pair(entry.getSortKey(), entry.getDeleteKey()), entry.getValue() ) );
//                         }
//                         else {
//                             match = 0;
//                         }
//                     }
//                 }   
//             }
//             moving_head = moving_head->getNextFilePtr();
//         }

//         SSTFile *moving_head_RD = head_level_1_RD;
//         while (moving_head_RD)
//         {
//     // Find the current level file that overlaps with the file from the previous level
//             if (moving_head_RD->getMinSortKey() >= endval_RD || moving_head_RD->getMaxSortKey() <= startval_RD )
//             {
//                 moving_head_RD = moving_head_RD->getNextFilePtr();
//                 continue;
//             }

//     // Iterate through the DeleteTiles, Pages, and RangeDeleteEntries to merge overlapping tombstones
//             for (int k = 0; k < moving_head_RD->getRangeDeleteTileCount(); k++)
//             {
//                 DeleteTile range_delete_tile = moving_head_RD->getIthRangeDeleteTile(k);
//                 for (int l = 0; l < range_delete_tile.getRangePageVectorSize(); l++)
//                 {
//                     Page page = range_delete_tile.getIthRangePage(l);
//                     for (int m = 0; m < page.getRangeEntryVectorSize(); m++)
//                     {
//                         pll current_tombstone = make_pair(page.getIthRangeEntry(m).getStartKey(), page.getIthRangeEntry(m).getEndKey());
//                         bool merged = false;

//                         for (int p = 0; p < entries_to_compact.range_delete_tombstone_vector.size(); p++)
//                         {
//                     // Check if tombstones overlap
//                             if (current_tombstone.first <= entries_to_compact.range_delete_tombstone_vector[p].second +1 &&
//                                 current_tombstone.second >= entries_to_compact.range_delete_tombstone_vector[p].first -1)
//                             {
//                         // Merge overlapping tombstones
//                                 entries_to_compact.range_delete_tombstone_vector[p].first = min(entries_to_compact.range_delete_tombstone_vector[p].first, current_tombstone.first);
//                                 entries_to_compact.range_delete_tombstone_vector[p].second = max(entries_to_compact.range_delete_tombstone_vector[p].second, current_tombstone.second);
//                                 merged = true;
//                                 break;
//                             }
//                         }

//                 // If tombstone is not merged, add it to the vector
//                         if (!merged)
//                         {
//                             entries_to_compact.range_delete_tombstone_vector.push_back(current_tombstone);
//                         }
//                     }
//                 }
//             }
//             moving_head_RD = moving_head_RD->getNextFilePtr();
//         }


//         sort(entries_to_compact.kv_vector.begin(), entries_to_compact.kv_vector.end(), [](const KV& a, const KV& b) {
//             return a.first.first < b.first.first;
//         });
//         sort(entries_to_compact.range_delete_tombstone_vector.begin(), entries_to_compact.range_delete_tombstone_vector.end(), [](const pll& a, const pll& b) {
//             return a.first < b.first;
//         });


//         Utility::compactAndFlush_RDF(entries_to_compact, level_to_flush_in);

//     }



//     int saturation = DiskMetaFile::checkAndAdjustLevelSaturation(level_to_flush_in);
//     std::cout << "sortAndWrite done" << std::endl;
// }

// Han modified by YC-Huang
void Utility::sortAndWrite_RDF(EntryList entries_to_compact, int level_to_flush_in){
  EmuEnv *_env = EmuEnv::getInstance();
  SSTFile *head_level_1 = DiskMetaFile::getSSTFileHead(level_to_flush_in);
  int entries_per_file = _env->entries_per_page * _env->buffer_size_in_pages;


  entries_to_compact.sortPointEntryVectorOnSortKseyInAscendingOrder(); // sort
  entries_to_compact.sortRangeEntryVectorOnStartKeyInAscendingOrder();   // sort
  if(level_to_flush_in == 1){ //only for flush from buffer to level 1
    entries_to_compact.merge_its_own_range_entry(); //overlapping -> non-overlapping

    vector<RangeEntry> range_tombstone_list = entries_to_compact.get_range_entry_vector();
  }


  if(level_to_flush_in > 1){
    vector<RangeEntry> range_tombstone_from_upper_level_list = entries_to_compact.get_range_entry_vector();
    DiskMetaFile::removeRangeDeleteFromRangeDeleteFilterAtLevel(level_to_flush_in-1, range_tombstone_from_upper_level_list);
    DiskMetaFile::removeRangeDeleteFromSplitRangeDeleteFilterAtLevel(level_to_flush_in-1, range_tombstone_from_upper_level_list);
  }

  long start_value = entries_to_compact.get_min_key();
  long end_value = entries_to_compact.get_max_key();


  //level not exists, compact to a new level
  if (!head_level_1)
  {
    if (entries_to_compact.get_PointEntry_size() % _env->getDeleteTileSize(level_to_flush_in) != 0 && entries_to_compact.get_PointEntry_size() / _env->getDeleteTileSize(level_to_flush_in) < 1)
    {
      std::cout << " ERROR:  wrong size relation between point_entry, delete_tile " << std::endl;
      exit(1);
    }
    else
    {
      vector<RangeEntry> range_tombstone_list = entries_to_compact.get_range_entry_vector();
      DiskMetaFile::addRangeDeleteToRangeDeleteFilterAtLevel(level_to_flush_in, range_tombstone_list); 
      DiskMetaFile::addRangeDeleteToSplitRangeDeleteFilterAtLevel(level_to_flush_in, range_tombstone_list); 

      DiskMetaFile::splitRangesInSplitRangeDeleteFilterAtLevelByEntryList(level_to_flush_in, entries_to_compact);

      entries_to_compact.sortRangeEntryVectorOnEndKeyInAscendingOrder();   // sort
      compactAndFlush_RDF(entries_to_compact, level_to_flush_in);

    }
  }
  // level exists, gather all the different entries from overlapped next leval them flush it down
  else{

    EntryList extracted_entryList = entries_to_compact.get_minmax_range_overlapped_entries_from_files(head_level_1);
    vector<PointEntry> point_entires_extracted = extracted_entryList.get_point_entry_vector();
    point_entires_extracted = entries_to_compact.apply_range_delete_on_point_entry_vector(point_entires_extracted);
    entries_to_compact.merge_with_another_point_entry_vector(point_entires_extracted); //repetive -> non-repetive  (key)

    vector<RangeEntry> range_entries_extracted = extracted_entryList.get_range_entry_vector();
    entries_to_compact.merge_with_another_range_entry_vector(range_entries_extracted); //overlapping -> non-overlapping (key)
    vector<RangeEntry> range_tombstone_list = entries_to_compact.get_range_entry_vector();
    DiskMetaFile::addRangeDeleteToRangeDeleteFilterAtLevel(level_to_flush_in, range_tombstone_list); 
    DiskMetaFile::addRangeDeleteToSplitRangeDeleteFilterAtLevel(level_to_flush_in, range_tombstone_list);
    DiskMetaFile::splitRangesInSplitRangeDeleteFilterAtLevelByEntryList(level_to_flush_in, entries_to_compact);
    DiskMetaFile::splitRangesInSplitRangeDeleteFilterAtLevelByEntryList(level_to_flush_in, entries_to_compact);
    entries_to_compact.sortRangeEntryVectorOnEndKeyInAscendingOrder();   // sort
    compactAndFlush_RDF(entries_to_compact, level_to_flush_in);
  }

  int saturation = DiskMetaFile::checkAndAdjustLevelSaturation_RDF(level_to_flush_in);

}







// //TODO: compactAndFlushForRDF()  --> compactAndFlushFactory() ???  
// void Utility::compactAndFlush(vector < pair < pair < long, long >, string > > vector_to_compact, int level_to_flush_in)
// {
//   EmuEnv *_env = EmuEnv::getInstance();
  
//   int entries_per_file = _env->entries_per_page * _env->buffer_size_in_pages;
//   int file_count = ceil(vector_to_compact.size() / (entries_per_file*1.0));
//   if (MemoryBuffer::verbosity == 2)
//     std::cout << "\nwriting " << file_count << " file(s)\n";

//   SSTFile *head_level_1 = DiskMetaFile::getSSTFileHead(level_to_flush_in);
//   SSTFile *moving_head = head_level_1;
//   SSTFile *moving_head_prev = head_level_1;   //Need this to overwrite current overlapping file
//   SSTFile *end_ptr = moving_head;

//   //if current level is empty, then create new files and populate them
//   if (!head_level_1)
//   {
//     for (int i = 0; i < file_count; i++)
//     {
//       vector<pair<pair<long, long>, string>> vector_to_populate_file;
//       entries_per_file = Utility::minInt(entries_per_file, vector_to_compact.size());
//       for (int j = 0; j < entries_per_file; ++j)
//       {
//         vector_to_populate_file.push_back(vector_to_compact[j]);
//       }
//       vector_to_compact.erase(vector_to_compact.begin(), vector_to_compact.begin() + entries_per_file);
//       SSTFile *new_file = SSTFile::createNewSSTFile(level_to_flush_in);
//       SSTFile *moving_head = head_level_1;
//       if (i == 0)
//       {
//         head_level_1 = new_file;
//         DiskMetaFile::setSSTFileHead(head_level_1, level_to_flush_in);
//         int status = SSTFile::PopulateFile(new_file, vector_to_populate_file, level_to_flush_in);
//       }
//       else
//       {
//         // while (moving_head->next_file_ptr)
//         while (moving_head->getNextFilePtr())
//         {
//           //moving_head = moving_head->next_file_ptr;
//           moving_head = moving_head->getNextFilePtr();
//         }
//         int status = SSTFile::PopulateFile(new_file, vector_to_populate_file, level_to_flush_in);
//         // moving_head->next_file_ptr = new_file;
//         moving_head->setNextFilePtr(new_file);
//       }
//       vector_to_populate_file.clear();
//     }
//   }  
//   // if current level is not empty, then check for overlapping and insert accordingly
//   else
//   {
//     //Calculating end pointer
//     while (moving_head)
//     {
//       if (moving_head->getMaxSortKey() <= vector_to_compact[0].first.first)
//       {
//         end_ptr = NULL;
//         // moving_head = moving_head->next_file_ptr;
//         moving_head = moving_head->getNextFilePtr();
//         continue;
//       }
//       end_ptr = moving_head;  // Need to assign this in case vector gets inserted in between with no overlapping
//       if (moving_head->getMinSortKey() >= vector_to_compact[vector_to_compact.size() - 1].first.first )
//       {
//         //safe to say we have calculated end pointer
//         break;
//       }
//       //end_ptr = moving_head->next_file_ptr;      
//       end_ptr = moving_head->getNextFilePtr();
//       // moving_head = moving_head->next_file_ptr;
//       moving_head = moving_head->getNextFilePtr();
//     }

//     if (MemoryBuffer::verbosity == 2)
//       cout << "Calculated end pointer" << endl;
    
//     moving_head = head_level_1;
//     int flag = 0; //0 means overlapping
//                   //1 means vector to be appended at the begining or vector to be inserted in between two files (NO OVERLAPPING); 
//                   //2 means the current inserting (vector_to_compact) is not possible to be the head of the level; 
                  
//     int hasHeadChanged = 0;  //head?

//     while (moving_head)
//     {
//       if (moving_head->getMaxSortKey() <= vector_to_compact[0].first.first)
//       {
//         moving_head_prev = moving_head; 
//         flag = 2; //Not possible to be a head anymore
//         // if (moving_head->next_file_ptr)
//         if (moving_head->getNextFilePtr())
//         {
//           // moving_head = moving_head->next_file_ptr;
//           moving_head = moving_head->getNextFilePtr();
//           continue;
//         }
//       }
//       //cout << "Prev max: " << moving_head_prev->max_sort_key << endl;
//       //cout << "Current max: " << moving_head->max_sort_key << endl;
//       // if (end_ptr != NULL)
//       //   cout << "End max: " << end_ptr->max_sort_key << endl;
//       if (moving_head->getMinSortKey() >= vector_to_compact[vector_to_compact.size()-1].first.first)
//       {
//         flag = 1; //vector to be appended very begining or vector to be inserted in between with no overlapping
//       }
//       //std::cout << "\nFile Count: " << file_count << std::endl; 

//       //begin: continuously inserting n files
//       for (int i = 0; i < file_count; i++)
//       {
//         vector<pair<pair<long, long>, string>> vector_to_populate_file;
//         entries_per_file = Utility::minInt(entries_per_file, vector_to_compact.size());
//         for (int j = 0; j < entries_per_file; ++j)
//         {
//           vector_to_populate_file.push_back(vector_to_compact[j]);
//         }

//         if (MemoryBuffer::verbosity == 2)
//         {
//           std::cout << "\nprinting before trimming " << std::endl;
//           for (int j = 0; j < vector_to_compact.size(); ++j)
//             std::cout << "< " << vector_to_compact[j].first.first << ",  " << vector_to_compact[j].first.second << " >"
//                       << "\t" << std::endl;
//         }
//         vector_to_compact.erase(vector_to_compact.begin(), vector_to_compact.begin() + entries_per_file);

//         if (MemoryBuffer::verbosity == 2)
//         {
//           std::cout << "\nprinting after trimming " << std::endl;
//           for (int j = 0; j < vector_to_compact.size(); ++j)
//             std::cout << "< " << vector_to_compact[j].first.first << ",  " << vector_to_compact[j].first.second << " >"
//                       << "\t";
//         }
//         if (MemoryBuffer::verbosity == 2)
//         {
//           std::cout << "\npopulating file " << head_level_1 << std::endl;
//           std::cout << "Vector to populate file: " << endl;
//           for (int j = 0; j < vector_to_populate_file.size(); ++j)
//             std::cout << "< " << vector_to_populate_file[j].first.first << ",  " << vector_to_populate_file[j].first.second << " >"
//                       << "\t";
//         }                          

//         SSTFile *new_file = SSTFile::createNewSSTFile(level_to_flush_in);
//         int status = SSTFile::PopulateFile(new_file, vector_to_populate_file, level_to_flush_in);
//         // new_file->next_file_ptr = end_ptr;  //set the next file of the new_file to the first non-overlapping file 
//         //                                     //after the overlapping area
//         new_file->setNextFilePtr(end_ptr);  //set the next file of the new_file to the first non-overlapping file 
//                                             //after the overlapping area
//         //SSTFile *moving_head = head_level_1;

//         //if the data is going to be inserted at the begining of the level, then update the head of the level
//         if (moving_head == DiskMetaFile::getSSTFileHead(level_to_flush_in) && i==0 && flag!=2)    //Further Optimize?
//         {
//           DiskMetaFile::setSSTFileHead(new_file, level_to_flush_in);
//           hasHeadChanged = 1;
//         }

//         //if not overlapping and won't inserted at the begining of the level, then insert the data in between two files
//         if (flag == 1 && hasHeadChanged == 0)   //This means file to be inserted in between with no overlapping
//         {
//           // moving_head_prev->next_file_ptr = new_file;
//           moving_head_prev->setNextFilePtr(new_file);
//         }

//         //Either overlapping or is inserted at the begining of the level, 
//         if (flag != 1)
//         {
//           // moving_head_prev->next_file_ptr = new_file;
//           moving_head_prev->setNextFilePtr(new_file);
//           moving_head_prev = new_file;
//         }
        
//         vector_to_populate_file.clear();
//       }
//       break;
//     }
//     //fin: continuous inserting n files

//   }
// }


int WorkloadExecutor::getExecutionStatistics()
{
  std::cout << "********** PRINTING EXECUTED WORKLOAD STATISTICS **********" << std::endl;
  std::cout << "Current total merge deleted entry count (@ compact) = " << total_merge_deleted_entry_count << std::endl;
  std::cout << "Current total merge deleted point entry count (@ compact) = " << total_merge_deleted_point_entry_count << std::endl;
  std::cout << "Current total merge deleted range entry count (@ compact) = " << total_merge_deleted_range_entry_count << std::endl;
  std::cout << "Current total range deleted point entry count (@ compact) = " << total_range_deleted_point_entry_count << std::endl;
  std::cout << "Current total increased range entry count caused by splitting at file bounday  (@ compact) = " << total_file_boundary_split_range_entry_count << std::endl;
  std::cout << "Current total count of point entries that are deleted by range tombstones happened in the buffer (@ buffer) = " << total_buffer_range_tombstone_deleted_point_entry_count << std::endl;
  std::cout << "********************************************************" << std::endl;
  return 1;
}