#ifndef UTILS_LOGGER_DURING_INSERTION_H
#define UTILS_LOGGER_DURING_INSERTION_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "../workload/args.hxx"
#include "../workload/workload_generator.h"
#include "../env_settings/emu_environment.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/advanced_options.h"
#include "rocksdb/system_verifier.h"
#include "utils_rdf.h"
#include "utils_db.h"
#include "utils_log.h"
#include "rocksdb/SuRF/include/surf.hpp"


class LoggerDuringInsertion{
private:
    // static std::ofstream testing_result_file, testing_result_file_during_insertion;
    std::ofstream testing_result_file_during_insertion, running_log_during_insertion;

    static LoggerDuringInsertion *logger_during_insertion;

    bool flag_reopen_db_for_each_RDF_testing = false;

    // PLRDF plrdf_prime, split_plrdf_prime;
    // PLRDF top_level_rdf_prime;
    // SkyLineRDF skyline_rdf_prime;
    // static std::vector<t3ll> skyline_rdf_prime;
    // static std::vector<int> skyline__numbers_of_ranges_in_rdf_log;

    vector<int> ranges_log_PLRDF;
    vector<int> ranges_log_SplitPLRDF;
    vector<int> ranges_log_TopLevelRDF;
    vector<int> ranges_log_SkylineRDF;
    vector<int> ranges_log_SuRFLevelFileRDF;
    vector<int> ranges_log_SuRFLevelFileSplitRDF;
    
    vector<int> memory_usage_log_PLRDF;
    vector<int> memory_usage_log_SplitPLRDF;
    vector<int> memory_usage_log_TopLevelRDF;
    vector<int> memory_usage_log_SkylineRDF;
    vector<int> memory_usage_log_SuRFLevelFileRDF;
    vector<int> memory_usage_log_SuRFLevelFileSplitRDF;
    TestingLogger testing_logger;

public:
    static LoggerDuringInsertion* getInstance(EmuEnv* _env);
    // static LoggerDuringInsertion* init(DB** db_ptr2, ReadOptions& read_op, EmuEnv* _env);
    void start(EmuEnv* _env);
    void recordCurrentMemoryFootprint(DB** db_ptr2);
    void writeRecord(DB** db_ptr2);
    void runPQVerification(DB** db_ptr2, Options& op, WriteOptions& write_op, 
                            ReadOptions& read_op, EmuEnv* _env, int number_of_PQs = -1,
                            string kDBPath = "/tmp/cs561_project1");

    void runPQonCurrentlyDeletedKeys(
      uint i_insertion, DB** db_ptr2, Options& op, WriteOptions& write_op, 
      ReadOptions& read_op, EmuEnv* _env, int number_of_PQs, string kDBPath);
      
    void end();
};
LoggerDuringInsertion* LoggerDuringInsertion::logger_during_insertion = NULL;
// std::ofstream logger_during_insertion::testing_result_file_during_insertion;
// // std::ofstream logger_during_insertion::testing_result_file_during_insertion;

// TestingLogger logger_during_insertion::testing_logger;

// // bool verification_runner::flag_reopen_db_for_each_RDF_testing = true;
// bool logger_during_insertion::flag_reopen_db_for_each_RDF_testing = false;

// PLRDF logger_during_insertion::plrdf_prime, logger_during_insertion::split_plrdf_prime;
// PLRDF logger_during_insertion::top_level_rdf_prime;
// SkyLineRDF logger_during_insertion::skyline_rdf_prime;
// // std::vector<t3ll> verification_runner::skyline_rdf_prime;
// // std::vector<int> verification_runner::skyline__numbers_of_ranges_in_rdf_log;


LoggerDuringInsertion* LoggerDuringInsertion::getInstance(EmuEnv* _env){
    if(logger_during_insertion == NULL){
        logger_during_insertion = new LoggerDuringInsertion();
        logger_during_insertion->start(_env);
    }
    return logger_during_insertion;
}

// LoggerDuringInsertion* LoggerDuringInsertion::init(DB** db_ptr2, ReadOptions& read_op, EmuEnv* _env){
//     if(logger_during_insertion == NULL){
//         logger_during_insertion = new LoggerDuringInsertion();
//     }
//     logger_during_insertion->start(db_ptr2, read_op, _env);
//     return logger_during_insertion;
// }

void LoggerDuringInsertion::start(EmuEnv* _env){
  
    std::string running_log_name_during_insertion = "output_statistics/" + _env->workload_file_name + "_during_insertion.log";
    running_log_during_insertion.open(running_log_name_during_insertion);

    std::string testing_result_file_name_during_insertion = "output_statistics/" + _env->workload_file_name + "_during_insertion.json";
    testing_result_file_during_insertion.open(testing_result_file_name_during_insertion);
    testing_result_file_during_insertion << "{"<< std::endl;
    testing_result_file_during_insertion << "\"start\" : \"start\""<< std::endl;
    write_log2(testing_result_file_during_insertion, _env);
}


// void logger_during_insertion::init(DB** db_ptr2, ReadOptions& read_op, EmuEnv* _env){
//   DB* db = *db_ptr2;
//   Status s;

//   checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();
//   int KEY_SIZE = checking::SystemVerifier::getSystemVerifier()->getKeySize();

//   // system_verifier->setRunningPQ();

//   // system_verifier->enable_log__deleted_keys__max_sequnce_number();
//   // system_verifier->setRDFTypeChosed(0); // 0: NONE
  
//   // for(auto x: system_verifier->getCurrentlyDeletedKeys()){
//   //   std::string value;
//   //   std::stringstream searching_key;
//   //   searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;

//   //   s = db->Get(read_op, searching_key.str(), &value);
//   // }
//   // system_verifier->disable_log__deleted_keys__max_sequnce_number();





    
//   ranges_log_PLRDF = db->getLogOfNumbersOfRangesInPLRDF();
//   ranges_log_SplitPLRDF = db->getLogOfNumbersOfRangesInSplitPLRDF();
//   ranges_log_TopLevelRDF = db->getLogOfNumbersOfRangesInTopLevelRDF();
//   ranges_log_SkylineRDF = db->getLogOfNumbersOfRangesInSkylineRDF();
//   ranges_log_SuRFLevelFileRDF = db->getLogOfNumbersOfRangesInSuRFLevelFileRDF();
//   ranges_log_SuRFLevelFileSplitRDF = db->getLogOfNumbersOfRangesInSuRFLevelFileSplitRDF();
  
