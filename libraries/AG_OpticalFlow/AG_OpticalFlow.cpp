#include <AG_BoardConfig/AG_BoardConfig.h>
#include "AG_OpticalFlow.h"

#if AP_OPTICALFLOW_ENABLED

#include "AG_OpticalFlow_Onboard.h"
#include "AG_OpticalFlow_SITL.h"
#include "AG_OpticalFlow_Pixart.h"
#include "AG_OpticalFlow_PX4Flow.h"
#include "AG_OpticalFlow_CXOF.h"
#include "AG_OpticalFlow_MAV.h"
#include "AG_OpticalFlow_HereFlow.h"
#include "AG_OpticalFlow_MSP.h"
#include "AG_OpticalFlow_UPFLOW.h"
#include <AG_Logger/AG_Logger.h>
#include <GCS_MAVLink/GCS.h>

extern const AG_HAL::HAL& hal;

#ifndef OPTICAL_FLOW_TYPE_DEFAULT
 #if CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_CHIBIOS_SKYVIPER_F412 || defined(HAL_HAVE_PIXARTFLOW_SPI)
  #define OPTICAL_FLOW_TYPE_DEFAULT Type::PIXART
 #elif CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_BEBOP
  #define OPTICAL_FLOW_TYPE_DEFAULT Type::BEBOP
 #else
  #define OPTICAL_FLOW_TYPE_DEFAULT Type::NONE
 #endif
#endif

const AG_Param::GroupInfo AG_OpticalFlow::var_info[] = {
    // @Param: _TYPE
    // @DisplayName: Optical flow sensor type
    // @Description: Optical flow sensor type
    // @Values: 0:None, 1:PX4Flow, 2:Pixart, 3:Bebop, 4:CXOF, 5:MAVLink, 6:DroneCAN, 7:MSP, 8:UPFLOW
    // @User: Standard
    // @RebootRequired: True
    AP_GROUPINFO_FLAGS("_TYPE", 0,  AG_OpticalFlow,    _type,   (float)OPTICAL_FLOW_TYPE_DEFAULT, AP_PARAM_FLAG_ENABLE),

    // @Param: _FXSCALER
    // @DisplayName: X axis optical flow scale factor correction
    // @Description: This sets the parts per thousand scale factor correction applied to the flow sensor X axis optical rate. It can be used to correct for variations in effective focal length. Each positive increment of 1 increases the scale factor applied to the X axis optical flow reading by 0.1%. Negative values reduce the scale factor.
    // @Range: -200 +200
    // @Increment: 1
    // @User: Standard
    AP_GROUPINFO("_FXSCALER", 1,  AG_OpticalFlow,    _flowScalerX,   0),

    // @Param: _FYSCALER
    // @DisplayName: Y axis optical flow scale factor correction
    // @Description: This sets the parts per thousand scale factor correction applied to the flow sensor Y axis optical rate. It can be used to correct for variations in effective focal length. Each positive increment of 1 increases the scale factor applied to the Y axis optical flow reading by 0.1%. Negative values reduce the scale factor.
    // @Range: -200 +200
    // @Increment: 1
    // @User: Standard
    AP_GROUPINFO("_FYSCALER", 2,  AG_OpticalFlow,    _flowScalerY,   0),

    // @Param: _ORIENT_YAW
    // @DisplayName: Flow sensor yaw alignment
    // @Description: Specifies the number of centi-degrees that the flow sensor is yawed relative to the vehicle. A sensor with its X-axis pointing to the right of the vehicle X axis has a positive yaw angle.
    // @Units: cdeg
    // @Range: -17999 +18000
    // @Increment: 10
    // @User: Standard
    AP_GROUPINFO("_ORIENT_YAW", 3,  AG_OpticalFlow,    _yawAngle_cd,   0),

    // @Param: _POS_X
    // @DisplayName:  X position offset
    // @Description: X position of the optical flow sensor focal point in body frame. Positive X is forward of the origin.
    // @Units: m
    // @Range: -5 5
    // @Increment: 0.01
    // @User: Advanced

    // @Param: _POS_Y
    // @DisplayName: Y position offset
    // @Description: Y position of the optical flow sensor focal point in body frame. Positive Y is to the right of the origin.
    // @Units: m
    // @Range: -5 5
    // @Increment: 0.01
    // @User: Advanced

    // @Param: _POS_Z
    // @DisplayName: Z position offset
    // @Description: Z position of the optical flow sensor focal point in body frame. Positive Z is down from the origin.
    // @Units: m
    // @Range: -5 5
    // @Increment: 0.01
    // @User: Advanced
    AP_GROUPINFO("_POS", 4, AG_OpticalFlow, _pos_offset, 0.0f),

    // @Param: _ADDR
    // @DisplayName: Address on the bus
    // @Description: This is used to select between multiple possible I2C addresses for some sensor types. For PX4Flow you can choose 0 to 7 for the 8 possible addresses on the I2C bus.
    // @Range: 0 127
    // @User: Advanced
    AP_GROUPINFO("_ADDR", 5,  AG_OpticalFlow, _address,   0),

    // @Param: _HGT_OVR
    // @DisplayName: Height override of sensor above ground
    // @Description: This is used in rover vehicles, where the sensor is a fixed height above the ground
    // @Units: m
    // @Range: 0 2
    // @Increment: 0.01
    // @User: Advanced
    AP_GROUPINFO_FRAME("_HGT_OVR", 6,  AG_OpticalFlow, _height_override,   0.0f, AP_PARAM_FRAME_ROVER),

    AP_GROUPEND
};

