#include <iostream>
#include <vector>
#include <string>
#include <assert.h>
#include <algorithm>
#include <utility>


#include "include/surf.hpp"

using namespace surf;

static bool isEqual(const std::string& a, const std::string& b, const unsigned bitlen) {
    if (bitlen == 0) {
	return (a.compare(b) == 0);
    } else {
	std::string a_prefix = a.substr(0, a.length() - 1);
	std::string b_prefix = b.substr(0, b.length() - 1);
	if (a_prefix.compare(b_prefix) != 0) return false;
	char mask = 0xFF << (8 - bitlen);
	char a_suf = a[a.length() - 1] & mask;
	char b_suf = b[b.length() - 1] & mask;
	return (a_suf == b_suf);
    }
}

// void testMoveToKeyGreaterThanWord(SuRF* surf_, std::vector<std::string> words, surf::SuffixType kSuffixType){

// 	    bool inclusive = true;
// 	    for (int i = 0; i < 2; i++) {
//             if (i == 1){
//                 inclusive = false;
//             }
//             for (int j = -1; j <= (int)words.size(); j++) {
//                 SuRF::Iter iter;
//                 if (j < 0){
//                     iter = surf_->moveToFirst();
//                 }else if (j >= (int)words.size()){
//                     iter = surf_->moveToLast();
//                 }else{
//                     iter = surf_->moveToKeyGreaterThan(words[j], inclusive);
//                 }

//                 unsigned bitlen;
//                 bool is_prefix = false;
//                 if (j < 0) {
//                     std::cout << "inclusive: " << inclusive << " j < 0" << std::endl;
//                     // ASSERT_TRUE(iter.isValid());
//                     if(iter.isValid() == false){
//                         std::cout << "Error: " << "iter.isValid() = " << iter.isValid() 
//                             << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                     }
//                     std::string iter_key = iter.getKeyWithSuffix(&bitlen);
//                     std::string word_prefix = words[0].substr(0, iter_key.length());
//                     std::cout << "\titer_key: " << iter_key << " " << "bitlen: " << bitlen << std::endl;
//                     std::cout << "\tword_prefix: " << word_prefix << std::endl;
//                     is_prefix = isEqual(word_prefix, iter_key, bitlen);
//                     // ASSERT_TRUE(is_prefix);
//                     if(is_prefix == false){
//                         std::cout << "Error: " << "is_prefix = " << is_prefix 
//                             << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                     }
//                 } else if (j >= (int)words.size()) {
//                     std::cout << "inclusive: " << inclusive << " j >= words.size()" << std::endl;
//                     // ASSERT_TRUE(iter.isValid());
//                     if(iter.isValid() == false){
//                         std::cout << "Error: " << "iter.isValid() = " << iter.isValid()
//                             << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                     }
//                     std::string iter_key = iter.getKeyWithSuffix(&bitlen);
//                     std::string word_prefix = words[words.size() - 1].substr(0, iter_key.length());
//                     std::cout << "\titer_key: " << iter_key << " " << "bitlen: " << bitlen << std::endl;
//                     std::cout << "\tword_prefix: " << word_prefix << std::endl;
//                     is_prefix = isEqual(word_prefix, iter_key, bitlen);
//                     // ASSERT_TRUE(is_prefix);
//                     if(is_prefix == false){
//                         std::cout << "Error: " << "is_prefix = " << is_prefix 
//                             << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                     }
//                 } else if (j == (int)words.size() - 1) {
//                     std::cout << "inclusive: " << inclusive << " j == words.size() - 1" << std::endl;
//                     if (iter.getFpFlag()) {
//                         // ASSERT_TRUE(iter.isValid());
//                         if(iter.isValid() == false){
//                             std::cout << "Error: " << "iter.isValid() = " << iter.isValid() 
//                                 << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                         }
//                         std::string iter_key = iter.getKeyWithSuffix(&bitlen);
//                         std::string word_prefix = words[words.size() - 1].substr(0, iter_key.length());
//                         std::cout << "\titer_key: " << iter_key << " " << "bitlen: " << bitlen << std::endl;
//                         std::cout << "\tword_prefix: " << word_prefix << std::endl;
//                         is_prefix = isEqual(word_prefix, iter_key, bitlen);
//                         // ASSERT_TRUE(is_prefix);
//                         if(is_prefix == false){
//                             std::cout << "Error: " << "is_prefix = " << is_prefix 
//                                 << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                         }
//                     } else {
//                         // ASSERT_FALSE(iter.isValid());
//                         if(iter.isValid() == true){
//                             std::cout << "Error: " << "iter.isValid() = " << iter.isValid() 
//                                 << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                         }
//                     }
//                 } else {
//                     std::cout << "inclusive: " << inclusive << " j >= 0 && j < words.size() - 1" << std::endl;
//                     std::cout << "words[j] = " << words[j] << std::endl;
//                     // ASSERT_TRUE(iter.isValid());
//                     if(iter.isValid() == false){
//                         std::cout << "Error: " << "iter.isValid() = " << iter.isValid()
//                             << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                     }
//                     std::string iter_key = iter.getKeyWithSuffix(&bitlen);
//                     std::string word_prefix_fp = words[j].substr(0, iter_key.length());
//                     std::string word_prefix_true = words[j+1].substr(0, iter_key.length());
//                     std::cout << "\titer_key: " << iter_key << " " << "bitlen: " << bitlen << std::endl;
//                     std::cout << "\titer.getFpFlag(): " << iter.getFpFlag() << std::endl;
//                     std::cout << "\tword_prefix_fp: " << word_prefix_fp << std::endl;
//                     std::cout << "\tword_prefix_true: " << word_prefix_true << std::endl;
//                     if (iter.getFpFlag())
//                         is_prefix = isEqual(word_prefix_fp, iter_key, bitlen);
//                     else
//                         is_prefix = isEqual(word_prefix_true, iter_key, bitlen);
//                     // ASSERT_TRUE(is_prefix);
//                     if(is_prefix == false){
//                         std::cout << "Error: " << "is_prefix = " << is_prefix 
//                             << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                     }

