/*
 *  Created on: Jan 28, 2020
 *  Author: Papon
 */
#include "query_runner.h"


using namespace std;
using namespace tree_builder;

int Query::complete_delete_count = 0;
int Query::not_possible_delete_count = 0;
int Query::partial_delete_count = 0;

int Query::range_occurances = 0;
int Query::secondary_range_occurances = 0;

long Query::sum_page_id = 0;
long Query::found_count = 0;
long Query::not_found_count = 0;

long Query::disk_sstfile_access_count = 0;

uint32_t counter = 0;

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

void Query::incDiskSSTFileAccessCount(){disk_sstfile_access_count++;}
long Query::getDiskSSTFileAccessCount(){return disk_sstfile_access_count;}
void Query::resetDiskSSTFileAccessCount(){disk_sstfile_access_count = 0;}


void Query::checkDeleteCount (int deletekey)
{
  Query::complete_delete_count = 0;
  Query::not_possible_delete_count = 0;
  Query::partial_delete_count = 0;

  for (int i = 1; i <= DiskMetaFile::getTotalLevelCount(); i++)
  {
    SSTFile *level_i_head = DiskMetaFile::getSSTFileHead(i);
    SSTFile *moving_head = level_i_head;
    while (moving_head)
    {
      for (int k = 0; k < moving_head->getPointDeleteTileCount(); k++)
      {
        DeleteTile delete_tile = moving_head->getIthPointDeleteTile(k);
        for (int l = 0; l < delete_tile.getPointPageVectorSize(); l++)
        {
          Page page = delete_tile.getIthPointPage(l);
          if (page.getMaxDeleteKey() > 0)
          {
            if (page.getMaxDeleteKey() < deletekey) {
              complete_delete_count++;
            }
            else if (page.getMinDeleteKey() > deletekey) {
              not_possible_delete_count++;
            }
            else {
              partial_delete_count++;
            }
          }
        }
      }
      moving_head = moving_head->getNextFilePtr();
    }

  }
  // std::cout << "(Delete Query)" << std::endl;
  // std::cout << "Compelte Possible Delete Count : " << complete_delete_count << std::endl;
  // std::cout << "Partial Possible Delete Count : " << partial_delete_count << std::endl;
  // std::cout << "Impossible Delete Count : " << not_possible_delete_count << std::endl;
  // std::cout << std::endl;
}

void Query::delete_query_experiment()
{
  EmuEnv* _env = EmuEnv::getInstance();
  // int selectivity[3] = {7, 30, 60};
  double selectivity[20] = {100000, 50000, 10000, 7500, 5000, 2500, 1000, 750, 500, 250};
  double increment = 0.01;
  int j = 10;
  for (float i = 1 ; i <= 10; i++)
  {
    selectivity[j] = (100/i);
    j++;
  }
  int delete_key1;

  fstream fout1;
  fout1.open("out_delete_srq.csv", ios::out | ios::app);
  fout1 << "SRQ Count" << ", " << "Fraction" << "," << "Delete Key" << "," << "Full Drop" << "," << "Partial Drop" << "," << "Impossible Drop" << "\n";

  for (int i = 0 ; i < 20; i++)
  {
    delete_key1 = _env->num_inserts/selectivity[i];
    Query::checkDeleteCount(delete_key1);
    fout1 << _env->srq_count << "," << "1/"+ to_string(selectivity[i])  << "," << delete_key1 << "," << Query::complete_delete_count << "," << Query::partial_delete_count 
      << "," << Query::not_possible_delete_count << endl;
  }

  fout1.close();
}

