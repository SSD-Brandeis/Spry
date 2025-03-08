/*
 *  Created on: May 13, 2019
 *  Author: Subhadeep, Papon
 */


#include "emu_runner.h"

#include <sstream>
#include <iostream>
#include <cstdio>
#include <sys/time.h>
#include <cmath>
#include <unistd.h>
#include <assert.h>
#include <fstream>
#include <stdlib.h>
#include <cstdlib>
#include <iomanip>

#include "args.hxx"
#include "emu_environment.h"
#include "tree_builder/tree_builder.h"
#include "workload_executor.h"
#include "workload_generator.h"
#include "query_runner.h"

#include "instruction_decoder.h"
#include "checking/checking.h"

using namespace std;
using namespace tree_builder;
using namespace workload_exec;






/*
 * DECLARATIONS
*/
int Query::delete_key;
int Query::range_start_key;
int Query::range_end_key;
int Query::sec_range_start_key;
int Query::sec_range_end_key;
int Query::iterations_point_query;

//long inserts(EmuEnv* _env);
int parse_arguments2(int argc, char *argvx[], EmuEnv* _env);
void printEmulationOutput(EmuEnv* _env);
void calculateDeleteTileSize(EmuEnv* _env);

int runWorkload(EmuEnv* _env);

void showProgress_w(const uint32_t &workload_size, const uint32_t &counter) {
    if (counter / (max(workload_size,(uint32_t)100)/100) >= 1) {
      for (int i = 0; i<104; i++){
        std::cout << "\b";
        fflush(stdout);
      }
    }

    for (int i = 0; i<counter / (max(workload_size,(uint32_t)100)/100); i++){
      std::cout << "=" ;
      fflush(stdout);
    }

    std::cout << std::setfill(' ') << std::setw(101 - counter / (max(workload_size,(uint32_t)100)/100));
    std::cout << counter*100/workload_size << "%";
      fflush(stdout);

  if (counter == workload_size) {
    std::cout << "\n";
    return;
  }
}