//                     // test getKey()
//                     std::string iter_get_key = iter.getKey();
//                     std::cout << "\titer_get_key: " << iter_get_key << std::endl;   
//                     std::string iter_key_prefix = iter_key.substr(0, iter_get_key.length());

//                     iter++;
//                     if (iter.isValid()) {
//                         std::string iter_next_key = iter.getKey();
//                         std::cout << "\titer++ -> next_key: " << iter_next_key << std::endl;
//                     }

//                     is_prefix = (iter_key_prefix.compare(iter_get_key) == 0);
//                     // ASSERT_TRUE(is_prefix);
//                     if(is_prefix == false){
//                         std::cout << "Error: " << "is_prefix = " << is_prefix
//                             << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                     }


//                     // test getSuffix()
//                     if (kSuffixType == kReal || kSuffixType == kMixed) {
//                         word_t iter_suffix = 0;
//                         int iter_suffix_len = iter.getSuffix(&iter_suffix);
//                         // assert(kSuffixLenList[k] == iter_suffix_len);
//                         surf::level_t kSuffixLen = iter_suffix_len;
//                         std::cout << "iter_suffix_len: " << iter_suffix_len << std::endl;
//                         if(kSuffixLen != iter_suffix_len){
//                             std::cout << "Error: " << "kSuffixLen = " << kSuffixLen << " iter_suffix_len = " << iter_suffix_len 
//                                 << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                         }
//                         std::string iter_key_suffix_str
//                         = iter_key.substr(iter_get_key.length(), iter_key.length());
//                         word_t iter_key_suffix = 0;
//                         int suffix_len = (int)kSuffixLen;
//                         int suffix_str_len = (int)(iter_key.length() - iter_get_key.length());
//                         level_t pos = 0;
//                         while (suffix_len > 0 && suffix_str_len > 0) {
//                         iter_key_suffix += (word_t)iter_key_suffix_str[pos];
//                         iter_key_suffix <<= 8;
//                         suffix_len -= 8;
//                         suffix_str_len--;
//                         pos++;
//                         }
//                         if (pos > 0) {
//                         iter_key_suffix >>= 8;
//                         if (kSuffixLen % 8 != 0)
//                             iter_key_suffix >>= (8 - (kSuffixLen % 8));
//                         }
//                         // ASSERT_EQ(iter_key_suffix, iter_suffix);
//                         if(iter_key_suffix != iter_suffix){
//                             std::cout << "Error: " <<"iter_key_suffix = " << iter_key_suffix << " iter_suffix = " << iter_suffix 
//                                 << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                         }
//                     }
//                 }
//             }
// 	    }
// 	    // surf_->destroy();
// 	    // delete surf_;
// }

std::string int_to_bytes(int x, int len_in_bytes = 4){
    std::string  bytes(len_in_bytes, 0);
    int i = len_in_bytes - 1;
    while(x){
        assert(i >= 0);
        bytes[i] = x & 0xFF;
        x >>= 8;
        i--;
    }
    return bytes;
}

// SuRF* rangesToSurf(std::vector<std::pair<std::string, std::string>> ranges, int surf_key_length_in_bytes = 8, 
//                 surf::SuffixType kSuffixType = surf::SuffixType::kReal, 
//                 surf::level_t hash_suffix_len = 0, surf::level_t real_suffix_len = 8,
//                 bool include_dense = true, uint32_t sparse_dense_ratio = 16, 
//                 bool flag_build_until_unique = false){