//   memory_usage_log_PLRDF = db->getLogOfMemoryUsageInPLRDF();
//   memory_usage_log_SplitPLRDF = db->getLogOfMemoryUsageInSplitRDF();
//   memory_usage_log_TopLevelRDF = db->getLogOfMemoryUsageInTopLevelRDF();
//   memory_usage_log_SkylineRDF = db->getLogOfMemoryUsageInSkylineRDF();
//   memory_usage_log_SuRFLevelFileRDF = db->getLogOfMemoryUsageInSuRFLevelFileRDF();
//   memory_usage_log_SuRFLevelFileSplitRDF = db->getLogOfMemoryUsageInSuRFLevelFileSplitRDF();



//   // const long long N_repetitions = checking::SystemVerifier::EXPERIMENT_REPETITION_TIMES;
//   // // testing_result_file << "N_repetitions = " << N_repetitions << std::endl << std::endl;
//   // testing_result_file_during_insertion << ",\"N_repetitions\" : " << N_repetitions << std::endl;
//   // testing_result_file_during_insertion << ",\"Total number of SST Files\" : " << db->getTotalNumberOfSSTFiles() << std::endl << std::endl;


//   // testing_logger.reset();

// }


void LoggerDuringInsertion::recordCurrentMemoryFootprint(DB** db_ptr2){
  DB* db = *db_ptr2;
  // Status s;
  running_log_during_insertion << "recordCurrentMemoryFootprint Start " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;


  // checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();
  // int KEY_SIZE = checking::SystemVerifier::getSystemVerifier()->getKeySize();

  vector<int> tmp;
  tmp = db->getLogOfNumbersOfRangesInPLRDF();
  if(tmp.size() > 0){
    ranges_log_PLRDF.push_back(tmp.back());
  }else{
    ranges_log_PLRDF.push_back(0);
  }
  tmp = db->getLogOfNumbersOfRangesInSplitPLRDF();
  if(tmp.size() > 0){
    ranges_log_SplitPLRDF.push_back(tmp.back());
  }else{
    ranges_log_SplitPLRDF.push_back(0);
  }
  tmp = db->getLogOfNumbersOfRangesInTopLevelRDF();
  if(tmp.size() > 0){
    ranges_log_TopLevelRDF.push_back(tmp.back());
  }else{
    ranges_log_TopLevelRDF.push_back(0);
  }
  tmp = db->getLogOfNumbersOfRangesInSkylineRDF();
  if(tmp.size() > 0){
    ranges_log_SkylineRDF.push_back(tmp.back());
  }else{
    ranges_log_SkylineRDF.push_back(0);
  }
  tmp = db->getLogOfNumbersOfRangesInSuRFLevelFileRDF();
  if(tmp.size() > 0){
    ranges_log_SuRFLevelFileRDF.push_back(tmp.back());
  }else{
    ranges_log_SuRFLevelFileRDF.push_back(0);
  }
  tmp = db->getLogOfNumbersOfRangesInSuRFLevelFileSplitRDF();
  if(tmp.size() > 0){
    ranges_log_SuRFLevelFileSplitRDF.push_back(tmp.back());
  }else{
    ranges_log_SuRFLevelFileSplitRDF.push_back(0);
  }

  tmp = db->getLogOfMemoryUsageInPLRDF();
  if(tmp.size() > 0){
    memory_usage_log_PLRDF.push_back(tmp.back());
  }else{
    memory_usage_log_PLRDF.push_back(0);
  }
  tmp = db->getLogOfMemoryUsageInSplitRDF();
  if(tmp.size() > 0){
    memory_usage_log_SplitPLRDF.push_back(tmp.back());
  }else{
    memory_usage_log_SplitPLRDF.push_back(0);
  }
  tmp = db->getLogOfMemoryUsageInTopLevelRDF();
  if(tmp.size() > 0){
    memory_usage_log_TopLevelRDF.push_back(tmp.back());
  }else{
    memory_usage_log_TopLevelRDF.push_back(0);
  }
  tmp = db->getLogOfMemoryUsageInSkylineRDF();
  if(tmp.size() > 0){
    memory_usage_log_SkylineRDF.push_back(tmp.back());
  }else{
    memory_usage_log_SkylineRDF.push_back(0);
  }
  tmp = db->getLogOfMemoryUsageInSuRFLevelFileRDF();
  if(tmp.size() > 0){
    memory_usage_log_SuRFLevelFileRDF.push_back(tmp.back());
  }else{
    memory_usage_log_SuRFLevelFileRDF.push_back(0);
  }
  tmp = db->getLogOfMemoryUsageInSuRFLevelFileSplitRDF();
  if(tmp.size() > 0){
    memory_usage_log_SuRFLevelFileSplitRDF.push_back(tmp.back());
  }else{
    memory_usage_log_SuRFLevelFileSplitRDF.push_back(0);
  }

  running_log_during_insertion << "recordCurrentMemoryFootprint End " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
}