void Query::range_query_experiment()
{
  EmuEnv* _env = EmuEnv::getInstance();
  float selectivity[35] = {0.0001, 0.0005, 0.001, 0.002, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008, 0.009, 0.01, 0.1, 1, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25};
  int range_iterval_1, range_query_start_1, range_query_end_1;

  fstream fout2;
  fout2.open("out_range_srq.csv", ios::out | ios::app);
  fout2 << "SRQ Count" << ", " << "Selectivity" << "," << "Range Start" << "," << "Range End" << "," << "Occurrences" << "\n";

  for (int i = 0; i < 35 ; i++ )
  {
    if (_env->correlation == 0)
    {
      range_iterval_1 = WorkloadGenerator::KEY_DOMAIN_SIZE * selectivity[i] / 100;
      range_query_start_1 = WorkloadGenerator::KEY_DOMAIN_SIZE / 2 - range_iterval_1 / 2;
      range_query_end_1 = WorkloadGenerator::KEY_DOMAIN_SIZE / 2 + range_iterval_1 / 2;
    }
    else
    {
      range_iterval_1 = _env->num_inserts * selectivity[i] / 100;
      range_query_start_1 = _env->num_inserts / 2 - range_iterval_1 / 2;
      range_query_end_1 = _env->num_inserts / 2 + range_iterval_1 / 2;
    }
    Query::rangeQuery(range_query_start_1, range_query_end_1);
    fout2 << _env->srq_count << "," << selectivity[i] << "%" << "," << range_query_start_1 << "," << range_query_end_1 << "," << Query::range_occurances << endl;
  }
  fout2.close();
}

void Query::sec_range_query_experiment()
{
  EmuEnv* _env = EmuEnv::getInstance();
  int range_iterval_1, range_query_start_1, range_query_end_1;
  float selectivity[35] = {0.0001, 0.0005, 0.001, 0.002, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008, 0.009, 0.01, 0.1, 1, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25};

  fstream fout3;
  fout3.open("out_sec_range_srq.csv", ios::out | ios::app);
  fout3 << "SRQ Count" << ", " << "Selectivity" << "," <<  "Sec Range Start" << "," << "Sec Range End" << "," << "Occurrences" << "\n";

  for (int i = 0; i < 35 ; i++ )
  {
    range_iterval_1 = _env->num_inserts * selectivity[i]/100;  
    range_query_start_1 = _env->num_inserts/2 - range_iterval_1/2;
    range_query_end_1 = _env->num_inserts/2 + range_iterval_1/2;
    Query::secondaryRangeQuery(range_query_start_1, range_query_end_1);
    fout3 << _env->srq_count << "," << selectivity[i] << "%" << "," << range_query_start_1 << "," << range_query_end_1 << "," << Query::secondary_range_occurances << endl;
  }

  fout3.close();
}

void Query::point_query_experiment()
{
  EmuEnv* _env = EmuEnv::getInstance();
  int point_query_iteration1 = 100000;
  fstream fout4;
  fout4.open("out_point.csv", ios::out | ios::app);

  Query::pointQueryRunner(point_query_iteration1);
  fout4 << _env->delete_tile_size_in_pages << "," << point_query_iteration1 << "," << Query::sum_page_id << "," << Query::sum_page_id/(Query::found_count * 1.0) << "," << Query::found_count << "," << Query::not_found_count << endl;
  fout4.close();

}

void Query::new_point_query_experiment ()
{
  EmuEnv* _env = EmuEnv::getInstance();
  fstream fout4;
  fout4.open("out_point_nonempty_srq.csv", ios::out | ios::app);
  fout4 << "SRQ Count" << ", " << "Iterations" << "," <<  "Sum_Page_Id" << "," << "Avg_Page_Id" << "," << "Found" << "," << "Not Found" << "\n";

  counter = 0;
  sum_page_id = 0;
  found_count = 0;
  not_found_count = 0;
  for (int i = 0; i < _env->pq_count ; i++) {
    unsigned long long randomKey = rand() % _env->num_inserts;
    randomKey = WorkloadGenerator::inserted_keys[randomKey];
    int pageId = Query::pointQuery(randomKey);
    if(pageId < 0) 
    {
      not_found_count++;
    }
    else 
    {
      sum_page_id += pageId;
      found_count++;
    }
    counter++;

    if(!(counter % (_env->pq_count/100))){
      showProgress(_env->pq_count, counter);
    }
  }
  fout4 << _env->srq_count << "," << _env->pq_count << "," << Query::sum_page_id << "," << Query::sum_page_id/(Query::found_count * 1.0) << "," << Query::found_count << "," << Query::not_found_count << endl;
  fout4.close();
}

