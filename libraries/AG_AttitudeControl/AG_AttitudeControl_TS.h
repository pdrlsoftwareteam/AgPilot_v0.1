#pragma once

/// @file    AG_AttitudeControl_TVBS.h
/// @brief   ArduCopter attitude control library

#include "AG_AttitudeControl_Multi.h"

class AG_AttitudeControl_TS : public AG_AttitudeControl_Multi
{
public:
    using AG_AttitudeControl_Multi::AG_AttitudeControl_Multi;

    // empty destructor to suppress compiler warning
    virtual ~AG_AttitudeControl_TS() {}

    // Ensure attitude controllers have zero errors to relax rate controller output
    // Relax only the roll and yaw rate controllers if exclude_pitch is true
    virtual void relax_attitude_controllers(bool exclude_pitch) override;
    virtual void input_euler_rate_yaw_euler_angle_pitch_bf_roll(bool plane_controls, float body_roll_cd, float euler_pitch_cd, float euler_yaw_rate_cds) override;
};
