
#ifndef BITS_HPP
#define BITS_HPP

#include <vector>
#include <cstddef>

static constexpr auto BYTE_BITS_SIZE = size_t(8);
static constexpr auto SIZE_T_BITS_SIZE = sizeof(size_t) * BYTE_BITS_SIZE;

class BitWriter {
public:
    BitWriter(const size_t expectedBitSize = 0);
    
    void writeBits(size_t value, const size_t size);
    std::vector<size_t> finish();

private:
    std::vector<size_t> data;
    size_t offset;
    size_t wordToPush;
};

class BitReader {
public:
    BitReader(const size_t data[]);

    size_t readBits(const size_t size);

private:
    const size_t *data;
    size_t offset;
    size_t index;
};


#endif
