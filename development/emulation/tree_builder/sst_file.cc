#include "sst_file.h"

using namespace std;
using namespace tree_builder;
using namespace workload_exec;


SSTFile::SSTFile() : min_sort_key(-1), max_sort_key(-1), min_delete_key(-1), max_delete_key(-1),  min_range_key(-1), max_range_key(-1)
                    ,next_file_ptr(NULL) {}
            // , sst_file_type(SSTFile::NOT_SET_YET)  {}
            // , sst_file_type(SSTFile::POINT)  {}



SSTFile& SSTFile::setFileLevel(int file_level){
  this->file_level = file_level;
  return *this;
}
SSTFile& SSTFile::setFileId(string file_id){
  this->file_id = file_id;
  return *this;
}

int SSTFile::getFileLevel(){
  return this->file_level;
}
string SSTFile::getFileId(){
  return this->file_id;
}


SSTFile& SSTFile::setNextFilePtr(SSTFile* next_file_ptr){
  this->next_file_ptr = next_file_ptr;
  return *this;
}
SSTFile* SSTFile::getNextFilePtr(){
  return this->next_file_ptr;
}


SSTFile& SSTFile::setMinSortKey(long min_sort_key){
  // if(this->getSSTFileType() != SSTFile::POINT){
  //   cout << "ERROR: setMinSortKey() called on a non-POINT SSTFile" << endl;
  //   exit(1);
  // }
  this->min_sort_key = min_sort_key;
  return *this;
}
SSTFile& SSTFile::setMaxSortKey(long max_sort_key){
  // if(this->getSSTFileType() != SSTFile::POINT){
  //   cout << "ERROR: setMaxSortKey() called on a non-POINT SSTFile" << endl;
  //   exit(1);
  // }
  this->max_sort_key = max_sort_key;
  return *this;
}
SSTFile& SSTFile::setMinDeleteKey(long min_delete_key){
  // if(this->getSSTFileType() != SSTFile::POINT){
  //   cout << "ERROR: setMinDeleteKey() called on a non-POINT SSTFile" << endl;
  //   exit(1);
  // }
  this->min_delete_key = min_delete_key;
  return *this;
}
SSTFile& SSTFile::setMaxDeleteKey(long max_delete_key){
  // if(this->getSSTFileType() != SSTFile::POINT){
  //   cout << "ERROR: setMaxDeleteKey() called on a non-POINT SSTFile" << endl;
  //   exit(1);
  // }
  this->max_delete_key = max_delete_key;
  return *this;
}
SSTFile& SSTFile::setMinRangeKey(long min_range_key){
  // if(this->getSSTFileType() != SSTFile::RANGE){
  //   cout << "ERROR: setMinRangeKey() called on a non-RANGE SSTFile" << endl;
  //   exit(1);
  // }
  this->min_range_key = min_range_key;
  return *this;
}
SSTFile& SSTFile::setMaxRangeKey(long max_range_key){
  // if(this->getSSTFileType() != SSTFile::RANGE){
  //   cout << "ERROR: setMaxRangeKey() called on a non-RANGE SSTFile" << endl;
  //   exit(1);
  // }
  this->max_range_key = max_range_key;
  return *this;
}

long SSTFile::getMinSortKey(){
  // if(this->getSSTFileType() != SSTFile::POINT){
  //   cout << "ERROR: getMinSortKey() called on a non-POINT SSTFile" << endl;
  //   exit(1);
  // }
  return this->min_sort_key;
}
long SSTFile::getMaxSortKey(){
  // if(this->getSSTFileType() != SSTFile::POINT){
  //   cout << "ERROR: getMaxSortKey() called on a non-POINT SSTFile" << endl;
  //   exit(1);
  // }
  return this->max_sort_key;
}
long SSTFile::getMinDeleteKey(){
  // if(this->getSSTFileType() != SSTFile::POINT){
  //   cout << "ERROR: getMinDeleteKey() called on a non-POINT SSTFile" << endl;
  //   exit(1);
  // }
  return this->min_delete_key;
}
long SSTFile::getMaxDeleteKey(){
  // if(this->getSSTFileType() != SSTFile::POINT){
  //   cout << "ERROR: getMaxDeleteKey() called on a non-POINT SSTFile" << endl;
  //   exit(1);
  // }
  return this->max_delete_key;
}
long SSTFile::getMinRangeKey(){
  // if(this->getSSTFileType() != SSTFile::RANGE){
  //   cout << "ERROR: getMinRangeKey() called on a non-RANGE SSTFile" << endl;
  //   exit(1);
  // }
  return this->min_range_key;
}
long SSTFile::getMaxRangeKey(){
  // if(this->getSSTFileType() != SSTFile::RANGE){
  //   cout << "ERROR: getMaxRangeKey() called on a non-RANGE SSTFile" << endl;
  //   exit(1);
  // }
  return this->max_range_key;
}


