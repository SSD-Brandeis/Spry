#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "rocksdb/statistics.h"
#include "rocksdb/advanced_options.h"
#include "rocksdb/cache.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"
#include "rocksdb/table.h"

#include "rocksdb/system_verifier.h"

using namespace rocksdb;
std::string kDBPath = "/tmp/cs561_project1";

inline void showProgress(const uint64_t& workload_size,
                         const uint64_t& counter) {
  if (counter / (workload_size / 100) >= 1) {
    for (int i = 0; i < 104; i++) {
      std::cout << "\b";
      fflush(stdout);
    }
  }
  for (int i = 0; i < counter / (workload_size / 100); i++) {
    std::cout << "=";
    fflush(stdout);
  }
  std::cout << std::setfill(' ')
            << std::setw(101 - counter / (workload_size / 100));
  std::cout << counter * 100 / workload_size << "%";
  fflush(stdout);

  if (counter == workload_size) {
    std::cout << "\n";
    return;
  }
}

void printStats(DB* db, Options& options) {
    std::string each_level_stats;
    std::string sst_file_size;

    bool result = db->GetProperty("rocksdb.levelstats", &each_level_stats);
    bool live_sst_file_size = db->GetProperty("rocksdb.live-sst-files-size", &sst_file_size);

    std::cout << std::endl;
    std::cout << "Level Statistics" << std::endl;

    if (result){
        std::cout << "Level, Total Files, " << each_level_stats << std::endl;  // printing level stats
    }
    if (live_sst_file_size) {
        std::cout << "Total SST Files Size : " << sst_file_size << std::endl;  // printing sst file size
    }
    std::cout << "----------------------------------------" << std::endl;

    std::cout << std::endl;
    std::cout << "RocksDB Statistics : " << std::endl;
    std::cout << "----------------------------------------" << std::endl;
}

void init(){

  checking::SystemVerifier::init();
}


