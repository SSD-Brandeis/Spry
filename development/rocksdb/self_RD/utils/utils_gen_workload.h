#ifndef UTILS_GEN_WORKLOAD_H
#define UTILS_GEN_WORKLOAD_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "../workload/args.hxx"
#include "../workload/workload_generator.h"
#include "../env_settings/emu_environment.h"
#include "rocksdb/system_verifier.h"





void gen_workload(EmuEnv* _env){


  int entry_size = _env->entry_size;
  double correlation = _env->correlation;
  long long num_inserts = _env->num_inserts;
  int rd_count = _env->rd_count;
  double selectivity = _env->selectivity;
  string workload_file_name = _env->workload_file_name;
  double insert_before_rangeDelete = _env->insert_before_rangeDelete;

  
    
  WorkloadGenerator workload_generator;
  // long number_Of_point_in_the_beginning = (long) ceil(num_inserts * insert_before_rangeDelete);
  // assert(1.0*rd_count*selectivity <= 1.0);
  // workload_generator.generateWorkload((long)num_inserts, (long)entry_size, (double) correlation, 
  //         (long)rd_count, (double) selectivity, (long) number_Of_point_in_the_beginning, (string) workload_file_name,   
  //         (int) checking::SystemVerifier::getKeySize()
  //         );    

  string gen_workload_command = string("./K-V-Workload-Generator-master/load_gen")
      + string(" --insert=") + to_string(num_inserts) 
      + string(" --range_delete=") + to_string(rd_count)  
      + string(" --range_delete_selectivity=") + to_string(selectivity) 
      + string(" --entry_size=") + to_string(entry_size - checking::SystemVerifier::getKeySize() + sizeof(uint32_t));

  string move_workload_command = string(" mv workload.txt ./K-V-Workload-Generator-master/ ");

  // The command you want to execute, for example, "ls" to list files in the current directory.
  string sed_workload_command = string("cat ./K-V-Workload-Generator-master/workload.txt | sed 's/^R/D Range/g' > ") + string("./") + workload_file_name;

  cout << "gen_workload_command: " << gen_workload_command << endl;
  // Use the system function to execute the command.
  int returnCode = system(gen_workload_command.c_str());

  // Check the return code to see if the command was executed successfully.
  if (returnCode == 0) {
      // std::cout << "Gen workload command executed successfully." << std::endl;
  } else {
      std::cout << "Gen workload command failed to execute." << std::endl;
      std::cerr << "Gen workload command failed to execute." << std::endl;
      // std::perror("system");
      exit(-1);
  }

  cout << "move_workload_command: " << move_workload_command << endl;
  // Use the system function to execute the command.
  returnCode = system(move_workload_command.c_str());

  // Check the return code to see if the command was executed successfully.
  if (returnCode == 0) {
      // std::cout << "Move workload command executed successfully." << std::endl;
  } else {
      std::cout << "Move workload command failed to execute." << std::endl;
      std::cerr << "Move workload command failed to execute." << std::endl;
      // std::perror("system");
      exit(-1);
  }

  
  cout << "sed_workload_command: " << sed_workload_command << endl;
  // Use the system function to execute the command.
  returnCode = system(sed_workload_command.c_str());

  // Check the return code to see if the command was executed successfully.
  if (returnCode == 0) {
      // std::cout << "Move workload command executed successfully." << std::endl;
  } else {
      std::cout << "Sed workload command failed to execute." << std::endl;
      std::cerr << "Sed workload command failed to execute." << std::endl;
      // std::perror("system");
      exit(-1);
  }


  std::cout << "Workload Generated!" << std::endl;
}


