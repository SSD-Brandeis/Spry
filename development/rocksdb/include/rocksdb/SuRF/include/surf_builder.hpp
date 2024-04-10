#ifndef SURFBUILDER_H_
#define SURFBUILDER_H_

#include <assert.h>

#include <string>
#include <vector>

#include "config.hpp"
#include "hash.hpp"
#include "suffix.hpp"

namespace surf {

class SuRFBuilder {
public: 
    SuRFBuilder() : sparse_start_level_(0), suffix_type_(kNone) {};
    explicit SuRFBuilder(bool include_dense, uint32_t sparse_dense_ratio,
			 SuffixType suffix_type, level_t hash_suffix_len, level_t real_suffix_len)
	: include_dense_(include_dense), sparse_dense_ratio_(sparse_dense_ratio),
	  sparse_start_level_(0), suffix_type_(suffix_type),
          hash_suffix_len_(hash_suffix_len), real_suffix_len_(real_suffix_len) {};

    ~SuRFBuilder() {};

    // Fills in the LOUDS-dense and sparse vectors (members of this class)
    // through a single scan of the sorted key list.
    // After build, the member vectors are used in SuRF constructor.
    // REQUIRED: provided key list must be sorted.
    void build(const std::vector<std::string>& keys);

    // YCHUANG ADDED START
    // void build(const std::vector<std::string>& keys, const std::vector<bool>& left_parentheses, const std::vector<bool>& right_parentheses, const bool flag_build_until_unique, const uint16_t max_num_level=-1);
    void build(const std::vector<std::string>& keys, const std::vector<bool>& left_parentheses, const std::vector<bool>& right_parentheses, const uint16_t max_num_level=-1);
    // YCHUANG ADDED END

    static bool readBit(const std::vector<word_t>& bits, const position_t pos) {
	assert(pos < (bits.size() * kWordSize));
	position_t word_id = pos / kWordSize;
	position_t offset = pos % kWordSize;
	return (bits[word_id] & (kMsbMask >> offset));
    }

    static void setBit(std::vector<word_t>& bits, const position_t pos) {
        assert(pos < (bits.size() * kWordSize));
        position_t word_id = pos / kWordSize;
        position_t offset = pos % kWordSize;
        bits[word_id] |= (kMsbMask >> offset);
    }

    level_t getTreeHeight() const {
	return labels_.size();
    }

    // const accessors
    const std::vector<std::vector<word_t> >& getBitmapLabels() const {
	return bitmap_labels_;
    }
    const std::vector<std::vector<word_t> >& getBitmapChildIndicatorBits() const {
	return bitmap_child_indicator_bits_;
    }
    const std::vector<std::vector<word_t> >& getPrefixkeyIndicatorBits() const {
	return prefixkey_indicator_bits_;
    }
    const std::vector<std::vector<label_t> >& getLabels() const {
	return labels_;
    }
    const std::vector<std::vector<word_t> >& getChildIndicatorBits() const {
	return child_indicator_bits_;
    }
    const std::vector<std::vector<word_t> >& getLoudsBits() const {
	return louds_bits_;
    }
    const std::vector<std::vector<word_t> >& getSuffixes() const {
	return suffixes_;
    }
    const std::vector<position_t>& getSuffixCounts() const {
	return suffix_counts_;
    }
    // YCHUANG ADDED START
    const std::vector<std::vector<word_t> >& getLeftParentheses() const {
        return left_parentheses_;
    }
    const std::vector<std::vector<word_t> >& getRightParentheses() const {
        return right_parentheses_;
    }
    const std::vector<position_t>& getLeftParenthesesCounts() const {
        return left_parentheses_counts_;
    }
    const std::vector<position_t>& getRightParenthesesCounts() const {
        return right_parentheses_counts_;
    }
    // YCHUANG ADDED END
    const std::vector<position_t>& getNodeCounts() const {
	return node_counts_;
    }
    level_t getSparseStartLevel() const {
	return sparse_start_level_;
    }
    SuffixType getSuffixType() const {
	return suffix_type_;
    }
    level_t getSuffixLen() const {
	return hash_suffix_len_ + real_suffix_len_;
    }
    level_t getHashSuffixLen() const {
	return hash_suffix_len_;
    }
    level_t getRealSuffixLen() const {
	return real_suffix_len_;
    }
    // YCHUANG ADDED START
    level_t getLeftParenthesesLen() const {
        return 1; // bits
    }
    level_t getRightParenthesesLen() const {
        return 1; // bits
    }
    uint64_t getMaxNumLevel() const {
        return max_num_level_;
    }
    // YCHUANG ADDED END

private:
    static bool isSameKey(const std::string& a, const std::string& b) {
	return a.compare(b) == 0;
    }

