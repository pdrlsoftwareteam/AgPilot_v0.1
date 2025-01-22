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
 * AG_EFI_Currawong_ECU.h
 *
 *      Author: Reilly Callaway / Currawong Engineering Pty Ltd
 */
 
#pragma once

#include "AG_EFI_config.h"

#if AG_EFI_CURRAWONG_ECU_ENABLED

#include "AG_EFI.h"
#include "AG_EFI_Backend.h"

class AG_EFI_Currawong_ECU : public AG_EFI_Backend {
public:
    AG_EFI_Currawong_ECU(AG_EFI &_frontend);
    
    void update() override;

    static AG_EFI_Currawong_ECU* get_instance(void)
    {
        return _singleton;
    }

private:
    bool handle_message(AG_HAL::CANFrame &frame);

    static AG_EFI_Currawong_ECU* _singleton;

    friend class AG_PiccoloCAN;
};

#endif // AG_EFI_CURRAWONG_ECU_ENABLED