SSTFile& SSTFile::addPointDeleteTile(DeleteTile delete_tile){
  // if(this->getSSTFileType() != SSTFile::POINT){
  //   cout << "ERROR: addPointDeleteTile() called on a non-POINT SSTFile" << endl;
  //   exit(1);
  // }

  long delete_tile_min_sort_key = delete_tile.getMinSortKey();
  long delete_tile_max_sort_key = delete_tile.getMaxSortKey();
  long min_sort_key = this->getMinSortKey();
  long max_sort_key = this->getMaxSortKey();

  if(min_sort_key == -1 || delete_tile_min_sort_key < min_sort_key){
    this->setMinSortKey(delete_tile_min_sort_key);
  }
  if(max_sort_key == -1 || delete_tile_max_sort_key > max_sort_key){
    this->setMaxSortKey(delete_tile_max_sort_key);
  }

  long delete_tile_min_delete_key = delete_tile.getMinDeleteKey();
  long delete_tile_max_delete_key = delete_tile.getMaxDeleteKey();
  long min_delete_key = this->getMinDeleteKey();
  long max_delete_key = this->getMaxDeleteKey();

  if(min_delete_key == -1 || delete_tile_min_delete_key < min_delete_key){
    this->setMinDeleteKey(delete_tile_min_delete_key);
  }
  if(max_delete_key == -1 || delete_tile_max_delete_key > max_delete_key){
    this->setMaxDeleteKey(delete_tile_max_delete_key);
  }

  this->point_delete_tile_vector.push_back(delete_tile);
  return *this;
}
SSTFile& SSTFile::addRangeDeleteTile(DeleteTile delete_tile){
  // if(this->getSSTFileType() != SSTFile::RANGE){
  //   cout << "ERROR: addRangeDeleteTile() called on a non-RANGE SSTFile" << endl;
  //   exit(1);
  // }

  long delete_tile_min_range_key = delete_tile.getMinRangeKey();
  long delete_tile_max_range_key = delete_tile.getMaxRangeKey();
  long min_range_key = this->getMinRangeKey();
  long max_range_key = this->getMaxRangeKey();

  if(min_range_key == -1 || delete_tile_min_range_key < min_range_key){
    this->setMinRangeKey(delete_tile_min_range_key);
  }
  if(max_range_key == -1 || delete_tile_max_range_key > max_range_key){
    this->setMaxRangeKey(delete_tile_max_range_key);
  }

  this->range_delete_tile_vector.push_back(delete_tile);
  return *this;
}
SSTFile& SSTFile::setPointDeleteTileVector(vector<DeleteTile> point_delete_tile_vector){
  // if(this->getSSTFileType() != SSTFile::POINT){
  //   cout << "ERROR: setPointDeleteTileVector() called on a non-POINT SSTFile" << endl;
  //   exit(1);
  // }
  for(DeleteTile &delete_tile: point_delete_tile_vector){
    long delete_tile_min_sort_key = delete_tile.getMinSortKey();
    long delete_tile_max_sort_key = delete_tile.getMaxSortKey();
    long min_sort_key = this->getMinSortKey();
    long max_sort_key = this->getMaxSortKey();

    if(min_sort_key == -1 || delete_tile_min_sort_key < min_sort_key){
      this->setMinSortKey(delete_tile_min_sort_key);
    }
    if(max_sort_key == -1 || delete_tile_max_sort_key > max_sort_key){
      this->setMaxSortKey(delete_tile_max_sort_key);
    }

    long delete_tile_min_delete_key = delete_tile.getMinDeleteKey();
    long delete_tile_max_delete_key = delete_tile.getMaxDeleteKey();
    long min_delete_key = this->getMinDeleteKey();
    long max_delete_key = this->getMaxDeleteKey();

    if(min_delete_key == -1 || delete_tile_min_delete_key < min_delete_key){
      this->setMinDeleteKey(delete_tile_min_delete_key);
    }
    if(max_delete_key == -1 || delete_tile_max_delete_key > max_delete_key){
      this->setMaxDeleteKey(delete_tile_max_delete_key);
    }
  }

  this->point_delete_tile_vector = point_delete_tile_vector;
  return *this;
}
SSTFile& SSTFile::setRangeDeleteTileVector(vector<DeleteTile> range_delete_tile_vector){
  // if(this->getSSTFileType() != SSTFile::RANGE){
  //   cout << "ERROR: setRangeDeleteTileVector() called on a non-RANGE SSTFile" << endl;
  //   exit(1);
  // }
  for(DeleteTile &delete_tile: range_delete_tile_vector){
    long delete_tile_min_range_key = delete_tile.getMinRangeKey();
    long delete_tile_max_range_key = delete_tile.getMaxRangeKey();
    long min_range_key = this->getMinRangeKey();
    long max_range_key = this->getMaxRangeKey();

    if(min_range_key == -1 || delete_tile_min_range_key < min_range_key){
      this->setMinRangeKey(delete_tile_min_range_key);
    }
    if(max_range_key == -1 || delete_tile_max_range_key > max_range_key){
      this->setMaxRangeKey(delete_tile_max_range_key);
    }
  }
  this->range_delete_tile_vector = range_delete_tile_vector;
  return *this;
}
vector<DeleteTile> SSTFile::getPointDeleteTileVector(){
  // if(this->getSSTFileType() != SSTFile::POINT){
  //   cout << "ERROR: getPointDeleteTileVector() called on a non-POINT SSTFile" << endl;
  //   exit(1);
  // }
  return this->point_delete_tile_vector;
}
vector<DeleteTile> SSTFile::getRangeDeleteTileVector(){
  // if(this->getSSTFileType() != SSTFile::RANGE){
  //   cout << "ERROR: getRangeDeleteTileVector() called on a non-RANGE SSTFile" << endl;
  //   exit(1);
  // }
  return this->range_delete_tile_vector;
}