int parse_arguments2(int argc, char *argv[], EmuEnv* _env) {
  args::ArgumentParser parser("RocksDB_parser.", "");

  args::Group group1(parser, "This group is all exclusive:", args::Group::Validators::DontCare);
/*
  args::Group group1(parser, "This group is all exclusive:", args::Group::Validators::AtMostOne);
  args::Group group2(parser, "Path is needed:", args::Group::Validators::All);
  args::Group group3(parser, "This group is all exclusive (either N or L):", args::Group::Validators::Xor);
  args::Group group4(parser, "Optional switches and parameters:", args::Group::Validators::DontCare);
  args::Group group5(parser, "Optional less frequent switches and parameters:", args::Group::Validators::DontCare);
*/

  args::ValueFlag<int> destroy_database_cmd(group1, "d", "Destroy and recreate the database [def: 1]", {'d', "destroy"});
  args::ValueFlag<int> clear_system_cache_cmd(group1, "cc", "Clear system cache [def: 1]", {"cc"}); // !YBS-sep09-XX!

  args::ValueFlag<int> size_ratio_cmd(group1, "T", "The number of unique inserts to issue in the experiment [def: 10]", {'T', "size_ratio"});
  args::ValueFlag<int> buffer_size_in_pages_cmd(group1, "P", "The number of unique inserts to issue in the experiment [def: 4096]", {'P', "buffer_size_in_pages"});
  args::ValueFlag<int> entries_per_page_cmd(group1, "B", "The number of unique inserts to issue in the experiment [def: 4]", {'B', "entries_per_page"});
  args::ValueFlag<int> entry_size_cmd(group1, "E", "The number of unique inserts to issue in the experiment [def: 1024 B]", {'E', "entry_size"});
  args::ValueFlag<long> buffer_size_cmd(group1, "M", "The number of unique inserts to issue in the experiment [def: 16 MB]", {'M', "memory_size"});
  args::ValueFlag<int> file_to_memtable_size_ratio_cmd(group1, "file_to_memtable_size_ratio", "The number of unique inserts to issue in the experiment [def: 1]", {'f', "file_to_memtable_size_ratio"});
  args::ValueFlag<long> file_size_cmd(group1, "file_size", "The number of unique inserts to issue in the experiment [def: 256 KB]", {'F', "file_size"});
  args::ValueFlag<int> verbosity_cmd(group1, "verbosity", "The verbosity level of execution [0,1,2; def: 0]", {'V', "verbosity"});
  args::ValueFlag<int> compaction_pri_cmd(group1, "compaction_pri", "[Compaction priority: 1 for kMinOverlappingRatio, 2 for kByCompensatedSize, 3 for kOldestLargestSeqFirst, 4 for kOldestSmallestSeqFirst; def: 1]", {'c', "compaction_pri"});
  args::ValueFlag<int> compaction_style_cmd(group1, "compaction_style", "[Compaction priority: 1 for kCompactionStyleLevel, 2 for kCompactionStyleUniversal, 3 for kCompactionStyleFIFO, 4 for kCompactionStyleNone; def: 1]", {'C', "compaction_style"}); // !YBS-sep07-XX!
  args::ValueFlag<int> bits_per_key_cmd(group1, "bits_per_key", "The number of bits per key assigned to Bloom filter [def: 10]", {'b', "bits_per_key"});
  args::ValueFlag<int> block_cache_cmd(group1, "bb", "Block cache size in MB [def: 8 MB]", {"bb"}); // !YBS-sep09-XX!
  args::ValueFlag<int> show_progress_cmd(group1, "show_progress", "Show progress [def: 0]", {'s', "sp"}); // !YBS-sep17-XX!

  args::ValueFlag<long> num_inserts_cmd(group1, "inserts", "The number of unique inserts to issue in the experiment [def: 0]", {'i', "inserts"});





  //YuCheng Added Start
  // args::ValueFlag<int> entry_size_cmd(group1, "E", "Entry size in bytes [def: 128 B]", {'E', "entry_size"});
  args::ValueFlag<double> cor_cmd(group1, "#correlation", "Correlation between sort key and delete key [def: 0]", {"correlation"});
  // args::ValueFlag<long long> num_inserts_cmd(group1, "#inserts", "The number of unique inserts to issue in the experiment [def: 0]", {'i', "num_inserts"});
  args::ValueFlag<int> RD_cmd(group1, "range_delete", "Count of range delete [def:1]", {'R', "RD"});
  args::ValueFlag<double> selectivity_cmd(group1, "selectivity_of_range_delete", "Selectivity of range delete [def:0.001]", {"selectivity"});
  args::ValueFlag<string> workload_filename_cmd(group1, "workload_filename", "workload filename [def:0.001]", {"workload_filename"});
  args::ValueFlag<double> insert_before_range_delete_cmd(group1, "insert_before_range_delete", "percent of inserts in insert themself that precede any of the range delete [def:0.5]", {"insert_before_range_delete"});
  args::ValueFlag<int> gen_workload_cmd(group1, "gen_workload", "want to generate workload [def:1]", {"gen_workload"});
  args::ValueFlag<int> max_open_files_cmd(group1, "max_open_files", "maximum number of opened SST [def:1]", {"max_open_files"});
  args::ValueFlag<int> skip_reading_RD_blocks_cmd(group1, "skip_reading_RD_blocks", "skip_reading_RD_blocks [def:0 (false)]", {"skip_reading_RD_blocks"});
  args::ValueFlag<int> number_of_PQ_cmd(group1, "number_of_PQ", "number_of_PQ [def:5000]", {"number_of_PQ"});
  //YuCheng Added End





  try {
      parser.ParseCLI(argc, argv);
  }
  catch (args::Help&) {
      std::cout << parser;
      exit(0);
      // return 0;
  }
  catch (args::ParseError& e) {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
  }
  catch (args::ValidationError& e) {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
  }

  _env->destroy_database = destroy_database_cmd ? args::get(destroy_database_cmd) : 1;
  _env->clear_system_cache = clear_system_cache_cmd ? args::get(clear_system_cache_cmd) : 1; // !YBS-sep09-XX!

  _env->size_ratio = size_ratio_cmd ? args::get(size_ratio_cmd) : 10;
  _env->buffer_size_in_pages = buffer_size_in_pages_cmd ? args::get(buffer_size_in_pages_cmd) : 4096;
  _env->entries_per_page = entries_per_page_cmd ? args::get(entries_per_page_cmd) : 4;
  _env->entry_size = entry_size_cmd ? args::get(entry_size_cmd) : 1024;
  _env->buffer_size = buffer_size_cmd ? args::get(buffer_size_cmd) : _env->buffer_size_in_pages * _env->entries_per_page * _env->entry_size;
  _env->file_to_memtable_size_ratio = file_to_memtable_size_ratio_cmd ? args::get(file_to_memtable_size_ratio_cmd) : 1;
  _env->file_size = file_size_cmd ? args::get(file_size_cmd) : _env->buffer_size;
  _env->verbosity = verbosity_cmd ? args::get(verbosity_cmd) : 0;
  _env->compaction_pri = compaction_pri_cmd ? args::get(compaction_pri_cmd) : 1;
  _env->compaction_style = compaction_style_cmd ? args::get(compaction_style_cmd) : 1; // !YBS-sep07-XX!
  _env->bits_per_key = bits_per_key_cmd ? args::get(bits_per_key_cmd) : 10;
  _env->block_cache = block_cache_cmd ? args::get(block_cache_cmd) : 8; // !YBS-sep09-XX!
  _env->show_progress = show_progress_cmd ? args::get(show_progress_cmd) : 0; // !YBS-sep17-XX!

  _env->num_inserts = num_inserts_cmd ? args::get(num_inserts_cmd) : 0;

  _env->target_file_size_base = _env->buffer_size; // !YBS-sep07-XX! <-----------
  // _env->max_bytes_for_level_base = _env->buffer_size * _env->size_ratio; // !YBS-sep07-XX!
  // _env->max_bytes_for_level_base = _env->buffer_size; // <-------------------------------------------------------------
  // op->max_bytes_for_level_base = _env->buffer_size; // <-------------------------------------------------------------





  //YuCheng Added Start
  // int entry_size = entry_size_cmd ? args::get(entry_size_cmd) : 128;
  double correlation = cor_cmd ? args::get(cor_cmd) : 0;
  // long long num_inserts = num_inserts_cmd ? args::get(num_inserts_cmd) : 0;
  int rd_count = RD_cmd ? args::get(RD_cmd) : 1;
  double selectivity = selectivity_cmd ? args::get(selectivity_cmd) : 0.001;
  string workload_file_name = workload_filename_cmd ? args::get(workload_filename_cmd) : "workload.txt";
  double insert_before_rangeDelete = insert_before_range_delete_cmd ? args::get(insert_before_range_delete_cmd) : 0.5;
  bool gen_workload = gen_workload_cmd ? (args::get(gen_workload_cmd) != 0) : 1;
  int max_open_files = max_open_files_cmd ? args::get(max_open_files_cmd) : 9999;
  bool skip_reading_RD_blocks = skip_reading_RD_blocks_cmd ? (args::get(skip_reading_RD_blocks_cmd) != 0) : false;
  int number_of_PQ = number_of_PQ_cmd ? args::get(number_of_PQ_cmd) : 5000;
  // _env->entry_size = entry_size;
  _env->correlation = correlation;
  // _env.num_inserts = num_inserts;
  _env->rd_count = rd_count;
  _env->selectivity = selectivity;
  _env->workload_file_name = workload_file_name;
  _env->insert_before_rangeDelete = insert_before_rangeDelete;
  _env->gen_workload = gen_workload;
  _env->max_open_files = max_open_files;
  _env->skip_reading_RD_blocks = skip_reading_RD_blocks;
  _env->number_of_PQ = number_of_PQ;
  //YuCheng Added End
  return 0;
}

#endif