/*
 *  RangeFinder test code
 */

#include <AG_HAL/AG_HAL.h>
#include <AG_RangeFinder/AG_RangeFinder_Backend.h>
#include <GCS_MAVLink/GCS_Dummy.h>

const struct AG_Param::GroupInfo        GCS_MAVLINK_Parameters::var_info[] = {
    AP_GROUPEND
};
GCS_Dummy _gcs;

void setup();
void loop();

const AG_HAL::HAL& hal = AG_HAL::get_HAL();

static AG_SerialManager serial_manager;
static RangeFinder sonar;

void setup()
{
    // print welcome message
    hal.console->printf("Range Finder library test\n");

    // setup for analog pin 13
    AG_Param::set_object_value(&sonar, sonar.var_info, "_PIN", -1.0f);
    AG_Param::set_object_value(&sonar, sonar.var_info, "_SCALING", 1.0f);

    // initialise sensor, delaying to make debug easier
    hal.scheduler->delay(2000);
    sonar.init(ROTATION_PITCH_270);
    hal.console->printf("RangeFinder: %d devices detected\n", sonar.num_sensors());
}

void loop()
{
    // Delay between reads
    hal.scheduler->delay(100);
    sonar.update();

    bool had_data = false;
    for (uint8_t i=0; i<sonar.num_sensors(); i++) {
        AG_RangeFinder_Backend *sensor = sonar.get_backend(i);
        if (sensor == nullptr) {
            continue;
        }
        if (!sensor->has_data()) {
            continue;
        }
        hal.console->printf("All: device_%u type %d status %d distance_cm %d\n",
                            i,
                            (int)sensor->type(),
                            (int)sensor->status(),
                            sensor->distance_cm());
        had_data = true;
    }
    if (!had_data) {
        hal.console->printf("All: no data on any sensor\n");
    }

}
AG_HAL_MAIN();
