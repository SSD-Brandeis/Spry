#ifndef SURF_H_
#define SURF_H_

#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

#include <climits>
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
        
    private:
        SuRF_Env() {}
        static SuRF_Env* surf_env_ptr;

        int surf__key_len_in_bytes = 12;
        uint32_t surf__hash_suffix_len = 0;
        uint32_t surf__real_suffix_len = 0;
        bool surf__include_dense = true;
        uint32_t surf__sparse_dense_ratio = 16;
        bool surf__flag_bypass_if_same_key = true;
        bool surf__flag_allow_range_boundary_overlapped = false;
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

            // for(auto d: digits){
            //     std::cout  << d << " ";
            // }
            // std::cout << std::endl;

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

            return out;
        }


        // static std::string decode_byte_string_to_digit_string(std::string &byte_string);
        static std::string decode_byte_string_to_digit_string(const std::string &byte_string){
            std::string digit_string;
            //int base = 256;
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

            // for(auto d: digits){
            //     std::cout << d << " ";
            // }
            // std::cout << endl;
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
    private:
};

class SuRF_RDF {
    public:
        // SuRF_RDF(int level, std::vector<std::pair<std::string, std::string>> ranges) {
        //     surf_ = SuRF::rangesToSurf(ranges);
        // }
        enum RDF_MODE {PER_LEVEL, PER_FILE};

        SuRF_RDF(RDF_MODE rdf_mode_in); 
        // {
        //     rdf_mode = rdf_mode_in;
        // }

        SuRF_RDF(SuRF_RDF &surf_rdf_in) ;
        // {
        //     RDF_MODE rdf_mode_in = surf_rdf_in.getRDFMode();
        //     // // std::vector<SuRF*> level_surf_rdf_in = surf_rdf.getLevelRDF();
        //     // level_t num_level = surf_rdf_in.getNumberOfTotalLevels();
        //     // if(rdf_mode_in == PER_LEVEL){
        //     //     rdf_mode = PER_LEVEL;
        //     //     auto level_surf_rdf_in = surf_rdf_in.getLevelRDF();
        //     //     for(auto fd_rdf: level_surf_rdf_in){
        //     //         int num_of_ranges = fd_rdf.first;
        //     //         SuRF* surf_orig = fd_rdf.second;
        //     //         char* serizlid_data = surf_orig->serialize();
        //     //         SuRF* surf_new = SuRF::deSerialize(serizlid_data);
        //     //         level_surf_rdf.push_back(std::make_pair(num_of_ranges, surf_new));
        //     //     }
        //     // }else if(rdf_mode_in == PER_FILE){
        //     //     rdf_mode = PER_FILE;
        //     //     auto level_file_surf_rdf_in = surf_rdf_in.getLevelFileRDF();
        //     //     for(auto fd_rdf: level_file_surf_rdf_in){
        //     //         std::unordered_map<uint64_t,std::pair<int, SuRF*>> level_file_surf_rdf_in_at_level;
        //     //         auto it = fd_rdf.begin();
        //     //         auto end = fd_rdf.end();
        //     //         for(; it != end; it++){
        //     //             auto rd_surf = it->second;
        //     //         // for(auto &[fd, rd_surf]: fd_rdf){
        //     //             int num_of_ranges = rd_surf.first;
        //     //             SuRF* surf_orig = rd_surf.second;
        //     //             char* serizlid_data = surf_orig->serialize();
        //     //             SuRF* surf_new = SuRF::deSerialize(serizlid_data);
        //     //             level_file_surf_rdf_in_at_level[fd] = std::make_pair(num_of_ranges, surf_new);
        //     //         }
        //     //         level_file_surf_rdf.push_back(level_file_surf_rdf_in_at_level);
        //     //     }
        //     // }else{
        //     //     assert(false);
        //     // }

        //     // numbers_of_ranges_in_RDF_log = surf_rdf_in.getNumbersOfRangesInRDFLog();
        //     // memory_usage_in_RDF_log = surf_rdf_in.getMemoryUsageInRDFLog();
        // }

        SuRF_RDF(const VMP &level_file_surf_rdf_in,
                const std::vector<int> &numbers_of_ranges_in_RDF_log_in, const std::vector<int> &memory_usage_in_RDF_log_in,
                const RDF_MODE rdf_mode_in = PER_FILE) ;
        // {
        //     assert(rdf_mode_in == PER_FILE);
        //     rdf_mode= rdf_mode_in;
        //     level_file_surf_rdf = level_file_surf_rdf_in;
        //     // level_surf_rdf_numbers_of_ranges = level_surf_rdf_numbers_of_ranges_in;
        //     numbers_of_ranges_in_RDF_log = numbers_of_ranges_in_RDF_log_in;
        //     memory_usage_in_RDF_log = memory_usage_in_RDF_log_in;
        // }
        
