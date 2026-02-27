#ifndef SURF_CC_
#define SURF_CC_

#include "surf.hpp"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include "louds_sparse.hpp"

// #define DEBUG_SURF_COMPACTION

namespace surf {

// YCHUANG ADDED START
class SuRF;
class SuRF_RDF;

using VP = std::vector<std::pair<int, SuRF*>>;
using VMP = std::vector<std::unordered_map<uint64_t, std::pair<int, SuRF*>>>;
using pss = std::pair<std::string, std::string>;
using vpss = std::vector<std::pair<std::string, std::string>>;

// YCHUANG ADDED START
SuRF_Env* SuRF_Env::surf_env_ptr;
SuRF_Env* SuRF_Env::getInstance() {
  if (surf_env_ptr == NULL) {
    surf_env_ptr = new SuRF_Env();
  }
  return surf_env_ptr;
}

void SuRF::create(const std::vector<std::string>& keys,
                  std::vector<bool>& left_parentheses,
                  std::vector<bool>& right_parentheses,
                  const bool include_dense, const uint32_t sparse_dense_ratio,
                  const SuffixType suffix_type, const level_t hash_suffix_len,
                  const level_t real_suffix_len, const uint16_t max_num_level) {
  if (include_dense == false) {
    if (suffix_type != surf::SuffixType::kNone) {
      assert(suffix_type == surf::SuffixType::kNone);
      std::cout << "Error: include_dense == false, but suffix_type != "
                   "surf::SuffixType::kNone"
                << std::endl;
      std::cerr << "Error: include_dense == false, but suffix_type != "
                   "surf::SuffixType::kNone"
                << std::endl;
    }
  }

  builder_ = new SuRFBuilder(include_dense, sparse_dense_ratio, suffix_type,
                             hash_suffix_len, real_suffix_len);
  builder_->build(keys, left_parentheses, right_parentheses, max_num_level);
  louds_dense_ = new LoudsDense(builder_);
  // std::cout << "louds_dense_->getHeight() = " << louds_dense_->getHeight() <<
  // " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
  louds_sparse_ = new LoudsSparse(builder_);
  iter_ = SuRF::Iter(this);
  delete builder_;
}
// YCHUANG ADDED END

void SuRF::create(const std::vector<std::string>& keys,
                  const bool include_dense, const uint32_t sparse_dense_ratio,
                  const SuffixType suffix_type, const level_t hash_suffix_len,
                  const level_t real_suffix_len) {
  if (include_dense == false) {
    if (suffix_type != surf::SuffixType::kNone) {
      assert(suffix_type == surf::SuffixType::kNone);
      std::cout << "Error: include_dense == false, but suffix_type != "
                   "surf::SuffixType::kNone"
                << std::endl;
      std::cerr << "Error: include_dense == false, but suffix_type != "
                   "surf::SuffixType::kNone"
                << std::endl;
    }
  }

  builder_ = new SuRFBuilder(include_dense, sparse_dense_ratio, suffix_type,
                             hash_suffix_len, real_suffix_len);
  builder_->build(keys);
  louds_dense_ = new LoudsDense(builder_);
  // std::cout << "louds_dense_->getHeight() = " << louds_dense_->getHeight() <<
  // " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
  louds_sparse_ = new LoudsSparse(builder_);
  iter_ = SuRF::Iter(this);
  delete builder_;
}

bool SuRF::lookupKey(const std::string& key) const {
  position_t connect_node_num = 0;
  bool dense_rst = louds_dense_->lookupKey(key, connect_node_num);
  std::cout << "****** " << "Dense result: " << dense_rst << " "
            << " Connect node num: " << connect_node_num << " " << __FILE__
            << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
  if (!dense_rst) {
    return false;
  } else if (connect_node_num != 0) {
    bool sparse_rst = louds_sparse_->lookupKey(key, connect_node_num);
    std::cout << "****** " << "Sparse result: " << sparse_rst << " "
              << " Connect node num: " << connect_node_num << " " << __FILE__
              << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
    return sparse_rst;
  }
  return true;
}

// YCHUANG ADDED START
SuRF::Iter SuRF::moveToNextCommonPrefixKey(
    const std::string& key,
    bool flag_direct_return_if_found_key_end_with_same_prefix) const {
  SuRF::Iter iter(this);

  // std::cout << "louds_dense_->getHeight() = " <<louds_dense_->getHeight() <<
  // " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

  if (louds_dense_->getHeight() == 0) {
    iter.passToSparse();
    iter.could_be_fp_ = louds_sparse_->moveToNextCommonPrefixKey(
        key, iter.sparse_iter_,
        flag_direct_return_if_found_key_end_with_same_prefix);
    return iter;
  }

  iter.could_be_fp_ = louds_dense_->moveToNextCommonPrefixKey(
      key, iter.dense_iter_,
      flag_direct_return_if_found_key_end_with_same_prefix);

  if (!iter.dense_iter_.isValid()) return iter;
  if (iter.dense_iter_.isComplete()) return iter;

  if (!iter.dense_iter_.isSearchComplete()) {
    iter.passToSparse();
    iter.could_be_fp_ = louds_sparse_->moveToNextCommonPrefixKey(
        key, iter.sparse_iter_,
        flag_direct_return_if_found_key_end_with_same_prefix);
    if (!iter.sparse_iter_.isValid()) {
      iter.incrementDenseIter();
    }
    return iter;
  } else if (!iter.dense_iter_.isMoveLeftComplete()) {
    iter.passToSparse();
    iter.sparse_iter_.moveToLeftMostKey();
    return iter;
  }

  assert(false);  // shouldn't reach here
  return iter;
}
// YCHUANG ADDED END

SuRF::Iter SuRF::moveToKeyGreaterThan(const std::string& key,
                                      const bool inclusive) const {
  SuRF::Iter iter(this);
  // iter.could_be_fp_ = louds_dense_->moveToKeyGreaterThan(key, inclusive,
  // iter.dense_iter_);
  iter.could_be_fp_ = louds_dense_->moveToKeyGreaterThan(key, iter.dense_iter_);

  if (!iter.dense_iter_.isValid()) return iter;
  if (iter.dense_iter_.isComplete()) return iter;

  if (!iter.dense_iter_.isSearchComplete()) {
    iter.passToSparse();
    iter.could_be_fp_ =
        louds_sparse_->moveToKeyGreaterThan(key, inclusive, iter.sparse_iter_);
    if (!iter.sparse_iter_.isValid()) iter.incrementDenseIter();
    return iter;
  } else if (!iter.dense_iter_.isMoveLeftComplete()) {
    iter.passToSparse();
    iter.sparse_iter_.moveToLeftMostKey();
    return iter;
  }

  assert(false);  // shouldn't reach here
  return iter;
}

SuRF::Iter SuRF::moveToFirst() const {
  SuRF::Iter iter(this);
  if (louds_dense_->getHeight() > 0) {
    iter.dense_iter_.setToFirstLabelInRoot();
    iter.dense_iter_.moveToLeftMostKey();
    if (iter.dense_iter_.isMoveLeftComplete()) return iter;
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
    if (iter.dense_iter_.isMoveRightComplete()) return iter;
    iter.passToSparse();
    iter.sparse_iter_.moveToRightMostKey();
  } else {
    iter.sparse_iter_.setToLastLabelInRoot();
    iter.sparse_iter_.moveToRightMostKey();
  }
  return iter;
}

bool SuRF::lookupRange(const std::string& left_key, const bool left_inclusive,
                       const std::string& right_key,
                       const bool right_inclusive) {
  iter_.clear();
  louds_dense_->moveToKeyGreaterThan(left_key, iter_.dense_iter_);
  if (!iter_.dense_iter_.isValid()) return false;
  if (!iter_.dense_iter_.isComplete()) {
    if (!iter_.dense_iter_.isSearchComplete()) {
      iter_.passToSparse();
      louds_sparse_->moveToKeyGreaterThan(left_key, left_inclusive,
                                          iter_.sparse_iter_);
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
  if (compare == kCouldBePositive) return true;
  if (right_inclusive)
    return (compare <= 0);
  else
    return (compare < 0);
}

uint64_t SuRF::approxCount(const SuRF::Iter* iter, const SuRF::Iter* iter2) {
  if (!iter->isValid() || !iter2->isValid()) return 0;
  position_t out_node_num_left = 0, out_node_num_right = 0;
  uint64_t count =
      louds_dense_->approxCount(&(iter->dense_iter_), &(iter2->dense_iter_),
                                out_node_num_left, out_node_num_right);
  count +=
      louds_sparse_->approxCount(&(iter->sparse_iter_), &(iter2->sparse_iter_),
                                 out_node_num_left, out_node_num_right);
  return count;
}

uint64_t SuRF::approxCount(const std::string& left_key,
                           const std::string& right_key) {
  iter_.clear();
  iter2_.clear();
  iter_ = moveToKeyGreaterThan(left_key, true);
  if (!iter_.isValid()) return 0;
  iter2_ = moveToKeyGreaterThan(right_key, true);
  if (!iter2_.isValid()) iter2_ = moveToLast();

  return approxCount(&iter_, &iter2_);
}

uint64_t SuRF::serializedSize() const {
  return (louds_dense_->serializedSize() + louds_sparse_->serializedSize());
}

uint64_t SuRF::getMemoryUsage() const {
  return (sizeof(SuRF) + louds_dense_->getMemoryUsage() +
          louds_sparse_->getMemoryUsage());
}

uint64_t SuRF::getDensePartMemoryUsageInBitsSelf() const {
  return (louds_dense_->getMemoryUsageInBitsSelf());
}

uint64_t SuRF::getSparsePartMemoryUsageInBitsSelf() const {
  return (louds_sparse_->getMemoryUsageInBitsSelf());
}

uint64_t SuRF::getMemoryUsageInBitsSelf() const {
  surf::SuRF_Env* _surf_env = surf::SuRF_Env::getInstance();
  if (_surf_env->getShowSurfCompactionInfo()) {
    std::cout << "SuRF::getMemoryUsageInBitsSelf " << __FILE__ << ":"
              << __LINE__ << " " << __FUNCTION__ << std::endl;
    std::cout << "sparse start level " << louds_sparse_->getStartLevel() << " "
              << "louds_sparse_=>getHeight " << louds_sparse_->getHeight()
              << std::endl;
    std::cout << "sizeof(SuRF) " << sizeof(SuRF) << "  "
              << "louds_dense_->getMemoryUsageInBitsSelf() "
              << louds_dense_->getMemoryUsageInBitsSelf() << "  "
              << "louds_sparse_->getMemoryUsageInBitsSelf() "
              << louds_sparse_->getMemoryUsageInBitsSelf() << std::endl;
  }
  // if(_surf_env->getSuRFIncludeDense() == false){
  //     return (louds_sparse_->getMemoryUsageInBitsSelf());
  // }
  if (louds_dense_ == nullptr && louds_sparse_ == nullptr) {
    return 0;
  }
  if (louds_dense_ == nullptr) {
    return (louds_sparse_->getMemoryUsageInBitsSelf());
  }
  if (louds_sparse_ == nullptr) {
    return (louds_dense_->getMemoryUsageInBitsSelf());
  }
  return (louds_dense_->getMemoryUsageInBitsSelf() +
          louds_sparse_->getMemoryUsageInBitsSelf());
}

level_t SuRF::getHeight() const { return louds_sparse_->getHeight(); }

level_t SuRF::getSparseStartLevel() const {
  return louds_sparse_->getStartLevel();
}

void SuRF::Iter::clear() {
  dense_iter_.clear();
  sparse_iter_.clear();
}

bool SuRF::Iter::getFpFlag() const { return could_be_fp_; }

bool SuRF::Iter::isValid() const {
  return dense_iter_.isValid() &&
         (dense_iter_.isComplete() || sparse_iter_.isValid());
}

int SuRF::Iter::compare(const std::string& key) const {
  assert(isValid());
  int dense_compare = dense_iter_.compare(key);
  if (dense_iter_.isComplete() || dense_compare != 0) return dense_compare;
  return sparse_iter_.compare(key);
}

std::string SuRF::Iter::getKey() const {
  if (!isValid()) return std::string();
  if (dense_iter_.isComplete()) return dense_iter_.getKey();
  return dense_iter_.getKey() + sparse_iter_.getKey();
}

int SuRF::Iter::getSuffix(word_t* suffix) const {
  if (!isValid()) return 0;
  if (dense_iter_.isComplete()) return dense_iter_.getSuffix(suffix);
  return sparse_iter_.getSuffix(suffix);
}

// YCHUANG ADDED START
bool SuRF::Iter::getLeftParenthesis() const {
  if (!isValid()) return false;
  if (dense_iter_.isComplete()) return dense_iter_.getLeftParenthesis();
  return sparse_iter_.getLeftParenthesis();
}
bool SuRF::Iter::getRightParenthesis() const {
  if (!isValid()) return false;
  if (dense_iter_.isComplete()) return dense_iter_.getRightParenthesis();
  return sparse_iter_.getRightParenthesis();
}
// YCHUANG ADDED END

std::string SuRF::Iter::getKeyWithSuffix(unsigned* bitlen) const {
  *bitlen = 0;
  if (!isValid()) return std::string();
  if (dense_iter_.isComplete()) return dense_iter_.getKeyWithSuffix(bitlen);
  return dense_iter_.getKeyWithSuffix(bitlen) +
         sparse_iter_.getKeyWithSuffix(bitlen);
}

void SuRF::Iter::passToSparse() {
  sparse_iter_.setStartNodeNum(dense_iter_.getSendOutNodeNum());
}

bool SuRF::Iter::incrementDenseIter() {
  if (!dense_iter_.isValid()) return false;

  dense_iter_++;
  if (!dense_iter_.isValid()) return false;
  if (dense_iter_.isMoveLeftComplete()) return true;

  passToSparse();
  sparse_iter_.moveToLeftMostKey();
  return true;
}

bool SuRF::Iter::incrementSparseIter() {
  if (!sparse_iter_.isValid()) return false;
  sparse_iter_++;
  return sparse_iter_.isValid();
}

bool SuRF::Iter::operator++(int) {
  if (!isValid()) return false;
  if (incrementSparseIter()) return true;
  return incrementDenseIter();
}

bool SuRF::Iter::decrementDenseIter() {
  if (!dense_iter_.isValid()) return false;

  dense_iter_--;
  if (!dense_iter_.isValid()) return false;
  if (dense_iter_.isMoveRightComplete()) return true;

  passToSparse();
  sparse_iter_.moveToRightMostKey();
  return true;
}

bool SuRF::Iter::decrementSparseIter() {
  if (!sparse_iter_.isValid()) return false;
  sparse_iter_--;
  return sparse_iter_.isValid();
}

bool SuRF::Iter::operator--(int) {
  if (!isValid()) return false;
  if (decrementSparseIter()) return true;
  return decrementDenseIter();
}

// YCHUANG ADDED START
SuRF* SuRF::rangesToSurf(
    std::vector<std::pair<std::string, std::string>> ranges,
    size_t surf_key_length_in_bytes, surf::SuffixType kSuffixType,
    surf::level_t hash_suffix_len, surf::level_t real_suffix_len,
    bool include_dense, uint32_t sparse_dense_ratio,
    bool flag_allow_boundary_overlapped) {
  if (include_dense == false) {
    if (kSuffixType != surf::SuffixType::kNone) {
      assert(kSuffixType == surf::SuffixType::kNone);
      std::cout << "Error: include_dense == false, but kSuffixType != "
                   "surf::SuffixType::kNone"
                << std::endl;
      std::cerr << "Error: include_dense == false, but kSuffixType != "
                   "surf::SuffixType::kNone"
                << std::endl;
    }
  }

  std::sort(ranges.begin(), ranges.end());
  size_t len = ranges.size();
  assert(len > 0);
  bool flag_merge = false;
  if (len > 1) {
    for (size_t i = 0; i < len - 1; i++) {
      flag_merge = (flag_allow_boundary_overlapped == true)
                       ? (ranges[i].second > ranges[i + 1].first)
                       : (ranges[i].second >= ranges[i + 1].first);
      if (flag_merge) {
        break;
      }
    }
  }
  if (flag_merge) {
    // doing merging
    std::vector<pss> ranges_merged;
    ranges_merged.push_back(ranges[0]);
    for (size_t i = 1; i < len; i++) {
      bool flag_shall_be_merged =
          (flag_allow_boundary_overlapped == true)
              ? (ranges_merged.back().second > ranges[i].first)
              : (ranges_merged.back().second >= ranges[i].first);

      if (flag_shall_be_merged) {
        ranges_merged.back().second =
            std::max(ranges_merged.back().second, ranges[i].second);
      } else {
        ranges_merged.push_back(ranges[i]);
      }
    }
    ranges = ranges_merged;
  }

  len = ranges.size();
  assert(len > 0);
  if (len > 1) {
    for (size_t i = 0; i < len - 1; i++) {
      assert(
          ("cannot have overlapped, ranges[i].second shall <= "
           "ranges[i+1].first",
           ranges[i].second <= ranges[i + 1].first));
    }
  }
  // std::cout << "surf_key_length_in_bytes = " << surf_key_length_in_bytes <<
  // " " << __FILE__ << ":" << __LINE__ << " " << std::endl;

  std::vector<std::string> keys;
  std::vector<bool> left_parentheses;
  std::vector<bool> right_parentheses;
  for (size_t i = 0; i < len; i++) {
    std::string key_start = ranges[i].first;
    std::string key_end = ranges[i].second;
    if (key_start.size() > surf_key_length_in_bytes) {
      // std::cout << "key_start = " << key_start;
      key_start = key_start.substr(0, surf_key_length_in_bytes);
      // std::cout << " key_start = " << key_start << " " << __FILE__ << ":"
      // <<
      // __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    if (key_end.size() > surf_key_length_in_bytes) {
      // std::cout << "key_end = " << key_end;
      key_end = key_end.substr(0, surf_key_length_in_bytes);
      // std::cout << " key_end = " << key_end << " " << __FILE__ << ":" <<
      // __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    // 2024-10
    if (key_start == key_end) {
      if (keys.size() > 0 && key_start == keys.back()) {
        left_parentheses.back() = true;
        // right_parentheses.back() = true;
      } else {
        keys.push_back(key_start);
        left_parentheses.push_back(true);
        right_parentheses.push_back(false);
      }
      continue;
    }
    //
    if (keys.size() > 0 && keys.back() == key_start) {
      left_parentheses.back() = true;
    } else {
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

  // cheking no repetitive keys and in ascending order
  {
    for (uint32_t i = 1; i < keys.size(); i++) {
      if (keys[i - 1] >= keys[i]) {
        std::cout << "keys[i-1] should be smaller than keys[i] " << __FILE__
                  << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      }
    }
  }
  if (surf::SuRF_Env::getInstance()->getFlagUseSuRFBase() == true) {
    int surf_base_store_key_to_k_diff =
        surf::SuRF_Env::getInstance()->getSuRFBaseStoreKeyToKDiff();
    keys = surf::SuRF_Utils::processStringsToFirstKDifference(
        keys, surf_base_store_key_to_k_diff);
  }

  // string_length
  size_t string_len = 0;
  for (auto& key : keys) {
    if (key.length() > string_len) {
      string_len = key.length();
    }
  }

  uint16_t max_num_level = string_len;
  SuRF* surf_ =
      new SuRF(keys, left_parentheses, right_parentheses, include_dense,
               sparse_dense_ratio,
               // surf::kHash, 8, 0,
               kSuffixType, hash_suffix_len, real_suffix_len, max_num_level);

  return surf_;
}

std::pair<SuRF*, size_t> SuRF::rangesWithPointKeysToSurf(
    std::vector<pss> ranges, std::vector<std::string> point_keys,
    size_t surf_key_length_in_bytes, surf::SuffixType kSuffixType,
    surf::level_t hash_suffix_len, surf::level_t real_suffix_len,
    bool include_dense, uint32_t sparse_dense_ratio,
    bool flag_allow_boundary_overlapped) {
  if (include_dense == false) {
    if (kSuffixType != surf::SuffixType::kNone) {
      assert(kSuffixType == surf::SuffixType::kNone);
      std::cout << "Error: include_dense == false, but kSuffixType != "
                   "surf::SuffixType::kNone"
                << std::endl;
      std::cerr << "Error: include_dense == false, but kSuffixType != "
                   "surf::SuffixType::kNone"
                << std::endl;
    }
  }

  std::sort(ranges.begin(), ranges.end());
  size_t len = ranges.size();
  assert(len > 0);
  bool flag_merge = false;
  if (len > 1) {
    for (size_t i = 0; i < len - 1; i++) {
      flag_merge = (flag_allow_boundary_overlapped == true)
                       ? (ranges[i].second > ranges[i + 1].first)
                       : (ranges[i].second >= ranges[i + 1].first);
      if (flag_merge) {
        break;
      }
    }
  }
  if (flag_merge) {
    // doing merging
    std::vector<pss> ranges_merged;
    ranges_merged.push_back(ranges[0]);
    for (size_t i = 1; i < len; i++) {
      bool flag_shall_be_merged =
          (flag_allow_boundary_overlapped == true)
              ? (ranges_merged.back().second > ranges[i].first)
              : (ranges_merged.back().second >= ranges[i].first);

      if (flag_shall_be_merged) {
        ranges_merged.back().second =
            std::max(ranges_merged.back().second, ranges[i].second);
      } else {
        ranges_merged.push_back(ranges[i]);
      }
    }
    ranges = ranges_merged;
  }

  len = ranges.size();
  assert(len > 0);
  if (len > 1) {
    for (size_t i = 0; i < len - 1; i++) {
      assert(
          ("cannot have overlapped, ranges[i].second shall <= "
           "ranges[i+1].first",
           ranges[i].second <= ranges[i + 1].first));
    }
  }

  size_t len_point_keys = point_keys.size();
  size_t j_point_keys = 0;
  size_t split_count = 0;

  std::vector<std::string> keys;
  std::vector<bool> left_parentheses;
  std::vector<bool> right_parentheses;
  for (size_t i = 0; i < len; i++) {
    std::string key_start = ranges[i].first;
    std::string key_end = ranges[i].second;

#ifdef DEBUG_SURF_COMPACTION
    std::cout << "bypassing keys " << __FILE__ << ":" << __LINE__ << " "
              << __FUNCTION__ << std::endl;
#endif
    while (j_point_keys < len_point_keys &&
           point_keys[j_point_keys] < key_start) {
#ifdef DEBUG_SURF_COMPACTION
      if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
          true) {
        auto pk = surf::SuRF_Utils::decode_byte_string_to_digit_string(
            point_keys[j_point_keys]);
        std::cout << pk << " ";
      } else {
        std::cout << point_keys[j_point_keys] << " ";
      }
#endif
      j_point_keys++;
    }
#ifdef DEBUG_SURF_COMPACTION
    std::cout << std::endl;
#endif

    // std::cout << "surf_key_length_in_bytes = " << surf_key_length_in_bytes
    // << " " << __FILE__ << ":" << __LINE__ << " " << std::endl;

    if (key_start.size() > surf_key_length_in_bytes) {
      // std::cout << "key_start = " << key_start;
      key_start = key_start.substr(0, surf_key_length_in_bytes);
      // std::cout << " key_start = " << key_start << " " << __FILE__ << ":"
      // <<
      // __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    if (key_end.size() > surf_key_length_in_bytes) {
      // std::cout << "key_end = " << key_end;
      key_end = key_end.substr(0, surf_key_length_in_bytes);
      // std::cout << " key_end = " << key_end << " " << __FILE__ << ":" <<
      // __LINE__ << " " << __FUNCTION__ << std::endl;
    }
    // 2024-10
    if (key_start == key_end) {
#ifdef DEBUG_SURF_COMPACTION
      if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
          true) {
        auto ks =
            surf::SuRF_Utils::decode_byte_string_to_digit_string(key_start);
        std::cout << "key_start == key_end = " << ks << " " << __FILE__ << ":"
                  << __LINE__ << " " << __FUNCTION__ << std::endl;
      } else {
        std::cout << "key_start == key_end = " << key_start << " " << __FILE__
                  << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      }
#endif

      if (keys.size() > 0 && key_start == keys.back()) {
        left_parentheses.back() = true;
        // right_parentheses.back() = true;
      } else {
        keys.push_back(key_start);
        left_parentheses.push_back(true);
        right_parentheses.push_back(false);
      }
      continue;
    }
    //

    if (keys.size() > 0 && keys.back() == key_start) {
      left_parentheses.back() = true;
    } else {
      keys.push_back(key_start);
      left_parentheses.push_back(true);
      right_parentheses.push_back(false);
    }

    // splitting ranges by incoming point keys
    while (j_point_keys < len_point_keys) {
      std::string point_key = point_keys[j_point_keys];
      if (point_key.size() > surf_key_length_in_bytes) {
        point_key = point_key.substr(0, surf_key_length_in_bytes);
      }
      if (point_key >= key_end) {
        break;
      }

      if (j_point_keys > 0) {
        std::string prev_point_key = point_keys[j_point_keys - 1];
        if (prev_point_key.size() > surf_key_length_in_bytes) {
          prev_point_key = prev_point_key.substr(0, surf_key_length_in_bytes);
        }
        if (point_key == prev_point_key) {
          j_point_keys++;
          continue;
        }
      }

      if (point_key == key_start) {
#ifdef DEBUG_SURF_COMPACTION
        if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
            true) {
          auto pk = surf::SuRF_Utils::decode_byte_string_to_digit_string(
              point_keys[j_point_keys]);
          auto ks =
              surf::SuRF_Utils::decode_byte_string_to_digit_string(key_start);
          std::cout << "(==) point_key = " << pk << " key_start = " << ks << " "
                    << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                    << std::endl;
        } else {
          std::cout << "(==) point_key = " << point_key
                    << " key_start = " << key_start << " " << __FILE__ << ":"
                    << __LINE__ << " " << __FUNCTION__ << std::endl;
        }
#endif
        if (flag_allow_boundary_overlapped == false) {
          // right_parentheses.back() = true;
          // 2024-10
          left_parentheses.back() = false;
          right_parentheses.back() = false;
          //
        } else {
          if (left_parentheses.back() == true &&
              right_parentheses.back() == true) {
            left_parentheses.back() = true;
            right_parentheses.back() = true;
          } else {
            left_parentheses.back() = false;
            right_parentheses.back() = false;
          }
        }
      } else {
        keys.push_back(point_key);
        left_parentheses.push_back(false);
        right_parentheses.push_back(true);

        split_count++;
      }

      j_point_keys++;
    }

    keys.push_back(key_end);
    left_parentheses.push_back(false);
    right_parentheses.push_back(true);
  }
  assert(keys.size() == left_parentheses.size());
  assert(keys.size() == right_parentheses.size());

  // cheking no repetitive keys and in ascending order
  {
    for (uint32_t i = 1; i < keys.size(); i++) {
      if (keys[i - 1] >= keys[i]) {
        std::cout << "keys[i-1] should be smaller than keys[i] " << __FILE__
                  << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      }
    }
  }
  if (surf::SuRF_Env::getInstance()->getFlagUseSuRFBase() == true) {
#ifdef DEBUG_SURF_COMPACTION
    std::cout << "before processStringsToFirstKDifference " << __FILE__ << ":"
              << __LINE__ << " " << __FUNCTION__ << std::endl;
    for (uint32_t i = 0; i < keys.size(); i++) {
      if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
          true) {
        auto ks = surf::SuRF_Utils::decode_byte_string_to_digit_string(keys[i]);
        std::cout << "key =  " << ks << " (" << left_parentheses[i] << ","
                  << right_parentheses[i] << ") " << __FILE__ << ":" << __LINE__
                  << " " << __FUNCTION__ << std::endl;
      } else {
        std::cout << "key =  " << keys[i] << " (" << left_parentheses[i] << ","
                  << right_parentheses[i] << ") " << __FILE__ << ":" << __LINE__
                  << " " << __FUNCTION__ << std::endl;
      }
    }
    std::cout << std::endl;
#endif

    int surf_base_store_key_to_k_diff =
        surf::SuRF_Env::getInstance()->getSuRFBaseStoreKeyToKDiff();
    keys = surf::SuRF_Utils::processStringsToFirstKDifference(
        keys, surf_base_store_key_to_k_diff);

#ifdef DEBUG_SURF_COMPACTION
    std::cout << "after processStringsToFirstKDifference " << __FILE__ << ":"
              << __LINE__ << " " << __FUNCTION__ << std::endl;
    for (uint32_t i = 0; i < keys.size(); i++) {
      if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
          true) {
        auto ks = surf::SuRF_Utils::decode_byte_string_to_digit_string(keys[i]);
        std::cout << "key =  " << ks << " (" << left_parentheses[i] << ","
                  << right_parentheses[i] << ") " << __FILE__ << ":" << __LINE__
                  << " " << __FUNCTION__ << std::endl;
      } else {
        std::cout << "key =  " << keys[i] << " (" << left_parentheses[i] << ","
                  << right_parentheses[i] << ") " << __FILE__ << ":" << __LINE__
                  << " " << __FUNCTION__ << std::endl;
      }
    }
    std::cout << std::endl;
#endif
  }

  // string_length
  size_t string_len = 0;
  for (auto& key : keys) {
    if (key.length() > string_len) {
      string_len = key.length();
    }
  }

  uint16_t max_num_level = string_len;
  SuRF* surf_ = new SuRF(keys, left_parentheses, right_parentheses,
                         include_dense, sparse_dense_ratio, kSuffixType,
                         hash_suffix_len, real_suffix_len, max_num_level);

  return std::make_pair(surf_, split_count);
}

// cannot be used under split setting because the start key may need to moved
// to the next larger one if it is removed by the point_key
std::vector<std::pair<std::string, std::string>> SuRF::surfToRanges(
    SuRF* surf_, bool flag_allow_boundary_overlapped) {
  // SuRFRangesAndSplitPoint SuRF::surfToRanges(SuRF* surf_, bool
  // flag_allow_boundary_overlapped){
  std::vector<std::string> keys;
  std::vector<bool> left_parentheses;
  std::vector<bool> right_parentheses;

  SuRF::Iter iter = surf_->moveToFirst();

  if (surf_->louds_dense_->getHeight() == 0) {
    std::cout << "0 " << "surf rdf out" << std::endl;
    while (iter.sparse_iter_.isValid()) {
      std::string key = iter.sparse_iter_.getKey();
      bool left_parenthesis = iter.sparse_iter_.getLeftParenthesis();
      bool right_parenthesis = iter.sparse_iter_.getRightParenthesis();
      keys.push_back(key);
      left_parentheses.push_back(left_parenthesis);
      right_parentheses.push_back(right_parenthesis);
      iter.sparse_iter_++;
      // std::cout << key << "("<<left_parenthesis << "," << right_parenthesis
      // << ") ";
    }
    std::cout << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
              << std::endl;
  } else {
    std::cout << "surfToRanges height>0 " << "surf rdf out" << std::endl;
    while (iter.isValid()) {
      std::string key = iter.getKey();
      bool left_parenthesis = iter.getLeftParenthesis();
      bool right_parenthesis = iter.getRightParenthesis();
      keys.push_back(key);
      left_parentheses.push_back(left_parenthesis);
      right_parentheses.push_back(right_parenthesis);
      iter++;
      std::cout << key << "(" << left_parenthesis << "," << right_parenthesis
                << ") ";
    }
    std::cout << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
              << std::endl;
  }

  std::vector<std::pair<std::string, std::string>> ranges;
  int len = keys.size();

  if (len > 0) {
    std::string end;
    std::string start;
    bool flag_first_key = true;
    if (flag_allow_boundary_overlapped == false) {
      for (int i = 0; i < len; i++) {
        // if(left_parentheses[i]){
        //     //assert(left_parentheses[i] == true && right_parentheses[i] ==
        //     true); if(flag_first_key != true){
        //         ranges.push_back(std::make_pair(start, end));
        //     }
        //     start = keys[i];
        //     // end = keys[i];
        // }else{
        //     end = keys[i];
        // }
        // 2024-10
        if (left_parentheses[i] == true) {
          //(1,1), (1,0)
          if (right_parentheses[i] == true) {
            //(1,1)
            end = keys[i];
          }
          if (flag_first_key != true) {
            ranges.push_back(std::make_pair(start, end));
          }
          start = keys[i];
          end = keys[i];
        } else if (left_parentheses[i] == false &&
                   right_parentheses[i] == false) {
          //(0, 0)
          if (flag_first_key != true) {
            ranges.push_back(std::make_pair(start, end));
          }
          start = keys[i];
          end = keys[i];
        } else {
          //(0,1)
          end = keys[i];
        }
        //
        flag_first_key = false;
      }
      // if(end == "" && start > end){

      // }else{
      if (start > end) {
        if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
            true) {
          auto ks = surf::SuRF_Utils::decode_byte_string_to_digit_string(start);
          auto ke = surf::SuRF_Utils::decode_byte_string_to_digit_string(end);

          std::cerr << "Error: start > end" << " start = " << ks
                    << " end = " << ke << " " << __FILE__ << ":" << __LINE__
                    << std::endl;
          std::cout << "Error: start > end" << " start = " << ks
                    << " end = " << ke << " " << __FILE__ << ":" << __LINE__
                    << std::endl;
        } else {
          std::cerr << "Error: start > end" << " start = " << start
                    << " end = " << end << " " << __FILE__ << ":" << __LINE__
                    << std::endl;
          std::cout << "Error: start > end" << " start = " << start
                    << " end = " << end << " " << __FILE__ << ":" << __LINE__
                    << std::endl;
        }
      }
      ranges.push_back(std::make_pair(start, end));
      // }
    } else {
      for (int i = 0; i < len; i++) {
        // if(left_parentheses[i]){
        //   if(right_parentheses[i]){
        //     end = keys[i];
        //   }

        //   if(flag_first_key != true){
        //     ranges.push_back(std::make_pair(start, end));
        //   }
        //   start = keys[i];
        // }else{
        //   end = keys[i];
        // }
        // 2024-10
        if (left_parentheses[i] == true) {
          //(1,1), (1,0)
          if (right_parentheses[i] == true) {
            //(1,1)
            end = keys[i];
            // std::cout << "left, right = true shouldn't be the case @ non
            // overlapping range settings " << __FILE__ << ":" << __LINE__ <<
            // " " << __FUNCTION__ << std::endl; exist when ranges @ could
            // overlap on boundary
          }
          if (flag_first_key != true) {
            ranges.push_back(std::make_pair(start, end));
          }
          start = keys[i];
          end = keys[i];
        } else if (left_parentheses[i] == false &&
                   right_parentheses[i] == false) {
          //(0, 0)
          if (flag_first_key != true) {
            ranges.push_back(std::make_pair(start, end));
          }
          start = keys[i];
          end = keys[i];
        } else {
          if (surf::SuRF_Env::getInstance()
                  ->getFlagSurfUseCondensedDigitKey() == true) {
            std::cout << "x "
                      << surf::SuRF_Utils::decode_byte_string_to_digit_string(
                             keys[i])
                      << " ";
          } else {
            std::cout << "x " << keys[i] << " ";
          }
          //(0,1)
          end = keys[i];
        }
        //
        flag_first_key = false;
      }
      if (start > end) {
        if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
            true) {
          auto ks = surf::SuRF_Utils::decode_byte_string_to_digit_string(start);
          auto ke = surf::SuRF_Utils::decode_byte_string_to_digit_string(end);

          std::cerr << "Error: start > end" << " start = " << ks
                    << " end = " << ke << " " << __FILE__ << ":" << __LINE__
                    << std::endl;
          std::cout << "Error: start > end" << " start = " << ks
                    << " end = " << ke << " " << __FILE__ << ":" << __LINE__
                    << std::endl;
        } else {
          std::cerr << "Error: start > end" << " start = " << start
                    << " end = " << end << " " << __FILE__ << ":" << __LINE__
                    << std::endl;
          std::cout << "Error: start > end" << " start = " << start
                    << " end = " << end << " " << __FILE__ << ":" << __LINE__
                    << std::endl;
        }
        assert(start > end);
      }
      ranges.push_back(std::make_pair(start, end));
    }
  }
  std::cout << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
            << std::endl;

  return ranges;
  // return SuRFRangesAndSplitPoint{.ranges = ranges, .split_points =
  // split_points};
}