void LoggerDuringInsertion::writeRecord(DB** db_ptr2){
  DB* db = *db_ptr2;
  Status s;

  running_log_during_insertion << "writeRecord Start " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  // checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();
  // int KEY_SIZE = checking::SystemVerifier::getSystemVerifier()->getKeySize();

  testing_result_file_during_insertion << ",\"PLRDF Number Of Total Ranges\" : " << db->getPLRDFNumberOfTotalRanges() << std::endl;
  testing_result_file_during_insertion << ",\"Split PLRDF Number Of Total Ranges\" : " << db->getSplitPLRDFNumberOfTotalRanges() << std::endl;
  testing_result_file_during_insertion << ",\"TopLevel RDF Number Of Total Ranges\" : " << db->getTopLevelRDFNumberOfTotalRanges() << std::endl;
  testing_result_file_during_insertion << ",\"Skyline RDF Number Of Total Ranges\" : " << db->getSkylineRDFNumberOfTotalRanges() << std::endl;
  testing_result_file_during_insertion << ",\"SuRF Level File RDF Number Of Total Ranges\" : " << db->getSuRFLevelFileRDFNumberOfTotalRanges() << std::endl;
  testing_result_file_during_insertion << ",\"SuRF Level File Split RDF Number Of Total Ranges\" : " << db->getSuRFLevelFileSplitRDFNumberOfTotalRanges() << std::endl;

  
  
  /*Numbers of Range Tombstones*/
  testing_result_file_during_insertion << ",\"Log Of Numbers Of Ranges In PLRDF\" : [";
  for(int i = 0; i < ranges_log_PLRDF.size(); i++){
    testing_result_file_during_insertion << ranges_log_PLRDF[i];
    if(i != ranges_log_PLRDF.size() - 1){
      testing_result_file_during_insertion << ", ";
    }
  }
  testing_result_file_during_insertion << "]" << std::endl;
  testing_result_file_during_insertion << ",\"Log Of Numbers Of Ranges In SplitPLRDF\" : [";
  for(int i = 0; i < ranges_log_SplitPLRDF.size(); i++){
    testing_result_file_during_insertion << ranges_log_SplitPLRDF[i];
    if(i != ranges_log_SplitPLRDF.size() - 1){
      testing_result_file_during_insertion << ", ";
    }
  }
  testing_result_file_during_insertion << "]" << std::endl;
  testing_result_file_during_insertion << ",\"Log Of Numbers Of Ranges In TopLevelRDF\" : [";
  for(int i = 0; i < ranges_log_TopLevelRDF.size(); i++){
    testing_result_file_during_insertion << ranges_log_TopLevelRDF[i];
    if(i != ranges_log_TopLevelRDF.size() - 1){
      testing_result_file_during_insertion << ", ";
    }
  }
  testing_result_file_during_insertion << "]" << std::endl;;
  testing_result_file_during_insertion << ",\"Log Of Numbers Of Ranges In SkylineRDF\" : [";
  for(int i = 0; i < ranges_log_SkylineRDF.size(); i++){
    testing_result_file_during_insertion << ranges_log_SkylineRDF[i];
    if(i != ranges_log_SkylineRDF.size() - 1){
      testing_result_file_during_insertion << ", ";
    }
  }
  testing_result_file_during_insertion << "]" << std::endl;
  testing_result_file_during_insertion << ",\"Log Of Numbers Of Ranges In SuRFLevelFileRDF\" : [";
  for(int i = 0; i < ranges_log_SuRFLevelFileRDF.size(); i++){
    testing_result_file_during_insertion << ranges_log_SuRFLevelFileRDF[i];
    if(i != ranges_log_SuRFLevelFileRDF.size() - 1){
      testing_result_file_during_insertion << ", ";
    }
  }
  testing_result_file_during_insertion << "]" << std::endl;
  testing_result_file_during_insertion << ",\"Log Of Numbers Of Ranges In SuRFLevelFileSplitRDF\" : [";
  for(int i = 0; i < ranges_log_SuRFLevelFileSplitRDF.size(); i++){
    testing_result_file_during_insertion << ranges_log_SuRFLevelFileSplitRDF[i];
    if(i != ranges_log_SuRFLevelFileSplitRDF.size() - 1){
      testing_result_file_during_insertion << ", ";
    }
  }
  testing_result_file_during_insertion << "]" << std::endl;


 /*memory usage*/
 testing_result_file_during_insertion << ",\"Log Of Memory Usage Of PLRDF\" : [";
  for(int i = 0; i < memory_usage_log_PLRDF.size(); i++){
    testing_result_file_during_insertion << memory_usage_log_PLRDF[i];
    if(i != memory_usage_log_PLRDF.size() - 1){
      testing_result_file_during_insertion << ", ";
    }
  }
  testing_result_file_during_insertion << "]" << std::endl;
  testing_result_file_during_insertion << ",\"Log Of Memory Usage Of SplitPLRDF\" : [";
  for(int i = 0; i < memory_usage_log_SplitPLRDF.size(); i++){
    testing_result_file_during_insertion << memory_usage_log_SplitPLRDF[i];
    if(i != memory_usage_log_SplitPLRDF.size() - 1){
      testing_result_file_during_insertion << ", ";
    }
  }
  testing_result_file_during_insertion << "]" << std::endl;
  testing_result_file_during_insertion << ",\"Log Of Memory Usage Of TopLevelRDF\" : [";
  for(int i = 0; i < memory_usage_log_TopLevelRDF.size(); i++){
    testing_result_file_during_insertion << memory_usage_log_TopLevelRDF[i];
    if(i != memory_usage_log_TopLevelRDF.size() - 1){
      testing_result_file_during_insertion << ", ";
    }
  }
  testing_result_file_during_insertion << "]" << std::endl;;
  testing_result_file_during_insertion << ",\"Log Of Memory Usage Of SkylineRDF\" : [";
  for(int i = 0; i < memory_usage_log_SkylineRDF.size(); i++){
    testing_result_file_during_insertion << memory_usage_log_SkylineRDF[i];
    if(i != memory_usage_log_SkylineRDF.size() - 1){
      testing_result_file_during_insertion << ", ";
    }
  }
  testing_result_file_during_insertion << "]" << std::endl; 
  testing_result_file_during_insertion << ",\"Log Of Memory Usage Of SuRFLevelFileRDF\" : [";
  for(int i = 0; i < memory_usage_log_SuRFLevelFileRDF.size(); i++){
    testing_result_file_during_insertion << memory_usage_log_SuRFLevelFileRDF[i];
    if(i != memory_usage_log_SuRFLevelFileRDF.size() - 1){
      testing_result_file_during_insertion << ", ";
    }
  }
  testing_result_file_during_insertion << "]" << std::endl;
  testing_result_file_during_insertion << ",\"Log Of Memory Usage Of SuRFLevelFileSplitRDF\" : [";
  for(int i = 0; i < memory_usage_log_SuRFLevelFileSplitRDF.size(); i++){
    testing_result_file_during_insertion << memory_usage_log_SuRFLevelFileSplitRDF[i];
    if(i != memory_usage_log_SuRFLevelFileSplitRDF.size() - 1){
      testing_result_file_during_insertion << ", ";
    }
  }
  testing_result_file_during_insertion << "]" << std::endl;

  running_log_during_insertion << "writeRecord End " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

}

// // void verification_runner::runPQVerification(DB** db_ptr2, 
// //                                             Options& op, 
// //                                             WriteOptions& write_op, 
// //                                             ReadOptions& read_op, 
// //                                             EmuEnv* _env){


