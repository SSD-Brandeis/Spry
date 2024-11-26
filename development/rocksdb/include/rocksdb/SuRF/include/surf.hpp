#ifndef SURF_H_
#define SURF_H_

#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <sstream>

#include <climits>
#include <limits>
#include <cmath>
#include <iomanip>

#include "config.hpp"
#include "louds_dense.hpp"
#include "louds_sparse.hpp"
#include "surf_builder.hpp"

namespace surf {

// YCHUANG ADDED START
class SuRF;
class SuRF_RDF;

using VP = std::vector<std::pair<int,SuRF*>>;
using VMP = std::vector<std::unordered_map<uint64_t,std::pair<int, SuRF*>>>;
using pss = std::pair<std::string, std::string>;
using vpss = std::vector<std::pair<std::string, std::string>>;

class SuRF_Env {
    public:
        static SuRF_Env* getInstance();

        void setSuRFKeyLenInBytes(int key_len_in_bytes) {
            surf__key_len_in_bytes = key_len_in_bytes;
            return;
        }
        void setSuRFHashSuffixLen(uint32_t hash_suffix_len) {
            surf__hash_suffix_len = hash_suffix_len;
            return;
        }
        void setSuRFRealSuffixLen(uint32_t real_suffix_len) {
            surf__real_suffix_len = real_suffix_len;
            return;
        }
        void setSuRFIncludeDense(bool include_dense) {
            surf__include_dense = include_dense;
            return;
        }
        void setSuRFSparseDenseRatio(uint32_t sparse_dense_ratio) {
            surf__sparse_dense_ratio = sparse_dense_ratio;
            return;
        }
        void setFlagBypassIfSameKey(bool flag_bypass_if_same_key) {
            surf__flag_bypass_if_same_key = flag_bypass_if_same_key;
            return;
        }
        void setFlagAllowRangeBoundaryOverlapped(bool flag_allow_range_boundary_overlapped){
            surf__flag_allow_range_boundary_overlapped = flag_allow_range_boundary_overlapped;
            return;
        }


        int getSuRFKeyLenInBytes() const {
            return surf__key_len_in_bytes;
        }
        uint32_t getSuRFHashSuffixLen() const {
            return surf__hash_suffix_len;
        }
        uint32_t getSuRFRealSuffixLen() const {
            return surf__real_suffix_len;
        }
        bool getSuRFIncludeDense() const {
            return surf__include_dense;
        }
        uint32_t getSuRFSparseDenseRatio() const {
            return surf__sparse_dense_ratio;
        }
        bool getFlagBypassIfSameKey() const {
            if(surf__flag_bypass_if_same_key != surf__flag_allow_range_boundary_overlapped){
                assert(surf__flag_bypass_if_same_key == surf__flag_allow_range_boundary_overlapped);
                std::cerr << "Error: flag_bypass_if_same_key and flag_allow_range_boundary_overlapped are not consistent" << " " << __FILE__ << " " << __LINE__ << std::endl;
            }
            return surf__flag_bypass_if_same_key;
        }
        bool getFlagAllowRangeBoundaryOverlapped() const {
            if(surf__flag_bypass_if_same_key != surf__flag_allow_range_boundary_overlapped){
                assert(surf__flag_bypass_if_same_key == surf__flag_allow_range_boundary_overlapped);
                std::cerr << "Error: flag_bypass_if_same_key and flag_allow_range_boundary_overlapped are not consistent" << " " << __FILE__ << " " << __LINE__ << std::endl;
            }
            return surf__flag_allow_range_boundary_overlapped;
        }
        
        // Start: used during Get()
        void setFlagKeyMayDeleted(bool flag){
            flag_key_may_deleted = flag;
        }
        bool getFlagKeyMayDeleted(){
            return flag_key_may_deleted;
        }
        void clearFlagKeyMayDeleted(){
            flag_key_may_deleted = false;
        }
        // End: used during Get()

        void setFlagSurfUseCondensedDigitKey(bool flag){
            surf__flag_use_condensed_digit_key = flag;
        }
        bool getFlagSurfUseCondensedDigitKey(){
            return surf__flag_use_condensed_digit_key;
        }