long SSTFile::getPointDeleteTileCount(){
  // if(this->getSSTFileType() != SSTFile::POINT){
  //   cout << "ERROR: getPointDeleteTileCount() called on a non-POINT SSTFile" << endl;
  //   exit(1);
  // }
  return this->point_delete_tile_vector.size();
}

long SSTFile::getRangeDeleteTileCount(){
  // if(this->getSSTFileType() != SSTFile::RANGE){
  //   cout << "ERROR: getRangeDeleteTileCount() called on a non-RANGE SSTFile" << endl;
  //   exit(1);
  // }
  return this->range_delete_tile_vector.size();
}

DeleteTile& SSTFile::getIthPointDeleteTile(int i){
  // if(this->getSSTFileType() != SSTFile::POINT){
  //   cout << "ERROR: getIthPointDeleteTile() called on a non-POINT SSTFile" << endl;
  //   exit(1);
  // }
  return this->point_delete_tile_vector[i];
}

DeleteTile& SSTFile::getIthRangeDeleteTile(int i){
  // if(this->getSSTFileType() != SSTFile::RANGE){
  //   cout << "ERROR: getIthRangeDeleteTile() called on a non-RANGE SSTFile" << endl;
  //   exit(1);
  // }
  return this->range_delete_tile_vector[i];
}