//     std::sort(ranges.begin(), ranges.end());
//     int len = ranges.size();
//     for(int i = 0; i < len-1; i++){
//         assert(("cannot have overlapped, ranges[i].second shall <= ranges[i+1].first", ranges[i].second <= ranges[i+1].first));
//     }

//     std::vector<std::string> keys;
//     std::vector<bool> left_parentheses;
//     std::vector<bool> right_parentheses;
//     for(int i = 0; i < len; i++){
//         // std::string key_start = int_to_bytes(ranges[i].first, surf_key_length_in_bytes);
//         // std::string key_end = int_to_bytes(ranges[i].second, surf_key_length_in_bytes);
//         std::string key_start = ranges[i].first;
//         std::string key_end = ranges[i].second;
//         if(key_start.size() > surf_key_length_in_bytes){
//             key_start = key_start.substr(0, surf_key_length_in_bytes);
//         }
//         if(key_end.size() > surf_key_length_in_bytes){
//             key_end = key_end.substr(0, surf_key_length_in_bytes);
//         }
//         if(keys.size() > 0 && keys.back() == key_start){
//             left_parentheses.back() = true;
//         }else{
//             keys.push_back(key_start);
//             left_parentheses.push_back(true);
//             right_parentheses.push_back(false);
//         }
//         keys.push_back(key_end);
//         left_parentheses.push_back(false);
//         right_parentheses.push_back(true);
//     }
//     assert(keys.size() == left_parentheses.size());
//     assert(keys.size() == right_parentheses.size());


//     // string_length
//     int string_len = 0;
//     for(auto &key: keys){
//         if(key.length() > string_len){
//             string_len = key.length();
//         }
//     }
    

//     // bool flag_build_until_unique = false;
//     //One shall clip the key into the prefix of length "max_num_level"
//     // must >= max([len(key) for key in keys])  --- maximum key length
//     // uint16_t max_num_level = 5; 
//     uint16_t max_num_level = string_len; 
//     // bool include_dense = true;
//     // uint32_t sparse_dense_ratio = 16;
//     // uint32_t sparse_dense_ratio = 1;
//     SuRF* surf_ = new SuRF(keys, left_parentheses, right_parentheses, 
//                                 include_dense, sparse_dense_ratio,
//                                 // surf::kHash, 8, 0,
//                                 kSuffixType, hash_suffix_len, real_suffix_len,
//                                 flag_build_until_unique, max_num_level);

//     return surf_;
// }

// std::vector<std::pair<std::string, std::string>> surfToRanges(SuRF* surf_){
//     std::vector<std::string> keys;
//     std::vector<bool> left_parentheses;
//     std::vector<bool> right_parentheses;

//     SuRF::Iter iter = surf_->moveToFirst();
//     while(iter.isValid()){
//         std::string key = iter.getKey();
//         bool left_parenthesis = iter.getLeftParenthesis();
//         bool right_parenthesis = iter.getRightParenthesis();
//         keys.push_back(key);
//         left_parentheses.push_back(left_parenthesis);
//         right_parentheses.push_back(right_parenthesis);
//         iter++;
//     }

//     std::vector<std::pair<std::string, std::string>> ranges;
//     int len = keys.size();
//     for(int i = 0; i < len-1; i++){
//         if(left_parentheses[i]){
//             assert(left_parentheses[i] == true && right_parentheses[i+1] == true);
//             std::string start = keys[i];
//             std::string end = keys[i+1];
//             ranges.push_back(std::make_pair(start, end));
//         }
//     }

//     return ranges;
// }


