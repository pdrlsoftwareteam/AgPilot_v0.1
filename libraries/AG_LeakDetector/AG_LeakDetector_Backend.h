#pragma once

#include "AG_LeakDetector.h"

class AG_LeakDetector_Backend {
public:
    AG_LeakDetector_Backend(AG_LeakDetector &_leak_detector, AG_LeakDetector::LeakDetector_State &_state);

    // Each backend type must provide an implementation to read the sensor
    virtual void read(void) = 0;

protected:
    AG_LeakDetector &leak_detector;
    AG_LeakDetector::LeakDetector_State &state;
};