void LoggerDuringInsertion::runPQonCurrentlyDeletedKeys(
                                                        uint i_insertion,
                                                        DB** db_ptr2, 
                                                        Options& op, 
                                                        WriteOptions& write_op, 
                                                        ReadOptions& read_op, 
                                                        EmuEnv* _env,
                                                        int number_of_PQs,
                                                        string kDBPath){
                                                          
  DB* db = *db_ptr2;
  Status s;
  
  checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();
  int KEY_SIZE = checking::SystemVerifier::getSystemVerifier()->getKeySize();

  running_log_during_insertion << "!!! Testing On Currently Deleted Keys " << std::endl;
  
  const long long N_repetitions = checking::SystemVerifier::EXPERIMENT_REPETITION_TIMES;
  // long long num_RDF_types = getNumberOfRDFTypes();
  long long disk_access_count = 0;

  
  std::string prefix_number_of_PQs = "";
  if(number_of_PQs != -1){
    prefix_number_of_PQs = "fixed #PQ = " + std::to_string(number_of_PQs);
  }
  system_verifier->gen_workload_with_numbers_of_PQ(N_repetitions, number_of_PQs);

  string i_insertion_str = "i_insert="+std::to_string(i_insertion)+" ";
  testing_result_file_during_insertion << system_verifier->getCurrentlyDeletedKeysVec2dString(",", "\"", i_insertion_str+prefix_number_of_PQs) << std::endl;

  running_log_during_insertion << "@A1 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
  auto start_pq = std::chrono::high_resolution_clock::now();
  auto stop_pq = std::chrono::high_resolution_clock::now();
  // auto duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
  auto duration_pq = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_pq - start_pq);
  unsigned long long point_query_time = duration_pq.count();

  // testing_result_file << std::endl << std::endl;
  // testing_result_file << "----------------------Testing On historic-existing Keys-----------------------" << std::endl;
  for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
    long long total_read_count = 0;
    long long total_read_bytes = 0;
    testing_logger.reset();

  running_log_during_insertion << "@A2 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;



    system_verifier->setRDFTypeChosed(t);
    system_verifier->resetAllCount();
    system_verifier->resetAllDuration();
    
    if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
      s = db->SetOptions({{"max_open_files", "1"}}); // is there any compaction happended after this????
    }else{
      s = db->SetOptions({{"max_open_files", std::to_string(_env->max_open_files)}}); // is there any compaction happended after this????
    }

    // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << std::endl;
    // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << prefix_number_of_PQs << std::endl;
    disk_access_count = 0;
    point_query_time = 0;
    start_pq = std::chrono::high_resolution_clock::now();
    rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
    rocksdb::get_perf_context()->Reset();
    rocksdb::get_iostats_context()->Reset();
    for(auto i = 0; i < N_repetitions; i++){
      clearCache(op);
      db = *db_ptr2;
  running_log_during_insertion << "@A3 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

      system_verifier->resetDiskAccessCount();
      testing_logger.set_to_start(op);



      // if(flag_reopen_db_for_each_RDF_testing == true){
      //   // retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
      //   retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime);
      //   reopen_DB(db_ptr2, op, write_op, read_op, _env, kDBPath);
      //   // set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
      //   set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime);
      // }
    //   for(auto &x: system_verifier->getHistoricExistingKeys()){
      for(auto x: system_verifier->getHistoricExistingKeysAtNRound(i)){
        bool gt_is_exist = system_verifier->isKeyExist(x);
        std::string gt_value = system_verifier->get(x);

        std::string value;
        std::string time_stamp;
        std::stringstream searching_key;
        searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;

        start_pq = std::chrono::high_resolution_clock::now();
        system_verifier->start_remaining_get_path();
        s = db->Get(read_op, searching_key.str(), &value);
        system_verifier->stop_remaining_get_path(); 
        stop_pq = std::chrono::high_resolution_clock::now();

        duration_pq = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_pq - start_pq);
        point_query_time += duration_pq.count();
        if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
          continue;
        }
        size_t separator_pos = value.find("|");
        time_stamp = value.substr(separator_pos + 1);
        value = value.substr(0, separator_pos);

        if(s.ok() != gt_is_exist){
          #ifdef DEBUG_VERIFICATION
            std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
          #endif
          running_log_during_insertion << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
        }
        if(gt_is_exist == false){continue;}
        if(value != gt_value){
          #ifdef DEBUG_VERIFICATION
            std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
          #endif
          running_log_during_insertion << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
        }
      }
      if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
        continue;
      }
      disk_access_count += system_verifier->getDiskAccessCount();

      running_log_during_insertion << " Disk Access count = " << system_verifier->getDiskAccessCount() << std::endl;
      testing_logger.set_to_end(op, running_log_during_insertion);
    }  
    if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
      continue;
    }
    double block_read_cpu_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), ".*block_read_cpu_time = ([0-9.]+)");
  running_log_during_insertion << "@A4 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

    running_log_during_insertion << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*disk_access_count/N_repetitions << std::endl;
    running_log_during_insertion << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions/1e6 << " (ms) " << std::endl << std::endl;
    running_log_during_insertion << "filtered by RDF count = " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl; 
    running_log_during_insertion << "number of PQ = " << system_verifier->getHistoricExistingKeys().size() << std::endl;
    running_log_during_insertion << system_verifier->getAllCount("", "", "", N_repetitions) << std::endl;
    running_log_during_insertion << "block_read_cpu_time = " << 1.0*block_read_cpu_time/N_repetitions/1e3  << "" << std::endl;

    // std::string prefix = " (Historcially Exist Keys) " + system_verifier->getStringOfRDFTypeChosed() + " ";
    std::string prefix = " (Historcially Exist Keys " + i_insertion_str + prefix_number_of_PQs + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
    testing_result_file_during_insertion << ",\"" + prefix + " elapsed time\" : " << 1.0*point_query_time/N_repetitions/1e6 << std::endl;
    testing_result_file_during_insertion << ",\"" + prefix + " filtered by RDF count\" : " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl;
    if (number_of_PQs == -1){
      testing_result_file_during_insertion << ",\"" + prefix + " number of PQ\" : " << system_verifier->getHistoricExistingKeys().size() << std::endl;
    }else{
      testing_result_file_during_insertion << ",\"" + prefix + " number of PQ\" : " << number_of_PQs << std::endl;
    }
    testing_result_file_during_insertion << system_verifier->getAllCount(",", "\"", prefix, N_repetitions) << std::endl;
    testing_result_file_during_insertion << ",\"" + prefix + " block_read_cpu_time\" : " << 1.0*block_read_cpu_time/N_repetitions/1e3 << std::endl;

    testing_logger.output_statistics(running_log_during_insertion, testing_result_file_during_insertion, prefix);
  }
  system_verifier->reset_flag_testing_on_currently_deleted_keys();