void testRangesToSurfToRanges(){
    // std::vector<std::pair<std::string, std::string>> ranges = {
    //     {"a", "b"},
    //     {"c", "d"},
    //     {"e", "f"},
    //     {"g", "h"},
    //     {"i", "j"},
    //     {"k", "l"},
    //     {"m", "n"},
    //     {"o", "p"},
    //     {"q", "r"},
    //     {"s", "t"},
    //     {"u", "v"},
    //     {"w", "x"},
    //     {"y", "z"},
    // };
    int key_len_in_bytes = 10;
    surf::level_t hash_suffix_len = 0;
    surf::level_t real_suffix_len = 8;
    bool include_dense = true;
    uint32_t sparse_dense_ratio = 16;
    bool flag_build_until_unique = false;


    std::vector<std::string> keys;
    for(int i = 0; i < 100; i++){
        std::string key;
        for(int j = 0; j < key_len_in_bytes; j++){
            key.push_back('a' + rand() % 26);
        }
        keys.push_back(key);
    }

    sort(keys.begin(), keys.end());
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
    std::cout << "keys.size(): " << keys.size() << std::endl;

    // ranges
    std::vector<std::pair<std::string, std::string>> ranges;
    for(int i = 1; i < keys.size(); ){
        std::string start = keys[i-1];
        std::string end = keys[i];
        ranges.push_back(std::make_pair(start, end));
        i += (rand() % 2 + 1);
        // i += 2;
    }

    // query points
    std::vector<std::string> query_points;
    for(int i = 0; i < 100; i++){
        std::string key;
        int len_key = rand() % key_len_in_bytes + 1;
        for(int j = 0; j < len_key; j++){
            key.push_back('a' + rand() % 26);
        }
        query_points.push_back(key);
    }
    for(auto &key: keys){
        std::string tmp = key.substr(0, rand() % key_len_in_bytes + 1);
        query_points.push_back(tmp);
    }
    for(auto &key: keys){
        query_points.push_back(key);
    }
    for(auto &key: keys){
        std::string tmp = key + "a";
        query_points.push_back(tmp);
    }
    for(auto &key: keys){
        for(int i_c = 0; i_c < key.size(); i_c++){
            std::string tmp = key;
            tmp[i_c] = 'a' + rand() % 26;
            query_points.push_back(tmp);
        }
    }

    bool flag_allow_boundary_overlapped = false;
    // // build SuREF from ranges
    // SuRF* surf_pre = SuRF::rangesToSurf(ranges, key_len_in_bytes, surf::SuffixType::kReal, 
    //                         hash_suffix_len, real_suffix_len, include_dense, 
    //                         sparse_dense_ratio, flag_build_until_unique);
    SuRF* surf_pre = SuRF::rangesToSurf(ranges, key_len_in_bytes, surf::SuffixType::kReal, 
                            hash_suffix_len, real_suffix_len, include_dense, 
                            sparse_dense_ratio, flag_allow_boundary_overlapped);

    std::cout << "surf serialized size: " << surf_pre->serializedSize() << " bytes" << std::endl;
    std::cout << "surf memory usage: " << surf_pre->getMemoryUsage() << " bytes" << std::endl;
    // serialize
    char* buffer = surf_pre->serialize();
    // deserialize
    SuRF* surf_ = SuRF::deSerialize(buffer);

    //perform point query, and check if point falls in the range
    for(auto &key: query_points){
        auto it = lower_bound(ranges.begin(), ranges.end(), std::make_pair(key, key));
        if(it != ranges.begin()){
            it--;
        }
        bool gt_overlapping = false;
        if(it != ranges.end() && key > it->first && key < it->second){
            gt_overlapping = true;
        }else if(key == it->first && (it->first).size() < key_len_in_bytes){
            gt_overlapping = true;
        }else if(key == it->second && (it->second).size() < key_len_in_bytes){
            gt_overlapping = true;
        }
        // for key == it->first or key == it->second, cound be uncertain about whether it is overlapping
        // because the ranges may just store prefix of keys

        bool overlapping = false;
        SuRF::Iter iter = surf_->moveToNextCommonPrefixKey(key);
        if(iter.isValid()){
            std::string key_found = iter.getKey();
            if(key_found.size() < key_len_in_bytes){
                overlapping = (iter.getRightParenthesis() == true);
            }else{
                overlapping = (key_found != key) && (iter.getRightParenthesis() == true);
            }
        }
        
        if(overlapping != gt_overlapping){
            if(it != ranges.end()){
                std::cout << "it->first: " << it->first << " it->second: " << it->second << std::endl;
            }
            std::cout << "key: " << key << " overlapping: " << overlapping << " gt_overlapping: " << gt_overlapping << std::endl;
            std::cout << "iter.getLeftParenthesis(): " << iter.getLeftParenthesis() <<  " "  << " iter.getRightParenthesis(): " << iter.getRightParenthesis() << std::endl;
            assert(overlapping == gt_overlapping);
        }

    }


    //retrieve ranges from SuRf
    std::vector<std::pair<std::string, std::string>> ranges2 = SuRF::surfToRanges(surf_);
    assert(ranges == ranges2);
    std::cout << "testRangesToSurfToRanges passed" << std::endl;
}

// void testMoveToKeyGreaterThanWord(SuRF* surf_, std::vector<std::string> words, surf::SuffixType kSuffixType){

// 	    bool inclusive = true;
// 	    for (int i = 0; i < 2; i++) {
//             if (i == 1){
//                 inclusive = false;
//             }
//             for (int j = -1; j <= (int)words.size(); j++) {
//                 std::cout << std::endl << std::endl;
//                 SuRF::Iter iter;
//                 if (j < 0){
//                     iter = surf_->moveToFirst();
//                 }else if (j >= (int)words.size()){
//                     iter = surf_->moveToLast();
//                 }else{
// // std::cout << "------------ word[j] = " << words[j] << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                     iter = surf_->moveToNextCommonPrefixKey(words[j]);
//                     // iter = surf_->moveToKeyGreaterThan(words[j], inclusive);
//                 }

