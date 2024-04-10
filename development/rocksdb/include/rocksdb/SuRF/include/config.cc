#ifndef CONFIG_CC_
#define CONFIG_CC_

#include <stdint.h>
#include <string.h>
#include <string>

#include "config.hpp"

namespace surf {


void align(char*& ptr) {
    ptr = (char*)(((uint64_t)ptr + 7) & ~((uint64_t)7));
}

using position_t = uint32_t;
void sizeAlign(position_t& size) {
    size = (size + 7) & ~((position_t)7);
}

void sizeAlign(uint64_t& size) {
    size = (size + 7) & ~((uint64_t)7);
}

std::string uint64ToString(const uint64_t word) {
    uint64_t endian_swapped_word = __builtin_bswap64(word);
    return std::string(reinterpret_cast<const char*>(&endian_swapped_word), 8);
}

uint64_t stringToUint64(const std::string& str_word) {
    uint64_t int_word = 0;
    memcpy(reinterpret_cast<char*>(&int_word), str_word.data(), 8);
    return __builtin_bswap64(int_word);
}

} // namespace surf

#endif // CONFIG_CC_