// struct SuRFRangesAndSplitPoints{
//     std::vector<std::pair<std::string, std::string>> ranges;
//     std::vector<std::string> split_points;
// };

// // std::vector<std::pair<std::string, std::string>>
// SuRF::surfToRanges(SuRF* surf_, bool flag_allow_boundary_overlapped){
// SuRFRangesAndSplitPoints SuRF::surfToRangesAndSplitPoints(SuRF* surf_, bool
// flag_allow_boundary_overlapped){
//     std::vector<std::string> keys;
//     std::vector<bool> left_parentheses;
//     std::vector<bool> right_parentheses;

//     SuRF::Iter iter = surf_->moveToFirst();

//     if(surf_->louds_dense_->getHeight() == 0){
//         while(iter.sparse_iter_.isValid()){
//             std::string key = iter.sparse_iter_.getKey();
//             bool left_parenthesis = iter.sparse_iter_.getLeftParenthesis();
//             bool right_parenthesis =
//             iter.sparse_iter_.getRightParenthesis(); keys.push_back(key);
//             left_parentheses.push_back(left_parenthesis);
//             right_parentheses.push_back(right_parenthesis);
//             iter.sparse_iter_++;
//         }
//     }else{
//         while(iter.isValid()){
//             std::string key = iter.getKey();
//             bool left_parenthesis = iter.getLeftParenthesis();
//             bool right_parenthesis = iter.getRightParenthesis();
//             keys.push_back(key);
//             left_parentheses.push_back(left_parenthesis);
//             right_parentheses.push_back(right_parenthesis);
//             iter++;
//         }
//     }