void SSTFile::clearPointDeleteTileVector(){
  // if(this->getSSTFileType() != SSTFile::POINT){
  //   cout << "ERROR: clearPointDeleteTileVector() called on a non-POINT SSTFile" << endl;
  //   exit(1);
  // }
  this->point_delete_tile_vector.clear();
}

void SSTFile::clearRangeDeleteTileVector(){
  // if(this->getSSTFileType() != SSTFile::RANGE){
  //   cout << "ERROR: clearRangeDeleteTileVector() called on a non-RANGE SSTFile" << endl;
  //   exit(1);
  // }
  this->range_delete_tile_vector.clear();
}

void SSTFile::clearAllVectors(){
  this->point_delete_tile_vector.clear();
  this->range_delete_tile_vector.clear();
}

void SSTFile::setAllKeysToDefaultNegativeOne(){
  this->min_sort_key = -1;
  this->max_sort_key = -1;
  this->min_delete_key = -1;
  this->max_delete_key = -1;
  this->min_range_key = -1;
  this->max_range_key = -1;
}

void SSTFile::sortPointDeleteTileVectorOnMinSortKeyInAscendingOrder(){
  // if(this->getSSTFileType() != SSTFile::POINT){
  //   cout << "ERROR: sortPointDeleteTileVectorOnSortKeyInAscendingOrder() called on a non-POINT SSTFile" << endl;
  //   exit(1);
  // }
  sort(this->point_delete_tile_vector.begin(), this->point_delete_tile_vector.end(), [&](DeleteTile &a, DeleteTile &b){
                                                                              return a.getMinSortKey() < b.getMinSortKey();
  });
}

void SSTFile::sortPointDeleteTileVectorOnMinDeleteKeyInAscendingOrder(){
  // if(this->getSSTFileType() != SSTFile::POINT){
  //   cout << "ERROR: sortPointDeleteTileVectorOnMinDeleteKeyInAscendingOrder() called on a non-POINT SSTFile" << endl;
  //   exit(1);
  // }
  sort(this->point_delete_tile_vector.begin(), this->point_delete_tile_vector.end(), [&](DeleteTile &a, DeleteTile &b){
                                                                              return a.getMinDeleteKey() < b.getMinDeleteKey();
  });
}

void SSTFile::sortRangeDeleteTileVectorOnMinRangeKeyInAscendingOrder(){
  // if(this->getSSTFileType() != SSTFile::RANGE){
  //   cout << "ERROR: sortRangeDeleteTileVectorOnStartKeyInAscendingOrder() called on a non-RANGE SSTFile" << endl;
  //   exit(1);
  // }
  sort(this->range_delete_tile_vector.begin(), this->range_delete_tile_vector.end(), [&](DeleteTile &a, DeleteTile &b){
                                                                              return a.getMinRangeKey() < b.getMinRangeKey();
  });
}

SSTFile *SSTFile::createNewSSTFile_RDF(int level_to_flush_in, int point_deletetile_count, int range_delete_tile_count)
{
  EmuEnv *_env = EmuEnv::getInstance();

  SSTFile *new_file = new SSTFile();
  new_file->setAllKeysToDefaultNegativeOne();

  int delete_tile_count_in_a_file = ceil((_env->buffer_size_in_pages * 1.0) / (_env->getDeleteTileSize(level_to_flush_in) * 1.0));

// ****************************************************************************************************************************************
  new_file->setPointDeleteTileVector(DeleteTile::createNewPointDeleteTiles(point_deletetile_count, level_to_flush_in));
  new_file->setRangeDeleteTileVector(DeleteTile::createNewRangeDeleteTiles(range_delete_tile_count, level_to_flush_in));
// ****************************************************************************************************************************************  
  new_file->file_level = level_to_flush_in;
  new_file->next_file_ptr = NULL;
  DiskMetaFile::global_level_file_counter[level_to_flush_in]++;
  new_file->file_id = "L" + std::to_string(level_to_flush_in) + "F" + std::to_string(DiskMetaFile::global_level_file_counter[level_to_flush_in]);

  return new_file;
}