        void setLengthOfCondensedDigitKey(uint32_t len){
            std::cout << "set SuRF length of condensed digit keys to " << len 
            << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
            surf__length_of_condensed_digit_key = len;
        }
        uint32_t getLengthOfCondensedDigitKey(){
            return surf__length_of_condensed_digit_key;
        }

        void setShowSurfCompactionInfo(bool flag){
            show_surf_compaction_info = flag;
        }

        bool getShowSurfCompactionInfo(){
            return show_surf_compaction_info;
        }
        

        void setFlagUseSuRFBase(bool flag){
            use_surf_base = flag;
        }
        bool getFlagUseSuRFBase(){
            return use_surf_base;
        }

        void setSuRFBaseStoreKeyToKDiff(uint32_t k_diff){
            assert(k_diff >= 1);
            surf_base_store_key_to_k_diff = k_diff;
        }
        uint32_t getSuRFBaseStoreKeyToKDiff(){
            return surf_base_store_key_to_k_diff;
        }

    private:
        SuRF_Env() {}
        static SuRF_Env* surf_env_ptr;
        bool flag_key_may_deleted = false; // used during get()

        int surf__key_len_in_bytes = 12;
        uint32_t surf__hash_suffix_len = 0;
        uint32_t surf__real_suffix_len = 0;
        bool surf__include_dense = true;
        uint32_t surf__sparse_dense_ratio = 16;
        bool surf__flag_bypass_if_same_key = true;
        bool surf__flag_allow_range_boundary_overlapped = false;
        bool surf__flag_use_condensed_digit_key = true;
        uint32_t surf__length_of_condensed_digit_key = 5;

        bool show_surf_compaction_info = false;

        bool use_surf_base = false;
        uint32_t surf_base_store_key_to_k_diff = 1;
};


class SuRF_Utils {
    public:
        // static std::string encode_digit_string_to_byte_string(std::string digit_string);
        static std::string encode_digit_string_to_byte_string(const std::string &digit_string){

            uint32_t base = 256;
            int32_t spacing =  std::log(ULONG_LONG_MAX) / std::log(10); //19
            spacing -= (std::log(base)/std::log(10)+1); //16
            //std::cout << spacing << std::endl;
            std::vector<unsigned long long> digits; 
            // uint64_t num2 = 0;
            for(int32_t i = digit_string.size(); i > 0; i-=spacing){
                int32_t start = std::max((int32_t)0, (int32_t)i-spacing);
                std::string sub = digit_string.substr(start, i-start);
                uint64_t a = std::stoull(sub);
                digits.push_back(a);
            }
            reverse(digits.begin(), digits.end());

            std::string out;
            for(uint32_t i = 0; i < digits.size(); ){
                
                uint64_t carry_on = 0, tmp = 0;
                for(uint32_t j = i; j < digits.size(); j++){
                    tmp *= pow(10, spacing);
                    tmp += digits[j];
                    
                    carry_on = tmp % base;
                    digits[j] = tmp / base;

                    tmp = carry_on;
                }

                out.push_back( (char) (((int32_t)carry_on)));

                while(i < digits.size() && digits[i] == 0){
                    i++;
                }
            }

            reverse(out.begin(), out.end());
// std::cout << "input digit_string = " << digit_string << ", encoded_str = " << out << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;

            return out;
        }


        static std::string decode_byte_string_to_digit_string(const std::string &byte_string){
            std::string digit_string;
            int32_t spacing = 7;
            std::vector<uint64_t> digits;
            for(int32_t i = byte_string.size(); i > 0; i-=spacing){
                uint32_t start = std::max(i-spacing, 0);
                uint32_t end = i;
                unsigned long long tmp = 0;
                for(uint32_t j = start; j < end; j++){
                    tmp <<= 8;
                    tmp += (unsigned)(unsigned char)byte_string[j];
                } 
                digits.push_back(tmp);
            }
            reverse(digits.begin(), digits.end());

            std::string out;
            for(uint64_t i = 0; i < digits.size(); ){
                uint64_t carry_on = 0, num = 0;
                for(uint64_t j = i; j < digits.size(); j++){
                    // num <<= (8 * spacing);
                    num *= std::pow(256, spacing);
                    num += digits[j];

                    carry_on = num % 10;
                    digits[j] = num / 10;
                    
                    num = carry_on;
                }
                out.push_back(carry_on + '0');

                while(i < digits.size() && digits[i] == 0){
                    i++;
                }
            }

            reverse(out.begin(), out.end());

            return out;
        }