int main(int argc, char *argvx[]) {

  // check emu_environment.h for the contents of EmuEnv and also the definitions of the singleton experimental environment 
  EmuEnv* _env = EmuEnv::getInstance();
  //parse the command line arguments
  if (parse_arguments2(argc, argvx, _env)){
    exit(1);
  }
  _env->read_variables();

  fstream fout1, fout2, fout3, fout4;
 
std::cout << "Tracking Ax @ " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;
    
  // Issuing INSERTS
  if (_env->num_inserts > 0) 
  {
std::cout << "Tracking A1 @ " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;

    std::cerr << "Issuing inserts ... " << std::endl << std::flush; 
    
    WorkloadGenerator workload_generator;
    double selectivity = (double) _env->selectivity;
    double insertBeforeRangeDelete = (double) _env->insertBeforeRangeDelete;
cout << "selectivity " << selectivity << endl;
cout << "insertBeforeRangeDelete " << insertBeforeRangeDelete << endl;
    long numberOfPointInTheBeginning = (long) ceil(_env->num_inserts * _env->insertBeforeRangeDelete);
    string workloadFilename = _env->workloadFilename;
    assert(_env->num_inserts*1.0 >= 1.0*_env->rd_count*selectivity);
    workload_generator.generateWorkload((long)_env->num_inserts, (long)_env->entry_size, _env->correlation, 
            (long)_env->rd_count, (double) selectivity, (long) numberOfPointInTheBeginning, (string) workloadFilename);    

    std::cout << "Workload Generated!" << std::endl;

    int only_file_meta_data = 0;

    if(_env->delete_tile_size_in_pages > 0 && _env->lethe_new == 0)
    {

      int s = runWorkload(_env);
      std::cout << "Insert complete ... " << std::endl << std::flush; 
      cout<<"per kv size " <<_env->per_kv_size <<endl;
      cout<<"per range delete tombstone size " <<_env->per_range_delete_size <<endl;
      // DiskMetaFile::printAllEntries(only_file_meta_data);
      // MemoryBuffer::getCurrentBufferStatistics();
      // DiskMetaFile::getMetaStatistics();

      if (MemoryBuffer::verbosity == 1 || MemoryBuffer::verbosity == 2 || MemoryBuffer::verbosity == 3)
      {

        DiskMetaFile::printAllEntries(only_file_meta_data);
        MemoryBuffer::getCurrentBufferStatistics();
        DiskMetaFile::getMetaStatistics();
      }

      // DiskMetaFile::printAllEntriesStyle2(only_file_meta_data);
      DiskMetaFile::printAllLevelRangeDeleteFilter();
      DiskMetaFile::printAllLevelSplitRangeDeleteFilter();
      DiskMetaFile::printSkylineRangeDeleteFilter();
      // Query::interactive_point_query(); // interactive point query ADDED BY YC_HUANG
      MemoryBuffer::getCurrentBufferStatistics();
      DiskMetaFile::getMetaStatistics();
      WorkloadExecutor::getExecutionStatistics();
      // WorkloadRecorder::printGroundTruth();

      vector<int> disk_access_count_list_on_existing_keys;
      vector<int> disk_access_count_list_on_all_inserted_keys;
      vector<int> disk_access_count_list_on_currently_deleted_keys;
      vector<int> disk_access_count_list_on_random_keys;
      RandomKeysTestingResult randomKeysTestingResult;
      disk_access_count_list_on_existing_keys = SystemVerifier::checkOnExistingKeys();
      disk_access_count_list_on_all_inserted_keys = SystemVerifier::checkOnAllInsertedKeys();
      disk_access_count_list_on_currently_deleted_keys = SystemVerifier::checkOnAllCurrentlyDeletedKeys();
      randomKeysTestingResult = SystemVerifier::checkOnRandomKeys(10000);
      disk_access_count_list_on_random_keys = randomKeysTestingResult.disk_access_count_list;
      SystemVerifier::checkEquation();

      

      
      ofstream csv_result_file;
      csv_result_file.open("result.csv", ios::app);
      
      int len_implement_types =  disk_access_count_list_on_existing_keys.size();
      //Perlevel RDF, Skyline RDF, Split Perlevel RDF
      assert(len_implement_types == 3);
      string workloadFilename = _env->workloadFilename;
      long entry_size = (long)_env->entry_size;
      long per_kv_size = (long)_env->per_kv_size;
      long per_range_delete_size = (long)_env->per_range_delete_size;
      
      long insert_count = (long)_env->num_inserts;
      long range_delete_count = (long)_env->rd_count;
      double selectivity = (double) _env->selectivity;
      double insertBeforeRangeDelete = (double) _env->insertBeforeRangeDelete;
      long numberOfPointInTheBeginning = (long) ceil(_env->num_inserts * _env->insertBeforeRangeDelete);

      csv_result_file << "workloadFilename,," << "entry_size,per_kv_size,per_range_delete_size,," ;
      csv_result_file << "insert_count,range_delete_count,selectivity,insertBeforeRangeDelete,numberOfPointInTheBeginning,,,";
      csv_result_file << "#PQ,," ;
      for(int i = 0; i < len_implement_types; i++){
        csv_result_file << "#Disk Access_" << i << ",";
      }
      csv_result_file << ",,,,," ;
      csv_result_file << "#PQ,," ;
      for(int i = 0; i < len_implement_types; i++){
        csv_result_file << "#Disk Access_" << i << ",";
      }
      csv_result_file << ",,,,," ;
      csv_result_file << "#PQ,," ;
      for(int i = 0; i < len_implement_types; i++){
        csv_result_file << "#Disk Access_" << i << ",";
      }
      csv_result_file << ",,,,," ;
      csv_result_file << "#exist,#non-exist,," ;
      for(int i = 0; i < len_implement_types; i++){
        csv_result_file << "#Disk Access_" << i << ",";
      }
      csv_result_file << ",,,,," ;
      csv_result_file << "perlevel_rdf_disk_access_count,perlevel_rdf_disk_access_without_last_level_count,,";
      csv_result_file << "skyline_rdf_disk_access_count,,";
      csv_result_file << "split_perlevel_rdf_disk_access_count,split_perlevel_rdf_disk_access_without_last_level_count,";
      csv_result_file << endl;

      csv_result_file << workloadFilename << ",,";
      csv_result_file << entry_size << "," << per_kv_size << "," << per_range_delete_size << ",," ;
      csv_result_file << insert_count << "," << range_delete_count << "," << selectivity << "," 
                      << insertBeforeRangeDelete << "," << numberOfPointInTheBeginning << ",,,";
          



      map<long, string> groundtruth = WorkloadRecorder::getGroundTruth();
      set<long> history_key_set = WorkloadRecorder::getHistoryKeySet();
      int count = groundtruth.size();
      csv_result_file << count << ",,";
      for(int i = 0; i < len_implement_types; i++){
        csv_result_file << disk_access_count_list_on_existing_keys[i] << ",";
      }
      csv_result_file << ",,,,,";
      count = history_key_set.size() ;
      csv_result_file << count << ",,";
      for(int i = 0; i < len_implement_types; i++){
        csv_result_file << disk_access_count_list_on_all_inserted_keys[i] << ",";
      }
      csv_result_file << ",,,,,";
      count = (history_key_set.size() - groundtruth.size());
      csv_result_file << count << ",,";
      for(int i = 0; i < len_implement_types; i++){
        csv_result_file << disk_access_count_list_on_currently_deleted_keys[i] << ",";
      }
      csv_result_file << ",,,,,";
      int count_exist_key = randomKeysTestingResult.count_exist_key;
      int count_non_exist_key = randomKeysTestingResult.count_non_exist_key;
      csv_result_file << count_exist_key << ",";
      csv_result_file << count_non_exist_key << ",,";
      for(int i = 0; i < len_implement_types; i++){
        csv_result_file << disk_access_count_list_on_random_keys[i] << ",";
      }
      csv_result_file << ",,,,,";
      int perlevel_rdf_disk_access_count = DiskMetaFile::getTotalRangeDeleteCountOfPerLevelRangeDeleteFilter();
      int perlevel_rdf_disk_access_without_last_level_count = DiskMetaFile::getTotalRangeDeleteWithoutLastLevelCountOfPerLevelRangeDeleteFilter();
      int skyline_rdf_disk_access_count = DiskMetaFile::getTotalRangeDeleteCountOfSkylineRangeDeleteFilter();
      int split_perlevel_rdf_disk_access_count = DiskMetaFile::getTotalRangeDeleteCountOfSplitPerLevelRangeDeleteFilter();
      int split_perlevel_rdf_disk_access_without_last_level_count = DiskMetaFile::getTotalRangeDeleteWithoutLastLevelCountOfSplitPerLevelRangeDeleteFilter();
      csv_result_file << perlevel_rdf_disk_access_count << ",";
      csv_result_file << perlevel_rdf_disk_access_without_last_level_count << ",,";
      csv_result_file << skyline_rdf_disk_access_count << ",,";
      csv_result_file << split_perlevel_rdf_disk_access_count << ",";
      csv_result_file << split_perlevel_rdf_disk_access_without_last_level_count << ",";
      csv_result_file << endl;



      if (MemoryBuffer::verbosity == 1 || MemoryBuffer::verbosity == 2 || MemoryBuffer::verbosity == 3)
        printEmulationOutput(_env);

    }
    else if (_env->delete_tile_size_in_pages == -1 && _env->lethe_new == 0)
    {

      for (int i = 1; i <= _env->buffer_size_in_pages; i = i*2)
      {

        cout << "Running for delete tile size: " << i << " ..." << endl;
        _env->delete_tile_size_in_pages = i;
        int s = runWorkload(_env);
        if (MemoryBuffer::verbosity == 1 || MemoryBuffer::verbosity == 2 || MemoryBuffer::verbosity == 3)
        {

          DiskMetaFile::printAllEntries(only_file_meta_data);
          MemoryBuffer::getCurrentBufferStatistics();
          DiskMetaFile::getMetaStatistics();
        }

        cout << "Running Delete Query..." << endl;
        Query::delete_query_experiment();
        cout << "Running Range Query..." << endl;
        Query::range_query_experiment();
        cout << "Running Secondary Range Query..." << endl;
        Query::sec_range_query_experiment();
        cout << "Running Point Query..." << endl;
        // Query::point_query_experiment();
        
        DiskMetaFile::clearAllEntries();
        WorkloadExecutor::counter = 0;
        
        if (MemoryBuffer::verbosity == 1 || MemoryBuffer::verbosity == 2 || MemoryBuffer::verbosity == 3)
          printEmulationOutput(_env);
      }
    }
    else if (_env->lethe_new == 1 || _env->lethe_new == 2)
    {

      int s = runWorkload(_env);
      // DiskMetaFile::printAllEntries(only_file_meta_data);
      MemoryBuffer::getCurrentBufferStatistics();
      DiskMetaFile::getMetaStatistics();
      if (MemoryBuffer::verbosity == 1 || MemoryBuffer::verbosity == 2 || MemoryBuffer::verbosity == 3)
      {

        DiskMetaFile::printAllEntries(only_file_meta_data);
        MemoryBuffer::getCurrentBufferStatistics();
        DiskMetaFile::getMetaStatistics();
      }

      cout << "Running Delete Query..." << endl;
      Query::delete_query_experiment();
      cout << "Running Range Query..." << endl;
      Query::range_query_experiment();
      cout << "Running Secondary Range Query..." << endl;
      Query::sec_range_query_experiment();
      cout << "Running Point Query..." << endl;
      Query::new_point_query_experiment();


      DiskMetaFile::clearAllEntries();
      WorkloadExecutor::counter = 0;
      printEmulationOutput(_env);

      if (MemoryBuffer::verbosity == 1 || MemoryBuffer::verbosity == 2 || MemoryBuffer::verbosity == 3)
        printEmulationOutput(_env);
    }

  }
std::cout << "Tracking Ax leave @ " << __FILE__ << " " << __LINE__ << " " << __func__ << std::endl << std::flush;



  return 0;
}