        SuRF_RDF(const VP &level_surf_rdf_in,
                const std::vector<int> &numbers_of_ranges_in_RDF_log_in, const std::vector<int> &memory_usage_in_RDF_log_in,
                const RDF_MODE rdf_mode_in = PER_LEVEL) ;
        // {
        //     assert(rdf_mode_in == PER_LEVEL);
        //     rdf_mode = rdf_mode_in;
        //     level_surf_rdf = level_surf_rdf_in;
        //     // level_surf_rdf_numbers_of_ranges = level_surf_rdf_numbers_of_ranges_in;
        //     numbers_of_ranges_in_RDF_log = numbers_of_ranges_in_RDF_log_in;
        //     memory_usage_in_RDF_log = memory_usage_in_RDF_log_in;
        // }
        
        // SuRF_RDF(VMP &level_file_surf_rdf_in, 
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
        ~SuRF_RDF() ;
        // {
        //     int num_level = getNumberOfTotalLevels();
        //     if(rdf_mode == PER_LEVEL){
        //         for(int i = 0; i < num_level; i++){
        //             if(level_surf_rdf[i].second != NULL){
        //                 delete level_surf_rdf[i].second;
        //             }
        //         }
        //     }else if(rdf_mode == PER_FILE){       
        //         for(int i = 0; i < num_level; i++){
        //             auto it = level_file_surf_rdf[i].begin();
        //             while(it != level_file_surf_rdf[i].end()){
        //                 delete ((it->second).second);
        //                 it++;
        //             }
        //         }
        //     }else{
        //         assert(false);
        //     }
        // }

        // SuRF* getSurf() {
        //     return surf_;
        // }
        
        // bool isAliveAfterSuRFTopLevelRDFilter(long long key){
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

        // void insertRangeDeleteToLevel0(uint64_t file_num, std::vector<pss> &range_delete_list_in, std::vector<uint64_t> exist_level0_file_nums);
        // void insertRangeDeleteToLevel0(uint64_t file_num, std::vector<pss> &range_delete_list_in);
        void insertRangeDeleteToLevel0(uint64_t file_num, std::vector<pss> &range_delete_list_in, bool flag_allow_boundary_overlapped);
        // {
        //     if(rdf_mode == PER_LEVEL){
        //         if(level_surf_rdf.size() == 0){
        //             SuRF* surf = SuRF::rangesToSurf(range_delete_list_in);
        //             level_surf_rdf.push_back(std::make_pair(range_delete_list_in.size(), surf));
        //         }else{
        //             SuRF* surf = level_surf_rdf[0].second;
        //             std::vector<pss> range_list = SuRF_RDF::mergeRanges(surf->surfToRanges(surf), range_delete_list_in);
        //             delete level_surf_rdf[0].second;
        //             surf = SuRF::rangesToSurf(range_list);
        //             level_surf_rdf[0].second = surf;
        //             level_surf_rdf[0].first = range_list.size();
        //         }
        //     }else if(rdf_mode == PER_FILE){
        //         if(level_file_surf_rdf.size() == 0){
        //             std::unordered_map<uint64_t,std::pair<int, SuRF*>> level_file_surf_rdf_in; // fd -> (# RD, surf)
        //             SuRF* surf = SuRF::rangesToSurf(range_delete_list_in);
        //             level_file_surf_rdf_in[file_num] = std::make_pair(range_delete_list_in.size(), surf);
        //             level_file_surf_rdf.push_back(level_file_surf_rdf_in);
        //         }else{
        //             std::unordered_map<uint64_t,std::pair<int, SuRF*>> &level_file_surf_rdf_in = level_file_surf_rdf[0];
        //             if(level_file_surf_rdf_in.find(file_num) != level_file_surf_rdf_in.end()){
        //                 std::cout << "Error. Flusing in file shall have unique file_num and greater than previous ones" << std::endl;
        //             }else{
        //                 SuRF* surf = SuRF::rangesToSurf(range_delete_list_in);
        //                 level_file_surf_rdf[0][file_num] = std::make_pair(range_delete_list_in.size(), surf);
        //             }
        //         }
        //     }else{
        //         assert(false);
        //     }
        // }

        void directMoveFileToLevel(uint64_t fd, uint32_t src_level, uint32_t dst_level);

