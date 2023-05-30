/*
 *  Created on: May 15, 2019
 *  Author: Subhadeep
 */

#include <sstream>
#include <iostream>
#include <cstdio>
#include <sys/time.h>
#include <cmath>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <fstream>
#include <stdlib.h>

#include "args.hxx"
#include "workload_generator.h"

using namespace std;


/*
 * DEFINITIONS 
 * 
 */

long long WorkloadGenerator::KEY_DOMAIN_SIZE = 100000000;
vector < long > WorkloadGenerator::inserted_keys;

int WorkloadGenerator::generateWorkload(long long insert_count, long entry_size, int correlation, 
         long range_delete_count) {
  
  ofstream workload_file;
  workload_file.open("workload.txt");

  //srand(time(0));
  string sortkey, deletekey;
  long i_insert = 0;
  long i_delete = 0;
  long insert_group_size = 7;
  long delete_group_size = 3;

  while( (i_insert < insert_count) || (i_delete < range_delete_count) ){
    
    for (; i_delete<range_delete_count; ++i_delete) {
      long start = rand() %  KEY_DOMAIN_SIZE;
      long end = rand() %  KEY_DOMAIN_SIZE;
      if(end < start + 10000){end = start+10000;}

      workload_file << "RD " << std::to_string(start) << " " << std::to_string(end) << std::endl;
      if((i_delete+1) % delete_group_size == 0){i_delete++; break;}
    }
    
    for (; i_insert<insert_count; ++i_insert) {
      if (correlation == 0){
        sortkey = generateKey();
      }else{
        sortkey = std::to_string(i_insert + 1);
      }

      deletekey = std::to_string(i_insert + 1);

      long value_size = entry_size - 2*sizeof(long);
      string value = generateValue(value_size);
      workload_file << "I " << sortkey << " " << deletekey << " " << value << std::endl;
      if( (i_insert+1) % insert_group_size == 0){i_insert++; break;}
    }
    
    
  }

  workload_file.close();

  return 1;
}

int WorkloadGenerator::generateWorkload(long long insert_count, long entry_size, int correlation, 
         long range_delete_count, double selectivity, int numberOfInsertInTheBeginning, string workloadFilename) {
  
  ofstream workload_file;
  workload_file.open(workloadFilename);

  set<long> existingKeys;

  //srand(time(0));
  string sortkey, deletekey;
  long i_insert = 0;
  long i_delete = 0;
  // long insert_group_size = 7;
// cout << range_delete_count << endl;
  long insert_group_size = (((long)(insert_count)) - ((long)numberOfInsertInTheBeginning))/range_delete_count;
  assert(insert_group_size > 0);
// cout << insert_group_size << endl;
  // long delete_group_size = 3;
  long delete_group_size = 1;

  for(; i_insert < numberOfInsertInTheBeginning; i_insert++){
      if (correlation == 0){
        sortkey = generateKey();
      }else{
        sortkey = std::to_string(i_insert + 1);
      }

      deletekey = std::to_string(i_insert + 1);

      long value_size = entry_size - 2*sizeof(long);
      string value = generateValue(value_size);
      workload_file << "I " << sortkey << " " << deletekey << " " << value << std::endl;
      existingKeys.insert(stol(sortkey));
  }

  while( (i_insert < insert_count) || (i_delete < range_delete_count) ){
    
    for (; i_delete<range_delete_count; ++i_delete) {
      using pll = pair<long, long>;
      int totPoint = insert_count;
      int numN = 1;
      int currentExistingCount = existingKeys.size();
      int widthN = ceil(insert_count*selectivity);
      if(currentExistingCount < ceil(insert_count*selectivity) ){
        if(i_insert != insert_count){break;}
          
        cout << "Error: Not enough insert to generate range delete workload" << endl;
        exit(1);
      }

      auto its = existingKeys.begin();
      auto ite = existingKeys.end();
      auto it1 = its;
      auto it2 = its;
      int displacement = 0;
      if(widthN < currentExistingCount){
        displacement = ((int) rand()) % (currentExistingCount-widthN);
      }
      for(int i = 0; i < displacement; i++){ it1++; }
      it2 = it1;
      for(int i = 0; i < widthN-1; i++){ it2++; }
      it2++;
        
      long start = *it1;
      long end = *it2;


      workload_file << "RD " << std::to_string(start) << " " << std::to_string(end) << std::endl;
      existingKeys.erase(it1, it2);
      if((i_delete+1) % delete_group_size == 0){i_delete++; break;}
    }
    
    for (; i_insert<insert_count; ++i_insert) {
      if (correlation == 0){
        sortkey = generateKey();
      }else{
        sortkey = std::to_string(i_insert + 1);
      }

      deletekey = std::to_string(i_insert + 1);

      long value_size = entry_size - 2*sizeof(long);
      string value = generateValue(value_size);
      workload_file << "I " << sortkey << " " << deletekey << " " << value << std::endl;
      existingKeys.insert(stol(sortkey));
      if( (i_insert+1) % insert_group_size == 0){i_insert++; break;}
    }
    
    
  }

  workload_file.close();

  return 1;
}








string WorkloadGenerator::generateKey() {
  unsigned long randomness = rand() %  KEY_DOMAIN_SIZE;
  WorkloadGenerator::inserted_keys.push_back(randomness);
  string key = std::to_string(randomness);
  return key;
}

string WorkloadGenerator::generateValue(long value_size) {
  string value = std::string(value_size, 'a' + (rand() % 26));
  return value;
}

