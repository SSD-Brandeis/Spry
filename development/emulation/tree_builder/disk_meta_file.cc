#include "disk_meta_file.h"

using namespace std;
using namespace tree_builder;
using namespace workload_exec;


vector<PL_RDF> DiskMetaFile::per_level_range_delete_filter({PL_RDF()}); //LSM level on disk start from 1 //added by ychaung
vector<PL_RDF> DiskMetaFile::per_level_range_delete_filter_split({PL_RDF()}); //LSM level on disk start from 1 //added by ychaung
SkylineRangeDeleteFilter DiskMetaFile::skyline_range_delete_filter; //added by ychaung

SSTFile *DiskMetaFile::level_head[32] = {};
long DiskMetaFile::level_min_key[32] = {};
long DiskMetaFile::level_max_key[32] = {};
long DiskMetaFile::level_file_count[32] = {};
long DiskMetaFile::level_entry_count[32] = {};
long DiskMetaFile::level_max_size[32] = {};
long DiskMetaFile::level_max_file_count[32] = {};
long DiskMetaFile::level_current_size[32] = {};

long DiskMetaFile::global_level_file_counter[32] = {};
float DiskMetaFile::disk_run_flush_threshold[32] = {};

int DiskMetaFile::compaction_counter[32] = {};
int DiskMetaFile::compaction_through_sortmerge_counter[32] = {};
int DiskMetaFile::compaction_through_point_manipulation_counter[32] = {};
int DiskMetaFile::compaction_file_counter[32] = {};





//Class DiskMetaFile

int DiskMetaFile::getLevelFileCount(int level)
{
  SSTFile *level_head = DiskMetaFile::getSSTFileHead(level);
  SSTFile *moving_head = level_head;
  long level_size_in_files = 0;
  while (moving_head)
  {
    level_size_in_files++;
    moving_head = moving_head->getNextFilePtr();
  }
  return level_size_in_files;
}

int DiskMetaFile::getTotalPointPageCount()
{
  int counter = 0;
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
          counter++;
        }
      }
      moving_head = moving_head->getNextFilePtr();
    }
  }
  return counter;
}


int DiskMetaFile::getTotalRangePageCount()
{
  int counter = 0;
  for (int i = 1; i <= DiskMetaFile::getTotalLevelCount(); i++)
  {
    SSTFile *level_i_head = DiskMetaFile::getSSTFileHead(i);
    SSTFile *moving_head = level_i_head;
    while (moving_head)
    {
      for (int k = 0; k < moving_head->getRangeDeleteTileCount(); k++)
      {
        DeleteTile delete_tile = moving_head->getIthRangeDeleteTile(k);
        for (int l = 0; l < delete_tile.getRangePageVectorSize(); l++)
        {
          counter++;
        }
      }
      moving_head = moving_head->getNextFilePtr();
    }
  }
  return counter;
}

// PointEntry
long DiskMetaFile::getLevelPointEntryCount(int level) 
{
  SSTFile *level_head = DiskMetaFile::getSSTFileHead(level);
  SSTFile *moving_head = level_head;
  long level_size_in_entries = 0;
  while (moving_head)
  {
    
    for (int k = 0; k < moving_head->getPointDeleteTileCount(); k++)
    {
      DeleteTile delete_tile = moving_head->getIthPointDeleteTile(k);
      for (int l = 0; l < delete_tile.getPointPageVectorSize(); l++)
      {
        Page page = delete_tile.getIthPointPage(l);

        for (int m = 0; m < page.getPointEntryVectorSize(); m++)
        {
          level_size_in_entries++;
        }
      }
    }
    moving_head = moving_head->getNextFilePtr();
  }
  return level_size_in_entries;
}

// RangeEntry
long DiskMetaFile::getLevelRangeEntryCount(int level) 
{
  SSTFile *level_head = DiskMetaFile::getSSTFileHead(level);
  SSTFile *moving_head = level_head;
  long level_size_in_range_entries = 0;
  while (moving_head)
  {    
    for (int k = 0; k < moving_head->getRangeDeleteTileCount(); k++)
    {
      DeleteTile delete_tile = moving_head->getIthRangeDeleteTile(k);
      for (int l = 0; l < delete_tile.getRangePageVectorSize(); l++)
      {
        Page page = delete_tile.getIthRangePage(l);

        for (int m = 0; m < page.getRangeEntryVectorSize(); m++)
        {
          level_size_in_range_entries++;
        }
      }
    }
    moving_head = moving_head->getNextFilePtr();
  }
  return level_size_in_range_entries;
}