        // std::vector<pss> getRangeTombstonesAtLevelOfFd(uint32_t src_level, uint64_t fd);
        std::vector<pss> getRangeTombstonesAtLevelOfFd(uint32_t src_level, uint64_t fd, bool flag_allow_boundary_overlapped);

        void removeSuRFAtLevelOfFd(uint32_t src_level, uint64_t fd);

        void insertRangesAtLevelOfFd(uint32_t level, uint64_t fd, std::vector<pss> &ranges, bool flag_allow_boundary_overlapped);
        
        void insertRangesWithPointKeysAtLevelOfFd(uint32_t level, uint64_t fd, std::vector<pss> &ranges, std::vector<std::string> &point_keys, bool flag_allow_boundary_overlapped);

        // void shiftRDFToOutputLevel(SuRFCompactionMovingRDInfo *surf__compaction_moving_RD_vector);

        void deleteLastLevelIfEqualsBottomLevel(uint bottom_level);
        // {
        // // init();
        // // std::lock_guard<std::mutex> guard(update_mutex);

        //     while (level_surf_rdf.size()-1 >= bottom_level)
        //     {
        //         level_surf_rdf.pop_back();
        //     }
        // }

        // void shiftRDFToOutputLevel(rocksdb::SuRFCompactionLevelList)

        int getNumberOfRangesAtIthLevel(int level);
        // {
        //     if(rdf_mode == PER_LEVEL){
        //         assert(level < level_surf_rdf.size());
        //         assert(level_surf_rdf[level].second != NULL);
        //         return level_surf_rdf[level].first;
        //     }else if(rdf_mode == PER_FILE){
        //         assert(level < level_file_surf_rdf.size());
        //         int num = 0;
        //         for(auto &[fd, surf]: level_file_surf_rdf[level]){
        //             num += surf.first;
        //         }
        //         return num;
        //     }else{
        //         assert(false);
        //     }
        // }
        int getNumberOfTotalRanges();
        // {
        //     int num = 0;
        //     int level = getNumberOfTotalLevels();

        //     if(rdf_mode == PER_LEVEL){
        //         for(int i = 0; i < level; i++){
        //             num += getNumberOfRangesAtIthLevel(i);
        //         }
        //     }else{
        //         for(int i = 0; i < level; i++){
        //             num += getNumberOfRangesAtIthLevel(i);
        //         }
        //     }
        //     return num;
        // }
        uint64_t getMemoryUsageAtIthLevel(int level);
        // {
        //     if(rdf_mode == PER_LEVEL){
        //         assert(level < level_surf_rdf.size());
        //         assert(level_surf_rdf[level].second != NULL);
        //         return level_surf_rdf[level].second->getMemoryUsage();
        //     }else if(rdf_mode == PER_FILE){
        //         assert(level < level_surf_rdf.size());
        //         assert(level_file_surf_rdf[level].size() != 0);
        //         uint64_t mem = 0;
        //         for(auto &[fd, surf]: level_file_surf_rdf[level]){
        //             mem += (surf.second)->getMemoryUsage();
        //             auto a = fd;
        //             a += 0;
        //         }
        //         return mem;
        //     }
        // }
        uint64_t getNumberOfTotalMemoryUsage();
        // {
        //     uint64_t num = 0;
        //     int level = getNumberOfTotalLevels();
        //     if(rdf_mode == PER_LEVEL){
        //         for(int i = 0; i < level; i++){
        //             num += getMemoryUsageAtIthLevel(i);
        //         }
        //     }else if(rdf_mode == PER_FILE){
        //         for(int i = 0; i < level; i++){
        //             num += getMemoryUsageAtIthLevel(i);
        //         }
        //     }
        //     return num;
        // }
        void logCurrentTotalNumbersOfRanges();
        // {
        //     // std::cout << "Current total number of ranges: " << ranges.size() << std::endl;
        //     int num = getNumberOfTotalRanges();
        //     numbers_of_ranges_in_RDF_log.push_back(num);
        // }
        void logCurrentTotalMemoryUsage();
        // {
        //     // std::cout << "Current total memory usage: " << surf_->getMemoryUsage() << std::endl;
        //     uint64_t num = getNumberOfTotalMemoryUsage();
        //     memory_usage_in_RDF_log.push_back(num);
        // }
        