int runWorkload(EmuEnv* _env) {

  MemoryBuffer* buffer_instance = MemoryBuffer::getBufferInstance(_env);
  WorkloadExecutor workload_executer;
  InstructionDecoder instruction_decoder;
  
  fstream workload_file;
  // workload_file.open("workload.txt");
  workload_file.open(_env->workloadFilename);
  assert(workload_file);
  int counter = 0;
 
  string instruction;
  while(std::getline(workload_file, instruction)) {
     instruction_decoder.decode(instruction, workload_executer);

     counter++;

    if(!(counter % (max(_env->num_inserts,100LL)/100))){
      uint32_t num_inserts = _env->num_inserts;
      uint32_t counter_cp = counter;
      showProgress_w(num_inserts, counter_cp);
    }
  }

  return 1;
}


int parse_arguments2(int argc,char *argvx[], EmuEnv* _env) {
  args::ArgumentParser parser("KIWI Emulator", "");

  args::Group group1(parser, "This group is all exclusive:", args::Group::Validators::DontCare);

  args::ValueFlag<int> size_ratio_cmd(group1, "T", "The size ratio of the tree [def: 2]", {'T', "size_ratio"});
  args::ValueFlag<int> buffer_size_in_pages_cmd(group1, "P", "Size of the memory buffer in terms of pages [def: 128]", {'P', "buffer_size_in_pages"});
  args::ValueFlag<int> entries_per_page_cmd(group1, "B", "No of entries in one page [def: 128]", {'B', "entries_per_page"});
  args::ValueFlag<int> entry_size_cmd(group1, "E", "Entry size in bytes [def: 128 B]", {'E', "entry_size"});
  args::ValueFlag<long> buffer_size_cmd(group1, "M", "Memory size (PBE) [def: 2 MB]", {'M', "memory_size"});
  args::ValueFlag<int> delete_tile_size_in_pages_cmd(group1, "delete_tile_size_in_pages", "Size of a delete tile in terms of pages [def: -1]", {'h', "delete_tile_size_in_pages"});
  args::ValueFlag<long> file_size_cmd(group1, "file_size", "file size [def: 256 KB]", {"file_size"});
  args::ValueFlag<long long> num_inserts_cmd(group1, "#inserts", "The number of unique inserts to issue in the experiment [def: 0]", {'i', "num_inserts"});
  args::ValueFlag<int> cor_cmd(group1, "#correlation", "Correlation between sort key and delete key [def: 0]", {'c', "correlation"});
  args::ValueFlag<int> verbosity_cmd(group1, "verbosity", "The verbosity level of execution [0,1,2; def:0]", {'V', "verbosity"});
  args::ValueFlag<int> lethe_new_cmd(group1, "lethe_new", "Specific h across tree(0), Optimal h across tree(1) or different optimal h in each levels(2) [0, 1, 2; def:0]", {'X', "lethe_new"});
  args::ValueFlag<int> SRD_cmd(group1, "SRD", "Count of secondary range delete [def:1]", {'I', "SRD"});
  args::ValueFlag<int> EPQ_cmd(group1, "EPQ", "Count of empty point queries [def:1000000]", {'J', "EPQ"});
  args::ValueFlag<int> PQ_cmd(group1, "PQ", "Count of non-empty point queries [def:1000000]", {'K', "PQ"});
  args::ValueFlag<int> SRQ_cmd(group1, "SRQ", "Count of short range queries [def:1]", {'L', "SRQ"});
  
  args::ValueFlag<int> delete_key_cmd(group1, "delete_key", "Delete all keys less than DK [def:700]", {'D', "delete_key"});
  args::ValueFlag<int> range_start_key_cmd(group1, "range_start_key", "Starting key of the range query [def:2000]", {'S', "range_start_key"});
  args::ValueFlag<int> range_end_key_cmd(group1, "range_end_key", "Ending key of the range query [def:5000]", {'F', "range_end_key"});
  args::ValueFlag<int> sec_range_start_key_cmd(group1, "sec_range_start_key", "Starting key of the secondary range query [def:200]", {'s', "sec_range_start_key"});
  args::ValueFlag<int> sec_range_end_key_cmd(group1, "sec_range_end_key", "Ending key of the secondary range query [def:500]", {'f', "sec_range_end_key"});
  args::ValueFlag<int> iterations_point_query_cmd(group1, "iterations_point_query", "Number of point queries to be performed [def:100000]", {'N', "iterations_point_query"});

  args::ValueFlag<int> RD_cmd(group1, "range_delete", "Count of range delete [def:1]", {'R', "RD"});
  args::ValueFlag<double> selectivity_cmd(group1, "selectivity_of_range_delete", "Selectivity of range delete [def:0.001]", {"z", "selectivity"});
  args::ValueFlag<string> workload_filename_cmd(group1, "workload_filename", "workload filename [def:0.001]", {"f", "workload_filename"});
  args::ValueFlag<double> insert_before_range_delete_cmd(group1, "insert_before_range_delete", "percent of inserts in insert themself that precede any of the range delete [def:0.5]", {"b", "insert_before_range_delete"});


  try
  {
      parser.ParseCLI(argc, argvx);
  }
  catch (args::Help&)
  {
      std::cout << parser;
      exit(0);
      // return 0;
  }
  catch (args::ParseError& e)
  {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
  }
  catch (args::ValidationError& e)
  {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
  }

  _env->size_ratio = size_ratio_cmd ? args::get(size_ratio_cmd) : 2;
  _env->buffer_size_in_pages = buffer_size_in_pages_cmd ? args::get(buffer_size_in_pages_cmd) : 128;
  _env->entries_per_page = entries_per_page_cmd ? args::get(entries_per_page_cmd) : 128;
  _env->entry_size = entry_size_cmd ? args::get(entry_size_cmd) : 128;
  _env->buffer_size = buffer_size_cmd ? args::get(buffer_size_cmd) : _env->buffer_size_in_pages * _env->entries_per_page * _env->entry_size;
  _env->delete_tile_size_in_pages = delete_tile_size_in_pages_cmd ? args::get(delete_tile_size_in_pages_cmd) : -1;
  _env->file_size = file_size_cmd ? args::get(file_size_cmd) : _env->buffer_size;
  _env->num_inserts = num_inserts_cmd ? args::get(num_inserts_cmd) : 0;
  _env->verbosity = verbosity_cmd ? args::get(verbosity_cmd) : 0;
  _env->correlation = cor_cmd ? args::get(cor_cmd) : 0;
  _env->lethe_new = lethe_new_cmd ? args::get(lethe_new_cmd) : 0;

  _env->srd_count = SRD_cmd ? args::get(SRD_cmd) : 1;
  _env->epq_count = EPQ_cmd ? args::get(EPQ_cmd) : 1000000;
  _env->pq_count = PQ_cmd ? args::get(PQ_cmd) : 1000000;
  _env->srq_count = SRQ_cmd ? args::get(SRQ_cmd) : 10000;
  calculateDeleteTileSize(_env);

  Query::delete_key = delete_key_cmd ? args::get(delete_key_cmd) : 700;
  Query::range_start_key = range_start_key_cmd ? args::get(range_start_key_cmd) : 2000;
  Query::range_end_key = range_end_key_cmd ? args::get(range_end_key_cmd) : 5000;
  Query::sec_range_start_key = sec_range_start_key_cmd ? args::get(sec_range_start_key_cmd) : 200;
  Query::sec_range_end_key = sec_range_end_key_cmd ? args::get(sec_range_end_key_cmd) : 500;
  Query::iterations_point_query = iterations_point_query_cmd ? args::get(iterations_point_query_cmd) : 100000;


  _env->rd_count = RD_cmd ? args::get(RD_cmd) : 1;
  _env->selectivity = selectivity_cmd ? args::get(selectivity_cmd) : 0.001;
  _env->workloadFilename = workload_filename_cmd ? args::get(workload_filename_cmd) : "workload.txt";
  _env->insertBeforeRangeDelete = insert_before_range_delete_cmd ? args::get(insert_before_range_delete_cmd) : 0.5;


  return 0;
}

