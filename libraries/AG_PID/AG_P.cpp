/// @file	AG_P.cpp
/// @brief	Generic P algorithm

#include <AG_Math/AG_Math.h>
#include "AG_P.h"

const AG_Param::GroupInfo AG_P::var_info[] = {
    // @Param: P
    // @DisplayName: PI Proportional Gain
    // @Description: P Gain which produces an output value that is proportional to the current error value
    AP_GROUPINFO_FLAGS_DEFAULT_POINTER("P",    0, AG_P, _kp, default_kp),
    AP_GROUPEND
};

float AG_P::get_p(float error) const
{
    return (float)error * _kp;
}

void AG_P::load_gains()
{
    _kp.load();
}

void AG_P::save_gains()
{
    _kp.save();
}
