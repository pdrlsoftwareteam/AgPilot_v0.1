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

#include "AG_RPM.h"
#include "RPM_Backend.h"
#include <AG_Generator/AG_Generator.h>

#if AG_RPM_GENERATOR_ENABLED

class AG_RPM_Generator : public AG_RPM_Backend
{
public:
    // constructor
    using AG_RPM_Backend::AG_RPM_Backend;

    // update state
    void update(void) override;
};

#endif // AG_RPM_GENERATOR_ENABLED
