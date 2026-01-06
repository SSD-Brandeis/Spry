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
    std::ofstream testing_result_file_during_insertion, running_log_during_insertion;

    static LoggerDuringInsertion *logger_during_insertion;

    bool flag_reopen_db_for_each_RDF_testing = false;
    

    vector<int> ranges_log_Origin;
    vector<int> ranges_log_PLRDF;
    vector<int> ranges_log_SplitPLRDF;
    vector<int> ranges_log_PLRDFStringKey;
    vector<int> ranges_log_SplitPLRDFStringKey;
    vector<int> ranges_log_TopLevelRDF;
    vector<int> ranges_log_TopLevelRDFStringKey;
    vector<int> ranges_log_SkylineRDF;
    vector<int> ranges_log_SuRFLevelFileRDF;
    vector<int> ranges_log_SuRFLevelFileSplitRDF;
    
    vector<int> memory_usage_log_Origin;
    vector<int> memory_usage_log_PLRDF;
    vector<int> memory_usage_log_SplitPLRDF;
    vector<int> memory_usage_log_PLRDFStringKey;
    vector<int> memory_usage_log_SplitPLRDFStringKey;
    vector<int> memory_usage_log_TopLevelRDF;
    vector<int> memory_usage_log_TopLevelRDFStringKey;
    vector<int> memory_usage_log_SkylineRDF;
    vector<int> memory_usage_log_SuRFLevelFileRDF;
    vector<int> memory_usage_log_SuRFLevelFileSplitRDF;
    TestingLogger testing_logger;

public:
    static LoggerDuringInsertion* getInstance(EmuEnv* _env);
    void start(EmuEnv* _env);
    void recordCurrentMemoryFootprint(DB** db_ptr2);
    void writeRecord(DB** db_ptr2);
    // void runPQVerification(DB** db_ptr2, Options& op, WriteOptions& write_op, 
    //                         ReadOptions& read_op, EmuEnv* _env, int number_of_PQs = -1,
    //                         string kDBPath = "/tmp/cs561_project1");
    // void runPQVerification(DB** db_ptr2, Options& op, WriteOptions& write_op, 
    //                         ReadOptions& read_op, EmuEnv* _env,
    //                         int number_of_PQs_on_existing_keys = -1,
    //                         int number_of_PQs_on_historic_existing_keys = -1,
    //                         int number_of_PQs_on_currently_deleted_keys = -1,
    //                         int number_of_PQs_on_currently_non_inserted_keys = -1,
    //                         string kDBPath = "/tmp/cs561_project1");

    // void runPQonCurrentlyDeletedKeys(
    //   uint i_insertion, DB** db_ptr2, Options& op, WriteOptions& write_op, 
    //   ReadOptions& read_op, EmuEnv* _env, int number_of_PQs, string kDBPath);
      
    void runPQonCurrentlyDeletedKeys(
      uint i_insertion, DB** db_ptr2, Options& op, WriteOptions& write_op, 
      ReadOptions& read_op, EmuEnv* _env,
      // int number_of_PQs_on_existing_keys,
      // int number_of_PQs_on_historic_existing_keys,
      int number_of_PQs_on_currently_deleted_keys,
      // int number_of_PQs_on_currently_non_inserted_keys,
      string kDBPath);
      
    void end();
};
LoggerDuringInsertion* LoggerDuringInsertion::logger_during_insertion = NULL;


LoggerDuringInsertion* LoggerDuringInsertion::getInstance(EmuEnv* _env){
    if(logger_during_insertion == NULL){
        logger_during_insertion = new LoggerDuringInsertion();
        logger_during_insertion->start(_env);
    }
    return logger_during_insertion;
}

void LoggerDuringInsertion::start(EmuEnv* _env){
  
    std::string running_log_name_during_insertion = "output_statistics/" + _env->logging_filename + "_during_insertion.log";
    running_log_during_insertion.open(running_log_name_during_insertion);

    std::string testing_result_file_name_during_insertion = "output_statistics/" + _env->logging_filename + "_during_insertion.json";
    testing_result_file_during_insertion.open(testing_result_file_name_during_insertion);
    testing_result_file_during_insertion << "{"<< std::endl;
    testing_result_file_during_insertion << "\"start\" : \"start\""<< std::endl;
    
    surf::SuRF_Env *_surf_env = surf::SuRF_Env::getInstance();
    write_log2(testing_result_file_during_insertion, _env, _surf_env);
}


