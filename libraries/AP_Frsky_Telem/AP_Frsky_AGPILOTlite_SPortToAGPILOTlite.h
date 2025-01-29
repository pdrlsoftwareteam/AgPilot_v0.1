#pragma once

#include "AP_Frsky_AGPILOTlite_Message.h"
#include "AP_Frsky_SPort.h"

#include <stdint.h>

#if HAL_WITH_FRSKY_TELEM_BIDIRECTIONAL
/*
 * An instance of this class decodes a stream of SPort packets into a
 * AGPILOTlite message (see AP_Frsky_AGPILOTlite_Message.h).  It is expected
 * that the same rxmsg is passed into process() multiple times, each
 * time with a new sport packet.  If a packet is successfully decodes
 * then process() will return true and rxmsg can be used as a AGPILOTlite
 * message.
 *
 * See AP_Frsky_AGPILOTlite.h for a description of the encoding of a
 * AGPILOTlite message in SPort packets.
 */
class AP_Frsky_AGPILOTlite_SPortToAGPILOTlite {
public:

    bool process(AP_Frsky_AGPILOTlite_Message &rxmsg,
                 const AP_Frsky_SPort::sport_packet_t &packet) WARN_IF_UNUSED;

private:

    void reset();

    uint8_t expected_seq;
    uint8_t payload_next_byte;

    enum class State : uint8_t {
        IDLE=0,
        ERROR,
        WANT_LEN,
        WANT_MSGID,
        WANT_PAYLOAD,
        WANT_CHECKSUM,
        MESSAGE_RECEIVED,
    };
    State parse_state = State::IDLE;

    AP_Frsky_AGPILOTlite_Message _rxmsg;
    void parse(const uint8_t byte);

    int16_t checksum;                       // sent at end of packet
    void update_checksum(const uint8_t c);
};
#endif