int DiskMetaFile::getTotalLevelCount()
{
  int level_count = 0;
  for (int i = 1; i <= 32; i++)
  {
    SSTFile *level_head = DiskMetaFile::getSSTFileHead(i);
    if (level_head)
    {
      level_count++;
    }
    else
    {
      return level_count;
    }
  }
  return level_count;
}


int DiskMetaFile::checkOverlapping(SSTFile *file, int level)
{
  SSTFile *head_level_1 = DiskMetaFile::getSSTFileHead(level);
  SSTFile *moving_head = head_level_1;
  int overlap_count = 0;
  while (moving_head)
  {
    if (moving_head->getMaxSortKey() <= file->getMinSortKey() || moving_head->getMinSortKey() >= file->getMaxSortKey())
    {
      moving_head = moving_head->getNextFilePtr();
      continue;
    }
    overlap_count++;
    moving_head = moving_head->getNextFilePtr();
  }
  return overlap_count;
}

// int DiskMetaFile::checkAndAdjustLevelSaturation(int level)
// {
//   int entry_count_in_level = getLevelPointEntryCount(level);
//   int file_count_in_level = getLevelFileCount(level);
//   EmuEnv *_env = EmuEnv::getInstance();
//   //count the true point entry + range entry size
//   int max_entry_count_in_level = DiskMetaFile::level_max_size[level] / _env->entry_size;
//   int max_file_count_in_level = DiskMetaFile::level_max_file_count[level];
//   if (file_count_in_level >= max_file_count_in_level)
//   {
//     if (MemoryBuffer::verbosity == 2)
//       std::cout << "Saturation Reached......" << endl;
    
//     //Push the file with minimum overlapping into the next level
//     SSTFile *level_head = DiskMetaFile::getSSTFileHead(level);
//     SSTFile *moving_head = level_head;
//     SSTFile *min_overlap_file = level_head;
//     int overlap;
//     int min_overlap = INT32_MAX;
//     while (moving_head)
//     {
//       overlap = DiskMetaFile::checkOverlapping(moving_head, level + 1);
//       if (overlap < min_overlap)
//       {
//         min_overlap = overlap;
//         min_overlap_file = moving_head;
//       }
//       moving_head = moving_head->getNextFilePtr();
//     }


//     //If that file is level head, update head
//     if (min_overlap_file == DiskMetaFile::getSSTFileHead(level))
//     {
//       DiskMetaFile::setSSTFileHead(level_head->getNextFilePtr(), level);
//     }

//     moving_head = level_head;

//     while (moving_head)
//     {
//       if (moving_head->getNextFilePtr() == min_overlap_file)
//       {
//         moving_head->setNextFilePtr(min_overlap_file->getNextFilePtr());
//       }
//       moving_head = moving_head->getNextFilePtr();
//     }

//     //Converting to vector (only that file)
//     vector < pair < pair < long, long >, string > > vector_to_compact;

//     for (int k = 0; k < min_overlap_file->getPointDeleteTileCount(); k++)
//     {
//       DeleteTile delete_tile = min_overlap_file->getIthPointDeleteTile(k);
//       for (int l = 0; l < delete_tile.getPointPageVectorSize(); l++)
//       {
//         Page page = delete_tile.getIthPointPage(l);
//         for (int m = 0; m < page.getPointEntryVectorSize(); m++)
//         {
//           PointEntry entry = page.getIthPointEntry(m);
//           vector_to_compact.push_back( make_pair( make_pair(entry.getSortKey(), entry.getDeleteKey()), entry.getValue() ) );
//         }
//       }
//     }

//     //Sending it to next level.
//     if (MemoryBuffer::verbosity == 2)
//     {
//       cout << "Level : " << level << " Calling sort and write from Disk Saturation Checker............................" << endl;
//       for (int j = 0; j < vector_to_compact.size(); ++j)
//         std::cout << "< " << vector_to_compact[j].first.first << ",  " << vector_to_compact[j].first.second << " >"
//                   << "\t" << std::endl;
//     }

//     Utility::sortAndWrite(vector_to_compact, level + 1);

//   }
//   return 1;
// }


