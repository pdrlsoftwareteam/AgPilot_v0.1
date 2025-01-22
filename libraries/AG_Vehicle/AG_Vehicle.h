/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

/*
  this header holds a parameter structure for each vehicle type for
  parameters needed by multiple libraries
 */

#include "ModeReason.h" // reasons can't be defined in this header due to circular loops

#include <AG_AHRS/AG_AHRS.h>
#include <AG_Airspeed/AG_Airspeed.h>
#include <AG_Baro/AG_Baro.h>
#include <AG_BoardConfig/AG_BoardConfig.h>     // board configuration library
#include <AG_CANManager/AG_CANManager.h>
#include <AG_Button/AG_Button.h>
#include <AG_Compass/AG_Compass.h>
#include <AG_EFI/AG_EFI.h>
#include <AG_GPS/AG_GPS.h>
#include <AG_Generator/AG_Generator.h>
#include <AG_Notify/AG_Notify.h>                    // Notify library
#include <AG_Param/AG_Param.h>
#include <AG_RangeFinder/AG_RangeFinder.h>
#include <AG_Relay/AG_Relay.h>                      // APM relay
#include <AG_RSSI/AG_RSSI.h>                        // RSSI Library
#include <AG_Scheduler/AG_Scheduler.h>
#include <AG_SerialManager/AG_SerialManager.h>      // Serial manager library
#include <AG_ServoRelayEvents/AG_ServoRelayEvents.h>
#include <AG_Camera/AG_RunCam.h>
#include <AG_OpenDroneID/AG_OpenDroneID.h>
#include <AG_Hott_Telem/AG_Hott_Telem.h>
#include <AG_ESC_Telem/AG_ESC_Telem.h>
#include <AG_GyroFFT/AG_GyroFFT.h>
#include <AG_VisualOdom/AG_VisualOdom.h>
#include <AG_VideoTX/AG_VideoTX.h>
#include <AG_MSP/AG_MSP.h>
#include <AG_Frsky_Telem/AG_Frsky_Parameters.h>
#include <AG_ExternalAHRS/AG_ExternalAHRS.h>
#include <AG_VideoTX/AG_SmartAudio.h>
#include <AG_VideoTX/AG_Tramp.h>
#include <AG_TemperatureSensor/AG_TemperatureSensor.h>
#include <SITL/SITL.h>
#include <AG_CustomRotations/AG_CustomRotations.h>
#include <AG_AIS/AG_AIS.h>
#include <AG_NMEA_Output/AG_NMEA_Output.h>
#include <AG_Fence/AG_Fence.h>
#include <AG_CheckFirmware/AG_CheckFirmware.h>
#include <Filter/LowPassFilter.h>

class AG_Vehicle : public AG_HAL::HAL::Callbacks {

public:

    AG_Vehicle() {
        if (_singleton) {
            AG_HAL::panic("Too many Vehicles");
        }
        AG_Param::setup_object_defaults(this, var_info);
        _singleton = this;
    }

    /* Do not allow copies */
    CLASS_NO_COPY(AG_Vehicle);

    static AG_Vehicle *get_singleton();

    // setup() is called once during vehicle startup to initialise the
    // vehicle object and the objects it contains.  The
    // AG_HAL_MAIN_CALLBACKS pragma creates a main(...) function
    // referencing an object containing setup() and loop() functions.
    // A vehicle is not expected to override setup(), but
    // subclass-specific initialisation can be done in init_ardupilot
    // which is called from setup().
    void setup(void) override final;

    // HAL::Callbacks implementation.
    void loop() override final;

    // set_mode *must* set control_mode_reason
    virtual bool set_mode(const uint8_t new_mode, const ModeReason reason) = 0;
    virtual uint8_t get_mode() const = 0;

    ModeReason get_control_mode_reason() const {
        return control_mode_reason;
    }

    // perform any notifications required to indicate a mode change
    // failed due to a bad mode number being supplied.  This can
    // happen for many reasons - bad mavlink packet and bad mode
    // parameters for example.
    void notify_no_such_mode(uint8_t mode_number);

