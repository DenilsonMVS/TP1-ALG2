
#include "compression.hpp"

#include <functional>


static constexpr auto INTEGER_8_VALUES = size_t(256);
static constexpr auto NONE = size_t(-1);
static constexpr auto BYTE_BITS_SIZE = size_t(8);
static constexpr auto SIZE_T_BITS_SIZE = sizeof(size_t) * BYTE_BITS_SIZE;
static constexpr auto INDEX_SIZE_SIZE_BITS = 7;


class BitWriter {
public:
    BitWriter(const size_t expectedBitSize = 0):
        offset(0),
        wordToPush(0)
    {
        this->data.reserve(expectedBitSize / SIZE_T_BITS_SIZE + ((expectedBitSize % SIZE_T_BITS_SIZE ) > 0));
    }
    
    void writeBits(size_t value, const size_t size) {
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

    std::vector<size_t> finish() {
        if(offset > 0)
            this->data.push_back(this->wordToPush);
        this->offset = 0;
        this->wordToPush = 0;
        return std::move(this->data);
    }

private:
    std::vector<size_t> data;
    size_t offset;
    size_t wordToPush;
};

class BitReader {
public:
    BitReader(const size_t data[]):
        data(data),
        offset(0),
        index(0) {}

    size_t readBits(const size_t size) {
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

private:
    const size_t *data;
    size_t offset;
    size_t index;
};


static size_t getMaxIndex(
    const std::vector<std::pair<uint8_t, size_t>> &compressionStream)
{
    auto max = compressionStream.size();
    for(auto &[_, index] : compressionStream)
        max = std::max(max, index);
    return max;
}

static size_t getIndexSize(
    const std::vector<std::pair<uint8_t, size_t>> &compressionStream)
{
    const auto maxIndex = getMaxIndex(compressionStream);
    for(int8_t i = 63; i >= 0; i--)
        if(maxIndex >> i)
            return i + 1;
    return 0;
}


static std::pair<std::vector<size_t>, size_t> buildFile(
    const std::vector<std::pair<uint8_t, size_t>> &compressionStream)
{
    const auto indexSize = getIndexSize(compressionStream);
    const auto fileSizeBits = (indexSize + BYTE_BITS_SIZE) * compressionStream.size() + INDEX_SIZE_SIZE_BITS + indexSize;

    auto bitWriter = BitWriter(fileSizeBits);
    bitWriter.writeBits(indexSize, INDEX_SIZE_SIZE_BITS);
    bitWriter.writeBits(compressionStream.size(), indexSize);

    for(auto &[character, index] : compressionStream) {
        bitWriter.writeBits(character, BYTE_BITS_SIZE);
        bitWriter.writeBits(index, indexSize);
    }

    return {bitWriter.finish(), fileSizeBits / BYTE_BITS_SIZE + ((fileSizeBits % BYTE_BITS_SIZE) > 0)};
}

std::vector<std::pair<uint8_t, size_t>> run(const uint8_t bytes[], const size_t size) {

    struct Node {
        Node() {
            for(size_t i = 0; i < INTEGER_8_VALUES; i++)
                this->childs[i] = NONE;
        }

        size_t childs[INTEGER_8_VALUES];
    };


    auto nodes = std::vector<Node>();

    const auto createNode = std::function([&nodes]() {
        const auto nodeIndex = nodes.size();
        nodes.push_back(Node());
        return nodeIndex;
    });
    

    auto output = std::vector<std::pair<uint8_t, size_t>>();
    createNode();
    for(size_t i = 0; i < size; i++) {
        
        size_t current = 0;
        while(nodes[current].childs[bytes[i]] != NONE) {
            if((i + 1) == size) {
                output.push_back({bytes[i], current});
                return output;
            }

            current = nodes[current].childs[bytes[i++]];
        }
        
        nodes[current].childs[bytes[i]] = createNode();
        output.push_back({bytes[i], current});
    }

    return output;
}

std::pair<std::vector<size_t>, size_t> compress(const void * data, const size_t size) {
    return buildFile(run((const uint8_t *) data, size));
}


static std::vector<std::pair<uint8_t, size_t>> loadCompressionStream(
    const uint8_t data[],
    const size_t size)
{
    auto bitReader = BitReader((const size_t *) data);

    const auto indexSize = bitReader.readBits(INDEX_SIZE_SIZE_BITS);
    const auto streamSize = bitReader.readBits(indexSize);

    auto compressionStream = std::vector<std::pair<uint8_t, size_t>>();
    compressionStream.reserve(streamSize);

    for(size_t i = 0; i < streamSize; i++) {
        const auto c = uint8_t(bitReader.readBits(BYTE_BITS_SIZE));
        const auto index = bitReader.readBits(indexSize);
        compressionStream.push_back({c, index});
    }

    return compressionStream;
}

static std::vector<uint8_t> processCompressionStream(
    const std::vector<std::pair<uint8_t, size_t>> &compressionStream)
{
    struct Node {
        Node(const size_t parent = NONE, const uint8_t evaluation = '\0'):
            evaluation(evaluation),
            parent(parent)
        {
            for(size_t i = 0; i < INTEGER_8_VALUES; i++)
                this->childs[i] = NONE;
        }

        uint8_t evaluation;
        size_t parent;
        size_t childs[INTEGER_8_VALUES];
    };


    auto nodes = std::vector<Node>();

    const auto createNode = std::function([&nodes](const size_t parent, const uint8_t evaluation) {
        const auto nodeIndex = nodes.size();
        nodes.push_back(Node(parent, evaluation));
        return nodeIndex;
    });

    auto path = std::vector<uint8_t>();
    auto output = std::vector<uint8_t>();
    createNode(NONE, '\0');
    for(auto &[character, index] : compressionStream) {

        auto current = index;
        while(nodes[current].parent != NONE) {
            path.push_back(nodes[current].evaluation);
            current = nodes[current].parent;
        }

        if(nodes[index].childs[character] == NONE)
            nodes[index].childs[character] = createNode(index, character);
        
        while(!path.empty()) {
            output.push_back(path.back());
            path.pop_back();
        }
        output.push_back(character);
    }

    return output;
}

std::vector<uint8_t> decompress(const void * data, const size_t size) {
    return processCompressionStream(loadCompressionStream((const uint8_t *) data, size));
}
