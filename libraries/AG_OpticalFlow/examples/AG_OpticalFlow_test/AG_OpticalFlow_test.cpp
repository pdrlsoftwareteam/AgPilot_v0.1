/*
 *  Example of AG_OpticalFlow library.
 *  Code by Randy Mackay. DIYDrones.com
 */

#include <AG_AHRS/AG_AHRS.h>
#include <AG_Baro/AG_Baro.h>
#include <AG_Compass/AG_Compass.h>
#include <AG_GPS/AG_GPS.h>
#include <AG_HAL/AG_HAL.h>
#include <AG_InertialSensor/AG_InertialSensor.h>
#include <AG_NavEKF2/AG_NavEKF2.h>
#include <AG_NavEKF3/AG_NavEKF3.h>
#include <AG_OpticalFlow/AG_OpticalFlow.h>
#include <AG_RangeFinder/AG_RangeFinder.h>
#include <AG_SerialManager/AG_SerialManager.h>

void setup();
void loop();

const AG_HAL::HAL& hal = AG_HAL::get_HAL();

class DummyVehicle {
public:
    AG_GPS gps;
    AG_Baro barometer;
    Compass compass;
    AG_InertialSensor ins;
    AG_SerialManager serial_manager;
    RangeFinder sonar;
    AG_AHRS ahrs{AG_AHRS::FLAG_ALWAYS_USE_EKF};
};

static DummyVehicle vehicle;
#if AP_OPTICALFLOW_ENABLED
static AG_OpticalFlow optflow;
#endif

void setup()
{
    hal.console->printf("OpticalFlow library test ver 1.6\n");

    hal.scheduler->delay(1000);

#if AP_OPTICALFLOW_ENABLED
    // flowSensor initialization
    optflow.init(-1);

    if (!optflow.healthy()) {
        hal.console->printf("Failed to initialise OpticalFlow");
    }
#else
    hal.console->printf("OpticalFlow compiled out");
#endif

    hal.scheduler->delay(1000);
}

void loop()
{
    hal.console->printf("this only tests compilation succeeds\n");

    hal.scheduler->delay(5000);
}

AG_HAL_MAIN();
