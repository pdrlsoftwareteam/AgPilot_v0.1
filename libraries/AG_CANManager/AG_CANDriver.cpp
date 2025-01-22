/*
 * This program is free software: you can redistribute it and/or modify
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

#if HAL_MAX_CAN_PROTOCOL_DRIVERS && HAL_CANMANAGER_ENABLED
#include "AG_CANManager.h"
#include <AG_Vehicle/AG_Vehicle_Type.h>

#include <AG_UAVCAN/AG_UAVCAN.h>
#include <AG_PiccoloCAN/AG_PiccoloCAN.h>
#include "AG_CANTester.h"
#include <AG_KDECAN/AG_KDECAN.h>


// table of user settable CAN bus parameters
const AG_Param::GroupInfo AG_CANManager::CANDriver_Params::var_info[] = {

    // @Param: PROTOCOL
    // @DisplayName: Enable use of specific protocol over virtual driver
    // @Description: Enabling this option starts selected protocol that will use this virtual driver
    // @Values: 0:Disabled,1:DroneCAN,4:PiccoloCAN,5:CANTester,6:EFI_NWPMU,7:USD1,8:KDECAN,10:Scripting,11:Benewake,12:Scripting2
    // @User: Advanced
    // @RebootRequired: True
    AP_GROUPINFO("PROTOCOL", 1, AG_CANManager::CANDriver_Params, _driver_type, AG_CANManager::Driver_Type_UAVCAN),

    // @Group: UC_
    // @Path: ../AG_UAVCAN/AG_UAVCAN.cpp
    AP_SUBGROUPPTR(_uavcan, "UC_", 2, AG_CANManager::CANDriver_Params, AG_UAVCAN),

#if (APM_BUILD_COPTER_OR_HELI || APM_BUILD_TYPE(APM_BUILD_ArduPlane) || APM_BUILD_TYPE(APM_BUILD_ArduSub))
    // @Group: KDE_
    // @Path: ../AG_KDECAN/AG_KDECAN.cpp
    AP_SUBGROUPPTR(_kdecan, "KDE_", 3, AG_CANManager::CANDriver_Params, AG_KDECAN),
#endif

#if HAL_NUM_CAN_IFACES > 1 && !HAL_MINIMIZE_FEATURES && HAL_ENABLE_CANTESTER
    // @Group: TST_
    // @Path: ../AG_CANManager/AG_CANTester.cpp
    AP_SUBGROUPPTR(_testcan, "TST_", 4, AG_CANManager::CANDriver_Params, CANTester),
#endif

#if HAL_PICCOLO_CAN_ENABLE
    // @Group: PC_
    // @Path: ../AG_PiccoloCAN/AG_PiccoloCAN.cpp
    AP_SUBGROUPPTR(_piccolocan, "PC_", 5, AG_CANManager::CANDriver_Params, AG_PiccoloCAN),
#endif

    AP_GROUPEND
};
#endif
