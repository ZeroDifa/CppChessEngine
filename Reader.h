#include <cstdint>

class Reader {
private:
    uint8_t* buffer;
    size_t position;

public:
    Reader(uint8_t* buffer)
        : buffer(buffer), position(0) {}

    uint8_t readUint8() {
        int index = position;
        position += 1;
        uint8_t value = buffer[index];
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