//     std::vector<std::pair<std::string, std::string>> ranges;
//     std::vector<std::string> split_points;
//     int len = keys.size();

//     if(len > 0){
//         std::string end;
//         std::string start;
//         bool flag_first_key = true;
//         if(flag_allow_boundary_overlapped == false){
//             for(int i = 0; i < len; i++){
//                 if(left_parentheses[i]){
//                     //assert(left_parentheses[i] == true &&
//                     right_parentheses[i] == true); if(flag_first_key !=
//                     true){
//                         ranges.push_back(std::make_pair(start, end));
//                     }
//                     start = keys[i];
//                     //
//                     if(right_parentheses[i]){
//                         split_points.push_back(start);
//                     }
//                     //
//                     // end = keys[i];
//                 }else{
//                     end = keys[i];
//                 }
//                 flag_first_key = false;
//             }
//             if(start > end){
//                 std::cerr << "Error: start > end" << " start = " << start
//                 << " end = " << end << " " << __FILE__ << ":" << __LINE__
//                 << std::endl; std::cout << "Error: start > end" << " start
//                 = "
//                 << start << " end = " << end << " " << __FILE__ << ":" <<
//                 __LINE__ << std::endl; assert(start > end);
//             }
//             ranges.push_back(std::make_pair(start, end));
//         }else{
//             for(int i = 0; i < len; i++){
//                 if(left_parentheses[i]){
//                   if(right_parentheses[i]){
//                     end = keys[i];
//                   }

//                   if(flag_first_key != true){
//                     ranges.push_back(std::make_pair(start, end));
//                   }
//                   start = keys[i];
//                 }else{
//                   end = keys[i];
//                 }
//                 flag_first_key = false;
//             }
//             if(start > end){
//                 std::cerr << "Error: start > end" << " " << __FILE__ << ":"
//                 << __LINE__ << std::endl; std::cout << "Error: start > end"
//                 << " " << __FILE__ << ":" << __LINE__ << std::endl;
//                 assert(start > end);
//             }
//             ranges.push_back(std::make_pair(start, end));
//         }
//     }

//     // return ranges;
//     return SuRFRangesAndSplitPoints{.ranges = ranges, .split_points =
//     split_points};
// }

SuRF_RDF::SuRF_RDF(SuRF_RDF::RDF_MODE rdf_mode_in) { rdf_mode = rdf_mode_in; }

