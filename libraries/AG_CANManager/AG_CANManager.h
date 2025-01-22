/*
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Code by Siddharth Bharat Purohit
 */

#pragma once

#include "AG_CANManager_config.h"

#include <AG_HAL/AG_HAL.h>

#if HAL_MAX_CAN_PROTOCOL_DRIVERS

#include <AG_Param/AG_Param.h>
#include "AG_SLCANIface.h"
#include "AG_CANDriver.h"
#include <GCS_MAVLink/GCS_config.h>
#if HAL_GCS_ENABLED
#include <GCS_MAVLink/GCS_MAVLink.h>
#include <AG_HAL/utility/RingBuffer.h>
#endif

class AG_CANManager
{
public:
    AG_CANManager();

    /* Do not allow copies */
    CLASS_NO_COPY(AG_CANManager);

    static AG_CANManager* get_singleton()
    {
        if (_singleton == nullptr) {
            AG_HAL::panic("CANManager used before allocation.");
        }
        return _singleton;
    }

    enum LogLevel : uint8_t {
        LOG_NONE,
        LOG_ERROR,
        LOG_WARNING,
        LOG_INFO,
        LOG_DEBUG,
    };

    enum Driver_Type : uint8_t {
        Driver_Type_None = 0,
        Driver_Type_UAVCAN = 1,
        // 2 was KDECAN -- do not re-use
        // 3 was ToshibaCAN -- do not re-use
        Driver_Type_PiccoloCAN = 4,
        Driver_Type_CANTester = 5,
        Driver_Type_EFI_NWPMU = 6,
        Driver_Type_USD1 = 7,
        Driver_Type_KDECAN = 8,
        // 9 was Driver_Type_MPPT_PacketDigital
        Driver_Type_Scripting = 10,
        Driver_Type_Benewake = 11,
        Driver_Type_Scripting2 = 12,
    };

    void init(void);

    // register a new driver
    bool register_driver(Driver_Type dtype, AG_CANDriver *driver);

    // returns number of active CAN Drivers
    uint8_t get_num_drivers(void) const
    {
        return HAL_MAX_CAN_PROTOCOL_DRIVERS;
    }

    // return driver for index i
    AG_CANDriver* get_driver(uint8_t i) const
    {
        if (i < HAL_NUM_CAN_IFACES) {
            return _drivers[i];
        }
        return nullptr;
    }

    // returns current log level
    LogLevel get_log_level(void) const
    {
        return LogLevel(_loglevel.get());
    }
    
    // Method to log status and debug information for review while debugging
    void log_text(AG_CANManager::LogLevel loglevel, const char *tag, const char *fmt, ...) FMT_PRINTF(4,5);

    void log_retrieve(ExpandingString &str) const;

    // return driver type index i
    Driver_Type get_driver_type(uint8_t i) const
    {
        if (i < HAL_NUM_CAN_IFACES) {
            return _driver_type_cache[i];
        }
        return Driver_Type_None;
    }

    static const struct AG_Param::GroupInfo var_info[];

#if HAL_GCS_ENABLED
    bool handle_can_forward(mavlink_channel_t chan, const mavlink_command_long_t &packet, const mavlink_message_t &msg);
    void handle_can_frame(const mavlink_message_t &msg);
    void handle_can_filter_modify(const mavlink_message_t &msg);
#endif

private:

    // Parameter interface for CANIfaces
    class CANIface_Params
    {
        friend class AG_CANManager;

    public:
        CANIface_Params()
        {
            AG_Param::setup_object_defaults(this, var_info);
        }

        static const struct AG_Param::GroupInfo var_info[];

    private:
        AP_Int8 _driver_number;
        AP_Int32 _bitrate;
        AP_Int32 _fdbitrate;
    };

    //Parameter Interface for CANDrivers
    class CANDriver_Params
    {
        friend class AG_CANManager;

    public:
        CANDriver_Params()
        {
            AG_Param::setup_object_defaults(this, var_info);
        }
        static const struct AG_Param::GroupInfo var_info[];

    private:
        AP_Int8 _driver_type;
        AG_CANDriver* _testcan;
        AG_CANDriver* _uavcan;
        AG_CANDriver* _kdecan;
        AG_CANDriver* _piccolocan;
    };

    CANIface_Params _interfaces[HAL_NUM_CAN_IFACES];
    AG_CANDriver* _drivers[HAL_MAX_CAN_PROTOCOL_DRIVERS];
    CANDriver_Params _drv_param[HAL_MAX_CAN_PROTOCOL_DRIVERS];
    Driver_Type _driver_type_cache[HAL_MAX_CAN_PROTOCOL_DRIVERS];

    AP_Int8 _loglevel;
    uint8_t _num_drivers;
#if AP_CAN_SLCAN_ENABLED
    SLCAN::CANIface _slcan_interface;
#endif

    static AG_CANManager *_singleton;

    char* _log_buf;
    uint32_t _log_pos;

    HAL_Semaphore _sem;

#if HAL_GCS_ENABLED
    /*
      handler for CAN frames from the registered callback, sending frames
      out as CAN_FRAME messages
    */
    void can_frame_callback(uint8_t bus, const AG_HAL::CANFrame &frame);

    struct {
        mavlink_channel_t chan;
        uint8_t system_id;
        uint8_t component_id;
        uint8_t frame_counter;
        uint32_t last_callback_enable_ms;
        HAL_Semaphore sem;
        uint16_t num_filter_ids;
        uint16_t *filter_ids;
    } can_forward;

    // buffer for MAVCAN frames
    struct BufferFrame {
        uint8_t bus;
        AG_HAL::CANFrame frame;
    };
    ObjectBuffer<BufferFrame> *frame_buffer;

    void process_frame_buffer(void);
#endif // HAL_GCS_ENABLED
};

namespace AP
{
AG_CANManager& can();
}

#endif