//                 unsigned bitlen;
//                 bool is_prefix = false;
//                 if (j < 0) {
//                     std::cout << "inclusive: " << inclusive << " j < 0" << std::endl;
//                     // ASSERT_TRUE(iter.isValid());
//                     if(iter.isValid() == false){
//                         std::cout << "Error: " << "iter.isValid() = " << iter.isValid() 
//                             << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                     }
//                     std::string iter_key = iter.getKeyWithSuffix(&bitlen);
//                     // std::string word_prefix = words[0].substr(0, iter_key.length());
//                     std::cout << "\titer_key: " << iter_key << " " << "bitlen: " << bitlen << std::endl;
//                     // std::cout << "\tword_prefix: " << word_prefix << std::endl;
//                     // is_prefix = isEqual(word_prefix, iter_key, bitlen);
//                     // // ASSERT_TRUE(is_prefix);
//                     // if(is_prefix == false){
//                     //     std::cout << "Error: " << "is_prefix = " << is_prefix 
//                     //         << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                     // }
//                 } else if (j >= (int)words.size()) {
//                     std::cout << "inclusive: " << inclusive << " j >= words.size()" << std::endl;
//                     // ASSERT_TRUE(iter.isValid());
//                     if(iter.isValid() == false){
//                         std::cout << "Error: " << "iter.isValid() = " << iter.isValid()
//                             << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                     }
//                     std::string iter_key = iter.getKeyWithSuffix(&bitlen);
//                     // std::string word_prefix = words[words.size() - 1].substr(0, iter_key.length());
//                     std::cout << "\titer_key: " << iter_key << " " << " bitlen: " << bitlen << std::endl;
//                     // std::cout << "\tword_prefix: " << word_prefix << std::endl;
//                     // is_prefix = isEqual(word_prefix, iter_key, bitlen);
//                     // // ASSERT_TRUE(is_prefix);
//                     // if(is_prefix == false){
//                     //     std::cout << "Error: " << "is_prefix = " << is_prefix 
//                     //         << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                     // }
//                 } else if (j == (int)words.size() - 1) {
//                     std::cout << "inclusive: " << inclusive << " j == words.size() - 1" << std::endl;
//                     std::string iter_key = iter.getKeyWithSuffix(&bitlen);
//                     // std::string word_prefix = words[words.size() - 1].substr(0, iter_key.length());
//                     std::cout << "\twords[j]: " <<  words[j] << " --> " << "iter_key: " << iter_key << " " << "bitlen: " << bitlen << std::endl;
//                     // if (iter.getFpFlag()) {
//                     //     // ASSERT_TRUE(iter.isValid());
//                     //     if(iter.isValid() == false){
//                     //         std::cout << "Error: " << "iter.isValid() = " << iter.isValid() 
//                     //             << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                     //     }
//                     //     std::string iter_key = iter.getKeyWithSuffix(&bitlen);
//                     //     // std::string word_prefix = words[words.size() - 1].substr(0, iter_key.length());
//                     //     std::cout << "\titer_key: " << iter_key << " " << "bitlen: " << bitlen << std::endl;
//                     //     // std::cout << "\tword_prefix: " << word_prefix << std::endl;
//                     //     // is_prefix = isEqual(word_prefix, iter_key, bitlen);
//                     //     // // // ASSERT_TRUE(is_prefix);
//                     //     // if(is_prefix == false){
//                     //     //     std::cout << "Error: " << "is_prefix = " << is_prefix 
//                     //     //         << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                     //     // }
//                     // } else {
//                     //     // ASSERT_FALSE(iter.isValid());
//                     //     if(iter.isValid() == true){
//                     //         std::cout << "Error: " << "iter.isValid() = " << iter.isValid() 
//                     //             << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                     //     }
//                     // }
//                 } else {
//                     std::cout << "inclusive: " << inclusive << " j >= 0 && j < words.size() - 1" << std::endl;
//                     // std::cout << "words[j] = " << words[j] << std::endl;
//                     // ASSERT_TRUE(iter.isValid());
//                     if(iter.isValid() == false){
//                         std::cout << "Error: " << "iter.isValid() = " << iter.isValid()
//                             << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
//                     }
//                     // std::string iter_key = iter.getKeyWithSuffix(&bitlen);
//                     // std::cout << "\titer_key: " << iter_key << " " << "bitlen: " << bitlen << std::endl;
//                     // test getKey()
//                     std::string iter_get_key = iter.getKey();
//                     std::cout << "\twords[j]: " << words[j] << " --> " << "iter_get_key: " << iter_get_key << std::endl;   
//                     // std::string iter_key_prefix = iter_key.substr(0, iter_get_key.length());

