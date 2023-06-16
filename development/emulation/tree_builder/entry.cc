#include "entry.h"

using namespace std;
using namespace tree_builder;

using KV = pair < pair < long, long> , string>;
using pll = pair<long, long>;

PointEntry::PointEntry(KV kv, long timetag){
  this->sort_key = kv.first.first;
  this->delete_key = kv.first.second;
  this->value = kv.second;
  this->timetag = timetag;
}

KV PointEntry::get_keyvalue_pair(){
  return {{this->sort_key,this->delete_key}, this->value};
}

long PointEntry::getSortKey(){
  return this->sort_key;
}

long PointEntry::getDeleteKey(){
  return this->delete_key;
}

string PointEntry::getValue(){
  return this->value;
}

void PointEntry::setValue(string value){
  this->value = value;
}

long PointEntry::getTimetag(){
  return this->timetag;
}

RangeEntry::RangeEntry(pll range, long timetag){
  this->start_key = range.first;
  this->end_key = range.second;
  this->timetag = timetag;
}

pll RangeEntry::get_pll_pair() {
  return {this->getStartKey(), this->getEndKey()};
}

long RangeEntry::getStartKey(){
  return this->start_key;
}

long RangeEntry::getEndKey(){
  return this->end_key;
}

long RangeEntry::getTimetag(){
  return this->timetag;
}

void RangeEntry::setStartKey(long start_key){
  this->start_key = start_key;
}

void RangeEntry::setEndKey(long end_key){
  this->end_key = end_key;
}

void EntryList::print(){
  std:cout << "Kv vector: " << endl;
  for(int i = 0; i < point_entry_vector.size(); i++){
    cout << to_string(point_entry_vector[i].getSortKey()) + "| " ;
  }
  cout << endl;
  cout << "Range deletes: " << endl;
  for(int j = 0; j < range_entry_vector.size(); j++){
    cout << std::to_string(range_entry_vector[j].getStartKey()) +" "+ to_string(range_entry_vector[j].getEndKey()) + "|" ;
  }
  cout <<endl;
}


EntryList& EntryList::add_point_entry(PointEntry point_entry){
  this->point_entry_vector.push_back(point_entry);
  return *this;
}
EntryList& EntryList::add_range_entry(RangeEntry range_entry){
  this->range_entry_vector.push_back(range_entry);
  return *this;
}

vector<PointEntry> EntryList::get_point_entry_vector(){
  return this->point_entry_vector;
}
vector<RangeEntry> EntryList::get_range_entry_vector(){
  return this->range_entry_vector;
}



void EntryList::remove_point_entries_in_range(long start_key, long end_key){
  vector<PointEntry> new_point_entry_vector;
  int entry_count = 0;
  int entry_size = 0;
  for(PointEntry &point_entry: this->point_entry_vector){
    if((point_entry.getSortKey() >= start_key) && (point_entry.getSortKey() <= end_key)){
      entry_count += 1;
      entry_size += sizeof(point_entry.getSortKey()) + sizeof(point_entry.getDeleteKey()) + point_entry.getValue().size();
      continue;
    }
    
    new_point_entry_vector.push_back(point_entry);
  }

  MemoryBuffer::setCurrentBufferStatistics(-1*entry_count, 0, -1*entry_size);
  WorkloadExecutor::total_buffer_range_tombstone_deleted_point_entry_count += entry_count;


  this->point_entry_vector = new_point_entry_vector;
}


