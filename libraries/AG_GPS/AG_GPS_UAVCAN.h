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

//
//  UAVCAN GPS driver
//
#pragma once

#include <AG_Common/AG_Common.h>
#include <AG_HAL/AG_HAL.h>
#include "AG_GPS.h"
#include "GPS_Backend.h"
#include "RTCM3_Parser.h"
#include <AG_UAVCAN/AG_UAVCAN.h>

class FixCb;
class Fix2Cb;
class AuxCb;
class HeadingCb;
class StatusCb;
#if GPS_MOVING_BASELINE
class MovingBaselineDataCb;
class RelPosHeadingCb;
#endif

class AG_GPS_UAVCAN : public AG_GPS_Backend {
public:
    AG_GPS_UAVCAN(AG_GPS &_gps, AG_GPS::GPS_State &_state, AG_GPS::GPS_Role role);
    ~AG_GPS_UAVCAN();

    bool read() override;

    bool is_healthy(void) const override;

    bool logging_healthy(void) const override;

    bool is_configured(void) const override;

    const char *name() const override { return _name; }

    static void subscribe_msgs(AG_UAVCAN* ap_uavcan);
    static AG_GPS_Backend* probe(AG_GPS &_gps, AG_GPS::GPS_State &_state);

    static void handle_fix2_msg_trampoline(AG_UAVCAN* ap_uavcan, uint8_t node_id, const Fix2Cb &cb);
    static void handle_aux_msg_trampoline(AG_UAVCAN* ap_uavcan, uint8_t node_id, const AuxCb &cb);
    static void handle_heading_msg_trampoline(AG_UAVCAN* ap_uavcan, uint8_t node_id, const HeadingCb &cb);
    static void handle_status_msg_trampoline(AG_UAVCAN* ap_uavcan, uint8_t node_id, const StatusCb &cb);
#if GPS_MOVING_BASELINE
    static void handle_moving_baseline_msg_trampoline(AG_UAVCAN* ap_uavcan, uint8_t node_id, const MovingBaselineDataCb &cb);
    static void handle_relposheading_msg_trampoline(AG_UAVCAN* ap_uavcan, uint8_t node_id, const RelPosHeadingCb &cb);
#endif
    static bool backends_healthy(char failure_msg[], uint16_t failure_msg_len);
    void inject_data(const uint8_t *data, uint16_t len) override;

    bool get_error_codes(uint32_t &error_codes) const override { error_codes = error_code; return seen_status; };

#if GPS_MOVING_BASELINE
    bool get_RTCMV3(const uint8_t *&data, uint16_t &len) override;
    void clear_RTCMV3() override;
#endif

#if AP_DRONECAN_SEND_GPS
    static bool instance_exists(const AG_UAVCAN* ap_uavcan);
#endif

private:

    bool param_configured = true;
    enum config_step {
        STEP_SET_TYPE = 0,
        STEP_SET_MB_CAN_TX,
        STEP_SAVE_AND_REBOOT,
        STEP_FINISHED
    };
    uint8_t cfg_step;
    bool requires_save_and_reboot;

    // returns true once configuration has finished
    bool do_config(void);

    void handle_fix2_msg(const Fix2Cb &cb);
    void handle_aux_msg(const AuxCb &cb);
    void handle_heading_msg(const HeadingCb &cb);
    void handle_status_msg(const StatusCb &cb);
    void handle_velocity(const float vx, const float vy, const float vz);

#if GPS_MOVING_BASELINE
    void handle_moving_baseline_msg(const MovingBaselineDataCb &cb, uint8_t node_id);
    void handle_relposheading_msg(const RelPosHeadingCb &cb, uint8_t node_id);
#endif

    static bool take_registry();
    static void give_registry();
    static AG_GPS_UAVCAN* get_uavcan_backend(AG_UAVCAN* ap_uavcan, uint8_t node_id);

    bool _new_data;
    AG_GPS::GPS_State interim_state;

    HAL_Semaphore sem;

    uint8_t _detected_module;
    bool seen_message;
    bool seen_fix2;
    bool seen_aux;
    bool seen_status;
    bool seen_relposheading;

    bool healthy;
    uint32_t status_flags;
    uint32_t error_code;
    char _name[15];

    // Module Detection Registry
    static struct DetectedModules {
        AG_UAVCAN* ap_uavcan;
        uint8_t node_id;
        uint8_t instance;
        uint32_t last_inject_ms;
        AG_GPS_UAVCAN* driver;
    } _detected_modules[GPS_MAX_RECEIVERS];

    static HAL_Semaphore _sem_registry;

#if GPS_MOVING_BASELINE
    // RTCM3 parser for when in moving baseline base mode
    RTCM3_Parser *rtcm3_parser;
#endif
    // the role set from GPS_TYPE
    AG_GPS::GPS_Role role;

    FUNCTOR_DECLARE(param_int_cb, bool, AG_UAVCAN*, const uint8_t, const char*, int32_t &);
    FUNCTOR_DECLARE(param_float_cb, bool, AG_UAVCAN*, const uint8_t, const char*, float &);
    FUNCTOR_DECLARE(param_save_cb, void, AG_UAVCAN*, const uint8_t, bool);

    bool handle_param_get_set_response_int(AG_UAVCAN* ap_uavcan, const uint8_t node_id, const char* name, int32_t &value);
    bool handle_param_get_set_response_float(AG_UAVCAN* ap_uavcan, const uint8_t node_id, const char* name, float &value);
    void handle_param_save_response(AG_UAVCAN* ap_uavcan, const uint8_t node_id, bool success);
};