//                     iter++;
//                     if (iter.isValid()) {
//                         std::string iter_next_key = iter.getKey();
//                         std::cout << "\titer++ -> next_key: " << iter_next_key << std::endl;
//                     }

//                 }
//             }
// 	    }
// 	    // surf_->destroy();
// 	    // delete surf_;
// }


void test_find_next_string(
    surf::SuffixType kSuffixType = surf::SuffixType::kReal, 
    surf::level_t hash_suffix_len = 0, surf::level_t real_suffix_len = 8
){
    std::cout << std::string(40, '-') << "test_find_next_string Start" << std::string(40, '-') << std::endl;
    std::vector<std::string> keys;
    std::vector<bool> left_parentheses;
    std::vector<bool> right_parentheses;
    int string_len = 10;
    int N_key = 20;
    // time_t rand_seed = 1710605356;
    time_t rand_seed = time(NULL);
    srand(rand_seed);
    for(int i = 0; i < N_key; i++){
        //random generate a string
        std::string tmp;
        for(int j = 0; j < string_len; j++){
            char c = 'a' + rand() % 26;
            tmp.push_back(c);
        }
        std::string tmp2 = tmp.substr(0, rand() % string_len + 1);
        if(rand() % 2 == 0){
            keys.push_back(tmp2);
        }
        if(rand() % 2 == 0){
            keys.push_back(tmp);    
        }
    }

    std::sort(keys.begin(), keys.end());

    // uniquify
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());



    // std::vector<std::string> keys = {
    //     "anfzrjs",
    //     "ay",
    //     "bpmvde",
    //     "bpmvdessvs",
    //     "capvf",
    //     "g",
    //     "gdmszvdvcq",
    //     "jlufkfiytu",
    //     "kdmmm",
    //     "llwbmsjy",
    //     "llwbmsjyeb",
    //     "ngiawgg",
    //     "nrspstggqk",
    //     "oklpkp",
    //     "oklpkpqbkx",
    //     "rxkcbshnmi",
    //     "vbdhahwq",
    //     "vbdhahwqhr",
    //     "whklqcswzy",
    //     "z",
    //     "zzunthzqjn"
    // };










    for(int i = 0; i < keys.size(); i++){
        left_parentheses.push_back(rand() % 2 == 0);
        right_parentheses.push_back(rand() % 2 == 0);
    }


    bool flag_build_until_unique = false;
    //One shall clip the key into the prefix of length "max_num_level"
    // mast >= max([len(key) for key in keys])  --- maximum key length
    // uint16_t max_num_level = 5; 
    uint16_t max_num_level = string_len; 
    bool include_dense = true;
    // uint32_t sparse_dense_ratio = 16;
    uint32_t sparse_dense_ratio = 1;
    // SuRF* surf_ = new SuRF(keys, left_parentheses, right_parentheses, 
    //                             include_dense, sparse_dense_ratio,
    //                             // surf::kHash, 8, 0,
    //                             kSuffixType, hash_suffix_len, real_suffix_len,
    //                             flag_build_until_unique, max_num_level);
    SuRF* surf_ = new SuRF(keys, left_parentheses, right_parentheses, 
                                include_dense, sparse_dense_ratio,
                                // surf::kHash, 8, 0,
                                kSuffixType, hash_suffix_len, real_suffix_len,
                                max_num_level);

    //Test
    std::vector<std::string> test_keys;
    for(int i = 0; i < 10; i++){
        //random generate a string
        std::string tmp;
        int delta = rand() % 10 - 5;
        for(int j = 0; j < string_len+delta; j++){
            char c = 'a' + rand() % 26;
            tmp.push_back(c);
        }
        std::string tmp2 = tmp.substr(0, rand() % (string_len+delta) + 1);
        std::string tmp3 = tmp;
        for(int j = 0; j < rand() % 5; j++){
            tmp3[j] = 'a' + rand() % 26;
        }
        std::string tmp4 = tmp2;
        for(int j = 0; j < rand() % 5; j++){
            tmp4[j] = 'a' + rand() % 26;
        }
        test_keys.push_back(tmp);
        test_keys.push_back(tmp2);
        test_keys.push_back(tmp3);
        test_keys.push_back(tmp4);
    }
    // add to test keys
    for(auto key: keys){
        test_keys.push_back(key);
    }
    for(auto key: keys){
        int len = key.length();
        for(int j = 0; j < len; j++){
            std::string tmp = key;
            tmp[j] = 'a' + rand() % 26;
            test_keys.push_back(tmp);
        }
    }
    std::cout << "size of test_keys: "  << test_keys.size() << std::endl;

    // std::vector<std::string> test_keys = {"bpmvde"};

    for(auto &key_query: test_keys){
        auto it = std::lower_bound(keys.begin(), keys.end(), key_query);
        // std::string str_gt = *it;

        SuRF::Iter iter = surf_->moveToNextCommonPrefixKey(key_query);

        if(it == keys.end()){
            std::cout << "it == keys.end()" << std::endl;
            continue;
            if(iter.isValid()){
                // std::string key = iter.getKey();
                // std::cout << "key: " << key << " str_gt: " << "NULL" << std::endl;
            }else{
                std::cout << "Error: iter.isValid() shall be false" << std::endl;
                assert(false);
            }
            continue;
        }

        std::string key_found = iter.getKey();
        if(key_found != *it){
            std::cout << "random seed: " << rand_seed << std::endl;
            std::cout << "keys: " << std::endl;
            for(auto &key: keys){
                std::cout << key << " " << std::endl;
            }
            std::cout << "Error: key_gt != *it" << std::endl;
            std::cout << "key_query = " << key_query <<  " key_found = " << key_found << " key_gt: " <<  *it << std::endl;
            assert(false);
        }

        // int idx = it - keys.begin();
        // bool left_parenthesis = iter.getLeftParenthesis();
        // bool right_praenthesis = iter.getRightParenthesis();
        // bool left_gt = left_parentheses[idx];
        // bool right_gt = right_parentheses[idx];
        // if(left_parenthesis != left_gt){
        //     std::cout << "Error: left_parenthesis != left_gt" << std::endl;
        //     std::cout << "left_parenthesis: " << left_parenthesis << " left_gt: " << left_gt << std::endl;
        //     assert(false);
        // }
        // if(right_praenthesis != right_gt){
        //     std::cout << "Error: right_praenthesis != right_gt" << std::endl;
        //     std::cout << "right_praenthesis: " << right_praenthesis << " right_gt: " << right_gt << std::endl;
        //     assert(false);
        // }
    }
}

