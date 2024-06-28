#include <cstdint>

class Writer {
private:
    uint8_t* buffer;
    size_t position;

public:
    Writer(uint8_t* buffer)
        : buffer(buffer), position(0) {}

    Writer& writeUint8(uint8_t value) {
        buffer[position++] = value;
        return *this;
    }

    Writer& writeUint8(size_t index, uint8_t value) {
        buffer[index] = value;
        return *this;
    }
    size_t getPosition() const {
        return position;
    }
    Writer& seek(size_t newPosition) {
        position = newPosition;
        return *this;
    }
    Writer& skip(size_t count = 1) {
        position += count;
        return *this;
    }
};