// SSTFile *SSTFile::createNewSSTFile(int level_to_flush_in)
// {
//   EmuEnv *_env = EmuEnv::getInstance();

//   SSTFile *new_file = new SSTFile();
//   new_file->setAllKeysToDefaultNegativeOne();

//   int delete_tile_count_in_a_file = ceil((_env->buffer_size_in_pages * 1.0) / (_env->getDeleteTileSize(level_to_flush_in) * 1.0));


//   new_file->setPointDeleteTileVector(DeleteTile::createNewPointDeleteTiles(delete_tile_count_in_a_file , level_to_flush_in));


//   new_file->file_level = level_to_flush_in;
//   new_file->next_file_ptr = NULL;
//   DiskMetaFile::global_level_file_counter[level_to_flush_in]++;
//   new_file->file_id = "L" + std::to_string(level_to_flush_in) + "F" + std::to_string(DiskMetaFile::global_level_file_counter[level_to_flush_in]);

//   return new_file;
// }

int SSTFile::generateMetaData(SSTFile *file, DeleteTile &deletetile, Page &page, long sort_key_to_insert, long delete_key_to_insert)
{
  if (file->min_sort_key < 0 || sort_key_to_insert < file->min_sort_key)
  {
    file->min_sort_key = sort_key_to_insert;
  }

  if (file->max_sort_key < 0 || sort_key_to_insert > file->max_sort_key)
  {
    file->max_sort_key = sort_key_to_insert;
  }

  if (file->min_delete_key < 0 || delete_key_to_insert < file->min_delete_key)
  {
    file->min_delete_key = delete_key_to_insert;
  }

  if (file->max_delete_key < 0 || delete_key_to_insert > file->max_delete_key)
  {
    file->max_delete_key = delete_key_to_insert;
  }

  if (deletetile.getMinSortKey() < 0 || (sort_key_to_insert < deletetile.getMinSortKey() && sort_key_to_insert > 0) )
  {
    deletetile.setMinSortKey(sort_key_to_insert);
  }

  if (deletetile.getMaxSortKey() < 0 || (sort_key_to_insert > deletetile.getMaxSortKey() && sort_key_to_insert > 0))
  {
    deletetile.setMaxSortKey(sort_key_to_insert);
  }

  if (deletetile.getMinDeleteKey() < 0 || (delete_key_to_insert < deletetile.getMinDeleteKey() && delete_key_to_insert > 0))
  {
    deletetile.setMinDeleteKey(delete_key_to_insert);
  }

  if (deletetile.getMaxDeleteKey() < 0 || (delete_key_to_insert > deletetile.getMaxDeleteKey() && delete_key_to_insert > 0))
  {
    deletetile.setMaxDeleteKey(delete_key_to_insert);
  }

  if (page.getMinSortKey() < 0 || sort_key_to_insert < page.getMinSortKey())
  {
    page.setMinSortKey(sort_key_to_insert);
  }

  if (page.getMaxSortKey() < 0 || sort_key_to_insert > page.getMaxSortKey())
  {
    page.setMaxSortKey(sort_key_to_insert);
  }
  if (page.getMinDeleteKey() < 0 || delete_key_to_insert < page.getMinDeleteKey())
  {
    page.setMinDeleteKey(delete_key_to_insert);
  }

  if (page.getMaxDeleteKey() < 0 || delete_key_to_insert > page.getMaxDeleteKey())
  {
    page.setMaxDeleteKey(delete_key_to_insert);
  }
  return 1;
}


