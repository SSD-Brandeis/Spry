#ifndef SURF_CC_
#define SURF_CC_


#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

#include "surf.hpp"

namespace surf {

// YCHUANG ADDED START
class SuRF;
class SuRF_RDF;

using VP = std::vector<std::pair<int,SuRF*>>;
using VMP = std::vector<std::unordered_map<uint64_t,std::pair<int, SuRF*>>>;
using pss = std::pair<std::string, std::string>;
using vpss = std::vector<std::pair<std::string, std::string>>;

// YCHUANG ADDED START
SuRF_Env* SuRF_Env::surf_env_ptr;
SuRF_Env* SuRF_Env::getInstance(){
    if(surf_env_ptr == NULL){
        surf_env_ptr = new SuRF_Env();
    }
    return surf_env_ptr;
}

// void SuRF::create(const std::vector<std::string>& keys, std::vector<bool> &left_parentheses, std::vector<bool> &right_parentheses,
// 		  const bool include_dense, const uint32_t sparse_dense_ratio,
// 		  const SuffixType suffix_type,
//           const level_t hash_suffix_len, const level_t real_suffix_len,
//           const bool flag_build_until_unique, const uint16_t max_num_level) {
void SuRF::create(const std::vector<std::string>& keys, std::vector<bool> &left_parentheses, std::vector<bool> &right_parentheses,
		  const bool include_dense, const uint32_t sparse_dense_ratio,
		  const SuffixType suffix_type,
          const level_t hash_suffix_len, const level_t real_suffix_len,
          const uint16_t max_num_level) {

    builder_ = new SuRFBuilder(include_dense, sparse_dense_ratio,
                              suffix_type, hash_suffix_len, real_suffix_len);
    // builder_->build(keys);
    // builder_->build(keys, left_parentheses, right_parentheses, flag_build_until_unique, max_num_level);
    builder_->build(keys, left_parentheses, right_parentheses, max_num_level);
    louds_dense_ = new LoudsDense(builder_);
    // louds_dense_->setMaxNumLevel(max_num_level);
    louds_sparse_ = new LoudsSparse(builder_);
    // louds_sparse_->setMaxNumLevel(max_num_level);
    iter_ = SuRF::Iter(this);
    delete builder_;
}
// YCHUANG ADDED END

void SuRF::create(const std::vector<std::string>& keys, 
		  const bool include_dense, const uint32_t sparse_dense_ratio,
		  const SuffixType suffix_type,
                  const level_t hash_suffix_len, const level_t real_suffix_len) {
    builder_ = new SuRFBuilder(include_dense, sparse_dense_ratio,
                              suffix_type, hash_suffix_len, real_suffix_len);
    builder_->build(keys);
    louds_dense_ = new LoudsDense(builder_);
    louds_sparse_ = new LoudsSparse(builder_);
    iter_ = SuRF::Iter(this);
    delete builder_;
}

bool SuRF::lookupKey(const std::string& key) const {
    position_t connect_node_num = 0;
    bool dense_rst = louds_dense_->lookupKey(key, connect_node_num);
    std::cout << "****** " << "Dense result: " << dense_rst << " " << " Connect node num: " << connect_node_num
          << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    if (!dense_rst){
	    return false;
    }else if (connect_node_num != 0){
        bool sparse_rst = louds_sparse_->lookupKey(key, connect_node_num);
	    std::cout << "****** " << "Sparse result: " << sparse_rst << " " << " Connect node num: " << connect_node_num
          << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        return sparse_rst;
    }
    return true;
}

// YCHUANG ADDED START
SuRF::Iter SuRF::moveToNextCommonPrefixKey(const std::string& key) const {
    SuRF::Iter iter(this);
    iter.could_be_fp_ = louds_dense_->moveToNextCommonPrefixKey(key, iter.dense_iter_);

    if (!iter.dense_iter_.isValid())
	return iter;
    if (iter.dense_iter_.isComplete())
	return iter;

    
    if (!iter.dense_iter_.isSearchComplete()) {
        iter.passToSparse();
        iter.could_be_fp_ = louds_sparse_->moveToNextCommonPrefixKey(key, iter.sparse_iter_);
        if (!iter.sparse_iter_.isValid()){
            iter.incrementDenseIter();
        }
	    return iter;
    } else if (!iter.dense_iter_.isMoveLeftComplete()) {
        iter.passToSparse();
        iter.sparse_iter_.moveToLeftMostKey();
        return iter;
    }

    assert(false); // shouldn't reach here
    return iter;
}
// YCHUANG ADDED END

SuRF::Iter SuRF::moveToKeyGreaterThan(const std::string& key, const bool inclusive) const {
    SuRF::Iter iter(this);
    // iter.could_be_fp_ = louds_dense_->moveToKeyGreaterThan(key, inclusive, iter.dense_iter_);
    iter.could_be_fp_ = louds_dense_->moveToKeyGreaterThan(key, iter.dense_iter_);

    if (!iter.dense_iter_.isValid())
	return iter;
    if (iter.dense_iter_.isComplete())
	return iter;

    if (!iter.dense_iter_.isSearchComplete()) {
        iter.passToSparse();
        iter.could_be_fp_ = louds_sparse_->moveToKeyGreaterThan(key, inclusive, iter.sparse_iter_);
        if (!iter.sparse_iter_.isValid())
            iter.incrementDenseIter();
        return iter;
    } else if (!iter.dense_iter_.isMoveLeftComplete()) {
        iter.passToSparse();
        iter.sparse_iter_.moveToLeftMostKey();
        return iter;
    }

    assert(false); // shouldn't reach here
    return iter;
}

// SuRF::Iter SuRF::moveToKeyLessThan(const std::string& key, const bool inclusive) const {
//     SuRF::Iter iter = moveToKeyGreaterThan(key, false);
//     if (!iter.isValid()) {
//         iter = moveToLast();
//         return iter;
//     }
//     if (!iter.getFpFlag()) {
//         iter--;
//         if (lookupKey(key))
//             iter--;
//     }
//     return iter;
// }

SuRF::Iter SuRF::moveToFirst() const {
    SuRF::Iter iter(this);
    if (louds_dense_->getHeight() > 0) {
	iter.dense_iter_.setToFirstLabelInRoot();
	iter.dense_iter_.moveToLeftMostKey();
	if (iter.dense_iter_.isMoveLeftComplete())
	    return iter;
	iter.passToSparse();
	iter.sparse_iter_.moveToLeftMostKey();
    } else {
	iter.sparse_iter_.setToFirstLabelInRoot();
	iter.sparse_iter_.moveToLeftMostKey();
    }
    return iter;
}

SuRF::Iter SuRF::moveToLast() const {
    SuRF::Iter iter(this);
    if (louds_dense_->getHeight() > 0) {
	iter.dense_iter_.setToLastLabelInRoot();
	iter.dense_iter_.moveToRightMostKey();
	if (iter.dense_iter_.isMoveRightComplete())
	    return iter;
	iter.passToSparse();
	iter.sparse_iter_.moveToRightMostKey();
    } else {
	iter.sparse_iter_.setToLastLabelInRoot();
	iter.sparse_iter_.moveToRightMostKey();
    }
    return iter;
}

bool SuRF::lookupRange(const std::string& left_key, const bool left_inclusive, 
		       const std::string& right_key, const bool right_inclusive) {
    iter_.clear();
    // louds_dense_->moveToKeyGreaterThan(left_key, left_inclusive, iter_.dense_iter_);
    louds_dense_->moveToKeyGreaterThan(left_key, iter_.dense_iter_);
    if (!iter_.dense_iter_.isValid()) return false;
    if (!iter_.dense_iter_.isComplete()) {
	if (!iter_.dense_iter_.isSearchComplete()) {
	    iter_.passToSparse();
	    louds_sparse_->moveToKeyGreaterThan(left_key, left_inclusive, iter_.sparse_iter_);
	    if (!iter_.sparse_iter_.isValid()) {
		iter_.incrementDenseIter();
	    }
	} else if (!iter_.dense_iter_.isMoveLeftComplete()) {
	    iter_.passToSparse();
	    iter_.sparse_iter_.moveToLeftMostKey();
	}
    }
    if (!iter_.isValid()) return false;
    int compare = iter_.compare(right_key);
    if (compare == kCouldBePositive)
	return true;
    if (right_inclusive)
	return (compare <= 0);
    else
	return (compare < 0);
}

uint64_t SuRF::approxCount(const SuRF::Iter* iter, const SuRF::Iter* iter2) {
    if (!iter->isValid() || !iter2->isValid()) return 0;
    position_t out_node_num_left = 0, out_node_num_right = 0;
    uint64_t count = louds_dense_->approxCount(&(iter->dense_iter_),
					       &(iter2->dense_iter_),
					       out_node_num_left,
					       out_node_num_right);
    count += louds_sparse_->approxCount(&(iter->sparse_iter_),
					&(iter2->sparse_iter_),
					out_node_num_left,
					out_node_num_right);
    return count;
}

uint64_t SuRF::approxCount(const std::string& left_key,
			   const std::string& right_key) {
    iter_.clear(); iter2_.clear();
    iter_ = moveToKeyGreaterThan(left_key, true);
    if (!iter_.isValid()) return 0;
    iter2_ = moveToKeyGreaterThan(right_key, true);
    if (!iter2_.isValid())
	iter2_ = moveToLast();

    return approxCount(&iter_, &iter2_);
}

uint64_t SuRF::serializedSize() const {
    return (louds_dense_->serializedSize()
	    + louds_sparse_->serializedSize());
}

uint64_t SuRF::getMemoryUsage() const {
    return (sizeof(SuRF) + louds_dense_->getMemoryUsage() + louds_sparse_->getMemoryUsage());
}

level_t SuRF::getHeight() const {
    return louds_sparse_->getHeight();
}

level_t SuRF::getSparseStartLevel() const {
    return louds_sparse_->getStartLevel();
}

//============================================================================

void SuRF::Iter::clear() {
    dense_iter_.clear();
    sparse_iter_.clear();
}

bool SuRF::Iter::getFpFlag() const {
    return could_be_fp_;
}

bool SuRF::Iter::isValid() const {
    return dense_iter_.isValid() 
	&& (dense_iter_.isComplete() || sparse_iter_.isValid());
}

int SuRF::Iter::compare(const std::string& key) const {
    assert(isValid());
    int dense_compare = dense_iter_.compare(key);
    if (dense_iter_.isComplete() || dense_compare != 0) 
	return dense_compare;
    return sparse_iter_.compare(key);
}

std::string SuRF::Iter::getKey() const {
    if (!isValid())
	return std::string();
    if (dense_iter_.isComplete())
	return dense_iter_.getKey();
    return dense_iter_.getKey() + sparse_iter_.getKey();
}

int SuRF::Iter::getSuffix(word_t* suffix) const {
    if (!isValid())
	return 0;
    if (dense_iter_.isComplete())
	return dense_iter_.getSuffix(suffix);
    return sparse_iter_.getSuffix(suffix);
}

// YCHUANG ADDED START
bool SuRF::Iter::getLeftParenthesis() const {
    if (!isValid())
        return false;
    if (dense_iter_.isComplete())
        return dense_iter_.getLeftParenthesis();
    return sparse_iter_.getLeftParenthesis();
}
bool SuRF::Iter::getRightParenthesis() const {
    if (!isValid())
        return false;
    if (dense_iter_.isComplete())
        return dense_iter_.getRightParenthesis();
    return sparse_iter_.getRightParenthesis();
}
// YCHUANG ADDED END

std::string SuRF::Iter::getKeyWithSuffix(unsigned* bitlen) const {
    *bitlen = 0;
    if (!isValid())
	return std::string();
    if (dense_iter_.isComplete())
	return dense_iter_.getKeyWithSuffix(bitlen);
    return dense_iter_.getKeyWithSuffix(bitlen) + sparse_iter_.getKeyWithSuffix(bitlen);
}

void SuRF::Iter::passToSparse() {
    sparse_iter_.setStartNodeNum(dense_iter_.getSendOutNodeNum());
}

bool SuRF::Iter::incrementDenseIter() {
    if (!dense_iter_.isValid()) 
	return false;

    dense_iter_++;
    if (!dense_iter_.isValid()) 
	return false;
    if (dense_iter_.isMoveLeftComplete()) 
	return true;

    passToSparse();
    sparse_iter_.moveToLeftMostKey();
    return true;
}

bool SuRF::Iter::incrementSparseIter() {
    if (!sparse_iter_.isValid()) 
	return false;
    sparse_iter_++;
    return sparse_iter_.isValid();
}

bool SuRF::Iter::operator ++(int) {
    if (!isValid()) 
	return false;
    if (incrementSparseIter()) 
	return true;
    return incrementDenseIter();
}

bool SuRF::Iter::decrementDenseIter() {
    if (!dense_iter_.isValid()) 
	return false;

    dense_iter_--;
    if (!dense_iter_.isValid()) 
	return false;
    if (dense_iter_.isMoveRightComplete()) 
	return true;

    passToSparse();
    sparse_iter_.moveToRightMostKey();
    return true;
}

bool SuRF::Iter::decrementSparseIter() {
    if (!sparse_iter_.isValid()) 
	return false;
    sparse_iter_--;
    return sparse_iter_.isValid();
}

bool SuRF::Iter::operator --(int) {
    if (!isValid()) 
	return false;
    if (decrementSparseIter()) 
	return true;
    return decrementDenseIter();
}


//YCHUANG ADDED START
// SuRF* SuRF::rangesToSurf(std::vector<std::pair<std::string, std::string>> ranges, size_t surf_key_length_in_bytes, 
//                 surf::SuffixType kSuffixType, 
//                 surf::level_t hash_suffix_len, surf::level_t real_suffix_len,
//                 bool include_dense, uint32_t sparse_dense_ratio, 
//                 bool flag_build_until_unique){
SuRF* SuRF::rangesToSurf(std::vector<std::pair<std::string, std::string>> ranges, size_t surf_key_length_in_bytes, 
                surf::SuffixType kSuffixType, 
                surf::level_t hash_suffix_len, surf::level_t real_suffix_len,
                bool include_dense, uint32_t sparse_dense_ratio, bool flag_allow_boundary_overlapped){

    std::sort(ranges.begin(), ranges.end());
    size_t len = ranges.size();
    assert(len > 0);
    bool flag_merge = false;
    if(len > 1){
        for(size_t i = 0; i < len-1; i++){
            // std::cout << "i = " << i << " len=" << len << " " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
            // assert(("cannot have overlapped, ranges[i].second shall <= ranges[i+1].first", ranges[i].second <= ranges[i+1].first));
            flag_merge = (flag_allow_boundary_overlapped == true)? 
                (ranges[i].second > ranges[i+1].first):(ranges[i].second >= ranges[i+1].first);
            if(flag_merge){break;}
        }
    }
    if(flag_merge){
        //doing merging
        std::vector<pss> ranges_merged;
        ranges_merged.push_back(ranges[0]);
        for (size_t i = 1; i < len; i++)
        {   
            bool flag_shall_be_merged = (flag_allow_boundary_overlapped == true)?
                (ranges_merged.back().second > ranges[i].first):
                (ranges_merged.back().second >= ranges[i].first);

            if(flag_shall_be_merged){
                ranges_merged.back().second = std::max(ranges_merged.back().second, ranges[i].second);
            }else{
                ranges_merged.push_back(ranges[i]);
            }
        }
        ranges = ranges_merged;
    }

    len = ranges.size();
    assert(len > 0);
    if(len > 1){
        for(size_t i = 0; i < len-1; i++){
            assert(("cannot have overlapped, ranges[i].second shall <= ranges[i+1].first", ranges[i].second <= ranges[i+1].first));
        }
    }

    std::vector<std::string> keys;
    std::vector<bool> left_parentheses;
    std::vector<bool> right_parentheses;
    for(size_t i = 0; i < len; i++){
        // std::string key_start = int_to_bytes(ranges[i].first, surf_key_length_in_bytes);
        // std::string key_end = int_to_bytes(ranges[i].second, surf_key_length_in_bytes);
        std::string key_start = ranges[i].first;
        std::string key_end = ranges[i].second;
        if(key_start.size() > surf_key_length_in_bytes){
            key_start = key_start.substr(0, surf_key_length_in_bytes);
        }
        if(key_end.size() > surf_key_length_in_bytes){
            key_end = key_end.substr(0, surf_key_length_in_bytes);
        }
        if(keys.size() > 0 && keys.back() == key_start){
            left_parentheses.back() = true;
        }else{
            keys.push_back(key_start);
            left_parentheses.push_back(true);
            right_parentheses.push_back(false);
        }
        keys.push_back(key_end);
        left_parentheses.push_back(false);
        right_parentheses.push_back(true);
    }
    assert(keys.size() == left_parentheses.size());
    assert(keys.size() == right_parentheses.size());


    // string_length
    size_t string_len = 0;
    for(auto &key: keys){
        if(key.length() > string_len){
            string_len = key.length();
        }
    }
    

    // bool flag_build_until_unique = false;
    //One shall clip the key into the prefix of length "max_num_level"
    // must >= max([len(key) for key in keys])  --- maximum key length
    // uint16_t max_num_level = 5; 
    uint16_t max_num_level = string_len; 
    // bool include_dense = true;
    // uint32_t sparse_dense_ratio = 16;
    // uint32_t sparse_dense_ratio = 1;
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

    return surf_;
}

std::vector<std::pair<std::string, std::string>> SuRF::surfToRanges(SuRF* surf_){
    std::vector<std::string> keys;
    std::vector<bool> left_parentheses;
    std::vector<bool> right_parentheses;

    SuRF::Iter iter = surf_->moveToFirst();
    while(iter.isValid()){
        std::string key = iter.getKey();
        bool left_parenthesis = iter.getLeftParenthesis();
        bool right_parenthesis = iter.getRightParenthesis();
        keys.push_back(key);
        left_parentheses.push_back(left_parenthesis);
        right_parentheses.push_back(right_parenthesis);
        iter++;
    }

    std::vector<std::pair<std::string, std::string>> ranges;
    int len = keys.size();
    std::string start;
    for(int i = 0; i < len-1; i++){
        if(left_parentheses[i]){
            assert(left_parentheses[i] == true && right_parentheses[i+1] == true);
            std::string start = keys[i];
            std::string end = keys[i+1];
            ranges.push_back(std::make_pair(start, end));
        }
    }

    return ranges;
}


// SuRF_RDF class member functions
SuRF_RDF::SuRF_RDF(SuRF_RDF::RDF_MODE rdf_mode_in) {
    rdf_mode = rdf_mode_in;
}

// SuRF_RDF::SuRF_RDF(SuRF_RDF &surf_rdf_in) {
//     RDF_MODE rdf_mode_in = surf_rdf_in.getRDFMode();
    // // std::vector<SuRF*> level_surf_rdf_in = surf_rdf.getLevelRDF();
    // level_t num_level = surf_rdf_in.getNumberOfTotalLevels();
    // if(rdf_mode_in == PER_LEVEL){
    //     rdf_mode = PER_LEVEL;
    //     auto level_surf_rdf_in = surf_rdf_in.getLevelRDF();
    //     for(auto fd_rdf: level_surf_rdf_in){
    //         int num_of_ranges = fd_rdf.first;
    //         SuRF* surf_orig = fd_rdf.second;
    //         char* serizlid_data = surf_orig->serialize();
    //         SuRF* surf_new = SuRF::deSerialize(serizlid_data);
    //         level_surf_rdf.push_back(std::make_pair(num_of_ranges, surf_new));
    //     }
    // }else if(rdf_mode_in == PER_FILE){
    //     rdf_mode = PER_FILE;
    //     auto level_file_surf_rdf_in = surf_rdf_in.getLevelFileRDF();
    //     for(auto fd_rdf: level_file_surf_rdf_in){
    //         std::unordered_map<uint64_t,std::pair<int, SuRF*>> level_file_surf_rdf_in_at_level;
    //         auto it = fd_rdf.begin();
    //         auto end = fd_rdf.end();
    //         for(; it != end; it++){
    //             auto rd_surf = it->second;
    //         // for(auto &[fd, rd_surf]: fd_rdf){
    //             int num_of_ranges = rd_surf.first;
    //             SuRF* surf_orig = rd_surf.second;
    //             char* serizlid_data = surf_orig->serialize();
    //             SuRF* surf_new = SuRF::deSerialize(serizlid_data);
    //             level_file_surf_rdf_in_at_level[fd] = std::make_pair(num_of_ranges, surf_new);
    //         }
    //         level_file_surf_rdf.push_back(level_file_surf_rdf_in_at_level);
    //     }
    // }else{
    //     assert(false);
    // }