int DiskMetaFile::checkAndAdjustLevelSaturation_RDF(int level)
{
  int entry_count_in_level = getLevelPointEntryCount(level);
  int file_count_in_level = getLevelFileCount(level);
  EmuEnv *_env = EmuEnv::getInstance();
  //count the true point entry + range entry size
  int max_entry_count_in_level = DiskMetaFile::level_max_size[level] / _env->entry_size;
  int max_file_count_in_level = DiskMetaFile::level_max_file_count[level];
  if (file_count_in_level >= max_file_count_in_level)
  {
    
    //Push the file with minimum overlapping into the next level
    SSTFile *level_head = DiskMetaFile::getSSTFileHead(level);
    SSTFile *moving_head = level_head;
    SSTFile *min_overlap_file = level_head;
    int overlap;
    int min_overlap = INT32_MAX;
    while (moving_head)
    {
      overlap = DiskMetaFile::checkOverlapping(moving_head, level + 1);
      if (overlap < min_overlap)
      {
        min_overlap = overlap;
        min_overlap_file = moving_head;
      }
      moving_head = moving_head->getNextFilePtr();
    }


    //If that file is level head, update head
    if (min_overlap_file == DiskMetaFile::getSSTFileHead(level))
    {
      DiskMetaFile::setSSTFileHead(level_head->getNextFilePtr(), level);
    }

    moving_head = level_head;

    while (moving_head)
    {
      if (moving_head->getNextFilePtr() == min_overlap_file)
      {
        moving_head->setNextFilePtr(min_overlap_file->getNextFilePtr());
      }
      moving_head = moving_head->getNextFilePtr();
    }

    //Converting to vector (only that file)
    EntryList &entries_to_compact = *(new EntryList());

    //point part
    for (int k = 0; k < min_overlap_file->getPointDeleteTileCount(); k++)
    {
      DeleteTile delete_tile = min_overlap_file->getIthPointDeleteTile(k);
      for (int l = 0; l < delete_tile.getPointPageVectorSize(); l++)
      {
        Page page = delete_tile.getIthPointPage(l);
        if(page.getPageType() == Page::POINT){ //always this case
          for (int m = 0; m < page.getPointEntryVectorSize(); m++)
          {
            PointEntry entry = page.getIthPointEntry(m);
            entries_to_compact.add_point_entry(entry);
          }
        }else if(page.getPageType() == Page::RANGE){ 
          cout << "Error: Page type is range, but it should be point" << endl;
          exit(1);
        }
      }
    }
    //range part (by yucheng)
    for (int k = 0; k < min_overlap_file->getRangeDeleteTileCount(); k++)
    {
      DeleteTile delete_tile = min_overlap_file->getIthRangeDeleteTile(k);
      for (int l = 0; l < delete_tile.getRangePageVectorSize(); l++)
      {
        Page page = delete_tile.getIthRangePage(l);
        if(page.getPageType() == Page::POINT){
          cout << "Error: Page type is point, but it should be range" << endl;
          exit(1);
        }else if(page.getPageType() == Page::RANGE){ //always this case
          for(int m = 0; m < page.getRangeEntryVectorSize(); m++)
          {
            RangeEntry entry = page.getIthRangeEntry(m);
            entries_to_compact.add_range_entry(entry);
          }
        }
      }
    }

    Utility::sortAndWrite_RDF(entries_to_compact, level + 1);
    checkAndAdjustLevelSaturation_RDF(level); // after compaction the file number may still be saturated
  }
  
  return 1;
}




int DiskMetaFile::setSSTFileHead(SSTFile *arg, int level_to_flush_in)
{
  DiskMetaFile::level_head[level_to_flush_in] = arg;
  return 1;
}

SSTFile *DiskMetaFile::getSSTFileHead(int level)
{
  return DiskMetaFile::level_head[level];
}


long DiskMetaFile::getTotalPointEntryCount(){
  long total_point_entry_count = 0;
  for (int i = 1; i <= DiskMetaFile::getTotalLevelCount(); i++)
  {
    long point_count = DiskMetaFile::getLevelPointEntryCount(i);
    total_point_entry_count += point_count;
  }
  return total_point_entry_count;
}

long DiskMetaFile::getTotalRangeEntryCount(){
  long total_range_entry_count = 0;
  for (int i = 1; i <= DiskMetaFile::getTotalLevelCount(); i++)
  {
    long range_count = DiskMetaFile::getLevelRangeEntryCount(i);
    total_range_entry_count += range_count;
  }
  return total_range_entry_count;
}