int SSTFile::generateRangeMetaData(SSTFile *file, DeleteTile &deletetile, Page &page, long start_key_to_insert, long end_key_to_insert)
{
  if (file->getMinRangeKey() < 0 || start_key_to_insert < file->getMinRangeKey()) {
    file->setMinRangeKey(start_key_to_insert);
  }

  if (file->getMaxRangeKey() < 0 || end_key_to_insert > file->getMaxRangeKey()) {
    file->setMaxRangeKey(end_key_to_insert);
  }

  if (deletetile.getMinRangeKey() < 0 || start_key_to_insert < deletetile.getMinRangeKey()) {
    deletetile.setMinRangeKey(start_key_to_insert);
  }

    if (deletetile.getMaxRangeKey() < 0 || end_key_to_insert > deletetile.getMaxRangeKey()) {
    deletetile.setMaxRangeKey(end_key_to_insert);
  }

  if (page.getMinRangeKey() < 0 || start_key_to_insert < page.getMinRangeKey()) {
    page.setMinRangeKey(start_key_to_insert);
  }

    if (page.getMaxRangeKey() < 0 || end_key_to_insert > page.getMaxRangeKey()) {
    page.setMaxRangeKey(end_key_to_insert);
  }

  return 1;
}



// int SSTFile::PopulateDeleteTile(SSTFile *file, vector<pair<pair<long, long>, string>> vector_to_populate_tile, int deletetileid, int level_to_flush_in)
// {
//   if (MemoryBuffer::verbosity == 2)
//     std::cout << "In PopulateDeleteTile() ... " << std::endl;

//   EmuEnv *_env = EmuEnv::getInstance();
//   int page_count;
//   page_count = file->getIthPointDeleteTile(deletetileid).getPointPageVectorSize();

//   int entries_per_page = _env->entries_per_page;
//   for (int i = 0; i < page_count; ++i)
//   {
//     vector<pair<pair<long, long>, string>> vector_to_populate_page;
//     entries_per_page = Utility::minInt(entries_per_page, vector_to_populate_tile.size());
//     for (int j = 0; j < entries_per_page; ++j)
//     {
//       vector_to_populate_page.push_back(vector_to_populate_tile[j]);
//     }

//     std::sort(vector_to_populate_page.begin(), vector_to_populate_page.end(), Utility::sortbysortkey);

//     if (MemoryBuffer::verbosity == 2)
//     {
//       std::cout << "\npopulating pages :: vector length = " << vector_to_populate_tile.size() << std::endl;
//       std::cout << "\nEntries per page = " << entries_per_page << std::endl;
//     }

//     for (int j = 0; j < entries_per_page; ++j)
//     {
//       long sort_key_to_insert = vector_to_populate_page[j].first.first;
//       long delete_key_to_insert = vector_to_populate_page[j].first.second;

//       Page &retrieved_page = file->getIthPointDeleteTile(deletetileid).getIthPointPage(i);
//       DeleteTile &deletetile = file->getIthPointDeleteTile(deletetileid);
//       int status = SSTFile::generateMetaData(file, deletetile,
//                                     retrieved_page, sort_key_to_insert, delete_key_to_insert);

//       PointEntry *point_entry = new PointEntry(vector_to_populate_page[j], -1/*TODO: TIMETAG*/);
      
//       file->getIthPointDeleteTile(deletetileid).getIthPointPage(i).addPointEntry(*point_entry);                                                                            

//       if (MemoryBuffer::verbosity == 2)
//         std::cout << vector_to_populate_page[j].first.first << " " << vector_to_populate_page[j].first.second << std::endl;
//     }

//     vector_to_populate_tile.erase(vector_to_populate_tile.begin(), vector_to_populate_tile.begin() + entries_per_page);

//     if (MemoryBuffer::verbosity == 2)
//       std::cout << "populated pages\n"
//                 << std::endl;

//     vector_to_populate_page.clear();
//   }

//   return 1;
// }

// int SSTFile::PopulateFile(SSTFile *file, vector<pair<pair<long, long>, string>> vector_to_populate_file, int level_to_flush_in)
// {
//   EmuEnv *_env = EmuEnv::getInstance();
//   int delete_tile_count = ceil((_env->buffer_size_in_pages * 1.0) / (_env->getDeleteTileSize(level_to_flush_in) * 1.0));
//   int entries_per_delete_tile = _env->getDeleteTileSize(level_to_flush_in) * _env->entries_per_page;

//   if (MemoryBuffer::verbosity == 2)
//     std::cout << "In PopulateFile() ... " << std::endl;

