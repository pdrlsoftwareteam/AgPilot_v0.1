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

#include <AG_Rally/AG_Rally.h>
#include <AG_AHRS/AG_AHRS.h>

class AG_Rally_Copter : public AG_Rally
{
public:
    AG_Rally_Copter() : AG_Rally() { }

    /* Do not allow copies */
    CLASS_NO_COPY(AG_Rally_Copter);

private:
    bool is_valid(const Location &rally_point) const override;
};