running_log_during_insertion << "@A5 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  running_log_during_insertion << "!!! Testing On Currently Deleted Keys " << std::endl;
  system_verifier->set_flag_testing_on_currently_deleted_keys();
  system_verifier->startPQTracing();

  running_log_during_insertion << std::endl << std::endl;
running_log_during_insertion << "@A6 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
}                     

// void verification_runner::runPQVerification(DB** db_ptr2, 
//                                             Options& op, 
//                                             WriteOptions& write_op, 
//                                             ReadOptions& read_op, 
//                                             EmuEnv* _env,
//                                             int number_of_PQs,
//                                             string kDBPath){
// std::cout << "number_of_PQ = " << number_of_PQs <<  std::endl; 
//   assert(number_of_PQs != 0);
//   assert(number_of_PQs >= -1);
  

//   DB* db = *db_ptr2;
//   Status s;
  
//   checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();
//   int KEY_SIZE = checking::SystemVerifier::getSystemVerifier()->getKeySize();

//   long long total_read_count = 0;
//   long long total_read_bytes = 0;

//   const long long N_repetitions = checking::SystemVerifier::EXPERIMENT_REPETITION_TIMES;
//   // long long num_RDF_types = getNumberOfRDFTypes();
//   long long disk_access_count = 0;

  
//   std::string prefix_number_of_PQs = "";
//   if(number_of_PQs != -1){
//     prefix_number_of_PQs = "fixed #PQ = " + std::to_string(number_of_PQs);
//   }
//   system_verifier->gen_workload_with_numbers_of_PQ(N_repetitions, number_of_PQs);

//   testing_result_file_during_insertion << system_verifier->getCurrentlyDeletedKeysVec2dString(",", "\"", prefix_number_of_PQs) << std::endl;


//   auto start_pq = std::chrono::high_resolution_clock::now();
//   auto stop_pq = std::chrono::high_resolution_clock::now();
//   // auto duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
//   auto duration_pq = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_pq - start_pq);
//   unsigned long long point_query_time = duration_pq.count();

// std::cout << "!!! Testing On Existing Keys " << std::endl;

//   testing_result_file << std::endl << std::endl;
//   testing_result_file << "----------------------Testing On Existing Keys-----------------------" << std::endl; 
//   for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
//     system_verifier->setRDFTypeChosed(t);
//     system_verifier->resetAllCount();
//     system_verifier->resetAllDuration();
    
//     if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
//       s = db->SetOptions({{"max_open_files", "1"}}); // is there any compaction happended after this????
//     }else{
//       s = db->SetOptions({{"max_open_files", std::to_string(_env->max_open_files)}}); // is there any compaction happended after this????
//     }

//     // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << std::endl;
//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << prefix_number_of_PQs << std::endl;
//     disk_access_count = 0;
//     point_query_time = 0;
//     start_pq = std::chrono::high_resolution_clock::now();
//     rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
//     rocksdb::get_perf_context()->Reset();
//     rocksdb::get_iostats_context()->Reset();
//     for(auto i = 0; i < N_repetitions; i++){
//       clearCache(op);

//       db = *db_ptr2;

//       system_verifier->resetDiskAccessCount();
//       testing_logger.set_to_start(op);
    


//       if(flag_reopen_db_for_each_RDF_testing == true){
//         // retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
//         retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime);
//         reopen_DB(db_ptr2, op, write_op, read_op, _env, kDBPath);
//         // set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
//         set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime);
//       }
//     //   for(auto &x: system_verifier->getAllExistingKeys()){
//       for(auto x: system_verifier->getAllExistingKeysAtNRound(i)){
//         bool gt_is_exist = system_verifier->isKeyExist(x);
//         std::string gt_value = system_verifier->get(x);

//         std::string value;
//         std::string time_stamp;
//         std::stringstream searching_key;
//         searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;

//         start_pq = std::chrono::high_resolution_clock::now();
//         system_verifier->start_remaining_get_path();
//         s = db->Get(read_op, searching_key.str(), &value); 
//         system_verifier->stop_remaining_get_path(); 
//         stop_pq = std::chrono::high_resolution_clock::now();

//         duration_pq = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_pq - start_pq);
//         point_query_time += duration_pq.count();
//         if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
//           continue;
//         }
//         size_t separator_pos = value.find("|");
//         time_stamp = value.substr(separator_pos + 1);
//         value = value.substr(0, separator_pos);

//         if(s.ok() != gt_is_exist){
//           #ifdef DEBUG_VERIFICATION
//             std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
//           #endif  
//           testing_result_file << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
//         }
//         if(gt_is_exist == false){continue;}
//         if(value != gt_value){
//           #ifdef DEBUG_VERIFICATION
//               std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
//           #endif
//           testing_result_file << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
//         }
//       }
//       if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
//         continue;
//       }
//       disk_access_count += system_verifier->getDiskAccessCount();

//       testing_result_file << " Disk Access count = " << system_verifier->getDiskAccessCount() << std::endl;
//       testing_logger.set_to_end(op, testing_result_file);
//     }  
//     if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
//       continue;
//     }
//     double block_read_cpu_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), ".*block_read_cpu_time = ([0-9.]+)");

//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*disk_access_count/N_repetitions << std::endl;
//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions/1e6 << " (ms) " << std::endl << std::endl;
//     testing_result_file << "filtered by RDF count = " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl; 
//     testing_result_file << "number of PQ = " << system_verifier->getAllExistingKeys().size() << std::endl;
//     testing_result_file << system_verifier->getAllCount("", "", "", N_repetitions) << std::endl;
//     testing_result_file << "block_read_cpu_time = " << 1.0*block_read_cpu_time/N_repetitions/1e3  << "" << std::endl;
 
