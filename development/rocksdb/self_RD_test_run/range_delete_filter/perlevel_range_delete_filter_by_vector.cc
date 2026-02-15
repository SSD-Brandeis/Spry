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