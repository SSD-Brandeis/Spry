/*
 *  Created on: May 14, 2019
 *  Author: Subhadeep, Papon
 */

#ifndef SST_FILE_H_
#define SST_FILE_H_


namespace tree_builder {
  class SSTFile;
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
  class SSTFile {
  private:
    vector<DeleteTile> point_delete_tile_vector;   //ADDED
    vector<DeleteTile> range_delete_tile_vector;   //ADDED

    long min_range_key;
    long max_range_key;

    
    int file_level;
    string file_id;
    long min_sort_key;
    long max_sort_key;
    long min_delete_key;
    long max_delete_key;

    struct SSTFile* next_file_ptr;

  public: 
    // enum SSTFileType {NOT_SET_YET, POINT, RANGE};

  // int file_level;
  // string file_id;
  // long min_sort_key;
  // long max_sort_key;
  // long min_delete_key;
  // long max_delete_key;

  // vector < DeleteTile > tile_vector;
  //struct SSTFile* next_file_ptr;
  
  static struct SSTFile* createNewSSTFile(int level_to_flush_in);
  static int PopulateFile(SSTFile *file, vector<pair<pair<long, long>, string>> vector_to_populate_file, int level_to_flush_in);
  static int PopulateDeleteTile(SSTFile *file, vector<pair<pair<long, long>, string>> vector_to_populate_tile, int deletetileid, int level_to_flush_in);
  
  static int generateMetaData(SSTFile *file, DeleteTile &deletetile, Page &page, long sort_key_to_insert, long delete_key_to_insert);
    static int generateRangeMetaData(SSTFile *file, DeleteTile &deletetile, Page &page, long sart_key_to_insert, long end_key_to_insert);
    
    static int PopulateFile_RDF(SSTFile *file, EntryList vector_to_populate_file, int level_to_flush_in);
    static int Populate_Point_DeleteTile_RDF(SSTFile *file, vector<PointEntry> PointEntries_to_populate_tile, int deletetileid, int level_to_flush_in);
    static int Populate_Range_DeleteTile_RDF(SSTFile *file, vector<RangeEntry> RangeEntries_to_populate_tile, int deletetileid, int level_to_flush_in);

    // SSTFileType sst_file_type;

    SSTFile();
    // SSTFile& setSSTFileType(SSTFileType sst_file_type);
    // SSTFileType getSSTFileType();

    static struct SSTFile* createNewSSTFile_RDF(int level_to_flush_in, int point_deletetile_count, int range_delete_tile_count);

    SSTFile& setFileLevel(int file_level);
    SSTFile& setFileId(string file_id);
    
    int getFileLevel();
    string getFileId();

    int getSSTMinKey();
    int getSSTMaxKey();


    SSTFile& setNextFilePtr(SSTFile* next_file_ptr);
    SSTFile* getNextFilePtr();


    SSTFile& setMinSortKey(long min_sort_key);
    SSTFile& setMaxSortKey(long max_sort_key);
    SSTFile& setMinDeleteKey(long min_delete_key);
    SSTFile& setMaxDeleteKey(long max_delete_key);
    SSTFile& setMinRangeKey(long min_range_key);
    SSTFile& setMaxRangeKey(long max_range_key);

    long getMinSortKey();
    long getMaxSortKey();
    long getMinDeleteKey();
    long getMaxDeleteKey();
    long getMinRangeKey();
    long getMaxRangeKey();

    SSTFile& addPointDeleteTile(DeleteTile point_delete_tile);
    SSTFile& addRangeDeleteTile(DeleteTile range_delete_tile);
    SSTFile& setPointDeleteTileVector(vector<DeleteTile> point_delete_tile_vector);
    SSTFile& setRangeDeleteTileVector(vector<DeleteTile> range_delete_tile_vector);
    vector<DeleteTile> getPointDeleteTileVector();
    vector<DeleteTile> getRangeDeleteTileVector();

    long getPointDeleteTileCount();
    long getRangeDeleteTileCount();
    DeleteTile& getIthPointDeleteTile(int i);
    DeleteTile& getIthRangeDeleteTile(int i);
    void clearPointDeleteTileVector();
    void clearRangeDeleteTileVector();
    void clearAllVectors();
    void setAllKeysToDefaultNegativeOne();

    void sortPointDeleteTileVectorOnMinSortKeyInAscendingOrder();
    void sortPointDeleteTileVectorOnMinDeleteKeyInAscendingOrder();
    void sortRangeDeleteTileVectorOnMinRangeKeyInAscendingOrder();



  };

} // namespace

#endif /* SST_FILE_H_ */