SuRF_RDF::SuRF_RDF(const SuRF_RDF& rdf_in) {
  this->rdf_mode = rdf_in.rdf_mode;
  this->numbers_of_ranges_in_RDF_log = rdf_in.numbers_of_ranges_in_RDF_log;
  this->memory_usage_in_RDF_log = rdf_in.memory_usage_in_RDF_log;

  if (this->rdf_mode == PER_LEVEL) {
    for (const auto& pair : rdf_in.level_surf_rdf) {
      if (pair.second != nullptr) {
        this->level_surf_rdf.push_back({pair.first, pair.second->clone()});
      } else {
        this->level_surf_rdf.push_back({pair.first, nullptr});
      }
    }
  } else if (this->rdf_mode == PER_FILE) {
    for (const auto& map : rdf_in.level_file_surf_rdf) {
      std::unordered_map<uint64_t, std::pair<int, SuRF*>> new_map;
      for (const auto& entry : map) {
        if (entry.second.second != nullptr) {
          new_map[entry.first] = {entry.second.first,
                                  entry.second.second->clone()};
        } else {
          new_map[entry.first] = {entry.second.first, nullptr};
        }
      }
      this->level_file_surf_rdf.push_back(new_map);
    }
  }
}

SuRF_RDF::SuRF_RDF(const VMP& level_file_surf_rdf_in,
                   const std::vector<int>& numbers_of_ranges_in_RDF_log_in,
                   const std::vector<int>& memory_usage_in_RDF_log_in,
                   const SuRF_RDF::RDF_MODE rdf_mode_in) {
  assert(rdf_mode_in == PER_FILE);
  rdf_mode = rdf_mode_in;
  level_file_surf_rdf = level_file_surf_rdf_in;
  numbers_of_ranges_in_RDF_log = numbers_of_ranges_in_RDF_log_in;
  memory_usage_in_RDF_log = memory_usage_in_RDF_log_in;
}

SuRF_RDF::SuRF_RDF(const VP& level_surf_rdf_in,
                   const std::vector<int>& numbers_of_ranges_in_RDF_log_in,
                   const std::vector<int>& memory_usage_in_RDF_log_in,
                   const SuRF_RDF::RDF_MODE rdf_mode_in) {
  assert(rdf_mode_in == PER_LEVEL);
  rdf_mode = rdf_mode_in;
  level_surf_rdf = level_surf_rdf_in;
  numbers_of_ranges_in_RDF_log = numbers_of_ranges_in_RDF_log_in;
  memory_usage_in_RDF_log = memory_usage_in_RDF_log_in;
}

SuRF_RDF::~SuRF_RDF() {
  int num_level = getNumberOfTotalLevels();
  if (rdf_mode == PER_LEVEL) {
    int len = level_surf_rdf.size();
    for (int i = 0; i < len; i++) {
      if (level_surf_rdf[i].second != NULL) {
        delete level_surf_rdf[i].second;
      }
    }
  } else if (rdf_mode == PER_FILE) {
    for (int i = 0; i < num_level; i++) {
      auto it = level_file_surf_rdf[i].begin();
      while (it != level_file_surf_rdf[i].end()) {
        delete ((it->second).second);
        it++;
      }
    }
  } else {
    assert(false);
  }
}

void SuRF_RDF::insertRangeDeleteToLevel0(uint64_t file_num,
                                         std::vector<pss>& range_delete_list_in,
                                         bool flag_allow_boundary_overlapped) {
  if (range_delete_list_in.size() == 0) {
    return;
  }

  if (rdf_mode == PER_LEVEL) {
    if (level_surf_rdf.size() == 0) {
      // TODO:  init SuRF_RDF with following parameters set
      //  int key_len_in_bytes = 10;
      //  surf::level_t hash_suffix_len = 0;
      //  surf::level_t real_suffix_len = 0;
      //  bool include_dense = true;
      //  uint32_t sparse_dense_ratio = 16;

      surf::SuRF_Env* _surf_env = surf::SuRF_Env::getInstance();
      int key_len_in_bytes = _surf_env->getSuRFKeyLenInBytes();
      surf::level_t hash_suffix_len = _surf_env->getSuRFHashSuffixLen();
      surf::level_t real_suffix_len = _surf_env->getSuRFRealSuffixLen();
      bool include_dense = _surf_env->getSuRFIncludeDense();
      uint32_t sparse_dense_ratio = _surf_env->getSuRFSparseDenseRatio();

      SuRF* surf = SuRF::rangesToSurf(
          range_delete_list_in, key_len_in_bytes, surf::SuffixType::kNone,
          hash_suffix_len, real_suffix_len, include_dense, sparse_dense_ratio,
          flag_allow_boundary_overlapped);

      level_surf_rdf.push_back(
          std::make_pair(range_delete_list_in.size(), surf));
    } else {
      SuRF* surf = level_surf_rdf[0].second;
      std::vector<pss> range_list = SuRF_RDF::mergeRanges(
          surf->surfToRanges(surf, flag_allow_boundary_overlapped),
          range_delete_list_in);
      delete level_surf_rdf[0].second;

      // TODO:  init SuRF_RDF with following parameters set
      //  int key_len_in_bytes = 10;
      //  surf::level_t hash_suffix_len = 0;
      //  surf::level_t real_suffix_len = 0;
      //  bool include_dense = true;
      //  uint32_t sparse_dense_ratio = 16;

      surf::SuRF_Env* _surf_env = surf::SuRF_Env::getInstance();
      int key_len_in_bytes = _surf_env->getSuRFKeyLenInBytes();
      surf::level_t hash_suffix_len = _surf_env->getSuRFHashSuffixLen();
      surf::level_t real_suffix_len = _surf_env->getSuRFRealSuffixLen();
      bool include_dense = _surf_env->getSuRFIncludeDense();
      uint32_t sparse_dense_ratio = _surf_env->getSuRFSparseDenseRatio();

      SuRF* surf_next = SuRF::rangesToSurf(
          range_delete_list_in, key_len_in_bytes, surf::SuffixType::kNone,
          hash_suffix_len, real_suffix_len, include_dense, sparse_dense_ratio,
          flag_allow_boundary_overlapped);

      level_surf_rdf[0].second = surf_next;
      level_surf_rdf[0].first = range_list.size();
    }
  } else if (rdf_mode == PER_FILE) {
    if (level_file_surf_rdf.size() == 0) {
      std::unordered_map<uint64_t, std::pair<int, SuRF*>>
          level_file_surf_rdf_in;  // fd -> (# RD, surf)
      this->insertRangesAtLevelOfFd(0, file_num, range_delete_list_in,
                                    flag_allow_boundary_overlapped);
    } else {
      std::unordered_map<uint64_t, std::pair<int, SuRF*>>&
          level_file_surf_rdf_in = level_file_surf_rdf[0];
      if (level_file_surf_rdf_in.find(file_num) !=
          level_file_surf_rdf_in.end()) {
        std::cout << "Error. Flusing in file shall have unique file_num and "
                     "greater than previous ones"
                  << std::endl;
      } else {
        this->insertRangesAtLevelOfFd(0, file_num, range_delete_list_in,
                                      flag_allow_boundary_overlapped);
      }
    }
  } else {
    assert(false);
  }
}

void SuRF_RDF::directMoveFileToLevel(uint64_t fd, uint32_t src_level,
                                     uint32_t dst_level) {
  if (rdf_mode == PER_LEVEL) {
    if (src_level >= level_surf_rdf.size()) {
      return;
    }
    if (level_surf_rdf[src_level].second == NULL) {
      return;
    }
    while (dst_level >= level_surf_rdf.size()) {
      std::pair<int, SuRF*> tmp;
      level_surf_rdf.push_back(tmp);
    }
    std::cerr << "Not Implemented Yet, Requiured: file_boundary" << std::endl;
    assert(false);
  } else if (rdf_mode == PER_FILE) {
    if (src_level >= level_file_surf_rdf.size()) {
      return;
    }
    if (level_file_surf_rdf[src_level].count(fd) == 0) {
      return;
    }
    int num_ragnes_in_surf = level_file_surf_rdf[src_level][fd].first;
    if (num_ragnes_in_surf <= 0) {
      assert(num_ragnes_in_surf > 0);
    }
    while (dst_level >= level_file_surf_rdf.size()) {
      std::unordered_map<uint64_t, std::pair<int, SuRF*>> tmp;
      level_file_surf_rdf.push_back(tmp);
    }
    level_file_surf_rdf[dst_level][fd] = level_file_surf_rdf[src_level][fd];
    level_file_surf_rdf[src_level].erase(fd);
  } else {
    assert(false);
  }
}

std::vector<pss> SuRF_RDF::getRangeTombstonesAtLevelOfFd(
    uint32_t src_level, uint64_t fd, bool flag_allow_boundary_overlapped) {
  assert(rdf_mode == surf::SuRF_RDF::RDF_MODE::PER_FILE);
  if (src_level >= level_file_surf_rdf.size()) {
    return std::vector<pss>();
  }
  if (level_file_surf_rdf[src_level].count(fd) == 0) {
    return std::vector<pss>();
  }

#ifdef DEBUG_SURF_COMPACTION
  std::cout << "src_level = " << src_level << " fd = " << fd << " " << __FILE__
            << ":" << __LINE__ << " " << __func__ << std::endl;
#endif
  int num_ranges_in_surf = level_file_surf_rdf[src_level][fd].first;

  if (num_ranges_in_surf <= 0) {
    assert(num_ranges_in_surf > 0);
    std::cout << "Error: num_ranges_in_surf = " << num_ranges_in_surf << " "
              << __FILE__ << ":" << __LINE__ << " " << __func__ << std::endl;
  }
  SuRF* surf = level_file_surf_rdf[src_level][fd].second;
  assert(surf != NULL);
  std::vector<pss> range_list =
      SuRF::surfToRanges(surf, flag_allow_boundary_overlapped);

  return range_list;
}

void SuRF_RDF::removeSuRFAtLevelOfFd(uint32_t src_level, uint64_t fd) {
  assert(rdf_mode == surf::SuRF_RDF::RDF_MODE::PER_FILE);
  if (src_level >= level_file_surf_rdf.size()) {
    return;
  }
  if (level_file_surf_rdf[src_level].count(fd) == 0) {
    // std::cout << "@SuRF remove file. fd = " << fd << " src_level = " <<
    // src_level << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
    // << std::endl;
    return;
  }

  delete level_file_surf_rdf[src_level][fd].second;
  level_file_surf_rdf[src_level].erase(fd);
}

void SuRF_RDF::insertRangesAtLevelOfFd(uint32_t level, uint64_t fd,
                                       std::vector<pss>& ranges,
                                       bool flag_allow_boundary_overlapped) {
  assert(ranges.size() > 0);
  if (ranges.size() <= 0) {
    std::cout << "Error: ranges.size() = " << ranges.size() << " " << __FILE__
              << ":" << __LINE__ << " " << __func__ << std::endl;
  }

  // surf::SuRF_Env *_surf_env = surf::SuRF_Env::getInstance();
  // int surf_key_length_in_bytes = _surf_env->getSuRFKeyLenInBytes();
  std::vector<pss> ranges_processed;
  for (uint32_t i = 0; i < ranges.size(); i++) {
    std::string key_start = ranges[i].first, key_end = ranges[i].second;
    // if(key_start.size() > surf_key_length_in_bytes){
    //     key_start = key_start.substr(0, surf_key_length_in_bytes);
    // }
    // if(key_end.size() > surf_key_length_in_bytes){
    //     key_end = key_end.substr(0, surf_key_length_in_bytes);
    // }

    if (ranges_processed.size() > 0 && key_start == key_end &&
        key_start == ranges_processed.back().first &&
        key_end == ranges_processed.back().second) {
      continue;
    }
    ranges_processed.push_back(make_pair(key_start, key_end));
  }

  surf::SuRF_Env* _surf_env = surf::SuRF_Env::getInstance();
  int key_len_in_bytes = _surf_env->getSuRFKeyLenInBytes();
  surf::level_t hash_suffix_len = _surf_env->getSuRFHashSuffixLen();
  surf::level_t real_suffix_len = _surf_env->getSuRFRealSuffixLen();
  bool include_dense = _surf_env->getSuRFIncludeDense();
  uint32_t sparse_dense_ratio = _surf_env->getSuRFSparseDenseRatio();

  assert(rdf_mode == surf::SuRF_RDF::RDF_MODE::PER_FILE);
  while (level >= level_file_surf_rdf.size()) {
    std::unordered_map<uint64_t, std::pair<int, SuRF*>> tmp;
    level_file_surf_rdf.push_back(tmp);
  }
  assert(level < level_file_surf_rdf.size());
  assert(level_file_surf_rdf[level].count(fd) == 0);

  // SuRF* surf_ = SuRF::rangesToSurf(ranges, key_len_in_bytes,
  // surf::SuffixType::kNone,
  SuRF* surf_ = SuRF::rangesToSurf(
      ranges_processed, key_len_in_bytes, surf::SuffixType::kNone,
      hash_suffix_len, real_suffix_len, include_dense, sparse_dense_ratio,
      flag_allow_boundary_overlapped);

  // std::cout << "ranges.size() = " << ranges.size() << "
  // surf_->getMemoryUsageInBitsSelf() = " <<
  // surf_->getMemoryUsageInBitsSelf()
  // << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<
  // std::endl; assert(ranges.size() > 0); level_file_surf_rdf[level][fd] =
  // std::make_pair(ranges.size(), surf_);
  assert(ranges_processed.size() > 0);
  level_file_surf_rdf[level][fd] =
      std::make_pair(ranges_processed.size(), surf_);
}

