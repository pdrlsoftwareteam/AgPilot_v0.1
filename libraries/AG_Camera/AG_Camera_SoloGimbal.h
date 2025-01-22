#pragma once

#include "AG_Camera_Backend.h"
#include <AG_Mount/AG_Mount.h>

#if AP_CAMERA_SOLOGIMBAL_ENABLED

#include <GCS_MAVLink/GCS_MAVLink.h>

class AG_Camera_SoloGimbal : public AG_Camera_Backend
{
public:

    // Constructor
    using AG_Camera_Backend::AG_Camera_Backend;

    /* Do not allow copies */
    CLASS_NO_COPY(AG_Camera_SoloGimbal);

    // entry point to actually take a picture.  returns true on success
    bool trigger_pic() override;

    // momentary switch to change camera between picture and video modes
    void cam_mode_toggle() override;

    // handle incoming mavlink message
    void handle_message(mavlink_channel_t chan, const mavlink_message_t &msg) override;

private:

    GOPRO_CAPTURE_MODE gopro_capture_mode;
    GOPRO_HEARTBEAT_STATUS gopro_status;
    bool gopro_is_recording;
    mavlink_channel_t heartbeat_channel;
};

#endif // AP_CAMERA_SOLOGIMBAL_ENABLED
