#include <cstdint>

class Reader {
private:
    int* buffer;
    size_t position;

public:
    Reader(int* buffer)
        : buffer(buffer), position(0) {}

    int readUint8() {
        int index = position;
        position += 1;
        int value = buffer[index];
        return buffer[index];
    }

    Reader& seek(size_t newPosition) {
        position = newPosition;
        return *this;
    }
    Reader& skip(size_t count = 1) {
        position += count;
        return *this;
    }
};
