// All operations connected with buffers processing in
// serializing/deserializing data.

#ifndef ZAD2_BUFFER_H
#define ZAD2_BUFFER_H

#include <iostream>
#include <vector>
#include <cstring>

using string_size_t = uint8_t;
using map_size_t = uint32_t;
using list_size_t = uint32_t;
using buffer_t = std::vector<uint8_t>;

// Error when message received from client isn't fully loaded yet.
class MessageTooShort : public std::exception {
public:
    const char *what() const noexcept override {
        return "Message is too short.";
    }
};

// Error when client sent wrong message.
class WrongMessage : public std::exception {
public:
    const char *what() const noexcept override {
        return "Wrong message received.";
    }
};

// Class used to pass data while sending messages.
class SendBuffer {
private:
    constexpr static size_t initial_length = 1024;
    // Buffer contains data.
    buffer_t buffer;
    // Pointer describes amount of elements present already in buffer.
    size_t ptr;

public:
    inline SendBuffer() {
        buffer.resize(initial_length, 0);
        ptr = 0;
    }

    inline explicit SendBuffer(size_t size) {
        buffer.resize(size, 0);
        ptr = size;
    }

    inline void reset_state() {
        ptr = 0;
    }

    inline size_t get_bytes_no() {
        return ptr;
    }

    inline buffer_t *get_buffer() {
        return &buffer;
    }

    inline void set_size(size_t size) {
        ptr = size;
    }

    // Enlarge buffer if it is to small.
    inline void check_resize(size_t size) {
        while (size + ptr > buffer.size())
            buffer.resize(2 * buffer.size(), 0);
    }

    // Copies given value to buffer in position `ptr`. Moves pointer afterwards.
    // Takes care of endiannes.
    template<typename T>
    inline void copy_into(const T &value) {
        check_resize(sizeof(value));
        if (std::is_same_v<T, uint16_t>) {
            T new_value = htons(value);
            memcpy(&buffer[ptr], &new_value, sizeof(value));
        } else if (std::is_same_v<T, uint32_t>) {
            T new_value = htonl(value);
            memcpy(&buffer[ptr], &new_value, sizeof(value));
        } else {
            memcpy(&buffer[ptr], &value, sizeof(value));
        }
        ptr += sizeof(value);
    }

    // Copies string to buffer (starting from `ptr`). Moves pointer afterwards.
    inline void copy_string_into(const std::string &value) {
        auto length = (string_size_t) value.size();
        check_resize(length);
        memcpy(&buffer[ptr], &length, sizeof(length));
        ptr += sizeof(length);
        memcpy(&buffer[ptr], &(value[0]), length);
        ptr += length;
    }
};

class ReceiveBuffer {
private:
    constexpr static size_t initial_length = 1024;
    // Buffer contains data.
    buffer_t buffer;
    // Pointer describes amount of elements present already in buffer.
    size_t ptr;
    // Number of processed bytes while decoding information from buffer.
    size_t processed;

    // Enlarge buffer if it is too small.
    inline void enlarge() {
        buffer.resize(2 * buffer.size(), 0);
    }

public:
    inline ReceiveBuffer() {
        buffer.resize(initial_length, 0);
        ptr = processed = 0;
    }

    inline size_t get_bytes_no() {
        return ptr;
    }

    inline void reset_state() {
        ptr = processed = 0;
    }

    inline void increase_size(size_t size) {
        ptr += size;
    }

    inline void reset_processed() {
        processed = 0;
    }

    inline buffer_t *get_buffer() {
        return &buffer;
    }

    inline void enlarge_if_full() {
        if (buffer.size() == ptr)
            enlarge();
    }

    // After parsing one message, next message might have already start coming.
    inline void reload() {
        for (auto i = processed; i < ptr; i++)
            buffer[i - processed] = buffer[i];
        ptr -= processed;
        processed = 0;
    }

    // Copies 'size' bytes from buffer on position `ptr`. Moves pointer afterwards.
    // Expects value to be properly resized. Takes care of endiannes.
    template<typename T>
    inline void copy_from(T &value) {
        if (processed + sizeof(value) > ptr)
            throw MessageTooShort();
        memcpy(&value, &buffer[processed], sizeof(value));
        if (std::is_same_v<T, uint16_t>)
            value = ntohs(value);
        else if (std::is_same_v<T, uint32_t>)
            value = ntohl(value);
        processed += sizeof(value);
    }

    // Decodes string from buffer. Moves pointer afterwards.
    inline void copy_string_from(std::string &dest) {
        if (processed == ptr)
            throw MessageTooShort();
        string_size_t size = buffer[processed];
        if (processed + size + 1 > ptr)
            throw MessageTooShort();
        dest.resize(size);
        processed++;
        memcpy(&dest[0], &buffer[processed], size);
        processed += size;
    }
};

#endif //ZAD2_BUFFER_H
