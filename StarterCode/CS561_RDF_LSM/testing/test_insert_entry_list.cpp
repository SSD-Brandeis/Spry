
#include <iostream>
#include <bits/stdc++.h>
#include "../tree_builder/tree_builder.h"

using namespace std;
using namespace tree_builder;


void insert_entry_list_example();
void test_extract_filter_and_merge__point_entry_from_overlapping_file();
void test_extract_merge__range_entry_from_overlapping_file();

int main(){
    // cout << "Insert Entry List Example" << endl;
    // insert_entry_list_example();

    cout << endl << endl << endl;
    cout << "Test extract, filter and merge point entry from overlapping file" << endl;
    test_extract_filter_and_merge__point_entry_from_overlapping_file();

    cout << endl << endl << endl;
    cout << "Test extract and merge range entry from overlapping file" << endl;
    test_extract_merge__range_entry_from_overlapping_file();

    return 0;
}


void test_extract_merge__range_entry_from_overlapping_file(){
    //data to be flushed
    EntryList &entry_list = *(new EntryList());
    entry_list.add_range_entry(RangeEntry(pll(1,2), -1));
    entry_list.add_range_entry(RangeEntry(pll(2,3), -1));
    entry_list.add_range_entry(RangeEntry(pll(3,4), -1));
    entry_list.add_range_entry(RangeEntry(pll(4,5), -1));
    entry_list.add_range_entry(RangeEntry(pll(7,11), -1));
    entry_list.add_range_entry(RangeEntry(pll(14,17), -1));
    entry_list.add_range_entry(RangeEntry(pll(21,27), -1));
    entry_list.add_range_entry(RangeEntry(pll(28,29), -1));

    vector<RangeEntry> range_entry_vector = entry_list.get_range_entry_vector();
    for_each(range_entry_vector.begin(), range_entry_vector.end(), [](RangeEntry &entry){
        cout << entry.getStartKey() << " " << entry.getEndKey() << " " << entry.getTimetag() << endl;
    });

    cout << "-----------------------" << endl;

    //data to be merged
    RangeEntry range_entry = RangeEntry(pll(5,6), -1);
    RangeEntry range_entry2 = RangeEntry(pll(6,7), -1);
    RangeEntry range_entry3 = RangeEntry(pll(7,8), -1);
    RangeEntry range_entry4 = RangeEntry(pll(8,9), -1);

    RangeEntry range_entry5 = RangeEntry(pll(9,10), -1);
    RangeEntry range_entry6 = RangeEntry(pll(10,11), -1);
    RangeEntry range_entry7 = RangeEntry(pll(11,12), -1);
    RangeEntry range_entry8 = RangeEntry(pll(12,13), -1);

    RangeEntry range_entry9 = RangeEntry(pll(14,15), -1);
    RangeEntry range_entry10 = RangeEntry(pll(13,14), -1);
    RangeEntry range_entry11 = RangeEntry(pll(16,17), -1);
    RangeEntry range_entry12 = RangeEntry(pll(15,16), -1);

    RangeEntry range_entry13 = RangeEntry(pll(18,19), -1);
    RangeEntry range_entry14 = RangeEntry(pll(19,20), -1);

    Page &page = *(new Page());
    Page &page2 = *(new Page());
    Page &page3 = *(new Page());
    Page &page4 = *(new Page());
    page.setPageType(Page::RANGE).addRangeEntry(range_entry).addRangeEntry(range_entry2).addRangeEntry(range_entry3).addRangeEntry(range_entry4).addRangeEntry(range_entry5);
    page2.setPageType(Page::RANGE).addRangeEntry(range_entry6).addRangeEntry(range_entry7).addRangeEntry(range_entry8);
    page3.setPageType(Page::RANGE).addRangeEntry(range_entry9).addRangeEntry(range_entry10).addRangeEntry(range_entry11).addRangeEntry(range_entry12);
    page4.setPageType(Page::RANGE).addRangeEntry(range_entry13).addRangeEntry(range_entry14);
    page.sortRangeEntryVectorOnStartKeyInAscendingOrder();
    page2.sortRangeEntryVectorOnStartKeyInAscendingOrder();
    page3.sortRangeEntryVectorOnStartKeyInAscendingOrder();
    page4.sortRangeEntryVectorOnStartKeyInAscendingOrder();


    vector<Page> page_vector, page_vector2, page_vector3;
    page_vector.push_back(page);
    page_vector.push_back(page2);
    page_vector2.push_back(page3);
    page_vector3.push_back(page4);

    DeleteTile &delete_tile = *(new DeleteTile());
    DeleteTile &delete_tile2 = *(new DeleteTile());
    DeleteTile &delete_tile3 = *(new DeleteTile());
    delete_tile.setDeleteTileType(DeleteTile::RANGE).setRangePageVector(page_vector);
    delete_tile2.setDeleteTileType(DeleteTile::RANGE).setRangePageVector(page_vector2);
    delete_tile3.setDeleteTileType(DeleteTile::RANGE).setRangePageVector(page_vector3);
    delete_tile.sortRangePageVectorOnMinRangeKeyInAscendingOrder();
    delete_tile2.sortRangePageVectorOnMinRangeKeyInAscendingOrder();
    delete_tile3.sortRangePageVectorOnMinRangeKeyInAscendingOrder();
    
    vector<DeleteTile> delete_tile_vector, delete_tile_vector2;
    delete_tile_vector.push_back(delete_tile);
    delete_tile_vector.push_back(delete_tile2);
    delete_tile_vector2.push_back(delete_tile3);

    SSTFile &sst_file = *(new SSTFile());
    SSTFile &sst_file2 = *(new SSTFile());
    sst_file.setRangeDeleteTileVector(delete_tile_vector);
    sst_file2.setRangeDeleteTileVector(delete_tile_vector2);
    sst_file.sortRangeDeleteTileVectorOnMinRangeKeyInAscendingOrder();
    sst_file2.sortRangeDeleteTileVectorOnMinRangeKeyInAscendingOrder();

    sst_file.setNextFilePtr(&sst_file2); 

    int f = 0, i = 0, j = 0, k = 0;
    for(SSTFile *file = &sst_file; file != NULL; file = file->getNextFilePtr()){
        cout << "-----------------------" << endl;
        cout << "file " << f << " min_range_key " << file->getMinRangeKey() << " max_range_key " << file->getMaxRangeKey() << endl;
        for(DeleteTile &delete_tile : file->getRangeDeleteTileVector()){
            cout << "\tdelete_tile " << i << " min_range_key " << delete_tile.getMinRangeKey() << " max_range_key " << delete_tile.getMaxRangeKey() << endl;
            for(Page &page : delete_tile.getRangePageVector()){
                cout << "\t\tpage " << j << " min_range_key " << page.getMinRangeKey() << " max_range_key " << page.getMaxRangeKey() << endl;
                for(RangeEntry &entry : page.getRangeEntryVector()){
                    cout << "\t\t\tentry " << k << "   ";
                    cout << entry.getStartKey() << " " << entry.getEndKey() << " " << entry.getTimetag() << endl;
                    k++;
                }
                k = 0;
                j++;
            }
            j = 0;
            i++;
        }
        i = 0;
        f++;
    }
    
    cout << "-----------------------" << endl;

    //extract range entries from overlapping files
    vector<RangeEntry> range_entry_vector2 = entry_list.extract_range_entry_from_overlapping_file(&sst_file);

    cout << "extracted range entries:" << endl;
    for(RangeEntry &entry : range_entry_vector2){
        cout << entry.getStartKey() << " " << entry.getEndKey() << " " << entry.getTimetag() << endl;
    }

    cout << "-----------------------" << endl;

    //merge range entries
    cout << "Before merge:" << endl;
    for(RangeEntry &entry : entry_list.get_range_entry_vector()){
        cout << entry.getStartKey() << " " << entry.getEndKey() << " " << entry.getTimetag() << endl;
    }
    cout << "Do merging..." <<  endl;
    entry_list.merge_with_another_range_entry_vector(range_entry_vector2);
    cout << "After merge:" << endl;
    for(RangeEntry &entry : entry_list.get_range_entry_vector()){
        cout << entry.getStartKey() << " " << entry.getEndKey() << " " << entry.getTimetag() << endl;
    }

}



