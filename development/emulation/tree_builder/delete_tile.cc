#include "delete_tile.h"

using namespace std;
using namespace tree_builder;
using namespace workload_exec;

DeleteTile::DeleteTile() : min_sort_key(-1), max_sort_key(-1), min_delete_key(-1), max_delete_key(-1),  min_range_key(-1), max_range_key(-1),
            // delete_tile_type(DeleteTile::NOT_SET_YET)  {}
            delete_tile_type(DeleteTile::POINT)  {}

DeleteTile& DeleteTile::setDeleteTileType(DeleteTileType delete_tile_type){
  this->delete_tile_type = delete_tile_type;
  return *this;
}

DeleteTile::DeleteTileType DeleteTile::getDeleteTileType(){
  return this->delete_tile_type;
}

DeleteTile& DeleteTile::setMinSortKey(long min_sort_key){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::POINT){
    throw runtime_error("DeleteTile type is not POINT");
  }
  this->min_sort_key = min_sort_key;
  return *this;
}

DeleteTile& DeleteTile::setMaxSortKey(long max_sort_key){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::POINT){
    throw runtime_error("DeleteTile type is not POINT");
  }

  this->max_sort_key = max_sort_key;
  return *this;
}

DeleteTile& DeleteTile::setMinDeleteKey(long min_delete_key){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::POINT){
    throw runtime_error("DeleteTile type is not POINT");
  }

  this->min_delete_key = min_delete_key;
  return *this;
}

DeleteTile& DeleteTile::setMaxDeleteKey(long max_delete_key){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::POINT){
    throw runtime_error("DeleteTile type is not POINT");
  }

  this->max_delete_key = max_delete_key;
  return *this;
}

DeleteTile& DeleteTile::setMinRangeKey(long min_range_key){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::RANGE){
    throw runtime_error("DeleteTile type is not RANGE");
  }

  this->min_range_key = min_range_key;
  return *this;
}

DeleteTile& DeleteTile::setMaxRangeKey(long max_range_key){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::RANGE){
    throw runtime_error("DeleteTile type is not RANGE");
  }

  this->max_range_key = max_range_key;
  return *this;
}

long DeleteTile::getMinSortKey(){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::POINT){
    throw runtime_error("DeleteTile type is not POINT");
  }
  return this->min_sort_key;
}

long DeleteTile::getMaxSortKey(){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::POINT){
    throw runtime_error("DeleteTile type is not POINT");
  }
  return this->max_sort_key;
}

long DeleteTile::getMinDeleteKey(){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::POINT){
    throw runtime_error("DeleteTile type is not POINT");
  }
  return this->min_delete_key;
}

long DeleteTile::getMaxDeleteKey(){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::POINT){
    throw runtime_error("DeleteTile type is not POINT");
  }
  return this->max_delete_key;
}

long DeleteTile::getMinRangeKey(){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::RANGE){
    throw runtime_error("DeleteTile type is not RANGE");
  }
  return this->min_range_key;
}

long DeleteTile::getMaxRangeKey(){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::RANGE){
    throw runtime_error("DeleteTile type is not RANGE");
  }
  return this->max_range_key;
}

DeleteTile& DeleteTile::addPointPage(Page page){
  if(page.getPageType() != Page::PageType::POINT){
    throw runtime_error("Page type is not POINT");
  }
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::POINT){
    throw runtime_error("DeleteTile type is not POINT");
  }
  
  long page_min_sort_key = page.getMinSortKey();
  long page_max_sort_key = page.getMaxSortKey();
  long min_sort_key = this->getMinSortKey();
  long max_sort_key = this->getMaxSortKey();
  if (min_sort_key == -1 || page_min_sort_key < min_sort_key){
    this->setMinSortKey(page_min_sort_key);
  }
  if (max_sort_key == -1 || page_max_sort_key > max_sort_key){
    this->setMaxSortKey(page_max_sort_key);
  }

  long page_min_delete_key = page.getMinDeleteKey();
  long page_max_delete_key = page.getMaxDeleteKey();
  long min_delete_key = this->getMinDeleteKey();
  long max_delete_key = this->getMaxDeleteKey();
  if (min_delete_key == -1 || page_min_delete_key < min_delete_key){
    this->setMinDeleteKey(page_min_delete_key);
  }
  if (max_delete_key == -1 || page_max_delete_key > max_delete_key){
    this->setMaxDeleteKey(page_max_delete_key);
  }
  
  this->point_page_vector.push_back(page);
  return *this;
}