void SuRF_RDF::insertRangesWithPointKeysAtLevelOfFd(
    uint32_t level, uint64_t fd, std::vector<pss>& ranges,
    std::vector<std::string>& point_keys, bool flag_allow_boundary_overlapped) {
  assert(ranges.size() > 0);
  if (ranges.size() <= 0) {
    std::cout << "Error: ranges.size() = " << ranges.size() << " " << __FILE__
              << ":" << __LINE__ << " " << __func__ << std::endl;
  }

  // surf::SuRF_Env *_surf_env = surf::SuRF_Env::getInstance();
  // int surf_key_length_in_bytes = _surf_env->getSuRFKeyLenInBytes();
  std::vector<pss> ranges_processed;
  for (uint32_t i = 0; i < ranges.size(); i++) {
    std::string key_start = ranges[i].first, key_end = ranges[i].second;
    // if(key_start.size() > surf_key_length_in_bytes){
    //     key_start = key_start.substr(0, surf_key_length_in_bytes);
    // }
    // if(key_end.size() > surf_key_length_in_bytes){
    //     key_end = key_end.substr(0, surf_key_length_in_bytes);
    // }

    if (ranges_processed.size() > 0 &&
        key_start == ranges_processed.back().first &&
        key_end == ranges_processed.back().second) {
      continue;
    }
    ranges_processed.push_back(make_pair(key_start, key_end));
  }

  // TODO:  init SuRF_RDF with following parameters set
  surf::SuRF_Env* _surf_env = surf::SuRF_Env::getInstance();
  int key_len_in_bytes = _surf_env->getSuRFKeyLenInBytes();
  surf::level_t hash_suffix_len = _surf_env->getSuRFHashSuffixLen();
  surf::level_t real_suffix_len = _surf_env->getSuRFRealSuffixLen();
  bool include_dense = _surf_env->getSuRFIncludeDense();
  uint32_t sparse_dense_ratio = _surf_env->getSuRFSparseDenseRatio();

  assert(rdf_mode == surf::SuRF_RDF::RDF_MODE::PER_FILE);
  while (level >= level_file_surf_rdf.size()) {
    std::unordered_map<uint64_t, std::pair<int, SuRF*>> tmp;
    level_file_surf_rdf.push_back(tmp);
  }
  assert(level < level_file_surf_rdf.size());
  assert(level_file_surf_rdf[level].count(fd) == 0);

  // auto rtn = SuRF::rangesWithPointKeysToSurf(ranges, point_keys,
  auto rtn = SuRF::rangesWithPointKeysToSurf(
      ranges_processed, point_keys, key_len_in_bytes, surf::SuffixType::kNone,
      hash_suffix_len, real_suffix_len, include_dense, sparse_dense_ratio,
      flag_allow_boundary_overlapped);
  SuRF* surf_ = rtn.first;
  size_t split_count = rtn.second;

  // assert(ranges.size() > 0);
  // level_file_surf_rdf[level][fd] =
  // std::make_pair(ranges.size()+split_count, surf_);
  assert(ranges_processed.size() > 0);
  level_file_surf_rdf[level][fd] =
      std::make_pair(ranges_processed.size() + split_count, surf_);
}

void SuRF_RDF::deleteLastLevelIfEqualsBottomLevel(uint bottom_level) {
  if (rdf_mode == PER_LEVEL) {
    while (level_surf_rdf.size() > 0 &&
           level_surf_rdf.size() - 1 >= bottom_level) {
      level_surf_rdf.pop_back();
    }
  } else if (rdf_mode == PER_FILE) {
    while (level_file_surf_rdf.size() > 0 &&
           level_file_surf_rdf.size() - 1 >= bottom_level) {
      std::cout << "remove last level " << bottom_level << " "
                << level_file_surf_rdf.size() - 1 << " " << __FILE__ << ":"
                << __LINE__ << " " << __FUNCTION__ << std::endl;
      for (auto& fd : level_file_surf_rdf.back()) {
        std::cout << "fd = " << fd.first << " " << __FILE__ << ":" << __LINE__
                  << " " << __FUNCTION__ << std::endl;
      }

      level_file_surf_rdf.pop_back();
    }
  } else {
    assert(false);
  }
}

void SuRF_RDF::RemoveSuRF(std::vector<uint32_t>& src_level_list,
                          std::vector<std::vector<uint64_t>>& src_fd_list2d) {
  assert(src_level_list.size() == src_fd_list2d.size());
  size_t len = src_level_list.size();
  for (size_t i = 0; i < len; i++) {
    uint32_t& src_level = src_level_list[i];
    std::vector<uint64_t>& src_fd_list = src_fd_list2d[i];

    for (auto& fd : src_fd_list) {
      this->removeSuRFAtLevelOfFd(src_level, fd);
    }
  }
}

std::vector<pss> SuRF_RDF::gatherSortedRangeTombstonesAndRemoveSuRF(
    std::vector<uint32_t>& src_level_list,
    std::vector<std::vector<uint64_t>>& src_fd_list2d,
    bool surf_flag__allow_range_boundary_overlapped) {
  assert(src_level_list.size() == src_fd_list2d.size());

  std::vector<pss> range_tombstone_list_agg;

  size_t len = src_level_list.size();
  for (size_t i = 0; i < len; i++) {
    uint32_t& src_level = src_level_list[i];
    std::vector<uint64_t>& src_fd_list = src_fd_list2d[i];

    for (auto& fd : src_fd_list) {
      std::vector<pss> range_tombstone_list =
          this->getRangeTombstonesAtLevelOfFd(
              src_level, fd, surf_flag__allow_range_boundary_overlapped);

      this->removeSuRFAtLevelOfFd(src_level, fd);
      for (auto& range_tombstone : range_tombstone_list) {
#ifdef DEBUG_SURF_COMPACTION
        std::cout << "range_tombstone = " << range_tombstone.first << " "
                  << range_tombstone.second << " " << __FILE__ << ":"
                  << __LINE__ << " " << __FUNCTION__ << std::endl;
#endif
        range_tombstone_list_agg.push_back(range_tombstone);
      }
    }
  }
  sort(range_tombstone_list_agg.begin(), range_tombstone_list_agg.end());

  // merge
  std::vector<pss> rd_merged;
  size_t len_rd = range_tombstone_list_agg.size();
  assert(len_rd > 0ULL);
  if (len_rd > 0) {
    rd_merged.push_back(range_tombstone_list_agg[0]);
    for (size_t i_rd = 1; i_rd < len_rd; i_rd++) {
      pss& rd = range_tombstone_list_agg[i_rd];
      pss& rd_last = rd_merged.back();
      if (surf_flag__allow_range_boundary_overlapped) {
        if (rd_last.second > rd.first) {
          rd_last.second = std::max(rd_last.second, rd.second);
        } else {
          rd_merged.push_back(rd);
        }
      } else {
        if (rd_last.second >= rd.first) {
          rd_last.second = std::max(rd_last.second, rd.second);
        } else {
          rd_merged.push_back(rd);
        }
      }
    }
  }

  return rd_merged;
}

void SuRF_RDF::shiftRDFToOutputLevel(
    std::vector<pss>& rd_merged, uint32_t dst_level,
    std::vector<uint64_t>& dst_fd_list, std::vector<pss>& file_boundary_list,
    bool surf_flag__allow_range_boundary_overlapped) {
  // merge
  size_t len_rd = rd_merged.size();
  assert(len_rd > 0ULL);
  if (len_rd > 0) {
    assert(dst_fd_list.size() == file_boundary_list.size());
    size_t len_dst = dst_fd_list.size();
    size_t start_i_rd = 0;
    for (size_t i_dst = 0; i_dst < len_dst; i_dst++) {
      uint64_t dst_fd = dst_fd_list[i_dst];

      if (dst_fd == 163302) {
        std::cout << "dst_level = " << dst_level << " " << "dst_fd = " << dst_fd
                  << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                  << std::endl;
        for (auto& fd2 : dst_fd_list) {
          std::cout << "fd2 = " << fd2 << " " << __FILE__ << ":" << __LINE__
                    << " " << __FUNCTION__ << std::endl;
        }
        for (auto& fb2 : file_boundary_list) {
          std::cout << "fb2 = " << fb2.first << " " << fb2.second << " "
                    << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                    << std::endl;
        }
      }

      pss file_boundary = file_boundary_list[i_dst];
      // 2024-10
      if (file_boundary.first == file_boundary.second) {
        continue;
      }

      std::vector<pss> ranges_to_insert;
      size_t i_rd = start_i_rd;
      if (surf_flag__allow_range_boundary_overlapped == true) {
        while (i_rd < len_rd && rd_merged[i_rd].second <= file_boundary.first) {
          i_rd++;
        }
      } else {
        while (i_rd < len_rd && rd_merged[i_rd].second <= file_boundary.first) {
          i_rd++;
        }
      }
      start_i_rd = i_rd;  // Optimization: next file cannot start before this
                          // i_rd if boundaries are sorted

      while (i_rd < len_rd && rd_merged[i_rd].first < file_boundary.second) {
        pss range_in = std::make_pair(
            std::max(rd_merged[i_rd].first, file_boundary.first),
            std::min(rd_merged[i_rd].second, file_boundary.second));
        ranges_to_insert.push_back(range_in);
        i_rd++;

        if (dst_fd == 163302) {
          std::cout << "ranges_to_insert = " << range_in.first << " "
                    << range_in.second << " " << __FILE__ << ":" << __LINE__
                    << " " << __FUNCTION__ << std::endl;
        }
      }
      // if (i_rd < len_rd && rd_merged[i_rd].first < file_boundary.second) {
      //   pss range_in = std::make_pair(
      //       std::max(rd_merged[i_rd].first, file_boundary.first),
      //       std::min(rd_merged[i_rd].second, file_boundary.second));
      //   ranges_to_insert.push_back(range_in);
      //   // don't i_rd ++;
      // }

      if (ranges_to_insert.size() > 0) {
        this->insertRangesAtLevelOfFd(
            dst_level, dst_fd, ranges_to_insert,
            surf_flag__allow_range_boundary_overlapped);
      }
    }
  }
}

void SuRF_RDF::shiftRDFWithPointKeysToOutputLevel(
    std::vector<pss>& rd_merged, std::vector<std::string>& point_keys,
    uint32_t dst_level, std::vector<uint64_t>& dst_fd_list,
    std::vector<pss>& file_boundary_list,
    bool surf_flag__allow_range_boundary_overlapped) {
#ifdef DEBUG_SURF_COMPACTION
  // check point_keys are sorted and unique-*665
  for (size_t i = 1; i < point_keys.size(); i++) {
    assert(point_keys[i - 1] < point_keys[i]);
    if (point_keys[i - 1] > point_keys[i]) {
      std::cout << "Error: point_keys[i-1] " << point_keys[i - 1]
                << " shall be smaller than point_keys[i] " << point_keys[i]
                << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                << std::endl;
    } else if (point_keys[i - 1] == point_keys[i]) {
      std::cout << "Error: point_keys[i-1] " << point_keys[i - 1]
                << " shall be unique from point_keys[i] " << point_keys[i]
                << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                << std::endl;
    }
  }
#endif
  // merge
  size_t len_rd = rd_merged.size();
  assert(len_rd > 0ULL);
  if (len_rd > 0) {
    assert(dst_fd_list.size() == file_boundary_list.size());
    size_t len_dst = dst_fd_list.size();
    size_t start_i_rd = 0;
    for (size_t i_dst = 0; i_dst < len_dst; i_dst++) {
      uint64_t dst_fd = dst_fd_list[i_dst];

      if (dst_fd == 163302) {
        std::cout << "dst_level = " << dst_level << " " << "dst_fd = " << dst_fd
                  << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                  << std::endl;
        for (auto& fd2 : dst_fd_list) {
          std::cout << "fd2 = " << fd2 << " " << __FILE__ << ":" << __LINE__
                    << " " << __FUNCTION__ << std::endl;
        }
        for (auto& fb2 : file_boundary_list) {
          std::cout << "fb2 = " << fb2.first << " " << fb2.second << " "
                    << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                    << std::endl;
        }
      }

      pss file_boundary = file_boundary_list[i_dst];
      // 2024-10
      if (file_boundary.first == file_boundary.second) {
        continue;
      }
      //
      // separate
      std::vector<pss> ranges_to_insert;
      size_t i_rd = start_i_rd;
      if (surf_flag__allow_range_boundary_overlapped == true) {
        while (i_rd < len_rd && rd_merged[i_rd].second <= file_boundary.first) {
          i_rd++;
        }
      } else {
        // while(i_rd < len_rd && rd_merged[i_rd].second <
        // file_boundary.first){
        while (i_rd < len_rd && rd_merged[i_rd].second <= file_boundary.first) {
          i_rd++;
        }
      }
      start_i_rd = i_rd;  // Optimization: next file cannot start before this
                          // i_rd if boundaries are sorted

      while (i_rd < len_rd && rd_merged[i_rd].second <= file_boundary.second) {
        pss range_in = std::make_pair(
            std::max(rd_merged[i_rd].first, file_boundary.first),
            std::min(rd_merged[i_rd].second, file_boundary.second));
        ranges_to_insert.push_back(range_in);
        i_rd++;

        if (dst_fd == 163302) {
          std::cout << "ranges_to_insert = " << range_in.first << " "
                    << range_in.second << " " << __FILE__ << ":" << __LINE__
                    << " " << __FUNCTION__ << std::endl;
        }
      }
      // if (i_rd < len_rd && rd_merged[i_rd].first < file_boundary.second) {
      //   pss range_in = std::make_pair(
      //       std::max(rd_merged[i_rd].first, file_boundary.first),
      //       std::min(rd_merged[i_rd].second, file_boundary.second));

      //   ranges_to_insert.push_back(range_in);
      // }

      if (ranges_to_insert.size() > 0) {
#ifdef DEBUG_SURF_COMPACTION
        std::cout << "within SuRF:" << " " << __FILE__ << ":" << __LINE__ << " "
                  << __FUNCTION__ << std::endl;
        for (auto& rd : ranges_to_insert) {
          if (surf::SuRF_Env::getInstance()
                  ->getFlagSurfUseCondensedDigitKey() == true) {
            // uint32_t len_condensed_key =
            // surf::SuRF_Env::getInstance()->getLengthOfCondensedDigitKey();
            auto ks =
                surf::SuRF_Utils::decode_byte_string_to_digit_string(rd.first);
            auto ke =
                surf::SuRF_Utils::decode_byte_string_to_digit_string(rd.second);
            std::cout << (ks) << " " << (ke) << std::endl;
          } else {
            std::cout << (rd.first) << " " << (rd.second) << std::endl;
          }
        }
        std::cout << "within SuRF point keys:" << " " << __FILE__ << ":"
                  << __LINE__ << " " << __FUNCTION__ << std::endl;
        for (size_t i = 1; i < point_keys.size(); i++) {
          if (surf::SuRF_Env::getInstance()
                  ->getFlagSurfUseCondensedDigitKey() == true) {
            // uint32_t len_condensed_key =
            // surf::SuRF_Env::getInstance()->getLengthOfCondensedDigitKey();
            auto pk = surf::SuRF_Utils::decode_byte_string_to_digit_string(
                point_keys[i]);
            std::cout << (pk) << " ";
          } else {
            std::cout << (point_keys[i]) << " ";
          }
        }
        std::cout << std::endl;
#endif
        this->insertRangesWithPointKeysAtLevelOfFd(
            dst_level, dst_fd, ranges_to_insert, point_keys,
            surf_flag__allow_range_boundary_overlapped);
      }
    }
  }
}