vector<PointEntry> EntryList::extract_point_entry_from_overlapping_file(SSTFile *sst_file_level_head){
  if(this->point_entry_vector.size() == 0){return vector<PointEntry>();}

  vector<PointEntry> extracted_point_entry_vector;
  SSTFile *sst_file_level_ptr = sst_file_level_head;
  SSTFile *sst_head = sst_file_level_head;
  

  //check if point_entry_vector is sorted or not
  for(int i = 1; i < this->point_entry_vector.size(); i++){
    if(this->point_entry_vector[i-1].getSortKey() > this->point_entry_vector[i].getSortKey()){
      cout << "Error: point_entry_vector to be flushed has not been sorted in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }

  auto it = this->point_entry_vector.begin();
  auto ite = this->point_entry_vector.end();
  while(sst_file_level_ptr != NULL){
    long min_sort_key = sst_file_level_ptr->getMinSortKey();
    long max_sort_key = sst_file_level_ptr->getMaxSortKey();
    
    while((it->getSortKey() < min_sort_key) && (it != ite)){
 
      it++;  
    }
    if(it == ite){break;} // no overlap

    if(it->getSortKey() > max_sort_key){
      sst_file_level_ptr = sst_file_level_ptr->getNextFilePtr();
      continue;
    }

    // find overlapping
    vector<DeleteTile> point_delete_tile_vector = sst_file_level_ptr->getPointDeleteTileVector();
    for(DeleteTile &delete_tile : point_delete_tile_vector){
      
      vector<Page> point_page_vector = delete_tile.getPointPageVector();
      for(Page &page: point_page_vector){

        vector<PointEntry> point_entry_vector = page.getPointEntryVector();
        for(PointEntry &point_entry: point_entry_vector){

          long sort_key = point_entry.getSortKey();

          while((it->getSortKey() < sort_key) && (it != ite)){it++;}
          if(it == ite){
            extracted_point_entry_vector.push_back(point_entry);
            continue;
          }

          if(it->getSortKey() == sort_key){
            WorkloadExecutor::total_merge_deleted_entry_count++;
            WorkloadExecutor::total_merge_deleted_point_entry_count++;
            continue;
          } //add counter
          extracted_point_entry_vector.push_back(point_entry);
        }
      }
    }
    sst_file_level_ptr = sst_file_level_ptr->getNextFilePtr();
  }
  return extracted_point_entry_vector;
}


vector<PointEntry> EntryList::apply_range_delete_on_point_entry_vector(vector<PointEntry> &next_level_point_entry_vector){
  if(this->range_entry_vector.size() == 0){return next_level_point_entry_vector;}

  vector<PointEntry> filtered_next_level_point_entry_vector;
  vector<int> sst_ids; 
  int id_count = 0;

  auto it = this->range_entry_vector.begin();
  auto ite = this->range_entry_vector.end();
  for(PointEntry &point_entry: next_level_point_entry_vector){
    long sort_key = point_entry.getSortKey();

    long min_start_key = it->getStartKey();
    long max_end_key = it->getEndKey();

    while((sort_key > max_end_key) && (it != ite)){
      it++;
      min_start_key = it->getStartKey();
      max_end_key = it->getEndKey();
    }
    if(it == ite){
      filtered_next_level_point_entry_vector.push_back(point_entry);
      continue;
    }
    if(sort_key >= min_start_key && sort_key <= max_end_key){
      WorkloadExecutor::total_range_deleted_point_entry_count++;
      continue;
    }

    filtered_next_level_point_entry_vector.push_back(point_entry);
  }
  return filtered_next_level_point_entry_vector;
}

void EntryList::merge_with_another_point_entry_vector(vector<PointEntry> &next_level_point_entry_vector){
  vector<PointEntry> merged_point_entry_vector;
  vector<PointEntry> &A = this->point_entry_vector;
  vector<PointEntry> &B = next_level_point_entry_vector;

  //check that A doesn't have repetitive sort key entries
  for(int i = 1; i < A.size(); i++){
    if(A[i-1].getSortKey() == A[i].getSortKey()){
      cout << "Error: A (self point_entry_vector) has repetitive sort key entries (entries of the same sort key)" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }

  //check that B doesn't have repetitive sort key entries 
  for(int i = 1; i < B.size(); i++){
    if(B[i-1].getSortKey() == B[i].getSortKey()){
      cout << "Error: B (passed in point_entry_vector) has repetitive sort key entries (entries of the same sort key)" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }

  auto itA = A.begin();
  auto itB = B.begin();
  while( (itA != A.end()) || (itB != B.end()) ){
    if(itA == A.end()){

      merged_point_entry_vector.push_back(*itB);
      itB++;
    }
    else if(itB == B.end()){
      merged_point_entry_vector.push_back(*itA);
      itA++;
    }
    else if(itA->getSortKey() < itB->getSortKey()){
      merged_point_entry_vector.push_back(*itA);
      itA++;
    }
    // if the sort keys are the same, then get the upper-leveled entry (current)
    else if(itA->getSortKey() == itB->getSortKey()){
      merged_point_entry_vector.push_back(*itA);
      WorkloadExecutor::total_merge_deleted_entry_count++;
      WorkloadExecutor::total_merge_deleted_point_entry_count++;
      itA++;
      itB++;
    }
    else{
      merged_point_entry_vector.push_back(*itB);
      itB++;
    }
  }

  this->point_entry_vector.clear();
  this->point_entry_vector = merged_point_entry_vector;
}

//Notice: range entry boundary is now depending on the range key instead of sort key (Use range key boundary)
vector<RangeEntry> EntryList::extract_range_entry_from_overlapping_file(SSTFile *sst_file_level_head){
  if(this->range_entry_vector.size() == 0){return vector<RangeEntry>();}

  vector<RangeEntry> extracted_range_entry_vector;

  //check that range_entry_vector is sorted
  for(int i = 1; i < this->range_entry_vector.size(); i++){
    if(this->range_entry_vector[i-1].getStartKey() > this->range_entry_vector[i].getStartKey()){
      cout << "Error: range_entry_vector to be flushed has not been sorted in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }
  

  auto it = this->range_entry_vector.begin();
  auto ite = this->range_entry_vector.end();
  while(sst_file_level_head != NULL){

    long min_key = sst_file_level_head->getMinRangeKey();
    long max_key = sst_file_level_head->getMaxRangeKey();

    while( (it->getEndKey() < min_key-1) && (it != ite) ){
      it++;
    }
    if(it == ite){break;}

    if(it->getStartKey() > max_key+1){
      sst_file_level_head = sst_file_level_head->getNextFilePtr();
      continue;
    }
    vector<DeleteTile> range_delete_tile_vector = sst_file_level_head->getRangeDeleteTileVector();
    for(DeleteTile &delete_tile : range_delete_tile_vector){
      
      vector<Page> range_page_vector = delete_tile.getRangePageVector();
      for(Page &page: range_page_vector){

        vector<RangeEntry> range_entry_vector = page.getRangeEntryVector();
        for(RangeEntry &range_entry: range_entry_vector){

          long start_key = range_entry.getStartKey();
          long end_key = range_entry.getEndKey();

          while( (it->getEndKey() < start_key-1) && (it != ite) ){it++;}
          if(it == ite){
            extracted_range_entry_vector.push_back(range_entry);
            continue;
          }
          //fully overlapped the next level range entry
          if(it->getStartKey() <= start_key && it->getEndKey() >= end_key){
            WorkloadExecutor::total_merge_deleted_entry_count++;
            WorkloadExecutor::total_merge_deleted_range_entry_count++;
            continue;
          }
          
          extracted_range_entry_vector.push_back(range_entry);
        }
      }
    }

    sst_file_level_head = sst_file_level_head->getNextFilePtr();
  }
  return extracted_range_entry_vector;
}

void EntryList::merge_its_own_range_entry(){

  //check that range_entry_vector is sorted
  for(int i = 1; i < this->range_entry_vector.size(); i++){
    if(this->range_entry_vector[i-1].getStartKey() > this->range_entry_vector[i].getStartKey()){
      cout << "Error: range_entry_vector to be flushed has not been sorted in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }

  vector<RangeEntry> merged_range_entry_vector;
  vector<RangeEntry> &A = this->range_entry_vector;
  auto itA = A.begin();
  while(itA != A.end()){
    long currentMin, currentMax;
    currentMin = itA->getStartKey();
    currentMax = itA->getEndKey();

    int count = 0;
    while( (itA != A.end()) && (itA->getStartKey() <= currentMax+1) ){
      currentMax = max(currentMax, itA->getEndKey());
      itA++;
      count++;
    }
    count--;
    WorkloadExecutor::total_merge_deleted_entry_count += count;
    WorkloadExecutor::total_merge_deleted_range_entry_count += count;
    merged_range_entry_vector.push_back(RangeEntry( pll({currentMin, currentMax}), -1));
  }
  this->range_entry_vector.clear();
  this->range_entry_vector = merged_range_entry_vector;
}

void EntryList::merge_with_another_range_entry_vector(vector<RangeEntry> &next_level_range_entry_vector){
  //check that range_entry_vector is sorted
  for(int i = 1; i < this->range_entry_vector.size(); i++){
    if(this->range_entry_vector[i-1].getStartKey() > this->range_entry_vector[i].getStartKey()){
      cout << "Error: range_entry_vector to be flushed has not been sorted in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }
  
  //check that next_level_range_entry_vector is sorted
  for(int i = 1; i < next_level_range_entry_vector.size(); i++){
    if(next_level_range_entry_vector[i-1].getStartKey() > next_level_range_entry_vector[i].getStartKey()){
      cout << "Error: next_level_range_entry_vector has not been sorted in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }

  vector<RangeEntry> merged_range_entry_vector;
  vector<RangeEntry> &A = this->range_entry_vector;
  vector<RangeEntry> &B = next_level_range_entry_vector;
  auto itA = A.begin();
  auto itB = B.begin();
  while( (itA != A.end()) || (itB != B.end()) ){
    long currentMin, currentMax;
    if(itA == A.end()){
      currentMin = itB->getStartKey();
      currentMax = itB->getEndKey();
      itB++;
    }else if(itB == B.end()){
      currentMin = itA->getStartKey();
      currentMax = itA->getEndKey();
      itA++;
    }else if(itA->getStartKey() <= itB->getStartKey()){
      currentMin = itA->getStartKey();
      currentMax = itA->getEndKey();
      itA++;
    }else{
      currentMin = itB->getStartKey();
      currentMax = itB->getEndKey();
      itB++;
    }

    int count = 1;
    while(itA != A.end() || itB != B.end()){
      if(itA != A.end() && itA->getStartKey() <= currentMax+1){
        currentMax = max(currentMax, itA->getEndKey());
        itA++;
        count++;
        continue;
      }
      if(itB != B.end() && itB->getStartKey() <= currentMax+1){
        currentMax = max(currentMax, itB->getEndKey());
        itB++;
        count++;
        continue;
      }
      // merged_range_entry_vector.push_back(RangeEntry( pll({currentMin, currentMax}), -1));
      break;
    }
    count--;
    WorkloadExecutor::total_merge_deleted_entry_count += count;
    WorkloadExecutor::total_merge_deleted_range_entry_count += count;

    merged_range_entry_vector.push_back(RangeEntry( pll({currentMin, currentMax}), -1));
  }

  this->range_entry_vector.clear();
  this->range_entry_vector = merged_range_entry_vector;
}
  
  

EntryList EntryList::get_minmax_range_overlapped_entries_from_files(SSTFile *sst_file_level_head){
  EntryList overlapped_entry_list;
  if( (this->point_entry_vector.size() == 0) && (this->range_entry_vector.size() == 0) ){return overlapped_entry_list;}
  if(sst_file_level_head == NULL){return overlapped_entry_list;}
  
  //check if point_entry_vector is sorted or not
  for(int i = 1; i < this->point_entry_vector.size(); i++){
    if(this->point_entry_vector[i-1].getSortKey() > this->point_entry_vector[i].getSortKey()){
      cout << "Error: point_entry_vector to be flushed has not been sorted in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }

  //check if range_entry_vector is sorted or not
  for(int i = 1; i < this->range_entry_vector.size(); i++){
    if(this->range_entry_vector[i-1].getStartKey() > this->range_entry_vector[i].getStartKey()){
      cout << "Error: range_entry_vector to be flushed has not been sorted in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }

  //check that range_entry_vector is not overlapped
  for(int i = 1; i < this->range_entry_vector.size(); i++){
    // if(this->range_entry_vector[i-1].getEndKey() >= this->range_entry_vector[i].getStartKey()){
    if(this->range_entry_vector[i-1].getEndKey() > this->range_entry_vector[i].getStartKey()){
      cout << "Error: range_entry_vector to be flushed has overlapped ranges" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }



  long min = this->get_min_key();
  long max = this->get_max_key();

  //check min <= max
  if(min > max){
    cout << "Error: min (" << min << ") > max (" << max << ") in get_minmax_range_overlapped_entries_from_files" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
    exit(1);
  }


  SSTFile *sst_file_level_ptr = sst_file_level_head;
  SSTFile *sst_head = sst_file_level_head;


  while(sst_file_level_ptr != NULL){
    long file_min_key = sst_file_level_ptr->getSSTMinKey();
    long file_max_key = sst_file_level_ptr->getSSTMaxKey();
   
    if(file_max_key < min){
      sst_file_level_ptr = sst_file_level_ptr->getNextFilePtr();
      continue;
    }
    if(file_min_key > max){break;} // no overlap anymore



    // find overlapping entries
    //get point entries
    vector<DeleteTile> point_delete_tile_vector = sst_file_level_ptr->getPointDeleteTileVector();
    for(DeleteTile &delete_tile : point_delete_tile_vector){
      vector<Page> point_page_vector = delete_tile.getPointPageVector();
      for(Page &page: point_page_vector){
        vector<PointEntry> point_entry_vector = page.getPointEntryVector();
        for(PointEntry &point_entry: point_entry_vector){       
          overlapped_entry_list.add_point_entry(point_entry);
        }
      }
    }

    //get range entries
    vector<DeleteTile> range_delete_tile_vector = sst_file_level_ptr->getRangeDeleteTileVector();
    for(DeleteTile &delete_tile : range_delete_tile_vector){
      vector<Page> range_page_vector = delete_tile.getRangePageVector();
      for(Page &page: range_page_vector){
        vector<RangeEntry> range_entry_vector = page.getRangeEntryVector();
        for(RangeEntry &range_entry: range_entry_vector){       
          overlapped_entry_list.add_range_entry(range_entry);
        }
      }
    }

    //move to next file
    sst_file_level_ptr = sst_file_level_ptr->getNextFilePtr();
  }
  return overlapped_entry_list;
}


  
void EntryList::add_range_delete_tombstone(pll range_delete_tombstone){
  range_delete_tombstone_vector.push_back(range_delete_tombstone);
}

int EntryList::get_entrylist_size_inByte() { // in byte
  EmuEnv *_env = EmuEnv::getInstance();
  return this->point_entry_vector.size()*_env->per_kv_size + this->range_entry_vector.size()*_env->per_range_delete_size;
}

int EntryList::get_point_entry_vector_size_inByte() { // in byte 
  EmuEnv *_env = EmuEnv::getInstance();
  return this->point_entry_vector.size()*_env->per_kv_size;
}

int EntryList::get_range_entry_vector_size_inByte() { // in byte 
  EmuEnv *_env = EmuEnv::getInstance();
  return this->range_entry_vector.size()*_env->per_range_delete_size;
}


int EntryList::get_PointEntry_size() {
  return this->point_entry_vector.size();
}

int EntryList::get_RangeEntry_size() {
  return this->range_entry_vector.size();
}

EntryList EntryList::split_out_by_size_inByte(int split_size)
{
  //check that point_entry_vector is sorted 
  for(int i = 1; i < this->point_entry_vector.size(); i++){
    if(this->point_entry_vector[i-1].getSortKey() > this->point_entry_vector[i].getSortKey()){
      cout << "Error: point_entry_vector to be flushed has not been sorted in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }

  //check that range_entry_vector is sorted 
  for(int i = 1; i < this->range_entry_vector.size(); i++){
    if(this->range_entry_vector[i-1].getStartKey() > this->range_entry_vector[i].getStartKey()){
      cout << "Error: range_entry_vector to be flushed has not been sorted in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }

  //check that range_entry_vector is not overlapped
  for(int i = 1; i < this->range_entry_vector.size(); i++){
    if(this->range_entry_vector[i-1].getEndKey() >= this->range_entry_vector[i].getStartKey()){
      cout << "Error: range_entry_vector to be flushed has overlapped ranges" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }

  EmuEnv *_env = EmuEnv::getInstance();

  //check that file size shall be at least large enough to hold one point entry + one range entry
  if(split_size < _env->per_kv_size + _env->per_range_delete_size){
    cout << "Error: file size shall be at least large enough to hold one point entry + one range entry" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
    exit(1);
  }


  EntryList split_output;
  vector<PointEntry> remaining_point_entry_vector;
  vector<RangeEntry> remaining_range_entry_vector;

  auto kv_iter = this->point_entry_vector.begin();
  auto kv_end = this->point_entry_vector.end();
  auto pll_iter = this->range_entry_vector.begin();
  auto pll_end = this->range_entry_vector.end();

    // using 2 pointer points to the 2 first elements of vector

  EntryList entries_to_populate_file; // entrylist that gonna be output 
  int current_file_size = 0;

  while(current_file_size < split_size)
  {  // the file is not full
    kv_end = this->point_entry_vector.end(); //  end
    pll_end = this->range_entry_vector.end();

    if(kv_iter == kv_end && pll_iter == pll_end) {break;} // if we iterate all the element, break
    
    if(pll_iter == pll_end)
    { // if range delete ends, push kv
      if(current_file_size + _env->per_kv_size > split_size) {break;} // if the file cannot contains, break
      entries_to_populate_file.add_point_entry(*kv_iter);
      current_file_size += _env->per_kv_size; // may change
      kv_iter++;
      continue;
    }

    if(kv_iter == kv_end)
    { // if kv ends, push range deletes
      if(current_file_size + _env->per_range_delete_size > split_size) {break;} // if the file cannot contains, break
      entries_to_populate_file.add_range_entry(*pll_iter);
      current_file_size += _env->per_range_delete_size; // may change
      pll_iter++;
      continue;
    }

    if(kv_iter->getSortKey() < pll_iter->getEndKey())
    {
      if(current_file_size + _env->per_kv_size > split_size) {break;} // if the file cannot contains, break
      entries_to_populate_file.add_point_entry(*kv_iter);
      current_file_size += _env->per_kv_size; // may change
      kv_iter++;
    }else if(kv_iter->getSortKey() == pll_iter->getEndKey()){ // if equals both shall be pushed in the same file
      if(current_file_size + _env->per_kv_size + _env->per_range_delete_size > split_size) {break;} // if the file cannot contains, break
      entries_to_populate_file.add_point_entry(*kv_iter);
      entries_to_populate_file.add_range_entry(*pll_iter);
      current_file_size += _env->per_kv_size; // may change
      current_file_size += _env->per_range_delete_size; // may change
      kv_iter++;
      pll_iter++;
    }else{
      if(current_file_size + _env->per_range_delete_size > split_size) {break;} // if the file cannot contains, break
      entries_to_populate_file.add_range_entry(*pll_iter);
      current_file_size += _env->per_range_delete_size; // may change
      pll_iter++;
    }
  } 

  kv_end = this->point_entry_vector.end(); //  end
  pll_end = this->range_entry_vector.end();
  //range shall divided to fit in a file of certain SortKey range
  //A file can at least hold one point entry + one range entry
  if( (entries_to_populate_file.get_point_entry_vector_size() != 0) && (pll_iter != pll_end) ){
    long max_sort_key = entries_to_populate_file.get_ith_point_entry( entries_to_populate_file.get_point_entry_vector_size()-1 ).getSortKey();
    while( (pll_iter != pll_end) && (max_sort_key >= pll_iter->getStartKey()) ){
      if(current_file_size + _env->per_range_delete_size <= split_size){ break;}

      kv_iter--;   
      current_file_size -= _env->per_kv_size; // may change
      
      entries_to_populate_file.pop_point_entry_vector_back();
      max_sort_key = entries_to_populate_file.get_ith_point_entry( entries_to_populate_file.get_point_entry_vector_size()-1 ).getSortKey();

      //A file can at least hold one point entry + one range entry
      if(kv_iter == this->point_entry_vector.begin()){break;}
      continue;
    }
    if( (pll_iter != pll_end) && (max_sort_key >= pll_iter->getStartKey()) ){
      //split range entry
      long new_start_key = pll_iter->getStartKey();
      long new_end_key = max_sort_key;
      pll_iter->setStartKey(max_sort_key+1);

      entries_to_populate_file.add_range_entry(RangeEntry({make_pair(new_start_key, new_end_key), pll_iter->getTimetag()}));
      current_file_size += _env->per_range_delete_size; // may change

      WorkloadExecutor::total_file_boundary_split_range_entry_count++;
    }
  }

  for(; kv_iter != kv_end; kv_iter++){ remaining_point_entry_vector.push_back(*kv_iter); }
  for(; pll_iter != pll_end; pll_iter++){ remaining_range_entry_vector.push_back(*pll_iter); }
  this->point_entry_vector = remaining_point_entry_vector;
  this->range_entry_vector = remaining_range_entry_vector;

  return entries_to_populate_file;
}     

bool EntryList::empty() {
  return this->point_entry_vector.empty() && this->range_entry_vector.empty();
}

bool EntryList::PointEntry_empty(){
  return this->point_entry_vector.empty();
}

bool EntryList::RangeEntry_empty(){
  return this->range_entry_vector.empty();
}

void EntryList::clear() {
  this->point_entry_vector.clear();
  this->range_entry_vector.clear();
}

int EntryList::get_min_key() {
  if(this->get_point_entry_vector_size() == 0 && this->get_range_entry_vector_size() == 0){return -1;}

  if(this->get_point_entry_vector_size() == 0){
    return this->range_entry_vector[0].getStartKey();
  }

  if(this->get_range_entry_vector_size() == 0){
    return this->point_entry_vector[0].getSortKey();
  }

  return min(this->point_entry_vector[0].getSortKey(), this->range_entry_vector[0].getStartKey());
}

int EntryList::get_max_key() {
  if(this->get_point_entry_vector_size() == 0 && this->get_range_entry_vector_size() == 0){return -1;}

  if(this->get_point_entry_vector_size() == 0){
    return this->range_entry_vector.back().getEndKey();
  }

  if(this->get_range_entry_vector_size() == 0){
    return this->point_entry_vector.back().getSortKey();
  }

  return max(this->point_entry_vector.back().getSortKey(), this->range_entry_vector.back().getEndKey() );
}

void EntryList::add_entry(KV kv){
  kv_vector.push_back(kv);
}

void EntryList::sortPointEntryVectorOnSortKseyInAscendingOrder(){
  sort(this->point_entry_vector.begin(), this->point_entry_vector.end(), [&](PointEntry &a, PointEntry &b){
    return a.getSortKey() < b.getSortKey();
  });
}

void EntryList::sortRangeEntryVectorOnStartKeyInAscendingOrder(){
  sort(this->range_entry_vector.begin(), this->range_entry_vector.end(), [&](RangeEntry &a, RangeEntry &b){
    return a.getStartKey() < b.getStartKey();
  });
}

void EntryList::sortRangeEntryVectorOnEndKeyInAscendingOrder(){
  sort(this->range_entry_vector.begin(), this->range_entry_vector.end(), [&](RangeEntry &a, RangeEntry &b){
    return a.getEndKey() < b.getEndKey();
  });
}



int EntryList::get_point_entry_vector_size() {
  return this->point_entry_vector.size();
}

int EntryList::get_range_entry_vector_size() {
  return this->range_entry_vector.size();
}

PointEntry &EntryList::get_ith_point_entry(int i) {
  return this->point_entry_vector[i];
}

RangeEntry &EntryList::get_ith_range_entry(int i) {
  return this->range_entry_vector[i];
}

void EntryList::pop_point_entry_vector_back(){
  this->point_entry_vector.pop_back();
}