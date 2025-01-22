//
// Simple test for the AG_AHRS NMEA output
//

#include <AG_AHRS/AG_AHRS.h>
#include <AG_HAL/AG_HAL.h>
#include <AG_Param/AG_Param.h>
#include <AG_BoardConfig/AG_BoardConfig.h>
#include <GCS_MAVLink/GCS_Dummy.h>
#include <AG_RangeFinder/AG_RangeFinder.h>
#include <AG_Logger/AG_Logger.h>
#include <AG_GPS/AG_GPS.h>
#include <AG_Baro/AG_Baro.h>
#include <AG_NMEA_Output/AG_NMEA_Output.h>
#include <AG_SerialManager/AG_SerialManager.h>
#include <AG_Vehicle/AG_Vehicle.h>

void setup();
void loop();

const AG_HAL::HAL& hal = AG_HAL::get_HAL();



class Parameters {
public:

    enum {
        k_param_serial_manager = 1, // serial manager library
    };
};

static AG_SerialManager serial_manager;

const struct AG_Param::Info var_info[] = {
    { "SERIAL", (const void *)&serial_manager, {group_info : AG_SerialManager::var_info}, 0, Parameters::k_param_serial_manager, AP_PARAM_GROUP },
    AP_VAREND
};


static AG_Param param{var_info};


AP_Int32 logger_bitmask;
static AG_Logger logger{logger_bitmask};

class DummyVehicle : public AG_Vehicle {
public:
    AG_AHRS ahrs{AG_AHRS::FLAG_ALWAYS_USE_EKF};
    bool set_mode(const uint8_t new_mode, const ModeReason reason) override { return true; };
    uint8_t get_mode() const override { return 1; };
    void get_scheduler_tasks(const AG_Scheduler::Task *&tasks, uint8_t &task_count, uint32_t &log_bit) override {};
    void init_ardupilot() override {};
    void load_parameters() override {};
    void init() {
        BoardConfig.init();
    }
};

static DummyVehicle vehicle;

void setup(void)
{
    vehicle.init();
    if (!AG_Param::setup()) {
        hal.console->printf("Failed to call setup\n");
        while(true);
    }
    if (!AG_Param::set_by_name("SERIAL0_PROTOCOL", AG_SerialManager::SerialProtocol_NMEAOutput)) {
        hal.console->printf("Failed to set SERIAL0_PROTOCOL\n");
        while(true);
    }
    AP::ins().init(100);
    serial_manager.init_console();
    serial_manager.init();
    vehicle.ahrs.init();

    AP::compass().init();
    if(!AP::compass().read()) {
        hal.console->printf("No compass detected\n");
    }
    AP::gps().init(serial_manager);
    AP::rtc().set_utc_usec(1546300800000, AG_RTC::source_type::SOURCE_GPS);
}

void loop(void)
{
    static uint32_t last_compass;
    const uint32_t now = AG_HAL::micros();

    // read compass at 10Hz
    if (now - last_compass > 100 * 1000UL &&
        AP::compass().read()) {
        last_compass = now;
    }

    vehicle.ahrs.update();
}

const AG_Param::GroupInfo GCS_MAVLINK_Parameters::var_info[] = {
        AP_GROUPEND
};
GCS_Dummy _gcs;

AG_HAL_MAIN();
