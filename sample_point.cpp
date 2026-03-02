 #include "sample_point.h"

HWOverlay::SamplePoint HWOverlay::make_point(unsigned long long time, double measurement) {

    SamplePoint sample = {
        .measurement = measurement,
        .timestamp = time,
    };

    return sample;
}