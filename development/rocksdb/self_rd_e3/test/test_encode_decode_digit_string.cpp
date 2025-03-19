 //gen random number
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <chrono>
#include <regex>
#include <thread>
#include <cstdlib>
#include <climits>
#include <cmath>
#include <cassert>

using namespace std;

std::string encode_digit_string_to_byte_string(const std::string &digit_string){

    uint32_t base = 256;
    int32_t spacing =  log(ULONG_LONG_MAX) / log(10); //19
    spacing -= (log(base)/log(10)+1); //16
    // std::cout << spacing << std::endl;
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


std::string decode_byte_string_to_digit_string(const std::string &byte_string){
    std::string digit_string;
    // int base = 256;
    int32_t spacing = 7;
    std::vector<uint64_t> digits;
    for(int32_t i = byte_string.size(); i > 0; i-=spacing){
        int32_t start = std::max(i-spacing, 0);
        int32_t end = i;
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


void print_byte_string(std::string byte_string){
    for(auto c: byte_string){
        std::cout << (unsigned)(unsigned char)(c) << " ";
    }
    std::cout << std::endl;
}

void print_digit_string(std::string digit_string){
    std::cout << digit_string << std::endl;
}



int main(){
    
    int KEY_SIZE = 12;
    std::vector<std::string> v_str;
    for(int i = 0; i < 256; i++){
        string key = "abc";
        key+= (char)i;
        stringstream ss_key;
        // ss_key << std::setfill((char)-128) << std::setw(KEY_SIZE) << key;
        // std::cout << ss_key.str() << std::endl;
        v_str.push_back(ss_key.str());
        // std::cout << ssmake_key.str() << std::endl;
    }

    std::string str1, str2;

    for(int i = 1; i < 256; i++){
        str1.push_back(i-1);
        str2.push_back(i);
        if(str1 > str2){
            std::cout << "Error @i=" << i << std::endl;
        }
    }
// long long num = 0;
// for(auto s: v_str){
//     for(auto c: s){
//         std::cout << ""
//     }
// }
    std::vector<std::string> v_str2(v_str);
    std::sort(v_str2.begin(), v_str2.end());
    for(int i = 0; i < 256; i++){
        if(v_str[i] == v_str2[i]){continue;}
        std::cout << "v_str[i] != v_str2[i]" << " @i = " << i << std::endl;
    }
    std::cout << std::endl;

    std::string digit_string(29,'9');
    // for(int c = 128; c < 256; c++){
    //     int a = (char) c;
    //     // std::cout  << a << " ";
    //     std::cout << (char) c << " ";
    // }
    // std::cout << std::endl;

    

    string byte_string = encode_digit_string_to_byte_string(digit_string);
    // print_byte_string(byte_string);
    string digit_string2 = decode_byte_string_to_digit_string(byte_string);
    // print_digit_string(digit_string);
    // print_digit_string(digit_string2);

    // std::cout << (digit_string == digit_string2) << std::endl;


    int N_TEST = 1000;
    int LEN_STRING_BASE = 30;
    for(int i = 0; i < N_TEST; i++){
        std::string digit_string;
// std::cout << "A0" << std::endl;
        int LEN_STRING =  LEN_STRING_BASE + rand() % 15;
        for(int j = 0; j < LEN_STRING; j++){
            int r = rand() % 10;
            while(j == 0 && r == 0){r = rand() % 10;}
            digit_string.push_back(r + '0');
        }
// std::cout << "A1" << std::endl;
        std::string byte_string = encode_digit_string_to_byte_string(digit_string);
// std::cout << "A2" << std::endl;
        std::string digit_string2 = decode_byte_string_to_digit_string(byte_string);
// std::cout << "A3" << std::endl;

        // std::cout << digit_string << " " << digit_string2 << std::endl;
        // std::cout << (digit_string.size() - byte_string.size())*1.0/digit_string.size() << std::endl;

        if(digit_string2 != digit_string){
            assert(false);
        }
    }   
    std::cout << "PASS" << std::endl; 
}