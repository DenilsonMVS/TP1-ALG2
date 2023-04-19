
#include "argReader.hpp"


ArgReader::ArgReader(const int argc, const char * const argv[]):
    currentArg(1),
    argc(argc),
    argv(argv)
{}

void ArgReader::setPos(const int pos) {
    this->currentArg = pos;
}

void ArgReader::reset() {
    this->setPos(1);
}

bool ArgReader::canRead() const {
    return this->currentArg < this->argc;
}
