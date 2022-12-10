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
    if ((state == LISTEN || state == FIN_RECV) && seg.header().syn) {
        _isn = seg.header().seqno;
        state = SYN_RECV;

        push_payload(seg);
    } else if (state == SYN_RECV && (seg.header().syn || seg.header().fin)) {
        push_payload(seg);
    }
}

void TCPReceiver::push_payload(const TCPSegment &seg) {
    if (seg.header().ack) {
        _ackno.value() = seg.header().ackno;    
    } else {
        _ackno.reset();
    }
    
    if (seg.header().fin) {
        state = FIN_RECV;
        stream_out().end_input();
    }

    auto index = unwrap(seg.header().seqno, _isn, stream_out().bytes_written());
    _reassembler.push_substring(seg.payload().copy(), index, seg.header().fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    return _ackno;
}

size_t TCPReceiver::window_size() const { return stream_out().remaining_capacity(); }