        static std::string extend_string_to_length(std::string str, uint32_t len, char c = (char)0){
            std::stringstream ss_key;
            ss_key << std::setfill(c) << std::setw(len) << str;
            return ss_key.str();
        }


        static void print_byte_string(std::string byte_string){
            for(auto c: byte_string){
                std::cout << (unsigned)(unsigned char)(c) << " ";
            }
            std::cout << std::endl;
        }

        static void print_digit_string(std::string digit_string){
            std::cout << digit_string << std::endl;
        }

        static std::string stringToHexString(const std::string& input_string) {
            std::stringstream hex_stream;
            hex_stream << std::hex << std::setfill('0');
            
            // Iterate through each character in the string
            for (size_t i = 0; i < input_string.size(); ++i) {
                // Convert each character to its hex representation
                hex_stream << std::setw(2) << static_cast<int>(input_string[i]);
            }
            
            // Convert stringstream to string and return
            return hex_stream.str();
        }        
                
        // Function to truncate based on the first k_diff differing characters between two strings
        static std::string truncateToFirstKDifference(const std::string& str1, const std::string& str2, int k_diff = 1) {
            size_t minLen = std::min(str1.length(), str2.length());
            size_t i = 0;
            int diff_count = 0;
            
            assert(k_diff > 0);

            // Iterate over both strings and count differences until k_diff differences are found
            while (i < minLen) {
                if (str1[i] != str2[i]) {
                    diff_count++;
                }
                if (diff_count == k_diff) {
                    break;
                }
                ++i;
            }
            
            // Return the substring from str1 up to i+1
            return str1.substr(0, i + 1);
        }

        // Process the list of words to truncate based on k_diff differences from previous word
        static std::vector<std::string> processStringsToFirstKDifference(std::vector<std::string>& vec, int k_diff = 1) {
            // Sort the vector of strings
            std::sort(vec.begin(), vec.end());
            
            // std::cout << "@ processStringsToFirstKDifference " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
            // for(uint32_t i = 0; i < vec.size(); i++){
            //     if(surf::SuRF_Env::getInstance()->getFlagSurfUseCondensedDigitKey() == true){
            //         auto ks = surf::SuRF_Utils::decode_byte_string_to_digit_string(vec[i]);
            //         std::cout << "key =  " << ks << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
            //     }else{
            //         std::cout << "key =  " << vec[i] << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << std::endl;
            //     }
            // }

            if (vec.size() <= 1) {
                // return vec;  // Return the vector as is if it contains 1 or no elements
                vec[0] = truncateToFirstKDifference("", vec[0], k_diff);  // Return the vector as is if it contains 1 or no elements
                return vec;
            }

            std::vector<std::string> result;
            // Store the first string as is (no previous string to compare with)
            // result.push_back(vec[0]);
            result.push_back(truncateToFirstKDifference(vec[1], vec[0], k_diff));

            // Compare each string with the previous one and store the truncated version
            for (size_t i = 1; i < vec.size(); ++i) {
                result.push_back(truncateToFirstKDifference(vec[i], vec[i - 1], k_diff));
            }

            return result;
        }

    private:
};

class SuRF_RDF {
    public:
        enum RDF_MODE {PER_LEVEL, PER_FILE};

        SuRF_RDF(RDF_MODE rdf_mode_in); 

        SuRF_RDF(const SuRF_RDF &surf_rdf_in);
        SuRF_RDF(const VMP &level_file_surf_rdf_in,
                const std::vector<int> &numbers_of_ranges_in_RDF_log_in, const std::vector<int> &memory_usage_in_RDF_log_in,
                const RDF_MODE rdf_mode_in = PER_FILE);
        
