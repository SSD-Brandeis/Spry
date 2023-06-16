#include "skyline_range_delete_filter.h"

using namespace std;
using namespace range_delete_filter;


// void SkylineRangeDeleteFilter::addRangeDelete(long start, long end){
// #ifdef DEBUG
//     cout << "Adding range delete: " << start << " " << end << endl;
// #endif
//     // auto its = SkylineRangeDeleteFilter::range_delete_list.begin();
//     vector<pll> to_be_added;
//     for (auto it = SkylineRangeDeleteFilter::range_delete_list.begin(); it != SkylineRangeDeleteFilter::range_delete_list.end(); ){
//         if (it->second <= start){
//             // its = it;
//             it++;
//             continue;
//         }else if (it->first >= end){
//             // ite = it;
//             break;
//         }

//         //this entry is larger than the inserted entry
//         if(it->first < start && it->second > end){
//           int s1 = it->first, e1 = start;
//           int s2 = end, e2 = it->second;
//           SkylineRangeDeleteFilter::range_delete_list.erase(it++);
//           to_be_added.push_back(pll(s1, e1));
//           to_be_added.push_back(pll(s2, e2));
//           continue;
//         }

//         //partial overlap
//         //at most one entry will be in the range (at head)
//         if(it->first < start && it->second <= end){
//             long s1 = it->first, e1 = start;
//             SkylineRangeDeleteFilter::range_delete_list.erase(it++);
//             to_be_added.push_back(pll(s1, e1));
//             continue;
//         }

//         //partial overlap
//         //at most one entry will be in the range (at tail)
//         if(it->first >= start && it->second > end){
//             long s1 = end, e1 = it->second;
//             SkylineRangeDeleteFilter::range_delete_list.erase(it++);
//             to_be_added.push_back(pll(s1, e1));
//             continue;
//         }

//         //complete overlap
//         //when there is full overlap (it->first >= start && it->second <= end)
//         if(it->first >= start && it->second <= end){
//             SkylineRangeDeleteFilter::range_delete_list.erase(it++);
//             continue;
//         }

//     }

//     SkylineRangeDeleteFilter::range_delete_list.insert(pll(start, end));
//     for(auto rd_entry: to_be_added){
//         SkylineRangeDeleteFilter::range_delete_list.insert(rd_entry);
//     }
// }



//TODO: implement this function
void SkylineRangeDeleteFilter::addRangeDeleteWithTimetag(long start, long end, long timetag){
  
#ifdef DEBUG
    cout << "Adding range delete with timetag: " <<  start << " " << end << " @" << timetag << endl;
#endif

  vector<range_delete_entry> to_be_added;
  auto it = SkylineRangeDeleteFilter::range_delete_with_timetag_list.begin();
  auto ite = SkylineRangeDeleteFilter::range_delete_with_timetag_list.end();
  while(it != ite){
    if(it->end <= start){
      it++;
      continue;
    }else if(it->start >= end){
      break;
    }

    //this entry is larger than the inserted entry
    if(it->start < start && it->end > end){
      int s1 = it->start, e1 = start, t1 = it->timetag;
      int s3 = end, e3 = it->end, t3 = it->timetag;
      int timetag = it->timetag;

      auto it2 = it;
      it2++;
      SkylineRangeDeleteFilter::range_delete_with_timetag_list.erase(it);
      it = it2;

      to_be_added.push_back(range_delete_entry({s1, e1, t1}));
      to_be_added.push_back(range_delete_entry({s3, e3, t3}));
      break;
    }

    //partial overlap
    //at most one entry will be in the range (at head)
    if(it->start < start && it->end <= end){
      long s1 = it->start, e1 = start, t1 = it->timetag;
      
      auto it2 = it;
      it2++;
      SkylineRangeDeleteFilter::range_delete_with_timetag_list.erase(it);
      it = it2;

      to_be_added.push_back(range_delete_entry({s1, e1, t1}));
      continue;
    }

    //partial overlap
    //at most one entry will be in the range (at tail)
    if(it->start >= start && it->end > end){
      long s1 = end, e1 = it->end, t1 = it->timetag;
      auto it2 = it;
      it2++;
      SkylineRangeDeleteFilter::range_delete_with_timetag_list.erase(it);
      it = it2;

      to_be_added.push_back(range_delete_entry({s1, e1, t1}));
      // continue;
      break;
    }

    //complete overlap
    //when there is full overlap (it->start >= start && it->end <= end)
    if(it->start >= start && it->end <= end){
      auto it2 = it;
      it2++;
      SkylineRangeDeleteFilter::range_delete_with_timetag_list.erase(it);
      it = it2;
      continue;
    }
  }
  SkylineRangeDeleteFilter::range_delete_with_timetag_list.insert(range_delete_entry({start, end, timetag}));
  for(auto rd_entry: to_be_added){
    SkylineRangeDeleteFilter::range_delete_with_timetag_list.insert(rd_entry);
  }
  
}

