#ifndef HW_SAMPLE_POINT_H
#define HW_SAMPLE_POINT_H

#include <type_traits>

namespace HWOverlay {
/**
 * Container for storing rolling historical sample data.
 */
struct SamplePoint {
    
    /** Measurement value. */
    double measurement;

    /** Time that measurement was taken. */
    unsigned long long timestamp;

};

SamplePoint make_point(unsigned long long time, double measurement);
}

// Make sure it remains POD
static_assert(
    std::is_trivial_v<HWOverlay::SamplePoint> && std::is_standard_layout_v<HWOverlay::SamplePoint>,
    "SamplePoint is not trivially copyable"
);

#endif