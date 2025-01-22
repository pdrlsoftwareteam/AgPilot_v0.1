#include "AG_LeakDetector_Analog.h"
#include <AG_HAL/AG_HAL.h>

extern const AG_HAL::HAL& hal;

AG_LeakDetector_Analog::AG_LeakDetector_Analog(AG_LeakDetector &_leak_detector, AG_LeakDetector::LeakDetector_State &_state) :
    AG_LeakDetector_Backend(_leak_detector, _state)
{
    source = hal.analogin->channel(leak_detector._pin[state.instance]);
}

void AG_LeakDetector_Analog::read()
{
    if (source != NULL && leak_detector._pin[state.instance] >= 0 && source->set_pin(leak_detector._pin[state.instance])) {
        state.status = source->voltage_average() > 2.0f;
        state.status = state.status != leak_detector._default_reading[state.instance];
    } else {
        state.status = false;
    }
}
