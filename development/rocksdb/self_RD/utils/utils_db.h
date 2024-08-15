
#ifndef UTILS_DB_H
#define UTILS_DB_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <chrono>
#include "utils.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/advanced_options.h"
#include "rocksdb/system_verifier.h"
#include "utils_rdf.h"
#include "../env_settings/emu_environment.h"

using namespace std;

void init(DB **db_ptr2, Options& op, WriteOptions& write_op, ReadOptions& read_op, int max_background_jobs,
          EmuEnv* _env, string kDBPath){
  rocksdb::SetPerfLevel(rocksdb::PerfLevel::kEnableTimeExceptForMutex);
  
  checking::SystemVerifier::init();


  // Check if the directory exists
  if (std::system(("test -d " + kDBPath).c_str()) == 0) {
      // Remove the directory
      if (std::system(("rm -rf " + kDBPath).c_str()) == 0) {
          std::cout << "Existing directory removed: " << kDBPath << std::endl;
      } else {
          std::cerr << "Error removing directory" << std::endl;
      }
  } else {
      std::cout << "Directory does not exist: " << kDBPath << std::endl;
  }

  // Create the directory
  if (std::system(("mkdir -p " + kDBPath).c_str()) == 0) {
      std::cout << "Directory created: " << kDBPath << std::endl;
  } else {
      std::cerr << "Error creating directory" << std::endl;
  }

  // options are set in configOptions()
  // setNewBlockCacheForReading(op);
  // setNoBlockCacheForReading(op);

  clearCache(op);
}





void reopen_DB(DB** db_ptr2, Options& op,  WriteOptions& write_op, ReadOptions& read_op, EmuEnv* _env, string kDBPath){
  Status s = (*db_ptr2)->Close();
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());

  DB* db;
  db_ptr2 = &db;

  int max_background_jobs = 1;
  init(db_ptr2, op, write_op, read_op, max_background_jobs, _env, kDBPath);
  
  // Options options;
  // WriteOptions write_op;
  // ReadOptions read_op;
  BlockBasedTableOptions table_options;
  FlushOptions f_options;
  configOptions(_env, &op, &table_options, &write_op, &read_op, &f_options);

  s = DB::Open(op, kDBPath, db_ptr2);
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());

  string value = "value";
  string key = "000000000000";
  string time_stamp = "00000";
  s = db->Put(write_op, key, value + "|" + time_stamp);
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());
  s = db->Get(read_op, key, &value);
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());

}


void start(DB** db_ptr2, Options& op,  WriteOptions& write_op, ReadOptions& read_op, EmuEnv* _env, string kDBPath){
  string &workload_file_name = _env->workload_file_name;

  Status s = DB::Open(op, kDBPath, db_ptr2);
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());
}



void end(DB** db_ptr2){
  DB* db = *db_ptr2;
  Status s = db->Close();
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());
  delete db;
  std::cout << "\n----------------------Closing DB-----------------------"
            << std::endl;
}



#endif // UTILS_DB_H