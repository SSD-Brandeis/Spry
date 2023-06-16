#include "perLevel_range_delete_filter.h"

using namespace std;
using namespace range_delete_filter;


void PerlevelRangeDeleteFilterByVector::addRangeDelete(long start, long end){
  auto& rdList = PerlevelRangeDeleteFilterByVector::range_delete_list;
#ifdef DEBUG
    cout << "Adding range delete: " << start << " " << end << endl;
#endif
    vector<pll> rdList_new;
    long minK = start;
    long maxK = end;
    
    auto it = rdList.begin();
    while ( it != rdList.end() ){
        if (it->second < start-1){ rdList_new.push_back(*it); it++; continue;}
        if (it->first > end+1){ break;}

        minK = min(minK, it->first);
        maxK = max(maxK, it->second);
        it++;
    }
    rdList_new.push_back(pll({minK, maxK}));
    while(it != rdList.end()){
      rdList_new.push_back(*it);
      it++;
    }

    rdList.clear();
    for(auto &p : rdList_new){
      rdList.push_back(p);
    }
}

// void PerlevelRangeDeleteFilterByVector::addRangeDelete(vector<RangeEntry> &inserted_range_delete_list){
// #ifdef DEBUG
// cout << "insert ranges:" << endl;
// for(auto &p : inserted_range_delete_list){
//   cout << "(" << p.getStartKey() << " " << p.getEndKey() << ")" << " ";
// }
// cout << endl;
// #endif
//   auto& rdList = PerlevelRangeDeleteFilterByVector::range_delete_list;
//   auto& insertList = inserted_range_delete_list;

//   //check that rdList is in ascending order
//   for(int i = 1; i < rdList.size(); i++){
//     if(rdList[i-1].first > rdList[i].first){
//       cout << "Error: rdList is not in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
//       exit(1);
//     }
//   }

//   //check that insertList is in ascending order
//   for(int i = 1; i < insertList.size(); i++){
//     if(insertList[i-1].getStartKey() > insertList[i].getStartKey()){
//       cout << "Error: insertList is not in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
//       exit(1);
//     }
//   }

//   vector<pll> rdList_new;
  
//   vector<pll> &A = rdList;
//   vector<RangeEntry> &B = insertList;
//   auto itA = A.begin();
//   auto itB = B.begin();
//   while( (itA != A.end()) || (itB != B.end()) ){
//     long currentMin, currentMax;
//     if(itA == A.end()){
//       currentMin = itB->getStartKey();
//       currentMax = itB->getEndKey();
//       itB++;
//     }else if(itB == B.end()){
//       currentMin = itA->first;
//       currentMax = itA->second;
//       itA++;
//     }else if(itA->first <= itB->getStartKey()){
//       currentMin = itA->first;
//       currentMax = itA->second;
//       itA++;
//     }else{
//       currentMin = itB->getStartKey();
//       currentMax = itB->getEndKey();
//       itB++;
//     }

//     while(itA != A.end() || itB != B.end()){
//       if(itA != A.end() && itA->first <= currentMax+1){
//         currentMax = max(currentMax, itA->second);
//         itA++;
//         continue;
//       }
//       if(itB != B.end() && itB->getStartKey() <= currentMax+1){
//         currentMax = max(currentMax, itB->getEndKey());
//         itB++;
//         continue;
//       }
//       break;
//     }

//     rdList_new.push_back(pll({currentMin, currentMax}));
//   }

//   rdList.clear();
//   for(auto &p : rdList_new){
//     rdList.push_back(p);
//   }
// }


// void PerlevelRangeDeleteFilterByVector::removeRangeDelete(vector<RangeEntry> &range_delete_discard_list){

//   auto& rdList = this->PerlevelRangeDeleteFilterByVector::range_delete_list;
//   auto& discardList = range_delete_discard_list;
// #ifdef DEBUG
// cout << "discard ranges:" << endl;
// for(auto &p : discardList){
//   cout << "(" << p.getStartKey() << " " << p.getEndKey() << ")" << " ";
// }
// cout << endl;
// #endif
//   //check that rdList is in ascending order
//   for(int i = 1; i < rdList.size(); i++){
//     if(rdList[i-1].first > rdList[i].first){
//       cout << "Error: rdList is not in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
//       exit(1);
//     }
//   }
  