    // Fill in the LOUDS-Sparse vectors through a single scan
    // of the sorted key list.
    // void buildSparseWithparentheses(const std::vector<std::string>& keys, const std::vector<int>& values, const bool flag_build_until_unique, int max_num_level=-1);
    // void buildSparseWithparentheses(const std::vector<std::string>& keys, const std::vector<bool>& left_parentheses, const std::vector<bool>& right_parentheses, const bool flag_build_until_unique, int max_num_level=-1);
    void buildSparseWithparentheses(const std::vector<std::string>& keys, const std::vector<bool>& left_parentheses, const std::vector<bool>& right_parentheses, level_t max_num_level);

    // Fill in the LOUDS-Sparse vectors through a single scan
    // of the sorted key list.
    void buildSparse(const std::vector<std::string>& keys);

    // Walks down the current partially-filled trie by comparing key to
    // its previous key in the list until their prefixes do not match.
    // The previous key is stored as the last items in the per-level 
    // label vector.
    // For each matching prefix byte(label), it sets the corresponding
    // child indicator bit to 1 for that label.
    level_t skipCommonPrefix(const std::string& key);

    // YCHUANG ADDED START
    // Starting at the start_level of the trie, the function inserts 
    // key bytes to the trie vectors until the specified max_level.
    // This function is called after skipCommonPrefix. Therefore, it
    // guarantees that the stored prefix of key is unique in the trie.
    level_t insertKeyBytesToTrieUntilLevel(const std::string& key, const std::string& next_key, const level_t start_level, const level_t end_level);
    // YCHUANG ADDED END

    // Starting at the start_level of the trie, the function inserts 
    // key bytes to the trie vectors until the first byte/label where 
    // key and next_key do not match.
    // This function is called after skipCommonPrefix. Therefore, it
    // guarantees that the stored prefix of key is unique in the trie.
    level_t insertKeyBytesToTrieUntilUnique(const std::string& key, const std::string& next_key, const level_t start_level);

    // Fills in the suffix byte for key
    inline void insertSuffix(const std::string& key, const level_t level);

    inline bool isCharCommonPrefix(const label_t c, const level_t level) const;
    inline bool isLevelEmpty(const level_t level) const;
    inline void moveToNextItemSlot(const level_t level);
    void insertKeyByte(const char c, const level_t level, const bool is_start_of_node, const bool is_term);
    inline void storeSuffix(const level_t level, const word_t suffix);

    // YCHUANG ADDED START
    inline void insertLeftParenthesis(const bool& left_parenthesis, const level_t level);
    inline void insertRightParenthesis(const bool& right_parenthesis, const level_t level);
    inline void storeLeftParenthesis(const level_t level, const word_t left_parenthesis);
    inline void storeRightParenthesis(const level_t level, const word_t right_parenthesis);
    // YCHUANG ADDED END

    // Compute sparse_start_level_ according to the pre-defined
    // size ratio between Sparse and Dense levels.
    // Dense size < Sparse size / sparse_dense_ratio_
    inline void determineCutoffLevel();

    inline uint64_t computeDenseMem(const level_t downto_level) const;
    inline uint64_t computeSparseMem(const level_t start_level) const;
    
    // Fill in the LOUDS-Dense vectors based on the built
    // Sparse vectors.
    // Called after sparse_start_level_ is set.
    void buildDense();

    // YCHUANG ADDED START
    void buildDenseWithparentheses();
    // YCHUANG ADDED END

    void initDenseVectors(const level_t level);
    void setLabelAndChildIndicatorBitmap(const level_t level, const position_t node_num, const position_t pos);

    position_t getNumItems(const level_t level) const;
    void addLevel();
    bool isStartOfNode(const level_t level, const position_t pos) const;
    bool isTerminator(const level_t level, const position_t pos) const;

private:
    // trie level < sparse_start_level_: LOUDS-Dense
    // trie level >= sparse_start_level_: LOUDS-Sparse
    bool include_dense_;
    uint32_t sparse_dense_ratio_;
    level_t sparse_start_level_;

    // LOUDS-Sparse bit/byte vectors
    std::vector<std::vector<label_t> > labels_;
    std::vector<std::vector<word_t> > child_indicator_bits_;
    std::vector<std::vector<word_t> > louds_bits_;

    // LOUDS-Dense bit vectors
    std::vector<std::vector<word_t> > bitmap_labels_;
    std::vector<std::vector<word_t> > bitmap_child_indicator_bits_;
    std::vector<std::vector<word_t> > prefixkey_indicator_bits_;

    SuffixType suffix_type_;
    level_t hash_suffix_len_;
    level_t real_suffix_len_;
    std::vector<std::vector<word_t> > suffixes_;
    std::vector<position_t> suffix_counts_;
    // YCHUANG ADDED START
    std::vector<std::vector<word_t> > left_parentheses_;
    std::vector<std::vector<word_t> > right_parentheses_;
    std::vector<position_t> left_parentheses_counts_;
    std::vector<position_t> right_parentheses_counts_;
    uint16_t max_num_level_ = 0;
    // YCHUANG ADDED END

    // auxiliary per level bookkeeping vectors
    std::vector<position_t> node_counts_;
    std::vector<bool> is_last_item_terminator_;
};


} // namespace surf

#endif // SURFBUILDER_H_