// default constructor
AG_OpticalFlow::AG_OpticalFlow()
{
    _singleton = this;

    AG_Param::setup_object_defaults(this, var_info);
}

void AG_OpticalFlow::init(uint32_t log_bit)
{
     _log_bit = log_bit;

    // return immediately if not enabled or backend already created
    if ((_type == Type::NONE) || (backend != nullptr)) {
        return;
    }

    switch ((Type)_type) {
    case Type::NONE:
        break;
    case Type::PX4FLOW:
#if AP_OPTICALFLOW_PX4FLOW_ENABLED
        backend = AG_OpticalFlow_PX4Flow::detect(*this);
#endif
        break;
    case Type::PIXART:
#if AP_OPTICALFLOW_PIXART_ENABLED
        backend = AG_OpticalFlow_Pixart::detect("pixartflow", *this);
        if (backend == nullptr) {
            backend = AG_OpticalFlow_Pixart::detect("pixartPC15", *this);
        }
#endif
        break;
    case Type::BEBOP:
#if CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_BEBOP
        backend = new AG_OpticalFlow_Onboard(*this);
#endif
        break;
    case Type::CXOF:
#if AP_OPTICALFLOW_CXOF_ENABLED
        backend = AG_OpticalFlow_CXOF::detect(*this);
#endif
        break;
    case Type::MAVLINK:
#if AP_OPTICALFLOW_MAV_ENABLED
        backend = AG_OpticalFlow_MAV::detect(*this);
#endif
        break;
    case Type::UAVCAN:
#if AP_OPTICALFLOW_HEREFLOW_ENABLED
        backend = new AG_OpticalFlow_HereFlow(*this);
#endif
        break;
    case Type::MSP:
#if HAL_MSP_OPTICALFLOW_ENABLED
        backend = AG_OpticalFlow_MSP::detect(*this);
#endif
        break;
    case Type::UPFLOW:
#if AP_OPTICALFLOW_UPFLOW_ENABLED
        backend = AG_OpticalFlow_UPFLOW::detect(*this);
#endif
        break;
    case Type::SITL:
#if AP_OPTICALFLOW_SITL_ENABLED
        backend = new AG_OpticalFlow_SITL(*this);
#endif
        break;
    }

    if (backend != nullptr) {
        backend->init();
    }
}

