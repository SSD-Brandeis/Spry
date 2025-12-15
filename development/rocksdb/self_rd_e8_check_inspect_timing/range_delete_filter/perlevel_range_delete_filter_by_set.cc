#include "perLevel_range_delete_filter.h"

using namespace std;
using namespace range_delete_filter;


void PerlevelRangeDeleteFilterBySet::addRangeDelete(long start, long end){
    auto& rdList = PerlevelRangeDeleteFilterBySet::range_delete_list;
#ifdef DEBUG
    cout << "Adding range delete: " << start << " " << end << endl;
#endif
    vector<pll> to_be_added;
    long minK = start;
    long maxK = end;

    auto it = rdList.upper_bound(pll(start, start));
    if(it != rdList.begin()){it--;}

    while (it != rdList.end()){
        if (it->second < start-1){ it++; continue;}
        if (it->first > end+1){ break;}

        minK = min(minK, it->first);
        maxK = max(maxK, it->second);
        auto it2 = it;
        it2++;
        rdList.erase(it);
        it = it2;
    }
    rdList.insert(it, pll({minK, maxK}));
}

void PerlevelRangeDeleteFilterBySet::addRangeDelete(vector<RangeEntry> &inserted_range_delete_list){
#ifdef DEBUG
cout << "insert ranges:" << endl;
for(auto &p : inserted_range_delete_list){
  cout << "(" << p.getStartKey() << " " << p.getEndKey() << ")" << " ";
}
cout << endl;
#endif
  auto& rdList = PerlevelRangeDeleteFilterBySet::range_delete_list;
  auto& insertList = inserted_range_delete_list;


  //check that rdList is in ascending order
  if(rdList.size() > 0){
    auto it_check = rdList.begin();
    auto it_check2 = rdList.begin();
    it_check2++;
    while(it_check2 != rdList.end()){
      if(it_check->first > it_check2->first){
        cout << "Error: rdList is not in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
        exit(1);
      }
      it_check++;
      it_check2++;
    }
  }

  //check that insertList is in ascending order
  for(int i = 1; i < insertList.size(); i++){
    if(insertList[i-1].getStartKey() > insertList[i].getStartKey()){
      cout << "Error: insertList is not in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }

  //check that rdList is non-overlapping (because we don't iterate through all the element in rdList)
  if(rdList.size() > 0){
    auto it_check = rdList.begin();
    auto it_check2 = rdList.begin();
    it_check2++;
    while(it_check2 != rdList.end()){
      if(it_check->second > it_check2->first){
        cout << "Error: rdList is not non-overlapping yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
        exit(1);
      }
      it_check++;
      it_check2++;
    }
  }

  vector<pll> rdList_new;
  
  set<pll> &A = rdList;
  vector<RangeEntry> &B = insertList;
  auto itA = A.begin();
  auto itB = B.begin();
  while( (itA != A.end()) || (itB != B.end()) ){
    long currentMin, currentMax;
    if(itA == A.end()){
      currentMin = itB->getStartKey();
      currentMax = itB->getEndKey();

    }else if(itB == B.end()){

      break;
    }else if(itA->first > itB->getEndKey()+1){
      itB++;
      continue;
    }else if(itA->second < itB->getStartKey()-1){
      itA++;
      continue;
    }else if(itA->first <= itB->getStartKey()){
      currentMin = itA->first;
      currentMax = itA->second;
    }else{
      currentMin = itB->getStartKey();
      currentMax = itB->getEndKey();
    }

    while(itA != A.end() || itB != B.end()){
      if(itA != A.end() && itA->first <= currentMax+1){
        currentMax = max(currentMax, itA->second);
        itA = A.erase(itA);
        continue;
      }
      if(itB != B.end() && itB->getStartKey() <= currentMax+1){
        currentMax = max(currentMax, itB->getEndKey());
        itB++;
        continue;
      }
      break;
    }



    rdList_new.push_back(pll({currentMin, currentMax}));
  }

  for(auto &p : rdList_new){
    rdList.insert(p);
  }
}



void PerlevelRangeDeleteFilterBySet::removeRangeDelete(vector<RangeEntry> &range_delete_discard_list){

  auto& rdList = this->PerlevelRangeDeleteFilterBySet::range_delete_list;
  auto& discardList = range_delete_discard_list;
#ifdef DEBUG
cout << "discard ranges:" << endl;
for(auto &p : discardList){
  cout << "(" << p.getStartKey() << " " << p.getEndKey() << ")" << " ";
}
cout << endl;
#endif
  //check that rdList is in ascending order
  if(rdList.size() > 0){
    auto it_check = rdList.begin();
    auto it_check2 = rdList.begin();
    it_check2++;
    while(it_check2 != rdList.end()){
      if(it_check->first > it_check2->first){
        cout << "Error: rdList is not in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
        exit(1);
      }
      it_check++;
      it_check2++;
    }
  }

  //check that discardList is in ascending order
  for(int i = 1; i < discardList.size(); i++){
    if(discardList[i-1].getStartKey() > discardList[i].getStartKey()){
      cout << "Error: discardList is not in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }

  //check that rdList is non-overlapping
  if(rdList.size() > 0){
    auto it_check = rdList.begin();
    auto it_check2 = rdList.begin();
    it_check2++;
    while(it_check2 != rdList.end()){
      if(it_check->second > it_check2->first){
        cout << "Error: rdList is not non-overlapping yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
        exit(1);
      }
      it_check++;
      it_check2++;
    }
  }

  //check that discardList is non-overlapping
  for(int i = 1; i < discardList.size(); i++){
    if(discardList[i-1].getEndKey() > discardList[i].getStartKey()){
      cout << "Error: discardList is not non-overlapping yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }

  auto it = rdList.begin();
  if(discardList.size() > 0){
    it = rdList.upper_bound(pll(discardList.begin()->getStartKey(), discardList.begin()->getStartKey()));
    if(it != rdList.begin()){it--;}
  }
  auto ite = rdList.end();
  auto it_discard = discardList.begin();
  auto ite_discard = discardList.end();

  vector<pll> rdList_new;
  while( (it != ite) || (it_discard != ite_discard) ){
#ifdef DEBUG
if(it_discard != ite_discard){
    for(auto x : rdList_new){
      cout << "(" << x.first << " " << x.second << ") ";
    }
    cout << endl;
    cout << "remove range: " << it_discard->getStartKey() << " " << it_discard->getEndKey() << endl;
}
#endif
    if(it_discard == ite_discard){

      break;
    }

    if(it == ite){break;}

    if(it->first > it_discard->getEndKey()){
      it_discard++;
      continue;
    }

    if(it->second < it_discard->getStartKey()){
      it++;
      continue;
    }


    //overlapped
    if( (it->first == it_discard->getStartKey()) && (it->second == it_discard->getEndKey()) ){
      it = rdList.erase(it);
      it_discard++;
      continue;
    }

    if( (it->first < it_discard->getStartKey()) && (it->second <= it_discard->getEndKey()) ){
      rdList_new.push_back(pll({it->first, it_discard->getStartKey()-1}));
      it = rdList.erase(it);
      continue;
    }

    if( (it->first >= it_discard->getStartKey()) && (it->second <= it_discard->getEndKey()) ){
      it = rdList.erase(it);
      continue;
    }

    if( (it->first >= it_discard->getStartKey()) && (it->second > it_discard->getEndKey()) ){

      long a = it_discard->getEndKey() + 1;
      long b = it->second;
      while( (it_discard != ite_discard) && (b > it_discard->getEndKey()) ){
        if(it_discard->getStartKey() > a){
          rdList_new.push_back(pll({a, it_discard->getStartKey()-1}));
        }
        a = it_discard->getEndKey() + 1;
        it_discard++;
#ifdef DEBUG
if(it_discard != ite_discard){
    cout << "remove range: " << it_discard->getStartKey() << " " << it_discard->getEndKey() << endl;
}
#endif
      }
      if(it_discard != ite_discard){b = min(b, it_discard->getStartKey()-1);}
      if(a <= b){
        rdList_new.push_back(pll({a, b}));
      }
      it = rdList.erase(it);
      continue;
    }

    if( (it->first < it_discard->getStartKey()) && (it->second > it_discard->getEndKey()) ){
      rdList_new.push_back(pll({it->first, it_discard->getStartKey()-1}));

      long a = it_discard->getEndKey() + 1;
      long b = it->second;
      while( (it_discard != ite_discard) && (b > it_discard->getEndKey()) ){
        if(it_discard->getStartKey() > a){
          rdList_new.push_back(pll({a, it_discard->getStartKey()-1}));
        }
        a = it_discard->getEndKey() + 1;
        it_discard++;
#ifdef DEBUG
if(it_discard != ite_discard){
    cout << "remove range: " << it_discard->getStartKey() << " " << it_discard->getEndKey() << endl;
}
#endif
      }
      if(it_discard != ite_discard){b = min(b, it_discard->getStartKey()-1);}
      if(a <= b){
        rdList_new.push_back(pll({a, b}));
      }
      it = rdList.erase(it);
      continue;
    }

    cout << "Error: should not reach here" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
  }
#ifdef DEBUG
cout << "rdList_new:" << endl;
  for(auto x : rdList_new){
    cout << "(" << x.first << " " << x.second << ") ";
  }
  cout << endl << endl << endl;
  // this->printRangeDeleteList();
#endif

  for(auto &p : rdList_new){
    rdList.insert(p);
  }

#ifdef DEBUG
cout << "rdlist: " << endl;
  this->printRangeDeleteList();
#endif
}


void PerlevelRangeDeleteFilterBySet::removeRangeDelete(vector<pll> &range_delete_discard_list){

  auto& rdList = this->PerlevelRangeDeleteFilterBySet::range_delete_list;
  auto& discardList = range_delete_discard_list;
#ifdef DEBUG
cout << "discard ranges:" << endl;
for(auto &p : discardList){
  cout << "(" << p.first << " " << p.second << ")" << " ";
}
cout << endl;
#endif
  //check that rdList is in ascending order
  if(rdList.size() > 0){
    auto it_check = rdList.begin();
    auto it_check2 = rdList.begin();
    it_check2++;
    while(it_check2 != rdList.end()){
      if(it_check->first > it_check2->first){
        cout << "Error: rdList is not in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
        exit(1);
      }
      it_check++;
      it_check2++;
    }
  }

  //check that discardList is in ascending order
  for(int i = 1; i < discardList.size(); i++){
    if(discardList[i-1].first > discardList[i].first){
      cout << "Error: discardList is not in ascending order yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }

  //check that rdList is non-overlapping
  if(rdList.size() > 0){
    auto it_check = rdList.begin();
    auto it_check2 = rdList.begin();
    it_check2++;
    while(it_check2 != rdList.end()){
      if(it_check->second > it_check2->first){
        cout << "Error: rdList is not non-overlapping yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
        exit(1);
      }
      it_check++;
      it_check2++;
    }
  }

  //check that discardList is non-overlapping
  for(int i = 1; i < discardList.size(); i++){
    if(discardList[i-1].second > discardList[i].first){
      cout << "Error: discardList is not non-overlapping yet" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
      exit(1);
    }
  }



  auto it = rdList.begin();
  if(discardList.size() > 0){
    it = rdList.upper_bound(pll(discardList.begin()->first, discardList.begin()->first));
    if(it != rdList.begin()){it--;}
  }
  auto ite = rdList.end();
  auto it_discard = discardList.begin();
  auto ite_discard = discardList.end();

  vector<pll> rdList_new;
  while( (it != ite) || (it_discard != ite_discard) ){
#ifdef DEBUG
if(it_discard != ite_discard){
    for(auto x : rdList_new){
      cout << "(" << x.first << " " << x.second << ") ";
    }
    cout << endl;
    cout << "remove range: " << it_discard->first << " " << it_discard->second << endl;
    // this->printRangeDeleteList();
}
#endif
    if(it_discard == ite_discard){
      break;
    }

    if(it == ite){break;}

    if(it->first > it_discard->second){
      it_discard++;
      continue;
    }

    if(it->second < it_discard->first){
      it++;
      continue;
    }


    //overlapped
    if( (it->first == it_discard->first) && (it->second == it_discard->second) ){
      it = rdList.erase(it);
      it_discard++;
      continue;
    }

    if( (it->first < it_discard->first) && (it->second <= it_discard->second) ){
      rdList_new.push_back(pll({it->first, it_discard->first-1}));
      it = rdList.erase(it);
      continue;
    }

    if( (it->first >= it_discard->first) && (it->second <= it_discard->second) ){
      it = rdList.erase(it);;
      continue;
    }

    if( (it->first >= it_discard->first) && (it->second > it_discard->second) ){
      long a = it_discard->second + 1;
      long b = it->second;
      while( (it_discard != ite_discard) && (b > it_discard->second) ){
        if(it_discard->first > a){
          rdList_new.push_back(pll({a, it_discard->first-1}));
        }
        a = it_discard->second + 1;
        it_discard++;
#ifdef DEBUG
if(it_discard != ite_discard){
    cout << "remove range: " << it_discard->first << " " << it_discard->second << endl;
}
#endif
      }
      if(it_discard != ite_discard){b = min(b, it_discard->first-1);}
      if(a <= b){
        rdList_new.push_back(pll({a, b}));
      }
      it = rdList.erase(it);
      continue;
    }


    if( (it->first < it_discard->first) && (it->second > it_discard->second) ){
      rdList_new.push_back(pll({it->first, it_discard->first-1}));

      long a = it_discard->second + 1;
      long b = it->second;
      while( (it_discard != ite_discard) && (b > it_discard->second) ){
        if(it_discard->first > a){
          rdList_new.push_back(pll({a, it_discard->first-1}));
        }
        a = it_discard->second + 1;
        it_discard++;
#ifdef DEBUG
if(it_discard != ite_discard){
    cout << "remove range: " << it_discard->first << " " << it_discard->second << endl;
}
#endif
      }
      if(it_discard != ite_discard){b = min(b, it_discard->first-1);}
      if(a <= b){
        rdList_new.push_back(pll({a, b}));
      }
      it = rdList.erase(it);
      continue;
    }

    cout << "Error: should not reach here" << "\t" << __FILE__ << " " << __LINE__ << " " << __func__ << endl;
  }
#ifdef DEBUG
cout << "rdList_new: ";
  for(auto x : rdList_new){
    cout << "(" << x.first << " " << x.second << ") ";
  }
  cout << endl << endl << endl;
  // this->printRangeDeleteList();
#endif

  for(auto &p : rdList_new){
    rdList.insert(p);
  }
#ifdef DEBUG
cout << "rdlist: " << endl;
  this->printRangeDeleteList();
#endif
}


set<pll> PerlevelRangeDeleteFilterBySet::getRangeDeleteList(){
  return PerlevelRangeDeleteFilterBySet::range_delete_list;
}



void PerlevelRangeDeleteFilterBySet::printRangeDeleteList(){
  auto& rdList = PerlevelRangeDeleteFilterBySet::range_delete_list;

  for(auto it = rdList.begin(); it != rdList.end(); it++){
    cout << "(" << it->first << " " << it->second << ") ";
  }
  cout << endl;
}

//TODO: need to be optimized -> return vector<RangeEntry>
EntryList* PerlevelRangeDeleteFilterBySet::applyRangeFilter(long start, long end){
  auto& rdList = PerlevelRangeDeleteFilterBySet::range_delete_list;

  EntryList *result = new EntryList();
  
  if(rdList.size() == 0){return result;}

  vector<long> tmp_example;
  auto its = rdList.upper_bound(pll(start, start));
  if(its != rdList.begin()){its--;}
  auto ite = rdList.upper_bound(pll(end, end));


  long i_key = start;
  auto it = its;
  if(i_key >= its->first && i_key <= its->second){i_key = its->second + 1;}

  while( true ){
    while( (i_key < it->first) && (i_key <= end) ){
      tmp_example.push_back(i_key);
      i_key++;
    }
    if(i_key > end){break;}
    if(i_key >= it->first && i_key <= it->second){
      i_key = it->second + 1;
    }
    if(i_key > end){break;}
    if(it == ite){break;}
    it++;
  }
  while(i_key <= end){
    tmp_example.push_back(i_key);
    i_key++;
  }


#ifdef DEBUG
cout << endl;
cout << "start: " << start << " end: " << end  << endl;
for(long x: tmp_example){
  cout << x << " ";
}
cout << endl;
#endif

  return result;
}


bool PerlevelRangeDeleteFilterBySet::isEntryAlive(long start){
  auto& rdList = PerlevelRangeDeleteFilterBySet::range_delete_list;
  if(rdList.size() == 0){return true;}

  auto it = rdList.upper_bound(pll(start, start));
  if(it != rdList.begin()){it--;}
  if(start >= it->first && start <= it->second){return false;}
  return true;
}

int PerlevelRangeDeleteFilterBySet::getRangeDeleteCount(){
  return PerlevelRangeDeleteFilterBySet::range_delete_list.size();
}




