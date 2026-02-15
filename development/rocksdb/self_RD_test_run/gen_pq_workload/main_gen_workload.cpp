#include "../utils/utils_gen_PQ_workload.h"
#include "../utils/utils_read_write.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <sstream>

using namespace checking;
SystemVerifier* SystemVerifier::system_verifier;




int main(int argc, char *argv[]) {

  auto start_time = std::chrono::high_resolution_clock::now();
  auto stop_time = std::chrono::high_resolution_clock::now();
  auto duration_time = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_time - start_time);
  unsigned long long gen_PQ_workload_time_ns = 0;

  // check emu_environment.h for the contents of EmuEnv and also the definitions of the singleton experimental environment 
  EnvGenPQ env_gen_pq;
  checking::SystemVerifier *system_verifier = checking::SystemVerifier::getSystemVerifier();
  
  //parse the command line arguments
  if (parse_arguments(argc, argv, system_verifier, &env_gen_pq)){
    exit(1);
  }

  runInsertRDWorkload(env_gen_pq.workload_file_name);

  // int KEY_SIZE = checking::SystemVerifier::getSystemVerifier()->getKeySize();
  // system_verifier->setRDFTypes(_env->RDFTypes);
  const long long N_repetitions = checking::SystemVerifier::EXPERIMENT_REPETITION_TIMES;