//   for (int i = 0; i < delete_tile_count; i++)
//   {
//     vector<pair<pair<long, long>, string>> vector_to_populate_tile;
//     entries_per_delete_tile = Utility::minInt(entries_per_delete_tile, vector_to_populate_file.size());
//     for (int j = 0; j < entries_per_delete_tile; ++j)
//     {
//       vector_to_populate_tile.push_back(vector_to_populate_file[j]);
//     }
//     std::sort(vector_to_populate_tile.begin(), vector_to_populate_tile.end(), Utility::sortbydeletekey);

//     vector_to_populate_file.erase(vector_to_populate_file.begin(), vector_to_populate_file.begin() + entries_per_delete_tile);

//     if (MemoryBuffer::verbosity == 2)
//       std::cout << "populating delete tile ... \n";
//     int status = SSTFile::PopulateDeleteTile(file, vector_to_populate_tile, i, level_to_flush_in);
//     vector_to_populate_tile.clear();
//   }
//   return 1;
// }


int SSTFile::PopulateFile_RDF(SSTFile *file, EntryList entrylist_to_populate_file, int level_to_flush_in)
{
  EmuEnv *_env = EmuEnv::getInstance();

  int point_deletetile_count = ceil((entrylist_to_populate_file.get_point_entry_vector_size_inByte() * 1.0)/(_env->delete_tile_size * 1.0));
  int range_deletetile_count = ceil((entrylist_to_populate_file.get_range_entry_vector_size_inByte() * 1.0)/(_env->delete_tile_size * 1.0));
  int PointEntries_per_delete_tile = _env->delete_tile_size / _env->per_kv_size;
  int RangeEntries_per_delete_tile = _env->delete_tile_size / _env->per_range_delete_size;

  vector<PointEntry> PointEntry_vector_to_populate_file = entrylist_to_populate_file.get_point_entry_vector();
  vector<RangeEntry> RangeEntry_vector_to_populate_file = entrylist_to_populate_file.get_range_entry_vector();

  // populate point deletetile
  for (int i = 0; i < point_deletetile_count; i++)
  {
    vector<PointEntry> PointEntries_to_populate_tile;
    PointEntries_per_delete_tile = Utility::minInt(PointEntries_per_delete_tile, PointEntry_vector_to_populate_file.size());
    for (int j = 0; j < PointEntries_per_delete_tile; ++j)
    {
      PointEntries_to_populate_tile.push_back(PointEntry_vector_to_populate_file[j]);
    }
    PointEntry_vector_to_populate_file.erase(PointEntry_vector_to_populate_file.begin(), PointEntry_vector_to_populate_file.begin() + PointEntries_per_delete_tile);

    int status = SSTFile::Populate_Point_DeleteTile_RDF(file, PointEntries_to_populate_tile, i, level_to_flush_in); 
  }

  // populate range deletetile
  for (int i = 0; i < range_deletetile_count; i++)
  {
    vector<RangeEntry> RangeEntries_to_populate_tile;
    RangeEntries_per_delete_tile = Utility::minInt(RangeEntries_per_delete_tile, RangeEntry_vector_to_populate_file.size());
    for (int j = 0; j < RangeEntries_per_delete_tile; ++j)
    {
      RangeEntries_to_populate_tile.push_back(RangeEntry_vector_to_populate_file[j]);
    }
    RangeEntry_vector_to_populate_file.erase(RangeEntry_vector_to_populate_file.begin(), RangeEntry_vector_to_populate_file.begin() + RangeEntries_per_delete_tile);

    int status = SSTFile::Populate_Range_DeleteTile_RDF(file, RangeEntries_to_populate_tile, i, level_to_flush_in); 
  }

  entrylist_to_populate_file.clear();
  return 1;
}