        int getNumberOfTotalLevels();
        // {        
        //     int num = 0;
        //     if(rdf_mode == PER_LEVEL){
        //         int len = level_surf_rdf.size();
        //         for(int i = 1; i < len; i++){
        //             if(level_surf_rdf[i].second != NULL){
        //                 num = i+1;
        //             }
        //         }
        //     }else if(rdf_mode == PER_FILE){
        //         int len = level_file_surf_rdf.size();
        //         for(int i = 1; i < len; i++){
        //             if(level_file_surf_rdf[i].size() != 0){
        //                 num = i+1;
        //             }
        //         }
        //     }
        //     // int len = level_surf_rdf.size();
        //     return num;
        // }

        
        VP getLevelRDF();
        // {
        //     return level_surf_rdf;
        // }
        std::pair<int, SuRF*> getLevelRDFAtIthLevel(int level);
        // {
        //     assert(level < level_surf_rdf.size());
        //     return level_surf_rdf[level];
        // }

        // std::vector<int> getLevelRDFNumbersOfRanges(){
        //     return level_surf_rdf_numbers_of_ranges;
        // }

        VMP getLevelFileRDF();
        // {
        //     return level_file_surf_rdf;
        // }
        std::unordered_map<uint64_t,std::pair<int, SuRF*>> getLevelFileRDFAtIthLevel(int level);
        // {
        //     assert(level < level_file_surf_rdf.size());
        //     return level_file_surf_rdf[level];
        // }


        std::vector<pss>  gatherSortedRangeTombstonesAndRemoveSuRF(std::vector<uint32_t> &src_level_list, 
                                                                    std::vector<std::vector<uint64_t>> &src_fd_list2d, 
                                                                    bool surf_flag__allow_range_boundary_overlapped);

        // void shiftRDFToOutputLevel(std::vector<ROCKSDB_NAMESPACE::SuRFCompactionSourceLevelInfo> &src_level_info_list, 
        //                             uint32_t dst_level, 
        //                             std::vector<ROCKSDB_NAMESPACE::SuRFCompactionDestinationLevelInfo> &dst_level_info_list, 
        //                             bool surf_flag__allow_range_boundary_overlapped);
        void shiftRDFToOutputLevel(std::vector<pss> &rd_merged, uint32_t dst_level, 
                                    std::vector<uint64_t> &dst_fd_list, std::vector<pss> &file_boundary_list, 
                                    bool surf_flag__allow_range_boundary_overlapped);

        void shiftRDFWithPointKeysToOutputLevel(std::vector<pss> &rd_merged, std::vector<std::string> &point_keys, 
                                                            uint32_t dst_level, std::vector<uint64_t> &dst_fd_list, std::vector<pss> &file_boundary_list, 
                                                            bool surf_flag__allow_range_boundary_overlapped);

        std::vector<int> getNumbersOfRangesInRDFLog();
        // {
        //     return numbers_of_ranges_in_RDF_log;
        // }
        std::vector<int> getMemoryUsageInRDFLog();
        // {
        //     return memory_usage_in_RDF_log;
        // }

        RDF_MODE getRDFMode();
        // {
        //     return rdf_mode;
        // }

        static vpss mergeRanges(vpss ranges_1, vpss ranges_2, bool allow_boundary_overlap_not_merged = false);
        
        bool isEntryAliveAtLevelOfFd(level_t level, uint64_t fd, std::string key, bool flag_bypass_if_same_key) const ;

        bool isEntryAlive(level_t level, std::string key, bool flag_bypass_if_same_key) const ;
        // {
        //     assert(level < level_surf_rdf.size());
        //     assert(rdf_mode == PER_LEVEL);
        //     SuRF* surf = level_surf_rdf[level].second;
        //     SuRF::Iter iter = surf->moveToNextCommonPrefixKey(key);
        //     auto key_len_in_bytes = key.size();

        //     bool overlapping = false;
        //     if(iter.isValid()){
        //         std::string key_found = iter.getKey();
        //         if(key_found.size() < key_len_in_bytes){
        //             overlapping = (iter.getRightParenthesis() == true);
        //         }else{
        //             overlapping = (key_found != key) && (iter.getRightParenthesis() == true);
        //         }
        //     }
        //     return overlapping;
        // }

        bool isEntryAlive(level_t level, std::string key, uint64_t fd, bool flag_bypass_if_same_key) const ;
        // {
        //     assert(level < level_file_surf_rdf.size());
        //     assert(rdf_mode == PER_FILE);

        //     auto it = level_file_surf_rdf[level].find(fd);
        //     if(it == level_file_surf_rdf[level].end()){
        //         assert(false); 
        //     }


        //     bool overlapping = false;
        //     if(it != level_file_surf_rdf[level].end()){
        //         SuRF* surf = it->second.second;
        //         SuRF::Iter iter = surf->moveToNextCommonPrefixKey(key);
        //         auto key_len_in_bytes = key.size();

