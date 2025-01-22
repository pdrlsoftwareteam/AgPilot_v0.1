#pragma once

#include <AG_AHRS/AG_AHRS.h>

/*
  compass learning using magnetic field tables from AG_Declination and GSF
 */

class CompassLearn {
public:
    CompassLearn(Compass &compass);

    // called on each compass read
    void update(void);

private:
    Compass &compass;
};
