#include "AG_LeakDetector_Digital.h"
#include <AG_HAL/AG_HAL.h>

extern const AG_HAL::HAL& hal;

AG_LeakDetector_Digital::AG_LeakDetector_Digital(AG_LeakDetector &_leak_detector, AG_LeakDetector::LeakDetector_State &_state) :
    AG_LeakDetector_Backend(_leak_detector, _state)
{}

void AG_LeakDetector_Digital::read()
{
    if (leak_detector._pin[state.instance] >= 0) {
        hal.gpio->pinMode(leak_detector._pin[state.instance], HAL_GPIO_INPUT);
        state.status = hal.gpio->read(leak_detector._pin[state.instance]) != leak_detector._default_reading[state.instance];
    } else {
        state.status = false;
    }
}
