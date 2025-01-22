#include "AG_LeakDetector_Backend.h"

AG_LeakDetector_Backend::AG_LeakDetector_Backend(AG_LeakDetector &_leak_detector, AG_LeakDetector::LeakDetector_State &_state) :
    leak_detector(_leak_detector),
    state(_state)
{}