int SuRF_RDF::getNumberOfRangesAtIthLevel(int level) {
  if (rdf_mode == PER_LEVEL) {
    assert(level < level_surf_rdf.size());
    assert(level_surf_rdf[level].second != NULL);
    return level_surf_rdf[level].first;
  } else if (rdf_mode == PER_FILE) {
    assert(level < level_file_surf_rdf.size());
    int num = 0;
    auto it = level_file_surf_rdf[level].begin();
    while (it != level_file_surf_rdf[level].end()) {
      num += (it->second).first;
      it++;
    }
    return num;
  }
  assert(false);
  return -1;
}
int SuRF_RDF::getNumberOfTotalRanges() {
  int num = 0;
  int level = getNumberOfTotalLevels();

  for (int i = 0; i < level; i++) {
    num += getNumberOfRangesAtIthLevel(i);
  }
  return num;
}
// in bits
uint64_t SuRF_RDF::getMemoryUsageAtIthLevel(int level) {
  if (rdf_mode == PER_LEVEL) {
    assert(level < level_surf_rdf.size());
    assert(level_surf_rdf[level].second != NULL);
    return level_surf_rdf[level].second->getMemoryUsageInBitsSelf();
  } else if (rdf_mode == PER_FILE) {
    assert(level < level_file_surf_rdf.size());
    assert(level_file_surf_rdf[level].size() != 0);
    uint64_t mem = 0;
    auto it = level_file_surf_rdf[level].begin();
    while (it != level_file_surf_rdf[level].end()) {
      auto rd_surf = it->second;
      mem += (rd_surf.second)->getMemoryUsageInBitsSelf();
      it++;
    }
    return mem;
  }
  assert(false);
  return -1;
}

uint64_t SuRF_RDF::getNumberOfTotalMemoryUsage() {
  uint64_t num = 0;
  int level = getNumberOfTotalLevels();
  for (int i = 0; i < level; i++) {
    num += getMemoryUsageAtIthLevel(i);
  }
  return num / 8;  // bits --> bytes
}
void SuRF_RDF::logCurrentTotalNumbersOfRanges() {
  int num = getNumberOfTotalRanges();
  numbers_of_ranges_in_RDF_log.push_back(num);
}
void SuRF_RDF::logCurrentTotalMemoryUsage() {
  uint64_t num = getNumberOfTotalMemoryUsage();
  memory_usage_in_RDF_log.push_back(num);
}

int SuRF_RDF::getNumberOfTotalLevels() {
  int num = 0;
  if (rdf_mode == PER_LEVEL) {
    int len = level_surf_rdf.size();
    for (int i = 0; i < len; i++) {
      if (level_surf_rdf[i].second != NULL) {
        num = i + 1;
      }
    }
  } else if (rdf_mode == PER_FILE) {
    int len = level_file_surf_rdf.size();
    for (int i = 0; i < len; i++) {
      if (level_file_surf_rdf[i].size() != 0) {
        num = i + 1;
      }
    }
  } else {
    assert(false);
  }
  return num;
}

VP SuRF_RDF::getLevelRDF() { return level_surf_rdf; }
std::pair<int, SuRF*> SuRF_RDF::getLevelRDFAtIthLevel(int level) {
  assert(level < level_surf_rdf.size());
  return level_surf_rdf[level];
}

VMP SuRF_RDF::getLevelFileRDF() { return level_file_surf_rdf; }
std::unordered_map<uint64_t, std::pair<int, SuRF*>>
SuRF_RDF::getLevelFileRDFAtIthLevel(int level) {
  assert(level < level_file_surf_rdf.size());
  return level_file_surf_rdf[level];
}

std::vector<int> SuRF_RDF::getNumbersOfRangesInRDFLog() {
  return numbers_of_ranges_in_RDF_log;
}
std::vector<int> SuRF_RDF::getMemoryUsageInRDFLog() {
  return memory_usage_in_RDF_log;
}

SuRF_RDF::RDF_MODE SuRF_RDF::getRDFMode() { return rdf_mode; }

// SuRF_RDF
using vpss = std::vector<std::pair<std::string, std::string>>;
vpss SuRF_RDF::mergeRanges(vpss ranges_1, vpss ranges_2,
                           bool allow_boundary_overlap_not_merged) {
  if (ranges_1.size() == 0) {
    return ranges_2;
  }
  if (ranges_2.size() == 0) {
    return ranges_1;
  }

  vpss ranges_out;
  int len_1 = ranges_1.size();
  int len_2 = ranges_2.size();
  int i = 0, j = 0;
  auto a = ranges_1[i];
  auto b = ranges_2[j];
  if (a <= b) {
    ranges_out.push_back(a);
    i++;
  } else {
    ranges_out.push_back(b);
    j++;
  }

  while (i < len_1 || j < len_2) {
    if (j == len_2) {
      if (ranges_out.back().second < ranges_1[i].first ||
          (ranges_out.back().second == ranges_1[i].first &&
           allow_boundary_overlap_not_merged)) {
        ranges_out.push_back(ranges_1[i]);
      } else {
        ranges_out.back().second =
            max(ranges_out.back().second, ranges_1[i].second);
      }
      i++;
      continue;
    }
    if (i == len_1) {
      if (ranges_out.back().second < ranges_2[j].first ||
          (ranges_out.back().second == ranges_2[j].first &&
           allow_boundary_overlap_not_merged)) {
        ranges_out.push_back(ranges_2[j]);
      } else {
        ranges_out.back().second =
            max(ranges_out.back().second, ranges_2[j].second);
      }
      j++;
      continue;
    }
    a = ranges_1[i];
    b = ranges_2[j];
    if (a <= b) {
      if (ranges_out.back().second < a.first ||
          (ranges_out.back().second == a.first &&
           allow_boundary_overlap_not_merged)) {
        ranges_out.push_back(a);
      } else {
        ranges_out.back().second = max(ranges_out.back().second, a.second);
      }
      i++;
    } else {
      if (ranges_out.back().second < b.first ||
          (ranges_out.back().second == b.first &&
           allow_boundary_overlap_not_merged)) {
        ranges_out.push_back(b);
      } else {
        ranges_out.back().second = max(ranges_out.back().second, b.second);
      }
      j++;
    }
  }

  return ranges_out;
}

// Start: flag_key_may_deleted helper functions
// Make sure reset clearFlagKeyMayDeleted before each keySearching
// checkProperUsageOfFlagKeyMayDeleted helps to check
void SuRF_RDF::clearFilterFalsePositiveRate() {
  clearKeySearchCount();
  clearKeyMayDeletedCount();
}
double SuRF_RDF::getFilterFalsePositiveRate() {
  uint32_t total_key_search_count = getKeySearchCount();
  if (total_key_search_count == 0) {
    return -1;
  }
  return 1.0 * getKeyMayDeletedCount() / total_key_search_count;
}
// use clearFilterFalsePositiveRate for public called
void SuRF_RDF::clearKeySearchCount() { key_search_count = 0; }
void SuRF_RDF::incKeySearchCount() {
  if (key_search_count ==
      std::numeric_limits<decltype(key_search_count)>::max()) {
    std::cerr << "Error: overflow of key_search_count " << __FILE__ << ":"
              << __LINE__ << " " << __FUNCTION__ << std::endl;
  }
  key_search_count += 1;
}
uint32_t SuRF_RDF::getKeySearchCount() { return key_search_count; }
// use clearFilterFalsePositiveRate for public called
void SuRF_RDF::clearKeyMayDeletedCount() { key_may_deleted_count = 0; }
uint32_t SuRF_RDF::getKeyMayDeletedCount() { return key_may_deleted_count; }
void SuRF_RDF::setFlagKeyMayDeleted() {
  if (key_may_deleted_count ==
      std::numeric_limits<decltype(key_may_deleted_count)>::max()) {
    std::cerr << "Error: overflow of key_may_deleted_count " << __FILE__ << ":"
              << __LINE__ << " " << __FUNCTION__ << std::endl;
  }
  key_may_deleted_count += 1;
  flag_key_may_deleted = true;
}
bool SuRF_RDF::getFlagKeyMayDeleted() {
  checkProperUsageOfFlagKeyMayDeleted();
  return flag_key_may_deleted;
}
void SuRF_RDF::clearFlagKeyMayDeleted() {
  flag_key_may_deleted = false;
  key_search_count_kmd = 0;
}
void SuRF_RDF::incKeySearchCountKMD() { key_search_count_kmd += 1; }
void SuRF_RDF::checkProperUsageOfFlagKeyMayDeleted() {
  if (key_search_count_kmd > 1) {
    std::cout << "Error: key_search_count_kme shall be 0 or 1."
              << " Make sure clearFlagKeyMayDeleted is called in the beginning "
                 "of all the series of isEntryAlive functions"
              << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
              << std::endl;
    std::cerr << "Error: key_search_count_kme shall be 0 or 1."
              << " Make sure clearFlagKeyMayDeleted is called in the beginning "
                 "of all the series of isEntryAlive functions"
              << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
              << std::endl;
    exit(-1);
  }
}
// End: flag_key_may_exist helper functions

