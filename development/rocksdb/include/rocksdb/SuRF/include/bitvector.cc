#ifndef BITVECTOR_CC_
#define BITVECTOR_CC_

#include <assert.h>
#include <vector>

#include "bitvector.hpp"

namespace surf {

bool Bitvector::readBit (const position_t pos) const {
    assert(pos <= num_bits_);
    position_t word_id = pos / kWordSize;
    position_t offset = pos & (kWordSize - 1);
    return bits_[word_id] & (kMsbMask >> offset);
}

position_t Bitvector::distanceToNextSetBit (const position_t pos) const {
    assert(pos < num_bits_);
	// YCHUANG_ADDED START
	//TODO: handing if pos == num_bits_ - 1
	// if(pos == num_bits_ - 1) return -1;
	assert(pos != num_bits_ - 1); // undefined behavior
	// YCHUANG_ADDED END

    position_t distance = 1;

    position_t word_id = (pos + 1) / kWordSize;
    position_t offset = (pos + 1) % kWordSize;

    //first word left-over bits
    word_t test_bits = bits_[word_id] << offset;
    if (test_bits > 0) {
	return (distance + __builtin_clzll(test_bits));
    } else {
	if (word_id == numWords() - 1)
	    return (num_bits_ - pos);
	distance += (kWordSize - offset);
    }

    while (word_id < numWords() - 1) {
	word_id++;
	test_bits = bits_[word_id];
	if (test_bits > 0)
	    return (distance + __builtin_clzll(test_bits));
	distance += kWordSize;
    }
    return distance;
}

position_t Bitvector::distanceToPrevSetBit (const position_t pos) const {
    assert(pos <= num_bits_);
    if (pos == 0) return 0;
    position_t distance = 1;

    position_t word_id = (pos - 1) / kWordSize;
    position_t offset = (pos - 1) % kWordSize;

    //first word left-over bits
    word_t test_bits = bits_[word_id] >> (kWordSize - 1 - offset);
    if (test_bits > 0) {
	return (distance + __builtin_ctzll(test_bits));
    } else {
	//if (word_id == 0)
	//return (offset + 1);
	distance += (offset + 1);
    }

    while (word_id > 0) {
	word_id--;
	test_bits = bits_[word_id];
	if (test_bits > 0)
	    return (distance + __builtin_ctzll(test_bits));
	distance += kWordSize;
    }
    return distance;
}

position_t Bitvector::totalNumBits(const std::vector<position_t>& num_bits_per_level, 
			     const level_t start_level, 
			     const level_t end_level/* non-inclusive */) {
    position_t num_bits = 0;

	// // YCHUANG_ADDED START
	// if(num_bits_per_level.size() == 0) return 0;
	// // YCHUANG_ADDED END

    for (level_t level = start_level; level < end_level; level++)
	num_bits += num_bits_per_level[level];
    return num_bits;
}

// YCHuang Added
position_t Bitvector::serializedSize() const {
	position_t size = sizeof(num_bits_)+ bitsSize();
	sizeAlign(size);
	return size;
}

void Bitvector::serialize(char*& dst) const {
	memcpy(dst, &num_bits_, sizeof(num_bits_));
	dst += sizeof(num_bits_);
	memcpy(dst, bits_, bitsSize());
	dst += bitsSize();
	align(dst);
}

Bitvector* Bitvector::deSerialize(char*& src) {
	Bitvector* bv = new Bitvector();
	memcpy(&(bv->num_bits_), src, sizeof(bv->num_bits_));
	src += sizeof(bv->num_bits_);

	bv->bits_ = new word_t[bv->numWords()];
	memcpy(bv->bits_, src, bv->bitsSize());
	src += bv->bitsSize();
	
	align(src);
	return bv;
}
// YCHuang Added END

void Bitvector::concatenateBitvectors(const std::vector<std::vector<word_t> >& bitvector_per_level, 
				      const std::vector<position_t>& num_bits_per_level, 
				      const level_t start_level, 
				      const level_t end_level/* non-inclusive */) {
    position_t bit_shift = 0;
    position_t word_id = 0;

	// YCHUANG_ADDED START
	// if(num_bits_per_level.size() == 0) return;
	// YCHUANG_ADDED END

    for (level_t level = start_level; level < end_level; level++) {
	if (num_bits_per_level[level] == 0) continue;
	position_t num_complete_words = num_bits_per_level[level] / kWordSize;
	for (position_t word = 0; word < num_complete_words; word++) {
	    bits_[word_id] |= (bitvector_per_level[level][word] >> bit_shift);
	    word_id++;
	    if (bit_shift > 0)
		bits_[word_id] |= (bitvector_per_level[level][word] << (kWordSize - bit_shift));
	}

	word_t bits_remain = num_bits_per_level[level] - num_complete_words * kWordSize;
	if (bits_remain > 0) {
	    word_t last_word = bitvector_per_level[level][num_complete_words];
	    bits_[word_id] |= (last_word >> bit_shift);
	    if (bit_shift + bits_remain < kWordSize) {
		bit_shift += bits_remain;
	    } else {
		word_id++;
		bits_[word_id] |= (last_word << (kWordSize - bit_shift));
		bit_shift = bit_shift + bits_remain - kWordSize;
	    }
	}
    }
}

} // namespace surf

#endif // BITVECTOR_CC_
