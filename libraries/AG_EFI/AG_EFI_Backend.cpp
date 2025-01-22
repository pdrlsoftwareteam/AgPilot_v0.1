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

#include "AG_EFI.h"

#if HAL_EFI_ENABLED

#include "AG_EFI_Backend.h"

extern const AG_HAL::HAL &hal;

AG_EFI_Backend::AG_EFI_Backend(AG_EFI &_frontend) :
    frontend(_frontend)
{
}

void AG_EFI_Backend::copy_to_frontend() 
{
    WITH_SEMAPHORE(frontend.sem);
    frontend.state = internal_state;
}

float AG_EFI_Backend::get_coef1(void) const
{
    return frontend.coef1;
}

float AG_EFI_Backend::get_coef2(void) const
{
    return frontend.coef2;
}

HAL_Semaphore &AG_EFI_Backend::get_sem(void)
{
    return frontend.sem;
}

float AG_EFI_Backend::get_ecu_fuel_density(void) const
{
    return frontend.ecu_fuel_density;
}
#endif // HAL_EFI_ENABLED