DeleteTile& DeleteTile::addRangePage(Page page){
  if(page.getPageType() != Page::PageType::RANGE){
    throw runtime_error("Page type is not RANGE");
  }
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::RANGE){
    throw runtime_error("DeleteTile type is not RANGE");
  }

  long page_min_range_key = page.getMinRangeKey();
  long page_max_range_key = page.getMaxRangeKey();
  long min_range_key = this->getMinRangeKey();
  long max_range_key = this->getMaxRangeKey();

  if (min_range_key == -1 || page_min_range_key < min_range_key){
    this->setMinRangeKey(page_min_range_key);
  }
  if (max_range_key == -1 || page_max_range_key > max_range_key){
    this->setMaxRangeKey(page_max_range_key);
  }

  this->range_page_vector.push_back(page);
  return *this;
}

DeleteTile& DeleteTile::setPointPageVector(vector<Page> point_page_vector){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::POINT){
    throw runtime_error("DeleteTile type is not POINT");
  }

  for(Page page : point_page_vector){
    if(page.getPageType() != Page::PageType::POINT){
      throw runtime_error("Page type is not POINT");
    }
    if(min_sort_key == -1 || page.getMinSortKey() < min_sort_key){
      this->setMinSortKey(page.getMinSortKey());
    }
    if(max_sort_key == -1 || page.getMaxSortKey() > max_sort_key){
      this->setMaxSortKey(page.getMaxSortKey());
    }
    if(min_delete_key == -1 || page.getMinDeleteKey() < min_delete_key){
      this->setMinDeleteKey(page.getMinDeleteKey());
    }
    if(max_delete_key == -1 || page.getMaxDeleteKey() > max_delete_key){
      this->setMaxDeleteKey(page.getMaxDeleteKey());
    }
  }

  this->point_page_vector = point_page_vector;
  return *this;
}
DeleteTile& DeleteTile::setRangePageVector(vector<Page> range_page_vector){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::RANGE){
    throw runtime_error("DeleteTile type is not RANGE");
  }

  for(Page page : range_page_vector){
    if(page.getPageType() != Page::PageType::RANGE){
      throw runtime_error("Page type is not RANGE");
    }
    if(min_range_key == -1 || page.getMinRangeKey() < min_range_key){
      this->setMinRangeKey(page.getMinRangeKey());
    }
    if(max_range_key == -1 || page.getMaxRangeKey() > max_range_key){
      this->setMaxRangeKey(page.getMaxRangeKey());
    }
  }

  this->range_page_vector = range_page_vector;
  return *this;
}

vector<Page> DeleteTile::getPointPageVector(){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::POINT){
    throw runtime_error("DeleteTile type is not POINT");
  }

  return this->point_page_vector;
}

vector<Page> DeleteTile::getRangePageVector(){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::RANGE){
    throw runtime_error("DeleteTile type is not RANGE");
  }

  return this->range_page_vector;
}

long DeleteTile::getPointPageVectorSize(){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::POINT){
    throw runtime_error("DeleteTile type is not POINT");
  }

  return this->point_page_vector.size();
}
long DeleteTile::getRangePageVectorSize(){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::RANGE){
    throw runtime_error("DeleteTile type is not RANGE");
  }

  return this->range_page_vector.size();
}
Page& DeleteTile::getIthPointPage(int i){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::POINT){
    throw runtime_error("DeleteTile type is not POINT");
  }

  return this->point_page_vector[i];
}
Page& DeleteTile::getIthRangePage(int i){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::RANGE){
    throw runtime_error("DeleteTile type is not RANGE");
  }

  return this->range_page_vector[i];
}
void DeleteTile::clearPointPageVector(){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::POINT){
    throw runtime_error("DeleteTile type is not POINT");
  }

  this->point_page_vector.clear();
}
void DeleteTile::clearRangePageVector(){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::RANGE){
    throw runtime_error("DeleteTile type is not RANGE");
  }

  this->range_page_vector.clear();
} 
void DeleteTile::clearAllVectors(){
  this->point_page_vector.clear();
  this->range_page_vector.clear();
}
void DeleteTile::setAllKeysToDefaultNegativeOne(){
  this->max_sort_key = -1;
  this->min_sort_key = -1;
  this->max_delete_key = -1;
  this->min_delete_key = -1;
  this->max_range_key = -1;
  this->min_range_key = -1;
}

