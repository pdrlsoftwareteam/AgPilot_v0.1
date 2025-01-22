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

#include <AG_OpticalFlow/AG_OpticalFlow.h>

#ifndef AP_OPTICALFLOW_ONBOARD_ENABLED
#define AP_OPTICALFLOW_ONBOARD_ENABLED AP_OPTICALFLOW_ENABLED
#endif

#if AP_OPTICALFLOW_ONBOARD_ENABLED

#include <AG_AHRS/AG_AHRS.h>
#include <AG_Math/AG_Math.h>
#include <AG_NavEKF2/AG_NavEKF2.h>
#include <AG_NavEKF3/AG_NavEKF3.h>

#include "AG_OpticalFlow.h"

class AG_OpticalFlow_Onboard : public OpticalFlow_backend
{
public:

    using OpticalFlow_backend::OpticalFlow_backend;

    void init(void) override;
    void update(void) override;
private:
    uint32_t _last_read_ms;
};

#endif  // AP_OPTICALFLOW_ONBOARD_ENABLED
