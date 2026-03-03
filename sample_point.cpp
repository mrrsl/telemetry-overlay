 #include "sample_point.h"

HWOverlay::SamplePoint HWOverlay::make_point(unsigned long long time, double measurement) {

    SamplePoint sample;
    sample.measurement = measurement;
    sample.timestamp = time;

    return sample;
}