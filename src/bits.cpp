
#include "bits.hpp"


BitWriter::BitWriter(const size_t expectedBitSize):
    offset(0),
    wordToPush(0)
{
    this->data.reserve(expectedBitSize / SIZE_T_BITS_SIZE + ((expectedBitSize % SIZE_T_BITS_SIZE ) > 0));
}

void BitWriter::writeBits(size_t value, const size_t size) {
    value = ((size_t(1) << size) - 1) & value;

    if(this->offset + size <= SIZE_T_BITS_SIZE) {
        this->wordToPush |= value << this->offset;
        this->offset += size;
    } else {
        if(this->offset != 64)
            this->wordToPush |= value << this->offset;

        this->data.push_back(this->wordToPush);
        
        this->wordToPush = 0;
        const auto remainingBits = this->offset + size - SIZE_T_BITS_SIZE;
        this->wordToPush |= value >> (size - remainingBits);
        this->offset = remainingBits;
    }
}

std::vector<size_t> BitWriter::finish() {
    if(offset > 0)
        this->data.push_back(this->wordToPush);
    this->offset = 0;
    this->wordToPush = 0;
    return std::move(this->data);
}


BitReader::BitReader(const size_t data[]):
    data(data),
    offset(0),
    index(0) {}

size_t BitReader::readBits(const size_t size) {
    if(this->offset + size <= SIZE_T_BITS_SIZE) {
        const auto output = (this->data[this->index] >> this->offset) & ((size_t(1) << size) - 1);
        this->offset += size;
        return output;
    } else {
        size_t output = 0;
        if(this->offset != 64)
            output = (this->data[this->index] >> this->offset);
        this->index++;
        const auto remainingBits = this->offset + size - SIZE_T_BITS_SIZE;
        output |= (this->data[this->index] & ((size_t(1) << remainingBits) - 1)) << (size - remainingBits);
        this->offset = remainingBits;
        return output;
    }
}