void LoggerDuringInsertion::recordCurrentMemoryFootprint(DB** db_ptr2){
  DB* db = *db_ptr2;
  // running_log_during_insertion << "recordCurrentMemoryFootprint Start " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  vector<int> tmp;
  tmp = db->getLogOfNumbersOfRangesInOrigin();
  if(tmp.size() > 0){
    ranges_log_Origin.push_back(tmp.back());
  }else{
    ranges_log_Origin.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint A0B0 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  tmp = db->getLogOfMemoryUsageInOrigin();
  if(tmp.size() > 0){
    memory_usage_log_Origin.push_back(tmp.back());
  }else{
    memory_usage_log_Origin.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint A0B1 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  tmp = db->getLogOfNumbersOfRangesInPLRDF();
  if(tmp.size() > 0){
    ranges_log_PLRDF.push_back(tmp.back());
  }else{
    ranges_log_PLRDF.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint A1 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  tmp = db->getLogOfNumbersOfRangesInSplitPLRDF();
  if(tmp.size() > 0){
    ranges_log_SplitPLRDF.push_back(tmp.back());
  }else{
    ranges_log_SplitPLRDF.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint A2 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  tmp = db->getLogOfNumbersOfRangesInPLRDFStringKey();
  if(tmp.size() > 0){
    ranges_log_PLRDFStringKey.push_back(tmp.back());
  }else{
    ranges_log_PLRDFStringKey.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint XA1 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  tmp = db->getLogOfNumbersOfRangesInSplitPLRDFStringKey();
  if(tmp.size() > 0){
    ranges_log_SplitPLRDFStringKey.push_back(tmp.back());
  }else{
    ranges_log_SplitPLRDFStringKey.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint XA2 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  tmp = db->getLogOfNumbersOfRangesInTopLevelRDF();
  if(tmp.size() > 0){
    ranges_log_TopLevelRDF.push_back(tmp.back());
  }else{
    ranges_log_TopLevelRDF.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint A3 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  tmp = db->getLogOfNumbersOfRangesInTopLevelRDFStringKey();
  if(tmp.size() > 0){
    ranges_log_TopLevelRDFStringKey.push_back(tmp.back());
  }else{
    ranges_log_TopLevelRDFStringKey.push_back(0);
  }

  tmp = db->getLogOfNumbersOfRangesInSkylineRDF();
  if(tmp.size() > 0){
    ranges_log_SkylineRDF.push_back(tmp.back());
  }else{
    ranges_log_SkylineRDF.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint A4 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  tmp = db->getLogOfNumbersOfRangesInSuRFLevelFileRDF();
  if(tmp.size() > 0){
    ranges_log_SuRFLevelFileRDF.push_back(tmp.back());
  }else{
    ranges_log_SuRFLevelFileRDF.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint A5 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  tmp = db->getLogOfNumbersOfRangesInSuRFLevelFileSplitRDF();
  if(tmp.size() > 0){
    ranges_log_SuRFLevelFileSplitRDF.push_back(tmp.back());
  }else{
    ranges_log_SuRFLevelFileSplitRDF.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint A6 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;


  tmp = db->getLogOfMemoryUsageInPLRDF();
  if(tmp.size() > 0){
    memory_usage_log_PLRDF.push_back(tmp.back());
  }else{
    memory_usage_log_PLRDF.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint A7 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  tmp = db->getLogOfMemoryUsageInSplitRDF();
  if(tmp.size() > 0){
    memory_usage_log_SplitPLRDF.push_back(tmp.back());
  }else{
    memory_usage_log_SplitPLRDF.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint A8 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  tmp = db->getLogOfMemoryUsageInPLRDFStringKey();
  if(tmp.size() > 0){
    memory_usage_log_PLRDFStringKey.push_back(tmp.back());
  }else{
    memory_usage_log_PLRDFStringKey.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint A7 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  tmp = db->getLogOfMemoryUsageInSplitRDFStringKey();
  if(tmp.size() > 0){
    memory_usage_log_SplitPLRDFStringKey.push_back(tmp.back());
  }else{
    memory_usage_log_SplitPLRDFStringKey.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint A8 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  tmp = db->getLogOfMemoryUsageInTopLevelRDF();
  if(tmp.size() > 0){
    memory_usage_log_TopLevelRDF.push_back(tmp.back());
  }else{
    memory_usage_log_TopLevelRDF.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint A9 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  tmp = db->getLogOfMemoryUsageInTopLevelRDFStringKey();

  if(tmp.size() > 0){
    memory_usage_log_TopLevelRDFStringKey.push_back(tmp.back());
  }else{
    memory_usage_log_TopLevelRDFStringKey.push_back(0);
  }

  tmp = db->getLogOfMemoryUsageInSkylineRDF();
  if(tmp.size() > 0){
    memory_usage_log_SkylineRDF.push_back(tmp.back());
  }else{
    memory_usage_log_SkylineRDF.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint A10 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  tmp = db->getLogOfMemoryUsageInSuRFLevelFileRDF();
  if(tmp.size() > 0){
    memory_usage_log_SuRFLevelFileRDF.push_back(tmp.back());
  }else{
    memory_usage_log_SuRFLevelFileRDF.push_back(0);
  }
  // running_log_during_insertion << "recordCurrentMemoryFootprint A11 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  tmp = db->getLogOfMemoryUsageInSuRFLevelFileSplitRDF();
  if(tmp.size() > 0){
    memory_usage_log_SuRFLevelFileSplitRDF.push_back(tmp.back());
  }else{
    memory_usage_log_SuRFLevelFileSplitRDF.push_back(0);
  }

  // running_log_during_insertion << "recordCurrentMemoryFootprint End " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
}


void LoggerDuringInsertion::writeRecord(DB** db_ptr2){
  DB* db = *db_ptr2;
  Status s;

  running_log_during_insertion << "writeRecord Start " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
  
  testing_result_file_during_insertion << ",\"PLRDF Number Of Total Ranges\" : " << db->getPLRDFNumberOfTotalRanges() << std::endl;
  testing_result_file_during_insertion << ",\"Split PLRDF Number Of Total Ranges\" : " << db->getSplitPLRDFNumberOfTotalRanges() << std::endl;
  testing_result_file_during_insertion << ",\"PLRDF StringKey Number Of Total Ranges\" : " << db->getPLRDFStringKeyNumberOfTotalRanges() << std::endl;
  testing_result_file_during_insertion << ",\"Split PLRDF StringKey Number Of Total Ranges\" : " << db->getSplitPLRDFStringKeyNumberOfTotalRanges() << std::endl;
  testing_result_file_during_insertion << ",\"TopLevel RDF Number Of Total Ranges\" : " << db->getTopLevelRDFNumberOfTotalRanges() << std::endl;
  testing_result_file_during_insertion << ",\"TopLevel RDF StringKey Number Of Total Ranges\" : " << db->getTopLevelRDFStringKeyNumberOfTotalRanges() << std::endl;
  testing_result_file_during_insertion << ",\"Skyline RDF Number Of Total Ranges\" : " << db->getSkylineRDFNumberOfTotalRanges() << std::endl;
  testing_result_file_during_insertion << ",\"SuRF Level File RDF Number Of Total Ranges\" : " << db->getSuRFLevelFileRDFNumberOfTotalRanges() << std::endl;
  testing_result_file_during_insertion << ",\"SuRF Level File Split RDF Number Of Total Ranges\" : " << db->getSuRFLevelFileSplitRDFNumberOfTotalRanges() << std::endl;


  testing_result_file_during_insertion << ",\"RocksDB Number Of Total Memory Usage\" : " << memory_usage_log_Origin[memory_usage_log_Origin.size()-1] << " bytes" << std::endl;
  testing_result_file_during_insertion << ",\"PLRDF Number Of Total Memory Usage\" : " << memory_usage_log_PLRDF[memory_usage_log_PLRDF.size()-1] << std::endl;
  testing_result_file_during_insertion << ",\"Split PLRDF Number Of Total Memory Usage\" : " << memory_usage_log_SplitPLRDF[memory_usage_log_SplitPLRDF.size()-1] << " bytes" << std::endl;
  testing_result_file_during_insertion << ",\"TopLevel RDF Number Of Total Memory Usage\" : " << memory_usage_log_TopLevelRDF[memory_usage_log_TopLevelRDF.size()-1] << " bytes" << std::endl;
  testing_result_file_during_insertion << ",\"TopLevel RDF StringKey Number Of Total Memory Usage\" : " << memory_usage_log_TopLevelRDFStringKey[memory_usage_log_TopLevelRDFStringKey.size()-1] << " bytes" << std::endl;
  testing_result_file_during_insertion << ",\"Skyline RDF Number Of Total Memory Usage\" : " << memory_usage_log_SkylineRDF[memory_usage_log_SkylineRDF.size()-1] << " bytes" << std::endl;
  testing_result_file_during_insertion << ",\"SuRF Level File RDF Number Of Total Memory Usage\" : " << memory_usage_log_SuRFLevelFileRDF[memory_usage_log_SuRFLevelFileRDF.size()-1] << " bytes" << std::endl;
  testing_result_file_during_insertion << ",\"SuRF Level File Split RDF Number Of Total Memory Usage\" : " << memory_usage_log_SuRFLevelFileSplitRDF[memory_usage_log_SuRFLevelFileSplitRDF.size()-1] << " bytes" << std::endl;


  /*Numbers of Range Tombstones*/
  testing_result_file_during_insertion << ",\"Log Of Numbers Of Ranges In Origin\" : [";
  for(int i = 0; i < ranges_log_Origin.size(); i++){
    testing_result_file_during_insertion << ranges_log_Origin[i];
    if(i != ranges_log_Origin.size() - 1){
      testing_result_file_during_insertion << ", ";
    }
  }
  testing_result_file_during_insertion << "]" << std::endl;
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
  testing_result_file_during_insertion << ",\"Log Of Numbers Of Ranges In TopLevelRDFStringKey\" : [";
  for(int i = 0; i < ranges_log_TopLevelRDFStringKey.size(); i++){
    testing_result_file_during_insertion << ranges_log_TopLevelRDFStringKey[i];
    if(i != ranges_log_TopLevelRDFStringKey.size() - 1){
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
 testing_result_file_during_insertion << ",\"Log Of Memory Usage Of Origin\" : [";
  for(int i = 0; i < memory_usage_log_Origin.size(); i++){
    testing_result_file_during_insertion << memory_usage_log_Origin[i];
    if(i != memory_usage_log_Origin.size() - 1){
      testing_result_file_during_insertion << ", ";
    }
  }
  testing_result_file_during_insertion << "]" << std::endl;
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
  testing_result_file_during_insertion << "]" << std::endl;
  testing_result_file_during_insertion << ",\"Log Of Memory Usage Of TopLevelRDFStringKey\" : [";
  for(int i = 0; i < memory_usage_log_TopLevelRDFStringKey.size(); i++){
    testing_result_file_during_insertion << memory_usage_log_TopLevelRDFStringKey[i];
    if(i != memory_usage_log_TopLevelRDFStringKey.size() - 1){
      testing_result_file_during_insertion << ", ";
    }
  }
  testing_result_file_during_insertion << "]" << std::endl;
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

void LoggerDuringInsertion::runPQonCurrentlyDeletedKeys(
                                                        uint i_insertion,
                                                        DB** db_ptr2, 
                                                        Options& op, 
                                                        WriteOptions& write_op, 
                                                        ReadOptions& read_op, 
                                                        EmuEnv* _env,
                                                        // int number_of_PQs,
                                                        // int number_of_PQs_on_existing_keys,
                                                        // int number_of_PQs_on_historic_existing_keys,
                                                        int number_of_PQs_on_currently_deleted_keys,
                                                        // int number_of_PQs_on_currently_non_inserted_keys,
                                                        string kDBPath){
                                                          
  DB* db = *db_ptr2;
  Status s;
  
  checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();
  int KEY_SIZE = checking::SystemVerifier::getSystemVerifier()->getKeySize();


  running_log_during_insertion << "!!! Testing On Currently Deleted Keys " << std::endl;
  
  const long long N_repetitions = checking::SystemVerifier::EXPERIMENT_REPETITION_TIMES;
  long long disk_access_count = 0;

  
  // std::string prefix_number_of_PQs = "";
  // if(number_of_PQs != -1){
  //   prefix_number_of_PQs = "fixed #PQ = " + std::to_string(number_of_PQs);
  // }
  
  // std::string prefix_number_of_PQs_on_existing_keys = "";
  // if(number_of_PQs_on_existing_keys != -1){
  //   prefix_number_of_PQs_on_existing_keys = "fixed #PQ = " + std::to_string(number_of_PQs_on_existing_keys);
  // }
  
  // std::string prefix_number_of_PQs_on_historic_existing_keys = "";
  // if(number_of_PQs_on_historic_existing_keys != -1){
  //   prefix_number_of_PQs_on_historic_existing_keys = "fixed #PQ = " + std::to_string(number_of_PQs_on_historic_existing_keys);
  // }
  
  std::string prefix_number_of_PQs_on_currently_deleted_keys = "";
  if(number_of_PQs_on_currently_deleted_keys != -1){
    prefix_number_of_PQs_on_currently_deleted_keys = "fixed #PQ = " + std::to_string(number_of_PQs_on_currently_deleted_keys);
  }
  
  // std::string prefix_number_of_PQs_on_currently_non_inserted_keys = "";
  // if(number_of_PQs_on_currently_non_inserted_keys != -1){
  //   prefix_number_of_PQs_on_currently_non_inserted_keys = "fixed #PQ = " + std::to_string(number_of_PQs_on_currently_non_inserted_keys);
  // }

  // system_verifier->gen_workload_with_numbers_of_PQ(N_repetitions, number_of_PQs);
  int number_of_PQs_on_existing_keys_tmp = 10, number_of_PQs_on_historic_existing_keys_tmp = 10, number_of_PQs_on_currently_non_inserted_keys_tmp = 10;
  system_verifier->gen_workload_with_numbers_of_PQ(N_repetitions, 
    number_of_PQs_on_existing_keys_tmp, number_of_PQs_on_historic_existing_keys_tmp,
    number_of_PQs_on_currently_deleted_keys, number_of_PQs_on_currently_non_inserted_keys_tmp);

  string i_insertion_str = "i_insert="+std::to_string(i_insertion)+" ";
  testing_result_file_during_insertion << system_verifier->getCurrentlyDeletedKeysVec2dString(",", "\"", i_insertion_str+prefix_number_of_PQs_on_currently_deleted_keys) << std::endl;

  auto start_pq = std::chrono::high_resolution_clock::now();
  auto stop_pq = std::chrono::high_resolution_clock::now();
  auto duration_pq = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_pq - start_pq);
  unsigned long long point_query_time = duration_pq.count();

  // testing_result_file << "----------------------Testing On historic-existing Keys-----------------------" << std::endl;
  for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
    long long total_read_count = 0;
    long long total_read_bytes = 0;
    testing_logger.reset();

    system_verifier->setRDFTypeChosed(t);
    system_verifier->resetAllCount();
    system_verifier->resetAllDuration();
    
    if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
      s = db->SetOptions({{"max_open_files", "1"}}); // is there any compaction happended after this????
    }else{
      s = db->SetOptions({{"max_open_files", std::to_string(_env->max_open_files)}}); // is there any compaction happended after this????
    }

    disk_access_count = 0;
    point_query_time = 0;
    start_pq = std::chrono::high_resolution_clock::now();
    rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
    rocksdb::get_perf_context()->Reset();
    rocksdb::get_iostats_context()->Reset();
    for(auto i = 0; i < N_repetitions; i++){
      clearCache(op);
      db = *db_ptr2;

      system_verifier->resetDiskAccessCount();
      testing_logger.set_to_start(op);

      std::vector<uint64_t> cache_tombstone_bytes;
      for(auto x: system_verifier->getCurrentlyDeletedKeysAtNRound(i)){
        bool gt_is_exist = system_verifier->isKeyExist(x);
        std::string gt_value = system_verifier->get(x);

        std::string value;
        std::string time_stamp;
        std::stringstream searching_key;
        searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;

        std::cout << "issue get !! " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
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
        if(system_verifier->getStringOfRDFTypeChosed() == "NONE_CACHE_RANGETOMBSTONE_TRACING"){
          uint64_t bytes = checking::CacheTombstoneTracer::getInstance()->getTotalTombstoneBytes();
          cache_tombstone_bytes.push_back(bytes);
        }
        // size_t separator_pos = value.find("|");
        // time_stamp = value.substr(separator_pos + 1);
        // value = value.substr(0, separator_pos);

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
      if(system_verifier->getStringOfRDFTypeChosed() == "NONE_CACHE_RANGETOMBSTONE_TRACING"){
        string i_round_str = "i_round="+std::to_string(i)+" ";
        std::string prefix = " (Currently Deleted Keys " + i_insertion_str + prefix_number_of_PQs_on_currently_deleted_keys + " " + i_round_str + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
        testing_result_file_during_insertion << ",\"" + prefix + " cache tombstone bytes\" : " << "[";
        int len_ctb = cache_tombstone_bytes.size();
        int i_ctb = 0;
        if(i_ctb < len_ctb){
          testing_result_file_during_insertion << cache_tombstone_bytes[i_ctb];
          i_ctb++;
        }
        for(;i_ctb < len_ctb; i_ctb++){
          auto &x = cache_tombstone_bytes[i_ctb];
          testing_result_file_during_insertion << ", " << x;
        }
        testing_result_file_during_insertion << "]" << std::endl;
      }
      disk_access_count += system_verifier->getDiskAccessCount();

      running_log_during_insertion << " Disk Access count = " << system_verifier->getDiskAccessCount() << std::endl;
      testing_logger.set_to_end(op, running_log_during_insertion);
    }  
    if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
      continue;
    }
    double block_read_cpu_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), ".*block_read_cpu_time = ([0-9.]+)");

    running_log_during_insertion << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*disk_access_count/N_repetitions << std::endl;
    running_log_during_insertion << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions/1e6 << " (ms) " << std::endl << std::endl;
    running_log_during_insertion << "filtered by RDF count = " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl; 
    running_log_during_insertion << "number of PQ = " << system_verifier->getHistoricExistingKeys().size() << std::endl;
    running_log_during_insertion << system_verifier->getAllCount("", "", "", N_repetitions) << std::endl;
    running_log_during_insertion << "block_read_cpu_time = " << 1.0*block_read_cpu_time/N_repetitions/1e3  << "" << std::endl;

    std::string prefix = " (Currently Deleted Keys " + i_insertion_str + prefix_number_of_PQs_on_currently_deleted_keys + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
    testing_result_file_during_insertion << ",\"" + prefix + " elapsed time\" : " << 1.0*point_query_time/N_repetitions/1e6 << std::endl;
    testing_result_file_during_insertion << ",\"" + prefix + " filtered by RDF count\" : " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl;
    if (number_of_PQs_on_currently_deleted_keys == -1){
      testing_result_file_during_insertion << ",\"" + prefix + " number of PQ\" : " << system_verifier->getCurrentlyDeletedKeys().size() << std::endl;
    }else{
      testing_result_file_during_insertion << ",\"" + prefix + " number of PQ\" : " << prefix_number_of_PQs_on_currently_deleted_keys << std::endl;
    }
    testing_result_file_during_insertion << system_verifier->getAllCount(",", "\"", prefix, N_repetitions) << std::endl;
    testing_result_file_during_insertion << ",\"" + prefix + " block_read_cpu_time\" : " << 1.0*block_read_cpu_time/N_repetitions/1e3 << std::endl;
    
    testing_logger.output_statistics(running_log_during_insertion, testing_result_file_during_insertion, prefix);
  }
  system_verifier->reset_flag_testing_on_currently_deleted_keys();

  running_log_during_insertion << "!!! Testing On Currently Deleted Keys " << std::endl;
  system_verifier->set_flag_testing_on_currently_deleted_keys();
  system_verifier->startPQTracing();

  running_log_during_insertion << std::endl << std::endl;
}                     

void LoggerDuringInsertion::end(){
  running_log_during_insertion << std::endl << std::endl;
  running_log_during_insertion << "----------------------End Testing-----------------------" << std::endl;
  running_log_during_insertion.close();
  
  testing_result_file_during_insertion << ",\"End\" : \"End\""<< std::endl;
  testing_result_file_during_insertion << "}"<< std::endl;
  testing_result_file_during_insertion.close();
}

#endif // UTILS_LOGGER_DURING_INSERTION_H