void test1(){

    std::vector<std::string> keys = {
	"f",
	"far",
	"fast",
	"s",
	"top",
	"toy",
	"tria",      
	"trib",      
	"tric",      
	// "trid",      
	// "trie",      
	// "trif",      
	// "trig",      
	// "trih",      
	// "trii",      
    };

    // std::vector<std::string> keys = {
    //     "akqq",
    //     "akqqzlukno",
    //     "degngzsdca",
    //     "dmcqfaqymr",
    //     "flfe",
    //     "flfeukdmnu",
    //     "hsikvuba",
    //     "hsikvubafp",
    //     "jwbobihluc",
    //     "jzbrgvszbg",
    //     "mpikrggkiw",
    //     "pfkrwszoen",
    //     "tbsonlpfdu",
    //     "td",
    //     "texdgd",
    //     "texdgdtplh",
    //     "tsowlxummx",
    //     "ubsdmyqte",
    //     "waskbirl"
    // };

    std::vector<bool> left_parantheses = {
        true,
        false,
        true,
        false,
        true,
        false,
        false,
        false,
        false,
        // false,
        // false,
        // false,
        // false,
        // false,
        // false
    };
    std::vector<bool> right_parantheses = {
        false,
        true,
        false,
        true,
        false,
        false,
        true,
        false,
        false,
        // false,
        // false,
        // false,
        // false,
        // false,
        // false
    };

    for(int i = left_parantheses.size(); i < keys.size(); i++){
        left_parantheses.push_back(false);
        right_parantheses.push_back(false);
    }

    bool flag_build_until_unique = false;
    //One shall clip the key into the prefix of length "max_num_level"
    // mast >= max([len(key) for key in keys])  --- maximum key length
    uint16_t max_num_level = 5; 
    for(auto str: keys){
        if(str.length() > max_num_level){
            max_num_level = str.length();
        }
    }
    std::cout << "max_num_level: " << max_num_level << std::endl;

    // basic surf
    // SuRF* surf = new SuRF(keys);
    // std::cout << std::string(20, '*') << " basic " << std::string(20, '*') << std::endl;
    // testMoveToKeyGreaterThanWord(surf, keys, surf::kNone);
    // std::cout << std::endl << std::endl;

    // // use default dense-to-sparse ratio; specify suffix type and length
    // SuRF* surf_hash = new SuRF(keys, surf::kHash, 8, 0);
    // testMoveToKeyGreaterThanWord(surf_hash, keys, surf::kHash);
    // std::cout << std::endl << std::endl;

    bool include_dense = true;
    // uint32_t sparse_dense_ratio = 16;
    uint32_t sparse_dense_ratio = 1;
    // SuRF* surf_hash2 = new SuRF(keys, left_parantheses, right_parantheses, 
    //                             include_dense, sparse_dense_ratio,
    //                             surf::kHash, 8, 0,
    //                             flag_build_until_unique, max_num_level);
    SuRF* surf_hash2 = new SuRF(keys, left_parantheses, right_parantheses, 
                                include_dense, sparse_dense_ratio,
                                surf::kHash, 8, 0,
                                max_num_level);
    std::cout << std::string(20, '*') << " Hash " << std::string(20, '*') << std::endl;
    std::vector<std::string> searched_keys = {
	"f",
	"fan",
	"fa",
	"far",
	"fara",
	"fate",     
	"fuse",
    "s",
    "a",
    "z",
    "zz",
    "tr",
    "tu",
    "t",
    "s"
    // "tsowlxummx", 
    // "tsowlxummx"    
    };
    // testMoveToKeyGreaterThanWord(surf_hash2, searched_keys, surf::kHash);

    
    // SuRF::Iter iter = surf_hash2->moveToFirst();
    // // std::string key = iter.getKey();
    // // std::cout << "key: " << key << std::endl;
    // // key = iter.getKey();
    // // std::cout << "key: " << key << std::endl;
    // int num = 10;
    // while(iter.isValid()){
    //     std::string key = iter.getKey();
    //     std::cout << "key: " << key << std::endl;

    //     bool left_paenthesis = iter.getLeftParenthesis();
    //     bool right_parenthesis = iter.getRightParenthesis();

    //     std::cout << "left_paenthesis: " << left_paenthesis << " right_parenthesis: " << right_parenthesis << std::endl;

    //     iter++;
    //     num--;
    // }

    // SuRF* surf_real = new SuRF(keys, surf::kReal, 0, 8);
    // std::cout << std::string(20, '*') << " Real " << std::string(20, '*') << std::endl;
    // testMoveToKeyGreaterThanWord(surf_real, keys, surf::kReal);
    // std::cout << std::endl << std::endl;

    
    // SuRF* surf_real_32bit = new SuRF(keys, surf::kReal, 0, 32, 1024);

    // // customize dense-to-sparse ratio; specify suffix type and length
    // SuRF* surf_mixed = new SuRF(keys, true, 16,  surf::kMixed, 4, 4);
    // std::cout << std::string(20, '*') << " Mixed " << std::string(20, '*') << std::endl;
    // testMoveToKeyGreaterThanWord(surf_mixed, keys, surf::kMixed);
    // std::cout << std::endl << std::endl;

    // std::cout << "Basic SuRF Memory Usage: " << surf->getMemoryUsage() << std::endl;
    // std::cout << "SuRF Hash Memory Usage: " << surf_hash->getMemoryUsage() << std::endl;
    // std::cout << "SuRF Real Memory Usage: " << surf_real->getMemoryUsage() << std::endl;
    // std::cout << "SuRF Mixed Memory Usage: " << surf_mixed->getMemoryUsage() << std::endl;

}