void DiskMetaFile::getMetaStatistics()
{
  long total_entry_count = 0;
  long total_point_entry_count = 0;
  long total_range_entry_count = 0;
  std::cout << "**************************** PRINTING META FILE STATISTICS ****************************" << std::endl;

  for (int i = 1; i <= DiskMetaFile::getTotalLevelCount(); i++)
  {
    long point_count = DiskMetaFile::getLevelPointEntryCount(i);
    long range_count = DiskMetaFile::getLevelRangeEntryCount(i);
    total_entry_count += point_count + range_count;
    total_point_entry_count += point_count;
    total_range_entry_count += range_count;
  }
  std::cout << "\nTotal number of entries: " << total_entry_count << "\n" << std::endl;
  std::cout << "\nTotal number of point entries: " << total_point_entry_count << "\n" << std::endl;
  std::cout << "\nTotal number of range entries: " << total_range_entry_count << "\n" << std::endl;
  std::cout << "L\tfile_count\tpoint_entry_count\trange_entry_count" << std::endl;
  for (int i = 1; i <= DiskMetaFile::getTotalLevelCount(); i++)
  {
    std::cout << i << "\t" << setfill(' ') << setw(8) << DiskMetaFile::getLevelFileCount(i) << "\t\t"
              <<setw(8)<< DiskMetaFile::getLevelPointEntryCount(i) << "\t\t"<<setw(8)<< DiskMetaFile::getLevelRangeEntryCount(i) <<std::endl;    
  }

  std::cout << "***************************************************************************************" << std::endl;
}



int DiskMetaFile::printAllEntries(int only_file_meta_data)
{
  
  EmuEnv *_env = EmuEnv::getInstance();
  std::cout << "**************************** PARAMETERS ****************************" << std::endl;
  std::cout << "Buffer size in pages (P): " << _env->buffer_size_in_pages << std::endl;
  std::cout << "Entries per pages (B): " << _env->entries_per_page << std::endl;
  std::cout << "Entry Size (E): " << _env->entry_size << std::endl;
  std::cout << "Buffer Size (PBE): " << _env->buffer_size << std::endl;
  std::cout << "Delete Tile Size in Pages (H): " << _env->delete_tile_size_in_pages << std::endl;
  std::cout << "Size Ratio: " << _env->size_ratio << std::endl;
  std::cout << "********************************************************************\n" << std::endl;

  std::cout << "*************************************************** PRINTING ALL ENTRIES *********************************************************\n" << std::endl;
  std::cout << "\033[1;31mBuffer : " << "\033[0m" << std::endl;
  if (MemoryBuffer::current_buffer_entry_count == 0) {
    std::cout << "\t\t\t\tBuffer is currently empty." << std::endl;
  }
  else {    //UNCOMMENT
    std::cout << "\t\t\t\tBuffer entry count: " << MemoryBuffer::current_buffer_entry_count << std::endl;
    // Uncomment the following to print all entries
    // for (int i = 0; i < MemoryBuffer::current_buffer_entry_count; i++) {
    //   std::cout << "\t\t\t\tEntry : " << i << " (Sort_Key : " << MemoryBuffer::buffer[i].first.first
    //             << ", Delete_Key : " << MemoryBuffer::buffer[i].first.second << ")" << std::endl;
    // }
  }

  for (int i = 1; i <= DiskMetaFile::getTotalLevelCount(); i++)
  {
    SSTFile *level_i_head = DiskMetaFile::getSSTFileHead(i);
    SSTFile *moving_head = level_i_head;

    std::cout << "\033[1;31mLevel : " << i << "\033[0m" << std::endl;
    while (moving_head)
    {
      std::cout << "\033[1;34m\tFile : " << moving_head->getFileId() << " (Min_Sort_Key : " << moving_head->getMinSortKey()
                << ", Max_Sort_Key : " << moving_head->getMaxSortKey() << ", Min_Delete_Key : " << moving_head->getMinDeleteKey()
                << ", Max_Delete_Key : " << moving_head->getMaxDeleteKey() 
                << ", Min_Range_Key : " << moving_head->getMinRangeKey() << ", Max_Range_Key : " << moving_head->getMaxRangeKey() << ")"
                << "\033[0m" << std::endl;
      if (!only_file_meta_data)
      {
        //Point part
        for (int k = 0; k < moving_head->getPointDeleteTileCount(); k++)
        {
          DeleteTile delete_tile = moving_head->getIthPointDeleteTile(k);
          std::cout << "\033[1;32m\t\t(Point) Delete Tile : " << k << " (Min_Sort_Key : " << delete_tile.getMinSortKey()
                    << ", Max_Sort_Key : " << delete_tile.getMaxSortKey() << ", Min_Delete_Key : " << delete_tile.getMinDeleteKey()
                    << ", Max_Delete_Key : " << delete_tile.getMaxDeleteKey() << ")"
                    << "\033[0m" << std::endl;
          for (int l = 0; l < delete_tile.getPointPageVectorSize(); l++)
          {
            Page page = delete_tile.getIthPointPage(l);
            std::cout << "\033[1;33m\t\t\t(Point) Page : " << l << " (Min_Sort_Key : " << page.getMinSortKey()
                      << ", Max_Sort_Key : " << page.getMaxSortKey() << ", Min_Delete_Key : " << page.getMinDeleteKey() << ", Max_Delete_Key : " << page.getMaxDeleteKey() << ")"
                      << "\033[0m" << std::endl;
            std::cout << "\t\t\t\t(Point) Entry count: " << page.getPointEntryVectorSize() << std::endl;
            // Uncomment the following to print all entries
            // for (int m = 0; m < page.kv_vector.size(); m++)
            // {
            //   std::cout << "\t\t\t\tEntry : " << m << " (Sort_Key : " << page.kv_vector[m].first.first
            //             << ", Delete_Key : " << page.kv_vector[m].first.second << ")" << std::endl;
            // }
          }
        }

        //Range part
        for (int k = 0; k < moving_head->getRangeDeleteTileCount(); k++)
        {
          DeleteTile delete_tile = moving_head->getIthRangeDeleteTile(k);
          std::cout << "\033[1;32m\t\t(Range) Delete Tile : " << k << " (Min_Range_Key : " << delete_tile.getMinRangeKey()
                    << ", Max_Range_Key : " << delete_tile.getMaxRangeKey() 
                    << "\033[0m" << std::endl;
          for (int l = 0; l < delete_tile.getRangePageVectorSize(); l++)
          {
            Page page = delete_tile.getIthRangePage(l);
            std::cout << "\033[1;33m\t\t\t(Range) Page : " << l << " (Min_Range_Key : " << page.getMinRangeKey()
                      << ", Max_Range_Key : " << page.getMaxRangeKey() 
                      << "\033[0m" << std::endl;
            std::cout << "\t\t\t\t(Range) Entry count: " << page.getRangeEntryVectorSize() << std::endl;
            // Uncomment the following to print all entries
            // for (int m = 0; m < page.kv_vector.size(); m++)
            // {
            //   std::cout << "\t\t\t\tEntry : " << m << " (Sort_Key : " << page.kv_vector[m].first.first
            //             << ", Delete_Key : " << page.kv_vector[m].first.second << ")" << std::endl;
            // }
          }
        }
      }


      moving_head = moving_head->getNextFilePtr();
    }
  }
  std::cout << "\n**********************************************************************************************************************************\n" << std::endl;
  return 1;
}


