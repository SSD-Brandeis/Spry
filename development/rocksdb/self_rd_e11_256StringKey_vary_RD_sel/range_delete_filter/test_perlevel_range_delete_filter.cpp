
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
#include "perlevel_range_delete_filter_by_set.cc"
#include "perlevel_range_delete_filter_by_vector.cc"


using namespace std;
using namespace range_delete_filter;

void test_perlevel_range_delete_filter();
void test_perlevel_range_delete_filter2();
void test_vector_perlevel_range_delete_filter();
void test_remove_range_from_range_delete_filter();
void test_remove_range_from_range_delete_filter2();


int main(int argc, char** argv){
    cout << "test_perlevel_range_delete_filter() :" << endl;
    // test_perlevel_range_delete_filter();
    cout << endl << endl << endl;

    cout << "test_perlevel_range_delete_filter2() :" << endl;
    // test_perlevel_range_delete_filter2();
    cout << endl << endl << endl;

    cout << "test_vector_perlevel_range_delete_filter() :" << endl;
    // test_vector_perlevel_range_delete_filter();
    cout << endl << endl << endl;

    cout << "test_remove_range_from_range_delete_filter() :" << endl;
    // test_remove_range_from_range_delete_filter();
    cout << endl << endl << endl;
    
    cout << "test_remove_range_from_range_delete_filter2() :" << endl;
    test_remove_range_from_range_delete_filter2();
    cout << endl << endl << endl;


    return 0;
}


void test_remove_range_from_range_delete_filter2(){
    
    using PL_RDF = PerlevelRangeDeleteFilter;

    PL_RDF* pl_RDF = new PL_RDF();
    
    pl_RDF->addRangeDelete(1, 10);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(2, 5);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(3, 4);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(4, 5);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(5, 6);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(6, 7);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(31, 41);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(25, 35);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(37, 43);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(32, 38);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(12, 21);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(14, 17);
    pl_RDF->printRangeDeleteList();
    
    pl_RDF->addRangeDelete(24, 27);
    pl_RDF->printRangeDeleteList();
    
    pl_RDF->addRangeDelete(34, 37);
    pl_RDF->printRangeDeleteList();
    
    pl_RDF->addRangeDelete(44, 47);
    pl_RDF->printRangeDeleteList();
    
    pl_RDF->addRangeDelete(54, 57);
    pl_RDF->printRangeDeleteList();
    
    pl_RDF->addRangeDelete(64, 67);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(74, 77);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(84, 87);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(94, 97);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(104, 107);
    pl_RDF->printRangeDeleteList();

    cout << "remove range" << endl;
    vector<RangeEntry> range_delete_discard_list;
    range_delete_discard_list.push_back(RangeEntry({pll(1,10),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(12,15),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(15,17),-1}));

    pl_RDF->removeRangeDelete(range_delete_discard_list);
    pl_RDF->printRangeDeleteList();

    range_delete_discard_list.clear();
    range_delete_discard_list.push_back(RangeEntry({pll(24,27),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(31,37),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(41,46),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(54,56),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(64,67),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(74,79),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(83,88),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(91,99),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(105,106),-1}));

    pl_RDF->removeRangeDelete(range_delete_discard_list);
    pl_RDF->printRangeDeleteList();
}


void test_remove_range_from_range_delete_filter(){
    
    using PL_RDF = PerlevelRangeDeleteFilter;

    PL_RDF* pl_RDF = new PL_RDF();
    
    pl_RDF->addRangeDelete(1, 10);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(2, 5);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(3, 4);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(4, 5);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(5, 6);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(6, 7);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(31, 41);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(25, 35);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(37, 43);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(32, 38);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(12, 21);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(14, 17);
    pl_RDF->printRangeDeleteList();
    
    pl_RDF->addRangeDelete(24, 27);
    pl_RDF->printRangeDeleteList();
    
    pl_RDF->addRangeDelete(34, 37);
    pl_RDF->printRangeDeleteList();
    
    pl_RDF->addRangeDelete(44, 47);
    pl_RDF->printRangeDeleteList();
    
    pl_RDF->addRangeDelete(54, 57);
    pl_RDF->printRangeDeleteList();
    
    pl_RDF->addRangeDelete(64, 67);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(74, 77);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(84, 87);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(94, 97);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(104, 107);
    pl_RDF->printRangeDeleteList();

    cout << "remove range" << endl;
    vector<pll> range_delete_discard_list;
    range_delete_discard_list.push_back(pll(1, 10));
    range_delete_discard_list.push_back(pll(12, 15));
    range_delete_discard_list.push_back(pll(15, 17));

    pl_RDF->removeRangeDelete(range_delete_discard_list);
    pl_RDF->printRangeDeleteList();

    range_delete_discard_list.clear();
    range_delete_discard_list.push_back(pll(24, 27));
    range_delete_discard_list.push_back(pll(31, 37));
    range_delete_discard_list.push_back(pll(41, 46));
    range_delete_discard_list.push_back(pll(54, 56));
    range_delete_discard_list.push_back(pll(64, 67));
    range_delete_discard_list.push_back(pll(74, 79));
    range_delete_discard_list.push_back(pll(83, 88));
    range_delete_discard_list.push_back(pll(91, 99));
    range_delete_discard_list.push_back(pll(105, 106));
    pl_RDF->removeRangeDelete(range_delete_discard_list);
    pl_RDF->printRangeDeleteList();
}