int main() {
    //// test1
    // test1();
    
    //// test2
    // std::cout << std::endl << std::endl;
    // int test_count2 = 1000;
    // while(test_count2){
    //     // test_find_next_string(surf::kHash, 8, 0);
    //     // test_find_next_string(surf::kReal, 0, 8);
    //     // test_find_next_string(surf::kMixed, 4, 4);
    //     test_find_next_string(surf::kHash, 0, 0);
    //     test_find_next_string(surf::kReal, 0, 0);
    //     test_find_next_string(surf::kMixed, 0, 0);
    //     test_count2--;
    // }
    // std::cout << std::endl << std::endl;

    //// test3
    int test_count3 = 1000;
    while(test_count3){
        testRangesToSurfToRanges();
        test_count3--;
    }


    
    // usage
    std::vector<std::pair<std::string, std::string>> ranges = {
        {"aa", "abc"},
        {"acc", "acdd"},
        {"bcde", "bcdef"}
    };
    int key_len_in_bytes = 5;
    surf::level_t hash_suffix_len = 0;
    surf::level_t real_suffix_len = 8;
    bool include_dense = true;
    uint32_t sparse_dense_ratio = 16;
    bool flag_build_until_unique = false;

    bool flag_allow_boundary_overlapped = false;
    // build SuREF from ranges
    // SuRF* surf_ = SuRF::rangesToSurf(ranges, key_len_in_bytes, surf::SuffixType::kReal, 
    //                         hash_suffix_len, real_suffix_len, include_dense, 
    //                         sparse_dense_ratio, flag_build_until_unique);
    SuRF* surf_ = SuRF::rangesToSurf(ranges, key_len_in_bytes, surf::SuffixType::kReal, 
                            hash_suffix_len, real_suffix_len, include_dense, 
                            sparse_dense_ratio, flag_allow_boundary_overlapped);
    
    //retrieve ranges from SuRf
    std::vector<std::pair<std::string, std::string>> ranges2 = SuRF::surfToRanges(surf_);
    assert(ranges == ranges2);
    std::cout << "testRangesToSurfToRanges passed" << std::endl;

    return 0;
}
