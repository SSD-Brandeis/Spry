#ifndef UTILS_RUN_VERIFICATION_H
#define UTILS_RUN_VERIFICATION_H

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


class verification_runner{
private:
    static std::ofstream testing_result_file, testing_result_file2;
    static TestingLogger testing_logger;

    static bool flag_reopen_db_for_each_RDF_testing;

    static PLRDF plrdf_prime, split_plrdf_prime;
    static PLRDF_t<std::string> plrdf_stringkey_prime, split_plrdf_stringkey_prime;
    static PLRDF top_level_rdf_prime;
    static SkyLineRDF skyline_rdf_prime;
    // static std::vector<t3ll> skyline_rdf_prime;
    // static std::vector<int> skyline__numbers_of_ranges_in_rdf_log;

public:
    static void initPQVerification(DB** db_ptr2, ReadOptions& read_op, EmuEnv* _env);
    // static void runPQVerification(DB** db_ptr2, Options& op, WriteOptions& write_op, 
    //                         ReadOptions& read_op, EmuEnv* _env, int number_of_PQs = -1,
    //                         string kDBPath = "/tmp/cs561_project1");
    static void runPQVerification(DB** db_ptr2, Options& op, WriteOptions& write_op, 
                            ReadOptions& read_op, EmuEnv* _env, 
                            // int number_of_PQs = -1,
                            int number_of_PQs_on_existing_keys = -1,
                            int number_of_PQs_on_historic_existing_keys = -1,
                            int number_of_PQs_on_currently_deleted_keys = -1,
                            int number_of_PQs_on_currently_non_inserted_keys = -1,
                            string kDBPath = "/tmp/cs561_project1");
                            
    static void endPQVerification();
};
std::ofstream verification_runner::testing_result_file;
std::ofstream verification_runner::testing_result_file2;

TestingLogger verification_runner::testing_logger;

// bool verification_runner::flag_reopen_db_for_each_RDF_testing = true;
bool verification_runner::flag_reopen_db_for_each_RDF_testing = false;

PLRDF verification_runner::plrdf_prime, verification_runner::split_plrdf_prime;
PLRDF_t<std::string> verification_runner::plrdf_stringkey_prime, verification_runner::split_plrdf_stringkey_prime;
PLRDF verification_runner::top_level_rdf_prime;
SkyLineRDF verification_runner::skyline_rdf_prime;
// std::vector<t3ll> verification_runner::skyline_rdf_prime;
// std::vector<int> verification_runner::skyline__numbers_of_ranges_in_rdf_log;



