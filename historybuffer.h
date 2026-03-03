#ifndef HISTORYBUFFER_H
#define HISTORYBUFFER_H

#include <array>
#include <vector>
#include <chrono>

#include "sample_point.h"

namespace HWOverlay {
/**
 * Class for quickly storing historical sampling data as a value between `[0, 1)`.
 * Goals for this class:
 *  - Avoid making the QML engine iterate through the entire array to update the rolling sample history
 *  - Reduce the constant memory allocations made by QML whenever a new history entry is pushed onto the Array
 *
 * Since the use case involves consistent sampling intervals, no timestamps will be recorded.
 */
template <int buff_size>
class HistoryBuffer
{   
    /** Index at the head of the buffer. */
    unsigned head;

    /** Current number of items stored in buffer. */
    unsigned length;

    /** Maximum number of items held by buffer. */
    const unsigned size_limit;

    std::array<SamplePoint, buff_size> buffer;

public:
    /**
     * Default construction to set the internal max size.
     */
    HistoryBuffer<buff_size>():
        buffer{},
        size_limit(buffer.max_size()) {

        head = 0;
        length = 0;
        std::memset(buffer.data(), 0, sizeof(SamplePoint) * size_limit);
    }

    /**
     * Deep copy semantics.
     */
    HistoryBuffer(const HistoryBuffer& that) {
        buffer = std::array<SamplePoint, buff_size>(that);
        head = that.head;
        length = that.length;
        size_limit = buffer.max_size();
    }

    /**
     * Adds a measurement to the buffer.
     * @param measurement Measurement value.
     */
    void add(double measurement) {
        
        auto advanced = head + length;
        auto next = (head + length) % size_limit;
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());

        buffer[next].measurement = measurement;
        buffer[next].timestamp = now_ms.count();

        auto inc_length = length + 1;

        if (inc_length > size_limit) {
            length = size_limit;
            head = (head + 1) % size_limit;
        }
        else {
            length = inc_length;
        }
    }

    /**
     * Produce a linear representation of the buffer's underlying data.
     * @return Vector with the oldest sample occupying index 0.
     */
    std::vector<SamplePoint> get_linear_buffer() const {

        auto head_seg_length = size_limit - head;
        auto tail_seg_legth = head;
        auto head_it = buffer.begin() + head;

        auto lin_buff = std::vector<SamplePoint>(size_limit);
        auto lin_tail_seg_it = lin_buff.begin() + head_seg_length;

        std::copy(head_it, head_it + head_seg_length, lin_buff.begin());
        std::copy(buffer.begin(), head_it, lin_tail_seg_it);

        return lin_buff;
    }
};

}
#endif // HISTORYBUFFER_H
