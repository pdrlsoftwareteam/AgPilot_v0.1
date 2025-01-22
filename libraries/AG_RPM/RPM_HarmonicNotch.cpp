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

#include "RPM_HarmonicNotch.h"

#if AG_RPM_HARMONICNOTCH_ENABLED

#include <AG_HAL/AG_HAL.h>
#include <AG_InertialSensor/AG_InertialSensor.h>

AG_RPM_HarmonicNotch::AG_RPM_HarmonicNotch(AG_RPM &_ap_rpm, uint8_t _instance, AG_RPM::RPM_State &_state) :
    AG_RPM_Backend(_ap_rpm, _instance, _state)
{
    instance = _instance;
}

void AG_RPM_HarmonicNotch::update(void)
{
    const AG_InertialSensor& ins = AP::ins();
    for (const auto &notch : ins.harmonic_notches) {
        if (notch.params.enabled() &&
            notch.params.tracking_mode() != HarmonicNotchDynamicMode::Fixed) {
            state.rate_rpm = notch.calculated_notch_freq_hz[0] * 60;
            state.rate_rpm *= ap_rpm._params[state.instance].scaling;
            state.signal_quality = 0.5f;
            state.last_reading_ms = AG_HAL::millis();
        }
    }
}

#endif  // AG_RPM_HARMONICNOTCH_ENABLED