void verification_runner::initPQVerification(DB** db_ptr2, ReadOptions& read_op, EmuEnv* _env){
  DB* db = *db_ptr2;
  Status s;
  
  surf::SuRF_Env *_surf_env = surf::SuRF_Env::getInstance();


  checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();
  int KEY_SIZE = checking::SystemVerifier::getSystemVerifier()->getKeySize();
  // system_verifier->setRDFTypes(_env->RDFTypes);

  system_verifier->setRunningPQ();

  system_verifier->enable_log__deleted_keys__max_sequnce_number();
  system_verifier->setRDFTypeChosed(0); // 0: NONE
  
  for(auto x: system_verifier->getCurrentlyDeletedKeys()){
    std::string value;
    std::stringstream searching_key;
    searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;

    s = db->Get(read_op, searching_key.str(), &value);
  }
  system_verifier->disable_log__deleted_keys__max_sequnce_number();




  std::string testing_result_file_name = _env->logging_filename + ".testing_log";
  std::string testing_result_file_name2 = "output_statistics/" + _env->logging_filename + ".json";
std::cout << "testing_result_file_name2 =  " << testing_result_file_name2 << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
  testing_result_file.open(testing_result_file_name);
  testing_result_file2.open(testing_result_file_name2);
  testing_result_file2 << "{"<< std::endl;
  testing_result_file2 << "\"start\" : \"start\""<< std::endl;
  write_log2(testing_result_file2, _env, _surf_env);
  
  testing_result_file << "PLRDF Number Of Total Ranges: " << db->getPLRDFNumberOfTotalRanges() << std::endl;
  testing_result_file << "Split PLRDF Number Of Total Ranges: " << db->getSplitPLRDFNumberOfTotalRanges() << std::endl;
  testing_result_file << "PLRDF StringKey Number Of Total Ranges: " << db->getPLRDFStringKeyNumberOfTotalRanges() << std::endl;
  testing_result_file << "Split PLRDF StringKey Number Of Total Ranges: " << db->getSplitPLRDFStringKeyNumberOfTotalRanges() << std::endl;
  testing_result_file << "TopLevel RDF Number Of Total Ranges: " << db->getTopLevelRDFNumberOfTotalRanges() << std::endl;
  testing_result_file << "Skyline RDF Number Of Total Ranges: " << db->getSkylineRDFNumberOfTotalRanges() << std::endl;
  testing_result_file << "SuRF Level File RDF Number Of Total Ranges: " << db->getSuRFLevelFileRDFNumberOfTotalRanges() << std::endl;
  testing_result_file << "SuRF Level File Split RDF Number Of Total Ranges: " << db->getSuRFLevelFileSplitRDFNumberOfTotalRanges() << std::endl;
  testing_result_file << std::endl;
  
  testing_result_file2 << ",\"PLRDF Number Of Total Ranges\" : " << db->getPLRDFNumberOfTotalRanges() << std::endl;
  testing_result_file2 << ",\"Split PLRDF Number Of Total Ranges\" : " << db->getSplitPLRDFNumberOfTotalRanges() << std::endl;
  testing_result_file2 << ",\"PLRDF StringKey Number Of Total Ranges\" : " << db->getPLRDFStringKeyNumberOfTotalRanges() << std::endl;
  testing_result_file2 << ",\"Split PLRDF StringKey Number Of Total Ranges\" : " << db->getSplitPLRDFStringKeyNumberOfTotalRanges() << std::endl;
  testing_result_file2 << ",\"TopLevel RDF Number Of Total Ranges\" : " << db->getTopLevelRDFNumberOfTotalRanges() << std::endl;
  testing_result_file2 << ",\"Skyline RDF Number Of Total Ranges\" : " << db->getSkylineRDFNumberOfTotalRanges() << std::endl;
  testing_result_file2 << ",\"SuRF Level File RDF Number Of Total Ranges\" : " << db->getSuRFLevelFileRDFNumberOfTotalRanges() << std::endl;
  testing_result_file2 << ",\"SuRF Level File Split RDF Number Of Total Ranges\" : " << db->getSuRFLevelFileSplitRDFNumberOfTotalRanges() << std::endl;
  testing_result_file2 << std::endl;

  
  testing_result_file << "RocksDB (None) Number Of Total Memory Usage: " << db->getRTRocksDBNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file << "PLRDF Number Of Total Memory Usage: " << db->getPLRDFNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file << "Split PLRDF Number Of Total Memory Usage: " << db->getSplitPLRDFNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file << "PLRDF StringKey Number Of Total Memory Usage: " << db->getPLRDFStringKeyNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file << "Split PLRDF StringKey Number Of Total Memory Usage: " << db->getSplitPLRDFStringKeyNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file << "TopLevel RDF Number Of Total Memory Usage: " << db->getTopLevelRDFNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file << "Skyline RDF Number Of Total Memory Usage: " << db->getSkylineRDFNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file << "SuRF Level File RDF Number Of Total Memory Usage: " << db->getSuRFLevelFileRDFNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file << "SuRF Level File Split RDF Number Of Total Memory Usage: " << db->getSuRFLevelFileSplitRDFNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file << std::endl;

  testing_result_file2 << ",\"RocksDB (None) Number Of Total Memory Usage\" : " << db->getRTRocksDBNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file2 << ",\"PLRDF Number Of Total Memory Usage\" : " << db->getPLRDFNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file2 << ",\"Split PLRDF Number Of Total Memory Usage\" : " << db->getSplitPLRDFNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file2 << ",\"PLRDF StringKey Number Of Total Memory Usage\" : " << db->getPLRDFStringKeyNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file2 << ",\"Split PLRDF StringKey Number Of Total Memory Usage\" : " << db->getSplitPLRDFStringKeyNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file2 << ",\"TopLevel RDF Number Of Total Memory Usage\" : " << db->getTopLevelRDFNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file2 << ",\"Skyline RDF Number Of Total Memory Usage\" : " << db->getSkylineRDFNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file2 << ",\"SuRF Level File RDF Number Of Total Memory Usage\" : " << db->getSuRFLevelFileRDFNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file2 << ",\"SuRF Level File Split RDF Number Of Total Memory Usage\" : " << db->getSuRFLevelFileSplitRDFNumberOfTotalMemoryUsage() << std::endl;
  testing_result_file2 << std::endl;

  if(system_verifier->getStringOfRDFTypeChosed().substr(0,4) == "NONE"){
    std::cout << "RocksDB (None) Number Of Total Memory Usage _out = " << db->getRTRocksDBNumberOfTotalMemoryUsage() << std::endl;
  }else if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF"){
    std::cout << "PLRDF Number Of Total Memory Usage _out = " << db->getPLRDFNumberOfTotalMemoryUsage() << std::endl;
  }else if(system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF"){
    std::cout << "Split_PLRDF Number Of Total Memory Usage _out = " << db->getSplitPLRDFNumberOfTotalMemoryUsage() << std::endl;
  }else if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY"){
    std::cout << "PLRDF_STRING_KEY Number Of Total Memory Usage _out = " << db->getPLRDFStringKeyNumberOfTotalMemoryUsage() << std::endl;
  }else if(system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"){
    std::cout << "SPLIT_PLRDF_STRING_KEY Number Of Total Memory Usage _out = " << db->getSplitPLRDFStringKeyNumberOfTotalMemoryUsage() << std::endl;
  }else if(system_verifier->getStringOfRDFTypeChosed() == "TOP_LEVEL_RDF"){
    std::cout << "TOP_LEVEL_RDF Number Of Total Memory Usage _out = " << db->getTopLevelRDFNumberOfTotalMemoryUsage() << std::endl;
  }else if(system_verifier->getStringOfRDFTypeChosed() == "SKYLINE_RDF"){
    std::cout << "SKYLINE_RDF Number Of Total Memory Usage _out = " << db->getSkylineRDFNumberOfTotalMemoryUsage() << std::endl;
  }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF"){
    std::cout << "SuRF_LF_RDF Number Of Total Memory Usage _out = " << db->getSuRFLevelFileRDFNumberOfTotalMemoryUsage() << std::endl;
  }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF"){
    std::cout << "SuRF_LF_SPLIT_RDF Number Of Total Memory Usage _out = " << db->getSuRFLevelFileSplitRDFNumberOfTotalMemoryUsage() << std::endl;
  }

  vector<int> ranges_log_PLRDF = db->getLogOfNumbersOfRangesInPLRDF();
  vector<int> ranges_log_SplitPLRDF = db->getLogOfNumbersOfRangesInSplitPLRDF();
  vector<int> ranges_log_PLRDFStringKey = db->getLogOfNumbersOfRangesInPLRDFStringKey();
  vector<int> ranges_log_SplitPLRDFStringKey = db->getLogOfNumbersOfRangesInSplitPLRDFStringKey();
  vector<int> ranges_log_TopLevelRDF = db->getLogOfNumbersOfRangesInTopLevelRDF();
  vector<int> ranges_log_SkylineRDF = db->getLogOfNumbersOfRangesInSkylineRDF();
  vector<int> ranges_log_SuRFLevelFileRDF = db->getLogOfNumbersOfRangesInSuRFLevelFileRDF();
  vector<int> ranges_log_SuRFLevelFileSplitRDF = db->getLogOfNumbersOfRangesInSuRFLevelFileSplitRDF();
  
  testing_result_file2 << ",\"Log Of Numbers Of Ranges In PLRDF\" : [";
  for(int i = 0; i < ranges_log_PLRDF.size(); i++){
    testing_result_file2 << ranges_log_PLRDF[i];
    if(i != ranges_log_PLRDF.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;
  testing_result_file2 << ",\"Log Of Numbers Of Ranges In SplitPLRDF\" : [";
  for(int i = 0; i < ranges_log_SplitPLRDF.size(); i++){
    testing_result_file2 << ranges_log_SplitPLRDF[i];
    if(i != ranges_log_SplitPLRDF.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;
  testing_result_file2 << ",\"Log Of Numbers Of Ranges In PLRDFStringKey\" : [";
  for(int i = 0; i < ranges_log_PLRDFStringKey.size(); i++){
    testing_result_file2 << ranges_log_PLRDFStringKey[i];
    if(i != ranges_log_PLRDFStringKey.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;
  testing_result_file2 << ",\"Log Of Numbers Of Ranges In SplitPLRDFStringKey\" : [";
  for(int i = 0; i < ranges_log_SplitPLRDFStringKey.size(); i++){
    testing_result_file2 << ranges_log_SplitPLRDFStringKey[i];
    if(i != ranges_log_SplitPLRDFStringKey.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;
  testing_result_file2 << ",\"Log Of Numbers Of Ranges In TopLevelRDF\" : [";
  for(int i = 0; i < ranges_log_TopLevelRDF.size(); i++){
    testing_result_file2 << ranges_log_TopLevelRDF[i];
    if(i != ranges_log_TopLevelRDF.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;;
  testing_result_file2 << ",\"Log Of Numbers Of Ranges In SkylineRDF\" : [";
  for(int i = 0; i < ranges_log_SkylineRDF.size(); i++){
    testing_result_file2 << ranges_log_SkylineRDF[i];
    if(i != ranges_log_SkylineRDF.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;
  testing_result_file2 << ",\"Log Of Numbers Of Ranges In SuRFLevelFileRDF\" : [";
  for(int i = 0; i < ranges_log_SuRFLevelFileRDF.size(); i++){
    testing_result_file2 << ranges_log_SuRFLevelFileRDF[i];
    if(i != ranges_log_SuRFLevelFileRDF.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;
  testing_result_file2 << ",\"Log Of Numbers Of Ranges In SuRFLevelFileSplitRDF\" : [";
  for(int i = 0; i < ranges_log_SuRFLevelFileSplitRDF.size(); i++){
    testing_result_file2 << ranges_log_SuRFLevelFileSplitRDF[i];
    if(i != ranges_log_SuRFLevelFileSplitRDF.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;

  vector<int> memory_usage_log_PLRDF = db->getLogOfMemoryUsageInPLRDF();
  vector<int> memory_usage_log_SplitPLRDF = db->getLogOfMemoryUsageInSplitRDF();
  vector<int> memory_usage_log_PLRDFStringKey = db->getLogOfMemoryUsageInPLRDFStringKey();
  vector<int> memory_usage_log_SplitPLRDFStringKey = db->getLogOfMemoryUsageInSplitRDFStringKey();
  vector<int> memory_usage_log_TopLevelRDF = db->getLogOfMemoryUsageInTopLevelRDF();
  vector<int> memory_usage_log_SkylineRDF = db->getLogOfMemoryUsageInSkylineRDF();
  vector<int> memory_usage_log_SuRFLevelFileRDF = db->getLogOfMemoryUsageInSuRFLevelFileRDF();
  vector<int> memory_usage_log_SuRFLevelFileSplitRDF = db->getLogOfMemoryUsageInSuRFLevelFileSplitRDF();
  testing_result_file2 << ",\"Log Of Memory Usage Of PLRDF\" : [";
  for(int i = 0; i < memory_usage_log_PLRDF.size(); i++){
    testing_result_file2 << memory_usage_log_PLRDF[i];
    if(i != memory_usage_log_PLRDF.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;
  testing_result_file2 << ",\"Log Of Memory Usage Of SplitPLRDF\" : [";
  for(int i = 0; i < memory_usage_log_SplitPLRDF.size(); i++){
    testing_result_file2 << memory_usage_log_SplitPLRDF[i];
    if(i != memory_usage_log_SplitPLRDF.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;
  testing_result_file2 << ",\"Log Of Memory Usage Of PLRDFStringKey\" : [";
  for(int i = 0; i < memory_usage_log_PLRDFStringKey.size(); i++){
    testing_result_file2 << memory_usage_log_PLRDFStringKey[i];
    if(i != memory_usage_log_PLRDFStringKey.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;
  testing_result_file2 << ",\"Log Of Memory Usage Of SplitPLRDFStringKey\" : [";
  for(int i = 0; i < memory_usage_log_SplitPLRDFStringKey.size(); i++){
    testing_result_file2 << memory_usage_log_SplitPLRDFStringKey[i];
    if(i != memory_usage_log_SplitPLRDFStringKey.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;
  testing_result_file2 << ",\"Log Of Memory Usage Of TopLevelRDF\" : [";
  for(int i = 0; i < memory_usage_log_TopLevelRDF.size(); i++){
    testing_result_file2 << memory_usage_log_TopLevelRDF[i];
    if(i != memory_usage_log_TopLevelRDF.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;;
  testing_result_file2 << ",\"Log Of Memory Usage Of SkylineRDF\" : [";
  for(int i = 0; i < memory_usage_log_SkylineRDF.size(); i++){
    testing_result_file2 << memory_usage_log_SkylineRDF[i];
    if(i != memory_usage_log_SkylineRDF.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl; 
  testing_result_file2 << ",\"Log Of Memory Usage Of SuRFLevelFileRDF\" : [";
  for(int i = 0; i < memory_usage_log_SuRFLevelFileRDF.size(); i++){
    testing_result_file2 << memory_usage_log_SuRFLevelFileRDF[i];
    if(i != memory_usage_log_SuRFLevelFileRDF.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;
  testing_result_file2 << ",\"Log Of Memory Usage Of SuRFLevelFileSplitRDF\" : [";
  for(int i = 0; i < memory_usage_log_SuRFLevelFileSplitRDF.size(); i++){
    testing_result_file2 << memory_usage_log_SuRFLevelFileSplitRDF[i];
    if(i != memory_usage_log_SuRFLevelFileSplitRDF.size() - 1){
      testing_result_file2 << ", ";
    }
  }
  testing_result_file2 << "]" << std::endl;



  const long long N_repetitions = checking::SystemVerifier::EXPERIMENT_REPETITION_TIMES;
  testing_result_file << "N_repetitions = " << N_repetitions << std::endl << std::endl;
  testing_result_file2 << ",\"N_repetitions\" : " << N_repetitions << std::endl;
  testing_result_file2 << ",\"Total number of SST Files\" : " << db->getTotalNumberOfSSTFiles() << std::endl << std::endl;


  testing_logger.reset();

}



#define DEBUG_VERIFICATION                                               
void verification_runner::runPQVerification(DB** db_ptr2, 
                                            Options& op, 
                                            WriteOptions& write_op, 
                                            ReadOptions& read_op, 
                                            EmuEnv* _env,
                                            // int number_of_PQs,
                                            int number_of_PQs_on_existing_keys,
                                            int number_of_PQs_on_historic_existing_keys,
                                            int number_of_PQs_on_currently_deleted_keys,
                                            int number_of_PQs_on_currently_non_inserted_keys,
                                            string kDBPath){
// std::cout << "number_of_PQ = " << number_of_PQs <<  std::endl;
std::cout << "number_of_PQs_on_existing_keys = " << number_of_PQs_on_existing_keys <<  std::endl;
std::cout << "number_of_PQs_on_historic_existing_keys = " << number_of_PQs_on_historic_existing_keys <<  std::endl;
std::cout << "number_of_PQs_on_currently_deleted_keys = " << number_of_PQs_on_currently_deleted_keys <<  std::endl; 
std::cout << "number_of_PQs_on_currently_non_inserted_keys = " << number_of_PQs_on_currently_non_inserted_keys <<  std::endl; 

// assert(number_of_PQs != 0);
// assert(number_of_PQs >= -1);

assert(number_of_PQs_on_existing_keys != 0);
assert(number_of_PQs_on_existing_keys >= -1);

assert(number_of_PQs_on_historic_existing_keys != 0);
assert(number_of_PQs_on_historic_existing_keys >= -1);

assert(number_of_PQs_on_currently_deleted_keys != 0);
assert(number_of_PQs_on_currently_deleted_keys >= -1);

assert(number_of_PQs_on_currently_non_inserted_keys != 0);
assert(number_of_PQs_on_currently_non_inserted_keys >= -1);


  DB* db = *db_ptr2;
  Status s;
  
  checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();
  int KEY_SIZE = checking::SystemVerifier::getSystemVerifier()->getKeySize();

  long long total_read_count = 0;
  long long total_read_bytes = 0;

  const long long N_repetitions = checking::SystemVerifier::EXPERIMENT_REPETITION_TIMES;
  long long disk_access_count = 0;

  
  // std::string prefix_number_of_PQs = "";
  // if(number_of_PQs_on_existing_keys != -1){
  //   prefix_number_of_PQs = "fixed #PQ = " + std::to_string(number_of_PQs_on_existing_keys);
  // }
  
  std::string prefix_number_of_PQs_on_existing_keys = "";
  if(number_of_PQs_on_existing_keys != -1){
    prefix_number_of_PQs_on_existing_keys = "fixed #PQ = " + std::to_string(number_of_PQs_on_existing_keys);
  }
  
  std::string prefix_number_of_PQs_on_historic_existing_keys = "";
  if(number_of_PQs_on_historic_existing_keys != -1){
    prefix_number_of_PQs_on_historic_existing_keys = "fixed #PQ = " + std::to_string(number_of_PQs_on_historic_existing_keys);
  }
  
  std::string prefix_number_of_PQs_on_currently_deleted_keys = "";
  if(number_of_PQs_on_currently_deleted_keys != -1){
    prefix_number_of_PQs_on_currently_deleted_keys = "fixed #PQ = " + std::to_string(number_of_PQs_on_currently_deleted_keys);
  }
  
  std::string prefix_number_of_PQs_on_currently_non_inserted_keys = "";
  if(number_of_PQs_on_currently_non_inserted_keys != -1){
    prefix_number_of_PQs_on_currently_non_inserted_keys = "fixed #PQ = " + std::to_string(number_of_PQs_on_currently_non_inserted_keys);
  }
  
  // prefix_number_of_PQs_on_existing_keys, prefix_number_of_PQs_on_historic_existing_keys, prefix_number_of_PQs_on_currently_deleted_keys, prefix_number_of_PQs_on_currently_non_inserted_keys
  // std::cout << "number_of_PQs_on_existing_keys = " << number_of_PQs_on_existing_keys <<  std::endl;
  // std::cout << "number_of_PQs_on_historic_existing_keys = " << number_of_PQs_on_historic_existing_keys <<  std::endl;
  // std::cout << "number_of_PQs_on_currently_deleted_keys = " << number_of_PQs_on_currently_deleted_keys <<  std::endl; 
  // std::cout << "number_of_PQs_on_currently_non_inserted_keys = " << number_of_PQs_on_currently_non_inserted_keys <<  std::endl; 
  
  // number_of_PQs_on_existing_keys, number_of_PQs_on_historic_existing_keys, number_of_PQs_on_currently_deleted_keys, number_of_PQs_on_currently_non_inserted_keys
  if(_env->load_pq_workload == false){
    // system_verifier->gen_workload_with_numbers_of_PQ(N_repetitions, number_of_PQs);
    system_verifier->gen_workload_with_numbers_of_PQ(N_repetitions, 
      number_of_PQs_on_existing_keys, number_of_PQs_on_historic_existing_keys,
      number_of_PQs_on_currently_deleted_keys, number_of_PQs_on_currently_non_inserted_keys);
  }else{
    // system_verifier->load_workload_with_numbers_of_PQ(N_repetitions, number_of_PQs, _env->workload_file_name);
    system_verifier->load_workload_with_numbers_of_PQ(N_repetitions, 
      number_of_PQs_on_existing_keys, number_of_PQs_on_historic_existing_keys, 
      number_of_PQs_on_currently_deleted_keys, number_of_PQs_on_currently_non_inserted_keys, 
      _env->workload_file_name);
  }
  // testing_result_file2 << system_verifier->getCurrentlyDeletedKeysVec2dString(",", "\"", prefix_number_of_PQs) << std::endl;
  testing_result_file2 << system_verifier->getCurrentlyDeletedKeysVec2dString(",", "\"", prefix_number_of_PQs_on_currently_deleted_keys) << std::endl;
  
  auto start_pq = std::chrono::high_resolution_clock::now();
  auto stop_pq = std::chrono::high_resolution_clock::now();
  auto duration_pq = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_pq - start_pq);
  unsigned long long point_query_time = duration_pq.count();
  unsigned long long point_query_time_on_existing_keys_ns = 0;
  unsigned long long point_query_time_on_currently_deleted_all_ns = 0;

std::cout << "!!! Testing On Existing Keys " << std::endl;

  testing_result_file << std::endl << std::endl;
  testing_result_file << "----------------------Testing On Existing Keys-----------------------" << std::endl; 
  for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
    system_verifier->setRDFTypeChosed(t);
    system_verifier->resetAllCount();
    system_verifier->resetAllDuration();

    if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY" 
       || system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"
       || system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF" 
       || (system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF")){
      // double filter_false_positive_rate = -1;
      if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY"){
        db->clearFilterFalsePositiveRateInPLRDFStringKey();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"){
        db->clearFilterFalsePositiveRateInSplitPLRDFStringKey();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF"){
        // filter_false_positive_rate = db->getFilterFalsePositiveRateInSuRFLevelFileRDF();
        db->clearFilterFalsePositiveRateInSuRFLevelFileRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF"){
        // filter_false_positive_rate = db->getFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
        db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }
    }

    if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
      s = db->SetOptions({{"max_open_files", "1"}}); // is there any compaction happended after this????
    }else{
      s = db->SetOptions({{"max_open_files", std::to_string(_env->max_open_files)}}); // is there any compaction happended after this????
    }

    // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << prefix_number_of_PQs << std::endl;
    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << prefix_number_of_PQs_on_existing_keys << std::endl;

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
    


      if(flag_reopen_db_for_each_RDF_testing == true){
        retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, plrdf_stringkey_prime, split_plrdf_stringkey_prime, top_level_rdf_prime, skyline_rdf_prime);
        reopen_DB(db_ptr2, op, write_op, read_op, _env, kDBPath);
        set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, plrdf_stringkey_prime, split_plrdf_stringkey_prime, top_level_rdf_prime, skyline_rdf_prime);
      }
      std::vector<uint64_t> cache_tombstone_bytes;
      for(auto x: system_verifier->getAllExistingKeysAtNRound(i)){
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
        point_query_time_on_existing_keys_ns += duration_pq.count();
        if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
          continue;
        }
        if(system_verifier->getStringOfRDFTypeChosed() == "NONE_CACHE_RANGETOMBSTONE_TRACING"){
          uint64_t bytes = checking::CacheTombstoneTracer::getInstance()->getTotalTombstoneBytes();
          cache_tombstone_bytes.push_back(bytes);
        }
        size_t separator_pos = value.find("|");
        time_stamp = value.substr(separator_pos + 1);
        value = value.substr(0, separator_pos);

        if(s.ok() != gt_is_exist){
          #ifdef DEBUG_VERIFICATION
            std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
          #endif  
          testing_result_file << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
        }
        if(gt_is_exist == false){continue;}
        if(value != gt_value){
          #ifdef DEBUG_VERIFICATION
              std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
          #endif
          testing_result_file << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
        }
      }
      if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
        continue;
      }
      if(system_verifier->getStringOfRDFTypeChosed() == "NONE_CACHE_RANGETOMBSTONE_TRACING"){
        string i_round_str = "i_round="+std::to_string(i)+" ";
        // std::string prefix = " (Existing Keys " + i + prefix_number_of_PQs + " " + i_round_str + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
        std::string prefix = " (Existing Keys " + i + prefix_number_of_PQs_on_existing_keys + " " + i_round_str + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
        testing_result_file << ",\"" + prefix + " cache tombstone bytes\" : " << "[";
        testing_result_file2 << ",\"" + prefix + " cache tombstone bytes\" : " << "[";
        int len_ctb = cache_tombstone_bytes.size();
        int i_ctb = 0;
        if(i_ctb < len_ctb){
          testing_result_file << cache_tombstone_bytes[i_ctb];
          testing_result_file2 << cache_tombstone_bytes[i_ctb];
          i_ctb++;
        }
        for(;i_ctb < len_ctb; i_ctb++){
          auto &x = cache_tombstone_bytes[i_ctb];
          testing_result_file << ", " << x;
          testing_result_file2 << ", " << x;
        }
        testing_result_file << "]" << std::endl;
        testing_result_file2 << "]" << std::endl;
      }
      disk_access_count += system_verifier->getDiskAccessCount();

      testing_result_file << " Disk Access count = " << system_verifier->getDiskAccessCount() << std::endl;
      testing_logger.set_to_end(op, testing_result_file);
    }      
    // std::cout << " rocksdb::get_perf_context()->bloom_sst_miss_count = " <<  rocksdb::get_perf_context()->bloom_sst_miss_count << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    // std::cout << " rocksdb::get_perf_context()->bloom_sst_hit_count = " <<  rocksdb::get_perf_context()->bloom_sst_hit_count << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

    if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
      continue;
    }
    double block_read_cpu_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), ".*block_read_cpu_time = ([0-9.]+)");

    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*disk_access_count/N_repetitions << std::endl;
    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions/1e6 << " (ms) " << std::endl << std::endl;
    testing_result_file << "filtered by RDF count = " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl; 
    // if (number_of_PQs == -1){
    //   testing_result_file << "number of PQ = " << system_verifier->getAllExistingKeys().size() << std::endl;
    // }else{
    //   testing_result_file << "number of PQ = " << number_of_PQs << std::endl;
    // }
    if (number_of_PQs_on_existing_keys == -1){
      testing_result_file << "number of PQ = " << system_verifier->getAllExistingKeys().size() << std::endl;
    }else{
      testing_result_file << "number of PQ = " << number_of_PQs_on_existing_keys << std::endl;
    }
    testing_result_file << system_verifier->getAllCount("", "", "", N_repetitions) << std::endl;
    testing_result_file << "block_read_cpu_time = " << 1.0*block_read_cpu_time/N_repetitions/1e3  << "" << std::endl;
 

    // std::string prefix = " (Exist Keys) " + system_verifier->getStringOfRDFTypeChosed() + " ";
    // std::string prefix = " (Exist Keys " + prefix_number_of_PQs + ") "+ system_verifier->getStringOfRDFTypeChosed() + " ";
    std::string prefix = " (Exist Keys " + prefix_number_of_PQs_on_existing_keys + ") "+ system_verifier->getStringOfRDFTypeChosed() + " ";
    testing_result_file2 << ",\"" + prefix + " elapsed time\" : " << 1.0*point_query_time/N_repetitions/1e6 << std::endl;
    testing_result_file2 << ",\"" + prefix + " filtered by RDF count\" : " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl;
    // if (number_of_PQs == -1){
    //   testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << system_verifier->getAllExistingKeys().size() << std::endl;
    // }else{
    //   testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << number_of_PQs << std::endl;
    // }
    if (number_of_PQs_on_existing_keys == -1){
      testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << system_verifier->getAllExistingKeys().size() << std::endl;
    }else{
      testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << number_of_PQs_on_existing_keys << std::endl;
    }
    testing_result_file2 << system_verifier->getAllCount(",", "\"", prefix, N_repetitions) << std::endl;
    testing_result_file2 << ",\"" + prefix + " block_read_cpu_time\" : " << 1.0*block_read_cpu_time/N_repetitions/1e3 << std::endl;

    if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY" 
       || system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"
       || system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF" 
       || (system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF")){
      double filter_false_positive_rate = -1;
      if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY"){
        filter_false_positive_rate = db->getFilterFalsePositiveRateInPLRDFStringKey();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"){
        filter_false_positive_rate = db->getFilterFalsePositiveRateInSplitPLRDFStringKey();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF"){
        filter_false_positive_rate = db->getFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF"){
        filter_false_positive_rate = db->getFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }
      std::cout << prefix + " filter false positive rate _out = "  << std::fixed << std::setprecision(4) << filter_false_positive_rate << std::fixed << std::setprecision(2) << std::endl;
      testing_result_file << ",\"" + prefix + " filter false positive rate\" : "  << std::fixed << std::setprecision(4) << filter_false_positive_rate << std::fixed << std::setprecision(2) << std::endl;
      testing_result_file2 << ",\"" + prefix + " filter false positive rate\" : "  << std::fixed << std::setprecision(4) << filter_false_positive_rate << std::fixed << std::setprecision(2) << std::endl;
    }
    testing_logger.output_statistics(testing_result_file, testing_result_file2, prefix);
    // print_perf_iostats_context(std::cout, 1);
  }
  // std::cout << prefix_number_of_PQs << " point_query_time_on_existing_keys_ns = " << point_query_time_on_existing_keys_ns << std::endl;
  std::cout << prefix_number_of_PQs_on_existing_keys << " point_query_time_on_existing_keys_ns_out = " << point_query_time_on_existing_keys_ns << std::endl;

std::cout << "!!! Testing On historic-existing Keys " << std::endl;
system_verifier->set_flag_testing_on_currently_deleted_keys();

  testing_result_file << std::endl << std::endl;
  testing_result_file << "----------------------Testing On historic-existing Keys-----------------------" << std::endl;
  for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
    long long total_read_count = 0;
    long long total_read_bytes = 0;
    testing_logger.reset();




    system_verifier->setRDFTypeChosed(t);
    system_verifier->resetAllCount();
    system_verifier->resetAllDuration();

    if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY" 
       || system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"
       || system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF" 
       || (system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF")){
      // double filter_false_positive_rate = -1;
      if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY"){
        db->clearFilterFalsePositiveRateInPLRDFStringKey();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"){
        db->clearFilterFalsePositiveRateInSplitPLRDFStringKey();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF"){
        // filter_false_positive_rate = db->getFilterFalsePositiveRateInSuRFLevelFileRDF();
        db->clearFilterFalsePositiveRateInSuRFLevelFileRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF"){
        // filter_false_positive_rate = db->getFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
        db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }
    }

    if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
      s = db->SetOptions({{"max_open_files", "1"}}); // is there any compaction happended after this????
    }else{
      s = db->SetOptions({{"max_open_files", std::to_string(_env->max_open_files)}}); // is there any compaction happended after this????
    }

    // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << prefix_number_of_PQs << std::endl;
    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << prefix_number_of_PQs_on_historic_existing_keys << std::endl;
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



      if(flag_reopen_db_for_each_RDF_testing == true){
        retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, plrdf_stringkey_prime, split_plrdf_stringkey_prime, top_level_rdf_prime, skyline_rdf_prime);
        reopen_DB(db_ptr2, op, write_op, read_op, _env, kDBPath);
        set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, plrdf_stringkey_prime, split_plrdf_stringkey_prime, top_level_rdf_prime, skyline_rdf_prime);
      }

      std::vector<uint64_t> cache_tombstone_bytes;
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
        if(system_verifier->getStringOfRDFTypeChosed() == "NONE_CACHE_RANGETOMBSTONE_TRACING"){
          uint64_t bytes = checking::CacheTombstoneTracer::getInstance()->getTotalTombstoneBytes();
          cache_tombstone_bytes.push_back(bytes);
        }
        size_t separator_pos = value.find("|");
        time_stamp = value.substr(separator_pos + 1);
        value = value.substr(0, separator_pos);

        if(s.ok() != gt_is_exist){
          #ifdef DEBUG_VERIFICATION
            std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
          #endif
          testing_result_file << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
        }
        if(gt_is_exist == false){continue;}
        if(value != gt_value){
          #ifdef DEBUG_VERIFICATION
            std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
          #endif
          testing_result_file << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
        }
      }
      if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
        continue;
      }
      if(system_verifier->getStringOfRDFTypeChosed() == "NONE_CACHE_RANGETOMBSTONE_TRACING"){
        string i_round_str = "i_round="+std::to_string(i)+" ";
    // std::string prefix = " (Historcially Exist Keys " + i + prefix_number_of_PQs + " " + i_round_str + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
    std::string prefix = " (Historcially Exist Keys " + i + prefix_number_of_PQs_on_historic_existing_keys + " " + i_round_str + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
    testing_result_file << ",\"" + prefix + " cache tombstone bytes\" : " << "[";
        testing_result_file2 << ",\"" + prefix + " cache tombstone bytes\" : " << "[";
        int len_ctb = cache_tombstone_bytes.size();
        int i_ctb = 0;
        if(i_ctb < len_ctb){
          testing_result_file << cache_tombstone_bytes[i_ctb];
          testing_result_file2 << cache_tombstone_bytes[i_ctb];
          i_ctb++;
        }
        for(;i_ctb < len_ctb; i_ctb++){
          auto &x = cache_tombstone_bytes[i_ctb];
          testing_result_file << ", " << x;
          testing_result_file2 << ", " << x;
        }
        testing_result_file << "]" << std::endl;
        testing_result_file2 << "]" << std::endl;
      }
      disk_access_count += system_verifier->getDiskAccessCount();

      testing_result_file << " Disk Access count = " << system_verifier->getDiskAccessCount() << std::endl;
      testing_logger.set_to_end(op, testing_result_file);
    }      
    // std::cout << " rocksdb::get_perf_context()->bloom_sst_miss_count = " <<  rocksdb::get_perf_context()->bloom_sst_miss_count << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    // std::cout << " rocksdb::get_perf_context()->bloom_sst_hit_count = " <<  rocksdb::get_perf_context()->bloom_sst_hit_count << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

    if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
      continue;
    }
    double block_read_cpu_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), ".*block_read_cpu_time = ([0-9.]+)");

    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*disk_access_count/N_repetitions << std::endl;
    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions/1e6 << " (ms) " << std::endl << std::endl;
    testing_result_file << "filtered by RDF count = " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl; 
    // if (number_of_PQs == -1){
    //   testing_result_file << "number of PQ = " << system_verifier->getHistoricExistingKeys().size() << std::endl;
    // }else{
    //   testing_result_file << "number of PQ = " << number_of_PQs << std::endl;
    // }
    if (number_of_PQs_on_historic_existing_keys == -1){
      testing_result_file << "number of PQ = " << system_verifier->getHistoricExistingKeys().size() << std::endl;
    }else{
      testing_result_file << "number of PQ = " << number_of_PQs_on_historic_existing_keys << std::endl;
    }
    testing_result_file << system_verifier->getAllCount("", "", "", N_repetitions) << std::endl;
    testing_result_file << "block_read_cpu_time = " << 1.0*block_read_cpu_time/N_repetitions/1e3  << "" << std::endl;

    // std::string prefix = " (Historcially Exist Keys) " + system_verifier->getStringOfRDFTypeChosed() + " ";
    // std::string prefix = " (Historcially Exist Keys " + prefix_number_of_PQs + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
    std::string prefix = " (Historcially Exist Keys " + prefix_number_of_PQs_on_historic_existing_keys + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
    testing_result_file2 << ",\"" + prefix + " elapsed time\" : " << 1.0*point_query_time/N_repetitions/1e6 << std::endl;
    testing_result_file2 << ",\"" + prefix + " filtered by RDF count\" : " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl;
    // if (number_of_PQs == -1){
    //   testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << system_verifier->getHistoricExistingKeys().size() << std::endl;
    // }else{
    //   testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << number_of_PQs << std::endl;
    // }
    if (number_of_PQs_on_historic_existing_keys == -1){
      testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << system_verifier->getHistoricExistingKeys().size() << std::endl;
    }else{
      testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << number_of_PQs_on_historic_existing_keys << std::endl;
    }
    testing_result_file2 << system_verifier->getAllCount(",", "\"", prefix, N_repetitions) << std::endl;
    testing_result_file2 << ",\"" + prefix + " block_read_cpu_time\" : " << 1.0*block_read_cpu_time/N_repetitions/1e3 << std::endl;

    if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY" 
       || system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"
       || system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF" 
       || (system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF")){
      double filter_false_positive_rate = -1;
      if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY"){
        filter_false_positive_rate = db->getFilterFalsePositiveRateInPLRDFStringKey();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"){
        filter_false_positive_rate = db->getFilterFalsePositiveRateInSplitPLRDFStringKey();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF"){
        filter_false_positive_rate = db->getFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF"){
        filter_false_positive_rate = db->getFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }
      std::cout << prefix + " filter false positive rate _out = "  << std::fixed << std::setprecision(4) << filter_false_positive_rate << std::fixed << std::setprecision(2) << std::endl;
      testing_result_file << ",\"" + prefix + " filter false positive rate\" : "  << std::fixed << std::setprecision(4) << filter_false_positive_rate << std::fixed << std::setprecision(2) << std::endl;
      testing_result_file2 << ",\"" + prefix + " filter false positive rate\" : "  << std::fixed << std::setprecision(4) << filter_false_positive_rate << std::fixed << std::setprecision(2) << std::endl;
    }
    testing_logger.output_statistics(testing_result_file, testing_result_file2, prefix);
    // print_perf_iostats_context(std::cout, 1);
  }
system_verifier->reset_flag_testing_on_currently_deleted_keys();

std::cout << "!!! Testing On Currently Deleted Keys " << std::endl;
system_verifier->set_flag_testing_on_currently_deleted_keys();
system_verifier->startPQTracing();

  testing_result_file << std::endl << std::endl;
  testing_result_file << "----------------------Testing On Currently Deleted Keys-----------------------" << std::endl;

  for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
    long long total_read_count = 0;
    long long total_read_bytes = 0;
    testing_logger.reset();



    system_verifier->setRDFTypeChosed(t);
    system_verifier->resetAllCount();
    system_verifier->resetAllDuration();

    if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY" 
       || system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"
       || system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF" 
       || (system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF")){
      // double filter_false_positive_rate = -1;
      if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY"){
        db->clearFilterFalsePositiveRateInPLRDFStringKey();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"){
        db->clearFilterFalsePositiveRateInSplitPLRDFStringKey();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF"){
        // filter_false_positive_rate = db->getFilterFalsePositiveRateInSuRFLevelFileRDF();
        db->clearFilterFalsePositiveRateInSuRFLevelFileRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF"){
        // filter_false_positive_rate = db->getFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
        db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }
    }

    if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
      s = db->SetOptions({{"max_open_files", "1"}}); // is there any compaction happended after this????
    }else{
      s = db->SetOptions({{"max_open_files", std::to_string(_env->max_open_files)}}); // is there any compaction happended after this????
    }

    // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << prefix_number_of_PQs << std::endl;
    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << prefix_number_of_PQs_on_currently_deleted_keys << std::endl;
    disk_access_count = 0;
    point_query_time = 0;
    start_pq = std::chrono::high_resolution_clock::now();
    rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
    rocksdb::get_perf_context()->Reset();
    rocksdb::get_iostats_context()->Reset();
    for(auto i = 0; i < N_repetitions; i++){
      clearCache(op);
      system_verifier->clearMapPQTracingInfo();
      system_verifier->clearVPQTracingInfo();

      db = *db_ptr2;
      system_verifier->resetDiskAccessCount();
      testing_logger.set_to_start(op);



      if(flag_reopen_db_for_each_RDF_testing == true){
        // retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
        retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, plrdf_stringkey_prime, split_plrdf_stringkey_prime, top_level_rdf_prime, skyline_rdf_prime);
        reopen_DB(db_ptr2, op, write_op, read_op, _env, kDBPath);
        // set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
        set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, plrdf_stringkey_prime, split_plrdf_stringkey_prime, top_level_rdf_prime, skyline_rdf_prime);
      }

      std::vector<uint64_t> cache_tombstone_bytes;
      for(auto x: system_verifier->getCurrentlyDeletedKeysAtNRound(i)){
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
        point_query_time_on_currently_deleted_all_ns += duration_pq.count();
        if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
          continue;
        }
        if(system_verifier->getStringOfRDFTypeChosed() == "NONE_CACHE_RANGETOMBSTONE_TRACING"){
          uint64_t bytes = checking::CacheTombstoneTracer::getInstance()->getTotalTombstoneBytes();
          cache_tombstone_bytes.push_back(bytes);
        }
        size_t separator_pos = value.find("|");
        time_stamp = value.substr(separator_pos + 1);
        value = value.substr(0, separator_pos);

        if(s.ok() != gt_is_exist){
          #ifdef DEBUG_VERIFICATION
            std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
          #endif
          testing_result_file << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
        }
        if(gt_is_exist == false){continue;}
        if(value != gt_value){
          #ifdef DEBUG_VERIFICATION
            std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
          #endif
          testing_result_file << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
        }
      }
      if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
        continue;
      }
      if(system_verifier->getStringOfRDFTypeChosed() == "NONE_CACHE_RANGETOMBSTONE_TRACING"){
        string i_round_str = "i_round="+std::to_string(i)+" ";
        // std::string prefix = " (Currently Deleted Keys " + i + prefix_number_of_PQs + " " + i_round_str + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
        std::string prefix = " (Currently Deleted Keys " + i + prefix_number_of_PQs_on_currently_deleted_keys + " " + i_round_str + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
        testing_result_file << ",\"" + prefix + " cache tombstone bytes\" : " << "[";
        testing_result_file2 << ",\"" + prefix + " cache tombstone bytes\" : " << "[";
        int len_ctb = cache_tombstone_bytes.size();
        int i_ctb = 0;
        if(i_ctb < len_ctb){
          testing_result_file << cache_tombstone_bytes[i_ctb];
          testing_result_file2 << cache_tombstone_bytes[i_ctb];
          i_ctb++;
        }
        for(;i_ctb < len_ctb; i_ctb++){
          auto &x = cache_tombstone_bytes[i_ctb];
          testing_result_file << ", " << x;
          testing_result_file2 << ", " << x;
        }
        testing_result_file << "]" << std::endl;
        testing_result_file2 << "]" << std::endl;
      }
      disk_access_count += system_verifier->getDiskAccessCount();


      testing_result_file << " Disk Access count = " << system_verifier->getDiskAccessCount() << std::endl;
      testing_logger.set_to_end(op, testing_result_file);


      // std::string prefix = " (Currently Deleted Keys " + prefix_number_of_PQs + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
      std::string prefix = " (Currently Deleted Keys " + prefix_number_of_PQs_on_currently_deleted_keys + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
      testing_result_file2 << system_verifier->getMapPQTracingInfo(",", "\"", prefix, i) << std::endl;
      testing_result_file2 << system_verifier->getVPQTracingInfo(",", "\"", prefix, i) << std::endl;
      // system_verifier->clearMapPQTracingInfo();
      // system_verifier->clearVPQTracingInfo();
    }     
    // std::cout << " rocksdb::get_perf_context()->bloom_sst_miss_count = " <<  rocksdb::get_perf_context()->bloom_sst_miss_count << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    // std::cout << " rocksdb::get_perf_context()->bloom_sst_hit_count = " <<  rocksdb::get_perf_context()->bloom_sst_hit_count << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

    if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
      continue;
    }
    double block_read_cpu_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), ".*block_read_cpu_time = ([0-9.]+)");

    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*disk_access_count/N_repetitions << std::endl;
    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions/1e6 << " (ms) " << std::endl << std::endl;
    testing_result_file << "filtered by RDF count = " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl; 
    // if (number_of_PQs == -1){
    //   testing_result_file << "number of PQ = " << system_verifier->getCurrentlyDeletedKeys().size() << std::endl;
    // }else{
    //   testing_result_file << "number of PQ = " << number_of_PQs << std::endl;
    // }
    if (number_of_PQs_on_currently_deleted_keys == -1){
      testing_result_file << "number of PQ = " << system_verifier->getCurrentlyDeletedKeys().size() << std::endl;
    }else{
      testing_result_file << "number of PQ = " << number_of_PQs_on_currently_deleted_keys << std::endl;
    }
    testing_result_file << system_verifier->getAllCount("", "", "", N_repetitions) << std::endl;
    testing_result_file << "block_read_cpu_time = " << 1.0*block_read_cpu_time/N_repetitions/1e3  << "" << std::endl;

    // std::string prefix = " (Currently Deleted Keys " + prefix_number_of_PQs + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
    std::string prefix = " (Currently Deleted Keys " + prefix_number_of_PQs_on_currently_deleted_keys + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
    testing_result_file2 << ",\"" + prefix + " elapsed time\" : " << 1.0*point_query_time/N_repetitions/1e6 << std::endl;
    testing_result_file2 << ",\"" + prefix + " filtered by RDF count\" : " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl;
    // if (number_of_PQs == -1){
    //   testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << system_verifier->getCurrentlyDeletedKeys().size() << std::endl;
    // }else{
    //   testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << number_of_PQs << std::endl;
    // }
    if (number_of_PQs_on_currently_deleted_keys == -1){
      testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << system_verifier->getCurrentlyDeletedKeys().size() << std::endl;
    }else{
      testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << number_of_PQs_on_currently_deleted_keys << std::endl;
    }
    testing_result_file2 << system_verifier->getAllCount(",", "\"", prefix, N_repetitions) << std::endl;
    testing_result_file2 << ",\"" + prefix + " block_read_cpu_time\" : " << 1.0*block_read_cpu_time/N_repetitions/1e3 << std::endl;

    if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY" 
       || system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"
       || system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF" 
       || (system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF")){
      double filter_false_positive_rate = -1;
      if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY"){
        filter_false_positive_rate = db->getFilterFalsePositiveRateInPLRDFStringKey();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"){
        filter_false_positive_rate = db->getFilterFalsePositiveRateInSplitPLRDFStringKey();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF"){
        filter_false_positive_rate = db->getFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF"){
        filter_false_positive_rate = db->getFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }
      std::cout << prefix + " filter false positive rate _out = "  << std::fixed << std::setprecision(4) << filter_false_positive_rate << std::fixed << std::setprecision(2) << std::endl;
      testing_result_file << ",\"" + prefix + " filter false positive rate\" : "  << std::fixed << std::setprecision(4) << filter_false_positive_rate << std::fixed << std::setprecision(2) << std::endl;
      testing_result_file2 << ",\"" + prefix + " filter false positive rate\" : "  << std::fixed << std::setprecision(4) << filter_false_positive_rate << std::fixed << std::setprecision(2) << std::endl;
    }
    testing_logger.output_statistics(testing_result_file, testing_result_file2, prefix);
    // print_perf_iostats_context(std::cout, 1);
  }
