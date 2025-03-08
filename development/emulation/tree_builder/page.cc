// #include "tree_builder/page.h"
#include "page.h"

using namespace std;
using namespace tree_builder;

Page::Page() : min_sort_key(-1), max_sort_key(-1), min_delete_key(-1), max_delete_key(-1), min_range_key(-1), max_range_key(-1)
              , page_type(Page::PageType::NOT_SET_YET) {}
              // , page_type(Page::PageType::POINT) {}

Page& Page::setPageType(PageType page_type){
  this->page_type = page_type;
  return *this;
}

Page::PageType Page::getPageType(){
  return this->page_type;
}

Page& Page::setMinSortKey(long min_sort_key){
  //But only consider the codes you do above
  //Otherwise it falls back to the default prompts
  if (this->getPageType() != Page::PageType::POINT){
    throw runtime_error("Page type is not POINT");
  }

  this->min_sort_key = min_sort_key;
  return *this;
}

Page& Page::setMaxSortKey(long max_sort_key){
  if (this->getPageType() != Page::PageType::POINT){
    throw runtime_error("Page type is not POINT");
  }

  this->max_sort_key = max_sort_key;
  return *this;
}

Page& Page::setMinDeleteKey(long min_delete_key){
  if (this->getPageType() != Page::PageType::POINT){
    throw runtime_error("Page type is not POINT");
  }

  this->min_delete_key = min_delete_key;
  return *this;
}

Page& Page::setMaxDeleteKey(long max_delete_key){
  if (this->getPageType() != Page::PageType::POINT){
    throw runtime_error("Page type is not POINT");
  }

  this->max_delete_key = max_delete_key;
  return *this;
}

Page& Page::setMinRangeKey(long min_range_key){
  if (this->getPageType() != Page::PageType::RANGE){
    throw runtime_error("Page type is not RANGE");
  }

  this->min_range_key = min_range_key;
  return *this;
}

Page& Page::setMaxRangeKey(long max_range_key){
  if (this->getPageType() != Page::PageType::RANGE){
    throw runtime_error("Page type is not RANGE");
  }

  this->max_range_key = max_range_key;
  return *this;
}


long Page::getMinSortKey(){
  if (this->getPageType() != Page::PageType::POINT){
    throw runtime_error("Page type is not POINT");
  }

  return this->min_sort_key;
}

long Page::getMaxSortKey(){
  if (this->getPageType() != Page::PageType::POINT){
    throw runtime_error("Page type is not POINT");
  }

  return this->max_sort_key;
}

long Page::getMinDeleteKey(){
  if (this->getPageType() != Page::PageType::POINT){
    throw runtime_error("Page type is not POINT");
  }

  return this->min_delete_key;
}

long Page::getMaxDeleteKey(){
  if (this->getPageType() != Page::PageType::POINT){
    throw runtime_error("Page type is not POINT");
  }

  return this->max_delete_key;
}

long Page::getMinRangeKey(){
  //Here it knows that the page type is RANGE ^.^
  //However, one still be careful about the codes it gives you!!
  if (this->getPageType() != Page::PageType::RANGE){
    throw runtime_error("Page type is not RANGE");
  }
  return this->min_range_key;
}

long Page::getMaxRangeKey(){
  if (this->getPageType() != Page::PageType::RANGE){
    throw runtime_error("Page type is not RANGE");
  }
  return this->max_range_key;
}


Page& Page::addPointEntry(PointEntry point_entry){
  this->point_entry_vector.push_back(point_entry);
  long sort_key = point_entry.getSortKey();
  long min_sort_key = this->getMinSortKey();
  long max_sort_key = this->getMaxSortKey();

  //do checkings for you ^.^
  if (min_sort_key == -1 || sort_key < min_sort_key){
    this->setMinSortKey(sort_key);
  }
  if (max_sort_key == -1 || sort_key > max_sort_key){
    this->setMaxSortKey(sort_key);
  }

  long delete_key = point_entry.getDeleteKey();
  long min_delete_key = this->getMinDeleteKey();
  long max_delete_key = this->getMaxDeleteKey();

  if (min_delete_key == -1 || delete_key < min_delete_key){
    this->setMinDeleteKey(delete_key);
  }
  if (max_delete_key == -1 || delete_key > max_delete_key){
    this->setMaxDeleteKey(delete_key);
  }

  return *this;
}