    void get_common_scheduler_tasks(const AG_Scheduler::Task*& tasks, uint8_t& num_tasks);
    // implementations *MUST* fill in all passed-in fields or we get
    // Valgrind errors
    virtual void get_scheduler_tasks(const AG_Scheduler::Task *&tasks, uint8_t &task_count, uint32_t &log_bit) = 0;

    /*
      set the "likely flying" flag. This is not guaranteed to be
      accurate, but is the vehicle codes best guess as to the whether
      the vehicle is currently flying
    */
    void set_likely_flying(bool b) {
        if (b && !likely_flying) {
            _last_flying_ms = AG_HAL::millis();
        }
        likely_flying = b;
    }

    /*
      get the likely flying status. Returns true if the vehicle code
      thinks we are flying at the moment. Not guaranteed to be
      accurate
    */
    bool get_likely_flying(void) const {
        return likely_flying;
    }

    /*
      return time in milliseconds since likely_flying was set
      true. Returns zero if likely_flying is currently false
    */
    uint32_t get_time_flying_ms(void) const {
        if (!likely_flying) {
            return 0;
        }
        return AG_HAL::millis() - _last_flying_ms;
    }

    // returns true if the vehicle has crashed
    virtual bool is_crashed() const;

#if AP_SCRIPTING_ENABLED
    /*
      methods to control vehicle for use by scripting
    */
    virtual bool start_takeoff(float alt) { return false; }
    virtual bool set_target_location(const Location& target_loc) { return false; }
    virtual bool set_target_pos_NED(const Vector3f& target_pos, bool use_yaw, float yaw_deg, bool use_yaw_rate, float yaw_rate_degs, bool yaw_relative, bool terrain_alt) { return false; }
    virtual bool set_target_posvel_NED(const Vector3f& target_pos, const Vector3f& target_vel) { return false; }
    virtual bool set_target_posvelaccel_NED(const Vector3f& target_pos, const Vector3f& target_vel, const Vector3f& target_accel, bool use_yaw, float yaw_deg, bool use_yaw_rate, float yaw_rate_degs, bool yaw_relative) { return false; }
    virtual bool set_target_velocity_NED(const Vector3f& vel_ned) { return false; }
    virtual bool set_target_velaccel_NED(const Vector3f& target_vel, const Vector3f& target_accel, bool use_yaw, float yaw_deg, bool use_yaw_rate, float yaw_rate_degs, bool yaw_relative) { return false; }
    virtual bool set_target_angle_and_climbrate(float roll_deg, float pitch_deg, float yaw_deg, float climb_rate_ms, bool use_yaw_rate, float yaw_rate_degs) { return false; }

    // command throttle percentage and roll, pitch, yaw target
    // rates. For use with scripting controllers
    virtual void set_target_throttle_rate_rpy(float throttle_pct, float roll_rate_dps, float pitch_rate_dps, float yaw_rate_dps) {}
    virtual void set_rudder_offset(float rudder_pct, bool run_yaw_rate_controller) {}
    virtual bool nav_scripting_enable(uint8_t mode) {return false;}

    // get target location (for use by scripting)
    virtual bool get_target_location(Location& target_loc) { return false; }
    virtual bool update_target_location(const Location &old_loc, const Location &new_loc) { return false; }

    // circle mode controls (only used by scripting with Copter)
    virtual bool get_circle_radius(float &radius_m) { return false; }
    virtual bool set_circle_rate(float rate_dps) { return false; }

    // get or set steering and throttle (-1 to +1) (for use by scripting with Rover)
    virtual bool set_steering_and_throttle(float steering, float throttle) { return false; }
    virtual bool get_steering_and_throttle(float& steering, float& throttle) { return false; }

    // set turn rate in deg/sec and speed in meters/sec (for use by scripting with Rover)
    virtual bool set_desired_turn_rate_and_speed(float turn_rate, float speed) { return false; }