//   //check that discardList is in ascending order
//   for(int i = 1; i < discardList.size(); i++){
//     if(discardList[i-1].getStartKey() > discardList[i].getStartKey()){
//       cout << "Error: discardList is not in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
//       exit(1);
//     }
//   }

//   //check that rdList is non-overlapping
//   for(int i = 1; i < rdList.size(); i++){
//     if(rdList[i-1].second >= rdList[i].first){
//       cout << "Error: rdList is not non-overlapping yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
//       exit(1);
//     }
//   }

//   //check that discardList is non-overlapping
//   for(int i = 1; i < discardList.size(); i++){
//     if(discardList[i-1].getEndKey() > discardList[i].getStartKey()){
//       cout << "Error: discardList is not non-overlapping yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
//       exit(1);
//     }
//   }

//   auto it = rdList.begin();
 
//   auto ite = rdList.end();
//   auto it_discard = discardList.begin();
//   auto ite_discard = discardList.end();

//   vector<pll> rdList_new;
//   while( (it != ite) || (it_discard != ite_discard) ){
// #ifdef DEBUG
// if(it_discard != ite_discard){
//     for(auto x : rdList_new){
//       cout << "(" << x.first << " " << x.second << ") ";
//     }
//     cout << endl;
//     cout << "remove range: " << it_discard->getStartKey() << " " << it_discard->getEndKey() << endl;
//     // this->printRangeDeleteList();
// }
// #endif
//     if(it_discard == ite_discard){
//       rdList_new.push_back(*it);
//       it++;
//       continue;
//     }

//     if(it == ite){break;}

//     if(it->first > it_discard->getEndKey()){
//       it_discard++;
//       continue;
//     }

//     if(it->second < it_discard->getStartKey()){
//       rdList_new.push_back(*it);
//       it++;
//       continue;
//     }


//     //overlapped
//     if( (it->first == it_discard->getStartKey()) && (it->second == it_discard->getEndKey()) ){
//       it++;
//       it_discard++;
//       continue;
//     }

//     if( (it->first < it_discard->getStartKey()) && (it->second <= it_discard->getEndKey()) ){
//       rdList_new.push_back(pll({it->first, it_discard->getStartKey()-1}));
//       it++;
//       continue;
//     }

//     if( (it->first >= it_discard->getStartKey()) && (it->second <= it_discard->getEndKey()) ){
//       it++;
//       continue;
//     }

//     if( (it->first >= it_discard->getStartKey()) && (it->second > it_discard->getEndKey()) ){
//       it->first = it_discard->getEndKey() + 1;
//       it_discard++;
//       continue;
//     }

//     if( (it->first < it_discard->getStartKey()) && (it->second > it_discard->getEndKey()) ){
//       rdList_new.push_back(pll({it->first, it_discard->getStartKey()-1}));
//       it->first = it_discard->getEndKey() + 1;
//       it_discard++;
//       continue;
//     }

//     cout << "Error: should not reach here" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
//   }
// #ifdef DEBUG
//   for(auto x : rdList_new){
//     cout << "(" << x.first << " " << x.second << ") ";
//   }
//   cout << endl << endl << endl;
//   // this->printRangeDeleteList();
// #endif

//   rdList.clear();
//   for(auto &p : rdList_new){
//     rdList.push_back(p);
//   }
// }



// void PerlevelRangeDeleteFilterByVector::removeRangeDelete(vector<pll> &range_delete_discard_list){
//   auto& rdList = this->PerlevelRangeDeleteFilterByVector::range_delete_list;
//   auto& discardList = range_delete_discard_list;
// #ifdef DEBUG
// cout << "discard ranges:" << endl;
// for(auto &p : discardList){
//   cout << "(" << p.first << " " << p.second << ")" << " ";
// }
// cout << endl;
// #endif
//   //check that rdList is in ascending order
//   for(int i = 1; i < rdList.size(); i++){
//     if(rdList[i-1].first > rdList[i].first){
//       cout << "Error: rdList is not in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
//       exit(1);
//     }
//   }
  
  