int DiskMetaFile::printAllEntriesStyle2(int only_file_meta_data)
{

  

  for (int i = 1; i <= DiskMetaFile::getTotalLevelCount(); i++)
  {
    SSTFile *level_i_head = DiskMetaFile::getSSTFileHead(i);
    SSTFile *moving_head = level_i_head;

    std::cout << "\033[1;31mLevel : " << i << "\033[0m" << std::endl;
    while (moving_head)
    {
      std::cout << "\033[1;34m\tFile : " << moving_head->getFileId() << " (Min_Sort_Key : " << moving_head->getMinSortKey()
                << ", Max_Sort_Key : " << moving_head->getMaxSortKey() << ", Min_Delete_Key : " << moving_head->getMinDeleteKey()
                << ", Max_Delete_Key : " << moving_head->getMaxDeleteKey()
                << ", Min_Range_Key : " << moving_head->getMinRangeKey() << ", Max_Range_Key : " << moving_head->getMaxRangeKey() << ")"
                << "\033[0m" << std::endl;
      if (!only_file_meta_data)
      {
        //Point part
        for (int k = 0; k < moving_head->getPointDeleteTileCount(); k++)
        {
          DeleteTile delete_tile = moving_head->getIthPointDeleteTile(k);
          std::cout << "\033[1;32m\t\t(Point) Delete Tile : " << k << " (Min_Sort_Key : " << delete_tile.getMinSortKey()
                    << ", Max_Sort_Key : " << delete_tile.getMaxSortKey() << ", Min_Delete_Key : " << delete_tile.getMinDeleteKey()
                    << ", Max_Delete_Key : " << delete_tile.getMaxDeleteKey() << ")"
                    << "\033[0m" << std::endl;
          for (int l = 0; l < delete_tile.getPointPageVectorSize(); l++)
          {
            Page page = delete_tile.getIthPointPage(l);
            std::cout << "\033[1;33m\t\t\t(Point) Page : " << l << " (Min_Sort_Key : " << page.getMinSortKey()
                      << ", Max_Sort_Key : " << page.getMaxSortKey() << ", Min_Delete_Key : " << page.getMinDeleteKey() << ", Max_Delete_Key : " << page.getMaxDeleteKey() << ")"
                      << "\033[0m" << std::endl;
            std::cout << "\t\t\t\t(Point) Entry count: " << page.getPointEntryVectorSize() << std::endl;
            // Uncomment the following to print all entries
            for (int m = 0; m < page.getPointEntryVectorSize(); m++)
            {
              std::cout << "\t\t\t\t(Point) Entry : " << m << " (Sort_Key : " << page.getIthPointEntry(m).getSortKey()
                        << ", Delete_Key : " << page.getIthPointEntry(m).getDeleteKey() << ")" << std::endl;
            }
          }
        }
         

        for (int k = 0; k < moving_head->getRangeDeleteTileCount(); k++)
        {
          DeleteTile delete_tile = moving_head->getIthRangeDeleteTile(k);
          std::cout << "\033[1;32m\t\t(Range) Delete Tile : " << k << " (Min_Range_Key : " << delete_tile.getMinRangeKey()
                    << ", Max_Range_Key : " << delete_tile.getMaxRangeKey() 
                    << "\033[0m" << std::endl;
          for (int l = 0; l < delete_tile.getRangePageVectorSize(); l++)
          {
            Page page = delete_tile.getIthRangePage(l);
            std::cout << "\033[1;33m\t\t\t(Range) Page : " << l << " (Min_Range_Key : " << page.getMinRangeKey()
                      << ", Max_Range_Key : " << page.getMaxRangeKey() 
                      << "\033[0m" << std::endl;
            std::cout << "\t\t\t\t(Range) Entry count: " << page.getRangeEntryVectorSize() << std::endl;
            // Uncomment the following to print all entries
            for (int m = 0; m < page.getRangeEntryVectorSize(); m++)
            {
              std::cout << "\t\t\t\t(Range) Entry : " << m << " (Range_Key : " << page.getIthRangeEntry(m).getStartKey()
                        << ", Range_Key : " << page.getIthRangeEntry(m).getEndKey() << ")" << std::endl;
            }
          }
        }
      }
      moving_head = moving_head->getNextFilePtr();
    }
  }
  std::cout << "\n**********************************************************************************************************************************\n" << std::endl;
  return 1;
}