        //         if(iter.isValid()){
        //             std::string key_found = iter.getKey();
        //             if(key_found.size() < key_len_in_bytes){
        //                 overlapping = (iter.getRightParenthesis() == true);
        //             }else{
        //                 overlapping = (key_found != key) && (iter.getRightParenthesis() == true);
        //             }
        //         }
        //     }
        //     return overlapping;
        // }

        // void print();
        void print(bool flag_allow_boundary_overlapped);

    private:
        // SuRF* surf_ = nullptr;
        RDF_MODE rdf_mode = PER_LEVEL;
        VP level_surf_rdf; // (#ranges, SuRF*) per level
        VMP level_file_surf_rdf; // fd->(#ranges, SuRF*) per level
        // std::vector<std::unordered_map<uint64_t,int>> level_surf_rdf_numbers_of_ranges; // fd->SuRF*
        std::vector<int> numbers_of_ranges_in_RDF_log; //for level > 0, number of ranges in RDF
        std::vector<int> memory_usage_in_RDF_log; //for level > 0, number of ranges in RDF
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
    // SuRF(const std::vector<std::string>& keys, std::vector<bool> &left_parentheses, std::vector<bool> &right_parentheses,
    //     const bool include_dense, const uint32_t sparse_dense_ratio,
    //     const SuffixType suffix_type, const level_t hash_suffix_len, const level_t real_suffix_len,
    //     const bool flag_build_until_unique, const uint16_t max_num_level = -1) {
    //     create(keys, left_parentheses, right_parentheses, 
    //             include_dense, sparse_dense_ratio, 
    //             suffix_type, hash_suffix_len, real_suffix_len,
    //             flag_build_until_unique, max_num_level);
    // }
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
    // void create(const std::vector<std::string>& keys, std::vector<bool> &left_parentheses, std::vector<bool> &right_parentheses,
	// 	  const bool include_dense, const uint32_t sparse_dense_ratio,
	// 	  const SuffixType suffix_type,
    //       const level_t hash_suffix_len, const level_t real_suffix_len,
    //       const bool flag_build_until_unique, const uint16_t max_num_level = -1);
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
     SuRF::Iter moveToNextCommonPrefixKey(const std::string& key) const;
    // YCHUANG ADDED END
    // SuRF::Iter moveToKeyLessThan(const std::string& key, const bool inclusive) const;
    SuRF::Iter moveToFirst() const;
    SuRF::Iter moveToLast() const;
    bool lookupRange(const std::string& left_key, const bool left_inclusive, 
		     const std::string& right_key, const bool right_inclusive);
    // Accurate except at the boundaries --> undercount by at most 2
    uint64_t approxCount(const std::string& left_key, const std::string& right_key);
    uint64_t approxCount(const SuRF::Iter* iter, const SuRF::Iter* iter2);

    uint64_t serializedSize() const;
    uint64_t getMemoryUsage() const;
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
    // static SuRF* rangesToSurf(std::vector<std::pair<std::string, std::string>> ranges, size_t surf_key_length_in_bytes = 8, 
    //                 surf::SuffixType kSuffixType = surf::SuffixType::kReal, 
    //                 surf::level_t hash_suffix_len = 0, surf::level_t real_suffix_len = 8,
    //                 bool include_dense = true, uint32_t sparse_dense_ratio = 16, 
    //                 bool flag_build_until_unique = false);
    // static SuRF* rangesToSurf(std::vector<std::pair<std::string, std::string>> ranges, size_t surf_key_length_in_bytes = 8, 
    //                 surf::SuffixType kSuffixType = surf::SuffixType::kReal, 
    //                 surf::level_t hash_suffix_len = 0, surf::level_t real_suffix_len = 8,
    //                 bool include_dense = true, uint32_t sparse_dense_ratio = 16, bool flag_allow_boundary_overlapped = false
    //                 );
    static SuRF* rangesToSurf(std::vector<pss> ranges, size_t surf_key_length_in_bytes, 
                    surf::SuffixType kSuffixType, 
                    surf::level_t hash_suffix_len, surf::level_t real_suffix_len,
                    bool include_dense, uint32_t sparse_dense_ratio, bool flag_allow_boundary_overlapped
                    );
    
    static std::pair<SuRF*, size_t> rangesWithPointKeysToSurf(std::vector<pss> ranges, std::vector<std::string> point_keys, 
                size_t surf_key_length_in_bytes, surf::SuffixType kSuffixType, 
                surf::level_t hash_suffix_len, surf::level_t real_suffix_len,
                bool include_dense, uint32_t sparse_dense_ratio, bool flag_allow_boundary_overlapped);


    // static std::vector<std::pair<std::string, std::string>> surfToRanges(SuRF* surf_);
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