void calculateDeleteTileSize(EmuEnv* _env)
{
  // calculation of Kiwi++ (following desmos)
  float num = (_env->num_inserts * (_env->size_ratio - 1) * 1.0);
  float denum = (_env->buffer_size_in_pages * _env->entries_per_page * _env->size_ratio * 1.0);
  _env->level_count = ceil(log(num/denum)/log(_env->size_ratio));
  float E[20] = {-1};
  float F[20] = {-1};
  float G[20] = {-1};
  float R[20] = {-1};
  float temp_sum = 0;

  for (int j = 1; j <= _env->level_count; j++)
  {
    temp_sum += pow(_env->size_ratio, j);
  }
  for (int i = 1; i <= _env->level_count; i++)
  {
    E[i] = pow(_env->size_ratio, i)/(temp_sum * 1.0);
  }
  for (int i = 1; i <= _env->level_count; i++)
  {
    float tempsum2 = 0;
    for (int j = 1; j <= i - 1; j++)
    {
      tempsum2 += pow(_env->size_ratio, j);
    }
    F[i] = (tempsum2 * 1.0)/temp_sum;
  }
  for (int i = 1; i <= _env->level_count; i++)
  {
    G[i] = 1 - E[i] - F[i];
  }
  float phi = 0.0081925;
  for (int i = 1; i <= _env->level_count; i++)
  {
    R[i] = (pow((_env->size_ratio * 1.0),((_env->size_ratio * 1.0)/(_env->size_ratio - 1))) * phi)/(pow(_env->size_ratio,_env->level_count + 1 - i));
  } 

  for (int i = 1; i <= _env->level_count; i++)
  {
    float num2 = _env->buffer_size_in_pages * pow(_env->size_ratio, i) * _env->srd_count;
    float denum2 = ((_env->epq_count + (_env->pq_count * G[i])) * R[i]) + ((R[i] * _env->pq_count * E[i]) / 2) + _env->srq_count;
    _env->variable_delete_tile_size_in_pages[i] = round (pow(num2/denum2, 0.5));
    if (_env->variable_delete_tile_size_in_pages[i] == 0)
      _env->variable_delete_tile_size_in_pages[i] = 1;
    if (_env->variable_delete_tile_size_in_pages[i] > _env->buffer_size_in_pages)
      _env->variable_delete_tile_size_in_pages[i] = _env->buffer_size_in_pages;
    // cout << _env->variable_delete_tile_size_in_pages[i] << " " << endl;
  }
  if (_env->lethe_new == 1)
  {
    // Optimal h for Kiwi
    float phi_opt = ((pow(_env->size_ratio,(_env->size_ratio*1.0/(_env->size_ratio-1)*1.0)))/_env->size_ratio)*phi;
    float num_opt = (_env->srd_count * _env->num_inserts * 1.0)/_env->entries_per_page;
    float denum_opt = ((_env->epq_count + _env->pq_count) * phi_opt * _env->level_count) + (_env->level_count * _env->srq_count);
    _env->delete_tile_size_in_pages = round (pow(num_opt/denum_opt, 0.5));
    if (_env->delete_tile_size_in_pages == 0)
      _env->delete_tile_size_in_pages = 1;
    if (_env->delete_tile_size_in_pages > _env->buffer_size_in_pages)
      _env->delete_tile_size_in_pages = _env->buffer_size_in_pages;
    std::cout << "Optimal Delete Tile Size: " << _env->delete_tile_size_in_pages << std::endl;
  }
}

