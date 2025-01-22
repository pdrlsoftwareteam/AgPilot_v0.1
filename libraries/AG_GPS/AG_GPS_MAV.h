/*
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

//
//  Mavlink GPS driver which accepts gps position data from an external
//  companion computer
//
#pragma once

#include <AG_HAL/AG_HAL_Boards.h>

#include "AG_GPS.h"
#include "GPS_Backend.h"

#ifndef AG_GPS_MAV_ENABLED
  #define AG_GPS_MAV_ENABLED AG_GPS_BACKEND_DEFAULT_ENABLED
#endif 

#if AG_GPS_MAV_ENABLED
class AG_GPS_MAV : public AG_GPS_Backend {
public:

    using AG_GPS_Backend::AG_GPS_Backend;

    bool read() override;

    static bool _detect(struct MAV_detect_state &state, uint8_t data);

    void handle_msg(const mavlink_message_t &msg) override;

    const char *name() const override { return "MAV"; }

private:
    bool _new_data;
    uint32_t first_week;
    JitterCorrection jitter{2000};
};
#endif
