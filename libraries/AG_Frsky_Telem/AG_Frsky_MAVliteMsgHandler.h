#pragma once

#include "AG_Frsky_MAVlite.h"
#include "AG_Frsky_Telem.h"
#include "AG_Frsky_MAVlite_Message.h"

#if HAL_WITH_FRSKY_TELEM_BIDIRECTIONAL

class AG_Frsky_MAVliteMsgHandler {
public:

    FUNCTOR_TYPEDEF(send_mavlite_fn_t, bool, const AG_Frsky_MAVlite_Message &);
    AG_Frsky_MAVliteMsgHandler(send_mavlite_fn_t send_fn) :
        _send_fn(send_fn) {}

    void process_message(const AG_Frsky_MAVlite_Message &rxmsg);

private:
    // mavlite messages tx/rx methods
    bool send_message(AG_Frsky_MAVlite_Message &txmsg);

    // gcs mavlite methods
    void handle_param_request_read(const AG_Frsky_MAVlite_Message &rxmsg);
    void handle_param_set(const AG_Frsky_MAVlite_Message &rxmsg);

    void handle_command_long(const AG_Frsky_MAVlite_Message &rxmsg);

    MAV_RESULT handle_command(const mavlink_command_long_t &mav_command_long);
    MAV_RESULT handle_command_preflight_calibration_baro(const mavlink_command_long_t &mav_command_long);
    MAV_RESULT handle_command_do_set_mode(const mavlink_command_long_t &mav_command_long);
    MAV_RESULT handle_command_do_fence_enable(const mavlink_command_long_t &mav_command_long);
    MAV_RESULT handle_command_preflight_reboot(const mavlink_command_long_t &mav_command_long);

    void send_command_ack(const MAV_RESULT mav_result, const uint16_t cmdid);

    send_mavlite_fn_t _send_fn;
};

#endif