//     // std::string prefix = " (Exist Keys) " + system_verifier->getStringOfRDFTypeChosed() + " ";
//     std::string prefix = " (Exist Keys " + prefix_number_of_PQs + ") "+ system_verifier->getStringOfRDFTypeChosed() + " ";
//     testing_result_file_during_insertion << ",\"" + prefix + " elapsed time\" : " << 1.0*point_query_time/N_repetitions/1e6 << std::endl;
//     testing_result_file_during_insertion << ",\"" + prefix + " filtered by RDF count\" : " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl;
//     if (number_of_PQs == -1){
//       testing_result_file_during_insertion << ",\"" + prefix + " number of PQ\" : " << system_verifier->getAllExistingKeys().size() << std::endl;
//     }else{
//       testing_result_file_during_insertion << ",\"" + prefix + " number of PQ\" : " << number_of_PQs << std::endl;
//     }
//     testing_result_file_during_insertion << system_verifier->getAllCount(",", "\"", prefix, N_repetitions) << std::endl;
//     testing_result_file_during_insertion << ",\"" + prefix + " block_read_cpu_time\" : " << 1.0*block_read_cpu_time/N_repetitions/1e3 << std::endl;

//     testing_logger.output_statistics(testing_result_file, testing_result_file_during_insertion, prefix);
//   }

// std::cout << "!!! Testing On historic-existing Keys " << std::endl;
// system_verifier->set_flag_testing_on_currently_deleted_keys();

//   testing_result_file << std::endl << std::endl;
//   testing_result_file << "----------------------Testing On historic-existing Keys-----------------------" << std::endl;
//   for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
//     long long total_read_count = 0;
//     long long total_read_bytes = 0;
//     testing_logger.reset();




//     system_verifier->setRDFTypeChosed(t);
//     system_verifier->resetAllCount();
//     system_verifier->resetAllDuration();
    
//     if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
//       s = db->SetOptions({{"max_open_files", "1"}}); // is there any compaction happended after this????
//     }else{
//       s = db->SetOptions({{"max_open_files", std::to_string(_env->max_open_files)}}); // is there any compaction happended after this????
//     }

//     // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << std::endl;
//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << prefix_number_of_PQs << std::endl;
//     disk_access_count = 0;
//     point_query_time = 0;
//     start_pq = std::chrono::high_resolution_clock::now();
//     rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
//     rocksdb::get_perf_context()->Reset();
//     rocksdb::get_iostats_context()->Reset();
//     for(auto i = 0; i < N_repetitions; i++){
//       clearCache(op);
//       db = *db_ptr2;

//       system_verifier->resetDiskAccessCount();
//       testing_logger.set_to_start(op);



//       if(flag_reopen_db_for_each_RDF_testing == true){
//         // retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
//         retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime);
//         reopen_DB(db_ptr2, op, write_op, read_op, _env, kDBPath);
//         // set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
//         set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime);
//       }
//     //   for(auto &x: system_verifier->getHistoricExistingKeys()){
//       for(auto x: system_verifier->getHistoricExistingKeysAtNRound(i)){
//         bool gt_is_exist = system_verifier->isKeyExist(x);
//         std::string gt_value = system_verifier->get(x);

//         std::string value;
//         std::string time_stamp;
//         std::stringstream searching_key;
//         searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;

//         start_pq = std::chrono::high_resolution_clock::now();
//         system_verifier->start_remaining_get_path();
//         s = db->Get(read_op, searching_key.str(), &value);
//         system_verifier->stop_remaining_get_path(); 
//         stop_pq = std::chrono::high_resolution_clock::now();

//         duration_pq = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_pq - start_pq);
//         point_query_time += duration_pq.count();
//         if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
//           continue;
//         }
//         size_t separator_pos = value.find("|");
//         time_stamp = value.substr(separator_pos + 1);
//         value = value.substr(0, separator_pos);

//         if(s.ok() != gt_is_exist){
//           #ifdef DEBUG_VERIFICATION
//             std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
//           #endif
//           testing_result_file << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
//         }
//         if(gt_is_exist == false){continue;}
//         if(value != gt_value){
//           #ifdef DEBUG_VERIFICATION
//             std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
//           #endif
//           testing_result_file << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
//         }
//       }
//       if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
//         continue;
//       }
//       disk_access_count += system_verifier->getDiskAccessCount();

//       testing_result_file << " Disk Access count = " << system_verifier->getDiskAccessCount() << std::endl;
//       testing_logger.set_to_end(op, testing_result_file);
//     }  
//     if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
//       continue;
//     }
//     double block_read_cpu_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), ".*block_read_cpu_time = ([0-9.]+)");

//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*disk_access_count/N_repetitions << std::endl;
//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions/1e6 << " (ms) " << std::endl << std::endl;
//     testing_result_file << "filtered by RDF count = " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl; 
//     testing_result_file << "number of PQ = " << system_verifier->getHistoricExistingKeys().size() << std::endl;
//     testing_result_file << system_verifier->getAllCount("", "", "", N_repetitions) << std::endl;
//     testing_result_file << "block_read_cpu_time = " << 1.0*block_read_cpu_time/N_repetitions/1e3  << "" << std::endl;

//     // std::string prefix = " (Historcially Exist Keys) " + system_verifier->getStringOfRDFTypeChosed() + " ";
//     std::string prefix = " (Historcially Exist Keys " + prefix_number_of_PQs + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
//     testing_result_file_during_insertion << ",\"" + prefix + " elapsed time\" : " << 1.0*point_query_time/N_repetitions/1e6 << std::endl;
//     testing_result_file_during_insertion << ",\"" + prefix + " filtered by RDF count\" : " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl;
//     if (number_of_PQs == -1){
//       testing_result_file_during_insertion << ",\"" + prefix + " number of PQ\" : " << system_verifier->getHistoricExistingKeys().size() << std::endl;
//     }else{
//       testing_result_file_during_insertion << ",\"" + prefix + " number of PQ\" : " << number_of_PQs << std::endl;
//     }
//     testing_result_file_during_insertion << system_verifier->getAllCount(",", "\"", prefix, N_repetitions) << std::endl;
//     testing_result_file_during_insertion << ",\"" + prefix + " block_read_cpu_time\" : " << 1.0*block_read_cpu_time/N_repetitions/1e3 << std::endl;

//     testing_logger.output_statistics(testing_result_file, testing_result_file_during_insertion, prefix);