void DeleteTile::sortPointPageVectorOnMinSortKeyInAscendingOrder(){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::POINT){
    throw runtime_error("DeleteTile type is not POINT");
  }

  sort(this->point_page_vector.begin(), this->point_page_vector.end(), [&] (Page &a, Page &b) -> bool {
    return a.getMinSortKey() < b.getMinSortKey();
  });
}
void DeleteTile::sortPointPageVectorOnMinDeleteKeyInAscendingOrder(){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::POINT){
    throw runtime_error("DeleteTile type is not POINT");
  }

  sort(this->point_page_vector.begin(), this->point_page_vector.end(), [&] (Page &a, Page &b) -> bool {
    return a.getMinDeleteKey() < b.getMinDeleteKey();
  });
}
void DeleteTile::sortRangePageVectorOnMinRangeKeyInAscendingOrder(){
  if (this->getDeleteTileType() != DeleteTile::DeleteTileType::RANGE){
    throw runtime_error("DeleteTile type is not RANGE");
  }

  sort(this->range_page_vector.begin(), this->range_page_vector.end(), [&] (Page &a, Page &b) -> bool {
    return a.getMinRangeKey() < b.getMinRangeKey();
  });
}




// vector<DeleteTile> DeleteTile::createNewDeleteTiles(int delete_tile_count_in_a_file, int level_to_flush_in)
vector<DeleteTile> DeleteTile::createNewPointDeleteTiles(int delete_tile_count_in_a_file, int level_to_flush_in)
{
  EmuEnv *_env = EmuEnv::getInstance();
  vector<DeleteTile> point_delete_tiles(delete_tile_count_in_a_file);
  for (int i = 0; i < point_delete_tiles.size(); i++){
    DeleteTile &delete_tile = point_delete_tiles[i];

    delete_tile.setDeleteTileType(DeleteTile::DeleteTileType::POINT);
    delete_tile.setDeleteTileType(DeleteTile::DeleteTileType::POINT);
    delete_tile.setAllKeysToDefaultNegativeOne();

    if (_env->buffer_size_in_pages % _env->getDeleteTileSize(level_to_flush_in) == 0)
    {
      delete_tile.setPointPageVector(Page::createNewPointPages(_env->getDeleteTileSize(level_to_flush_in)));
    }
    else
    {
      if ( i == point_delete_tiles.size() - 1)
      {
        delete_tile.setPointPageVector(Page::createNewPointPages(_env->buffer_size_in_pages % _env->getDeleteTileSize(level_to_flush_in)));
      }
      else
      {
        delete_tile.setPointPageVector(Page::createNewPointPages(_env->getDeleteTileSize(level_to_flush_in)));
      }
    }
  }


  return point_delete_tiles;
}

vector<DeleteTile> DeleteTile::createNewRangeDeleteTiles(int delete_tile_count_in_a_file, int level_to_flush_in){
  EmuEnv *_env = EmuEnv::getInstance();

  vector<DeleteTile> range_delete_tiles(delete_tile_count_in_a_file);
  for (int i = 0; i < range_delete_tiles.size(); i++){
    DeleteTile &delete_tile = range_delete_tiles[i];

    delete_tile.setDeleteTileType(DeleteTile::DeleteTileType::RANGE);
    delete_tile.setDeleteTileType(DeleteTile::DeleteTileType::RANGE);
    delete_tile.setAllKeysToDefaultNegativeOne();

    if (_env->buffer_size_in_pages % _env->getDeleteTileSize(level_to_flush_in) == 0)
    {
      delete_tile.setRangePageVector(Page::createNewRangePages(_env->getDeleteTileSize(level_to_flush_in)));
    }
    else
    {
      if ( i == range_delete_tiles.size() - 1)
      {
        delete_tile.setRangePageVector(Page::createNewRangePages(_env->buffer_size_in_pages % _env->getDeleteTileSize(level_to_flush_in)));
      }
      else
      {
        delete_tile.setRangePageVector(Page::createNewRangePages(_env->getDeleteTileSize(level_to_flush_in)));
      }
    }
  }
  return range_delete_tiles;
}
