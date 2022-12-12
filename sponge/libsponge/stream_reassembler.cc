#include "stream_reassembler.hh"

#include <exception>
#include <ios>
#include <iostream>
#include <limits>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _total_size(numeric_limits<size_t>::max()), _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    auto flag_force_commit = false;
    if (eof) {
        _total_size = index + data.size();
    }
    if (index + data.size() >= _begin && index <= _begin) {
        flag_force_commit = true;
    } else if (_substrings_size >= _capacity || index + data.size() < _begin) {
        try_write_output();
        return;
    }

    size_t data_i = 0, data_size = data.size(), data_begin_index = index;

    if (index < _begin) {
        auto delta = _begin - index;
        data_i += delta;
        data_size -= delta;
        data_begin_index = _begin;
    }
    if (!flag_force_commit && data_size + _substrings_size > _capacity) {
        data_size = _capacity - _substrings_size;
    }

    auto substring = data.substr(data_i, data_size);

    // merge the substream before the new one if overlapped
    if (!sub_streams.empty()) {
        decltype(sub_streams)::iterator it;
        if (sub_streams.size() == 1) {
            it = sub_streams.begin();
            if (it->first >= data_begin_index) {
                goto merge_before_finished;
            }
        } else {
            it = sub_streams.lower_bound(data_begin_index);
            if (it == sub_streams.end() || it == sub_streams.begin()) {
                goto merge_before_finished;
            }
            it--;
        }
        if (it->first + it->second.size() >= data_begin_index) {
            if (it->first + it->second.size() >= data_begin_index + substring.size()) {
                return;
            }
            substring = it->second.substr(0, data_begin_index - it->first).append(substring);
            data_begin_index = it->first;
            _substrings_size -= it->second.size();
            sub_streams.erase(it);
        }
    }
merge_before_finished:

    // merge the substream after the new one if overlapped
    for (auto iter = sub_streams.lower_bound(data_begin_index);
         iter != sub_streams.end() && iter->first <= data_begin_index + substring.size();
         iter = sub_streams.lower_bound(data_begin_index)) {
        auto iter_substring_pos = data_begin_index + substring.size() - iter->first;
        if (iter->second.size() > iter_substring_pos) {
            auto iter_substring_count = iter->second.size() - iter_substring_pos;
            substring.append(iter->second.substr(iter_substring_pos, iter_substring_count));
        }
        _substrings_size -= iter->second.size();
        sub_streams.erase(iter);
    }

    sub_streams[data_begin_index] = substring;
    _substrings_size += substring.size();

    try_write_output();
}

// sub_streams must not be empty
void StreamReassembler::try_write_output() {
    auto min = sub_streams.begin();
    if (min->first == _begin && _output.remaining_capacity() != 0) {
        if (_output.remaining_capacity() < min->second.size()) {
            auto write_size = _output.remaining_capacity();
            _output.write(min->second.substr(0, write_size));
            _substrings_size -= write_size;

            auto new_substring = min->second.substr(write_size, min->second.size() - write_size);
            _begin += write_size;
            sub_streams.erase(min);
            sub_streams[_begin] = new_substring;
        } else {
            _output.write(min->second);
            _substrings_size -= min->second.size();
            _begin += min->second.size();
            sub_streams.erase(min);
        }
        if (_begin == _total_size) {
            _output.end_input();
        }
    }
}

size_t StreamReassembler::unassembled_bytes() const {
    if (sub_streams.empty()) {
        return 0;
    }
    auto min = sub_streams.begin();
    if (min != sub_streams.end() && min->first == _begin) {
        return _substrings_size - min->second.size();
    }
    return _substrings_size;
}

bool StreamReassembler::empty() const { return sub_streams.empty(); }
