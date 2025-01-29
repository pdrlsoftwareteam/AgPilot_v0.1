#pragma once

#include <AP_Rally/AP_Rally.h>

#if HAL_RALLY_ENABLED

#include "MissionItemProtocol.h"

class MissionItemProtocol_Rally : public MissionItemProtocol {
public:
    MissionItemProtocol_Rally(class AP_Rally &_rally) :
        rally(_rally) {}
    void truncate(const mavlink_mission_count_t &packet) override;
    AGPILOT_MISSION_TYPE mission_type() const override { return AGPILOT_MISSION_TYPE_RALLY; }

    AGPILOT_MISSION_RESULT complete(const GCS_AGPILOTLINK &_link) override;
    void timeout() override;

    /*
      static function to get rally item as mavlink_mission_item_int_t
    */
    static bool get_item_as_mission_item(uint16_t seq, mavlink_mission_item_int_t &ret_packet);
    
protected:

    ap_message next_item_ap_message_id() const override {
        return MSG_NEXT_MISSION_REQUEST_RALLY;
    }
    bool clear_all_items() override WARN_IF_UNUSED;

private:
    AP_Rally &rally;

    uint16_t item_count() const override;
    uint16_t max_items() const override;

    AGPILOT_MISSION_RESULT replace_item(const mavlink_mission_item_int_t&) override WARN_IF_UNUSED;
    AGPILOT_MISSION_RESULT append_item(const mavlink_mission_item_int_t&) override WARN_IF_UNUSED;

    AGPILOT_MISSION_RESULT get_item(const GCS_AGPILOTLINK &_link,
                                const mavlink_message_t &msg,
                                const mavlink_mission_request_int_t &packet,
                                mavlink_mission_item_int_t &ret_packet) override WARN_IF_UNUSED;

    static AGPILOT_MISSION_RESULT convert_MISSION_ITEM_INT_to_RallyLocation(const mavlink_mission_item_int_t &cmd, class RallyLocation &ret) WARN_IF_UNUSED;

};

#endif  // HAL_RALLY_ENABLED
