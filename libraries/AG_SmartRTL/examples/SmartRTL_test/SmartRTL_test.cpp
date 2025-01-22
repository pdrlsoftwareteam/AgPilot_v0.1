#include "SmartRTL_test.h"
#include <AG_AHRS/AG_AHRS.h>
#include <AG_Baro/AG_Baro.h>
#include <AG_Compass/AG_Compass.h>
#include <AG_GPS/AG_GPS.h>
#include <AG_InertialSensor/AG_InertialSensor.h>
#include <AG_NavEKF2/AG_NavEKF2.h>
#include <AG_NavEKF3/AG_NavEKF3.h>
#include <AG_RangeFinder/AG_RangeFinder.h>
#include <AG_SerialManager/AG_SerialManager.h>

const AG_HAL::HAL &hal = AG_HAL::get_HAL();

// INS and Baro declaration
static AG_InertialSensor ins;
static Compass compass;
static AG_GPS gps;
static AG_Baro barometer;
static AG_SerialManager serial_manager;

class DummyVehicle {
public:
    AG_AHRS ahrs{AG_AHRS::FLAG_ALWAYS_USE_EKF};
};

static DummyVehicle vehicle;

AG_AHRS &ahrs(vehicle.ahrs);
AG_SmartRTL smart_rtl{true};
AG_BoardConfig board_config;

void setup();
void loop();
void reset();
void check_path(const std::vector<Vector3f> &correct_path, const char* test_name, uint32_t time_us);

void setup()
{
    hal.console->printf("SmartRTL test\n");
    board_config.init();
    smart_rtl.init();
}

void loop()
{
    if (!hal.console->is_initialized()) {
        return;
    }
    uint32_t reference_time, run_time;

    hal.console->printf("--------------------\n");

    // reset path and upload "test_path_before" to smart_rtl
    reference_time = AG_HAL::micros();
    reset();
    run_time = AG_HAL::micros() - reference_time;

    // check path after initial load (no simplification or pruning)
    check_path(test_path_after_adding, "append", run_time);

    // test simplifications
    reference_time = AG_HAL::micros();
    while (!smart_rtl.request_thorough_cleanup(AG_SmartRTL::THOROUGH_CLEAN_SIMPLIFY_ONLY)) {
        smart_rtl.run_background_cleanup();
    }
    run_time = AG_HAL::micros() - reference_time;
    check_path(test_path_after_simplifying, "simplify", run_time);

    // test both simplification and pruning
    hal.scheduler->delay(5);    // delay 5 milliseconds because request_through_cleanup uses millisecond timestamps
    reset();
    reference_time = AG_HAL::micros();
    while (!smart_rtl.request_thorough_cleanup(AG_SmartRTL::THOROUGH_CLEAN_ALL)) {
        smart_rtl.run_background_cleanup();
    }
    run_time = AG_HAL::micros() - reference_time;
    check_path(test_path_complete, "simplify and pruning", run_time);

    // delay before next display
    hal.scheduler->delay(5e3); // 5 seconds
}

// reset path (i.e. clear path and add home) and upload "test_path_before" to smart_rtl
void reset()
{
    smart_rtl.set_home(true, Vector3f{0.0f, 0.0f, 0.0f});
    for (Vector3f v : test_path_before) {
        smart_rtl.update(true, v);
    }
}

// compare the vector array passed in with the path held in the smart_rtl object
void check_path(const std::vector<Vector3f>& correct_path, const char* test_name, uint32_t time_us)
{
    // check number of points
    bool num_points_match = correct_path.size() == smart_rtl.get_num_points();
    uint16_t points_to_compare = MIN(correct_path.size(), smart_rtl.get_num_points());

    // check all points match
    bool points_match = true;
    uint16_t failure_index = 0;
    for (uint16_t i = 0; i < points_to_compare; i++) {
        if (smart_rtl.get_point(i) != correct_path[i]) {
            failure_index = i;
            points_match = false;
        }
    }

    // display overall results
    hal.console->printf("%s: %s time:%u us\n", test_name, (num_points_match && points_match) ? "success" : "fail", (unsigned)time_us);

    // display number of points
    hal.console->printf("   expected %u points, got %u\n", (unsigned)correct_path.size(), (unsigned)smart_rtl.get_num_points());

    // display the first failed point and all subsequent points
    if (!points_match) {
        for (uint16_t j = failure_index; j < points_to_compare; j++) {
            const Vector3f& smartrtl_point = smart_rtl.get_point(j);
            hal.console->printf("   expected point %d to be %4.2f,%4.2f,%4.2f, got %4.2f,%4.2f,%4.2f\n",
                            (int)j,
                            (double)correct_path[j].x,
                            (double)correct_path[j].y,
                            (double)correct_path[j].z,
                            (double)smartrtl_point.x,
                            (double)smartrtl_point.y,
                            (double)smartrtl_point.z
                            );
        }
    }
}

// gcc9 produces a large frame
#pragma GCC diagnostic ignored "-Wframe-larger-than="

AG_HAL_MAIN();
