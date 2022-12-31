#include "tcp_receiver.hh"

#include "stream_reassembler.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (state == LISTEN && seg.header().syn) {
        state = SYN_RECV;
        _isn = seg.header().seqno;

        push_payload(seg);

        _isn = _isn + 1;
    } else if (state == SYN_RECV || state == FIN_RECV) {
        push_payload(seg);
    }
}

void TCPReceiver::push_payload(const TCPSegment &seg) {
    if (seg.header().fin) {
        state = FIN_RECV;
    }
    auto index = unwrap(seg.header().seqno, _isn, stream_out().bytes_written());
    _reassembler.push_substring(seg.payload().copy(), index, seg.header().fin);
    if (state == FIN_RECV && _reassembler.unassembled_bytes() == 0) {
        stream_out().end_input();
        _isn = _isn + 1;
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (state == LISTEN) {
        return {};
    }
    return _isn + stream_out().bytes_written();
}

size_t TCPReceiver::window_size() const { return stream_out().remaining_capacity(); }