//   //check that discardList is in ascending order
//   for(int i = 1; i < discardList.size(); i++){
//     if(discardList[i-1].first > discardList[i].first){
//       cout << "Error: discardList is not in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
//       exit(1);
//     }
//   }

//   //check that rdList is non-overlapping
//   for(int i = 1; i < rdList.size(); i++){
//     if(rdList[i-1].second >= rdList[i].first){
//       cout << "Error: rdList is not non-overlapping yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
//       exit(1);
//     }
//   }

//   //check that discardList is non-overlapping
//   for(int i = 1; i < discardList.size(); i++){
//     if(discardList[i-1].second > discardList[i].first){
//       cout << "Error: discardList is not non-overlapping yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
//       exit(1);
//     }
//   }

//   auto it = rdList.begin();
 
//   auto ite = rdList.end();
//   auto it_discard = discardList.begin();
//   auto ite_discard = discardList.end();

//   vector<pll> rdList_new;
//   while( (it != ite) || (it_discard != ite_discard) ){
// #ifdef DEBUG
// if(it_discard != ite_discard){
//     for(auto x : rdList_new){
//       cout << "(" << x.first << " " << x.second << ") ";
//     }
//     cout << endl;
//     cout << "remove range: " << it_discard->first << " " << it_discard->second << endl;
//     // this->printRangeDeleteList();
// }
// #endif
//     if(it_discard == ite_discard){
//       rdList_new.push_back(*it);
//       it++;
//       continue;
//     }

//     if(it == ite){break;}

//     if(it->first > it_discard->second){
//       it_discard++;
//       continue;
//     }

//     if(it->second < it_discard->first){
//       rdList_new.push_back(*it);
//       it++;
//       continue;
//     }


//     //overlapped
//     if( (it->first == it_discard->first) && (it->second == it_discard->second) ){
//       it++;
//       it_discard++;
//       continue;
//     }

//     if( (it->first < it_discard->first) && (it->second <= it_discard->second) ){
//       rdList_new.push_back(pll({it->first, it_discard->first-1}));
//       it++;
//       continue;
//     }

//     if( (it->first >= it_discard->first) && (it->second <= it_discard->second) ){
//       it++;
//       continue;
//     }

//     if( (it->first >= it_discard->first) && (it->second > it_discard->second) ){
//       it->first = it_discard->second + 1;
//       it_discard++;
//       continue;
//     }

//     if( (it->first < it_discard->first) && (it->second > it_discard->second) ){
//       rdList_new.push_back(pll({it->first, it_discard->first-1}));
//       it->first = it_discard->second + 1;
//       it_discard++;
//       continue;
//     }

//     cout << "Error: should not reach here" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
//   }
// #ifdef DEBUG
//   for(auto x : rdList_new){
//     cout << "(" << x.first << " " << x.second << ") ";
//   }
//   cout << endl << endl << endl;
//   // this->printRangeDeleteList();
// #endif

//   rdList.clear();
//   for(auto &p : rdList_new){
//     rdList.push_back(p);
//   }
// }

vector<pll> PerlevelRangeDeleteFilterByVector::getRangeDeleteList(){
  return PerlevelRangeDeleteFilterByVector::range_delete_list;
}



void PerlevelRangeDeleteFilterByVector::printRangeDeleteList(){
  auto& rdList = PerlevelRangeDeleteFilterByVector::range_delete_list;

  for(auto it = rdList.begin(); it != rdList.end(); it++){
    cout << "(" << it->first << " " << it->second << ") ";
  }
  cout << endl;
}

// //TODO: need to be optimized -> return vector<RangeEntry>
// EntryList* PerlevelRangeDeleteFilterByVector::applyRangeFilter(long start, long end){
//   EntryList *result = new EntryList(); //TODO
  
//   auto& rdList = PerlevelRangeDeleteFilterByVector::range_delete_list;

//   if(rdList.size() == 0){return result;}