void test_extract_filter_and_merge__point_entry_from_overlapping_file(){
    //data to be flushed
    EntryList &entry_list = *(new EntryList());
    entry_list.add_point_entry(PointEntry(KV(pll(1,2), "hello"), -1));
    entry_list.add_point_entry(PointEntry(KV(pll(2,3), "world"), -1));
    entry_list.add_point_entry(PointEntry(KV(pll(3,4), "hello"), -1));
    entry_list.add_point_entry(PointEntry(KV(pll(4,5), "world"), -1));
    entry_list.add_point_entry(PointEntry(KV(pll(7,9), "world"), -1));
    entry_list.add_point_entry(PointEntry(KV(pll(8,11), "world"), -1));


    vector<PointEntry> point_entry_vector = entry_list.get_point_entry_vector();
    for_each(point_entry_vector.begin(), point_entry_vector.end(), [](PointEntry &entry){
        cout << entry.getSortKey() << " " << entry.getDeleteKey() << " " << entry.getValue() << endl;
    });



    //SST files on Next Level
    PointEntry point_entry = PointEntry(KV(pll(5,6), "hello"), -1);
    PointEntry point_entry2 = PointEntry(KV(pll(6,7), "world"), -1);
    PointEntry point_entry3 = PointEntry(KV(pll(7,8), "hello"), -1);
    PointEntry point_entry4 = PointEntry(KV(pll(8,9), "world"), -1);

    PointEntry point_entry5 = PointEntry(KV(pll(9,10), "hello"), -1);
    PointEntry point_entry6 = PointEntry(KV(pll(10,11), "world"), -1);
    PointEntry point_entry7 = PointEntry(KV(pll(11,12), "hello"), -1);
    PointEntry point_entry8 = PointEntry(KV(pll(12,13), "world"), -1);

    PointEntry point_entry9 = PointEntry(KV(pll(14,15), "world"), -1);
    PointEntry point_entry10 = PointEntry(KV(pll(13,14), "hello"), -1);
    PointEntry point_entry11 = PointEntry(KV(pll(16,17), "world"), -1);
    PointEntry point_entry12 = PointEntry(KV(pll(15,16), "hello"), -1);

    PointEntry point_entry13 = PointEntry(KV(pll(17,18), "world"), -1);
    PointEntry point_entry14 = PointEntry(KV(pll(18,19), "hello"), -1);
    PointEntry point_entry15 = PointEntry(KV(pll(19,20), "world"), -1);
    PointEntry point_entry16 = PointEntry(KV(pll(20,21), "hello"), -1);


    vector<Page> point_page_vector;
    // point_page_vector.push_back(Page().setPageType(Page::POINT).setPointEntryVector(point_entry_vector));
    Page &page = *(new Page());
    page.setPageType(Page::POINT).addPointEntry(point_entry).addPointEntry(point_entry2).addPointEntry(point_entry3).addPointEntry(point_entry4);
    Page &page2 = *(new Page());
    page2.setPageType(Page::POINT).addPointEntry(point_entry5).addPointEntry(point_entry6).addPointEntry(point_entry7).addPointEntry(point_entry8);
    page.sortPointEntryVectorOnSortKeyInAscendingOrder(); //sort
    page2.sortPointEntryVectorOnSortKeyInAscendingOrder(); //sort
    point_page_vector.push_back( page2 );
    point_page_vector.push_back( page );

    vector<Page> point_page_vector2;

    Page &page3 = *(new Page());
    page3.setPageType(Page::POINT).addPointEntry(point_entry9).addPointEntry(point_entry10).addPointEntry(point_entry11).addPointEntry(point_entry12);
    page3.sortPointEntryVectorOnSortKeyInAscendingOrder(); //sort
    point_page_vector2.push_back( page3 );


    vector<Page> point_page_vector3;

    Page &page4 = *(new Page());
    page4.setPageType(Page::POINT).addPointEntry(point_entry13).addPointEntry(point_entry14).addPointEntry(point_entry15).addPointEntry(point_entry16);
    page4.sortPointEntryVectorOnSortKeyInAscendingOrder(); //sort
    point_page_vector3.push_back( page4 );


    DeleteTile &delete_tile = *(new DeleteTile());
    delete_tile.setDeleteTileType(DeleteTile::POINT).setPointPageVector(point_page_vector);
    delete_tile.sortPointPageVectorOnMinDeleteKeyInAscendingOrder();
    DeleteTile &delete_tile2 = *(new DeleteTile());
    delete_tile2.setDeleteTileType(DeleteTile::POINT).setPointPageVector(point_page_vector2);
    delete_tile2.sortPointPageVectorOnMinDeleteKeyInAscendingOrder();
    vector<DeleteTile> point_delete_tile_vector;
    point_delete_tile_vector.push_back(delete_tile); //1 delete tile 2 page
    point_delete_tile_vector.push_back(delete_tile2); //1 delete tile 1 page

    DeleteTile &delete_tile3 = *(new DeleteTile());
    delete_tile3.setDeleteTileType(DeleteTile::POINT).setPointPageVector(point_page_vector3);
    delete_tile3.sortPointPageVectorOnMinDeleteKeyInAscendingOrder();
    vector<DeleteTile> point_delete_tile_vector2;
    point_delete_tile_vector2.push_back(delete_tile3); //1 delete tile 1 page


    SSTFile &sst_file = *(new SSTFile());
    sst_file.setPointDeleteTileVector(point_delete_tile_vector);
    
    SSTFile &sst_file2 = *(new SSTFile());
    sst_file2.setPointDeleteTileVector(point_delete_tile_vector2);

    sst_file.setNextFilePtr(&sst_file2);

    int f = 0, i = 0, j = 0, k = 0;
    for(SSTFile *sst_file_ptr = &sst_file; sst_file_ptr != NULL; sst_file_ptr = sst_file_ptr->getNextFilePtr()){
        cout << "SST File " << f << " min_sort_key " << sst_file_ptr->getMinSortKey() << " max_sort_key " << sst_file_ptr->getMaxSortKey() << " min_delete_key " << sst_file_ptr->getMinDeleteKey() << " max_delete_key " << sst_file_ptr->getMaxDeleteKey() << endl;
        for(DeleteTile &delete_tile: sst_file_ptr->getPointDeleteTileVector()){
            cout << "\t Delete Tile " << i << " min_sort_key " << delete_tile.getMinSortKey() << " max_sort_key " << delete_tile.getMaxSortKey() << " min_delete_key " << delete_tile.getMinDeleteKey() << " max_delete_key " << delete_tile.getMaxDeleteKey() << endl;
            for(Page &page: delete_tile.getPointPageVector()){
                cout << "\t\t Page " << j << " min_sort_key " << page.getMinSortKey() << " max_sort_key " << page.getMaxSortKey() << " min_delete_key " << page.getMinDeleteKey() << " max_delete_key " << page.getMaxDeleteKey() << endl;
                for(PointEntry &entry: page.getPointEntryVector()){
                    cout << "\t\t\t Entry " << k <<  "  ";
                    cout << entry.getSortKey() << " " << entry.getDeleteKey() << " " << entry.getValue() << endl;
                    k++;
                }
                k = 0;
                j++;
            }
            j = 0;
            i++;
        }
        i = 0;
        f++;
    }

    //extract point entry from overlapping file
    vector<PointEntry> point_entry_vector2 = entry_list.extract_point_entry_from_overlapping_file(&sst_file);
    cout << "Extracted Point Entry" << endl;
    for_each(point_entry_vector2.begin(), point_entry_vector2.end(), [](PointEntry &entry){
        cout << entry.getSortKey() << " " << entry.getDeleteKey() << " " << entry.getValue() << endl;
    });


    entry_list.add_range_entry(RangeEntry(pll(10,12), -1));
    //entry_list.add_range_entry(RangeEntry(pll(1,18), -1));
    //apply range delete on point entry
    vector<PointEntry> point_entry_vector3 = entry_list.apply_range_delete_on_point_entry_vector(point_entry_vector2);
    // vector<PointEntry> point_entry_vector30;
    // vector<PointEntry> point_entry_vector3 = entry_list.apply_range_delete_on_point_entry_vector(point_entry_vector30);
    cout << "After Range Delete" << endl;
    for_each(point_entry_vector3.begin(), point_entry_vector3.end(), [](PointEntry &entry){
        cout << entry.getSortKey() << " " << entry.getDeleteKey() << " " << entry.getValue() << endl;
    });

    //merge point entry vector
    cout << "Before Merge" << endl;
    point_entry_vector = entry_list.get_point_entry_vector();
    for_each(point_entry_vector.begin(), point_entry_vector.end(), [](PointEntry &entry){
        cout << entry.getSortKey() << " " << entry.getDeleteKey() << " " << entry.getValue() << endl;
    });
    entry_list.merge_with_another_point_entry_vector(point_entry_vector3);
    cout << "After Merge" << endl;
    point_entry_vector = entry_list.get_point_entry_vector();
    for_each(point_entry_vector.begin(), point_entry_vector.end(), [](PointEntry &entry){
        cout << entry.getSortKey() << " " << entry.getDeleteKey() << " " << entry.getValue() << endl;
    });


}





