/*
  MAVLink enabled mount backend class
 */
#pragma once


#include <AG_HAL/AG_HAL.h>

#include "AG_Mount_Backend.h"
#if HAL_SOLO_GIMBAL_ENABLED
#include <AG_Math/AG_Math.h>
#include <AG_Common/AG_Common.h>
#include <GCS_MAVLink/GCS_MAVLink.h>
#include <RC_Channel/RC_Channel.h>
#include "SoloGimbal.h"


class AG_Mount_SoloGimbal : public AG_Mount_Backend
{

public:
    // Constructor
    AG_Mount_SoloGimbal(AG_Mount &frontend, AG_Mount_Params &params, uint8_t instance);

    // init - performs any required initialisation for this instance
    void init() override;

    // update mount position - should be called periodically
    void update() override;

    // has_pan_control - returns true if this mount can control its pan (required for multicopters)
    bool has_pan_control() const override { return false; }

    // handle a GIMBAL_REPORT message
    void handle_gimbal_report(mavlink_channel_t chan, const mavlink_message_t &msg) override;
    void handle_gimbal_torque_report(mavlink_channel_t chan, const mavlink_message_t &msg);
    void handle_param_value(const mavlink_message_t &msg) override;

    void update_fast() override;

protected:

    // returns true if heart beat should be suppressed for this gimbal
    bool suppress_heartbeat() const override { return true; }

    // get attitude as a quaternion.  returns true on success
    bool get_attitude_quaternion(Quaternion& att_quat) override;

private:
    // internal variables
    bool _initialised;              // true once the driver has been initialised

    // Write a gimbal measurament and estimation data packet
    void Log_Write_Gimbal(SoloGimbal &gimbal);

    bool _params_saved;
    MountTarget _angle_rad;         // angle target
    SoloGimbal _gimbal;
};

#endif // HAL_SOLO_GIMBAL_ENABLED
