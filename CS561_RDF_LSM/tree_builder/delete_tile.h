/*
 *  Created on: May 14, 2019
 *  Author: Subhadeep, Papon
 */

#ifndef LSM___DELETE_TILE_H_
#define LSM___DELETE_TILE_H_

namespace tree_builder {
  class DeleteTile;
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

  class DeleteTile {
  private:
    vector<Page> point_page_vector;   //ADDED
    vector<Page> range_page_vector;   //ADDED
    

    long min_sort_key;
    long max_sort_key;
    long min_delete_key;
    long max_delete_key;

    long min_range_key;
    long max_range_key;

  public:
    enum DeleteTileType {NOT_SET_YET, POINT, RANGE};

  // long min_sort_key;
  // long max_sort_key;
  // long min_delete_key;
  // long max_delete_key;
  // vector < Page > page_vector;
  // static struct vector<DeleteTile> createNewDeleteTiles(int delete_tile_count_in_a_file, int level_to_flush_in);
  
    DeleteTileType delete_tile_type;

    DeleteTile();
    DeleteTile& setDeleteTileType(DeleteTileType delete_tile_type);
    DeleteTileType getDeleteTileType();

    DeleteTile& setMinSortKey(long min_sort_key);
    DeleteTile& setMaxSortKey(long max_sort_key);
    DeleteTile& setMinDeleteKey(long min_delete_key);
    DeleteTile& setMaxDeleteKey(long max_delete_key);
    DeleteTile& setMinRangeKey(long min_range_key);
    DeleteTile& setMaxRangeKey(long max_range_key);

    long getMinSortKey();
    long getMaxSortKey();
    long getMinDeleteKey();
    long getMaxDeleteKey();
    long getMinRangeKey();
    long getMaxRangeKey();

    DeleteTile& addPointPage(Page page);
    DeleteTile& addRangePage(Page page);
    DeleteTile& setPointPageVector(vector<Page> point_page_vector);
    DeleteTile& setRangePageVector(vector<Page> range_page_vector);
    vector<Page> getPointPageVector();
    vector<Page> getRangePageVector();
    
    long getPointPageVectorSize();
    long getRangePageVectorSize();
    Page& getIthPointPage(int i);
    Page& getIthRangePage(int i);
    void clearPointPageVector();
    void clearRangePageVector();
    void clearAllVectors();
    void setAllKeysToDefaultNegativeOne();

    void sortPointPageVectorOnMinSortKeyInAscendingOrder();
    void sortPointPageVectorOnMinDeleteKeyInAscendingOrder();
    void sortRangePageVectorOnMinRangeKeyInAscendingOrder();

    static struct vector<DeleteTile> createNewPointDeleteTiles(int delete_tile_count_in_a_file, int level_to_flush_in);
    static struct vector<DeleteTile> createNewRangeDeleteTiles(int delete_tile_count_in_a_file, int level_to_flush_in);
  };


} // namespace

#endif /* LSM___DELETE_TILE_H_ */
