#pragma once

#include "AP_Frsky_AGPILOTlite.h"
#include "AP_Frsky_Telem.h"
#include "AP_Frsky_AGPILOTlite_Message.h"

#if HAL_WITH_FRSKY_TELEM_BIDIRECTIONAL

class AP_Frsky_AGPILOTliteMsgHandler {
public:

    FUNCTOR_TYPEDEF(send_mavlite_fn_t, bool, const AP_Frsky_AGPILOTlite_Message &);
    AP_Frsky_AGPILOTliteMsgHandler(send_mavlite_fn_t send_fn) :
        _send_fn(send_fn) {}

    void process_message(const AP_Frsky_AGPILOTlite_Message &rxmsg);

private:
    // mavlite messages tx/rx methods
    bool send_message(AP_Frsky_AGPILOTlite_Message &txmsg);

    // gcs mavlite methods
    void handle_param_request_read(const AP_Frsky_AGPILOTlite_Message &rxmsg);
    void handle_param_set(const AP_Frsky_AGPILOTlite_Message &rxmsg);

    void handle_command_long(const AP_Frsky_AGPILOTlite_Message &rxmsg);

    AGPILOT_RESULT handle_command(const mavlink_command_long_t &mav_command_long);
    AGPILOT_RESULT handle_command_preflight_calibration_baro(const mavlink_command_long_t &mav_command_long);
    AGPILOT_RESULT handle_command_do_set_mode(const mavlink_command_long_t &mav_command_long);
    AGPILOT_RESULT handle_command_do_fence_enable(const mavlink_command_long_t &mav_command_long);
    AGPILOT_RESULT handle_command_preflight_reboot(const mavlink_command_long_t &mav_command_long);

    void send_command_ack(const AGPILOT_RESULT mav_result, const uint16_t cmdid);

    send_mavlite_fn_t _send_fn;
};

#endif
