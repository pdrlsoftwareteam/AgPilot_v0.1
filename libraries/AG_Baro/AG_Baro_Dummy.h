/*
  dummy backend barometer. Used during board bringup. Selected using
  BARO line in hwdef.dat
 */
#pragma once

#include "AG_Baro_Backend.h"

#if AP_BARO_DUMMY_ENABLED

class AG_Baro_Dummy : public AG_Baro_Backend
{
public:
    AG_Baro_Dummy(AG_Baro &baro);
    void update(void) override;
    static AG_Baro_Backend *probe(AG_Baro &baro) {
        return new AG_Baro_Dummy(baro);
    }

private:
    uint8_t _instance;
};

#endif  // AP_BARO_DUMMY_ENABLED