int DiskMetaFile::printAllLevelRangeDeleteFilter(){
  std::cout << "*************************************************** PRINTING ALL LEVEL RANGE DELETE FILTER *********************************************************\n" << std::endl;
  int total_range_delete_count = 0;
  int total_range_delete_count_without_last_level = 0;
  for (int i = 0; i < per_level_range_delete_filter.size(); i++){
    std::cout << "level " << i << " range delete filter: " << std::endl;
    per_level_range_delete_filter[i].printRangeDeleteList();
    std::cout << std::endl;
    total_range_delete_count_without_last_level = total_range_delete_count;
    total_range_delete_count += per_level_range_delete_filter[i].getRangeDeleteCount();
  }
  std::cout << endl;
  std::cout << "Total range delete count: " << total_range_delete_count << std::endl;
  std::cout << "Total range delete count without last level: " << total_range_delete_count_without_last_level << std::endl;
  std::cout << "\n**********************************************************************************************************************************\n" << std::endl;
  return 1;
}

int DiskMetaFile::getTotalRangeDeleteCountOfPerLevelRangeDeleteFilter(){
  int total_range_delete_count = 0;
  for (int i = 0; i < per_level_range_delete_filter.size(); i++){
    total_range_delete_count += per_level_range_delete_filter[i].getRangeDeleteCount();
  }
  return total_range_delete_count;
}
int DiskMetaFile::getTotalRangeDeleteWithoutLastLevelCountOfPerLevelRangeDeleteFilter(){
  int total_range_delete_count = 0;
  for (int i = 0; i < per_level_range_delete_filter.size() - 1; i++){
    total_range_delete_count += per_level_range_delete_filter[i].getRangeDeleteCount();
  }
  return total_range_delete_count;
}

