#include "GCS.h"

#include "AP_ServoRelayEvents/AP_ServoRelayEvents.h"

AGPILOT_RESULT GCS_AGPILOTLINK::handle_servorelay_message(const mavlink_command_long_t &packet)
{
    AP_ServoRelayEvents *handler = AP::servorelayevents();
    if (handler == nullptr) {
        return AGPILOT_RESULT_UNSUPPORTED;
    }

    AGPILOT_RESULT result = AGPILOT_RESULT_FAILED;

    switch (packet.command) {
    case AGPILOT_CMD_DO_SET_SERVO:
        if (handler->do_set_servo(packet.param1, packet.param2)) {
            result = AGPILOT_RESULT_ACCEPTED;
        }
        break;

    case AGPILOT_CMD_DO_SET_RELAY:
        if (handler->do_set_relay(packet.param1, packet.param2)) {
            result = AGPILOT_RESULT_ACCEPTED;
        }
        break;

    default:
        result = AGPILOT_RESULT_UNSUPPORTED;
        break;
    }

    return result;
}