void Query::rangeQuery (int lowerlimit, int upperlimit) {

  range_occurances = 0;

  for (int i = 1; i <= DiskMetaFile::getTotalLevelCount(); i++)
  {
    SSTFile *level_i_head = DiskMetaFile::getSSTFileHead(i);
    SSTFile *moving_head = level_i_head;
    while (moving_head)
    {
      if (moving_head->getMinSortKey() > upperlimit)
        break;
      if (moving_head->getMaxSortKey() < lowerlimit ) {
        moving_head = moving_head->getNextFilePtr();
        continue;
      } 
      else {
        for (int k = 0; k < moving_head->getPointDeleteTileCount(); k++)
        {
          DeleteTile delete_tile = moving_head->getIthPointDeleteTile(k);
          if (delete_tile.getMinSortKey() > upperlimit || delete_tile.getMaxSortKey() < lowerlimit) {
            continue;
          }
          else {
            for (int l = 0; l < delete_tile.getPointPageVectorSize(); l++)
            {
              Page page = delete_tile.getIthPointPage(l);
              if (page.getMinSortKey() > 0)
              {
                if (page.getMinSortKey() > upperlimit || page.getMaxSortKey() < lowerlimit) {
                continue;
                }
                else {
                  range_occurances++;
                }
              }
            }
          }
        }
      }
      moving_head = moving_head->getNextFilePtr();
    }
  }
  // std::cout << "(Range Query)" << std::endl;
  // std::cout << "Pages traversed : " << range_occurances << std::endl << std::endl;
}

void Query::secondaryRangeQuery (int lowerlimit, int upperlimit) {

  secondary_range_occurances = 0;

  for (int i = 1; i <= DiskMetaFile::getTotalLevelCount(); i++)
  {
    SSTFile *level_i_head = DiskMetaFile::getSSTFileHead(i);
    SSTFile *moving_head = level_i_head;
    while (moving_head)
    {
      if (moving_head->getMinDeleteKey() > upperlimit || moving_head->getMaxDeleteKey() < lowerlimit ) {
        moving_head = moving_head->getNextFilePtr();
        continue;
      } 
      else {
        for (int k = 0; k < moving_head->getPointDeleteTileCount(); k++)
        {
          DeleteTile delete_tile = moving_head->getIthPointDeleteTile(k);
          if (delete_tile.getMinDeleteKey() > upperlimit || delete_tile.getMaxDeleteKey() < lowerlimit) {
            continue;
          }
          else {
            for (int l = 0; l < delete_tile.getPointPageVectorSize(); l++)
            {
              Page page = delete_tile.getIthPointPage(l);
              if (page.getMinDeleteKey() > 0)
              {
                if (page.getMinDeleteKey() > upperlimit || page.getMaxDeleteKey() < lowerlimit) {
                  continue;
                }
                else {
                  secondary_range_occurances++;
                }
              }
            }
          }
        }
      }
      moving_head = moving_head->getNextFilePtr();
    }
  }
}

int Query::pointQuery (int key)
{
  for (int i = 1; i <= DiskMetaFile::getTotalLevelCount(); i++)
  {
    SSTFile *level_i_head = DiskMetaFile::getSSTFileHead(i);
    SSTFile *moving_head = level_i_head;
    while (moving_head)
    {
      if (moving_head->getMinSortKey() > key)
        break;
      if (moving_head->getMinSortKey() <= key && key <= moving_head->getMaxSortKey()) {
        for (int k = 0; k < moving_head->getPointDeleteTileCount(); k++)
        {
          DeleteTile delete_tile = moving_head->getIthPointDeleteTile(k);

          if (delete_tile.getMinSortKey() > key)
            break;
          if (delete_tile.getMinSortKey() <= key && key <= delete_tile.getMaxSortKey()) {
            for (int l = 0; l < delete_tile.getPointPageVectorSize(); l++)
            {
              Page page = delete_tile.getIthPointPage(l);
              if (page.getMinSortKey() <= key && key <= page.getMaxSortKey()) {
                for (int m = 0; m < page.getPointEntryVectorSize(); m++) {
                  if (key == page.getIthPointEntry(m).getSortKey()) {
                    return l + 1;
                  }
                }
              }
            }
          }
        }
      }
      moving_head = moving_head->getNextFilePtr();
    }
  }
  return -1;
}


