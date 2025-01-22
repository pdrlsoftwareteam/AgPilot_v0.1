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

/*
 * AG_EFI_Currawong_ECU.cpp
 *
 *      Author: Reilly Callaway / Currawong Engineering Pty Ltd
 */

#include "AG_EFI_Currawong_ECU.h"

#if AG_EFI_CURRAWONG_ECU_ENABLED

#include <AG_Param/AG_Param.h>
#include <AG_PiccoloCAN/piccolo_protocol/ECUPackets.h>
#include <AG_Math/definitions.h>

AG_EFI_Currawong_ECU* AG_EFI_Currawong_ECU::_singleton;

AG_EFI_Currawong_ECU::AG_EFI_Currawong_ECU(AG_EFI &_frontend) :
    AG_EFI_Backend(_frontend)
{
    _singleton = this;
    // Indicate that temperature and fuel pressure are supported
    internal_state.fuel_pressure_status = Fuel_Pressure_Status::OK;
    internal_state.temperature_status = Temperature_Status::OK;
}

void AG_EFI_Currawong_ECU::update()
{
    // copy the data to the front end
    copy_to_frontend();
}

bool AG_EFI_Currawong_ECU::handle_message(AG_HAL::CANFrame &frame)
{
    bool valid  = true;

    // There are differences between Ardupilot EFI_State and types/scaling of Piccolo packets.
    // First decode to Piccolo structs, and then store the data we need in internal_state with any scaling required.

    // Structs to decode Piccolo messages into
    ECU_TelemetryFast_t telemetry_fast;
    ECU_TelemetrySlow0_t telemetry_slow0;
    ECU_TelemetrySlow1_t telemetry_slow1;
    ECU_TelemetrySlow2_t telemetry_slow2;

    // Throw the message at the decoding functions
    if (decodeECU_TelemetryFastPacketStructure(&frame, &telemetry_fast)) {
        internal_state.throttle_position_percent = static_cast<uint8_t>(telemetry_fast.throttle);
        internal_state.engine_load_percent = static_cast<uint8_t>(telemetry_fast.throttle);
        internal_state.engine_speed_rpm = static_cast<uint32_t>(telemetry_fast.rpm);

        if (internal_state.engine_speed_rpm > 0) {
            internal_state.engine_state = Engine_State::RUNNING;
        } else {
            internal_state.engine_state = Engine_State::STOPPED;
        }

        // Prevent div by zero
        if (get_ecu_fuel_density() > 0.01) {
            internal_state.estimated_consumed_fuel_volume_cm3 = static_cast<float>(telemetry_fast.fuelUsed) / KG_PER_M3_TO_G_PER_CM3(get_ecu_fuel_density());
        } else {
            // If no (reasonable) density is provided
            internal_state.estimated_consumed_fuel_volume_cm3 = 0.;
        }

        internal_state.general_error = telemetry_fast.ecuStatusBits.errorIndicator;
        if (!telemetry_fast.ecuStatusBits.enabled) {
            internal_state.engine_state = Engine_State::STOPPED;
        }
    } else if (decodeECU_TelemetrySlow0PacketStructure(&frame, &telemetry_slow0)) {
        internal_state.intake_manifold_pressure_kpa = telemetry_slow0.map;
        internal_state.atmospheric_pressure_kpa = telemetry_slow0.baro;
        internal_state.cylinder_status.cylinder_head_temperature = C_TO_KELVIN(telemetry_slow0.cht);
    } else if (decodeECU_TelemetrySlow1PacketStructure(&frame, &telemetry_slow1)) {
        internal_state.intake_manifold_temperature = C_TO_KELVIN(telemetry_slow1.mat);
        internal_state.fuel_pressure = telemetry_slow1.fuelPressure;
    } else if (decodeECU_TelemetrySlow2PacketStructure(&frame, &telemetry_slow2)) {
        internal_state.cylinder_status.ignition_timing_deg = telemetry_slow2.ignAngle1;

        internal_state.fuel_consumption_rate_cm3pm = telemetry_slow2.flowRate / KG_PER_M3_TO_G_PER_CM3(get_ecu_fuel_density());
    } else {
        valid = false;
    }

    if (valid) {
        internal_state.last_updated_ms = AG_HAL::millis();
    }

    return valid;
}

#endif // AG_EFI_CURRAWONG_ECU_ENABLED