int DiskMetaFile::printAllLevelSplitRangeDeleteFilter(){
  std::cout << "*************************************************** PRINTING ALL LEVEL SPLIT RANGE DELETE FILTER *********************************************************\n" << std::endl;
  int total_range_delete_count = 0;
  int total_range_delete_count_without_last_level = 0;
  for (int i = 0; i < per_level_range_delete_filter_split.size(); i++){
    std::cout << "level " << i << " range delete filter: " << std::endl;
    per_level_range_delete_filter_split[i].printRangeDeleteList();
    std::cout << std::endl;
    total_range_delete_count_without_last_level = total_range_delete_count;
    total_range_delete_count += per_level_range_delete_filter_split[i].getRangeDeleteCount();
  }
  std::cout << endl;
  std::cout << "Total range delete count: " << total_range_delete_count << std::endl;
  std::cout << "Total range delete count without last level: " << total_range_delete_count_without_last_level << std::endl;
  std::cout << "\n**********************************************************************************************************************************\n" << std::endl;
  return 1;
}


int DiskMetaFile::getTotalRangeDeleteCountOfSplitPerLevelRangeDeleteFilter(){
  int total_range_delete_count = 0;
  for (int i = 0; i < per_level_range_delete_filter_split.size(); i++){
    total_range_delete_count += per_level_range_delete_filter_split[i].getRangeDeleteCount();
  }
  return total_range_delete_count;
}
int DiskMetaFile::getTotalRangeDeleteWithoutLastLevelCountOfSplitPerLevelRangeDeleteFilter(){
  int total_range_delete_count = 0;
  for (int i = 0; i < per_level_range_delete_filter_split.size() - 1; i++){
    total_range_delete_count += per_level_range_delete_filter_split[i].getRangeDeleteCount();
  }
  return total_range_delete_count;
}


int DiskMetaFile::printSkylineRangeDeleteFilter(){
  int total_range_delete_count = 0;
  std::cout << "*************************************************** PRINTING SKYLINE RANGE DELETE FILTER *********************************************************\n" << std::endl;
  skyline_range_delete_filter.printRangeDeleteWithTimetagList();
  total_range_delete_count += skyline_range_delete_filter.getRangeDeleteWithTimetagCount();
  std::cout << endl;
  std::cout << "Total range delete count: " << total_range_delete_count << std::endl;
  std::cout << "\n**********************************************************************************************************************************\n" << std::endl;
  return 1;
}

int DiskMetaFile::getTotalRangeDeleteCountOfSkylineRangeDeleteFilter(){
  return skyline_range_delete_filter.getRangeDeleteWithTimetagCount();
}




int DiskMetaFile::clearAllEntries()
{
  cout << "Clearing previously created tree..." << endl;
  EmuEnv *_env = EmuEnv::getInstance();

  MemoryBuffer::getBuffer().clear();
  MemoryBuffer::current_buffer_entry_count = 0;
  MemoryBuffer::current_buffer_saturation = 0;
  MemoryBuffer::current_buffer_size = 0;
  MemoryBuffer::buffer_flush_count = 0;

  for (int i = 1; i <= 32; i++)
  {
    SSTFile *level_i_head = DiskMetaFile::getSSTFileHead(i);
    SSTFile *moving_head = level_i_head;
    SSTFile *moving_head_prev = level_i_head;

    while (moving_head)
    {
      moving_head_prev = moving_head;
      for (int k = 0; k < moving_head->getPointDeleteTileCount(); k++)
      {
        DeleteTile& delete_tile = moving_head->getIthPointDeleteTile(k);
 
        for (int l = 0; l < delete_tile.getPointPageVectorSize(); l++)
        {
          Page& page = delete_tile.getIthPointPage(l);
          page.clearAllVectors();
          page.setAllKeysToDefaultNegativeOne();
        }
        delete_tile.clearAllVectors();
        delete_tile.setAllKeysToDefaultNegativeOne();

      }
      moving_head->clearAllVectors();
      moving_head->setAllKeysToDefaultNegativeOne();


      moving_head = moving_head->getNextFilePtr();
      moving_head_prev->setNextFilePtr(NULL);
      free(moving_head_prev);
    }
    DiskMetaFile::setSSTFileHead(NULL, i);
  }
  return 1;
}