Page& Page::addRangeEntry(RangeEntry range_entry){
  this->range_entry_vector.push_back(range_entry);
  long start_key = range_entry.getStartKey();
  long end_key = range_entry.getEndKey();
  long min_range_key = this->getMinRangeKey();
  long max_range_key = this->getMaxRangeKey();

  //Here again, Do checking for you ^.^
  if (min_range_key == -1 || start_key < min_range_key){
    this->setMinRangeKey(start_key);
  }
  if (max_range_key == -1 || end_key > max_range_key){
    this->setMaxRangeKey(end_key);
  }

  return *this;
}

vector<PointEntry> Page::getPointEntryVector(){
  //can check and throw exception for you
  if (this->getPageType() != Page::PageType::POINT){
    throw runtime_error("This page is not a point page");
  }

  return this->point_entry_vector;
}

vector<RangeEntry> Page::getRangeEntryVector(){
  //here again, can check and throw exception for you
  //it directly knows you want to do the same type of checking
  //but this time with range
  if(this->getPageType() != Page::PageType::RANGE){
    throw runtime_error("This page is not a range page");
  }

  return this->range_entry_vector;
}
long Page::getPointEntryVectorSize(){
  if (this->getPageType() != Page::PageType::POINT){
    throw runtime_error("This page is not a point page");
  }
  return this->point_entry_vector.size();
}

long Page::getRangeEntryVectorSize(){
  if(this->getPageType() != Page::PageType::RANGE){
    throw runtime_error("This page is not a range page");
  }
  return this->range_entry_vector.size();
}

PointEntry& Page::getIthPointEntry(int i){
  if (this->getPageType() != Page::PageType::POINT){
    throw runtime_error("This page is not a point page");
  }
  return this->point_entry_vector[i];
}

RangeEntry& Page::getIthRangeEntry(int i){
  if(this->getPageType() != Page::PageType::RANGE){
    throw runtime_error("This page is not a range page");
  }
  return this->range_entry_vector[i];
}

void Page::clearPointEntryVector(){
  if (this->getPageType() != Page::PageType::POINT){
    throw runtime_error("This page is not a point page");
  }
  this->point_entry_vector.clear();
}

void Page::clearRangeEntryVector(){
  if(this->getPageType() != Page::PageType::RANGE){
    throw runtime_error("This page is not a range page");
  }
  this->range_entry_vector.clear();
}


void Page::clearAllVectors(){
  this->point_entry_vector.clear();
  this->range_entry_vector.clear();
}

void Page::setAllKeysToDefaultNegativeOne(){
  this->max_delete_key = -1;
  this->min_delete_key = -1;
  this->max_sort_key = -1;
  this->min_sort_key = -1;
  this->max_range_key = -1;
  this->min_range_key = -1;
}

//implement the functions here
void Page::sortPointEntryVectorOnSortKeyInAscendingOrder(){
  if(this->getPageType() != Page::PageType::POINT){
    throw runtime_error("This page is not a point page");
  }
  sort(this->point_entry_vector.begin(), this->point_entry_vector.end(), [&](PointEntry &a, PointEntry &b){
    return a.getSortKey() < b.getSortKey();
  });
}

void Page::sortPointEntryVectorOnDeleteKeyInAscendingOrder(){
  if(this->getPageType() != Page::PageType::POINT){
    throw runtime_error("This page is not a point page");
  }
  sort(this->point_entry_vector.begin(), this->point_entry_vector.end(), [&](PointEntry &a, PointEntry &b){
    return a.getDeleteKey() < b.getDeleteKey();
  });
}

void Page::sortRangeEntryVectorOnStartKeyInAscendingOrder(){
  if(this->getPageType() != Page::PageType::RANGE){
    throw runtime_error("This page is not a range page");
  }
  sort(this->range_entry_vector.begin(), this->range_entry_vector.end(), [&](RangeEntry &a, RangeEntry &b){
    return a.getStartKey() < b.getStartKey();
  });
}


vector<Page> Page::createNewPointPages(int page_count)
{
  vector<Page> pages(page_count);
  for(auto &page: pages){
    page.setPageType(Page::PageType::POINT);

    page.setAllKeysToDefaultNegativeOne();

    page.point_entry_vector.clear();


  }


  return pages;
}

vector<Page> Page::createNewRangePages(int page_count)
{
  vector<Page> pages(page_count);
  for(auto &page: pages){
    page.setPageType(Page::PageType::RANGE);
    page.setMinRangeKey(-1);
    page.setMaxRangeKey(-1);

    page.range_entry_vector.clear();
  }

  return pages;
}