// set<pll> SkylineRangeDeleteFilter::getRangeDeleteList(){
//   return SkylineRangeDeleteFilter::range_delete_list;
// }

set<range_delete_entry> SkylineRangeDeleteFilter::getRangeDeleteWithTimetagList(){
  return SkylineRangeDeleteFilter::range_delete_with_timetag_list;
}



// void SkylineRangeDeleteFilter::printRangeDeleteList(){
//   for(auto it = SkylineRangeDeleteFilter::range_delete_list.begin(); it != SkylineRangeDeleteFilter::range_delete_list.end(); it++){
//     cout << "(" << it->first << " " << it->second << ") ";
//   }
//   cout << endl;
// }

void SkylineRangeDeleteFilter::printRangeDeleteWithTimetagList(){
  for(auto it = SkylineRangeDeleteFilter::range_delete_with_timetag_list.begin(); it != SkylineRangeDeleteFilter::range_delete_with_timetag_list.end(); it++){
    cout << "(" << it->start << " " << it->end << "  @" << it->timetag << ") ";
  }
  cout << endl;
}


// //TODO: filter input entry list
// // EntryList apply_range_filter_without_timetag(long start, long end, EntryList &input){
// EntryList* SkylineRangeDeleteFilter::applyRangeFilterWithoutTimetag(long start, long end){
//   EntryList *result = new EntryList();
  
//   if(SkylineRangeDeleteFilter::range_delete_list.size() == 0){return result;}

//   vector<long> tmp_example;
//   auto its = SkylineRangeDeleteFilter::range_delete_list.upper_bound(pll(start, start));
//   if(its != SkylineRangeDeleteFilter::range_delete_list.begin()){its--;}
//   auto ite = SkylineRangeDeleteFilter::range_delete_list.upper_bound(pll(end, end));

//   // if(its == SkylineRangeDeleteFilter::range_delete_list.end()){return all range elements;}

//   long i_key = start;
//   auto it = its;
//   if(i_key >= its->first && i_key <= its->second){i_key = its->second + 1;}

//   while( true ){
//     while( (i_key < it->first) && (i_key <= end) ){
//       tmp_example.push_back(i_key); 
//       //TODO: add entry to result
//       i_key++;
//       continue;
//     }
//     if(i_key > end){break;}

//     i_key = it->second + 1;

//     it++;
//     if(it == ite){break;}
//   }

//   for(;i_key <= end; i_key++){
//     tmp_example.push_back(i_key);
//     //TODO: add entry to result
//   }

// #ifdef DEBUG
// cout << endl;
// cout << "start: " << start << " end: " << end << endl;
// for(long x: tmp_example){
//   cout << x << " ";
// }
// cout << endl;
// #endif

//   return result;
// }