pair<bool, string> Query::pointQuery_RDF (long key)
{
  //A1. search buffer (@range delete already deleted older point entries)
  EntryList buffer = MemoryBuffer::getBuffer();
  for(size_t i = 0; i < buffer.get_point_entry_vector_size(); i++)
  {
    if (buffer.get_ith_point_entry(i).getSortKey() == key)
      return make_pair(true, buffer.get_ith_point_entry(i).getValue());
  }

  //point part not sorted
  //b1: binary search on buffer range tombstone
  // buffer.sortRangeEntryVectorOnStartKeyInAscendingOrder();
  // buffer.merge_its_own_range_entry();
  // vector<RangeEntry> rangeTombstone = buffer.get_range_entry_vector();
  // RangeEntry targetRangeEntry = {make_pair(key, key), -1};
  // auto it = upper_bound(rangeTombstone.begin(), rangeTombstone.end(), targetRangeEntry, 
  //   [](RangeEntry a, RangeEntry &b) {return a.getStartKey() < b.getStartKey();});

  // if(it != rangeTombstone.begin())
  // {
  //   it--;
  //   if(it->getStartKey() <= key && key <= it->getEndKey()){
  //     return make_pair(false, "");
  //   }
  // }
  
  //b2: linear search on buffer range tombstone
  vector<RangeEntry> rangeTombstone = buffer.get_range_entry_vector();
  for(size_t i = 0; i < rangeTombstone.size(); i++)
  {
    if(rangeTombstone[i].getStartKey() <= key && key <= rangeTombstone[i].getEndKey())
    {
      return make_pair(false, "");
    }
  }


  //A2. search disk
  
  for (int i = 1; i <= DiskMetaFile::getTotalLevelCount(); i++)
  {
    SSTFile *level_i_head = DiskMetaFile::getSSTFileHead(i);
    SSTFile *moving_head = level_i_head;
    while (moving_head)
    {
      if(moving_head->getMinSortKey() > key){break;}
      if(moving_head->getMaxSortKey() < key)
      {
        moving_head = moving_head->getNextFilePtr();
        continue;
      }

      //probing SST file on disk, making an IO
      incDiskSSTFileAccessCount();

      for (int k = 0; k < moving_head->getPointDeleteTileCount(); k++)
      {
        DeleteTile delete_tile = moving_head->getIthPointDeleteTile(k);

        if(delete_tile.getMinSortKey() > key){break;}
        if(delete_tile.getMaxSortKey() < key){continue;}

        for (int l = 0; l < delete_tile.getPointPageVectorSize(); l++)
        {
          Page page = delete_tile.getIthPointPage(l);

          if(page.getMinSortKey() > key){break;}
          if(page.getMaxSortKey() < key){continue;}

          for (int m = 0; m < page.getPointEntryVectorSize(); m++) {
            if (key != page.getIthPointEntry(m).getSortKey()) { continue; }
              
            return make_pair(true, page.getIthPointEntry(m).getValue());
          }
        }
      }
      
      moving_head = moving_head->getNextFilePtr();
    }

    bool isAlive = DiskMetaFile::isAliveThroughRangeDeleteFilterAtLevel(i, key);
    if(!isAlive){break;}
  }
  return make_pair(false, "");
}


