#include "AG_Baro_Dummy.h"

#if AP_BARO_DUMMY_ENABLED

AG_Baro_Dummy::AG_Baro_Dummy(AG_Baro &baro) :
    AG_Baro_Backend(baro)
{
    _instance = _frontend.register_sensor();
}

// Read the sensor
void AG_Baro_Dummy::update(void)
{
    _copy_to_frontend(0, 91300, 21);
}

#endif  // AP_BARO_DUMMY_ENABLED