//TODO: filter input entry list
// EntryList apply_range_filter_with_timetag(long start, long end, long timetag, EntryList &input){
EntryList* SkylineRangeDeleteFilter::applyRangeFilterWithTimetag(long start, long end, long timetag){


  EntryList *result = new EntryList();
  
  if(SkylineRangeDeleteFilter::range_delete_with_timetag_list.size() == 0){return result;}

  vector<long> tmp_example;
  auto its = SkylineRangeDeleteFilter::range_delete_with_timetag_list.upper_bound(range_delete_entry({start, start, timetag}));
  if(its != SkylineRangeDeleteFilter::range_delete_with_timetag_list.begin()){its--;}
  auto ite = SkylineRangeDeleteFilter::range_delete_with_timetag_list.upper_bound(range_delete_entry({end, end, timetag}));

  // if(its == SkylineRangeDeleteFilter::range_delete_list.end()){return all range elements;}

  long i_key = start;
  auto it = its;
  // if(i_key >= its->first && i_key <= its->second){i_key = its->second + 1;}
  if(i_key >= its->start && i_key <= its->end){
    for(;(i_key < its->end) && (i_key <= end); i_key++){
      //TODO: if entry time tag is lesser or equal to  range delete time tag, than discard the entry
      // if(true/*entry time tag <= its->timetag*/){continue;}
      if(timetag <= its->timetag){continue;}
      tmp_example.push_back(i_key);
      //TODO: add entry to result
    }
    //@i_key == end, if timetag is lesser or equal to range delete timetag, then discard the entry
    //otherwise it has to check with the start of the next range delete
    //(last entry of the range delete may be the start of the next range delete but with different timetag)
    if( (i_key == its->end) && (timetag <= its->timetag)){ i_key ++;}

    // i_key = its->second + 1;
  }

  while( true ){
    while( (i_key < it->start) && (i_key <= end) ){
      tmp_example.push_back(i_key); 
      //TODO: add entry to result
      i_key++;
      continue;
    }
    if(i_key > end){break;}

    for(;(i_key < it->end) && (i_key <= end); i_key++){
      //TODO: if entry time tag is lesser or equal to  range delete time tag, than discard the entry
      // if(true/*entry time tag <= its->timetag*/){continue;}
      if(timetag <= it->timetag){continue;}
      tmp_example.push_back(i_key);
      //TODO: add entry to result
    }
    //@i_key == end, if timetag is lesser or equal to range delete timetag, then discard the entry
    //otherwise it has to check with the start of the next range delete
    //(last entry of the range delete may be the start of the next range delete but with different timetag)
    if( (i_key == it->end) && (timetag <= it->timetag)){ i_key ++;}

    it++;
    if(it == ite){break;}
  }

  for(;i_key <= end; i_key++){
    tmp_example.push_back(i_key);
    //TODO: add entry to result
  }

#ifdef DEBUG
cout << endl;
cout << "start: " << start << " end: " << end << " timetag: " << timetag << endl;
for(long x: tmp_example){
  cout << x << " ";
}
cout << endl;
#endif
  
  return result;


}


// bool SkylineRangeDeleteFilter::isEntryAliveWithoutTimetag(long start){
//   //TODO 
//   //start = entry->key;

//   auto it = SkylineRangeDeleteFilter::range_delete_list.upper_bound(pll(start, start));
//   if(it != SkylineRangeDeleteFilter::range_delete_list.begin()){it--;}
//   if(start >= it->first && start <= it->second){return false;}
//   return true;
// }

bool SkylineRangeDeleteFilter::isEntryAliveWithTimetag(long start, long timetag){


  auto it = SkylineRangeDeleteFilter::range_delete_with_timetag_list.lower_bound(range_delete_entry({start, start, timetag}));

  auto its = SkylineRangeDeleteFilter::range_delete_with_timetag_list.begin();  
  auto ite = SkylineRangeDeleteFilter::range_delete_with_timetag_list.end();

  //check when start == it->start
  if( (it != ite) && (it->start == start) ){
    
    if(it == its){return timetag > it->timetag;}
    if(timetag <= it->timetag){return false;}
    //move back another one step   (end of the prvious range delete may be the start of the next range delete)
    it--;
    if(start > it->end){return true;}
    return timetag > it->timetag;
  }

  
  //Otherwise needs to move back one step
  if(it == its){return true;}

  it--;
  //not overlap with any range delete
  if(start > it->end){return true;}

 
  return timetag > it->timetag;
}

int SkylineRangeDeleteFilter::getRangeDeleteWithTimetagCount(){
  return SkylineRangeDeleteFilter::range_delete_with_timetag_list.size();
}
