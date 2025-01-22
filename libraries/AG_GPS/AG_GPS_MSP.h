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
//  MSP GPS driver which accepts gps position data from an external source
//
#pragma once

#include <AG_Common/AG_Common.h>
#include <AG_HAL/AG_HAL.h>

#include "AG_GPS.h"
#include "GPS_Backend.h"

#if HAL_MSP_GPS_ENABLED

class AG_GPS_MSP : public AG_GPS_Backend
{
public:

    using AG_GPS_Backend::AG_GPS_Backend;

    bool read() override;
    void handle_msp(const MSP::msp_gps_data_message_t &pkt) override;

    const char *name() const override { return "MSP"; }

    bool get_lag(float &lag_sec) const override;

private:
    bool new_data;
};

#endif // HAL_MSP_GPS_ENABLED