void insert_entry_list_example(){
    vector<PointEntry> point_entry_vector;
    point_entry_vector.push_back(PointEntry(KV(pll(1,2), "hello"), -1));
    point_entry_vector.push_back(PointEntry(KV(pll(2,3), "world"), -1));
    point_entry_vector.push_back(PointEntry(KV(pll(3,4), "hello"), -1));
    point_entry_vector.push_back(PointEntry(KV(pll(4,5), "world"), -1));


    for_each(point_entry_vector.begin(), point_entry_vector.end(), [](PointEntry &entry){
        cout << entry.getSortKey() << " " << entry.getDeleteKey() << " " << entry.getValue() << endl;
    });

    PointEntry point_entry = PointEntry(KV(pll(5,6), "hello"), -1);
    PointEntry point_entry2 = PointEntry(KV(pll(6,7), "world"), -1);
    PointEntry point_entry3 = PointEntry(KV(pll(7,8), "hello"), -1);
    PointEntry point_entry4 = PointEntry(KV(pll(8,9), "world"), -1);

    PointEntry point_entry5 = PointEntry(KV(pll(9,10), "hello"), -1);
    PointEntry point_entry6 = PointEntry(KV(pll(10,11), "world"), -1);
    PointEntry point_entry7 = PointEntry(KV(pll(11,12), "hello"), -1);
    PointEntry point_entry8 = PointEntry(KV(pll(12,13), "world"), -1);

    PointEntry point_entry9 = PointEntry(KV(pll(13,14), "hello"), -1);
    PointEntry point_entry10 = PointEntry(KV(pll(14,15), "world"), -1);
    PointEntry point_entry11 = PointEntry(KV(pll(15,16), "hello"), -1);
    PointEntry point_entry12 = PointEntry(KV(pll(16,17), "world"), -1);


    vector<Page> point_page_vector;
    // point_page_vector.push_back(Page().setPageType(Page::POINT).setPointEntryVector(point_entry_vector));
    Page &page = *(new Page());
    page.setPageType(Page::POINT).addPointEntry(point_entry).addPointEntry(point_entry2).addPointEntry(point_entry3).addPointEntry(point_entry4);
    Page &page2 = *(new Page());
    page2.setPageType(Page::POINT).addPointEntry(point_entry5).addPointEntry(point_entry6).addPointEntry(point_entry7).addPointEntry(point_entry8);
    point_page_vector.push_back( page );
    point_page_vector.push_back( page2 );


    vector<Page> point_page_vector2;

    Page &page3 = *(new Page());
    page3.setPageType(Page::POINT).addPointEntry(point_entry9).addPointEntry(point_entry10).addPointEntry(point_entry11).addPointEntry(point_entry12);
    point_page_vector2.push_back( page3 );



    // cout << point_page_vector.size() << endl;
    // for(Page &page: point_page_vector){
    //     for(PointEntry &entry: page.getPointEntryVector()){
    //         cout << entry.getSortKey() << " " << entry.getDeleteKey() << " " << entry.getValue() << endl;
    //     }
    // }



    vector<DeleteTile> point_delete_tile_vector;
    point_delete_tile_vector.push_back(DeleteTile().setDeleteTileType(DeleteTile::POINT).setPointPageVector(point_page_vector)); //1 delete tile 2 page
    point_delete_tile_vector.push_back(DeleteTile().setDeleteTileType(DeleteTile::POINT).setPointPageVector(point_page_vector2)); //1 delete tile 1 page

    SSTFile &sst_file = *(new SSTFile());
    sst_file.setPointDeleteTileVector(point_delete_tile_vector);
    

    int i = 0, j = 0, k = 0;
    for(DeleteTile &delete_tile: sst_file.getPointDeleteTileVector()){
        cout << "Delete Tile " << i << endl;
        for(Page &page: delete_tile.getPointPageVector()){
            cout << "\t Page " << j << endl;
            for(PointEntry &entry: page.getPointEntryVector()){
                cout << "\t\t Entry " << k <<  "  ";
                cout << entry.getSortKey() << " " << entry.getDeleteKey() << " " << entry.getValue() << endl;
                k++;
            }
            k = 0;
            j++;
        }
        j = 0;
        i++;
    }
}