// [a, b), [c, d)
// 1. flag_bypass_if_same_key (== allow_boundary_overlapped) == 0
//         [       )
//                    [      )
// Alive:  X  xxx  0  X xxxx O
// left :  1       0  1      0
// right:  0       1  0      1
// 2. flag_bypass_if_same_key (== allow_boundary_overlapped)  == 1
//         (       )
//                 (      )
// Alive:  O  xxx  O xxxx O
// left :  1       1      0
// right:  0       1      1
//
//
// @insertion i) if inserted keys != boundary keys: add right bracket
// 1. flag_bypass_if_same_key (== allow_boundary_overlapped) == 0
//  inserted_key--
//               |
//         [     )  )
//                     [      )
// Alive:  X  xxx   0  X xxxx O
// left :  1        0  1      0
// right:  0        1  0      1
// 2. flag_bypass_if_same_key (== allow_boundary_overlapped)  == 1
//  inserted_key--
//               |
//         (     )  )
//                  (      )
// Alive:  O  xxx   O xxxx O
// left :  1        1      0
// right:  0        1      1
//
// @insertion ii) if inserted keys == boundary left key: 2 cases
// 1. flag_bypass_if_same_key (== allow_boundary_overlapped) == 0
//  inserted_key--------        (add right key)
//                     |
//                     |
//         [        )  )
//                     [      )
// Alive:  X  xxx   0  X xxxx O
// left :  1        0  1      0
// right:  0        1  1      1
// 2. flag_bypass_if_same_key (== allow_boundary_overlapped)  == 1
//  inserted_key-----           (doing nothing)
//                  |
//                  x
//         (        )
//                  (      )
// Alive:  O  xxx   O xxxx O
// left :  1        1      0
// right:  0        1      1
//
//
// @checking is covered by a range i) if searched key != boundary keys: 2
// cases
// 1. flag_bypass_if_same_key (== allow_boundary_overlapped) == 0
//  searched_key--  ---        (look right, if left & right parentheses -> not
//  covered, else if right parentheses -> covered, else left parentheses ->
//  not covered)
//               |    |
//         [        )
//                     [      )
// Alive:  X  xxx   0  X xxxx O
// left :  1        0  1      0
// right:  0        1  0      1
// 2. flag_bypass_if_same_key (== allow_boundary_overlapped)  == 1
//  searched_key-- ---        (look right, if right parentheses -> covered,
//  else left parentheses -> not covered)
//               |   |
//         (        )
//                    (      )
// Alive:  O  xxx   O O xxxx O
// left :  1        0 1      0
// right:  0        1 0      1
//
// @checking is covered by a range ii) if searched key == boundary left key: 2
// cases
// 1. flag_bypass_if_same_key (== allow_boundary_overlapped) == 0
//  searched_key--------      (look right (current key), if has right
//  parentheses -> not covered, else covered)
//                  |  |
//                  |  |
//         [        )  )
//                     [      )
// Alive:  X  xxx   0  X xxxx O
// left :  1        0  1      0
// right:  0        1  1      1
// 2. flag_bypass_if_same_key (== allow_boundary_overlapped)  == 1
//  searched_key-----           (look right (current key), -> not covered)
//                  |
//                  x
//         (        )
//                  (      )
// Alive:  O  xxx   O xxxx O
// left :  1        1      0
// right:  0        1      1
// #define DEBUG_SURF_GET_PATH
bool SuRF_RDF::isEntryAliveAtLevelOfFd(level_t level, uint64_t fd,
                                       std::string key,
                                       bool flag_bypass_if_same_key) {
  assert(rdf_mode == PER_FILE);
  assert(level < level_file_surf_rdf.size());
  clearFlagKeyMayDeleted();
  incKeySearchCountKMD();
  incKeySearchCount();
  if (level >= level_file_surf_rdf.size()) {
    return true;
  }
  assert(level_file_surf_rdf[level].count(fd) > 0);
  if (level_file_surf_rdf[level].count(fd) == 0) {
    return true;
  }
  SuRF* surf = (((level_file_surf_rdf[level].find(fd))->second).second);

  auto key_len_in_bytes = key.size();
  assert(key_len_in_bytes > 0);
  if (key_len_in_bytes == 0) {
    std::cout << "Error: key_len_in_bytes should not be 0 " << __FILE__ << ":"
              << __LINE__ << " " << __func__ << std::endl;
  }

  surf::SuRF_Env* _surf_env = surf::SuRF_Env::getInstance();
  uint32_t surf_key_length_in_bytes = _surf_env->getSuRFKeyLenInBytes();

  bool flag_direct_return_if_found_key_end_with_same_prefix = false;
  if (surf::SuRF_Env::getInstance()->getFlagUseSuRFBase() == true) {
    flag_direct_return_if_found_key_end_with_same_prefix = true;
  } else {
    // for surf with all the same key length
    if (key.size() > surf_key_length_in_bytes) {
      // std::cout << "key = " << key;
      key = key.substr(0, surf_key_length_in_bytes);
      // std::cout << " key = " << key << " " << __FILE__ << ":" << __LINE__
      // << " " << __FUNCTION__ << std::endl;
    }
  }

  // SuRF::Iter iter = surf->moveToNextCommonPrefixKey(key);
  SuRF::Iter iter = surf->moveToNextCommonPrefixKey(
      key, flag_direct_return_if_found_key_end_with_same_prefix);
  // TODO: for surf with different key length --> return the iter also if surf
  // key ends and completely matching the prefix of the searching_key

  bool non_overlapping = true;

  if (surf->getLoudsDenseHeight() > 0) {
#ifdef DEBUG_SURF_GET_PATH
    std::cout << "height > 0 iter.isValid()=" << iter.isValid() << " "
              << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
              << std::endl;
#endif
    if (iter.isValid()) {
      non_overlapping = false;
      std::string key_found = iter.getKey();
      if (surf::SuRF_Env::getInstance()->getFlagUseSuRFBase() == true) {
        key = key.substr(0, key_found.size());
      }

      //             // //TODO: To be added
      //             if(surf::SuRF_Env::getInstance()->getFlagUseSuRFBase() ==
      //             true){
      //                 if(key == key_found){
      //                     non_overlapping = true;
      //                     setFlagKeyMayDeleted();
      //                     return non_overlapping;
      //                 }else{
      //                     // SuRF::Iter iter2 =
      //                     surf->moveToNextCommonPrefixKey(key); SuRF::Iter
      //                     &iter2 = iter;
      //                     // iter2--;
      //                     // if(iter2.isValid() == false){
      //                     //     iter2 = surf->moveToLast();
      //                     // }else{
      //                     //     iter2 = iter;
      //                     //     iter--;
      //                     // }
      //                     // if(iter2.isValid()){
      //                     if((iter2--) == true){
      //                         std::string key_found2 = iter2.getKey();
      // #ifdef DEBUG_SURF_GET_PATH
      //                         std::cout << "(dense) iter2-- key_found = "
      //                         << key_found << " " << __FILE__ << ":" <<
      //                         __LINE__
      //                         << " " << __FUNCTION__ << std::endl;
      // #endif
      //                         // key_found.size() >= key.size() not exist
      //                         if(key.substr(0,key_found2.size()) ==
      //                         key_found2){
      //                             non_overlapping = true;
      //                             setFlagKeyMayDeleted();
      //                             return non_overlapping;
      //                         }else{
      //                             //key > key_found2
      //                         }
      //                     }else{
      //                         //key < key_found (must be the start key)
      //                         non_overlapping = true;
      //                         return non_overlapping;
      //                     }
      //                 }
      //             }
#ifdef DEBUG_SURF_GET_PATH
      if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
          true) {
        // uint32_t len_condensed_key =
        // surf::SuRF_Env::getInstance()->getLengthOfCondensedDigitKey();
        auto ks = surf::SuRF_Utils::decode_byte_string_to_digit_string(key);
        auto kf =
            surf::SuRF_Utils::decode_byte_string_to_digit_string(key_found);
        std::cout << "(dense iter) key_found = " << kf
                  << " key_searched = " << ks << " " << __FILE__ << ":"
                  << __LINE__ << " " << __func__ << std::endl;
      } else {
        std::cout << "(dense iter) key_found = " << key_found
                  << " key_searched = " << key << " " << __FILE__ << ":"
                  << __LINE__ << " " << __func__ << std::endl;
      }
#endif

      if (key_found < key) {
        // if(key_found < key.substr(0, key_found.size())){
        assert(false);
        std::cout << "Error: key_found < key " << __FILE__ << ":" << __LINE__
                  << " " << __func__ << std::endl;
      }

      // [a, b), [c, d)
      // 1. flag_bypass_if_same_key == 0
      //         [       )
      //                    [      )
      // Alive:  X  xxx  0  X xxxx O
      // left :  1       0  1      0
      // right:  0       1  0      1
      // 2. flag_bypass_if_same_key == 1
      //         (       )
      //                 (      )
      // Alive:  O  xxx  O xxxx O
      // left :  1       1      0
      // right:  0       1      1
      else if (key_found == key) {
        if (flag_bypass_if_same_key == true) {
          non_overlapping = true;
          setFlagKeyMayDeleted();
        } else {
          // if(iter.getRightParenthesis() == true){
          //     non_overlapping = true;
          //     setFlagKeyMayDeleted();
          // }else{
          //     non_overlapping = false;
          // }

          // if(iter.getSparseIter()->getRightParenthesis() == true){
          if (iter.getSparseIter()->getLeftParenthesis() == true &&
              iter.getSparseIter()->getRightParenthesis() == true) {
            // (1,1) -- end of a range is the start of an another range
#ifdef DEBUG_SURF_GET_PATH
            std::cout << "Error: shouldn't exist left, right = true for the "
                         "non overlapped ranges case "
                      << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                      << std::endl;
#endif
          } else if (iter.getSparseIter()->getLeftParenthesis() == false &&
                     iter.getSparseIter()->getRightParenthesis() == true) {
            // (0,1) -- end, or split point_key within a range
            non_overlapping = true;
            // setFlagKeyMayDeleted();
#ifdef DEBUG_SURF_GET_PATH
            std::cout
                << "key_found == key " << key
                << " left false right_parenthesis = true, flag_may_Deleted "
                << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                << std::endl;
#endif
          } else if (iter.getSparseIter()->getLeftParenthesis() == false &&
                     iter.getSparseIter()->getRightParenthesis() == false) {
            // (0,0) -- split point_key at range start
            non_overlapping = true;
            // setFlagKeyMayDeleted();
#ifdef DEBUG_SURF_GET_PATH
            std::cout
                << "key_found == key " << key
                << " left false right_parenthesis = false, flag_may_Deleted "
                << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                << std::endl;
#endif
          } else {
            // (1,0) -- start
            non_overlapping = false;
          }
        }
      } else {
        // if(flag_bypass_if_same_key == true){
        //     non_overlapping = (iter.getRightParenthesis() != true);
        // }else{
        //     if(iter.getLeftParenthesis() == true &&
        //     iter.getRightParenthesis() == true){
        //         non_overlapping = true;
        //     }else{
        //         non_overlapping = (iter.getRightParenthesis() != true);
        //     }
        // }

        if (flag_bypass_if_same_key == true) {
          // (1,0), (0,0) --> true
          // (0,1), (1,1) --> false
          non_overlapping =
              (iter.getSparseIter()->getRightParenthesis() != true);
        } else {
          // (1,0), (0,0) --> true
          // (0,1) --> false
          // (1,1) --> Not exist X
          // if(iter.getSparseIter()->getLeftParenthesis() == true &&
          // iter.getSparseIter()->getRightParenthesis() == true){
          //     non_overlapping = true;
          //     std::cout << "left == right == true " << __FILE__ << ":" <<
          //     __LINE__ << " " << __FUNCTION__ << std::endl;
          // }else{
          //     non_overlapping =
          //     (iter.getSparseIter()->getRightParenthesis()
          //     != true);
          // }

          // if(iter.getSparseIter()->getRightParenthesis() == true){
          if (iter.getSparseIter()->getLeftParenthesis() == true &&
              iter.getSparseIter()->getRightParenthesis() == true) {
            // (1,1) -- end of a range is the start of an another range
            std::cout << "Error: shouldn't exist left, right = true for the "
                         "non overlapped ranges case "
                      << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                      << std::endl;
          } else {
            non_overlapping =
                (iter.getSparseIter()->getRightParenthesis() != true);
          }
        }
      }
    }
  } else {
#ifdef DEBUG_SURF_GET_PATH
    std::cout << "height == 0 iter.getSparseIter()->isValid()="
              << iter.getSparseIter()->isValid() << " " << __FILE__ << ":"
              << __LINE__ << " " << __FUNCTION__ << std::endl;
#endif
    if (iter.getSparseIter()->isValid()) {
      non_overlapping = false;
      std::string key_found = iter.getSparseIter()->getKey();
      if (surf::SuRF_Env::getInstance()->getFlagUseSuRFBase() == true) {
        key = key.substr(0, key_found.size());
      }

      //             // //TODO: To be added
      //             if(surf::SuRF_Env::getInstance()->getFlagUseSuRFBase() ==
      //             true){
      //                 if(key == key_found){
      //                     non_overlapping = true;
      //                     setFlagKeyMayDeleted();
      //                     return non_overlapping;
      //                 }else{
      // #ifdef DEBUG_SURF_GET_PATH
      // SuRF::Iter iter3 = surf->moveToNextCommonPrefixKey(key);
      // while((iter3--) ==true){
      //     std::string key_found3 = iter3.getSparseIter()->getKey();
      //     std::cout << "iter3-- key_found3 = " << key_found3 << " " <<
      //     __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      // }
      // #endif
      //                     // SuRF::Iter iter2 =
      //                     surf->moveToNextCommonPrefixKey(key); SuRF::Iter
      //                     &iter2 = iter;
      //                     // iter2--;
      //                     // if(iter2.isValid() == false){
      //                     //     iter2 = surf->moveToLast();
      //                     // }else{
      //                     //     iter2 = iter;
      //                     //     iter--;
      //                     // }
      //                     // if(iter2.isValid()){
      //                     if((iter2--) == true){
      //                         std::string key_found2 =
      //                         iter2.getSparseIter()->getKey();
      // #ifdef DEBUG_SURF_GET_PATH
      //                         std::cout << "iter2-- key_found2 = " <<
      //                         key_found2 << " " << __FILE__ << ":" <<
      //                         __LINE__ << " " << __FUNCTION__ << std::endl;
      // #endif
      // std::cout << "iter2-- key_found2 = " << key_found2 << " " << __FILE__
      // << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
      //                         // key_found.size() >= key.size() not exist
      //                         if(key.substr(0,key_found2.size()) ==
      //                         key_found2){
      //                             non_overlapping = true;
      //                             setFlagKeyMayDeleted();
      //                             return non_overlapping;
      //                         }else{
      //                             //key > key_found2
      //                         }
      //                     }else{
      //                         //key < key_found (must be the start key)
      // #ifdef DEBUG_SURF_GET_PATH
      //                         std::cout << "iter2-- = false  " << "key = "
      //                         << key << " key_found = " << key_found << "
      //                         getFlagKeyMayDeleted " <<
      //                         getFlagKeyMayDeleted() << " " << __FILE__ <<
      //                         ":" << __LINE__ << " " << __FUNCTION__ <<
      //                         std::endl;
      // #endif
      // std::cout << "iter2-- = false  " << "key = " << key << " key_found =
      // "
      // << key_found << " getFlagKeyMayDeleted " << getFlagKeyMayDeleted() <<
      // " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<
      // std::endl;

      //                         non_overlapping = true;
      //                         return non_overlapping;
      //                     }
      //                 }
      //             }
#ifdef DEBUG_SURF_GET_PATH
      if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
          true) {
        // uint32_t len_condensed_key =
        // surf::SuRF_Env::getInstance()->getLengthOfCondensedDigitKey();
        auto ks = surf::SuRF_Utils::decode_byte_string_to_digit_string(key);
        auto kf =
            surf::SuRF_Utils::decode_byte_string_to_digit_string(key_found);
        std::cout << "(sparse iter) key_found = " << kf
                  << " key_searched = " << ks << " " << __FILE__ << ":"
                  << __LINE__ << " " << __func__ << std::endl;
      } else {
        std::cout << "(sparse iter) key_found = " << key_found
                  << " key_searched = " << key << " " << __FILE__ << ":"
                  << __LINE__ << " " << __func__ << std::endl;
      }
#endif

      if (key_found < key) {
        assert(false);
        std::cout << "Error: key_found < key " << __FILE__ << ":" << __LINE__
                  << " " << __func__ << std::endl;
      }

      // [a, b), [c, d)
      // 1. flag_bypass_if_same_key == 0
      //         [       )
      //                    [      )
      // Alive:  X  xxx  0  X xxxx O
      // left :  1       0  1      0
      // right:  0       1  0      1
      // 2. flag_bypass_if_same_key == 1
      //         (       )
      //                 (      )
      // Alive:  O  xxx  O xxxx O
      // left :  1       1      0
      // right:  0       1      1
      else if (key_found == key) {
        // if(surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey()
        // == true){
        //     // uint32_t len_condensed_key =
        //     surf::SuRF_Env::getInstance()->getLengthOfCondensedDigitKey();
        //     auto ks =
        //     surf::SuRF_Utils::decode_byte_string_to_digit_string(key); auto
        //     kf =
        //     surf::SuRF_Utils::decode_byte_string_to_digit_string(key_found);
        //     std::cout << "key_found = " << kf << " key = " << ks << "
        //     flag_bypass_if_same_key = " << flag_bypass_if_same_key << " "
        //     <<
        //     __FILE__ << ":" <<__LINE__ << " " << std::endl;
        // }else{
        //     std::cout << "key_found = " << key_found << " key = " << key <<
        //     " flag_bypass_if_same_key = " << flag_bypass_if_same_key << " "
        //     <<
        //     __FILE__ << ":" <<__LINE__ << " " << std::endl;
        // }
        if (flag_bypass_if_same_key == true) {
          non_overlapping = true;
          setFlagKeyMayDeleted();
        } else {
          // if(iter.getSparseIter()->getRightParenthesis() == true){
          if (iter.getSparseIter()->getLeftParenthesis() == true &&
              iter.getSparseIter()->getRightParenthesis() == true) {
            // (1,1) -- end of a range is the start of an another range
#ifdef DEBUG_SURF_GET_PATH
            std::cout << "Error: shouldn't exist left, right = true for the "
                         "non overlapped ranges case "
                      << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                      << std::endl;
#endif
          } else if (iter.getSparseIter()->getLeftParenthesis() == false &&
                     iter.getSparseIter()->getRightParenthesis() == true) {
            // (0,1) -- end, or split point_key within a range
            non_overlapping = true;
            // setFlagKeyMayDeleted();
#ifdef DEBUG_SURF_GET_PATH
            std::cout
                << "key_found == key " << key
                << " left false right_parenthesis = true, flag_may_Deleted "
                << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                << std::endl;
#endif
          } else if (iter.getSparseIter()->getLeftParenthesis() == false &&
                     iter.getSparseIter()->getRightParenthesis() == false) {
            // (0,0) -- split point_key at range start
            non_overlapping = true;
            // setFlagKeyMayDeleted();
#ifdef DEBUG_SURF_GET_PATH
            std::cout
                << "key_found == key " << key
                << " left false right_parenthesis = false, flag_may_Deleted "
                << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                << std::endl;
#endif
          } else {
            // (1,0) -- start
            non_overlapping = false;
          }
        }
#ifdef DEBUG_SURF_GET_PATH
        std::cout << "key_found == key" << std::endl;
        std::cout << "flag_bypass_if_same_key = " << flag_bypass_if_same_key
                  << " " << __FILE__ << ":" << __LINE__ << " " << __func__
                  << std::endl;
        std::cout << "non_overlapping = " << non_overlapping << " " << __FILE__
                  << ":" << __LINE__ << " " << __func__ << std::endl;
        std::cout << "left_parenthesis = "
                  << iter.getSparseIter()->getLeftParenthesis() << " "
                  << " right_parenthesis = "
                  << iter.getSparseIter()->getRightParenthesis() << " "
                  << __FILE__ << ":" << __LINE__ << " " << __func__
                  << std::endl;
#endif
      } else {
        // if(surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey()
        // == true){
        //     // uint32_t len_condensed_key =
        //     surf::SuRF_Env::getInstance()->getLengthOfCondensedDigitKey();
        //     auto ks =
        //     surf::SuRF_Utils::decode_byte_string_to_digit_string(key); auto
        //     kf =
        //     surf::SuRF_Utils::decode_byte_string_to_digit_string(key_found);
        //     std::cout << "key_found = " << kf << " > key = " << ks << "
        //     flag_bypass_if_same_key = " << flag_bypass_if_same_key << " "
        //     <<
        //     __FILE__ << ":" <<__LINE__ << " " << std::endl;
        // }else{
        //     std::cout << "key_found = " << key_found << " > key = " << key
        //     << " flag_bypass_if_same_key = " << flag_bypass_if_same_key <<
        //     " "
        //     << __FILE__ << ":" <<__LINE__ << " " << std::endl;
        // }
        if (flag_bypass_if_same_key == true) {
          // (1,0), (0,0) --> true
          // (0,1), (1,1) --> false
          non_overlapping =
              (iter.getSparseIter()->getRightParenthesis() != true);
        } else {
          // (1,0), (0,0) --> true
          // (0,1) --> false
          // (1,1) --> Not exist X
          // if(iter.getSparseIter()->getLeftParenthesis() == true &&
          // iter.getSparseIter()->getRightParenthesis() == true){
          //     non_overlapping = true;
          //     std::cout << "left == right == true " << __FILE__ << ":" <<
          //     __LINE__ << " " << __FUNCTION__ << std::endl;
          // }else{
          //     non_overlapping =
          //     (iter.getSparseIter()->getRightParenthesis()
          //     != true);
          // }

          // if(iter.getSparseIter()->getRightParenthesis() == true){
          if (iter.getSparseIter()->getLeftParenthesis() == true &&
              iter.getSparseIter()->getRightParenthesis() == true) {
            // (1,1) -- end of a range is the start of an another range
            std::cout << "Error: shouldn't exist left, right = true for the "
                         "non overlapped ranges case "
                      << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                      << std::endl;
          } else {
            non_overlapping =
                (iter.getSparseIter()->getRightParenthesis() != true);
          }
        }

#ifdef DEBUG_SURF_GET_PATH
        std::cout << "key_found != key" << std::endl;
        std::cout << "non_overlapping = " << non_overlapping << " " << __FILE__
                  << ":" << __LINE__ << " " << __func__ << std::endl;
        std::cout << "left_parenthesis = "
                  << iter.getSparseIter()->getLeftParenthesis() << " "
                  << " right_parenthesis = "
                  << iter.getSparseIter()->getRightParenthesis() << " "
                  << __FILE__ << ":" << __LINE__ << " " << __func__
                  << std::endl;
#endif
      }
    } else {
      // invalid
      if (surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() ==
          true) {
        // uint32_t len_condensed_key =
        // surf::SuRF_Env::getInstance()->getLengthOfCondensedDigitKey();
        auto ks = surf::SuRF_Utils::decode_byte_string_to_digit_string(key);
#ifdef DEBUG_SURF_GET_PATH
        std::cout << "Invalid @fd = " << fd << " key =" << key
                  << " flag_bypass_if_same_key = " << ks << " " << __FILE__
                  << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
#endif
      } else {
#ifdef DEBUG_SURF_GET_PATH
        std::cout << "Invalid @fd = " << fd << " key =" << key
                  << " flag_bypass_if_same_key = " << flag_bypass_if_same_key
                  << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__
                  << std::endl;
#endif
      }
    }
  }
  return non_overlapping;
}