void test_vector_perlevel_range_delete_filter(){
    using PL_RDF = PerlevelRangeDeleteFilter;

    PL_RDF pl_RDF;
    PL_RDF pl_RDF2;

    pl_RDF.addRangeDelete(1, 10);
    pl_RDF.printRangeDeleteList();

    vector<PL_RDF> pl_RDF_vector;
    pl_RDF_vector.push_back(pl_RDF);
    pl_RDF_vector.push_back(pl_RDF2);

    auto &it = pl_RDF_vector.at(0);
    it.addRangeDelete(23, 25);

    PL_RDF &rdf = pl_RDF_vector[0];
    rdf.addRangeDelete(31, 40);

    pl_RDF_vector[0].addRangeDelete(42, 45);

    pl_RDF_vector[1].addRangeDelete(3, 4);

    int l = 0;
    for(auto &x : pl_RDF_vector){
        cout << "pl_RDF_vector[" << l << "] :" << endl;
        x.printRangeDeleteList();
        l += 1;
    }


}

void test_perlevel_range_delete_filter(){
    
    using PL_RDF = PerlevelRangeDeleteFilter;

    PL_RDF* pl_RDF = new PL_RDF();
    
    cout << "add range delete " << endl;
    pl_RDF->addRangeDelete(1, 10);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(2, 5);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(3, 4);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(4, 5);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(5, 6);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(6, 7);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(31, 41);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(25, 35);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(37, 43);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(32, 38);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(12, 21);
    pl_RDF->printRangeDeleteList();

    pl_RDF->addRangeDelete(14, 17);
    pl_RDF->printRangeDeleteList();

    cout << "point search " << endl;
    cout << "1 " << pl_RDF->isEntryAlive(1) << endl;
    cout << "21 " << pl_RDF->isEntryAlive(21) << endl;
    cout << "11 " << pl_RDF->isEntryAlive(22) << endl;
    cout << "31 " << pl_RDF->isEntryAlive(31) << endl;

    cout << "range search " << endl;
    pl_RDF->applyRangeFilter(1,20);
    
    cout << "range search " << endl;
    pl_RDF->applyRangeFilter(1,20);    
    pl_RDF->applyRangeFilter(-1,45);
    pl_RDF->applyRangeFilter(10,25);
    pl_RDF->applyRangeFilter(11,24);

    cout << "add range delete " << endl;
    pl_RDF->addRangeDelete(11, 24);
    pl_RDF->printRangeDeleteList();
    pl_RDF->addRangeDelete(0, 48);
    pl_RDF->printRangeDeleteList();
}


void test_perlevel_range_delete_filter2(){
    
    using PL_RDF = PerlevelRangeDeleteFilter;

    PL_RDF* pl_RDF = new PL_RDF();

    vector<RangeEntry> range_delete_discard_list;

    cout << "add range delete " << endl;
    range_delete_discard_list.push_back(RangeEntry({pll(1, 10),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(2, 5),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(3, 4),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(4, 5),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(5, 6),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(6, 7),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(31, 41),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(25, 35),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(37, 43),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(32, 38),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(12, 21),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(14, 17),-1}));
    pl_RDF->printRangeDeleteList();
    sort(range_delete_discard_list.begin(), range_delete_discard_list.end(), [&](RangeEntry &a, RangeEntry &b){
        return a.getStartKey() < b.getStartKey();
    });
    pl_RDF->addRangeDelete(range_delete_discard_list);
    pl_RDF->printRangeDeleteList();



    cout << "point search " << endl;
    cout << "1 " << pl_RDF->isEntryAlive(1) << endl;
    cout << "21 " << pl_RDF->isEntryAlive(21) << endl;
    cout << "11 " << pl_RDF->isEntryAlive(22) << endl;
    cout << "31 " << pl_RDF->isEntryAlive(31) << endl;

    cout << "range search " << endl;
    pl_RDF->applyRangeFilter(1,20);
    
    cout << "range search " << endl;
    pl_RDF->applyRangeFilter(1,20);    
    pl_RDF->applyRangeFilter(-1,45);
    pl_RDF->applyRangeFilter(10,25);
    pl_RDF->applyRangeFilter(11,24);

    cout << "add range delete " << endl;
    range_delete_discard_list.clear();
    range_delete_discard_list.push_back(RangeEntry({pll(11, 24),-1}));
    range_delete_discard_list.push_back(RangeEntry({pll(0, 48),-1}));
    pl_RDF->printRangeDeleteList();
    sort(range_delete_discard_list.begin(), range_delete_discard_list.end(), [&](RangeEntry &a, RangeEntry &b){
        return a.getStartKey() < b.getStartKey();
    });
    pl_RDF->addRangeDelete(range_delete_discard_list);
    pl_RDF->printRangeDeleteList();
}
