#ifndef HISTORYBUFFER_H
#define HISTORYBUFFER_H

#include <vector>
#include <QVector>

namespace HWOverlay {

/**
 * Class for quickly storing historical sampling data as a value between `[0, 1)`.
 * Goals for this class:
 *  - Avoid making the QML engine iterate through the entire array to update the rolling sample history
 *  - Reduce the constant memory allocations made by QML whenever a new history entry is pushed onto the Array
 *
 * Since the use case involves consistent sampling intervals, no timestamps will be recorded.
 */

class HistoryBuffer
{


    std::vector<float> buffer;

public:
    /**
     * Reserve an initial size for HistoryBuffer.
     * @param initial_size Initial size of the internal buffer.
     */
    HistoryBuffer(unsigned initial_size);

    /**
     * Deep copy semantics.
     * @param Will not share ownership of any members.
     */
    HistoryBuffer(const HistoryBuffer&);

    ~HistoryBuffer();

    /**
     * Adds a measurement to the buffer.
     *
     */
    void add(float measurement);

};

}
#endif // HISTORYBUFFER_H
