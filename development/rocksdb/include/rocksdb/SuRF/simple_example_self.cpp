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



    for(int i = 0; i < keys.size(); i++){
        left_parentheses.push_back(rand() % 2 == 0);
        right_parentheses.push_back(rand() % 2 == 0);
    }


    bool flag_build_until_unique = false;
    //One shall clip the key into the prefix of length "max_num_level"
    // mast >= max([len(key) for key in keys])  --- maximum key length
    uint16_t max_num_level = string_len; 
    bool include_dense = true;
    uint32_t sparse_dense_ratio = 1;
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

    for(auto &key_query: test_keys){
        auto it = std::lower_bound(keys.begin(), keys.end(), key_query);

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
