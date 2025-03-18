#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <set>
#include "../emu_environment.h"
#include "../workload_executor.h"
#include "skyline_range_delete_filter.cc"

using namespace std;
using namespace range_delete_filter;


// void test_skyline_range_delete_filter();
void test_skyline_range_delete_with_timetag_filter();


int main(int argc, char** argv){
    // test_skyline_range_delete_filter();
    // cout << endl;
    test_skyline_range_delete_with_timetag_filter();
    return 0;
}

// void test_skyline_range_delete_filter(){
    

//     SkylineRangeDeleteFilter* skyline_range_delete_filter = new SkylineRangeDeleteFilter();

//     skyline_range_delete_filter->addRangeDelete(1, 10);

//     skyline_range_delete_filter->addRangeDelete(2, 5);
 
//     skyline_range_delete_filter->addRangeDelete(3, 4);

//     skyline_range_delete_filter->addRangeDelete(4, 5);

//     skyline_range_delete_filter->addRangeDelete(5, 6);

//     skyline_range_delete_filter->addRangeDelete(6, 7);
//     skyline_range_delete_filter->printRangeDeleteList();

//     skyline_range_delete_filter->addRangeDelete(31, 41);
//     skyline_range_delete_filter->printRangeDeleteList();

//     skyline_range_delete_filter->addRangeDelete(25, 35);
//     skyline_range_delete_filter->printRangeDeleteList();

//     skyline_range_delete_filter->addRangeDelete(37, 43);
//     skyline_range_delete_filter->printRangeDeleteList();

//     skyline_range_delete_filter->addRangeDelete(32, 38);
//     skyline_range_delete_filter->printRangeDeleteList();
    
//     skyline_range_delete_filter->addRangeDelete(12, 21);
//     skyline_range_delete_filter->printRangeDeleteList();

//     skyline_range_delete_filter->addRangeDelete(14, 17);
//     skyline_range_delete_filter->printRangeDeleteList();

//     skyline_range_delete_filter->applyRangeFilterWithoutTimetag(1, 10);
//     skyline_range_delete_filter->applyRangeFilterWithoutTimetag(2, 5);
//     skyline_range_delete_filter->applyRangeFilterWithoutTimetag(13, 21);
//     skyline_range_delete_filter->applyRangeFilterWithoutTimetag(13, 24);
//     skyline_range_delete_filter->applyRangeFilterWithoutTimetag(11, 21);
//     skyline_range_delete_filter->applyRangeFilterWithoutTimetag(11, 24);
//     skyline_range_delete_filter->applyRangeFilterWithoutTimetag(11, 44);
//     skyline_range_delete_filter->applyRangeFilterWithoutTimetag(-2, 44);

//     cout << "point search " << endl;
//     cout << "1 " << skyline_range_delete_filter->isEntrySurvivedWithoutTimetag(1) << endl;
//     cout << "21 " << skyline_range_delete_filter->isEntrySurvivedWithoutTimetag(21) << endl;
//     cout << "11 " << skyline_range_delete_filter->isEntrySurvivedWithoutTimetag(11) << endl;
// }

void test_skyline_range_delete_with_timetag_filter(){
        
        SkylineRangeDeleteFilter* skyline_range_delete_filter = new SkylineRangeDeleteFilter();
    
        skyline_range_delete_filter->addRangeDeleteWithTimetag(1, 10, 1);
        skyline_range_delete_filter->printRangeDeleteWithTimetagList();
    
        skyline_range_delete_filter->addRangeDeleteWithTimetag(2, 5, 2);
        skyline_range_delete_filter->printRangeDeleteWithTimetagList();
     
        skyline_range_delete_filter->addRangeDeleteWithTimetag(3, 4, 3);
        skyline_range_delete_filter->printRangeDeleteWithTimetagList();
    
        skyline_range_delete_filter->addRangeDeleteWithTimetag(4, 5, 4);
        skyline_range_delete_filter->printRangeDeleteWithTimetagList();
    
        skyline_range_delete_filter->addRangeDeleteWithTimetag(5, 6, 5);
        skyline_range_delete_filter->printRangeDeleteWithTimetagList();
    
        skyline_range_delete_filter->addRangeDeleteWithTimetag(6, 7, 6);
        skyline_range_delete_filter->printRangeDeleteWithTimetagList();
    
        skyline_range_delete_filter->addRangeDeleteWithTimetag(31, 41, 7);
        skyline_range_delete_filter->printRangeDeleteWithTimetagList();
    
        skyline_range_delete_filter->addRangeDeleteWithTimetag(25, 35, 8);
        skyline_range_delete_filter->printRangeDeleteWithTimetagList();
    
        skyline_range_delete_filter->addRangeDeleteWithTimetag(37, 43, 9);
        skyline_range_delete_filter->printRangeDeleteWithTimetagList();
    
        skyline_range_delete_filter->addRangeDeleteWithTimetag(32, 38, 10);
        skyline_range_delete_filter->printRangeDeleteWithTimetagList();
        
        skyline_range_delete_filter->addRangeDeleteWithTimetag(12, 21, 11);
        skyline_range_delete_filter->printRangeDeleteWithTimetagList();
    
        skyline_range_delete_filter->addRangeDeleteWithTimetag(14, 17, 12);
        skyline_range_delete_filter->printRangeDeleteWithTimetagList();
        
        printf("Test applyRangeFilterWithTimetag");
        skyline_range_delete_filter->applyRangeFilterWithTimetag(1, 10, 12);
        skyline_range_delete_filter->applyRangeFilterWithTimetag(2, 5, 15);
        skyline_range_delete_filter->applyRangeFilterWithTimetag(13, 21, 10);
        skyline_range_delete_filter->applyRangeFilterWithTimetag(13, 24, 10);
        skyline_range_delete_filter->applyRangeFilterWithTimetag(11, 21, 10);
        skyline_range_delete_filter->applyRangeFilterWithTimetag(11, 24, 10);
        skyline_range_delete_filter->applyRangeFilterWithTimetag(11, 44, 10);
        skyline_range_delete_filter->applyRangeFilterWithTimetag(-2, 44, 100);

        
    cout << "point search " << endl;
    cout << "1 @10 " << skyline_range_delete_filter->isEntryAliveWithTimetag(1, 10) << endl;
    cout << "21 @10 " << skyline_range_delete_filter->isEntryAliveWithTimetag(21, 10) << endl;
    cout << "11 @10 " << skyline_range_delete_filter->isEntryAliveWithTimetag(11, 10) << endl;
    cout << "14 @100 " << skyline_range_delete_filter->isEntryAliveWithTimetag(14, 100) << endl;
}