//   }
// system_verifier->reset_flag_testing_on_currently_deleted_keys();

// std::cout << "!!! Testing On Currently Deleted Keys " << std::endl;
// system_verifier->set_flag_testing_on_currently_deleted_keys();
// system_verifier->startPQTracing();

//   testing_result_file << std::endl << std::endl;
//   testing_result_file << "----------------------Testing On Currently Deleted Keys-----------------------" << std::endl;

//   for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
//     long long total_read_count = 0;
//     long long total_read_bytes = 0;
//     testing_logger.reset();



//     system_verifier->setRDFTypeChosed(t);
//     system_verifier->resetAllCount();
//     system_verifier->resetAllDuration();
    
//     if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
//       s = db->SetOptions({{"max_open_files", "1"}}); // is there any compaction happended after this????
//     }else{
//       s = db->SetOptions({{"max_open_files", std::to_string(_env->max_open_files)}}); // is there any compaction happended after this????
//     }

//     // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << std::endl;
//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << prefix_number_of_PQs << std::endl;
//     disk_access_count = 0;
//     point_query_time = 0;
//     start_pq = std::chrono::high_resolution_clock::now();
//     rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
//     rocksdb::get_perf_context()->Reset();
//     rocksdb::get_iostats_context()->Reset();
//     for(auto i = 0; i < N_repetitions; i++){
//       clearCache(op);
//       system_verifier->clearMapPQTracingInfo();
//       system_verifier->clearVPQTracingInfo();

//       db = *db_ptr2;
//       system_verifier->resetDiskAccessCount();
//       testing_logger.set_to_start(op);



//       if(flag_reopen_db_for_each_RDF_testing == true){
//         // retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
//         retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime);
//         reopen_DB(db_ptr2, op, write_op, read_op, _env, kDBPath);
//         // set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
//         set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime);
//       }
//     //   for(auto &x: system_verifier->getCurrentlyDeletedKeys()){
//       for(auto x: system_verifier->getCurrentlyDeletedKeysAtNRound(i)){
// // cout << "x = " << x << " " << __FILE__ << ":" << __LINE__ << endl;
//         bool gt_is_exist = system_verifier->isKeyExist(x);
//         std::string gt_value = system_verifier->get(x);

//         std::string value;
//         std::string time_stamp;
//         std::stringstream searching_key;
//         searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;

//         start_pq = std::chrono::high_resolution_clock::now();
//         system_verifier->start_remaining_get_path();
//         s = db->Get(read_op, searching_key.str(), &value);
//         system_verifier->stop_remaining_get_path(); 
//         stop_pq = std::chrono::high_resolution_clock::now();
//         // duration_pq = std::chrono::duration_cast<std::chrono::microseconds>(stop_pq - start_pq);
//         duration_pq = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_pq - start_pq);
//         point_query_time += duration_pq.count();
//         if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
//           continue;
//         }
//         size_t separator_pos = value.find("|");
//         time_stamp = value.substr(separator_pos + 1);
//         value = value.substr(0, separator_pos);

//         if(s.ok() != gt_is_exist){
//           #ifdef DEBUG_VERIFICATION
//             std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
//           #endif
//           testing_result_file << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
//         }
//         if(gt_is_exist == false){continue;}
//         if(value != gt_value){
//           #ifdef DEBUG_VERIFICATION
//             std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
//           #endif
//           testing_result_file << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
//         }
//       }
//       if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
//         continue;
//       }
//       disk_access_count += system_verifier->getDiskAccessCount();


//       testing_result_file << " Disk Access count = " << system_verifier->getDiskAccessCount() << std::endl;
//       testing_logger.set_to_end(op, testing_result_file);


//       std::string prefix = " (Currently Deleted Keys " + prefix_number_of_PQs + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
//       testing_result_file_during_insertion << system_verifier->getMapPQTracingInfo(",", "\"", prefix, i) << std::endl;
//       testing_result_file_during_insertion << system_verifier->getVPQTracingInfo(",", "\"", prefix, i) << std::endl;
//       // system_verifier->clearMapPQTracingInfo();
//       // system_verifier->clearVPQTracingInfo();
//     }  
//     if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
//       continue;
//     }
//     double block_read_cpu_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), ".*block_read_cpu_time = ([0-9.]+)");

//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*disk_access_count/N_repetitions << std::endl;
//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions/1e6 << " (ms) " << std::endl << std::endl;
//     testing_result_file << "filtered by RDF count = " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl; 
//     testing_result_file << "number of PQ = " << system_verifier->getCurrentlyDeletedKeys().size() << std::endl;
//     testing_result_file << system_verifier->getAllCount("", "", "", N_repetitions) << std::endl;
//     testing_result_file << "block_read_cpu_time = " << 1.0*block_read_cpu_time/N_repetitions/1e3  << "" << std::endl;

//     // std::string prefix = " (Currently Deleted Keys) " + system_verifier->getStringOfRDFTypeChosed() + " ";
//     std::string prefix = " (Currently Deleted Keys " + prefix_number_of_PQs + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
//     testing_result_file_during_insertion << ",\"" + prefix + " elapsed time\" : " << 1.0*point_query_time/N_repetitions/1e6 << std::endl;
//     testing_result_file_during_insertion << ",\"" + prefix + " filtered by RDF count\" : " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl;
//     if (number_of_PQs == -1){
//       testing_result_file_during_insertion << ",\"" + prefix + " number of PQ\" : " << system_verifier->getCurrentlyDeletedKeys().size() << std::endl;
//     }else{
//       testing_result_file_during_insertion << ",\"" + prefix + " number of PQ\" : " << number_of_PQs << std::endl;
//     }
//     testing_result_file_during_insertion << system_verifier->getAllCount(",", "\"", prefix, N_repetitions) << std::endl;
//     testing_result_file_during_insertion << ",\"" + prefix + " block_read_cpu_time\" : " << 1.0*block_read_cpu_time/N_repetitions/1e3 << std::endl;

//     testing_logger.output_statistics(testing_result_file, testing_result_file_during_insertion, prefix);

    
//   }
// system_verifier->reset_flag_testing_on_currently_deleted_keys();
// system_verifier->endPQTracing();

// std::cout << "!!! Testing On Currently Non-inserted Keys " << std::endl;