//TODO: + RD, range delete part
void printEmulationOutput(EmuEnv* _env) {
  //double percentage_non_zero_result_queries = ceil((100 * _env->nonzero_to_zero_ratio) / (_env->nonzero_to_zero_ratio + 1.0));
  //if (_env->verbosity >= 1) 
  std::cout << "T, P, B, E, M, h, file_size, #I \n";
  std::cout << _env->size_ratio << ", ";
  std::cout << _env->buffer_size_in_pages << ", ";  
  std::cout << _env->entries_per_page << ", ";
  std::cout << _env->entry_size << ", ";
  std::cout << _env->buffer_size << ", ";
  std::cout << _env->delete_tile_size_in_pages << ", ";
  std::cout << _env->file_size << ", ";
  std::cout << _env->num_inserts ;

  std::cout << std::endl;

  std::cout << "Kiwi++,\tSRD,\tEPQ,\tPQ,\tSRQ\n";
  std::cout << _env->lethe_new << "\t";
  std::cout << _env->srd_count << "\t";  
  std::cout << _env->epq_count << "\t";
  std::cout << _env->pq_count << "\t";
  std::cout << _env->srq_count << "\t";

  std::cout << std::endl;

  if (_env->lethe_new == 2)
  {
    std::cout << "Delete tiles size : " << std::endl;
    for (int i = 1; i <= _env->level_count; i++)
    {
      std::cout << _env->variable_delete_tile_size_in_pages[i] << "\t";
    }
    std::cout << std::endl;
  }
}

