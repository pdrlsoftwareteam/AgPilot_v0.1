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
#pragma once

#include "AG_Frsky_Telem.h"

#if HAL_WITH_FRSKY_TELEM_BIDIRECTIONAL
#include <AG_HAL/AG_HAL.h>
#include <AG_Param/AG_Param.h>

class AG_Frsky_Telem;

class AG_Frsky_Parameters
{
    friend class AG_Frsky_SPort_Passthrough;
public:
    AG_Frsky_Parameters();

    // parameters
    static const struct AG_Param::GroupInfo var_info[];

private:
    // settable parameters
    AP_Int8 _uplink_id;
    AP_Int8 _dnlink_id;
    AP_Int8 _dnlink1_id;
    AP_Int8 _dnlink2_id;
    AP_Int8 _options;
};

#endif //HAL_WITH_FRSKY_TELEM_BIDIRECTIONAL
