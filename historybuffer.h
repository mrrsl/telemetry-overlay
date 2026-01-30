#ifndef HISTORYBUFFER_H
#define HISTORYBUFFER_H

#include<immintrin.h>

/**
 * @brief Class for quickly storing historical sampling data
 */
template<typename sample_type>
class HistoryBuffer
{
    /** Maximum number of elements. */
    unsigned size;

public:
    HistoryBuffer();
    HistoryBuffer(unsigned);
};

#endif // HISTORYBUFFER_H
