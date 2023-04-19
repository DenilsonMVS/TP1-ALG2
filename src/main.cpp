
#include <fstream>
#include <iterator>

#include "argReader.hpp"
#include "compression.hpp"


static std::vector<uint8_t> loadFile(const std::string_view &source) {
    
    auto file = std::ifstream(source.data(), std::ios::binary);
    file.unsetf(std::ios::skipws);
    
    file.seekg(0, std::ios::end);
    auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    auto data = std::vector<uint8_t>();
    data.reserve(fileSize);

    data.insert(
        data.begin(),
        std::istream_iterator<uint8_t>(file),
        std::istream_iterator<uint8_t>()
    );

    return data;
}

static void writeToFile(const std::string_view &output, const void * data, const size_t size) {
    auto file = std::ofstream(output.data(), std::ios::binary);
    file.write((const char *) data, size);
}


static void compressFile(const std::string_view &input, const std::string_view &output) {
    const auto inputContent = loadFile(input);
    const auto compressedContent = compress(inputContent.data(), inputContent.size());
    writeToFile(output, compressedContent.first.data(), compressedContent.second);
}

static void decompressFile(const std::string_view &input, const std::string_view &output) {
    const auto inputContent = loadFile(input);
    const auto decompressedContent = decompress(inputContent.data(), inputContent.size());
    writeToFile(output, decompressedContent.data(), decompressedContent.size());
}


static constexpr auto COMPRESS = size_t(0);
static constexpr auto DECOMPRESS = size_t(1);

struct InputInfo {
    size_t mode;
    std::string_view inputFile;
    std::string outputFile;
};

static InputInfo getInputInfo(const int argc, const char * const * argv) {
    
    auto args = ArgReader(argc, argv);
    auto inputInfo = InputInfo();

    while(args.canRead()) {
        auto v = std::string_view(args.readNext<const char *>());
        if(v == "-c") {
            inputInfo.mode = COMPRESS;
            inputInfo.inputFile = std::string_view(args.readNext<const char *>());
        } else if(v == "-x") {
            inputInfo.mode = DECOMPRESS;
            inputInfo.inputFile = std::string_view(args.readNext<const char *>());
        } else if(v == "-o") {
            inputInfo.outputFile = args.readNext<std::string>();
        }
    }


    if(inputInfo.outputFile == "") {
        for(auto &c : inputInfo.inputFile) {
            if(c == '.')
                break;
            inputInfo.outputFile += c;
        }

        inputInfo.outputFile += (inputInfo.mode == COMPRESS) ? ".z78" : ".txt"; 
    }

    return inputInfo;
}

int main(const int argc, const char * const * argv) {

    const auto [mode, inputFile, outputFile] = getInputInfo(argc, argv);
    if(mode == COMPRESS)
        compressFile(inputFile, outputFile);
    else
        decompressFile(inputFile, outputFile);

    return 0;
}
