#include "AG_Gripper_Backend.h"

#if AP_GRIPPER_ENABLED

#include <AG_Math/AG_Math.h>

void AG_Gripper_Backend::init()
{
    init_gripper();
}

// update - should be called at at least 10hz
void AG_Gripper_Backend::update()
{
    update_gripper();

    // close the gripper again if autoclose_time > 0.0
    if (config.state == AG_Gripper::STATE_RELEASED && (_last_grab_or_release > 0) &&
        (is_positive(config.autoclose_time)) &&
        (AG_HAL::millis() - _last_grab_or_release > (config.autoclose_time * 1000.0))) {
        grab();
    }
}

#endif  // AP_GRIPPER_ENABLED
