#pragma once

#include "AG_LeakDetector_Backend.h"

class AG_LeakDetector_Digital : public AG_LeakDetector_Backend {
public:
    AG_LeakDetector_Digital(AG_LeakDetector &_leak_detector, AG_LeakDetector::LeakDetector_State &_state);
    void read(void) override;
};
