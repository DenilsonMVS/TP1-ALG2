
#ifndef ARG_READER_HPP
#define ARG_READER_HPP

#include <cstddef>
#include <sstream>


class ArgReader {
public:
    struct InvalidInput {};

    ArgReader(const int argc, const char * const argv[]);

    void setPos(const int pos);
    void reset();

    bool canRead() const;

    template<typename T>
    T readNext() {
        if(!this->canRead())
            throw InvalidInput();

        try {
            auto sstream = std::istringstream(this->argv[this->currentArg++]);

            T rtn;
            sstream >> rtn;

            return rtn;
        } catch(...) {
            throw InvalidInput();
        }
    }


private:
    int currentArg;
    const int argc;
    const char * const * argv;
};

template<>
inline const char *ArgReader::readNext<const char *>() {
    if(!this->canRead())
        throw InvalidInput();
    return this->argv[this->currentArg++];
}


#endif