   // set auto mode speed in meters/sec (for use by scripting with Copter/Rover)
    virtual bool set_desired_speed(float speed) { return false; }

    // support for NAV_SCRIPT_TIME mission command
    virtual bool nav_script_time(uint16_t &id, uint8_t &cmd, float &arg1, float &arg2, int16_t &arg3, int16_t &arg4) { return false; }
    virtual void nav_script_time_done(uint16_t id) {}

    // allow for VTOL velocity matching of a target
    virtual bool set_velocity_match(const Vector2f &velocity) { return false; }

    // returns true if the EKF failsafe has triggered
    virtual bool has_ekf_failsafed() const { return false; }

    // allow for landing descent rate to be overridden by a script, may be -ve to climb
    virtual bool set_land_descent_rate(float descent_rate) { return false; }
    
    // control outputs enumeration
    enum class ControlOutput {
        Roll = 1,
        Pitch = 2,
        Throttle = 3,
        Yaw = 4,
        Lateral = 5,
        MainSail = 6,
        WingSail = 7,
        Walking_Height = 8,
        Last_ControlOutput  // place new values before this
    };

    // get control output (for use in scripting)
    // returns true on success and control_value is set to a value in the range -1 to +1
    virtual bool get_control_output(AG_Vehicle::ControlOutput control_output, float &control_value) { return false; }

#endif // AP_SCRIPTING_ENABLED

    // zeroing the RC outputs can prevent unwanted motor movement:
    virtual bool should_zero_rc_outputs_on_reboot() const { return false; }

    // reboot the vehicle in an orderly manner, doing various cleanups
    // and flashing LEDs as appropriate
    void reboot(bool hold_in_bootloader);

    /*
      get the distance to next wp in meters
      return false if failed or n/a
     */
    virtual bool get_wp_distance_m(float &distance) const { return false; }

    /*
      get the current wp bearing in degrees
      return false if failed or n/a
     */
    virtual bool get_wp_bearing_deg(float &bearing) const { return false; }

    /*
      get the current wp crosstrack error in meters
      return false if failed or n/a
     */
    virtual bool get_wp_crosstrack_error_m(float &xtrack_error) const { return false; }

#if HAL_WITH_FRSKY_TELEM_BIDIRECTIONAL
    AG_Frsky_Parameters frsky_parameters;
#endif

    /*
      Returns the pan and tilt for use by onvif camera in scripting
     */
    virtual bool get_pan_tilt_norm(float &pan_norm, float &tilt_norm) const { return false; }

   // Returns roll and  pitch for OSD Horizon, Plane overrides to correct for VTOL view and fixed wing TRIM_PITCH_CD
    virtual void get_osd_roll_pitch_rad(float &roll, float &pitch) const;

    /*
     get the target earth-frame angular velocities in rad/s (Z-axis component used by some gimbals)
     */
    virtual bool get_rate_ef_targets(Vector3f& rate_ef_targets) const { return false; }

protected:

    virtual void init_ardupilot() = 0;
    virtual void load_parameters() = 0;
    virtual void set_control_channels() {}

    // board specific config
    AG_BoardConfig BoardConfig;

#if HAL_MAX_CAN_PROTOCOL_DRIVERS
    // board specific config for CAN bus
    AG_CANManager can_mgr;
#endif

    // main loop scheduler
    AG_Scheduler scheduler;

    // IMU variables
    // Integration time; time last loop took to run
    float G_Dt;

    // sensor drivers
    AG_GPS gps;
    AG_Baro barometer;
    Compass compass;
    AG_InertialSensor ins;
#if HAL_BUTTON_ENABLED
    AG_Button button;
#endif
    RangeFinder rangefinder;

    AG_RSSI rssi;
#if HAL_RUNCAM_ENABLED
    AG_RunCam runcam;
#endif
#if HAL_GYROFFT_ENABLED
    AG_GyroFFT gyro_fft;
#endif
#if AP_VIDEOTX_ENABLED
    AG_VideoTX vtx;
#endif
    AG_SerialManager serial_manager;

    AG_Relay relay;