//   int number_of_PQs = env_gen_pq.number_of_PQs;
  int number_of_PQs_on_existing_keys = env_gen_pq.number_of_PQs_on_existing_keys;
  int number_of_PQs_on_historic_existing_keys = env_gen_pq.number_of_PQs_on_historic_existing_keys;
  int number_of_PQs_on_currently_deleted_keys = env_gen_pq.number_of_PQs_on_currently_deleted_keys;
  int number_of_PQs_on_currently_non_inserted_keys = env_gen_pq.number_of_PQs_on_currently_non_inserted_keys;

  std::string prefix_number_of_PQs = "";
    //   if(number_of_PQs != -1){
    //     // prefix_number_of_PQs = "fixed #PQ = " + std::to_string(number_of_PQs);
    //   }
    // system_verifier->gen_workload_with_numbers_of_PQ(N_repetitions, number_of_PQs);
  system_verifier->gen_workload_with_numbers_of_PQ(N_repetitions, 
    number_of_PQs_on_existing_keys, number_of_PQs_on_historic_existing_keys,
    number_of_PQs_on_currently_deleted_keys, number_of_PQs_on_currently_non_inserted_keys);

  
  map<string, string> ground_truth = system_verifier->getGroundTruth();
  vector<string>  historic_existing_keys = system_verifier->getHistoricExistingKeys();
  vector<string>  currently_non_inserted_keys = system_verifier->getCurrentlyNonInsertedKeys();


  
  const string pq_workload_ground_truth_file_name = env_gen_pq.workload_file_name + "_ground_truth";
  const string pq_workload_historic_existing_file_name = env_gen_pq.workload_file_name + "_historic_existing";
  const string pq_workload_currently_non_existed_file_name = env_gen_pq.workload_file_name + "_currently_non_existed";
  {
    // for(auto &[k, v]: ground_truth){
    //   std::cout << k << " " << v << std::endl;
    // }

    // Write the map to a file
    writeDictToFile(ground_truth, pq_workload_ground_truth_file_name);

    // for(auto &k: historic_existing_keys){
    //   std::cout << k << std::endl;
    // }

    // Write the vector to a file
    writeVectorToFile(historic_existing_keys, pq_workload_historic_existing_file_name);


    // for(auto &k: currently_non_inserted_keys){
    //   std::cout << k << std::endl;
    // }

    // Write the vector to a file
    writeVectorToFile(currently_non_inserted_keys, pq_workload_currently_non_existed_file_name);
  }


  {
    // Read the map back from the file
    std::map<std::string, std::string> read_back_map = readDictFromFile<std::string, std::string>(pq_workload_ground_truth_file_name);

    // Compare the original and read-back maps
    if (compareMaps(ground_truth, read_back_map)) {
        std::cout << "Maps are identical!" << std::endl;
    } else {
        std::cout << "Maps are different!" << std::endl;
        for(auto &[k,v]: ground_truth){
            if(v != read_back_map[k]){
                std::cout << "k = " << k << " v = " << v << " v_read = " << read_back_map[k] << std::endl;
            }
        }
    }

    // Read the vector back from the file
    std::vector<std::string> read_back_vector = readVectorFromFile<std::string>(pq_workload_historic_existing_file_name);

    // Compare the original and read-back vectors
    if (compareVectors(historic_existing_keys, read_back_vector)) {
        std::cout << "Vectors are identical!" << std::endl;
    } else {
        std::cout << "Vectors are different!" << std::endl;
    }

    // Read the vector back from the file
    std::vector<std::string> read_back_vector2 = readVectorFromFile<std::string>(pq_workload_currently_non_existed_file_name);

    // Compare the original and read-back vectors
    if (compareVectors(currently_non_inserted_keys, read_back_vector2)) {
        std::cout << "Vectors are identical!" << std::endl;
    } else {
        std::cout << "Vectors are different!" << std::endl;
    }
  }

  //All keys
  {
    const string pq_workload_all_existing_keys_file_name = env_gen_pq.workload_file_name + "_all_existing_keys";
    const string pq_workload_historic_existing_keys_file_name = env_gen_pq.workload_file_name + "_historic_existing_keys";
    const string pq_workload_currently_deleted_keys_file_name = env_gen_pq.workload_file_name + "_currently_deleted_keys";
    const string pq_workload_currently_non_inserted_keys_file_name = env_gen_pq.workload_file_name + "_currently_non_inserted_keys";
    
    vector<std::string> workload_all_existing_keys = system_verifier->getAllExistingKeys();
    vector<std::string> workload_historic_existing_keys = system_verifier->getHistoricExistingKeys();
    vector<std::string> workload_currently_deleted_keys = system_verifier->getCurrentlyDeletedKeys();
    vector<std::string> workload_currently_non_inserted_keys = system_verifier->getCurrentlyNonInsertedKeys(); 
  
    // Write the vector to a file
    writeVectorToFile(workload_all_existing_keys, pq_workload_all_existing_keys_file_name);
    
    // Write the vector to a file
    writeVectorToFile(workload_historic_existing_keys, pq_workload_historic_existing_keys_file_name);

    // Write the vector to a file
    writeVectorToFile(workload_currently_deleted_keys, pq_workload_currently_deleted_keys_file_name);

    // Write the vector to a file
    writeVectorToFile(workload_currently_non_inserted_keys, pq_workload_currently_non_inserted_keys_file_name);
  }

  
  {
    const string pq_workload_all_existing_keys_file_name = env_gen_pq.workload_file_name + "_all_existing_keys";
    const string pq_workload_historic_existing_keys_file_name = env_gen_pq.workload_file_name + "_historic_existing_keys";
    const string pq_workload_currently_deleted_keys_file_name = env_gen_pq.workload_file_name + "_currently_deleted_keys";
    const string pq_workload_currently_non_inserted_keys_file_name = env_gen_pq.workload_file_name + "_currently_non_inserted_keys";
    
    vector<std::string> workload_all_existing_keys = system_verifier->getAllExistingKeys();
    vector<std::string> workload_historic_existing_keys = system_verifier->getHistoricExistingKeys();
    vector<std::string> workload_currently_deleted_keys = system_verifier->getCurrentlyDeletedKeys();
    vector<std::string> workload_currently_non_inserted_keys = system_verifier->getCurrentlyNonInsertedKeys(); 
  
    
    // Read the vector back from the file
    vector<std::string> read_back_vector = readVectorFromFile<std::string>(pq_workload_all_existing_keys_file_name);

    // Compare the original and read-back vectors
    if (compareVectors(workload_all_existing_keys, read_back_vector)) {
        std::cout << "Vectors are identical!" << std::endl;
    } else {
        std::cout << "Vectors are different!" << std::endl;
    }

    
    // Read the vector back from the file
    read_back_vector = readVectorFromFile<std::string>(pq_workload_historic_existing_keys_file_name);

    // Compare the original and read-back vectors
    if (compareVectors(workload_historic_existing_keys, read_back_vector)) {
        std::cout << "Vectors are identical!" << std::endl;
    } else {
        std::cout << "Vectors are different!" << std::endl;
    }
   
    // Read the vector back from the file
    read_back_vector = readVectorFromFile<std::string>(pq_workload_currently_deleted_keys_file_name);

    // Compare the original and read-back vectors
    if (compareVectors(workload_currently_deleted_keys, read_back_vector)) {
        std::cout << "Vectors are identical!" << std::endl;
    } else {
        std::cout << "Vectors are different!" << std::endl;
    }


    // Read the vector back from the file
    read_back_vector = readVectorFromFile<std::string>(pq_workload_currently_non_inserted_keys_file_name);

    // Compare the original and read-back vectors
    if (compareVectors(workload_currently_non_inserted_keys, read_back_vector)) {
        std::cout << "Vectors are identical!" << std::endl;
    } else {
        std::cout << "Vectors are different!" << std::endl;
    }
  }


  // #PQ = fixed
  for(int i = 0; i < N_repetitions; i++){
    const string pq_workload_all_existing_keys_file_name = env_gen_pq.workload_file_name + "_all_existing_keys" + "_round_" + to_string(i) + "_number_of_pq_" + to_string(number_of_PQs_on_existing_keys);
    const string pq_workload_historic_existing_keys_file_name = env_gen_pq.workload_file_name + "_historic_existing_keys" + "_round_" + to_string(i) + "_number_of_pq_" + to_string(number_of_PQs_on_historic_existing_keys);
    const string pq_workload_currently_deleted_keys_file_name = env_gen_pq.workload_file_name + "_currently_deleted_keys" + "_round_" + to_string(i) + "_number_of_pq_" + to_string(number_of_PQs_on_currently_deleted_keys);
    const string pq_workload_currently_non_inserted_keys_file_name = env_gen_pq.workload_file_name + "_currently_non_inserted_keys" + "_round_" + to_string(i) + "_number_of_pq_" + to_string(number_of_PQs_on_currently_non_inserted_keys);
    
    vector<std::string> workload_all_existing_keys = system_verifier->getAllExistingKeysAtNRound(i);
    vector<std::string> workload_historic_existing_keys = system_verifier->getHistoricExistingKeysAtNRound(i);
    vector<std::string> workload_currently_deleted_keys = system_verifier->getCurrentlyDeletedKeysAtNRound(i);
    vector<std::string> workload_currently_non_inserted_keys = system_verifier->getCurrentlyNonInsertedKeysAtNRound(i); 
  
    
    // Write the vector to a file
    writeVectorToFile(workload_all_existing_keys, pq_workload_all_existing_keys_file_name);

    // Write the vector to a file
    writeVectorToFile(workload_historic_existing_keys, pq_workload_historic_existing_keys_file_name);
    
    // Write the vector to a file
    writeVectorToFile(workload_currently_deleted_keys, pq_workload_currently_deleted_keys_file_name);

    // Write the vector to a file
    writeVectorToFile(workload_currently_non_inserted_keys, pq_workload_currently_non_inserted_keys_file_name);
  }

  // #PQ = fixed
  for(int i = 0; i < N_repetitions; i++){
    const string pq_workload_all_existing_keys_file_name = env_gen_pq.workload_file_name + "_all_existing_keys" + "_round_" + to_string(i) + "_number_of_pq_" + to_string(number_of_PQs_on_existing_keys);
    const string pq_workload_historic_existing_keys_file_name = env_gen_pq.workload_file_name + "_historic_existing_keys" + "_round_" + to_string(i) + "_number_of_pq_" + to_string(number_of_PQs_on_historic_existing_keys);
    const string pq_workload_currently_deleted_keys_file_name = env_gen_pq.workload_file_name + "_currently_deleted_keys" + "_round_" + to_string(i) + "_number_of_pq_" + to_string(number_of_PQs_on_currently_deleted_keys);
    const string pq_workload_currently_non_inserted_keys_file_name = env_gen_pq.workload_file_name + "_currently_non_inserted_keys" + "_round_" + to_string(i) + "_number_of_pq_" + to_string(number_of_PQs_on_currently_non_inserted_keys);
    
    vector<std::string> workload_all_existing_keys = system_verifier->getAllExistingKeysAtNRound(i);
    vector<std::string> workload_historic_existing_keys = system_verifier->getHistoricExistingKeysAtNRound(i);
    vector<std::string> workload_currently_deleted_keys = system_verifier->getCurrentlyDeletedKeysAtNRound(i);
    vector<std::string> workload_currently_non_inserted_keys = system_verifier->getCurrentlyNonInsertedKeysAtNRound(i); 
  
    // Read the vector back from the file
     vector<std::string> read_back_vector = readVectorFromFile<std::string>(pq_workload_all_existing_keys_file_name);

    // Compare the original and read-back vectors
    if (compareVectors(workload_all_existing_keys, read_back_vector)) {
        std::cout << "Vectors are identical!" << std::endl;
    } else {
        std::cout << "Vectors are different!" << std::endl;
    }

    // Read the vector back from the file
    read_back_vector = readVectorFromFile<std::string>(pq_workload_historic_existing_keys_file_name);

    // Compare the original and read-back vectors
    if (compareVectors(workload_historic_existing_keys, read_back_vector)) {
        std::cout << "Vectors are identical!" << std::endl;
    } else {
        std::cout << "Vectors are different!" << std::endl;
    }

    // Read the vector back from the file
    read_back_vector = readVectorFromFile<std::string>(pq_workload_currently_deleted_keys_file_name);

    // Compare the original and read-back vectors
    if (compareVectors(workload_currently_deleted_keys, read_back_vector)) {
        std::cout << "Vectors are identical!" << std::endl;
    } else {
        std::cout << "Vectors are different!" << std::endl;
    }
    
    // Read the vector back from the file
    read_back_vector = readVectorFromFile<std::string>(pq_workload_currently_non_inserted_keys_file_name);

    // Compare the original and read-back vectors
    if (compareVectors(workload_currently_non_inserted_keys, read_back_vector)) {
        std::cout << "Vectors are identical!" << std::endl;
    } else {
        std::cout << "Vectors are different!" << std::endl;
    }
  }

  
  stop_time = std::chrono::high_resolution_clock::now();
  duration_time = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_time - start_time);
  gen_PQ_workload_time_ns = duration_time.count();
  std::cout << "gen_PQ_workload_time_ns = " << gen_PQ_workload_time_ns << std::endl;
}