system_verifier->reset_flag_testing_on_currently_deleted_keys();
system_verifier->endPQTracing();
// std::cout << prefix_number_of_PQs << " point_query_time_on_currently_deleted_all_ns = " << point_query_time_on_currently_deleted_all_ns << std::endl;
std::cout << prefix_number_of_PQs_on_currently_deleted_keys << " point_query_time_on_currently_deleted_all_ns_out = " << point_query_time_on_currently_deleted_all_ns << std::endl;

std::cout << "!!! Testing On Currently Non-inserted Keys " << std::endl;

  testing_result_file << std::endl << std::endl;
  testing_result_file << "----------------------Testing On Currently Non-inserted Keys-----------------------" << std::endl;
  int CurrentlyNonInsertedKeysNum = system_verifier->getCurrentlyNonInsertedKeysNum();
  testing_result_file << "number of currently non-inserted keys = " << CurrentlyNonInsertedKeysNum << std::endl;

  for(uint t = 0; t < system_verifier->getNumberOfRDFTypes(); t++){
    long long total_read_count = 0;
    long long total_read_bytes = 0;
    testing_logger.reset();


    system_verifier->setRDFTypeChosed(t);
    system_verifier->resetAllCount();
    system_verifier->resetAllDuration();
    
    
    if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY" 
       || system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"
       || system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF" 
       || (system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF")){
      // double filter_false_positive_rate = -1;
      if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY"){
        db->clearFilterFalsePositiveRateInPLRDFStringKey();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"){
        db->clearFilterFalsePositiveRateInSplitPLRDFStringKey();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF"){
        // filter_false_positive_rate = db->getFilterFalsePositiveRateInSuRFLevelFileRDF();
        db->clearFilterFalsePositiveRateInSuRFLevelFileRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF"){
        // filter_false_positive_rate = db->getFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
        db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }
    }
    
    if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
      s = db->SetOptions({{"max_open_files", "1"}}); // is there any compaction happended after this????
    }else{
      s = db->SetOptions({{"max_open_files", std::to_string(_env->max_open_files)}}); // is there any compaction happended after this????
    }

    // testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << prefix_number_of_PQs << std::endl;
    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << prefix_number_of_PQs_on_currently_non_inserted_keys << std::endl;
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



      if(flag_reopen_db_for_each_RDF_testing == true){
        // retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
        retrieve_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, plrdf_stringkey_prime, split_plrdf_stringkey_prime, top_level_rdf_prime, skyline_rdf_prime);
        reopen_DB(db_ptr2, op, write_op, read_op, _env, kDBPath);
        // set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, top_level_rdf_prime, skyline_rdf_prime, skyline__numbers_of_ranges_in_rdf_log);
        set_all_RDFs(db_ptr2, plrdf_prime, split_plrdf_prime, plrdf_stringkey_prime, split_plrdf_stringkey_prime, top_level_rdf_prime, skyline_rdf_prime);
      }

      std::vector<uint64_t> cache_tombstone_bytes;
    //   for(auto &x: system_verifier->getCurrentlyNonInsertedKeys()){ // test on 1000 keys
      for(auto x: system_verifier->getCurrentlyNonInsertedKeysAtNRound(i)){ // test on 1000 keys
        bool gt_is_exist = system_verifier->isKeyExist(x); // should be false
        std::string gt_value = system_verifier->get(x); // should be ""

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
        if(system_verifier->getStringOfRDFTypeChosed() == "NONE_CACHE_RANGETOMBSTONE_TRACING"){
          uint64_t bytes = checking::CacheTombstoneTracer::getInstance()->getTotalTombstoneBytes();
          cache_tombstone_bytes.push_back(bytes);
        }
        size_t separator_pos = value.find("|");
        time_stamp = value.substr(separator_pos + 1);
        value = value.substr(0, separator_pos);
        if(s.ok() != gt_is_exist){
          #ifdef DEBUG_VERIFICATION
            std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
          #endif
          testing_result_file << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
        }
        if(gt_is_exist == false){continue;}
        if(value != gt_value){
          #ifdef DEBUG_VERIFICATION
            std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
          #endif
          testing_result_file << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
        }
      }
      if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
        continue;
      }
      if(system_verifier->getStringOfRDFTypeChosed() == "NONE_CACHE_RANGETOMBSTONE_TRACING"){
        string i_round_str = "i_round="+std::to_string(i)+" ";
        // std::string prefix = " (Currently Non-Inserted Keys " + i + prefix_number_of_PQs + " " + i_round_str + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
        std::string prefix = " (Currently Non-Inserted Keys " + i + prefix_number_of_PQs_on_currently_non_inserted_keys + " " + i_round_str + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
        testing_result_file << ",\"" + prefix + " cache tombstone bytes\" : " << "[";
        testing_result_file2 << ",\"" + prefix + " cache tombstone bytes\" : " << "[";
        int len_ctb = cache_tombstone_bytes.size();
        int i_ctb = 0;
        if(i_ctb < len_ctb){
          testing_result_file << cache_tombstone_bytes[i_ctb];
          testing_result_file2 << cache_tombstone_bytes[i_ctb];
          i_ctb++;
        }
        for(;i_ctb < len_ctb; i_ctb++){
          auto &x = cache_tombstone_bytes[i_ctb];
          testing_result_file << ", " << x;
          testing_result_file2 << ", " << x;
        }
        testing_result_file << "]" << std::endl;
        testing_result_file2 << "]" << std::endl;
      }
      disk_access_count += system_verifier->getDiskAccessCount();
 

      testing_result_file << " Disk Access count = " << system_verifier->getDiskAccessCount() << std::endl;
      testing_logger.set_to_end(op, testing_result_file);
    }  
    // std::cout << " rocksdb::get_perf_context()->bloom_sst_miss_count = " <<  rocksdb::get_perf_context()->bloom_sst_miss_count << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    // std::cout << " rocksdb::get_perf_context()->bloom_sst_hit_count = " <<  rocksdb::get_perf_context()->bloom_sst_hit_count << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

    if(system_verifier->getStringOfRDFTypeChosed() == "NONE_DUMMY"){
      continue;
    }
    double block_read_cpu_time = parsing_value_from_string(rocksdb::get_perf_context()->ToString(), ".*block_read_cpu_time = ([0-9.]+)");

    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << "Average Disk Access count = " << 1.0*disk_access_count/N_repetitions << std::endl;
    testing_result_file << system_verifier->getStringOfRDFTypeChosed() << " " << std::fixed << std::setprecision(2) << " elapsed time = " << 1.0*point_query_time/N_repetitions/1e6 << " (ms) " << std::endl << std::endl;
    testing_result_file << "filtered by RDF count = " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl; 
    // if (number_of_PQs == -1){
    //   testing_result_file << "number of PQ = " << system_verifier->getCurrentlyNonInsertedKeys().size() << std::endl;
    // }else{
    //   testing_result_file << "number of PQ = " << number_of_PQs << std::endl;
    // }
    if (number_of_PQs_on_currently_non_inserted_keys == -1){
      testing_result_file << "number of PQ = " << system_verifier->getCurrentlyNonInsertedKeys().size() << std::endl;
    }else{
      testing_result_file << "number of PQ = " << number_of_PQs_on_currently_non_inserted_keys << std::endl;
    }
    testing_result_file << system_verifier->getAllCount("", "", "", N_repetitions) << std::endl;
    testing_result_file << "block_read_cpu_time = " << 1.0*block_read_cpu_time/N_repetitions/1e3  << "" << std::endl;

    // std::string prefix = " (Non-inserted Keys " + prefix_number_of_PQs + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
    std::string prefix = " (Non-inserted Keys " + prefix_number_of_PQs_on_currently_non_inserted_keys + ") " + system_verifier->getStringOfRDFTypeChosed() + " ";
    testing_result_file2 << ",\"" + prefix + " elapsed time\" : " << 1.0*point_query_time/N_repetitions/1e6 << std::endl;
    testing_result_file2 << ",\"" + prefix + " filtered by RDF count\" : " << std::fixed << std::setprecision(2) << 1.0*system_verifier->getFilteredByRDFCount()/N_repetitions << std::endl;
    // if (number_of_PQs == -1){
    //   testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << system_verifier->getCurrentlyNonInsertedKeys().size() << std::endl;
    // }else{
    //   testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << number_of_PQs << std::endl;
    // }
    if (number_of_PQs_on_currently_non_inserted_keys == -1){
      testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << system_verifier->getCurrentlyNonInsertedKeys().size() << std::endl;
    }else{
      testing_result_file2 << ",\"" + prefix + " number of PQ\" : " << number_of_PQs_on_currently_non_inserted_keys << std::endl;
    }
    testing_result_file2 << system_verifier->getAllCount(",", "\"", prefix, N_repetitions) << std::endl;
    testing_result_file2 << ",\"" + prefix + " block_read_cpu_time\" : " << 1.0*block_read_cpu_time/N_repetitions/1e3 << std::endl;

    if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY" 
       || system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"
       || system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF" 
       || (system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF")){
      double filter_false_positive_rate = -1;
      if(system_verifier->getStringOfRDFTypeChosed() == "PLRDF_STRING_KEY"){
        filter_false_positive_rate = db->getFilterFalsePositiveRateInPLRDFStringKey();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SPLIT_PLRDF_STRING_KEY"){
        filter_false_positive_rate = db->getFilterFalsePositiveRateInSplitPLRDFStringKey();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_RDF"){
        filter_false_positive_rate = db->getFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }else if(system_verifier->getStringOfRDFTypeChosed() == "SuRF_LF_SPLIT_RDF"){
        filter_false_positive_rate = db->getFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
        // db->clearFilterFalsePositiveRateInSuRFLevelFileSplitRDF();
      }
      std::cout << prefix + " filter false positive rate _out = "  << std::fixed << std::setprecision(4) << filter_false_positive_rate << std::fixed << std::setprecision(2) << std::endl;
      testing_result_file << ",\"" + prefix + " filter false positive rate\" : "  << std::fixed << std::setprecision(4) << filter_false_positive_rate << std::fixed << std::setprecision(2) << std::endl;
      testing_result_file2 << ",\"" + prefix + " filter false positive rate\" : "  << std::fixed << std::setprecision(4) << filter_false_positive_rate << std::fixed << std::setprecision(2) << std::endl;
    }
    testing_logger.output_statistics(testing_result_file, testing_result_file2, prefix);
    print_perf_iostats_context(std::cout, prefix, 1);
  }

}

void verification_runner::endPQVerification(){
  checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();

  
  testing_result_file << std::endl << std::endl;
  testing_result_file << "----------------------End Testing-----------------------" << std::endl;

  testing_result_file.close();

  
  testing_result_file2 << ",\"End\" : \"End\""<< std::endl;
  testing_result_file2 << "}"<< std::endl;
  testing_result_file2.close();

  system_verifier->resetRunningPQ();
}

#endif // UTILS_RUN_VERIFICATION_H