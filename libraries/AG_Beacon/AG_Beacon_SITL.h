#pragma once

#include "AG_Beacon_Backend.h"

#if AP_BEACON_SITL_ENABLED

#include <SITL/SITL.h>

class AG_Beacon_SITL : public AG_Beacon_Backend
{

public:
    // constructor
    AG_Beacon_SITL(AG_Beacon &frontend);

    // return true if sensor is basically healthy (we are receiving data)
    bool healthy() override;

    // update
    void update() override;

private:
    SITL::SIM *sitl;
    uint8_t next_beacon;
    uint32_t last_update_ms;
};

#endif // AP_BEACON_SITL_ENABLED