void runWorkload(Options& op, WriteOptions& write_op, ReadOptions& read_op) {
  DB* db;

  op.write_buffer_size = 8 * 4;
  op.level0_file_num_compaction_trigger = 1;
  op.target_file_size_base = 8 * 4;
  op.target_file_size_multiplier = 1;  // Same files size across levels
  op.max_write_buffer_number = 1;      // 1 buffer in-memory
  op.max_bytes_for_level_base =
      op.write_buffer_size;               // same as write buffer size
  op.max_bytes_for_level_multiplier = 2;  // T-ratio
  op.statistics = CreateDBStatistics();
  op.create_if_missing = true;
  // op.write_buffer_size = 8 * 1024 * 1024;

  {
    op.memtable_factory =
        std::shared_ptr<VectorRepFactory>(new VectorRepFactory);
    op.allow_concurrent_memtable_write = false;
  }

  {
      // op.memtable_factory = std::shared_ptr<SkipListFactory>(new
      // SkipListFactory);
  }

  {
      // op.memtable_factory =
      // std::shared_ptr<MemTableRepFactory>(NewHashSkipListRepFactory());
      // op.allow_concurrent_memtable_write = false;
  }

  {
    // op.memtable_factory =
    // std::shared_ptr<MemTableRepFactory>(NewHashLinkListRepFactory());
    // op.allow_concurrent_memtable_write = false;
  }

  // BlockBasedTableOptions table_options;
  // table_options.block_cache = NewLRUCache(8*1048576);
  // op.table_factory.reset(NewBlockBasedTableFactory(table_options));

  Status s = DB::Open(op, kDBPath, &db);
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());

  // opening workload file for the first time
  std::ifstream workload_file;
  workload_file.open("workload.txt");
  assert(workload_file);
  // doing a first pass to get the workload size
  uint64_t workload_size = 0;
  std::string line;
  while (std::getline(workload_file, line)) ++workload_size;
  workload_file.close();

  workload_file.open("workload.txt");
  assert(workload_file);

  // Clearing the system cache
  std::cout << "Clearing system cache ..." << std::endl;
  int clean_flag = system("sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'");
  if (clean_flag) {
    std::cerr << "Cannot clean the system cache" << std::endl;
    exit(0);
  }

  checking::SystemVerifier* system_verifier = checking::SystemVerifier::getSystemVerifier();

  Iterator* it = db->NewIterator(read_op);  // for range reads
  uint64_t counter = 0;                     // for progress bar
  int KEY_SIZE = 12;

  while (!workload_file.eof()) {
    char instruction;
    long long key, start_key, end_key;
    std::string type;
    std::string value;
    std::stringstream ss_key, ss_start_key, ss_end_key;
    workload_file >> instruction;
    switch (instruction) {
      case 'I':  // insert
        workload_file >> key >> value;

        system_verifier->insert(key, value);

        // std::cout << "Insert " << key << std::endl;
        ss_key << std::setfill('0') << std::setw(KEY_SIZE) << key;
        // std::cout << "Insert " <<  ss_key.str() << std::endl;
        // Put key-value
        s = db->Put(write_op, ss_key.str(), value);
        if (!s.ok()) std::cerr << s.ToString() << std::endl;
        assert(s.ok());
        counter++;
        break;

      case 'Q':  // probe: point query
        workload_file >> key;

        // bool gt_is_exist = system_verifier->isKeyExist(key);
        // std::string gt_value = system_verifier->get(key);

        std::cout << "Query " << key << std::endl;
        ss_key << std::setfill('0') << std::setw(KEY_SIZE) << key;
        s = db->Get(read_op, ss_key.str(), &value);
        // if (!s.ok()) std::cerr << s.ToString() << "key = " << key <<
        // std::endl;
        //  assert(s.ok());
        counter++;
        break;

      case 'S':  // scan: range query
        workload_file >> start_key >> end_key;


        it->Refresh();
        assert(it->status().ok());
        ss_start_key << std::setfill('0') << std::setw(KEY_SIZE) << start_key;
        ss_end_key << std::setfill('0') << std::setw(KEY_SIZE) << end_key;
        for (it->Seek(ss_start_key.str()); it->Valid(); it->Next()) {
          // std::cout << "found key = " << it->key().ToString() << std ::endl;
          if (it->key().ToString() == ss_end_key.str()) {
            break;
          }
        }
        if (!it->status().ok()) {
          std::cerr << it->status().ToString() << std::endl;
        }
        counter++;
        break;

      case 'D':  // delete
        workload_file >> type >> start_key >> end_key;
        if (type == "Range") {

          system_verifier->rangeDelete(start_key, end_key);


          ss_start_key << std::setfill('0') << std::setw(KEY_SIZE) << start_key;
          ss_end_key << std::setfill('0') << std::setw(KEY_SIZE) << end_key;
          s = db->DeleteRange(write_op, db->DefaultColumnFamily(),
                              ss_start_key.str(), ss_end_key.str());
          if (!s.ok()) std::cerr << s.ToString() << std::endl;
          assert(s.ok());
          counter++;
        } else {
          std::cerr << "ERROR: Case match NOT found !!" << std::endl;
        std::cerr << "instruction = " << instruction << std::endl;
          std:cerr << "type = " << type << std::endl;
          std::cerr << "start_key = " << start_key << std::endl;
          std::cerr << "end_key = " << end_key << std::endl;
          break;
        }
        break;

      default:
        std::cerr << "ERROR: Case match NOT found !!" << std::endl;
        std::cerr << "instruction = " << instruction << std::endl;
        break;
    }

    if (workload_size < 100) workload_size = 100;
    if (counter % (workload_size / 100) == 0) {
      showProgress(workload_size, counter);
    }
  }


  std::vector<long long> testing_key_list({2500, 5000, 5001});

  for(auto &x: testing_key_list){
    bool gt_is_exist = system_verifier->isKeyExist(x);
    std::string gt_value = system_verifier->get(x);

    std::string value;
    std::stringstream searching_key;
    searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;
    s = db->Get(read_op, searching_key.str(), &value);
    std::cout << x << " " << s.ok() << " " << value << std::endl;
    std::cout << x << " " << gt_is_exist << " " << gt_value << std::endl;
  
    if(s.ok() != gt_is_exist){
      std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
    }
    if(gt_is_exist == false){continue;}
    if(value != gt_value){
      std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
    }
  }

  std::cout << std::endl << std::endl;
  std::cout << "----------------------Testing On Existing Keys-----------------------" << std::endl; 
  for(auto &x: system_verifier->getAllExistingKeys()){
    bool gt_is_exist = system_verifier->isKeyExist(x);
    std::string gt_value = system_verifier->get(x);

    std::string value;
    std::stringstream searching_key;
    searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;
    s = db->Get(read_op, searching_key.str(), &value);
    // std::cout << x << " " << s.ok() << " " << value << std::endl;
    // std::cout << x << " " << gt_is_exist << " " << gt_value << std::endl;
  
    if(s.ok() != gt_is_exist){
      std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
    }
    if(gt_is_exist == false){continue;}
    if(value != gt_value){
      std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
    }
  }

  std::cout << std::endl << std::endl;
  std::cout << "----------------------Testing On historic-existing Keys-----------------------" << std::endl;
  for(auto &x: system_verifier->getHistoricExistingKeys()){
    bool gt_is_exist = system_verifier->isKeyExist(x);
    std::string gt_value = system_verifier->get(x);

    std::string value;
    std::stringstream searching_key;
    searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;
    s = db->Get(read_op, searching_key.str(), &value);
    // std::cout << x << " " << s.ok() << " " << value << std::endl;
    // std::cout << x << " " << gt_is_exist << " " << gt_value << std::endl;
  
    if(s.ok() != gt_is_exist){
      std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
    }
    if(gt_is_exist == false){continue;}
    if(value != gt_value){
      std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
    }
  }

  std::cout << std::endl << std::endl;
  std::cout << "----------------------Testing On Currently Deleted Keys-----------------------" << std::endl;
  for(auto &x: system_verifier->getCurrentlyDeletedKeys()){
    bool gt_is_exist = system_verifier->isKeyExist(x);
    std::string gt_value = system_verifier->get(x);

    std::string value;
    std::stringstream searching_key;
    searching_key << std::setfill('0') << std::setw(KEY_SIZE) << x;
    s = db->Get(read_op, searching_key.str(), &value);
    // std::cout << x << " " << s.ok() << " " << value << std::endl;
    // std::cout << x << " " << gt_is_exist << " " << gt_value << std::endl;
  
    if(s.ok() != gt_is_exist){
      std::cout << "ERROR (Existence inconsistency): " << x << " (result, gt_result) " << s.ok() << " " << gt_is_exist << std::endl;
    }
    if(gt_is_exist == false){continue;}
    if(value != gt_value){
      std::cout << "ERROR (Value inconsistency): " << x << " (value, gt_value) " << value << " " << gt_value << std::endl;
    }
  }

  std::cout << std::endl << std::endl;
  std::cout << "----------------------End Testing-----------------------" << std::endl;





  workload_file.close();
  printStats(db, op);
  s = db->Close();
  if (!s.ok()) std::cerr << s.ToString() << std::endl;
  assert(s.ok());
  delete db;
  std::cout << "\n----------------------Closing DB-----------------------"
            << std::endl;

  return;
}

int main() {
  init();

  Options options;
  WriteOptions write_op;
  ReadOptions read_op;
  runWorkload(options, write_op, read_op);
}
