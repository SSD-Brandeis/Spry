#ifndef SURFBUILDER_CC_
#define SURFBUILDER_CC_

#include <assert.h>

#include <string>
#include <vector>
#include <iostream>
#include "surf_builder.hpp"

namespace surf {

void SuRFBuilder::build(const std::vector<std::string>& keys) {
    assert(keys.size() > 0);
    buildSparse(keys);
    if (include_dense_) {
	determineCutoffLevel();
	buildDense();
    }
    // YCHUANG ADDED START
	max_num_level_ = getLabels().size();
	// YCHUANG_ADDED END
}

// YCHUANG ADDED START
// void SuRFBuilder::build(const std::vector<std::string>& keys, const std::vector<bool>& left_parentheses, const std::vector<bool>& right_parentheses, const bool flag_build_until_unique, const uint16_t max_num_level) {
void SuRFBuilder::build(const std::vector<std::string>& keys, const std::vector<bool>& left_parentheses, const std::vector<bool>& right_parentheses, const uint16_t max_num_level) {
    assert(keys.size() > 0);
    // buildSparseWithparentheses(keys, left_parentheses, right_parentheses, flag_build_until_unique, max_num_level);
    buildSparseWithparentheses(keys, left_parentheses, right_parentheses, max_num_level);
    if (include_dense_) {
	determineCutoffLevel();
	buildDense();
    }
    max_num_level_ = max_num_level;
}

// void SuRFBuilder::buildSparseWithparentheses(const std::vector<std::string>& keys, const std::vector<bool>& left_parentheses, const std::vector<bool>& right_parentheses, const bool flag_build_until_unique, int max_num_level) {
void SuRFBuilder::buildSparseWithparentheses(const std::vector<std::string>& keys, const std::vector<bool>& left_parentheses, const std::vector<bool>& right_parentheses, level_t max_num_level) {
    assert(("len of parantheses should be the same as len of keys", keys.size() == left_parentheses.size() && keys.size() == right_parentheses.size()));
    assert(max_num_level > 0);
    assert(max_num_level < 1e3);
    // max_level is only used when flag_build_until_unique = false
    for (position_t i = 0; i < keys.size(); i++) {
        // level_t level = skipCommonPrefix(keys[i]);
        level_t level = 0;
        std::string key = keys[i];
        while (level < ((uint32_t)key.length()) && (level < max_num_level) && isCharCommonPrefix((label_t)key[level], level)) {
            setBit(child_indicator_bits_[level], getNumItems(level) - 1);
            level++;
        }


        position_t curpos = i;
        while ((i + 1 < keys.size()) && isSameKey(keys[curpos], keys[i+1])){
            i++;
        }

        // if (flag_build_until_unique == true){
        //     if (i < keys.size() - 1){
        //         level = insertKeyBytesToTrieUntilUnique(keys[curpos], keys[i+1], level);
        //         std::cout << "\t\t\t" << " level: " << level << " keys[curpos] " << keys[curpos] << " keys[i+1] " << keys[i+1] 
        //                 << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl; 
        //     }else{ // for last key, there is no successor key in the list
        //         level = insertKeyBytesToTrieUntilUnique(keys[curpos], std::string(), level);
        //         std::cout << "\t\t\t" << " level: " << level << " keys[curpos] " << keys[curpos] << " keys[i+1] " << ""
        //                 << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        //     }
        //     insertSuffix(keys[curpos], level);
        // }else{

        // level = insertKeyBytesToTrieUntilLevel(keys[curpos], level, max_num_level-1);
        if (i < keys.size() - 1){
            level = insertKeyBytesToTrieUntilLevel(keys[curpos], keys[i+1], level, max_num_level-1);
            // std::cout << "\t\t\t" << " level: " << level << " keys[curpos] " << keys[curpos] << " keys[i+1] " << keys[i+1] 
            //         << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl; 
        }else{ // for last key, there is no successor key in the list
            level = insertKeyBytesToTrieUntilLevel(keys[curpos], std::string(), level, max_num_level-1);
            // std::cout << "\t\t\t" << " level: " << level << " keys[curpos] " << keys[curpos] << " keys[i+1] " << ""
            //         << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        }
#ifdef DEBUG_BUILDER
        std::cout << "\t\t" << " --- " << " key: " << keys[curpos] << " " << " suffix insert level: " << level << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
#endif // DEBUG_BUILDER
        insertSuffix(keys[curpos], level);
        insertLeftParenthesis(left_parentheses[curpos], level);
        insertRightParenthesis(right_parentheses[curpos], level);
#ifdef DEBUG_BUILDER
        std::cout << "Finish inserting left and right parentheses" << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
#endif // DEBUG_BUILDER
        // }
    }
}
// YCHUANG ADDED END


void SuRFBuilder::buildSparse(const std::vector<std::string>& keys) {
    // max_level is only used when flag_build_until_unique = false
    for (position_t i = 0; i < keys.size(); i++) {
        level_t level = skipCommonPrefix(keys[i]);	
        position_t curpos = i;
        while ((i + 1 < keys.size()) && isSameKey(keys[curpos], keys[i+1])){
            i++;
        }

        if (i < keys.size() - 1){
            level = insertKeyBytesToTrieUntilUnique(keys[curpos], keys[i+1], level);
            std::cout << "\t\t\t" << " level: " << level << " keys[curpos] " << keys[curpos] << " keys[i+1] " << keys[i+1] 
                    << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl; 
        }else{ // for last key, there is no successor key in the list
            level = insertKeyBytesToTrieUntilUnique(keys[curpos], std::string(), level);
            std::cout << "\t\t\t" << " level: " << level << " keys[curpos] " << keys[curpos] << " keys[i+1] " << ""
                    << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        }

        std::cout << "\t\t" << " --- " << " key: " << keys[curpos] << " " << " suffix insert level: " << level << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

        insertSuffix(keys[curpos], level);
    }
}

level_t SuRFBuilder::skipCommonPrefix(const std::string& key) {
    level_t level = 0;
    while (level < key.length() && isCharCommonPrefix((label_t)key[level], level)) {
        setBit(child_indicator_bits_[level], getNumItems(level) - 1);
        level++;
    }
    return level;
}

// YCHUANG ADDED START
level_t SuRFBuilder::insertKeyBytesToTrieUntilLevel(const std::string& key, const std::string& next_key, 
                                                        const level_t start_level, const level_t end_level) {
    // if end_level == -1: insert all bytes of key to the trie 
    assert(start_level < key.length());
    // std::cout << " *A " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    // level_t max_level = end_level == -1? 1e8 : end_level;
    assert(start_level <= end_level);
    assert(end_level < 1e3);
    level_t max_level = end_level;

    assert(start_level <= max_level+1);
    if(start_level == max_level+1){
        return max_level+1;
    }

    level_t level = start_level;
    bool is_start_of_node = false;
    bool is_term = false;
    // std::cout << " *A2 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

    // If it is the start of level, the louds bit needs to be set.
    if (isLevelEmpty(level)){
        is_start_of_node = true;
    }  
    // After skipping the common prefix, the first following byte
    // shoud be in an the node as the previous key.
    insertKeyByte(key[level], level, is_start_of_node, is_term);
    level++;
    // std::cout << " *A3 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

    // All the following bytes inserted must be the start of a
    // new node.
    is_start_of_node = true;
    while (level < key.length()) {   
        // if (max_level != -1 && level > max_level){
        if (level > max_level){
            return level;
        }
        insertKeyByte(key[level], level, is_start_of_node, is_term);
        level++;
    }
    // std::cout << " *A4 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

    if(level == key.length()){
        bool is_prefix = false;
        if(next_key.length() >= key.length()){
            is_prefix = true;
            for(std::size_t i = 0; i < key.length(); i++){
                if(key[i] != next_key[i]){
                    is_prefix = false;
                    break;
                }
            }
        }else{
            is_prefix = false;
        }
        if(is_prefix){
            is_term = true;
            insertKeyByte(kTerminator, level, is_start_of_node, is_term);
            level++;
        }
    }
    // std::cout << " *A5 " << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

    return level;
}
// YCHUANG ADDED END


level_t SuRFBuilder::insertKeyBytesToTrieUntilUnique(const std::string& key, const std::string& next_key, const level_t start_level) {
    assert(start_level < key.length());

    level_t level = start_level;
    bool is_start_of_node = false;
    bool is_term = false;
    // If it is the start of level, the louds bit needs to be set.
    if (isLevelEmpty(level)){
	    is_start_of_node = true;
    }
    // After skipping the common prefix, the first following byte
    // shoud be in an the node as the previous key.
    insertKeyByte(key[level], level, is_start_of_node, is_term);
    level++;
    if (level > next_key.length()
	|| !isSameKey(key.substr(0, level), next_key.substr(0, level))){
	    return level;
    }

    // All the following bytes inserted must be the start of a
    // new node.
    is_start_of_node = true;
    while (level < key.length() && level < next_key.length() && key[level] == next_key[level]) {
        insertKeyByte(key[level], level, is_start_of_node, is_term);
        level++;
    }

    // The last byte inserted makes key unique in the trie.
    if (level < key.length()) {
	    insertKeyByte(key[level], level, is_start_of_node, is_term);
    } else {
        is_term = true;
        insertKeyByte(kTerminator, level, is_start_of_node, is_term);
    }
    level++;

    return level;
}

inline void SuRFBuilder::insertSuffix(const std::string& key, const level_t level) {
    if (level >= getTreeHeight())
	addLevel();
    assert(level - 1 < suffixes_.size());
    word_t suffix_word = BitvectorSuffix::constructSuffix(suffix_type_, key, hash_suffix_len_,
                                                          level, real_suffix_len_);
    // YCHUANG ADDED START
    #ifdef DEBUG_BUILDER
    std::cout << "\t\t\t insertSuffix sparse" << " Level: " << level << "\n"
            << "\t\t\t" << " key: " << key << " " << " suffix_word: " << suffix_word 
            << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    #endif // DEBUG_BUILDER
    // YCHUANG ADDED END
    storeSuffix(level, suffix_word);
}

// YCHUANG ADDED START
inline void SuRFBuilder::insertLeftParenthesis(const bool& left_parenthesis, const level_t level) {
    if (level >= getTreeHeight())
    addLevel();
    assert(level - 1 < left_parentheses_.size());
    // std::cout << "insertLeftParenthesis: " << (word_t) left_parenthesis << " " << "level " << level << " " << "left_parentheses_counts_[level-1] " << left_parentheses_counts_[level-1] 
    //         << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    storeLeftParenthesis(level, left_parenthesis);
}

inline void SuRFBuilder::insertRightParenthesis(const bool& right_parenthesis, const level_t level) {
    if (level >= getTreeHeight())
    addLevel();
    assert(level - 1 < right_parentheses_.size());
    // std::cout << "insertRightParenthesis: " << (word_t) right_parenthesis << " " << "level " << level << " " << "right_parentheses_counts_[level-1] " << right_parentheses_counts_[level-1] 
    //         << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    storeRightParenthesis(level, right_parenthesis);
}
// YCHUANG ADDED END

inline bool SuRFBuilder::isCharCommonPrefix(const label_t c, const level_t level) const {
    // level < getTreeHeight();
    // is_last_item_terminator_[level];
    // c == labels_[level].back();
    // if(level < getTreeHeight()){
    //     std::cout << "isCharCommonPrefix: " << "level " << level << " " << "getTreeHeight() " << getTreeHeight() << std::endl;
    //     std::cout << " " << "is_last_item_terminator_[level] " << is_last_item_terminator_[level] << std::endl;
    //     std::cout << " " << "c " << c << " " << "labels_[level].back() " << labels_[level].back() << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

    // }
    return (level < getTreeHeight())
	&& (!is_last_item_terminator_[level])
	&& (c == labels_[level].back());
}

inline bool SuRFBuilder::isLevelEmpty(const level_t level) const {
    return (level >= getTreeHeight()) || (labels_[level].size() == 0);
}

inline void SuRFBuilder::moveToNextItemSlot(const level_t level) {
    assert(level < getTreeHeight());
    position_t num_items = getNumItems(level);
    if (num_items % kWordSize == 0) {
        child_indicator_bits_[level].push_back(0);
        louds_bits_[level].push_back(0);
    }
}

void SuRFBuilder::insertKeyByte(const char c, const level_t level, const bool is_start_of_node, const bool is_term) {
    // level should be at most equal to tree height
    if (level >= getTreeHeight())
	addLevel();

    assert(level < getTreeHeight());

    // sets parent node's child indicator
    if (level > 0){
	    setBit(child_indicator_bits_[level-1], getNumItems(level-1) - 1);
    }

    labels_[level].push_back(c);
    if (is_start_of_node) {
        setBit(louds_bits_[level], getNumItems(level) - 1);
        node_counts_[level]++;
    }
    is_last_item_terminator_[level] = is_term;

    moveToNextItemSlot(level);
}


// YCHUANG ADDED START
inline void SuRFBuilder::storeLeftParenthesis(const level_t level, const word_t left_parenthesis) {
    level_t left_parentheses_len = getLeftParenthesesLen();
    position_t pos = left_parentheses_counts_[level-1] * left_parentheses_len;
    assert(pos <= (left_parentheses_[level-1].size() * kWordSize));
    if (pos == (left_parentheses_[level-1].size() * kWordSize))
	left_parentheses_[level-1].push_back(0);
    position_t word_id = pos / kWordSize;
    position_t offset = pos % kWordSize;
    position_t word_remaining_len = kWordSize - offset;
    if (left_parentheses_len <= word_remaining_len) {
        word_t shifted_parenthesis = left_parenthesis << (word_remaining_len - left_parentheses_len);
        left_parentheses_[level-1][word_id] += shifted_parenthesis;
    } else {
        word_t parenthesis_left_part = left_parenthesis >> (left_parentheses_len - word_remaining_len);
        left_parentheses_[level-1][word_id] += parenthesis_left_part;
        left_parentheses_[level-1].push_back(0);
        word_id++;
        word_t parenthesis_right_part = left_parenthesis << (kWordSize - (left_parentheses_len - word_remaining_len));
        left_parentheses_[level-1][word_id] += parenthesis_right_part;
    }
    left_parentheses_counts_[level-1]++;
}


inline void SuRFBuilder::storeRightParenthesis(const level_t level, const word_t right_parenthesis) {
    level_t right_parentheses_len = getRightParenthesesLen();
    position_t pos = right_parentheses_counts_[level-1] * right_parentheses_len;
    assert(pos <= (right_parentheses_[level-1].size() * kWordSize));
    if (pos == (right_parentheses_[level-1].size() * kWordSize))
	right_parentheses_[level-1].push_back(0);
    position_t word_id = pos / kWordSize;
    position_t offset = pos % kWordSize;
    position_t word_remaining_len = kWordSize - offset;
    if (right_parentheses_len <= word_remaining_len) {
        word_t shifted_parenthesis = right_parenthesis << (word_remaining_len - right_parentheses_len);
        right_parentheses_[level-1][word_id] += shifted_parenthesis;
    } else {
        word_t parenthesis_left_part = right_parenthesis >> (right_parentheses_len - word_remaining_len);
        right_parentheses_[level-1][word_id] += parenthesis_left_part;
        right_parentheses_[level-1].push_back(0);
        word_id++;
        word_t parenthesis_right_part = right_parenthesis << (kWordSize - (right_parentheses_len - word_remaining_len));
        right_parentheses_[level-1][word_id] += parenthesis_right_part;
    }
    right_parentheses_counts_[level-1]++;
}
// YCHUANG ADDED END

inline void SuRFBuilder::storeSuffix(const level_t level, const word_t suffix) {
    level_t suffix_len = getSuffixLen();
    position_t pos = suffix_counts_[level-1] * suffix_len;
    assert(pos <= (suffixes_[level-1].size() * kWordSize));
    if (pos == (suffixes_[level-1].size() * kWordSize))
	suffixes_[level-1].push_back(0);
    position_t word_id = pos / kWordSize;
    position_t offset = pos % kWordSize;
    position_t word_remaining_len = kWordSize - offset;
    if (suffix_len <= word_remaining_len) {
	word_t shifted_suffix = suffix << (word_remaining_len - suffix_len);
	suffixes_[level-1][word_id] += shifted_suffix;
    } else {
	word_t suffix_left_part = suffix >> (suffix_len - word_remaining_len);
	suffixes_[level-1][word_id] += suffix_left_part;
	suffixes_[level-1].push_back(0);
	word_id++;
	word_t suffix_right_part = suffix << (kWordSize - (suffix_len - word_remaining_len));
	suffixes_[level-1][word_id] += suffix_right_part;
    }
    suffix_counts_[level-1]++;
}

inline void SuRFBuilder::determineCutoffLevel() {
    level_t cutoff_level = 0;
    uint64_t dense_mem = computeDenseMem(cutoff_level);
    uint64_t sparse_mem = computeSparseMem(cutoff_level);
    while ((cutoff_level < getTreeHeight()) && (dense_mem * sparse_dense_ratio_ < sparse_mem)) {
	cutoff_level++;
	dense_mem = computeDenseMem(cutoff_level);
	sparse_mem = computeSparseMem(cutoff_level);
    }
    sparse_start_level_ = cutoff_level--;
}

inline uint64_t SuRFBuilder::computeDenseMem(const level_t downto_level) const {
    assert(downto_level <= getTreeHeight());
    uint64_t mem = 0;
    for (level_t level = 0; level < downto_level; level++) {
	mem += (2 * kFanout * node_counts_[level]);
	if (level > 0)
	    mem += (node_counts_[level - 1] / 8 + 1);
	mem += (suffix_counts_[level] * getSuffixLen() / 8);
    }
    return mem;
}

inline uint64_t SuRFBuilder::computeSparseMem(const level_t start_level) const {
    uint64_t mem = 0;
    for (level_t level = start_level; level < getTreeHeight(); level++) {
	position_t num_items = labels_[level].size();
	mem += (num_items + 2 * num_items / 8 + 1);
	mem += (suffix_counts_[level] * getSuffixLen() / 8);
    }
    return mem;
}

// // YCHUANG ADDED START
// void SuRFBuilder::buildDenseWithparentheses() {
//     for (level_t level = 0; level < sparse_start_level_; level++) {
//         initDenseVectors(level);
//         if (getNumItems(level) == 0) continue;

//         position_t node_num = 0;
//         if (isTerminator(level, 0))
//             setBit(prefixkey_indicator_bits_[level], 0);
//         else
//             setLabelAndChildIndicatorBitmap(level, node_num, 0);
//         for (position_t pos = 1; pos < getNumItems(level); pos++) {
//             if (isStartOfNode(level, pos)) {
//                 node_num++;
//                 if (isTerminator(level, pos)) {
//                     setBit(prefixkey_indicator_bits_[level], node_num);
//                     continue;
//                 }
//             }
//             setLabelAndChildIndicatorBitmap(level, node_num, pos);
//         }
//     }
// }

void SuRFBuilder::buildDense() {
    for (level_t level = 0; level < sparse_start_level_; level++) {
        initDenseVectors(level);
        if (getNumItems(level) == 0) continue;

        position_t node_num = 0;
        if (isTerminator(level, 0))
            setBit(prefixkey_indicator_bits_[level], 0);
        else
            setLabelAndChildIndicatorBitmap(level, node_num, 0);
        for (position_t pos = 1; pos < getNumItems(level); pos++) {
            if (isStartOfNode(level, pos)) {
                node_num++;
                if (isTerminator(level, pos)) {
                    setBit(prefixkey_indicator_bits_[level], node_num);
                    continue;
                }
            }
            setLabelAndChildIndicatorBitmap(level, node_num, pos);
        }
    }
}

void SuRFBuilder::initDenseVectors(const level_t level) {
    bitmap_labels_.push_back(std::vector<word_t>());
    bitmap_child_indicator_bits_.push_back(std::vector<word_t>());
    prefixkey_indicator_bits_.push_back(std::vector<word_t>());

    for (position_t nc = 0; nc < node_counts_[level]; nc++) {
        for (int i = 0; i < (int)kFanout; i += kWordSize) {
            bitmap_labels_[level].push_back(0);
            bitmap_child_indicator_bits_[level].push_back(0);
        }
        if (nc % kWordSize == 0){
            prefixkey_indicator_bits_[level].push_back(0);
        }
    }
}

void SuRFBuilder::setLabelAndChildIndicatorBitmap(const level_t level, 
						  const position_t node_num, const position_t pos) {
    label_t label = labels_[level][pos];
    setBit(bitmap_labels_[level], node_num * kFanout + label);
    if (readBit(child_indicator_bits_[level], pos))
	setBit(bitmap_child_indicator_bits_[level], node_num * kFanout + label);
}

void SuRFBuilder::addLevel() {
    labels_.push_back(std::vector<label_t>());
    child_indicator_bits_.push_back(std::vector<word_t>());
    louds_bits_.push_back(std::vector<word_t>());
    suffixes_.push_back(std::vector<word_t>());
    suffix_counts_.push_back(0);
    // YCHUANG ADDED START
    left_parentheses_.push_back(std::vector<word_t>());
    right_parentheses_.push_back(std::vector<word_t>());
    left_parentheses_counts_.push_back(0);
    right_parentheses_counts_.push_back(0);
    // YCHUANG ADDED END

    node_counts_.push_back(0);
    is_last_item_terminator_.push_back(false);

    child_indicator_bits_[getTreeHeight() - 1].push_back(0);
    louds_bits_[getTreeHeight() - 1].push_back(0);
}

position_t SuRFBuilder::getNumItems(const level_t level) const {
    return labels_[level].size();
}

bool SuRFBuilder::isStartOfNode(const level_t level, const position_t pos) const {
    return readBit(louds_bits_[level], pos);
}

bool SuRFBuilder::isTerminator(const level_t level, const position_t pos) const {
    label_t label = labels_[level][pos];
    return ((label == kTerminator) && !readBit(child_indicator_bits_[level], pos));
}

} // namespace surf

#endif // SURFBUILDER_CC_