void DiskMetaFile::addRangeDeleteToRangeDeleteFilterAtLevel(int level, vector<RangeEntry> range_delete_list){ 
  if(level > per_level_range_delete_filter.size() ){
    cout << "Error: Level " << level << " is out of range (> +1). max level of current Range Delete Filter size is " << per_level_range_delete_filter.size() << endl;
    exit(1);
  }
  if(level == per_level_range_delete_filter.size() ){
    per_level_range_delete_filter.push_back(PL_RDF());
  }


  per_level_range_delete_filter[level].addRangeDelete(range_delete_list);
}

void DiskMetaFile::removeRangeDeleteFromRangeDeleteFilterAtLevel(int level, vector<RangeEntry> range_delete_discard_list){
  if(level > per_level_range_delete_filter.size() ){
    cout << "Error: Level " << level << " is out of range (> +1). max level of current Range Delete Filter size is " << per_level_range_delete_filter.size() << endl;
    exit(1);
  }
  if(level == per_level_range_delete_filter.size() ){
    per_level_range_delete_filter.push_back(PL_RDF());
  }

  per_level_range_delete_filter[level].removeRangeDelete(range_delete_discard_list);
}


bool DiskMetaFile::isAliveThroughRangeDeleteFilterAtLevel(int level, long key){
  if(level >= per_level_range_delete_filter.size() ){
    cout << "Error: Level " << level << " is out of range. max level of current Range Delete Filter is " << (per_level_range_delete_filter.size()-1) << endl;
    exit(1);
  }
  if(level <= 0){
    cout << "Error: Level " << level << " is out of range. min level of current Range Delete Filter is 1" << endl;
    exit(1);
  }

  return per_level_range_delete_filter[level].isEntryAlive(key);
}


void DiskMetaFile::addRangeDeleteToSplitRangeDeleteFilterAtLevel(int level, vector<RangeEntry> range_delete_list){ 
  if(level > per_level_range_delete_filter_split.size() ){
    cout << "Error: Level " << level << " is out of range (> +1). max level of current Range Delete Filter size is " << per_level_range_delete_filter_split.size() << endl;
    exit(1);
  }
  if(level == per_level_range_delete_filter_split.size() ){
    per_level_range_delete_filter_split.push_back(PL_RDF());
  }


  per_level_range_delete_filter_split[level].addRangeDelete(range_delete_list);
}

void DiskMetaFile::removeRangeDeleteFromSplitRangeDeleteFilterAtLevel(int level, vector<RangeEntry> range_delete_discard_list){
  if(level > per_level_range_delete_filter_split.size() ){
    cout << "Error: Level " << level << " is out of range (> +1). max level of current Range Delete Filter size is " << per_level_range_delete_filter_split.size() << endl;
    exit(1);
  }
  if(level == per_level_range_delete_filter_split.size() ){
    per_level_range_delete_filter_split.push_back(PL_RDF());
  }

  per_level_range_delete_filter_split[level].removeRangeDelete(range_delete_discard_list);
}


bool DiskMetaFile::isAliveThroughSplitRangeDeleteFilterAtLevel(int level, long key){
  if(level >= per_level_range_delete_filter_split.size() ){
    cout << "Error: Level " << level << " is out of range. max level of current Range Delete Filter is " << (per_level_range_delete_filter_split.size()-1) << endl;
    exit(1);
  }
  if(level <= 0){
    cout << "Error: Level " << level << " is out of range. min level of current Range Delete Filter is 1" << endl;
    exit(1);
  }

  return per_level_range_delete_filter_split[level].isEntryAlive(key);
}



//all the entries on and below the level are deleted
void DiskMetaFile::splitRangesInSplitRangeDeleteFilterAtLevelByEntryList(int level, EntryList &entrylist){
  if(level >= per_level_range_delete_filter_split.size()){return;}
  vector<PointEntry> vector_of_points = entrylist.get_point_entry_vector();

  per_level_range_delete_filter_split[level].splitRangeByPointEntries(vector_of_points);


  return;


}







void  DiskMetaFile::addRangeDeleteToSkylineRangeDeleteFilter(long start_key, long end_key, long timestamp){
  skyline_range_delete_filter.addRangeDeleteWithTimetag(start_key, end_key, timestamp);
}

bool DiskMetaFile::isAliveThroughSkylineRangeDeleteFilter(long key, long timestamp){
  return skyline_range_delete_filter.isEntryAliveWithTimetag(key, timestamp);
}
