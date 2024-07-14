#ifndef LOUDSDENSE_CC_
#define LOUDSDENSE_CC_

#include <string>
#include <iostream>

// #include "bitvector.hpp"
#include "louds_dense.hpp"

namespace surf {

LoudsDense::LoudsDense(const SuRFBuilder* builder) {
    height_ = builder->getSparseStartLevel();
	// max_num_level = builder->getMaxNumLevel();
    std::vector<position_t> num_bits_per_level;
    for (level_t level = 0; level < height_; level++)
	num_bits_per_level.push_back(builder->getBitmapLabels()[level].size() * kWordSize);

    level_cuts_ = new position_t[height_];
    position_t bit_count = 0;
    for (level_t level = 0; level < height_; level++) {
		bit_count += num_bits_per_level[level];
		level_cuts_[level] = bit_count - 1;
    }

    label_bitmaps_ = new BitvectorRank(kRankBasicBlockSize, builder->getBitmapLabels(),
				       num_bits_per_level, 0, height_);
    child_indicator_bitmaps_ = new BitvectorRank(kRankBasicBlockSize,
						 builder->getBitmapChildIndicatorBits(),
						 num_bits_per_level, 0, height_);
    prefixkey_indicator_bits_ = new BitvectorRank(kRankBasicBlockSize,
						  builder->getPrefixkeyIndicatorBits(),
						  builder->getNodeCounts(), 0, height_);

    if (builder->getSuffixType() == kNone) {
	suffixes_ = new BitvectorSuffix();
    } else {
		level_t hash_suffix_len = builder->getHashSuffixLen();
        level_t real_suffix_len = builder->getRealSuffixLen();
        level_t suffix_len = hash_suffix_len + real_suffix_len;
		std::vector<position_t> num_suffix_bits_per_level;
		for (level_t level = 0; level < height_; level++)
			num_suffix_bits_per_level.push_back(builder->getSuffixCounts()[level] * suffix_len);
		suffixes_ = new BitvectorSuffix(builder->getSuffixType(), 
					hash_suffix_len, real_suffix_len,
										builder->getSuffixes(),
					num_suffix_bits_per_level, 0, height_);
    }


	// YCHUANG_ADDED START
	left_parentheses_ = new Bitvector(builder->getLeftParentheses(), builder->getLeftParenthesesCounts(), 0, height_);
	right_parentheses_ = new Bitvector(builder->getRightParentheses(), builder->getRightParenthesesCounts(), 0, height_);
	// YCHUANG_ADDED END

	// YCHUANG_ADDED START
	#ifdef DEBUG_LOUDS_DENSE
	std::cout << "\t\t" << "label_bitmaps_ " << "\n" << "\t\t";
	for(int i = 0; i < label_bitmaps_->numBits(); i+=kRankBasicBlockSize) {
		for(int j = 0; j < kRankBasicBlockSize; j++){
			if(i+j >= label_bitmaps_->numBits()){
				break;
			}
			if(label_bitmaps_->readBit(i+j) == 1){
				std::cout << (char)(i+j) << " ";
			}
		}
		std::cout << '|' << " ";
	}
	std::cout << std::endl;
	std::cout << "\t\t" << "child_indicator_bitmaps_ " << "\n" << "\t\t";
	for(int i = 0; i < child_indicator_bitmaps_->numBits(); i+=kRankBasicBlockSize) {
		for(int j = 0; j < kRankBasicBlockSize; j++){
			if(i+j >= child_indicator_bitmaps_->numBits()){
				break;
			}
			if(child_indicator_bitmaps_->readBit(i+j) == 1){
				std::cout << (char)(i+j) << " ";
			}
		}
		std::cout << '|' << " ";
	}
	std::cout << std::endl;
	std::cout << "\t\t" << "prefixkey_indicator_bits_ " << "\n" << "\t\t";
	for(int i = 0; i < prefixkey_indicator_bits_->numBits(); i++) {
		std::cout << prefixkey_indicator_bits_->readBit(i) << " ";
	}
	std::cout << std::endl;
	std::cout << "\t\t" << "suffixes_ " << "\n" << "\t\t" << "(";
	level_t suffix_len = 1;
	if (builder->getSuffixType() != kNone) {
		level_t hash_suffix_len = builder->getHashSuffixLen();
        level_t real_suffix_len = builder->getRealSuffixLen();
        suffix_len = hash_suffix_len + real_suffix_len;
	}
	for(int i = 0; i < suffixes_->numBits(); i++) {
		std::cout << suffixes_->readBit(i) << " ";
		if((i+1) % ((int)suffix_len) == 0) {
			if((i+1) != suffixes_->numBits()){
				std::cout << ") (";
			}else{
				std::cout << ")";
			}
		}
		// int value = 0;
		// value = (value << 1) + suffixes_->readBit(i);
		// if((i+1) % suffix_len == 0) {
		// 	std::cout << value << " ";
		// 	value = 0;
		// }
	}
	std::cout << std::endl;
	std::cout << "\t\t" << "left_parentheses_ " << "\n" << "\t\t";
	for(int i = 0; i < left_parentheses_->numBits(); i++) {
		std::cout << left_parentheses_->readBit(i) << " ";
	}
	std::cout << std::endl;
	std::cout << "\t\t" << "right_parentheses_ " << "\n" << "\t\t";
	for(int i = 0; i < right_parentheses_->numBits(); i++) {
		std::cout << right_parentheses_->readBit(i) << " ";
	}
	std::cout << std::endl;


	std::cout << " " <<  __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
	#endif // DEBUG_LOUDS_DENSE
	// YCHUANG_ADDED END
}

// // YCHUANG_ADDED START
// bool LoudsDense::setMaxNumLevel(uint16_t max_num_level){
// 	max_num_level = max_num_level;
// }
// // YCHUANG_ADDED END

bool LoudsDense::lookupKey(const std::string& key, position_t& out_node_num) const {
    position_t node_num = 0;
    position_t pos = 0;
    for (level_t level = 0; level < height_; level++) {
	pos = (node_num * kNodeFanout);
	if (level >= key.length()) { //if run out of searchKey bytes
	    if (prefixkey_indicator_bits_->readBit(node_num)) //if the prefix is also a key
			return suffixes_->checkEquality(getSuffixPos(pos, true), key, level + 1);
	    else
			return false;
	}
	pos += (label_t)key[level];

	//child_indicator_bitmaps_->prefetch(pos);

	if (!label_bitmaps_->readBit(pos)) //if key byte does not exist
	    return false;

	if (!child_indicator_bitmaps_->readBit(pos)) //if trie branch terminates
	    return suffixes_->checkEquality(getSuffixPos(pos, false), key, level + 1);

	node_num = getChildNodeNum(pos);
    }
    //search will continue in LoudsSparse
    out_node_num = node_num;
    return true;
}

// YCHUANG ADDED START
bool LoudsDense::moveToNextCommonPrefixKey(const std::string& key, 
				    LoudsDense::Iter& iter) const {
	bool could_be_fp_ = false;
    position_t node_num = 0;
    position_t pos = 0;
    for (level_t level = 0; level < height_; level++) {
		// if is_at_prefix_key_, pos is at the next valid position in the child node
		pos = node_num * kNodeFanout;
		
		// //if trie branch terminates ???
		// if (!child_indicator_bitmaps_->readBit(pos)){
		// 	//return compareSuffixGreaterThan(pos, key, level+1, inclusive, iter);
    	// 	iter.setFlags(true, false, true, true);
		// 	return true;
		// }

		if (level >= key.length()) { // if run out of searchKey bytes
			std::cout << "\t\t *** " << "level >= key.length()" << std::endl;
			std::cout << "\t\t *** " << "pos: " << pos << " " << " nextPos: " << getNextPos(pos - 1) 
			 		<< " " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
			iter.append(getNextPos(pos - 1));
			
			// if (prefixkey_indicator_bits_->readBit(node_num)){ //if the prefix is also a key
			// 	iter.is_at_prefix_key_ = true;
			// 	// valid, search complete, moveLeft complete, moveRight complete
			// 	iter.setFlags(true, true, true, true); 
			// }else{
			// 	iter.moveToLeftMostKey();
			// }

			iter.moveToLeftMostKey();

			return could_be_fp_;
		}

		pos += (label_t)key[level];
		iter.append(pos);

		// if no exact match
		if (!label_bitmaps_->readBit(pos)) {
			iter++;
			return could_be_fp_;
		}
		//if trie branch terminates
		if (!child_indicator_bitmaps_->readBit(pos)){
			// // return compareSuffixGreaterThan(pos, key, level+1, inclusive, iter);	
			// // if(level == key.length() - 1){		
    		// // 	iter.setFlags(true, false, true, true);
			// // 	iter++;
			// // 	// iter.moveToLeftMostKey();
			// // 	return false;
			// // }
			
			// // iter.moveToLeftMostKey();
			// // valid, search complete, moveLeft complete, moveRight complete
			// // iter.setFlags(true, true, true, true);
			// // iter.setFlags(true, true, false, true);

			// iter++;

// 			if(level == max_num_level - 1) { // ending at the last level --> stop at this key, because of not knowing (all of its postfix)
// std::cout << "\t\t *** " << "level == max_num_level - 1" << std::endl;
// std::cout << "\t\t *** " << "level: " << level << " " << "max_num_level: " << max_num_level << " " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
//     			iter.setFlags(true, true, true, true);
// 				return could_be_fp_;
// 			}else{ // ending before the last level --> move to next key
// 				iter++;
// 				return could_be_fp_;
// 			}

			if(level == key.size() - 1) { // same as  the key
    			iter.setFlags(true, true, true, true);
				return could_be_fp_;
			}else{ 
				iter++;
				return could_be_fp_;
			}

			return could_be_fp_;
		}
		node_num = getChildNodeNum(pos);
    }

    //search will continue in LoudsSparse
    iter.setSendOutNodeNum(node_num);
    // valid, search INCOMPLETE, moveLeft complete, moveRight complete
    iter.setFlags(true, false, true, true);
    return could_be_fp_;
}
// YCHUANG ADDED END

// bool LoudsDense::moveToKeyGreaterThan(const std::string& key, 
// 				      const bool inclusive, LoudsDense::Iter& iter) const {
bool LoudsDense::moveToKeyGreaterThan(const std::string& key, 
				       LoudsDense::Iter& iter) const {
    position_t node_num = 0;
    position_t pos = 0;
    for (level_t level = 0; level < height_; level++) {
		// if is_at_prefix_key_, pos is at the next valid position in the child node
		pos = node_num * kNodeFanout;
		if (level >= key.length()) { // if run out of searchKey bytes
			iter.append(getNextPos(pos - 1));
			//TODO: check correctness
			if (prefixkey_indicator_bits_->readBit(node_num)) //if the prefix is also a key
			iter.is_at_prefix_key_ = true;
			else
			iter.moveToLeftMostKey();
			// valid, search complete, moveLeft complete, moveRight complete
			iter.setFlags(true, true, true, true); 
			
			// if (prefixkey_indicator_bits_->readBit(node_num)){ //if the prefix is also a key
			// 	iter.is_at_prefix_key_ = true;
			// 	// valid, search complete, moveLeft complete, moveRight complete
			// 	iter.setFlags(true, true, true, true);
			// }else{
			// 	iter.moveToLeftMostKey();
			// }
			
			return true;
		}

		pos += (label_t)key[level];
		iter.append(pos);

		// if no exact match
		if (!label_bitmaps_->readBit(pos)) {
			iter++;
			return false;
		}
		//if trie branch terminates
		if (!child_indicator_bitmaps_->readBit(pos))
			// return compareSuffixGreaterThan(pos, key, level+1, inclusive, iter);
			return compareSuffixGreaterThan(pos, key, level+1, iter);
		node_num = getChildNodeNum(pos);
    }

    //search will continue in LoudsSparse
    iter.setSendOutNodeNum(node_num);
    // valid, search INCOMPLETE, moveLeft complete, moveRight complete
    iter.setFlags(true, false, true, true);
    return true;
}

void LoudsDense::extendPosList(std::vector<position_t>& pos_list,
			       position_t& out_node_num) const {
    position_t node_num = 0;
    position_t pos = pos_list[pos_list.size() - 1];
    for (level_t i = pos_list.size(); i < height_; i++) {
	node_num = getChildNodeNum(pos);
	if (!child_indicator_bitmaps_->readBit(pos))
	    node_num++;
	pos = (node_num * kNodeFanout);
	if (pos > level_cuts_[i]) {
	    pos = kMaxPos;
	    pos_list.push_back(pos);
	    break;
	}
	pos_list.push_back(pos);
    }
    if (pos == kMaxPos) {
	for (level_t i = pos_list.size(); i < height_; i++)
	    pos_list.push_back(pos);
	out_node_num = pos;
    } else {
	out_node_num = getChildNodeNum(pos);
	if (!child_indicator_bitmaps_->readBit(pos))
	    out_node_num++;
    }
}

uint64_t LoudsDense::approxCount(const LoudsDense::Iter* iter_left,
				 const LoudsDense::Iter* iter_right,
				 position_t& out_node_num_left,
				 position_t& out_node_num_right) const {
    std::vector<position_t> left_pos_list, right_pos_list;
    for (level_t i = 0; i < iter_left->key_len_; i++)
	left_pos_list.push_back(iter_left->pos_in_trie_[i]);
    level_t ori_left_len = left_pos_list.size();
    extendPosList(left_pos_list, out_node_num_left);
    
    for (level_t i = 0; i < iter_right->key_len_; i++)
	right_pos_list.push_back(iter_right->pos_in_trie_[i]);
    level_t ori_right_len = right_pos_list.size();
    extendPosList(right_pos_list, out_node_num_right);

    uint64_t count = 0;
    for (level_t i = 0; i < height_; i++) {
	position_t left_pos = left_pos_list[i];
	if (left_pos == kMaxPos) break;
	if (i == (ori_left_len - 1) && iter_left->is_at_prefix_key_)
	    left_pos = (left_pos / kNodeFanout) * kNodeFanout;
	position_t right_pos = right_pos_list[i];
	if (right_pos == kMaxPos)
	    right_pos = level_cuts_[i];
	if (i == (ori_right_len - 1) && iter_right->is_at_prefix_key_)
	    right_pos = (right_pos / kNodeFanout) * kNodeFanout;
	//assert(left_pos <= right_pos);
	if (left_pos < right_pos) {
	    if (i >= ori_left_len)
		left_pos = getNextPos(left_pos);
	    if (i >= ori_right_len && right_pos != level_cuts_[height_ - 1])
		right_pos = getNextPos(right_pos);
	    bool has_prefix_key_left
		= prefixkey_indicator_bits_->readBit(left_pos / kNodeFanout);
	    bool has_prefix_key_right
		= prefixkey_indicator_bits_->readBit(right_pos / kNodeFanout);
	    position_t rank_left_label = label_bitmaps_->rank(left_pos);
	    position_t rank_right_label = label_bitmaps_->rank(right_pos);
	    if (right_pos == level_cuts_[height_ - 1])
		rank_right_label++;
	    position_t rank_left_ind = child_indicator_bitmaps_->rank(left_pos);
	    position_t rank_right_ind = child_indicator_bitmaps_->rank(right_pos);
	    position_t rank_left_prefix
		= prefixkey_indicator_bits_->rank(left_pos / kNodeFanout);
	    position_t rank_right_prefix
		= prefixkey_indicator_bits_->rank(right_pos / kNodeFanout);
	    position_t num_leafs = (rank_right_label - rank_left_label)
		- (rank_right_ind - rank_left_ind)
		+ (rank_right_prefix - rank_left_prefix);
	    // offcount in child_indicators
	    if (child_indicator_bitmaps_->readBit(right_pos))
		num_leafs++;
	    if (child_indicator_bitmaps_->readBit(left_pos))
		num_leafs--;
	    // offcount in prefix keys
	    if (i >= ori_right_len && has_prefix_key_right)
		num_leafs--;
	    if (i >= ori_left_len && has_prefix_key_left)
		num_leafs++;
	    if (iter_left->is_search_complete_ && (i == ori_left_len - 1))
		num_leafs--;
	    count += num_leafs;
	}
    }
    return count;
}

uint64_t LoudsDense::serializedSize() const {
    uint64_t size = sizeof(height_)
	+ (sizeof(position_t) * height_);
    sizeAlign(size);
    size += (label_bitmaps_->serializedSize()
	     + child_indicator_bitmaps_->serializedSize()
	     + prefixkey_indicator_bits_->serializedSize()
	     + suffixes_->serializedSize());
	// YCHUANG_ADDED START
	size += (left_parentheses_->serializedSize()
	     + right_parentheses_->serializedSize());
	// YCHUANG_ADDED END
    sizeAlign(size);

	
    return size;
}

uint64_t LoudsDense::getMemoryUsage() const {
    // return (sizeof(LoudsDense)
	//     + label_bitmaps_->size()
	//     + child_indicator_bitmaps_->size()
	//     + prefixkey_indicator_bits_->size()
	//     + suffixes_->size());
	
	// YCHUANG_ADDED START
	return (sizeof(LoudsDense)
		+ label_bitmaps_->size()
		+ child_indicator_bitmaps_->size()
		+ prefixkey_indicator_bits_->size()
		+ suffixes_->size()
		+ left_parentheses_->size()
		+ right_parentheses_->size());
}

uint64_t LoudsDense::getMemoryUsageInBitsSelf() const {
	// std::cout << "getMemoryUsageInBitsSelf" << " " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
	// std::cout << "label_bitmaps_->nmBits(): " << label_bitmaps_->numBits() << " label_bitmaps_->rankLutSize(): " << label_bitmaps_->rankLutSize() << std::endl;
	// std::cout << "child_indicator_bitmaps_->nmBits(): " << child_indicator_bitmaps_->numBits() << " child_indicator_bitmaps_->rankLutSize(): " << child_indicator_bitmaps_->rankLutSize() << std::endl;
	// std::cout << "prefixkey_indicator_bits_->nmBits(): " << prefixkey_indicator_bits_->numBits() << " prefixkey_indicator_bits_->rankLutSize(): " << prefixkey_indicator_bits_->rankLutSize() << std::endl;
	// std::cout << "suffixes_->numBits(): " << suffixes_->numBits() << std::endl;
	// std::cout << "left_parentheses_->numBits(): " << left_parentheses_->numBits() << std::endl;
	// std::cout << "right_parentheses_->numBits(): " << right_parentheses_->numBits() << std::endl;
	return (sizeof(LoudsDense)
		+ label_bitmaps_->numBits()
		+ label_bitmaps_->rankLutSize()
		+ child_indicator_bitmaps_->numBits()
		+ child_indicator_bitmaps_->rankLutSize()
		+ prefixkey_indicator_bits_->numBits() 
		+ prefixkey_indicator_bits_->rankLutSize() 
		+ suffixes_->numBits()
		+ left_parentheses_->numBits()
		+ right_parentheses_->numBits());
}

position_t LoudsDense::getChildNodeNum(const position_t pos) const {
    return child_indicator_bitmaps_->rank(pos);
}

position_t LoudsDense::getSuffixPos(const position_t pos, const bool is_prefix_key) const {
    position_t node_num = pos / kNodeFanout;
    position_t suffix_pos = (label_bitmaps_->rank(pos)
			     - child_indicator_bitmaps_->rank(pos)
			     + prefixkey_indicator_bits_->rank(node_num)
			     - 1);
    if (is_prefix_key && label_bitmaps_->readBit(pos) && !child_indicator_bitmaps_->readBit(pos))
	suffix_pos--;
    return suffix_pos;
}

position_t LoudsDense::getNextPos(const position_t pos) const {
    return pos + label_bitmaps_->distanceToNextSetBit(pos);
}

position_t LoudsDense::getPrevPos(const position_t pos, bool* is_out_of_bound) const {
    position_t distance = label_bitmaps_->distanceToPrevSetBit(pos);
    if (pos <= distance) {
	*is_out_of_bound = true;
	return 0;
    }
    *is_out_of_bound = false;
    return (pos - distance);
}

// bool LoudsDense::compareSuffixGreaterThan(const position_t pos, const std::string& key, 
// 					  const level_t level, const bool inclusive, 
// 					  LoudsDense::Iter& iter) const {
bool LoudsDense::compareSuffixGreaterThan(const position_t pos, const std::string& key, 
					  const level_t level, 
					  LoudsDense::Iter& iter) const {
    position_t suffix_pos = getSuffixPos(pos, false);
    int compare = suffixes_->compare(suffix_pos, key, level);
    if ((compare != kCouldBePositive) && (compare < 0)) {
	iter++;
	return false;
    }
    // valid, search complete, moveLeft complete, moveRight complete
    iter.setFlags(true, true, true, true);
    return true;
}

//============================================================================

void LoudsDense::Iter::clear() {
    is_valid_ = false;
    key_len_ = 0;
    is_at_prefix_key_ = false;
}

int LoudsDense::Iter::compare(const std::string& key) const {
    if (is_at_prefix_key_ && (key_len_ - 1) < key.length())
	return -1;
    std::string iter_key = getKey();
    std::string key_dense = key.substr(0, iter_key.length());
    int compare = iter_key.compare(key_dense);
    if (compare != 0) return compare;
    if (isComplete()) {
	position_t suffix_pos = trie_->getSuffixPos(pos_in_trie_[key_len_ - 1], is_at_prefix_key_);
	return trie_->suffixes_->compare(suffix_pos, key, key_len_);
    }
    return compare;
}

std::string LoudsDense::Iter::getKey() const {
    if (!is_valid_)
	return std::string();
    level_t len = key_len_;
    if (is_at_prefix_key_)
	len--;
    return std::string((const char*)key_.data(), (size_t)len);
}

int LoudsDense::Iter::getSuffix(word_t* suffix) const {
    if (isComplete()
        && ((trie_->suffixes_->getType() == kReal) || (trie_->suffixes_->getType() == kMixed))) {
	position_t suffix_pos = trie_->getSuffixPos(pos_in_trie_[key_len_ - 1], is_at_prefix_key_);
	*suffix = trie_->suffixes_->readReal(suffix_pos);
	return trie_->suffixes_->getRealSuffixLen();
    }
    *suffix = 0;
    return 0;
}

// YCHUANG_ADDED START
bool LoudsDense::Iter::getLeftParenthesis() const {
	position_t pos = trie_->getSuffixPos(pos_in_trie_[key_len_-1], is_at_prefix_key_);
	// std::cout << "pos: " << pos << " " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
	return trie_->left_parentheses_->readBit(pos);
}
bool LoudsDense::Iter::getRightParenthesis() const {
	position_t pos = trie_->getSuffixPos(pos_in_trie_[key_len_-1], is_at_prefix_key_);
	// std::cout << "pos: " << pos << " " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
	return trie_->right_parentheses_->readBit(pos);
}
// YCHUANG_ADDED END

std::string LoudsDense::Iter::getKeyWithSuffix(unsigned* bitlen) const {
    std::string iter_key = getKey();
    if (isComplete()
        && ((trie_->suffixes_->getType() == kReal) || (trie_->suffixes_->getType() == kMixed))) {
	position_t suffix_pos = trie_->getSuffixPos(pos_in_trie_[key_len_ - 1], is_at_prefix_key_);
	word_t suffix = trie_->suffixes_->readReal(suffix_pos);
	if (suffix > 0) {
	    level_t suffix_len = trie_->suffixes_->getRealSuffixLen();
	    *bitlen = suffix_len % 8;
	    suffix <<= (64 - suffix_len);
	    char* suffix_str = reinterpret_cast<char*>(&suffix);
	    suffix_str += 7;
	    unsigned pos = 0;
	    while (pos < suffix_len) {
		iter_key.append(suffix_str, 1);
		suffix_str--;
		pos += 8;
	    }
	}
    }
    return iter_key;
}

void LoudsDense::Iter::append(position_t pos) {
    assert(key_len_ < key_.size());
    key_[key_len_] = (label_t)(pos % kNodeFanout);
    pos_in_trie_[key_len_] = pos;
    key_len_++;
}

void LoudsDense::Iter::set(level_t level, position_t pos) {
    assert(level < key_.size());
    key_[level] = (label_t)(pos % kNodeFanout);
    pos_in_trie_[level] = pos;
}

void LoudsDense::Iter::setFlags(const bool is_valid,
				const bool is_search_complete, 
				const bool is_move_left_complete,
				const bool is_move_right_complete) {
    is_valid_ = is_valid;
    is_search_complete_ = is_search_complete;
    is_move_left_complete_ = is_move_left_complete;
    is_move_right_complete_ = is_move_right_complete;
}

void LoudsDense::Iter::setToFirstLabelInRoot() {
    if (trie_->label_bitmaps_->readBit(0)) {
	pos_in_trie_[0] = 0;
	key_[0] = (label_t)0;
    } else {
	pos_in_trie_[0] = trie_->getNextPos(0);
	key_[0] = (label_t)pos_in_trie_[0];
    }
    key_len_++;
}

void LoudsDense::Iter::setToLastLabelInRoot() {
    bool is_out_of_bound;
    pos_in_trie_[0] = trie_->getPrevPos(kNodeFanout, &is_out_of_bound);
    key_[0] = (label_t)pos_in_trie_[0];
    key_len_++;
}

void LoudsDense::Iter::moveToLeftMostKey() {
    assert(key_len_ > 0);
    level_t level = key_len_ - 1;
    position_t pos = pos_in_trie_[level];
    if (!trie_->child_indicator_bitmaps_->readBit(pos))
	// valid, search complete, moveLeft complete, moveRight complete
	return setFlags(true, true, true, true);

    while (level < trie_->getHeight() - 1) {
		position_t node_num = trie_->getChildNodeNum(pos);
		//if the current prefix is also a key
		if (trie_->prefixkey_indicator_bits_->readBit(node_num)) {
			append(trie_->getNextPos(node_num * kNodeFanout - 1));
			is_at_prefix_key_ = true;
			// valid, search complete, moveLeft complete, moveRight complete
			return setFlags(true, true, true, true);
		}

		pos = trie_->getNextPos(node_num * kNodeFanout - 1);
		append(pos);

		// if trie branch terminates
		if (!trie_->child_indicator_bitmaps_->readBit(pos))
			// valid, search complete, moveLeft complete, moveRight complete
			return setFlags(true, true, true, true);

		level++;
    }
    send_out_node_num_ = trie_->getChildNodeNum(pos);
    // valid, search complete, moveLeft INCOMPLETE, moveRight complete
    setFlags(true, true, false, true);
}

void LoudsDense::Iter::moveToRightMostKey() {
    assert(key_len_ > 0);
    level_t level = key_len_ - 1;
    position_t pos = pos_in_trie_[level];
    if (!trie_->child_indicator_bitmaps_->readBit(pos))
	// valid, search complete, moveLeft complete, moveRight complete
	return setFlags(true, true, true, true);

    while (level < trie_->getHeight() - 1) {
	position_t node_num = trie_->getChildNodeNum(pos);
	bool is_out_of_bound;
	pos = trie_->getPrevPos((node_num + 1) * kNodeFanout, &is_out_of_bound);
	if (is_out_of_bound) {
	    is_valid_ = false;
	    return;
	}
	append(pos);

	// if trie branch terminates
	if (!trie_->child_indicator_bitmaps_->readBit(pos))
	    // valid, search complete, moveLeft complete, moveRight complete
	    return setFlags(true, true, true, true);

	level++;
    }
    send_out_node_num_ = trie_->getChildNodeNum(pos);
    // valid, search complete, moveleft complete, moveRight INCOMPLETE
    setFlags(true, true, true, false);
}

void LoudsDense::Iter::operator ++(int) {
    assert(key_len_ > 0);
    if (is_at_prefix_key_) {
	is_at_prefix_key_ = false;
	return moveToLeftMostKey();
    }
    position_t pos = pos_in_trie_[key_len_ - 1];
    position_t next_pos = trie_->getNextPos(pos);
    // if crossing node boundary
    while ((next_pos / kNodeFanout) > (pos / kNodeFanout)) {
	key_len_--;
	if (key_len_ == 0) {
	    is_valid_ = false;
	    return;
	}
	pos = pos_in_trie_[key_len_ - 1];
	next_pos = trie_->getNextPos(pos);
    }
    set(key_len_ - 1, next_pos);
    return moveToLeftMostKey();
}

void LoudsDense::Iter::operator --(int) {
    assert(key_len_ > 0);
    if (is_at_prefix_key_) {
	is_at_prefix_key_ = false;
	key_len_--;
    }
    position_t pos = pos_in_trie_[key_len_ - 1];
    bool is_out_of_bound;
    position_t prev_pos = trie_->getPrevPos(pos, &is_out_of_bound);
    if (is_out_of_bound) {
	is_valid_ = false;
	return;
    }
    
    // if crossing node boundary
    while ((prev_pos / kNodeFanout) < (pos / kNodeFanout)) {
	//if the current prefix is also a key
	position_t node_num = pos / kNodeFanout;
	if (trie_->prefixkey_indicator_bits_->readBit(node_num)) {
	    is_at_prefix_key_ = true;
	    // valid, search complete, moveLeft complete, moveRight complete
	    return setFlags(true, true, true, true);
	}
	
	key_len_--;
	if (key_len_ == 0) {
	    is_valid_ = false;
	    return;
	}
	pos = pos_in_trie_[key_len_ - 1];
	prev_pos = trie_->getPrevPos(pos, &is_out_of_bound);
	if (is_out_of_bound) {
	    is_valid_ = false;
	    return;
	}
    }
    set(key_len_ - 1, prev_pos);
    return moveToRightMostKey();
}

} //namespace surf

#endif // LOUDSDENSE_CC_