    AG_ServoRelayEvents ServoRelayEvents;

    // notification object for LEDs, buzzers etc (parameter set to
    // false disables external leds)
    AG_Notify notify;

    // Inertial Navigation EKF
    AG_AHRS ahrs;

#if HAL_HOTT_TELEM_ENABLED
    AG_Hott_Telem hott_telem;
#endif

#if HAL_VISUALODOM_ENABLED
    AG_VisualOdom visual_odom;
#endif

#if HAL_WITH_ESC_TELEM
    AG_ESC_Telem esc_telem;
#endif

#if AP_OPENDRONEID_ENABLED
    AG_OpenDroneID opendroneid;
#endif

#if HAL_MSP_ENABLED
    AG_MSP msp;
#endif

#if HAL_GENERATOR_ENABLED
    AG_Generator generator;
#endif

#if HAL_EXTERNAL_AHRS_ENABLED
    AG_ExternalAHRS externalAHRS;
#endif

#if AP_SMARTAUDIO_ENABLED
    AG_SmartAudio smartaudio;
#endif

#if AP_TRAMP_ENABLED
    AG_Tramp tramp;
#endif

#if HAL_EFI_ENABLED
    // EFI Engine Monitor
    AG_EFI efi;
#endif

#if AP_AIRSPEED_ENABLED
    AG_Airspeed airspeed;
#endif

#if AG_AIS_ENABLED
    // Automatic Identification System - for tracking sea-going vehicles
    AG_AIS ais;
#endif

#if HAL_NMEA_OUTPUT_ENABLED
    AG_NMEA_Output nmea;
#endif

#if AP_FENCE_ENABLED
    AG_Fence fence;
#endif

#if AP_TEMPERATURE_SENSOR_ENABLED
    AG_TemperatureSensor temperature_sensor;
#endif

    static const struct AG_Param::GroupInfo var_info[];
    static const struct AG_Scheduler::Task scheduler_tasks[];

#if OSD_ENABLED
    void publish_osd_info();
#endif

#if HAL_INS_ACCELCAL_ENABLED
    // update accel calibration
    void accel_cal_update();
#endif

    // call the arming library's update function
    void update_arming();

    // check for motor noise at a particular frequency
    void check_motor_noise();

    ModeReason control_mode_reason = ModeReason::UNKNOWN;

#if AP_SIM_ENABLED
    SITL::SIM sitl;
#endif

private:

    // delay() callback that processing MAVLink packets
    static void scheduler_delay_callback();

    // if there's been a watchdog reset, notify the world via a
    // statustext:
    void send_watchdog_reset_statustext();

    // update the harmonic notch for throttle based notch
    void update_throttle_notch(AG_InertialSensor::HarmonicNotch &notch);

    // update the harmonic notch
    void update_dynamic_notch(AG_InertialSensor::HarmonicNotch &notch);

    // run notch update at either loop rate or 200Hz
    void update_dynamic_notch_at_specified_rate();

    // decimation for 1Hz update
    uint8_t one_Hz_counter;
    void one_Hz_update();

    bool likely_flying;         // true if vehicle is probably flying
    uint32_t _last_flying_ms;   // time when likely_flying last went true
    uint32_t _last_notch_update_ms[HAL_INS_NUM_HARMONIC_NOTCH_FILTERS]; // last time update_dynamic_notch() was run

    static AG_Vehicle *_singleton;

#if HAL_GYROFFT_ENABLED && HAL_WITH_ESC_TELEM
    LowPassFilterFloat esc_noise[ESC_TELEM_MAX_ESCS];
    uint32_t last_motor_noise_ms;
#endif

    bool done_safety_init;


    uint32_t _last_internal_errors;  // backup of AG_InternalError::internal_errors bitmask

    AG_CustomRotations custom_rotations;
};

namespace AP {
    AG_Vehicle *vehicle();
};

extern const AG_HAL::HAL& hal;

extern const AG_Param::Info vehicle_var_info[];

#include "AG_Vehicle_Type.h"
