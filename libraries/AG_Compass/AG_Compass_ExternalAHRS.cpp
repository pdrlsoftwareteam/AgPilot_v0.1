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

#include <AG_HAL/AG_HAL.h>
#include "AG_Compass_ExternalAHRS.h"

#if AP_COMPASS_EXTERNALAHRS_ENABLED

AG_Compass_ExternalAHRS::AG_Compass_ExternalAHRS(uint8_t port)
{
    auto devid = AG_HAL::Device::make_bus_id(AG_HAL::Device::BUS_TYPE_SERIAL,port,0,0);
    register_compass(devid, instance);

    set_dev_id(instance, devid);
    set_external(instance, true);
}

void AG_Compass_ExternalAHRS::handle_external(const AG_ExternalAHRS::mag_data_message_t &pkt)
{
    Vector3f field = pkt.field;
    accumulate_sample(field, instance);
}

void AG_Compass_ExternalAHRS::read(void)
{
    drain_accumulated_samples(instance);
}

#endif // AP_COMPASS_EXTERNALAHRS_ENABLED
