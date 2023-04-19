
#ifndef COMPRESSION_HPP
#define COMPRESSION_HPP

#include <cstdint>
#include <vector>
#include <cstddef>

std::pair<std::vector<size_t>, size_t> compress(const void * data, const size_t size);
std::vector<uint8_t> decompress(const void * data, const size_t size);

#endif
