#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : content(capacity, '0')
    , _begin(0)
    , _size(0)
    , _total_read(0)
    , _total_write(0)
    , _input_end(false)
    , _error(false) {}

size_t ByteStream::write(const string &data) {
    if (_size < content.size()) {
        size_t write_size;
        if (data.size() + _size > content.size()) {
            write_size = content.size() - _size;
        } else {
            write_size = data.size();
        }

        if (_begin + _size < content.size() && _begin + _size + write_size > content.size()) {
            content.replace(
                _begin + _size, content.size() - (_begin + _size), data.substr(0, content.size() - (_begin + _size)));
            content.replace(
                0, _begin + _size + write_size - content.size(), data.substr(content.size() - (_begin + _size), write_size));
        } else {
            content.replace((_begin + _size) % content.size(), write_size, data.substr(0, write_size));
        }

        _size += write_size;
        _total_write += write_size;
        return write_size;
    }
    return 0;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    auto peek_size = _size > len ? len : _size;
    if (_begin + peek_size > content.size()) {
        return content.substr(_begin, content.size()) + content.substr(0, peek_size - (content.size() - _begin));
    }
    return content.substr(_begin, peek_size);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    _begin = (_begin + len) % content.size();
    if (len < _size) {
        _total_read += len;
        _size -= len;
    } else {
        _total_read += _size;
        _size = 0;
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    auto ret = peek_output(len);
    pop_output(ret.size());
    return ret;
}

void ByteStream::end_input() { _input_end = true; }

bool ByteStream::input_ended() const { return _input_end; }

size_t ByteStream::buffer_size() const { return _size; }

bool ByteStream::buffer_empty() const { return _size == 0; }

bool ByteStream::eof() const { return buffer_empty() && input_ended(); }

size_t ByteStream::bytes_written() const { return _total_write; }

size_t ByteStream::bytes_read() const { return _total_read; }

size_t ByteStream::remaining_capacity() const { return content.size() - _size; }