        SuRF_RDF(const VP &level_surf_rdf_in,
                const std::vector<int> &numbers_of_ranges_in_RDF_log_in, const std::vector<int> &memory_usage_in_RDF_log_in,
                const RDF_MODE rdf_mode_in = PER_LEVEL);
        ~SuRF_RDF();
        void insertRangeDeleteToLevel0(uint64_t file_num, std::vector<pss> &range_delete_list_in, bool flag_allow_boundary_overlapped);
        
        void directMoveFileToLevel(uint64_t fd, uint32_t src_level, uint32_t dst_level);

        std::vector<pss> getRangeTombstonesAtLevelOfFd(uint32_t src_level, uint64_t fd, bool flag_allow_boundary_overlapped);

        void removeSuRFAtLevelOfFd(uint32_t src_level, uint64_t fd);

        void insertRangesAtLevelOfFd(uint32_t level, uint64_t fd, std::vector<pss> &ranges, bool flag_allow_boundary_overlapped);
        
        void insertRangesWithPointKeysAtLevelOfFd(uint32_t level, uint64_t fd, std::vector<pss> &ranges, std::vector<std::string> &point_keys, bool flag_allow_boundary_overlapped);

        void deleteLastLevelIfEqualsBottomLevel(uint bottom_level);

        int getNumberOfRangesAtIthLevel(int level);

        int getNumberOfTotalRanges();

        uint64_t getMemoryUsageAtIthLevel(int level);

        uint64_t getDensePartMemoryUsageInBitsSelf();

        uint64_t getSparsePartMemoryUsageInBitsSelf();

        uint64_t getNumberOfTotalMemoryUsage();

        void logCurrentTotalNumbersOfRanges();

        void logCurrentTotalMemoryUsage();
        
        int getNumberOfTotalLevels();

        
        VP getLevelRDF();

        std::pair<int, SuRF*> getLevelRDFAtIthLevel(int level);

        VMP getLevelFileRDF();

        std::unordered_map<uint64_t,std::pair<int, SuRF*>> getLevelFileRDFAtIthLevel(int level);

        void RemoveSuRF(std::vector<uint32_t> &src_level_list, 
                        std::vector<std::vector<uint64_t>> &src_fd_list2d);

        std::vector<pss>  gatherSortedRangeTombstonesAndRemoveSuRF(std::vector<uint32_t> &src_level_list, 
                                                                    std::vector<std::vector<uint64_t>> &src_fd_list2d, 
                                                                    bool surf_flag__allow_range_boundary_overlapped);

        void shiftRDFToOutputLevel(std::vector<pss> &rd_merged, uint32_t dst_level, 
                                    std::vector<uint64_t> &dst_fd_list, std::vector<pss> &file_boundary_list, 
                                    bool surf_flag__allow_range_boundary_overlapped);

        void shiftRDFWithPointKeysToOutputLevel(std::vector<pss> &rd_merged, std::vector<std::string> &point_keys, 
                                                            uint32_t dst_level, std::vector<uint64_t> &dst_fd_list, std::vector<pss> &file_boundary_list, 
                                                            bool surf_flag__allow_range_boundary_overlapped);

        std::vector<int> getNumbersOfRangesInRDFLog();

        std::vector<int> getMemoryUsageInRDFLog();

        RDF_MODE getRDFMode();

        static vpss mergeRanges(vpss ranges_1, vpss ranges_2, bool allow_boundary_overlap_not_merged = false);
        
        void clearFilterFalsePositiveRate();
        double getFilterFalsePositiveRate();

        void setFlagKeyMayDeleted();
        bool getFlagKeyMayDeleted();
        void clearFlagKeyMayDeleted();
        void incKeySearchCountKMD();
        void checkProperUsageOfFlagKeyMayDeleted();
        bool isEntryAliveAtLevelOfFd(level_t level, uint64_t fd, std::string key, bool flag_bypass_if_same_key);

        // bool isEntryAlive(level_t level, std::string key, bool flag_bypass_if_same_key);

        // bool isEntryAlive(level_t level, std::string key, uint64_t fd, bool flag_bypass_if_same_key);

        void print(bool flag_allow_boundary_overlapped);