// bool  SuRF_RDF::isEntryAlive(level_t level, std::string key, uint64_t fd,
// bool flag_bypass_if_same_key) {
//     assert(level < level_file_surf_rdf.size());
//     assert(rdf_mode == PER_FILE);
//     clearFlagKeyMayDeleted();
//     incKeySearchCountKMD();

//     auto it_level_file_surf = level_file_surf_rdf[level].find(fd);
//     if(it_level_file_surf == level_file_surf_rdf[level].end()){
//         assert(false);
//     }

//     bool overlapping = false;
//     if(it_level_file_surf != level_file_surf_rdf[level].end()){
//         SuRF* surf = it_level_file_surf->second.second;
//         SuRF::Iter iter = surf->moveToNextCommonPrefixKey(key);

//         if(surf->getLoudsDenseHeight() > 0){
//             auto key_len_in_bytes = key.size();
//             assert(key_len_in_bytes > 0);

//             if(iter.isValid()){
//                 std::string key_found = iter.getKey();
//                 if(key_found.size() < key_len_in_bytes){
//                     overlapping = (iter.getRightParenthesis() != true);
//                 }else if(key_found.size() == key_len_in_bytes){
//                     overlapping = flag_bypass_if_same_key? true:
//                     (iter.getRightParenthesis() != true);
//                 }else{
//                     overlapping = (key_found != key) &&
//                     (iter.getRightParenthesis() != true);
//                 }
//             }
//         }else{
//             auto key_len_in_bytes = key.size();
//             assert(key_len_in_bytes > 0);

//             if(iter.getSparseIter()->isValid()){
//                 std::string key_found = iter.getSparseIter()->getKey();
//                 if(key_found.size() < key_len_in_bytes){
//                     overlapping =
//                     (iter.getSparseIter()->getRightParenthesis() != true);
//                 }else if(key_found.size() == key_len_in_bytes){
//                     overlapping = flag_bypass_if_same_key? true:
//                     (iter.getSparseIter()->getRightParenthesis() != true);
//                 }else{
//                     overlapping = (key_found != key) &&
//                     (iter.getSparseIter()->getRightParenthesis() != true);
//                 }
//             }
//         }
//     }
//     if(key.substr(0, key_found.size()) == key_found && overlapping ==
//     false){
//          setFlagKeyMayDeleted();
//     }
//     return overlapping;
// }

void SuRF_RDF::print(bool flag_allow_boundary_overlapped) {
  if (rdf_mode == PER_LEVEL) {
    for (size_t i = 0; i < level_surf_rdf.size(); i++) {
      std::cout << "Level: " << i << std::endl;
      if (level_surf_rdf[i].second != NULL) {
        int len_rd = level_surf_rdf[i].first;
        SuRF* surf = level_surf_rdf[i].second;
        std::vector<pss> range_list =
            SuRF::surfToRanges(surf, flag_allow_boundary_overlapped);
        std::cout << " Number of ranges: " << len_rd << " Dense part (bits): "
                  << surf->getDensePartMemoryUsageInBitsSelf()
                  << " Sparse part (bits): "
                  << surf->getSparsePartMemoryUsageInBitsSelf()
                  << " All (bytes): " << (surf->getMemoryUsageInBitsSelf() / 8)
                  << std::endl;
        std::cout << "\t";
        for (auto& range : range_list) {
          if (surf::SuRF_Env::getInstance()
                  ->getFlagSurfUseCondensedDigitKey() == true) {
            auto ks = surf::SuRF_Utils::decode_byte_string_to_digit_string(
                range.first);
            auto ke = surf::SuRF_Utils::decode_byte_string_to_digit_string(
                range.second);

            std::cout << "(" << ks << ", " << ke << ") ";
          } else {
            std::cout << "(" << range.first << ", " << range.second << ") ";
          }
        }
        std::cout << std::endl;
      }
    }
  } else if (rdf_mode == PER_FILE) {
    for (size_t i = 0; i < level_file_surf_rdf.size(); i++) {
      std::cout << "Level: " << i << std::endl;
      auto it = level_file_surf_rdf[i].begin();
      while (it != level_file_surf_rdf[i].end()) {
        SuRF* surf = (it->second).second;
        std::cout << "\tFile: " << it->first;
        std::cout << " Number of ranges: " << (it->second).first
                  << " Dense part (bits): "
                  << surf->getDensePartMemoryUsageInBitsSelf()
                  << " Sparse part (bits): "
                  << surf->getSparsePartMemoryUsageInBitsSelf()
                  << " All (bytes): " << (surf->getMemoryUsageInBitsSelf() / 8)
                  << std::endl;
        std::vector<pss> range_list =
            SuRF::surfToRanges(surf, flag_allow_boundary_overlapped);
        std::cout << "\t";
        for (auto& range : range_list) {
          if (surf::SuRF_Env::getInstance()
                  ->getFlagSurfUseCondensedDigitKey() == true) {
            auto ks = surf::SuRF_Utils::decode_byte_string_to_digit_string(
                range.first);
            auto ke = surf::SuRF_Utils::decode_byte_string_to_digit_string(
                range.second);

            std::cout << "(" << ks << ", " << ke << ") ";
          } else {
            std::cout << "(" << range.first << ", " << range.second << ") ";
          }
        }
        std::cout << std::endl;
        it++;
      }
    }
  } else {
    assert(false);
  }
}

// YCHUANG ADDED END

}  // namespace surf

#endif  // SURF_H
