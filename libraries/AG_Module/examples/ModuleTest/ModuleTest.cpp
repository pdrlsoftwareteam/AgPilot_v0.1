//
// Simple test for the AG_AHRS interface
//

#include <AG_AHRS/AG_AHRS.h>
#include <AG_HAL/AG_HAL.h>
#include <AG_Module/AG_Module.h>
#include <AG_GPS/AG_GPS.h>
#include <AG_Baro/AG_Baro.h>
#include <AG_ExternalAHRS/AG_ExternalAHRS.h>
#include <GCS_MAVLink/GCS_Dummy.h>

const struct AG_Param::GroupInfo        GCS_MAVLINK_Parameters::var_info[] = {
    AP_GROUPEND
};
GCS_Dummy _gcs;

void setup();
void loop();

const AG_HAL::HAL& hal = AG_HAL::get_HAL();

// sensor declaration
static AG_InertialSensor ins;
#if HAL_EXTERNAL_AHRS_ENABLED
 static AG_ExternalAHRS eAHRS;
#endif // HAL_EXTERNAL_AHRS_ENABLED
static AG_GPS gps;
static AG_Baro baro;
static AG_SerialManager serial_manager;

// choose which AHRS system to use
static AG_AHRS ahrs{};

void setup(void)
{
    serial_manager.init();
    ins.init(100);
    baro.init();
    ahrs.init();

    gps.init(serial_manager);
}

void loop(void)
{
    ahrs.update();
}

AG_HAL_MAIN();