    private:
        RDF_MODE rdf_mode = PER_LEVEL;
        VP level_surf_rdf; // (#ranges, SuRF*) per level
        VMP level_file_surf_rdf; // fd->(#ranges, SuRF*) per level
        std::vector<int> numbers_of_ranges_in_RDF_log; //for level > 0, number of ranges in RDF
        std::vector<int> memory_usage_in_RDF_log; //for level > 0, number of ranges in RDF
        bool flag_key_may_deleted = false;
        uint32_t key_search_count_kmd = 0; // kmd = key may deleted, just for functionality verification
        uint32_t key_search_count = 0;
        uint32_t key_may_deleted_count = 0;
        
        // use clearFilterFalsePositiveRate for public called
        void clearKeySearchCount();
        void incKeySearchCount();
        uint32_t getKeySearchCount();
        // use clearFilterFalsePositiveRate for public called
        void clearKeyMayDeletedCount();
        uint32_t getKeyMayDeletedCount();
};


// YCHUANG ADDED END
class SuRF {
public:
    class Iter {
    public:
	Iter() {};
	Iter(const SuRF* filter) {
	    dense_iter_ = LoudsDense::Iter(filter->louds_dense_);
	    sparse_iter_ = LoudsSparse::Iter(filter->louds_sparse_);
	    could_be_fp_ = false;
	}

	void clear();
	bool isValid() const;
	bool getFpFlag() const;
	int compare(const std::string& key) const;
	std::string getKey() const;
	int getSuffix(word_t* suffix) const;
    // YCHUANG ADDED START
    bool getLeftParenthesis() const;
    bool getRightParenthesis() const;
    LoudsSparse::Iter *getSparseIter() {
        return &sparse_iter_;
    }
    // YCHUANG ADDED END
	std::string getKeyWithSuffix(unsigned* bitlen) const;

	// Returns true if the status of the iterator after the operation is valid
	bool operator ++(int);
	bool operator --(int);

    private:
	void passToSparse();
	bool incrementDenseIter();
	bool incrementSparseIter();
	bool decrementDenseIter();
	bool decrementSparseIter();

    private:
	// true implies that dense_iter_ is valid
	LoudsDense::Iter dense_iter_;
	LoudsSparse::Iter sparse_iter_;
	bool could_be_fp_;

	friend class SuRF;
    };

public:
    SuRF() {};

    //------------------------------------------------------------------
    // Input keys must be SORTED
    //------------------------------------------------------------------
    SuRF(const std::vector<std::string>& keys) {
	create(keys, kIncludeDense, kSparseDenseRatio, kNone, 0, 0);
    }

    SuRF(const std::vector<std::string>& keys, const SuffixType suffix_type,
	 const level_t hash_suffix_len, const level_t real_suffix_len) {
	create(keys, kIncludeDense, kSparseDenseRatio, suffix_type, hash_suffix_len, real_suffix_len);
    }

    SuRF(const std::vector<std::string>& keys, const SuffixType suffix_type,
	 const level_t hash_suffix_len, const level_t real_suffix_len, uint32_t sparseDenseRatio) {
	create(keys, kIncludeDense, sparseDenseRatio, suffix_type, hash_suffix_len, real_suffix_len);
    }
    
    SuRF(const std::vector<std::string>& keys,
	 const bool include_dense, const uint32_t sparse_dense_ratio,
	 const SuffixType suffix_type, const level_t hash_suffix_len, const level_t real_suffix_len) {
	create(keys, include_dense, sparse_dense_ratio, suffix_type, hash_suffix_len, real_suffix_len);
    }

    // YCHUANG ADDED START
    SuRF(const std::vector<std::string>& keys, std::vector<bool> &left_parentheses, std::vector<bool> &right_parentheses,
        const bool include_dense, const uint32_t sparse_dense_ratio,
        const SuffixType suffix_type, const level_t hash_suffix_len, const level_t real_suffix_len,
        const uint16_t max_num_level = -1) {
        create(keys, left_parentheses, right_parentheses, 
                include_dense, sparse_dense_ratio, 
                suffix_type, hash_suffix_len, real_suffix_len,
                max_num_level);
    }
    // YCHUANG ADDED END