//   testing_result_file << std::endl << std::endl;
//   testing_result_file << "----------------------Testing On Currently Non-inserted Keys-----------------------" << std::endl;
//   int CurrentlyNonInsertedKeysNum = system_verifier->getCurrentlyNonInsertedKeysNum();
//   testing_result_file << "number of currently non-inserted keys = " << CurrentlyNonInsertedKeysNum << std::endl;
// //   system_verifier->genCurrentlyNonInsertedKeys(CurrentlyNonInsertedKeysNum);
//   for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
//     long long total_read_count = 0;
//     long long total_read_bytes = 0;
//     testing_logger.reset();


//     system_verifier->setRDFTypeChosed(t);
//     system_verifier->resetAllCount();
//     system_verifier->resetAllDuration();
    
//     if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
//       s = db->SetOptions({{"max_open_files", "1"}}); // is there any compaction happended after this????
//     }else{
//       s = db->SetOptions({{"max_open_files", std::to_string(_env->max_open_files)}}); // is there any compaction happended after this????
//     }

//     // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << std::endl;
//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << prefix_number_of_PQs << std::endl;
//     disk_access_count = 0;
//     point_query_time = 0;
//     start_pq = std::chrono::high_resolution_clock::now();
//     rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
//     rocksdb::get_perf_context()->Reset();
//     rocksdb::get_iostats_context()->Reset();
//     for(auto i = 0; i < N_repetitions; i++){
//       clearCache(op);
//       db = *db_ptr2;
//       system_verifier->resetDiskAccessCount();
//       testing_logger.set_to_start(op);



//       if(flag_reopen_db_for_each_RDF_testing == true){
//         // retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
//         retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime);
//         reopen_DB(db_ptr2, op, write_op, read_op, _env, kDBPath);
//         // set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
//         set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime);
//       }
//     //   for(auto &x: system_verifier->getCurrentlyNonInsertedKeys()){ // test on 1000 keys
//       for(auto x: system_verifier->getCurrentlyNonInsertedKeysAtNRound(i)){ // test on 1000 keys
//         bool gt_is_exist = system_verifier->isKeyExist(x); // should be false
//         std::string gt_value = system_verifier->get(x); // should be ""

//         std::string value;
//         std::string time_stamp;
//         std::stringstream searching_key;
//         searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;

//         start_pq = std::chrono::high_resolution_clock::now();
//         system_verifier->start_remaining_get_path();
//         s = db->Get(read_op, searching_key.str(), &value);
//         system_verifier->stop_remaining_get_path(); 
//         stop_pq = std::chrono::high_resolution_clock::now();
//         duration_pq = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_pq - start_pq);
//         point_query_time += duration_pq.count();
//         if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
//           continue;
//         }
//         size_t separator_pos = value.find("|");
//         time_stamp = value.substr(separator_pos + 1);
//         value = value.substr(0, separator_pos);
//         if(s.ok() != gt_is_exist){
//           #ifdef DEBUG_VERIFICATION
//             std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
//           #endif
//           testing_result_file << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
//         }
//         if(gt_is_exist == false){continue;}
//         if(value != gt_value){
//           #ifdef DEBUG_VERIFICATION
//             std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
//           #endif
//           testing_result_file << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
//         }
//       }
//       if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
//         continue;
//       }
//       disk_access_count += system_verifier->getDiskAccessCount();
 

//       testing_result_file << " Disk Access count = " << system_verifier->getDiskAccessCount() << std::endl;
//       testing_logger.set_to_end(op, testing_result_file);
//     }  
//     if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
//       continue;
//     }
//     double block_read_cpu_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), ".*block_read_cpu_time = ([0-9.]+)");

//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*disk_access_count/N_repetitions << std::endl;
//     testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions/1e6 << " (ms) " << std::endl << std::endl;
//     testing_result_file << "filtered by RDF count = " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl; 
//     testing_result_file << "number of PQ = " << system_verifier->getCurrentlyNonInsertedKeys().size() << std::endl;
//     testing_result_file << system_verifier->getAllCount("", "", "", N_repetitions) << std::endl;
//     testing_result_file << "block_read_cpu_time = " << 1.0*block_read_cpu_time/N_repetitions/1e3  << "" << std::endl;

//     // std::string prefix = " (Non-inserted Keys) " + system_verifier->getStringOfRDFTypeChosed() + " ";
//     std::string prefix = " (Non-inserted Keys " + prefix_number_of_PQs + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
//     testing_result_file_during_insertion << ",\"" + prefix + " elapsed time\" : " << 1.0*point_query_time/N_repetitions/1e6 << std::endl;
//     testing_result_file_during_insertion << ",\"" + prefix + " filtered by RDF count\" : " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl;
//     if (number_of_PQs == -1){
//       testing_result_file_during_insertion << ",\"" + prefix + " number of PQ\" : " << system_verifier->getCurrentlyNonInsertedKeys().size() << std::endl;
//     }else{
//       testing_result_file_during_insertion << ",\"" + prefix + " number of PQ\" : " << number_of_PQs << std::endl;
//     }
//     testing_result_file_during_insertion << system_verifier->getAllCount(",", "\"", prefix, N_repetitions) << std::endl;
//     testing_result_file_during_insertion << ",\"" + prefix + " block_read_cpu_time\" : " << 1.0*block_read_cpu_time/N_repetitions/1e3 << std::endl;

//     testing_logger.output_statistics(testing_result_file, testing_result_file_during_insertion, prefix);
//   }


// }

void LoggerDuringInsertion::end(){
  // checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();

  // testing_result_file.close();
  // testing_result_file_during_insertion.close();

  
  // testing_result_file << std::endl << std::endl;
  // testing_result_file << "----------------------End Testing-----------------------" << std::endl;

  // testing_result_file.close();

  running_log_during_insertion << std::endl << std::endl;
  running_log_during_insertion << "----------------------End Testing-----------------------" << std::endl;
  running_log_during_insertion.close();
  
  testing_result_file_during_insertion << ",\"End\" : \"End\""<< std::endl;
  testing_result_file_during_insertion << "}"<< std::endl;
  testing_result_file_during_insertion.close();
  // Status s = DB::Open(op, kDBPath, &db);

  // system_verifier->resetRunningPQ();
}

#endif // UTILS_LOGGER_DURING_INSERTION_H