void AG_OpticalFlow::update(void)
{
    // exit immediately if not enabled
    if (!enabled()) {
        return;
    }
    if (backend != nullptr) {
        backend->update();
    }

    // only healthy if the data is less than 0.5s old
    _flags.healthy = (AG_HAL::millis() - _last_update_ms < 500);

    // update calibrator and save resulting scaling
    if (_calibrator != nullptr) {
        if (_calibrator->update()) {
            // apply new calibration values
            const Vector2f new_scaling = _calibrator->get_scalars();
            const float flow_scalerx_as_multiplier = (1.0 + (_flowScalerX * 0.001)) * new_scaling.x;
            const float flow_scalery_as_multiplier = (1.0 + (_flowScalerY * 0.001)) * new_scaling.y;
            _flowScalerX.set_and_save_ifchanged((flow_scalerx_as_multiplier - 1.0) * 1000.0);
            _flowScalerY.set_and_save_ifchanged((flow_scalery_as_multiplier - 1.0) * 1000.0);
            _flowScalerX.notify();
            _flowScalerY.notify();
            GCS_SEND_TEXT(MAV_SEVERITY_INFO, "FlowCal: FLOW_FXSCALER=%d, FLOW_FYSCALER=%d", (int)_flowScalerX, (int)_flowScalerY);
        }
    }
}

void AG_OpticalFlow::handle_msg(const mavlink_message_t &msg)
{
    // exit immediately if not enabled
    if (!enabled()) {
        return;
    }

    if (backend != nullptr) {
        backend->handle_msg(msg);
    }
}

#if HAL_MSP_OPTICALFLOW_ENABLED
void AG_OpticalFlow::handle_msp(const MSP::msp_opflow_data_message_t &pkt)
{
    // exit immediately if not enabled
    if (!enabled()) {
        return;
    }

    if (backend != nullptr) {
        backend->handle_msp(pkt);
    }
}
#endif //HAL_MSP_OPTICALFLOW_ENABLED

// start calibration
void AG_OpticalFlow::start_calibration()
{
    if (_calibrator == nullptr) {
        _calibrator = new AG_OpticalFlow_Calibrator();
        if (_calibrator == nullptr) {
            GCS_SEND_TEXT(MAV_SEVERITY_CRITICAL, "FlowCal: failed to start");
            return;
        }
    }
    if (_calibrator != nullptr) {
        _calibrator->start();
    }
}

// stop calibration
void AG_OpticalFlow::stop_calibration()
{
    if (_calibrator != nullptr) {
        _calibrator->stop();
    }
}

void AG_OpticalFlow::update_state(const OpticalFlow_state &state)
{
    _state = state;
    _last_update_ms = AG_HAL::millis();

    // write to log and send to EKF if new data has arrived
    AP::ahrs().writeOptFlowMeas(quality(),
                                _state.flowRate,
                                _state.bodyRate,
                                _last_update_ms,
                                get_pos_offset(),
                                get_height_override());
    Log_Write_Optflow();
}

void AG_OpticalFlow::Log_Write_Optflow()
{
    AG_Logger *logger = AG_Logger::get_singleton();
    if (logger == nullptr) {
        return;
    }
    if (_log_bit != (uint32_t)-1 &&
        !logger->should_log(_log_bit)) {
        return;
    }

    struct log_Optflow pkt = {
        LOG_PACKET_HEADER_INIT(LOG_OPTFLOW_MSG),
        time_us         : AG_HAL::micros64(),
        surface_quality : _state.surface_quality,
        flow_x          : _state.flowRate.x,
        flow_y          : _state.flowRate.y,
        body_x          : _state.bodyRate.x,
        body_y          : _state.bodyRate.y
    };
    logger->WriteBlock(&pkt, sizeof(pkt));
}



// singleton instance
AG_OpticalFlow *AG_OpticalFlow::_singleton;

namespace AP {

AG_OpticalFlow *opticalflow()
{
    return AG_OpticalFlow::get_singleton();
}

}

#endif // AP_OPTICALFLOW_ENABLED