    ~SuRF() {}


    // YCHUANG ADDED START
    void create(const std::vector<std::string>& keys, std::vector<bool> &left_parentheses, std::vector<bool> &right_parentheses,
		  const bool include_dense, const uint32_t sparse_dense_ratio,
		  const SuffixType suffix_type,
          const level_t hash_suffix_len, const level_t real_suffix_len,
          const uint16_t max_num_level = -1);
    // YCHUANG ADDED END

    void create(const std::vector<std::string>& keys,
		const bool include_dense, const uint32_t sparse_dense_ratio,
		const SuffixType suffix_type,
                const level_t hash_suffix_len, const level_t real_suffix_len);

    bool lookupKey(const std::string& key) const;
    // This function searches in a conservative way: if inclusive is true
    // and the stored key prefix matches key, iter stays at this key prefix.
    SuRF::Iter moveToKeyGreaterThan(const std::string& key, const bool inclusive) const;
    // YCHUANG ADDED START
    SuRF::Iter moveToNextCommonPrefixKey(const std::string& key,
                bool flag_direct_return_if_found_key_end_with_same_prefix=false) const;
    // YCHUANG ADDED END
    SuRF::Iter moveToFirst() const;
    SuRF::Iter moveToLast() const;
    bool lookupRange(const std::string& left_key, const bool left_inclusive, 
		     const std::string& right_key, const bool right_inclusive);
    // Accurate except at the boundaries --> undercount by at most 2
    uint64_t approxCount(const std::string& left_key, const std::string& right_key);
    uint64_t approxCount(const SuRF::Iter* iter, const SuRF::Iter* iter2);

    uint64_t serializedSize() const;
    uint64_t getMemoryUsage() const;
    uint64_t getDensePartMemoryUsageInBitsSelf() const;
    uint64_t getSparsePartMemoryUsageInBitsSelf() const;
    uint64_t getMemoryUsageInBitsSelf() const;
    level_t getHeight() const;
    level_t getSparseStartLevel() const;

    char* serialize() const {
	uint64_t size = serializedSize();
	char* data = new char[size];
	char* cur_data = data;
	louds_dense_->serialize(cur_data);
	louds_sparse_->serialize(cur_data);
	assert(cur_data - data == (int64_t)size);
	return data;
    }

    static SuRF* deSerialize(char* src) {
	SuRF* surf = new SuRF();
	surf->louds_dense_ = LoudsDense::deSerialize(src);
	surf->louds_sparse_ = LoudsSparse::deSerialize(src);
	surf->iter_ = SuRF::Iter(surf);
	return surf;
    }

    void destroy() {
	louds_dense_->destroy();
	louds_sparse_->destroy();
    }

    // YCHUANG ADDED START
    static SuRF* rangesToSurf(std::vector<pss> ranges, size_t surf_key_length_in_bytes, 
                    surf::SuffixType kSuffixType, 
                    surf::level_t hash_suffix_len, surf::level_t real_suffix_len,
                    bool include_dense, uint32_t sparse_dense_ratio, bool flag_allow_boundary_overlapped
                    );
    
    static std::pair<SuRF*, size_t> rangesWithPointKeysToSurf(std::vector<pss> ranges, std::vector<std::string> point_keys, 
                size_t surf_key_length_in_bytes, surf::SuffixType kSuffixType, 
                surf::level_t hash_suffix_len, surf::level_t real_suffix_len,
                bool include_dense, uint32_t sparse_dense_ratio, bool flag_allow_boundary_overlapped);

    static std::vector<std::pair<std::string, std::string>> surfToRanges(SuRF* surf_, bool flag_allow_boundary_overlapped);

    int getLoudsDenseHeight() const {
        return louds_dense_->getHeight();
    }
    LoudsSparse* getLoudsSparse() const {
        return louds_sparse_;
    }
    // YCHUANG ADDED END

private:
    LoudsDense* louds_dense_;
    LoudsSparse* louds_sparse_;
    SuRFBuilder* builder_;
    SuRF::Iter iter_;
    SuRF::Iter iter2_;
};





// YCHUANG ADDED END


} // namespace surf


#endif // SURF_H