    // numbers_of_ranges_in_RDF_log = surf_rdf_in.getNumbersOfRangesInRDFLog();
    // memory_usage_in_RDF_log = surf_rdf_in.getMemoryUsageInRDFLog();
// }

SuRF_RDF::SuRF_RDF(const VMP &level_file_surf_rdf_in,
        const std::vector<int> &numbers_of_ranges_in_RDF_log_in, const std::vector<int> &memory_usage_in_RDF_log_in,
        const SuRF_RDF::RDF_MODE rdf_mode_in) {
    assert(rdf_mode_in == PER_FILE);
    rdf_mode= rdf_mode_in;
    level_file_surf_rdf = level_file_surf_rdf_in;
    // level_surf_rdf_numbers_of_ranges = level_surf_rdf_numbers_of_ranges_in;
    numbers_of_ranges_in_RDF_log = numbers_of_ranges_in_RDF_log_in;
    memory_usage_in_RDF_log = memory_usage_in_RDF_log_in;
}

SuRF_RDF::SuRF_RDF(const VP &level_surf_rdf_in,
        const std::vector<int> &numbers_of_ranges_in_RDF_log_in, const std::vector<int> &memory_usage_in_RDF_log_in,
        const SuRF_RDF::RDF_MODE rdf_mode_in) {
    assert(rdf_mode_in == PER_LEVEL);
    rdf_mode = rdf_mode_in;
    level_surf_rdf = level_surf_rdf_in;
    // level_surf_rdf_numbers_of_ranges = level_surf_rdf_numbers_of_ranges_in;
    numbers_of_ranges_in_RDF_log = numbers_of_ranges_in_RDF_log_in;
    memory_usage_in_RDF_log = memory_usage_in_RDF_log_in;
}

// SuRF_RDF::SuRF_RDF(VMP &level_file_surf_rdf_in, 
//         std::vector<int> &numbers_of_ranges_in_RDF_log_in, std::vector<int> &memory_usage_in_RDF_in,
//         RDF_MODE rdf_mode_in = PER_FILE) {
//     rdf_mode = PER_FILE;
//     assert(rdf_mode_in == PER_FILE);
//     level_file_surf_rdf = level_file_surf_rdf_in;
//     // level_surf_rdf_numbers_of_ranges = level_surf_rdf_numbers_of_ranges_in;
//     numbers_of_ranges_in_RDF_log = numbers_of_ranges_in_RDF_log_in;
//     memory_usage_in_RDF = memory_usage_in_RDF_in;
// }

// SuRF_RDF(SuRF *surf) {
//     surf_ = surf;
// }
SuRF_RDF::~SuRF_RDF() {
    int num_level = getNumberOfTotalLevels();
    if(rdf_mode == PER_LEVEL){
        for(int i = 0; i < num_level; i++){
            if(level_surf_rdf[i].second != NULL){
                delete level_surf_rdf[i].second;
            }
        }
    }else if(rdf_mode == PER_FILE){       
        for(int i = 0; i < num_level; i++){
            auto it = level_file_surf_rdf[i].begin();
            while(it != level_file_surf_rdf[i].end()){
                delete ((it->second).second);
                it++;
            }
        }
    }else{
        assert(false);
    }
}
// SuRF* SuRF_RDF::getSurf() {
//     return surf_;
// }

// bool SuRF_RDF::isAliveAfterSuRFTopLevelRDFilter(long long key){
//     assert(rdf_mode == PER_LEVEL);

//     bool overlapping = false;
//     surf::SuRF *surf_ = level_surf_rdf[0].second;
//     SuRF::Iter iter = surf_->moveToNextCommonPrefixKey(key);
//     if(iter.isValid()){
//         std::string key_found = iter.getKey();
//         if(key_found.size() < key_len_in_bytes){
//             overlapping = (iter.getRightParenthesis() == true);
//         }else{
//             overlapping = (key_found != key) && (iter.getRightParenthesis() == true);
//         }
//     }
// }
// bool isAliveAfterSuRFLevelFileRDFilter(long long key, uint64_t fd){
//     assert(rdf_mode == PER_FILE);
    
//     bool overlapping = false;
//     SuRF::Iter iter = surf_->moveToNextCommonPrefixKey(key);
//     if(iter.isValid()){
//         std::string key_found = iter.getKey();
//         if(key_found.size() < key_len_in_bytes){
//             overlapping = (iter.getRightParenthesis() == true);
//         }else{
//             overlapping = (key_found != key) && (iter.getRightParenthesis() == true);
//         }
//     }
// }

// void SuRF_RDF::insertRangeDeleteToLevel0(uint64_t file_num, std::vector<pss> &range_delete_list_in, std::vector<uint64_t> exist_level0_file_nums){
void SuRF_RDF::insertRangeDeleteToLevel0(uint64_t file_num, std::vector<pss> &range_delete_list_in){
    if(range_delete_list_in.size() == 0){return;}

    if(rdf_mode == PER_LEVEL){
        if(level_surf_rdf.size() == 0){
            //TODO:  init SuRF_RDF with following parameters set
            int key_len_in_bytes = 10;
            surf::level_t hash_suffix_len = 0;
            surf::level_t real_suffix_len = 0;
            bool include_dense = true;
            uint32_t sparse_dense_ratio = 16;
            // bool flag_build_until_unique = false;

            bool flag_allow_boundary_overlapped = false;
            //
            SuRF* surf = SuRF::rangesToSurf(range_delete_list_in, key_len_in_bytes, surf::SuffixType::kReal, 
                            hash_suffix_len, real_suffix_len, include_dense, 
                            sparse_dense_ratio, flag_allow_boundary_overlapped);


            level_surf_rdf.push_back(std::make_pair(range_delete_list_in.size(), surf));
        }else{
            SuRF* surf = level_surf_rdf[0].second;
            std::vector<pss> range_list = SuRF_RDF::mergeRanges(surf->surfToRanges(surf), range_delete_list_in);
            delete level_surf_rdf[0].second;
            
            //TODO:  init SuRF_RDF with following parameters set
            int key_len_in_bytes = 10;
            surf::level_t hash_suffix_len = 0;
            surf::level_t real_suffix_len = 0;
            bool include_dense = true;
            uint32_t sparse_dense_ratio = 16;
            // bool flag_build_until_unique = false;
            
            bool flag_allow_boundary_overlapped = false;
            //
            SuRF* surf_next = SuRF::rangesToSurf(range_delete_list_in, key_len_in_bytes, surf::SuffixType::kReal, 
                            hash_suffix_len, real_suffix_len, include_dense, 
                            sparse_dense_ratio, flag_allow_boundary_overlapped);

            level_surf_rdf[0].second = surf_next;
            level_surf_rdf[0].first = range_list.size();
        }
    }else if(rdf_mode == PER_FILE){
        if(level_file_surf_rdf.size() == 0){
            std::unordered_map<uint64_t,std::pair<int, SuRF*>> level_file_surf_rdf_in; // fd -> (# RD, surf)
            // SuRF* surf = SuRF::rangesToSurf(range_delete_list_in);
            // level_file_surf_rdf_in[file_num] = std::make_pair(range_delete_list_in.size(), surf);
            // level_file_surf_rdf.push_back(level_file_surf_rdf_in);
                // TODO: Get flag_allow_boundary_overlapped -> get from systemVerifier args
                bool flag_allow_boundary_overlapped = false;
                this->insertRangesAtLevelOfFd(0, file_num, range_delete_list_in, flag_allow_boundary_overlapped);
        }else{
            std::unordered_map<uint64_t,std::pair<int, SuRF*>> &level_file_surf_rdf_in = level_file_surf_rdf[0];
            if(level_file_surf_rdf_in.find(file_num) != level_file_surf_rdf_in.end()){
                std::cout << "Error. Flusing in file shall have unique file_num and greater than previous ones" << std::endl;
            }else{
                // SuRF* surf = SuRF::rangesToSurf(range_delete_list_in);
                // level_file_surf_rdf[0][file_num] = std::make_pair(range_delete_list_in.size(), surf);
                // TODO: Get flag_allow_boundary_overlapped -> get from systemVerifier args
                bool flag_allow_boundary_overlapped = false;
                this->insertRangesAtLevelOfFd(0, file_num, range_delete_list_in, flag_allow_boundary_overlapped);
            }
        }
    }else{
        assert(false);
    }
}

void SuRF_RDF::directMoveFileToLevel(uint64_t fd, uint32_t src_level, uint32_t dst_level){
    if(rdf_mode == PER_LEVEL){
        if(src_level >= level_surf_rdf.size()){
            return;
        }
        // assert(src_level < level_surf_rdf.size());
        if(level_surf_rdf[src_level].second == NULL){return;}
        // assert(level_surf_rdf[src_level].second != NULL);
        while(dst_level >= level_surf_rdf.size()){
            std::pair<int, SuRF*> tmp;
            level_surf_rdf.push_back(tmp);
        }
        // assert(dst_level < level_surf_rdf.size());
        std::cerr << "Not Implemented Yet, Requiured: file_boundary" << std::endl;
        assert(false);
    }else if(rdf_mode == PER_FILE){
        if(src_level >= level_file_surf_rdf.size()){return;}
        // assert(src_level < level_file_surf_rdf.size());
        if(level_file_surf_rdf[src_level].count(fd) == 0){return;}
        int num_ragnes_in_surf = level_file_surf_rdf[src_level][fd].first;
        if(num_ragnes_in_surf <= 0){
            assert(num_ragnes_in_surf > 0);
        }
        // assert(level_file_surf_rdf[src_level].count(fd) > 0);
        while(dst_level >= level_file_surf_rdf.size()){
            std::unordered_map<uint64_t,std::pair<int, SuRF*>> tmp;
            level_file_surf_rdf.push_back(tmp);
        }
        // assert(dst_level < level_file_surf_rdf.size());
        level_file_surf_rdf[dst_level][fd] = level_file_surf_rdf[src_level][fd];
        level_file_surf_rdf[src_level].erase(fd);
    }else{
        assert(false);
    }
}


std::vector<pss> SuRF_RDF::getRangeTombstonesAtLevelOfFd(uint32_t src_level, uint64_t fd){
    assert(rdf_mode == surf::SuRF_RDF::RDF_MODE::PER_FILE);
    if(src_level >= level_file_surf_rdf.size()){
        return std::vector<pss>();
    }
    // assert(src_level < level_file_surf_rdf.size());
    if(level_file_surf_rdf[src_level].count(fd) == 0){
        return std::vector<pss>();
    }   
    // assert(level_file_surf_rdf[src_level].count(fd) > 0);

#ifdef DEBUG_SURF_COMPACTION
    std::cout << "src_level = " << src_level << " fd = " << fd << " " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
#endif
    int num_ranges_in_surf = level_file_surf_rdf[src_level][fd].first;

    if(num_ranges_in_surf <= 0){
        assert(num_ranges_in_surf > 0);
        std::cout << "Error: num_ranges_in_surf = " << num_ranges_in_surf << " " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
    }
    //std::cout << " num_ranges_in_surf = " << num_ranges_in_surf << " " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
    //std::cout << " num_ranges_in_surf = " << level_file_surf_rdf[src_level][fd].first << " " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
    SuRF* surf = level_file_surf_rdf[src_level][fd].second;
    assert(surf != NULL);
    std::vector<pss> range_list = SuRF::surfToRanges(surf);

    return range_list;
}
void SuRF_RDF::removeSuRFAtLevelOfFd(uint32_t src_level, uint64_t fd){
    assert(rdf_mode == surf::SuRF_RDF::RDF_MODE::PER_FILE);
    if(src_level >= level_file_surf_rdf.size()){return;}
    // assert(src_level < level_file_surf_rdf.size());
    if(level_file_surf_rdf[src_level].count(fd) == 0){return;}
    // assert(level_file_surf_rdf[src_level].count(fd) > 0);

    delete level_file_surf_rdf[src_level][fd].second;
    level_file_surf_rdf[src_level].erase(fd);
}
// void SuRF_RDF::shiftRDFToOutputLevel(SuRFCompactionMovingRDInfo *surf__compaction_moving_RD_vector){
void SuRF_RDF::insertRangesAtLevelOfFd(uint32_t level, uint64_t fd, std::vector<pss> &ranges, bool flag_allow_boundary_overlapped){
    assert(ranges.size() > 0);
    if(ranges.size() <= 0){
        std::cout << "Error: ranges.size() = " << ranges.size() << " " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
    }

    //TODO:  init SuRF_RDF with following parameters set
    //int key_len_in_bytes = 12;
    //surf::level_t hash_suffix_len = 0;
    //surf::level_t real_suffix_len = 0;
    //bool include_dense = true;
    //uint32_t sparse_dense_ratio = 16;
    surf::SuRF_Env *_surf_env = surf::SuRF_Env::getInstance();
    int key_len_in_bytes = _surf_env->getSuRFKeyLenInBytes();
    surf::level_t hash_suffix_len = _surf_env->getSuRFHashSuffixLen();
    surf::level_t real_suffix_len = _surf_env->getSuRFRealSuffixLen();
    bool include_dense = _surf_env->getSuRFIncludeDense();
    uint32_t sparse_dense_ratio = _surf_env->getSuRFSparseDenseRatio();
    // bool flag_build_until_unique = false;
    //

    assert(rdf_mode == surf::SuRF_RDF::RDF_MODE::PER_FILE);
    // if(level == level_file_surf_rdf.size()){
    while(level >= level_file_surf_rdf.size()){
        std::unordered_map<uint64_t,std::pair<int, SuRF*>> tmp;
        level_file_surf_rdf.push_back(tmp);
    }
    assert(level < level_file_surf_rdf.size());
    assert(level_file_surf_rdf[level].count(fd) == 0);

    // bool flag_allow_boundary_overlapped = false;
    SuRF* surf_ = SuRF::rangesToSurf(ranges, key_len_in_bytes, surf::SuffixType::kReal, 
                            hash_suffix_len, real_suffix_len, include_dense, 
                            sparse_dense_ratio, flag_allow_boundary_overlapped);

    //std::cout << level << " " << fd << " " << " " << level_file_surf_rdf.size() << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    assert(ranges.size() > 0);
    level_file_surf_rdf[level][fd] = std::make_pair(ranges.size(), surf_);
}

void SuRF_RDF::deleteLastLevelIfEqualsBottomLevel(uint bottom_level){
// init();
// std::lock_guard<std::mutex> guard(update_mutex);
    if(rdf_mode == PER_LEVEL){
        while (level_surf_rdf.size() > 0 && level_surf_rdf.size()-1 >= bottom_level)
        {
            level_surf_rdf.pop_back();
        }
    }else if(rdf_mode == PER_FILE){
        while (level_file_surf_rdf.size() > 0 && level_file_surf_rdf.size()-1 >= bottom_level)
        {
            level_file_surf_rdf.pop_back();
        }
    }else{
        assert(false);
    }
}


std::vector<pss> SuRF_RDF::gatherSortedRangeTombstonesAndRemoveSuRF(std::vector<uint32_t> &src_level_list, std::vector<std::vector<uint64_t>> &src_fd_list2d, bool surf_flag__allow_range_boundary_overlapped){
    assert(src_level_list.size() == src_fd_list2d.size());

    std::vector<pss> range_tombstone_list_agg;

    size_t len = src_level_list.size();
    for(size_t i = 0; i < len; i++){
        uint32_t &src_level = src_level_list[i];
        std::vector<uint64_t> &src_fd_list = src_fd_list2d[i];
    
       for(auto &fd: src_fd_list){
            // std::vector<pss> range_tombstone_list = (this->surf__level_file_rdf_prime)->getRangeTombstonesAtLevelOfFd(src_level, fd);
            std::vector<pss> range_tombstone_list = this->getRangeTombstonesAtLevelOfFd(src_level, fd);

            // (this->surf__level_file_rdf_prime)->removeSuRFAtLevelOfFd(src_level, fd);
            this->removeSuRFAtLevelOfFd(src_level, fd);
            for(auto &range_tombstone: range_tombstone_list){
    #ifdef DEBUG_SURF_COMPACTION 
    std::cout << "range_tombstone = " << range_tombstone.first << " " << range_tombstone.second << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;  
    #endif
            range_tombstone_list_agg.push_back(range_tombstone);
            }
        }
    }
    sort(range_tombstone_list_agg.begin(), range_tombstone_list_agg.end());

            
    //merge        
    std::vector<pss> rd_merged;
    size_t len_rd = range_tombstone_list_agg.size();
//std::cout << "len_rd (range_tombstone_list_agg.size()): " << len_rd << " len_rd > 0: " << (len_rd > 0) << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    assert(len_rd > 0ULL);
    if(len_rd > 0){
    //std::cout << range_tombstone_list_agg[0].first << " " << range_tombstone_list_agg[0].second << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
        rd_merged.push_back(range_tombstone_list_agg[0]);
        for(size_t i_rd = 1; i_rd < len_rd; i_rd++){
            pss &rd = range_tombstone_list_agg[i_rd];
            pss &rd_last = rd_merged.back();
            if(surf_flag__allow_range_boundary_overlapped){
                if(rd_last.second > rd.first){
                    rd_last.second = std::max(rd_last.second, rd.second);
                }else{
                    rd_merged.push_back(rd);
                }
            }else{
                if(rd_last.second >= rd.first){
                    rd_last.second = std::max(rd_last.second, rd.second);
                }else{
                    rd_merged.push_back(rd);
                }
            }
        }
        // len_rd = rd_merged.size();
    }

    return rd_merged;
}
// void SuRF_RDF::shiftRDFToOutputLevel(std::vector<ROCKSDB_NAMESPACE::SuRFCompactionSourceLevelInfo> &src_level_info_list, uint32_t dst_level, std::vector<ROCKSDB_NAMESPACE::SuRFCompactionDestinationLevelInfo> &dst_level_info_list, bool surf_flag__allow_range_boundary_overlapped){
void SuRF_RDF::shiftRDFToOutputLevel(std::vector<pss> &rd_merged, uint32_t dst_level, std::vector<uint64_t> &dst_fd_list, std::vector<pss> &file_boundary_list, bool surf_flag__allow_range_boundary_overlapped){
            
    //merge
    size_t len_rd = rd_merged.size();
//std::cout << "len_rd (range_tombstone_list_agg.size()): " << len_rd << " len_rd > 0: " << (len_rd > 0) << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    assert(len_rd > 0ULL);
    if(len_rd > 0){

        size_t i_rd = 0;
        assert(dst_fd_list.size() == file_boundary_list.size());
        size_t len_dst = dst_fd_list.size();
        // for(auto &dst_level_info: dst_level_info_list){
        for(size_t i_dst = 0; i_dst < len_dst; i_dst++){
            if(i_rd >= len_rd){
                break;
            }           
            // uint64_t dst_fd = dst_level_info.fd;
            // pss file_boundary = dst_level_info.file_boundary;
            uint64_t dst_fd = dst_fd_list[i_dst];
            pss file_boundary = file_boundary_list[i_dst];
            //separate
            std::vector<pss> ranges_to_insert;
            if(surf_flag__allow_range_boundary_overlapped == true){
                while(i_rd < len_rd && rd_merged[i_rd].second <= file_boundary.first){
                    i_rd++;
                }
            }else{
                while(i_rd < len_rd && rd_merged[i_rd].second < file_boundary.first){
                    i_rd++;
                }
            }
            //TODO: check this part 
            while(i_rd < len_rd && rd_merged[i_rd].second <= file_boundary.second){
                //while(i_rd < len_rd && rd_merged[i_rd].second < file_boundary.second){
                pss range_in = std::make_pair(
                    std::max(rd_merged[i_rd].first, file_boundary.first),
                    std::min(rd_merged[i_rd].second, file_boundary.second)
                );
                ranges_to_insert.push_back(range_in);
                i_rd++;
            }
            if(i_rd < len_rd && rd_merged[i_rd].first < file_boundary.second){
                pss range_in = std::make_pair(
                    std::max(rd_merged[i_rd].first, file_boundary.first),
                    std::min(rd_merged[i_rd].second, file_boundary.second)
                );
                ranges_to_insert.push_back(range_in);
                // don't i_rd ++;
            }

            if(ranges_to_insert.size() > 0){
std::cout << "within SuRF:" << std::endl;
for(auto &rd: ranges_to_insert){
    std::cout << rd.first << " " << rd.second << std::endl;
}
                // (this->surf__level_file_rdf_prime)->insertRangesAtLevelOfFd(dst_level, dst_fd, ranges_to_insert, surf_flag__allow_range_boundary_overlapped);
                this->insertRangesAtLevelOfFd(dst_level, dst_fd, ranges_to_insert, surf_flag__allow_range_boundary_overlapped);
            }
            // (this->surf__level_file_rdf_prime)->insertRangesAtLevelOfFd(dst_level, dst_fd, ranges_to_insert, surf_flag__allow_range_boundary_overlapped);
        }
    // (this->surf__level_file_rdf_prime)->shiftRDFToOutputLevel(this->surf__compaction_moving_RD_vector);
    }
    // src_fd_list = 
    // std::sort(rd_list.begin(), rd_list.end()); 
    // (this->surf__level_file_rdf_prime)->shiftRDFToOutputLevel(fd_out, rd_list);
}





int SuRF_RDF::getNumberOfRangesAtIthLevel(int level){
    if(rdf_mode == PER_LEVEL){
        assert(level < level_surf_rdf.size());
        assert(level_surf_rdf[level].second != NULL);
        return level_surf_rdf[level].first;
    }else if(rdf_mode == PER_FILE){
        assert(level < level_file_surf_rdf.size());
        int num = 0;
        auto it = level_file_surf_rdf[level].begin();
        while(it != level_file_surf_rdf[level].end()){
            num += (it->second).first;
            it++;
        }
        return num;
    }
    assert(false);
    return -1;
}
int SuRF_RDF::getNumberOfTotalRanges(){
    int num = 0;
    int level = getNumberOfTotalLevels();

    for(int i = 0; i < level; i++){
        num += getNumberOfRangesAtIthLevel(i);
    }
    // if(rdf_mode == PER_LEVEL){
    //     for(int i = 0; i < level; i++){
    //         num += getNumberOfRangesAtIthLevel(i);
    //     }
    // }else{
    //     for(int i = 0; i < level; i++){
    //         num += getNumberOfRangesAtIthLevel(i);
    //     }
    // }
    return num;
}
uint64_t SuRF_RDF::getMemoryUsageAtIthLevel(int level){
    if(rdf_mode == PER_LEVEL){
        assert(level < level_surf_rdf.size());
        assert(level_surf_rdf[level].second != NULL);
        return level_surf_rdf[level].second->getMemoryUsage();
    }else if(rdf_mode == PER_FILE){
        assert(level < level_file_surf_rdf.size());
        assert(level_file_surf_rdf[level].size() != 0);
        uint64_t mem = 0;
        auto it = level_file_surf_rdf[level].begin();
        while(it != level_file_surf_rdf[level].end()){
            auto rd_surf = it->second;
            mem += (rd_surf.second)->getMemoryUsage();
            it++;
        }
        return mem;
    }
    assert(false);
    return -1;
}
uint64_t SuRF_RDF::getNumberOfTotalMemoryUsage(){
    uint64_t num = 0;
    int level = getNumberOfTotalLevels();
    for(int i = 0; i < level; i++){
        num += getMemoryUsageAtIthLevel(i);
    }
    // if(rdf_mode == PER_LEVEL){
    //     for(int i = 0; i < level; i++){
    //         num += getMemoryUsageAtIthLevel(i);
    //     }
    // }else if(rdf_mode == PER_FILE){
    //     for(int i = 0; i < level; i++){
    //         num += getMemoryUsageAtIthLevel(i);
    //     }
    // }
    return num;
}
void SuRF_RDF::logCurrentTotalNumbersOfRanges() {
    // std::cout << "Current total number of ranges: " << ranges.size() << std::endl;
    int num = getNumberOfTotalRanges();
    numbers_of_ranges_in_RDF_log.push_back(num);
}
void SuRF_RDF::logCurrentTotalMemoryUsage(){
    // std::cout << "Current total memory usage: " << surf_->getMemoryUsage() << std::endl;
    uint64_t num = getNumberOfTotalMemoryUsage();
    memory_usage_in_RDF_log.push_back(num);
}

int SuRF_RDF::getNumberOfTotalLevels(){        
    int num = 0;
    if(rdf_mode == PER_LEVEL){
        int len = level_surf_rdf.size();
        for(int i = 1; i < len; i++){
            if(level_surf_rdf[i].second != NULL){
                num = i+1;
            }
        }
    }else if(rdf_mode == PER_FILE){
        int len = level_file_surf_rdf.size();
        for(int i = 1; i < len; i++){
            if(level_file_surf_rdf[i].size() != 0){
                num = i+1;
            }
        }
    }else{
        assert(false);
    }
    // int len = level_surf_rdf.size();
    return num;
}


VP SuRF_RDF::getLevelRDF(){
    return level_surf_rdf;
}
std::pair<int, SuRF*> SuRF_RDF::getLevelRDFAtIthLevel(int level){
    assert(level < level_surf_rdf.size());
    return level_surf_rdf[level];
}
// std::vector<int> getLevelRDFNumbersOfRanges(){
//     return level_surf_rdf_numbers_of_ranges;
// }

VMP SuRF_RDF::getLevelFileRDF(){
    return level_file_surf_rdf;
}
std::unordered_map<uint64_t,std::pair<int, SuRF*>> SuRF_RDF::getLevelFileRDFAtIthLevel(int level){
    assert(level < level_file_surf_rdf.size());
    return level_file_surf_rdf[level];
}



std::vector<int> SuRF_RDF::getNumbersOfRangesInRDFLog(){
    return numbers_of_ranges_in_RDF_log;
}
std::vector<int> SuRF_RDF::getMemoryUsageInRDFLog(){
    return memory_usage_in_RDF_log;
}

SuRF_RDF::RDF_MODE SuRF_RDF::getRDFMode(){
    return rdf_mode;
}


//SuRF_RDF
using vpss = std::vector<std::pair<std::string, std::string>>;
vpss SuRF_RDF::mergeRanges(vpss ranges_1, vpss ranges_2, bool allow_boundary_overlap_not_merged){
    if(ranges_1.size() == 0){
        return ranges_2;
    }
    if(ranges_2.size() == 0){
        return ranges_1;
    }
        
    vpss ranges_out;
    int len_1 = ranges_1.size();
    int len_2 = ranges_2.size();
    int i = 0, j = 0;
    auto a = ranges_1[i];
    auto b = ranges_2[j];
    if(a <= b){
        ranges_out.push_back(a);
        i++;
    }else{
        ranges_out.push_back(b);
        j++;
    }

    while(i < len_1 || j < len_2){
        if(j == len_2){
            if(ranges_out.back().second < ranges_1[i].first ||
                (ranges_out.back().second == ranges_1[i].first && allow_boundary_overlap_not_merged)){
                ranges_out.push_back(ranges_1[i]);
            }else{
                ranges_out.back().second = max(ranges_out.back().second, ranges_1[i].second);
            }
            i++;
            continue;
        }
        if(i == len_1){
            if(ranges_out.back().second < ranges_2[j].first ||
                (ranges_out.back().second == ranges_2[j].first && allow_boundary_overlap_not_merged)){
                ranges_out.push_back(ranges_2[j]);
            }else{
                ranges_out.back().second = max(ranges_out.back().second, ranges_2[j].second);
            }
            j++;
            continue;
        }
        a = ranges_1[i];
        b = ranges_2[j];
        if(a <= b){
            if(ranges_out.back().second < a.first ||
                (ranges_out.back().second == a.first && allow_boundary_overlap_not_merged)){
                ranges_out.push_back(a);
            }else{
                ranges_out.back().second = max(ranges_out.back().second, a.second);
            }
            i++;
        }else{
            if(ranges_out.back().second < b.first ||
                (ranges_out.back().second == b.first && allow_boundary_overlap_not_merged)){
                ranges_out.push_back(b);
            }else{
                ranges_out.back().second = max(ranges_out.back().second, b.second);
            }
            j++;
        }
    }
    
    return ranges_out;
}

#define DEBUG_SURF_GET_PATH
bool SuRF_RDF::isEntryAliveAtLevelOfFd(level_t level, uint64_t fd, std::string key, bool flag_bypass_if_same_key) const {
    assert(rdf_mode == PER_FILE);
    assert(level < level_file_surf_rdf.size());
    if(level >= level_file_surf_rdf.size()){
        return true;
    }
    assert(level_file_surf_rdf[level].count(fd) > 0);
    if(level_file_surf_rdf[level].count(fd) == 0){
        return true;
    }
    SuRF* surf = (((level_file_surf_rdf[level].find(fd))->second).second);
    
    auto key_len_in_bytes = key.size();
    assert(key_len_in_bytes > 0);
    if(key_len_in_bytes == 0){
        std::cout << "Error: key_len_in_bytes should not be 0 " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
    }

    SuRF::Iter iter = surf->moveToNextCommonPrefixKey(key);

    bool non_overlapping = true;
    if(iter.isValid()){
        non_overlapping = false;
        std::string key_found = iter.getKey();
#ifdef DEBUG_SURF_GET_PATH  
std::cout << "key_found = " << key_found << " key_searched = " << key << " " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
#endif
        if(key_found < key){
            assert(false);
            std::cout << "Error: key_found < key " << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
        }

        // [a, b), [c, d)
        // 1. flag_bypass_if_same_key == 0
        //         [       ) 
        //                 [      )
        // Alive:  X  xxx  X xxxx O
        // left :  1       1      0
        // right:  0       1      1
        // 2. flag_bypass_if_same_key == 1
        //         (       ) 
        //                 (      )
        // Alive:  O  xxx  O xxxx O
        // left :  1       1      0
        // right:  0       1      1
        else if(key_found == key){
            non_overlapping = flag_bypass_if_same_key? true: (iter.getLeftParenthesis() != true);
        }

        // if(key_found.size() < key_len_in_bytes){ //suppose max(len(inserted_keys)) == max(len(searching_keys))
        //     non_overlapping = (iter.getRightParenthesis() != true);
        // }else if(key_found.size() == key_len_in_bytes){
        //     return flag_bypass_if_same_key? true: (iter.getRightParenthesis() != true);
        // }
        else{
            non_overlapping = (iter.getRightParenthesis() != true);
        }
    }
    return non_overlapping;
}

// bool SuRF_RDF::isEntryAlive(level_t level, std::string key, bool flag_bypass_if_same_key) const {
//     assert(level < level_surf_rdf.size());
//     assert(rdf_mode == PER_LEVEL);
//     SuRF* surf = level_surf_rdf[level].second;
//     SuRF::Iter iter = surf->moveToNextCommonPrefixKey(key);
//     auto key_len_in_bytes = key.size();
//     assert(key_len_in_bytes > 0);

//     bool non_overlapping = true;
//     if(iter.isValid()){
//         non_overlapping = false;
//         std::string key_found = iter.getKey();
//         if(key_found.size() < key_len_in_bytes){
//             non_overlapping = (iter.getRightParenthesis() != true);
//         }else if(key_found.size() == key_len_in_bytes){
//             return flag_bypass_if_same_key? true: (iter.getRightParenthesis() != true);
//         }else{
//             non_overlapping = (key_found != key) && (iter.getRightParenthesis() != true);
//         }
//     }
//     return non_overlapping;
// }

bool  SuRF_RDF::isEntryAlive(level_t level, std::string key, uint64_t fd, bool flag_bypass_if_same_key) const {
    assert(level < level_file_surf_rdf.size());
    assert(rdf_mode == PER_FILE);

    auto it_level_file_surf = level_file_surf_rdf[level].find(fd);
    if(it_level_file_surf == level_file_surf_rdf[level].end()){
        assert(false); 
    }

    bool overlapping = false;
    if(it_level_file_surf != level_file_surf_rdf[level].end()){
        SuRF* surf = it_level_file_surf->second.second;
        SuRF::Iter iter = surf->moveToNextCommonPrefixKey(key);
        auto key_len_in_bytes = key.size();
        assert(key_len_in_bytes > 0);

        if(iter.isValid()){
            std::string key_found = iter.getKey();
            if(key_found.size() < key_len_in_bytes){
                overlapping = (iter.getRightParenthesis() != true);
            }else if(key_found.size() == key_len_in_bytes){
                overlapping = flag_bypass_if_same_key? true: (iter.getRightParenthesis() != true);
            }else{
                overlapping = (key_found != key) && (iter.getRightParenthesis() != true);
            }
        }
    }
    return overlapping;
}


void SuRF_RDF::print(){
    if(rdf_mode == PER_LEVEL){
        for(size_t i = 0; i < level_surf_rdf.size(); i++){
            std::cout << "Level: " << i << std::endl;
            if(level_surf_rdf[i].second != NULL){
                int len_rd = level_surf_rdf[i].first;
                SuRF* surf = level_surf_rdf[i].second;
                std::vector<pss> range_list = SuRF::surfToRanges(surf);
                std::cout << "\t Number of ranges: " << len_rd << std::endl;
                std::cout << "\t";
                for(auto &range: range_list){
                    std::cout << "(" << range.first << ", " << range.second << ") ";
                }
                std::cout << std::endl;
            }
        }
    }else if(rdf_mode == PER_FILE){
        for(size_t i = 0; i < level_file_surf_rdf.size(); i++){
            std::cout << "Level: " << i << std::endl;
            auto it = level_file_surf_rdf[i].begin();
            while(it != level_file_surf_rdf[i].end()){
                std::cout << "\tFile: " << it->first;
                std::cout << " Number of ranges: " << (it->second).first << std::endl;
                SuRF* surf = (it->second).second;
                std::vector<pss> range_list = SuRF::surfToRanges(surf);
                std::cout << "\t";
                for(auto &range: range_list){
                    std::cout << "(" << range.first << ", " << range.second << ") ";
                }
                std::cout << std::endl;
                it++;
            }
        }
    }else{
        assert(false);
    }
}

// YCHUANG ADDED END


} // namespace surf


#endif // SURF_H
