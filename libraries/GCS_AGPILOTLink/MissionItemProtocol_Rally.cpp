/*
  Implementation details for transfering rally point information using
  the MISSION_ITEM protocol to and from a GCS.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "MissionItemProtocol_Rally.h"

#include <AP_Logger/AP_Logger.h>
#include <AP_Rally/AP_Rally.h>
#include <GCS_AGPILOTLink/GCS.h>

#if HAL_RALLY_ENABLED

AGPILOT_MISSION_RESULT MissionItemProtocol_Rally::append_item(const mavlink_mission_item_int_t &cmd)
{
    RallyLocation rallyloc;
    const AGPILOT_MISSION_RESULT ret = convert_MISSION_ITEM_INT_to_RallyLocation(cmd, rallyloc);
    if (ret != AGPILOT_MISSION_ACCEPTED) {
        return ret;
    }
    if (!rally.append(rallyloc)) {
        return AGPILOT_MISSION_ERROR;
    }
    return AGPILOT_MISSION_ACCEPTED;
}

AGPILOT_MISSION_RESULT MissionItemProtocol_Rally::complete(const GCS_AGPILOTLINK &_link)
{
    AP::logger().Write_Rally();
    return AGPILOT_MISSION_ACCEPTED;
}

bool MissionItemProtocol_Rally::clear_all_items()
{
    rally.truncate(0);
    return true;
}

AGPILOT_MISSION_RESULT MissionItemProtocol_Rally::convert_MISSION_ITEM_INT_to_RallyLocation(const mavlink_mission_item_int_t &cmd, RallyLocation &ret)
{
    if (cmd.command != AGPILOT_CMD_NAV_RALLY_POINT) {
        return AGPILOT_MISSION_UNSUPPORTED;
    }
    if (cmd.frame != AGPILOT_FRAME_GLOBAL_RELATIVE_ALT_INT &&
        cmd.frame != AGPILOT_FRAME_GLOBAL_RELATIVE_ALT) {
        return AGPILOT_MISSION_UNSUPPORTED_FRAME;
    }
    if (!check_lat(cmd.x)) {
        return AGPILOT_MISSION_INVALID_PARAM5_X;
    }
    if (!check_lng(cmd.y)) {
        return AGPILOT_MISSION_INVALID_PARAM6_Y;
    }
    if (cmd.z < INT16_MIN || cmd.z > INT16_MAX) {
        return AGPILOT_MISSION_INVALID_PARAM7;
    }
    ret = {};
    ret.lat = cmd.x;
    ret.lng = cmd.y;
    ret.alt = cmd.z;
    return AGPILOT_MISSION_ACCEPTED;
}

/*
  static function to get rally item as mavlink_mission_item_int_t
 */
bool MissionItemProtocol_Rally::get_item_as_mission_item(uint16_t seq,
                                                         mavlink_mission_item_int_t &ret_packet)
{
    auto *rallyp = AP::rally();

    RallyLocation rallypoint;

    if (rallyp == nullptr || !rallyp->get_rally_point_with_index(seq, rallypoint)) {
        return false;
    }

    ret_packet.frame = AGPILOT_FRAME_GLOBAL_RELATIVE_ALT;
    ret_packet.command = AGPILOT_CMD_NAV_RALLY_POINT;
    ret_packet.x = rallypoint.lat;
    ret_packet.y = rallypoint.lng;
    ret_packet.z = rallypoint.alt;

    return true;
}

AGPILOT_MISSION_RESULT MissionItemProtocol_Rally::get_item(const GCS_AGPILOTLINK &_link,
                                                       const mavlink_message_t &msg,
                                                       const mavlink_mission_request_int_t &packet,
                                                       mavlink_mission_item_int_t &ret_packet)
{
    if (!get_item_as_mission_item(packet.seq, ret_packet)) {
        return AGPILOT_MISSION_INVALID_SEQUENCE;
    }
    return AGPILOT_MISSION_ACCEPTED;
}

uint16_t MissionItemProtocol_Rally::item_count() const {
    return rally.get_rally_total();
}

uint16_t MissionItemProtocol_Rally::max_items() const {
    return rally.get_rally_max();
}

AGPILOT_MISSION_RESULT MissionItemProtocol_Rally::replace_item(const mavlink_mission_item_int_t &cmd)
{
    RallyLocation rallyloc;
    const AGPILOT_MISSION_RESULT ret = convert_MISSION_ITEM_INT_to_RallyLocation(cmd, rallyloc);
    if (ret != AGPILOT_MISSION_ACCEPTED) {
        return ret;
    }
    if (!rally.set_rally_point_with_index(cmd.seq, rallyloc)) {
        return AGPILOT_MISSION_ERROR;
    }
    return AGPILOT_MISSION_ACCEPTED;
}

void MissionItemProtocol_Rally::timeout()
{
    link->send_text(AGPILOT_SEVERITY_WARNING, "Rally upload timeout");
}

void MissionItemProtocol_Rally::truncate(const mavlink_mission_count_t &packet)
{
    rally.truncate(packet.count);
}

#endif  // HAL_RALLY_ENABLED