int SSTFile::Populate_Point_DeleteTile_RDF(SSTFile *file, vector<PointEntry> PointEntries_to_populate_tile, int deletetileid, int level_to_flush_in)
{
  EmuEnv *_env = EmuEnv::getInstance();
  int page_count = file->getIthPointDeleteTile(deletetileid).getPointPageVectorSize();
  
  int PointEntries_per_page = _env->Pointentries_per_page;
  for (int i = 0; i < page_count; ++i)
  {
    vector<PointEntry> PointEntries_to_populate_page;
    PointEntries_per_page = Utility::minInt(PointEntries_per_page, PointEntries_to_populate_tile.size());
    for (int j = 0; j < PointEntries_per_page; ++j)
    {
      PointEntries_to_populate_page.push_back(PointEntries_to_populate_tile[j]);
    }
    
    for (int j = 0; j < PointEntries_per_page; ++j)
    {
      long sort_key_to_insert = PointEntries_to_populate_page[j].getSortKey();
      long delete_key_to_insert = PointEntries_to_populate_page[j].getDeleteKey();

      Page &retrieved_page = file->getIthPointDeleteTile(deletetileid).getIthPointPage(i);
      DeleteTile &deletetile = file->getIthPointDeleteTile(deletetileid);
      int status = SSTFile::generateMetaData(file, deletetile,
                                    retrieved_page, sort_key_to_insert, delete_key_to_insert);
      PointEntry *point_entry = new PointEntry(PointEntries_to_populate_page[j].get_keyvalue_pair() , -1/*TODO: TIMETAG*/);
                                                                         
      file->getIthPointDeleteTile(deletetileid).getIthPointPage(i).addPointEntry(*point_entry);                                                                            
    }

    PointEntries_to_populate_tile.erase(PointEntries_to_populate_tile.begin(), PointEntries_to_populate_tile.begin() + PointEntries_per_page);

    PointEntries_to_populate_page.clear();
  }

  return 1;
}


int SSTFile::Populate_Range_DeleteTile_RDF(SSTFile *file, vector<RangeEntry> RangeEntries_to_populate_tile, int deletetileid, int level_to_flush_in)
{
  EmuEnv *_env = EmuEnv::getInstance();
  int page_count = file->getIthRangeDeleteTile(deletetileid).getRangePageVectorSize();

  int RangeEntries_per_page = _env->page_size / _env->per_range_delete_size;
  for (int i = 0; i < page_count; ++i)
  {
    vector<RangeEntry> RangeEntries_to_populate_page;
    RangeEntries_per_page = Utility::minInt(RangeEntries_per_page, RangeEntries_to_populate_tile.size());
    for (int j = 0; j < RangeEntries_per_page; ++j)
    {
      RangeEntries_to_populate_page.push_back(RangeEntries_to_populate_tile[j]);
    }
    
    for (int j = 0; j < RangeEntries_per_page; ++j)
    {
      Page& retrieved_page = file->getIthRangeDeleteTile(deletetileid).getIthRangePage(i);
      DeleteTile& deletetile = file->getIthRangeDeleteTile(deletetileid);

      long start_key_to_insert = RangeEntries_to_populate_tile[j].getStartKey();
      long end_key_to_insert = RangeEntries_to_populate_tile[j].getEndKey();
      int status = SSTFile::generateRangeMetaData(file, deletetile,
                                    retrieved_page, start_key_to_insert, end_key_to_insert);

      RangeEntry *range_entry = new RangeEntry(RangeEntries_to_populate_page[j].get_pll_pair() , -1/*TODO: TIMETAG*/);
                                                                            
      file->getIthRangeDeleteTile(deletetileid).getIthRangePage(i).addRangeEntry(*range_entry);                                                                            
    }

    RangeEntries_to_populate_tile.erase(RangeEntries_to_populate_tile.begin(), RangeEntries_to_populate_tile.begin() + RangeEntries_per_page);

    RangeEntries_to_populate_page.clear();
  }

  return 1;
}

int SSTFile::getSSTMaxKey(){
  return max(this->max_sort_key, this->max_range_key);
}

int SSTFile::getSSTMinKey(){
  if(this->min_range_key < 0 ){
    return this->min_sort_key;
  }
  if(this->min_sort_key < 0 ){
    return this->min_range_key;
  }
  return min(this->min_sort_key, this->min_range_key);
  
}
