#include <AG_HAL/AG_HAL.h>
#include <AG_HAL_Empty/AG_HAL_Empty.h>
#include <AG_Arming/AG_Arming.h>
#include <GCS_MAVLink/GCS_Dummy.h>
#include <AG_Vehicle/AG_Vehicle.h>
#include <AG_SerialManager/AG_SerialManager.h>
#include <AG_Logger/AG_Logger.h>
#include <AG_GyroFFT/AG_GyroFFT.h>
#include <AG_InertialSensor/AG_InertialSensor.h>
#include <AG_Baro/AG_Baro.h>
#include <AG_ExternalAHRS/AG_ExternalAHRS.h>
#include <AG_Scheduler/AG_Scheduler.h>
#include <AG_Arming/AG_Arming.h>
#include <SITL/SITL.h>

#if CONFIG_HAL_BOARD == HAL_BOARD_SITL
const AG_HAL::HAL &hal = AG_HAL::get_HAL();

static const uint32_t LOOP_RATE_HZ = 400;
static const uint32_t LOOP_DELTA_US = 1000000 / LOOP_RATE_HZ;
static const uint32_t RUN_TIME = 120;   // 2 mins
static const uint32_t LOOP_ITERATIONS = LOOP_RATE_HZ * RUN_TIME;

void setup();
void loop();

static AG_SerialManager serial_manager;
static AG_BoardConfig board_config;
static AG_InertialSensor ins;
static AG_Baro baro;
AP_Int32 logger_bitmask;
static AG_Logger logger{logger_bitmask};
#if HAL_EXTERNAL_AHRS_ENABLED
static AG_ExternalAHRS external_ahrs;
#endif
static SITL::SIM sitl;
static AG_Scheduler scheduler;

// create fake gcs object
GCS_Dummy _gcs;

const struct LogStructure log_structure[] = {
    LOG_COMMON_STRUCTURES
};

const AG_Param::GroupInfo GCS_MAVLINK_Parameters::var_info[] = {
    AP_GROUPEND
};

class Arming : public AG_Arming {
public:
    Arming() : AG_Arming() {}
    bool arm(AG_Arming::Method method, bool do_arming_checks=true) override {
        armed = true;
        return true;
    }
};

static Arming arming;

class ReplayGyroFFT {
public:
    void init() {
        fft._enable.set(1);             // FFT_ENABLE
        fft._window_size.set(64);       // FFT_WINDOW_SIZE
        fft._snr_threshold_db.set(10);  // FFT_SNR_REF
        fft._fft_min_hz.set(50);        // FFT_MINHZ
        fft._fft_max_hz.set(450);       // FFT_MAXHZ

        fft.init(LOOP_RATE_HZ);
        fft.update_parameters();
    }

    void loop() {
        fft.sample_gyros();
        fft.update();
        // calibrate the FFT
        uint32_t now = AG_HAL::millis();
        if (!arming.is_armed()) {
            char buf[32];
            if (!fft.pre_arm_check(buf, 32)) {
                if (now - last_output_ms > 1000) {
                    hal.console->printf("%s\n", buf);
                    last_output_ms = now;
                }
            } else {
                logger.PrepForArming();
                arming.arm(AG_Arming::Method::RUDDER);
                logger.set_vehicle_armed(true);
                // apply throttle values to motors to make sure the fake IMU generates energetic enough data
                for (uint8_t i=0; i<4; i++) {
                    hal.rcout->write(i, 1500);
                }
            }
        } else {
            if (now - last_output_ms > 1000) {
                hal.console->printf(".");
                last_output_ms = now;
            }
        }
        fft.write_log_messages();
    }
    AG_GyroFFT fft;
    uint32_t last_output_ms;
};

static ReplayGyroFFT replay;

void setup()
{
    hal.console->printf("ReplayGyroFFT\n");
    board_config.init();   
    serial_manager.init();

    const bool generate = false;
    if (generate) {
        sitl.vibe_freq.set(Vector3f(250,250,250));  // SIM_VIB_FREQ
        sitl.drift_speed.set(0);    // SIM_DRIFT_SPEED
        sitl.drift_time.set(0);     // SIM_DRIFT_TIME
        sitl.gyro_noise[0].set(20); // SIM_GYR1_RND
    } else {
        sitl.speedup.set(100);      // SIM_SPEEDUP
        sitl.gyro_file_rw.set(SITL::SIM::INSFileMode::INS_FILE_READ_STOP_ON_EOF);   // SIM_GYR_FILE_RW
    }
    logger_bitmask.set(128);    // IMU
    logger.Init(log_structure, ARRAY_SIZE(log_structure));
    ins.init(LOOP_RATE_HZ);
    baro.init();

    replay.init();
}

static uint32_t loop_iter = LOOP_ITERATIONS;

void loop()
{
    if (!hal.console->is_initialized()) {
        return;
    }

    ins.wait_for_sample();
    uint32_t sample_time_us = AG_HAL::micros();

    ins.update();
    ins.periodic();
    logger.periodic_tasks();
    ins.Write_IMU();
    replay.loop();

    uint32_t elapsed = AG_HAL::micros() - sample_time_us;
    if (elapsed < LOOP_DELTA_US) {
        hal.scheduler->delay_microseconds(LOOP_DELTA_US - elapsed);
    }

    if (sitl.gyro_file_rw != SITL::SIM::INSFileMode::INS_FILE_READ_STOP_ON_EOF && loop_iter-- == 0) {
        hal.console->printf("\n");
        exit(0);
    }
}

AG_HAL_MAIN();

#else

#include <stdio.h>

const AG_HAL::HAL& hal = AG_HAL::get_HAL();

static void loop() { }
static void setup()
{
    printf("Board not currently supported\n");
}

AG_HAL_MAIN();

#endif
