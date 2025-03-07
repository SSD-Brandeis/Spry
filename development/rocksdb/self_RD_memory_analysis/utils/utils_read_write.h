#ifndef UTILS_READ_WRITE_H
#define UTILS_READ_WRITE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <sstream>

// Template function to write map to a file
template <typename T1, typename T2>
void writeDictToFile(const std::map<T1, T2>& m, const std::string& file_name) {
    std::ofstream out_file(file_name);
    if (!out_file) {
        std::cerr << "Error opening file for writing: " << file_name << std::endl;
        return;
    }
    
    for (const auto& [k, v] : m) {
        out_file << k << ":" << v << std::endl;
    }
    
    out_file.close();
}

// Template function to read map from a file
template <typename T1, typename T2>
std::map<T1, T2> readDictFromFile(const std::string& file_name) {
    std::map<T1, T2> m;
    std::ifstream in_file(file_name);
    if (!in_file) {
        std::cerr << "Error opening file for reading: " << file_name << std::endl;
        return m;
    }

    std::string line;
    while (std::getline(in_file, line)) {
        std::stringstream ss(line);
        std::string key_str, value_str;

        // Split the line into key and value
        if (std::getline(ss, key_str, ':') && std::getline(ss, value_str)) {
            std::stringstream key_stream(key_str);
            std::stringstream value_stream(value_str);

            T1 key;
            T2 value;

            key_stream >> key;
            value_stream >> value;

            m[key] = value;
        }
    }

    in_file.close();
    return m;
}

// Function to compare two maps
template <typename T1, typename T2>
bool compareMaps(std::map<T1, T2>& m1, std::map<T1, T2>& m2) {
    for(auto &[k,v]:m1){
        if(m2.count(k) == 0){
            std::cout << "k = " << k << " v = " << v << " v_read = " << m2[k] << std::endl;
            return false;
        }
        if(v != m2[k]){
            std::cout << "k = " << k << " v = " << v << " v_read = " << m2[k] << std::endl;
            return false;
        }
    }
    for(auto &[k,v]:m2){
        if(m1.count(k) == 0){
            std::cout << "k = " << k << " v_read = " << v << " v = " << m1[k] << std::endl;
            return false;
        }
        if(v != m1[k]){
            std::cout << "k = " << k << " v_read = " << v << " v = " << m1[k] << std::endl;
            return false;
        }
    }
    return true;
    // return m1 == m2;  // Simple map comparison
}

// Template function to write vector to a file
template <typename T>
void writeVectorToFile(const std::vector<T>& v, const std::string& file_name) {
    std::ofstream out_file(file_name);
    if (!out_file) {
        std::cerr << "Error opening file for writing: " << file_name << std::endl;
        return;
    }
    
    for (const auto& element : v) {
        out_file << element << std::endl;  // Each element on a new line
    }
    
    out_file.close();
}

// Template function to read vector from a file
template <typename T>
std::vector<T> readVectorFromFile(const std::string& file_name) {
    std::vector<T> v;
    std::ifstream in_file(file_name);
    if (!in_file) {
        std::cerr << "Error opening file for reading: " << file_name << std::endl;
        return v;
    }

    std::string line;
    while (std::getline(in_file, line)) {
        std::stringstream ss(line);
        T element;
        ss >> element;  // Convert the line to the appropriate type
        v.push_back(element);
    }

    in_file.close();
    return v;
}

// Function to compare two vectors
template <typename T>
bool compareVectors(const std::vector<T>& v1, const std::vector<T>& v2) {
    return v1 == v2;  // Simple vector comparison
}

#endif //UTILS_READ_WRITE_H