//   vector<long> tmp_example;
//   auto its = upper_bound(rdList.begin(), rdList.end(), pll(start, start), [](const pll& a, const pll& b){return a.first < b.first;});
//   if(its != rdList.begin()){its--;}
//   auto ite = upper_bound(rdList.begin(), rdList.end(), pll(end, end), [](const pll& a, const pll& b){return a.first < b.first;});


//   long i_key = start;
//   auto it = its;
//   if(i_key >= its->first && i_key <= its->second){i_key = its->second + 1;}

//   while( true ){
//     while( (i_key < it->first) && (i_key <= end) ){
//       tmp_example.push_back(i_key);
//       i_key++;
//     }
//     if(i_key > end){break;}
//     if(i_key >= it->first && i_key <= it->second){
//       i_key = it->second + 1;
//     }
//     if(i_key > end){break;}
//     if(it == ite){break;}
//     it++;
//   }
//   while(i_key <= end){
//     tmp_example.push_back(i_key);
//     i_key++;
//   }


// #ifdef DEBUG
// cout << endl;
// cout << "start: " << start << " end: " << end  << endl;
// for(long x: tmp_example){
//   cout << x << " ";
// }
// cout << endl;
// #endif

//   return result;
// }


bool PerlevelRangeDeleteFilterByVector::isEntryAlive(long start){
  auto& rdList = PerlevelRangeDeleteFilterByVector::range_delete_list;
  if(rdList.size() == 0){return true;}

  auto it = upper_bound(rdList.begin(), rdList.end(), pll(start, start), [](const pll& a, const pll& b){return a.first < b.first;});
  if(it != rdList.begin()){it--;}
  if(start >= it->first && start <= it->second){return false;}
  return true;
}


int PerlevelRangeDeleteFilterByVector::getRangeDeleteCount(){
  return PerlevelRangeDeleteFilterByVector::range_delete_list.size();
}

// long PerlevelRangeDeleteFilterByVector::getIthRangeDeleteStartKey(int i){
//   return range_delete_list[i].first;
// }

// long PerlevelRangeDeleteFilterByVector::getIthRangeDeleteEndKey(int i){
//   return range_delete_list[i].second;
// }

// void PerlevelRangeDeleteFilterByVector::splitRangeByPointEntries(vector<PointEntry> &point_list){
//   // range_delete_list
//   // point_list

//   auto& rdList = PerlevelRangeDeleteFilterByVector::range_delete_list;
//   auto& pList = point_list;

//   vector<pll> rdList_new;

//   int i = 0;
//   auto it_points = pList.begin();
//   auto ite_points = pList.end();

//   auto it_ranges = rdList.begin();
//   auto ite_ranges = rdList.end();

//   while( (it_points != ite_points) && (it_ranges != ite_ranges) ){
//     long startKey = it_ranges->first;
//     long endKey = it_ranges->second;
//     long sortKey = it_points->getSortKey();

//     if(sortKey < startKey){
//       it_points++;
//       continue;
//     }

//     if(sortKey > endKey){
//       rdList_new.push_back(pll(startKey, endKey));
//       it_ranges++;
//       continue;
//     }
//     // have overlapping
//     if((sortKey == startKey) && (sortKey == endKey)){
//       it_points++;
//       it_ranges++;
//       continue;
//     }

//     if(sortKey == startKey){
//       it_ranges->first = sortKey+1;
//       it_points++;
//       continue;
//     }

//     if(sortKey == endKey){
//       rdList_new.push_back(pll(startKey, endKey-1));
//       it_ranges++;
//       continue;
//     }

//     if(sortKey > startKey && sortKey < endKey){

//       rdList_new.push_back(pll(startKey, sortKey-1));
//       it_ranges->first = sortKey+1;
//       it_points++;
//       continue;
//     }

//   }
 
//   while(it_ranges != ite_ranges){
//     rdList_new.push_back(pll(it_ranges->first, it_ranges->second));
//     it_ranges++;
//   }

//   rdList.clear();
//   for(auto &p : rdList_new){
//     rdList.push_back(p);
//   }

//   return;
// }