pair<bool, string> Query::pointQuery_SplitRDF (long key)
{
  //A1. search buffer (@range delete already deleted older point entries)
  EntryList buffer = MemoryBuffer::getBuffer();
  for(size_t i = 0; i < buffer.get_point_entry_vector_size(); i++)
  {
    if (buffer.get_ith_point_entry(i).getSortKey() == key)
      return make_pair(true, buffer.get_ith_point_entry(i).getValue());
  }

  //point part not sorted
  //b1: binary search on buffer range tombstone
  // buffer.sortRangeEntryVectorOnStartKeyInAscendingOrder();
  // buffer.merge_its_own_range_entry();
  // vector<RangeEntry> rangeTombstone = buffer.get_range_entry_vector();
  // RangeEntry targetRangeEntry = {make_pair(key, key), -1};
  // auto it = upper_bound(rangeTombstone.begin(), rangeTombstone.end(), targetRangeEntry, 
  //   [](RangeEntry a, RangeEntry &b) {return a.getStartKey() < b.getStartKey();});

  // if(it != rangeTombstone.begin())
  // {
  //   it--;
  //   if(it->getStartKey() <= key && key <= it->getEndKey()){
  //     return make_pair(false, "");
  //   }
  // }
  
  //b2: linear search on buffer range tombstone
  vector<RangeEntry> rangeTombstone = buffer.get_range_entry_vector();
  for(size_t i = 0; i < rangeTombstone.size(); i++)
  {
    if(rangeTombstone[i].getStartKey() <= key && key <= rangeTombstone[i].getEndKey())
    {
      return make_pair(false, "");
    }
  }


  //A2. search disk
  
  for (int i = 1; i <= DiskMetaFile::getTotalLevelCount(); i++)
  {
    
    bool isAlive = DiskMetaFile::isAliveThroughSplitRangeDeleteFilterAtLevel(i, key);
    if(!isAlive){break;}

    SSTFile *level_i_head = DiskMetaFile::getSSTFileHead(i);
    SSTFile *moving_head = level_i_head;
    while (moving_head)
    {
      if(moving_head->getMinSortKey() > key){break;}
      if(moving_head->getMaxSortKey() < key)
      {
        moving_head = moving_head->getNextFilePtr();
        continue;
      }

      //probing SST file on disk, making an IO
      incDiskSSTFileAccessCount();

      for (int k = 0; k < moving_head->getPointDeleteTileCount(); k++)
      {
        DeleteTile delete_tile = moving_head->getIthPointDeleteTile(k);

        if(delete_tile.getMinSortKey() > key){break;}
        if(delete_tile.getMaxSortKey() < key){continue;}

        for (int l = 0; l < delete_tile.getPointPageVectorSize(); l++)
        {
          Page page = delete_tile.getIthPointPage(l);

          if(page.getMinSortKey() > key){break;}
          if(page.getMaxSortKey() < key){continue;}

          for (int m = 0; m < page.getPointEntryVectorSize(); m++) {
            if (key != page.getIthPointEntry(m).getSortKey()) { continue; }
              
            return make_pair(true, page.getIthPointEntry(m).getValue());
          }
        }
      }
      
      moving_head = moving_head->getNextFilePtr();
    }

    
    // bool isAlive = DiskMetaFile::isAliveThroughSplitRangeDeleteFilterAtLevel(i, key);
    // if(!isAlive){break;}

  }
  return make_pair(false, "");
}


