#pragma once

#include "AG_LeakDetector_Backend.h"
#include <AG_HAL/AG_HAL.h>

class AG_LeakDetector_Analog : public AG_LeakDetector_Backend {
public:
    AG_LeakDetector_Analog(AG_LeakDetector &_leak_detector, AG_LeakDetector::LeakDetector_State &_state);
    void read(void) override;

private:
    AG_HAL::AnalogSource *source;
};
