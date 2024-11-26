#ifndef LOUDSSPARSE_CC_
#define LOUDSSPARSE_CC_

#include <string>
#include <cassert>	// YCHUANG_ADDED

// #include "bitvector.hpp"
#include "louds_sparse.hpp"
#include "surf.hpp"

namespace surf {

LoudsSparse::LoudsSparse(const SuRFBuilder* builder) {
    height_ = builder->getLabels().size();
    start_level_ = builder->getSparseStartLevel();

    node_count_dense_ = 0;
    for (level_t level = 0; level < start_level_; level++)
	node_count_dense_ += builder->getNodeCounts()[level];

    if (start_level_ == 0)
	child_count_dense_ = 0;
    else
	child_count_dense_ = node_count_dense_ + builder->getNodeCounts()[start_level_] - 1;

    labels_ = new LabelVector(builder->getLabels(), start_level_, height_);

    std::vector<position_t> num_items_per_level;
    for (level_t level = 0; level < height_; level++){
		num_items_per_level.push_back(builder->getLabels()[level].size());

		// YCHUANG_ADDED START
		#ifdef DEBUG_LOUDS_SPARSE
		std::cout << "\t\t\t" << "Sparse level: " << level << " num_items_per_level: " << num_items_per_level[level] << "\n";
		std::cout << "\t\t\t" ;
		for(int i = 0; i < num_items_per_level[level]; i++) {
			std::cout << builder->getLabels()[level][i] << " ";
		}
		std::cout << " " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
		#endif //DEBUG_LOUDS_SPARSE
		// YCHUANG_ADDED END
	}

    level_cuts_ = new position_t[height_];
    for (level_t level = 0; level < start_level_; level++) {
		level_cuts_[level] = 0;
    }
    position_t bit_count = 0;
    for (level_t level = start_level_; level < height_; level++) {
		bit_count += num_items_per_level[level];
		level_cuts_[level] = bit_count - 1;
    }

    child_indicator_bits_ = new BitvectorRank(kRankBasicBlockSize, builder->getChildIndicatorBits(), 
					      num_items_per_level, start_level_, height_);
    louds_bits_ = new BitvectorSelect(kSelectSampleInterval, builder->getLoudsBits(), 
				      num_items_per_level, start_level_, height_);

    if (builder->getSuffixType() == kNone) {
		suffixes_ = new BitvectorSuffix(); // basic surf
    } else {
		level_t hash_suffix_len = builder->getHashSuffixLen();
        level_t real_suffix_len = builder->getRealSuffixLen();
        level_t suffix_len = hash_suffix_len + real_suffix_len;
		std::vector<position_t> num_suffix_bits_per_level;
		for (level_t level = 0; level < height_; level++)
			num_suffix_bits_per_level.push_back(builder->getSuffixCounts()[level] * suffix_len);

		suffixes_ = new BitvectorSuffix(builder->getSuffixType(), hash_suffix_len, real_suffix_len,
											builder->getSuffixes(),
						num_suffix_bits_per_level, start_level_, height_);
	}

	// YCHUANG_ADDED START
	left_parentheses_ = new Bitvector(builder->getLeftParentheses(), builder->getLeftParenthesesCounts(), start_level_, height_);
	right_parentheses_ = new Bitvector(builder->getRightParentheses(), builder->getRightParenthesesCounts(), start_level_, height_);
	// YCHUANG_ADDED END

	// YCHUANG_ADDED START
	#ifdef DEBUG_LOUDS_SPARSE
	std::cout << "******* " << "Sparse init:" << "\n"
			<< "\t\t" << "height_: " << height_ << "\n"
			<< "\t\t" << "start_level_: " << start_level_ << "\n"
			<< "\t\t" << "node_count_dense_: " << node_count_dense_ << "\n"
			<< "\t\t" << "child_count_dense_: " << child_count_dense_ << "\n";
	for(int i = 0; i < height_; i++) {
		std::cout << "\t\t" << "level_cuts_[" << i << "]: " << level_cuts_[i] << "\n";
	}
	std::cout << "\t\t" << "labels_ " << "\n" << "\t\t";
	for(int i = 0; i < labels_->getNumBytes(); i++) {
		std::cout << (*labels_)[i] << " ";
	}
	std::cout << std::endl;
	std::cout << "\t\t" << "child_indicator_bits_ " << "\n" << "\t\t";
	for(int i = 0; i < child_indicator_bits_->numBits(); i++) {
		std::cout << child_indicator_bits_->readBit(i) << " ";
	}
	std::cout << std::endl;
	std::cout << "\t\t" << "louds_bits_ " << "\n" << "\t\t";
	for(int i = 0; i < louds_bits_->numBits(); i++) {
		std::cout << louds_bits_->readBit(i) << " ";
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
	std::cout  << std::endl;
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
	#endif //DEBUG_LOUDS_SPARSE
	// YCHUANG_ADDED END

}

bool LoudsSparse::lookupKey(const std::string& key, const position_t in_node_num) const {
    position_t node_num = in_node_num;
    position_t pos = getFirstLabelPos(node_num);
    level_t level = 0;
    for (level = start_level_; level < key.length(); level++) {
		//child_indicator_bits_->prefetch(pos);
		if (!labels_->search((label_t)key[level], pos, nodeSize(pos)))
			return false;

		// if trie branch terminates
		if (!child_indicator_bits_->readBit(pos)){
			bool termination_rst = suffixes_->checkEquality(getSuffixPos(pos), key, level + 1);
			return termination_rst;
		}

		// move to child
		node_num = getChildNodeNum(pos);
		pos = getFirstLabelPos(node_num);
    }
    if ((labels_->read(pos) == kTerminator) && (!child_indicator_bits_->readBit(pos)))
	return suffixes_->checkEquality(getSuffixPos(pos), key, level + 1);
    return false;
}

// YCHUANG_ADDED START
bool LoudsSparse::moveToNextCommonPrefixKey(const std::string& key, 
					LoudsSparse::Iter& iter,
					bool flag_direct_return_if_found_key_end_with_same_prefix) const {
	bool could_be_fp_ = false;
    position_t node_num = iter.getStartNodeNum();
    position_t pos = getFirstLabelPos(node_num);

    level_t level;
	level_t max_matching_full_prefix_key_level = 1e8;
    for (level = start_level_; level < key.length(); level++) {
		position_t node_size = nodeSize(pos);
		
		// if no exact match
		position_t pos_search = pos;
		// TO CHECK
		// pos search is unpdated within the search function, however there's a bug in the search function, moving pos_search unequally
		// position of pos_search will be moved by search function (not consistent)
		if (!labels_->search((label_t)key[level], pos_search, node_size)) {
			#ifdef DEBUG_LOUDS_SPARSE
			std::cout << " sparse diverge " << " level: " << level << " key:" << key << " " <<  __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;	
			std::cout << " sparse diverge " << " pos: " << pos << " node_label:" << labels_->read(pos)  << " node_size:" << node_size << " key[level]:" << key[level] << " " <<  __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
			#endif //DEBUG_LOUDS_SPARSE

			// if there's a complete prefix key fully matches the prefix of the search key
			// std::cout << "iter.key_len_ = " << iter.key_len_ << " max_matching_full_prefix_key_level = " << max_matching_full_prefix_key_level << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
			if(flag_direct_return_if_found_key_end_with_same_prefix == true
				&& max_matching_full_prefix_key_level < ((level_t)1e8) ){ // For SuRF_Base in SuRF_RDF (yucheng)
				iter.is_valid_ = true;
				// iter.append(key[level], pos);
				iter.key_len_ = max_matching_full_prefix_key_level;
				return could_be_fp_;
			}
			
			moveToLeftInNextSubtrie(pos, node_size, key[level], iter);

			return could_be_fp_;
		}else{ // key[level] matches the node at pos_search 
			pos = pos_search;
		}

		// // std::cout << "level = " << level << ", (*labels_)[" << pos << "]  = " << (*labels_)[pos] << " node_size = " << nodeSize(pos) << "  getFirstLabelPos(node_num) = " <<  getFirstLabelPos(node_num) << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
		// for(uint32_t i_pos = 0; i_pos < (*labels_).getNumBytes(); i_pos++){
		// 	std::cout <<  (*labels_)[i_pos]  << " "; 
		// }
		// std::cout << std::endl;
		// // iter.append(key[level], pos);

		// if trie branch terminates
		if (!child_indicator_bits_->readBit(pos)){

			#ifdef DEBUG_LOUDS_SPARSE
			std::cout << "sparse trie branch terminates" <<  " level:" << level << " key:" << key << " " <<  __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
			#endif //DEBUG_LOUDS_SPARSE

			// if it fully matches the prefix and doesn't have any successor
			if(flag_direct_return_if_found_key_end_with_same_prefix == true){ // For SuRF_Base in SuRF_RDF (yucheng)
				iter.is_valid_ = true;
				iter.append(key[level], pos);
				return could_be_fp_;
			}

			if(level == key.size() - 1) { // same as the key
				iter.is_valid_ = true;
				iter.append(key[level], pos);
				return could_be_fp_;
			}else{ 
				
				// if there's a complete prefix key fully matches the prefix of the search key
				if(flag_direct_return_if_found_key_end_with_same_prefix == true
					&& max_matching_full_prefix_key_level < ((level_t)1e8) ){ // For SuRF_Base in SuRF_RDF (yucheng)
					iter.is_valid_ = true;
					// iter.append(key[level], pos);
					iter.key_len_ = max_matching_full_prefix_key_level;
					return could_be_fp_;
				}

				iter.append(key[level], pos);
				iter++; //already at the end
				// moveToLeftInNextSubtrie(pos_search, node_size-(pos_search-pos), key[level], iter);
				//cannot set to valid because when sparse part reaches to the end (is_valid = False), 
				//we have to go back to the dense part (move it 1 node forward) to continue the search
				// iter.is_valid_ = true;
				
				return could_be_fp_;
			}


			// iter.is_valid_ = true;
			// return could_be_fp_;
		}


		iter.append(key[level], pos);

		// move to child
		node_num = getChildNodeNum(pos);
		pos = getFirstLabelPos(node_num);

		// std::cout << "next_pos = " << pos << " " << __FILE__ << ":" << __LINE__ << " " << std::endl;
		// log the complete prefix key that ends with the same prefix of the search key
		if (flag_direct_return_if_found_key_end_with_same_prefix == true 
			// && child_indicator_bits_->readBit(pos)
			&& nodeSize(pos) > 1    // if there's only 1 255 as its child, it cannot be the kTerminator 
			&&((*labels_)[pos] == kTerminator) // kTerminator can only exist as the first child key
			) {
			max_matching_full_prefix_key_level = level;
		}
    }

	// if there's a complete prefix key fully matches the prefix of the search key
	if(flag_direct_return_if_found_key_end_with_same_prefix == true
		&& max_matching_full_prefix_key_level < ((level_t)1e8) ){ // For SuRF_Base in SuRF_RDF (yucheng)
		iter.is_valid_ = true;
		// iter.append(key[level], pos);
		iter.key_len_ = max_matching_full_prefix_key_level;
		return could_be_fp_;
	}

    if ((labels_->read(pos) == kTerminator)
	&& (!child_indicator_bits_->readBit(pos))
	&& !isEndofNode(pos)) {
		iter.append(kTerminator, pos);
		iter.is_at_terminator_ = true;
		
		iter.is_valid_ = true;
		return could_be_fp_;
    }

    if (key.length() <= level) {
		iter.moveToLeftMostKey();
		iter.is_valid_ = true;
		return could_be_fp_;
    }

	assert(("Should not reach here", false));
   
    iter.is_valid_ = true;
    return true;
}
// YCHUANG_ADDED END

bool LoudsSparse::moveToKeyGreaterThan(const std::string& key, 
				       const bool inclusive, LoudsSparse::Iter& iter) const {
    position_t node_num = iter.getStartNodeNum();
    position_t pos = getFirstLabelPos(node_num);

    level_t level;
    for (level = start_level_; level < key.length(); level++) {
		position_t node_size = nodeSize(pos);
		// if no exact match
		if (!labels_->search((label_t)key[level], pos, node_size)) {
			moveToLeftInNextSubtrie(pos, node_size, key[level], iter);
			return false;
		}

		iter.append(key[level], pos);

		// if trie branch terminates
		if (!child_indicator_bits_->readBit(pos)){
			return compareSuffixGreaterThan(pos, key, level+1, iter);
		}

		// move to child
		node_num = getChildNodeNum(pos);
		pos = getFirstLabelPos(node_num);
    }

    if ((labels_->read(pos) == kTerminator)
	&& (!child_indicator_bits_->readBit(pos))
	&& !isEndofNode(pos)) {
		iter.append(kTerminator, pos);
		iter.is_at_terminator_ = true;
		if (!inclusive)
			iter++;
		iter.is_valid_ = true;
		return false;
    }

    if (key.length() <= level) {
		iter.moveToLeftMostKey();
		return false;
    }
   
    iter.is_valid_ = true;
    return true;
}

position_t LoudsSparse::appendToPosList(std::vector<position_t>& pos_list,
					const position_t node_num,
					const level_t level,
					const bool isLeft, bool& done) const {
    position_t pos = getFirstLabelPos(node_num);
    if (pos > level_cuts_[start_level_ + level]) {
	pos = kMaxPos;
	if (isLeft) {
	    pos_list.push_back(pos);
	} else {
	    for (level_t j = 0; j < (height_ - level) - 1; j++)
		pos_list.push_back(pos);
	}
	done = true;
    }
    pos_list.push_back(pos);
    return pos;
}

void LoudsSparse::extendPosList(std::vector<position_t>& left_pos_list,
				std::vector<position_t>& right_pos_list,
				const position_t left_in_node_num,
				const position_t right_in_node_num) const {
    position_t left_node_num = 0, right_node_num = 0, left_pos = 0, right_pos = 0;
    bool left_done = false, right_done = false;
    level_t start_depth = left_pos_list.size();
    if (start_depth > right_pos_list.size())
	start_depth = right_pos_list.size();
    if (start_depth == 0) {
	if (left_pos_list.size() == 0)
	    left_pos = appendToPosList(left_pos_list, left_in_node_num,
				       0, true, left_done);
	if (right_pos_list.size() == 0)
	    right_pos = appendToPosList(right_pos_list, right_in_node_num,
					0, false, right_done);
	start_depth++;
    }

    left_pos = left_pos_list[left_pos_list.size() - 1];
    right_pos = right_pos_list[right_pos_list.size() - 1];
    for (level_t i = start_depth; i < (height_ - start_level_); i++) {
	if (left_pos == right_pos) break;
	if (!left_done && left_pos_list.size() <= i) {
	    left_node_num = getChildNodeNum(left_pos);
	    if (!child_indicator_bits_->readBit(left_pos))
		left_node_num++;
	    left_pos = appendToPosList(left_pos_list, left_node_num,
				       i, true, left_done);
	}
	if (!right_done && right_pos_list.size() <= i) {
	    right_node_num = getChildNodeNum(right_pos);
	    if (!child_indicator_bits_->readBit(right_pos))
		right_node_num++;
	    right_pos = appendToPosList(right_pos_list, right_node_num,
					i, false, right_done);
	}
    }
}

uint64_t LoudsSparse::approxCount(const LoudsSparse::Iter* iter_left,
				  const LoudsSparse::Iter* iter_right,
				  const position_t in_node_num_left,
				  const position_t in_node_num_right) const {
    if (in_node_num_left == kMaxPos) return 0;
    std::vector<position_t> left_pos_list, right_pos_list;
    for (level_t i = 0; i < iter_left->key_len_; i++)
	left_pos_list.push_back(iter_left->pos_in_trie_[i]);
    level_t ori_left_len = left_pos_list.size();
    if (in_node_num_right == kMaxPos) {
	for (level_t i = 0; i < (height_ - start_level_); i++)
	    right_pos_list.push_back(kMaxPos);
    } else {
	for (level_t i = 0; i < iter_right->key_len_; i++)
	    right_pos_list.push_back(iter_right->pos_in_trie_[i]);
    }
    extendPosList(left_pos_list, right_pos_list, in_node_num_left, in_node_num_right);

    uint64_t count = 0;
    level_t search_depth = left_pos_list.size();
    if (search_depth > right_pos_list.size())
	search_depth = right_pos_list.size();
    for (level_t i = 0; i < search_depth; i++) {
	position_t left_pos = left_pos_list[i];
	if (left_pos == kMaxPos) break;
	position_t right_pos = right_pos_list[i];
	if (right_pos == kMaxPos)
	    right_pos = level_cuts_[start_level_ + i] + 1;
	//assert(left_pos <= right_pos);
	if (left_pos < right_pos) {
	    position_t rank_left = child_indicator_bits_->rank(left_pos);
	    position_t rank_right = child_indicator_bits_->rank(right_pos);
	    position_t num_leafs = (right_pos - left_pos) - (rank_right - rank_left);
	    if (child_indicator_bits_->readBit(right_pos))
		num_leafs++;
	    if (child_indicator_bits_->readBit(left_pos))
		num_leafs--;
	    if (i == ori_left_len - 1)
		num_leafs--;
	    count += num_leafs;
	}
    }
    return count;
}

uint64_t LoudsSparse::serializedSize() const {
    uint64_t size = sizeof(height_) + sizeof(start_level_)
	+ sizeof(node_count_dense_) + sizeof(child_count_dense_)
	+ (sizeof(position_t) * height_);
    sizeAlign(size);
    size += (labels_->serializedSize()
	     + child_indicator_bits_->serializedSize()
	     + louds_bits_->serializedSize()
	     + suffixes_->serializedSize());
	// YCHUANG_ADDED START
	size += (left_parentheses_->serializedSize());
	size += (right_parentheses_->serializedSize());
	// YCHUANG_ADDED END
    sizeAlign(size);
    return size;
}

uint64_t LoudsSparse::getMemoryUsage() const {
    return (sizeof(this)
	    + labels_->size()
	    + child_indicator_bits_->size()
	    + louds_bits_->size()
	    + suffixes_->size());
}

uint64_t LoudsSparse::getMemoryUsageInBitsSelf() const {
	surf::SuRF_Env *_surf_env = surf::SuRF_Env::getInstance();
	if(_surf_env->getShowSurfCompactionInfo()){
		std::cout << "@surf sparse getMemoryUsageInBitsSelf start-------------" << std::endl
				<< "(sizeof(this): " << sizeof(this) * 8 << " B "
				<< ", labels_->getNumBytes() * 8: " << labels_->getNumBytes() * 8 << " bits "
				<< ", child_indicator_bits_->numBits(): " << child_indicator_bits_->numBits() << " bits, "
				<< ", child_indicator_bits_->rankLutSize(): " << child_indicator_bits_->rankLutSize() << " bits "
				<< ", louds_bits_->numBits(): " << louds_bits_->numBits() << " bits "
				<< ", suffixes_->numBits(): " << suffixes_->numBits() << " bits "
				<< ", left_parentheses_->numBits(): " << left_parentheses_->numBits() << " bits "
				<< ", right_parentheses_->numBits(): " << right_parentheses_->numBits() << " bits)"
		  		<< " " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
	}

	// return (sizeof(this)
	//     + labels_->getNumBytes() * 8
	//     + child_indicator_bits_->numBits()
	//     + child_indicator_bits_->rankLutSize()
	//     + louds_bits_->numBits()
	//     + suffixes_->numBits()
	// 	// YCHUANG_ADDED START
	// 	+ left_parentheses_->numBits()
	// 	+ right_parentheses_->numBits()
	// 	// YCHUANG_ADDED END
	// 	);
	return (
	    labels_->getNumBytes() * 8
	    + child_indicator_bits_->numBits()
	    + child_indicator_bits_->rankLutSize()
	    + louds_bits_->numBits()
	    + suffixes_->numBits()
		// YCHUANG_ADDED START
		+ left_parentheses_->numBits()
		+ right_parentheses_->numBits()
		// YCHUANG_ADDED END
		);
}

position_t LoudsSparse::getChildNodeNum(const position_t pos) const {
    return (child_indicator_bits_->rank(pos) + child_count_dense_);
}

position_t LoudsSparse::getFirstLabelPos(const position_t node_num) const {
    return louds_bits_->select(node_num + 1 - node_count_dense_);
}

position_t LoudsSparse::getLastLabelPos(const position_t node_num) const {
    position_t next_rank = node_num + 2 - node_count_dense_;
    if (next_rank > louds_bits_->numOnes())
	return (louds_bits_->numBits() - 1);
    return (louds_bits_->select(next_rank) - 1);
}

position_t LoudsSparse::getSuffixPos(const position_t pos) const {
    return (pos - child_indicator_bits_->rank(pos));
}

position_t LoudsSparse::nodeSize(const position_t pos) const {
    assert(louds_bits_->readBit(pos));
	// YCHUANG_ADDED START
	if(pos == louds_bits_->numBits() - 1) {
		return 1;
	}
	// YCHUANG_ADDED END
    return louds_bits_->distanceToNextSetBit(pos);
}

bool LoudsSparse::isEndofNode(const position_t pos) const {
    return ((pos == louds_bits_->numBits() - 1)
	    || louds_bits_->readBit(pos + 1));
}

void LoudsSparse::moveToLeftInNextSubtrie(position_t pos, const position_t node_size, 
					  const label_t label, LoudsSparse::Iter& iter) const {
	position_t pos_last_node = pos + node_size - 1;
	// if no label is greater than key[level] in this node
	// searchGreaterThan moves the pos to the greater node, however, node_size is not changed 
    if (!labels_->searchGreaterThan(label, pos, node_size)) {
	// iter.append(pos + node_size - 1);
	iter.append(pos_last_node);
	return iter++;
    } else {
	iter.append(pos);
#ifdef DEBUG_LOUDS_SPARSE
	label_t label_read = labels_->read(pos);
	std::cout << "sparse move to left most key" << " label_read:" << label_read << " " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
#endif //DEBUG_LOUDS_SPARSE
	return iter.moveToLeftMostKey();
    }
}

bool LoudsSparse::compareSuffixGreaterThan(const position_t pos, const std::string& key, 
					   const level_t level,
					   LoudsSparse::Iter& iter) const {
    position_t suffix_pos = getSuffixPos(pos);
    int compare = suffixes_->compare(suffix_pos, key, level);
    if ((compare != kCouldBePositive) && (compare < 0)) {
	iter++;
	return false;
    }
    iter.is_valid_ = true;
    return true;
}


void LoudsSparse::Iter::clear() {
    is_valid_ = false;
    key_len_ = 0;
    is_at_terminator_ = false;
}

int LoudsSparse::Iter::compare(const std::string& key) const {
    if (is_at_terminator_ && (key_len_ - 1) < (key.length() - start_level_))
	return -1;
    std::string iter_key = getKey();
    std::string key_sparse = key.substr(start_level_);
    std::string key_sparse_same_length = key_sparse.substr(0, iter_key.length());
    int compare = iter_key.compare(key_sparse_same_length);
    if (compare != 0) 
	return compare;
    position_t suffix_pos = trie_->getSuffixPos(pos_in_trie_[key_len_ - 1]);
    return trie_->suffixes_->compare(suffix_pos, key_sparse, key_len_);
}

std::string LoudsSparse::Iter::getKey() const {
    if (!is_valid_)
	return std::string();
    level_t len = key_len_;
    if (is_at_terminator_)
	len--;
    return std::string((const char*)key_.data(), (size_t)len);
}

int LoudsSparse::Iter::getSuffix(word_t* suffix) const {
    if ((trie_->suffixes_->getType() == kReal) || (trie_->suffixes_->getType() == kMixed)) {
	position_t suffix_pos = trie_->getSuffixPos(pos_in_trie_[key_len_ - 1]);
	*suffix = trie_->suffixes_->readReal(suffix_pos);
	return trie_->suffixes_->getRealSuffixLen();
    }
    *suffix = 0;
    return 0;
}

// YCHUANG_ADDED START
bool LoudsSparse::Iter::getLeftParenthesis() const {
	position_t pos = trie_->getSuffixPos(pos_in_trie_[key_len_ - 1]);
	return trie_->left_parentheses_->readBit(pos);
}
bool LoudsSparse::Iter::getRightParenthesis() const {
	position_t pos = trie_->getSuffixPos(pos_in_trie_[key_len_ - 1]);
	return trie_->right_parentheses_->readBit(pos);
}
// YCHUANG_ADDED END

std::string LoudsSparse::Iter::getKeyWithSuffix(unsigned* bitlen) const {
    std::string iter_key = getKey();
    if ((trie_->suffixes_->getType() == kReal) || (trie_->suffixes_->getType() == kMixed)) {
	position_t suffix_pos = trie_->getSuffixPos(pos_in_trie_[key_len_ - 1]);
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

void LoudsSparse::Iter::append(const position_t pos) {
    assert(key_len_ < key_.size());
    key_[key_len_] = trie_->labels_->read(pos);
    pos_in_trie_[key_len_] = pos;
    key_len_++;
}

void LoudsSparse::Iter::append(const label_t label, const position_t pos) {
    assert(key_len_ < key_.size());
    key_[key_len_] = label;
    pos_in_trie_[key_len_] = pos;
    key_len_++;
}

void LoudsSparse::Iter::set(const level_t level, const position_t pos) {
    assert(level < key_.size());
    key_[level] = trie_->labels_->read(pos);
    pos_in_trie_[level] = pos;
}

void LoudsSparse::Iter::setToFirstLabelInRoot() {
    assert(start_level_ == 0);
    pos_in_trie_[0] = 0;
    key_[0] = trie_->labels_->read(0);
}

void LoudsSparse::Iter::setToLastLabelInRoot() {
    assert(start_level_ == 0);
    pos_in_trie_[0] = trie_->getLastLabelPos(0);
    key_[0] = trie_->labels_->read(pos_in_trie_[0]);
}

void LoudsSparse::Iter::moveToLeftMostKey() {
    if (key_len_ == 0) {
		position_t pos = trie_->getFirstLabelPos(start_node_num_);
		label_t label = trie_->labels_->read(pos);
		append(label, pos);
    }

    level_t level = key_len_ - 1;
    position_t pos = pos_in_trie_[level];
    label_t label = trie_->labels_->read(pos);

    if (!trie_->child_indicator_bits_->readBit(pos)) {
		if ((label == kTerminator)
			&& !trie_->isEndofNode(pos))
			is_at_terminator_ = true;
		is_valid_ = true;
		return;
    }

    while (level < trie_->getHeight()) {
		position_t node_num = trie_->getChildNodeNum(pos);
		pos = trie_->getFirstLabelPos(node_num);
		label = trie_->labels_->read(pos);
		// if trie branch terminates
		if (!trie_->child_indicator_bits_->readBit(pos)) {
			append(label, pos);
			if ((label == kTerminator)
			&& !trie_->isEndofNode(pos))
				is_at_terminator_ = true;
			is_valid_ = true;
			return;
		}
		append(label, pos);
		level++;
    }
    assert(false); // shouldn't reach here
}

void LoudsSparse::Iter::moveToRightMostKey() {
    if (key_len_ == 0) {
	position_t pos = trie_->getFirstLabelPos(start_node_num_);
	pos = trie_->getLastLabelPos(start_node_num_);
	label_t label = trie_->labels_->read(pos);
	append(label, pos);
    }

    level_t level = key_len_ - 1;
    position_t pos = pos_in_trie_[level];
    label_t label = trie_->labels_->read(pos);

    if (!trie_->child_indicator_bits_->readBit(pos)) {
	if ((label == kTerminator)
	    && !trie_->isEndofNode(pos))
	    is_at_terminator_ = true;
	is_valid_ = true;
	return;
    }
	
    while (level < trie_->getHeight()) {
	position_t node_num = trie_->getChildNodeNum(pos);
	pos = trie_->getLastLabelPos(node_num);
	label = trie_->labels_->read(pos);
	// if trie branch terminates
	if (!trie_->child_indicator_bits_->readBit(pos)) {
	    append(label, pos);
	    if ((label == kTerminator)
		&& !trie_->isEndofNode(pos))
		is_at_terminator_ = true;
	    is_valid_ = true;
	    return;
	}
	append(label, pos);
	level++;
    }
    assert(false); // shouldn't reach here
}

void LoudsSparse::Iter::operator ++(int) {
    assert(key_len_ > 0);
    is_at_terminator_ = false;
    position_t pos = pos_in_trie_[key_len_ - 1];
    pos++;
    while (pos >= trie_->louds_bits_->numBits() || trie_->louds_bits_->readBit(pos)) {
	key_len_--;
	if (key_len_ == 0) {
	    is_valid_ = false;
#ifdef DEBUG_LOUDS_SPARSE
	    std::cout << "sparse ++ " << " is_valid_ = false " << "key_len_:" << key_len_ << " " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
#endif //DEBUG_LOUDS_SPARSE
	    return;
	}
	pos = pos_in_trie_[key_len_ - 1];
	pos++;
    }
    set(key_len_ - 1, pos);
    return moveToLeftMostKey();
}

void LoudsSparse::Iter::operator --(int) {
    assert(key_len_ > 0);
    is_at_terminator_ = false;
    position_t pos = pos_in_trie_[key_len_ - 1];
    if (pos == 0) {
	is_valid_ = false;
	return;
    }
    while (trie_->louds_bits_->readBit(pos)) {
	key_len_--;
	if (key_len_ == 0) {
	    is_valid_ = false;
	    return;
	}
	pos = pos_in_trie_[key_len_ - 1];
    }
    pos--;
    set(key_len_ - 1, pos);
    return moveToRightMostKey();
}

} // namespace surf

#endif // LOUDSSPARSE_CC_