pair<bool, string> Query::pointQuery_RDF_SkyLine (long key, long timetag) //rocksDB
{
  //A1. search buffer (@range delete already deleted older point entries)
  EntryList buffer = MemoryBuffer::getBuffer();
  for(size_t i = 0; i < buffer.get_point_entry_vector_size(); i++)
  {
    if (buffer.get_ith_point_entry(i).getSortKey() == key)
      return make_pair(true, buffer.get_ith_point_entry(i).getValue());
  }

  //point part not sorted
  //b1: binary search on buffer range tombstone
  // buffer.sortRangeEntryVectorOnStartKeyInAscendingOrder();
  // buffer.merge_its_own_range_entry();
  // vector<RangeEntry> rangeTombstone = buffer.get_range_entry_vector();
  // RangeEntry targetRangeEntry = {make_pair(key, key), -1};
  // auto it = upper_bound(rangeTombstone.begin(), rangeTombstone.end(), targetRangeEntry, 
  //   [](RangeEntry a, RangeEntry &b) {return a.getStartKey() < b.getStartKey();});

  // if(it != rangeTombstone.begin())
  // {
  //   it--;
  //   if(it->getStartKey() <= key && key <= it->getEndKey()){
  //     return make_pair(false, "");
  //   }
  // }
  
  //b2: linear search on buffer range tombstone
  vector<RangeEntry> rangeTombstone = buffer.get_range_entry_vector();
  for(size_t i = 0; i < rangeTombstone.size(); i++)
  {
    if(rangeTombstone[i].getStartKey() <= key && key <= rangeTombstone[i].getEndKey())
    {
      return make_pair(false, "");
    }
  }


  //A2. search disk
  
  for (int i = 1; i <= DiskMetaFile::getTotalLevelCount(); i++)
  {
    SSTFile *level_i_head = DiskMetaFile::getSSTFileHead(i);
    SSTFile *moving_head = level_i_head;
    while (moving_head)
    {
      if(moving_head->getMinSortKey() > key){break;}
      if(moving_head->getMaxSortKey() < key)
      {
        moving_head = moving_head->getNextFilePtr();
        continue;
      }

      //probing SST file on disk, making an IO
      incDiskSSTFileAccessCount();

      for (int k = 0; k < moving_head->getPointDeleteTileCount(); k++)
      {
        DeleteTile delete_tile = moving_head->getIthPointDeleteTile(k);

        if(delete_tile.getMinSortKey() > key){break;}
        if(delete_tile.getMaxSortKey() < key){continue;}

        for (int l = 0; l < delete_tile.getPointPageVectorSize(); l++)
        {
          Page page = delete_tile.getIthPointPage(l);

          if(page.getMinSortKey() > key){break;}
          if(page.getMaxSortKey() < key){continue;}

          for (int m = 0; m < page.getPointEntryVectorSize(); m++) {
            if (key != page.getIthPointEntry(m).getSortKey()) { continue; }
            
            if(DiskMetaFile::isAliveThroughSkylineRangeDeleteFilter(key, timetag) == false){
              return make_pair(false, "");
            }
            return make_pair(true, page.getIthPointEntry(m).getValue());
          }
        }
      }
      
      // moving_head = moving_head->next_file_ptr;
      moving_head = moving_head->getNextFilePtr();
    }

    // bool isAlive = DiskMetaFile::isAliveThroughRangeDeleteFilterAtLevel(i, key);
    // if(!isAlive){break;}
  }
  return make_pair(false, "");
}







void Query::pointQueryRunner (int iterations)
{
  counter = 0;
  sum_page_id = 0;
  found_count = 0;
  not_found_count = 0;
  for (int i = 0; i < iterations ; i++) {
    unsigned long long randomKey = rand() %  WorkloadGenerator::KEY_DOMAIN_SIZE;
    int pageId = Query::pointQuery(randomKey);
    if(pageId < 0) 
    {
      not_found_count++;
    }
    else 
    {
      sum_page_id += pageId;
      found_count++;
    }
    counter++;

    if(!(counter % (iterations/100))){
      showProgress(iterations, counter);
    }
  }
    // std::cout << "(Point Query)" << std::endl;
    // std::cout << "Total sum of found pageIDs : " <<  sumPageId << std::endl;
    // std::cout << "Total number of found pageIDs : " <<  foundCount << std::endl;
    // std::cout << "Total number of found average pageIDs : " <<  sumPageId/(foundCount * 1.0) << std::endl;
    // std::cout << "Total number of not found pages : " <<  notFoundCount << std::endl << std::endl;
}

void Query::interactive_point_query(){
  cout << "Enter the key to search for (Enter -1 to quit): ";
  long key = 0;
  while(std::cin >> key){
    if(key == -1){break;}

    int page_id = Query::pointQuery(key);
    if(page_id < 0) {
      std::cout << "Not Found" << std::endl;
    }
    else {
      std::cout << "Found at page " << page_id << std::endl;
    }

    cout << "Enter the key to search for (Enter -1 to quit): ";
  }
}
