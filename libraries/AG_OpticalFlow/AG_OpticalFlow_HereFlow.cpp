#include "AG_OpticalFlow_HereFlow.h"

#if AP_OPTICALFLOW_HEREFLOW_ENABLED

#include <AG_HAL/AG_HAL.h>

#include <AG_CANManager/AG_CANManager.h>
#include <AG_UAVCAN/AG_UAVCAN.h>

#include <com/hex/equipment/flow/Measurement.hpp>

extern const AG_HAL::HAL& hal;

//UAVCAN Frontend Registry Binder
UC_REGISTRY_BINDER(MeasurementCb, com::hex::equipment::flow::Measurement);

uint8_t AG_OpticalFlow_HereFlow::_node_id = 0;
AG_OpticalFlow_HereFlow* AG_OpticalFlow_HereFlow::_driver = nullptr;
AG_UAVCAN* AG_OpticalFlow_HereFlow::_ap_uavcan = nullptr;
/*
  constructor - registers instance at top Flow driver
 */
AG_OpticalFlow_HereFlow::AG_OpticalFlow_HereFlow(AG_OpticalFlow &flow) :
    OpticalFlow_backend(flow)
{
    if (_driver) {
        AG_HAL::panic("Only one instance of Flow supported!");
    }
    _driver = this;
}

//links the HereFlow messages to the backend
void AG_OpticalFlow_HereFlow::subscribe_msgs(AG_UAVCAN* ap_uavcan)
{
    if (ap_uavcan == nullptr) {
        return;
    }

    auto* node = ap_uavcan->get_node();

    uavcan::Subscriber<com::hex::equipment::flow::Measurement, MeasurementCb> *measurement_listener;
    measurement_listener = new uavcan::Subscriber<com::hex::equipment::flow::Measurement, MeasurementCb>(*node);
    // Register method to handle incoming HereFlow measurement
    const int measurement_listener_res = measurement_listener->start(MeasurementCb(ap_uavcan, &handle_measurement));
    if (measurement_listener_res < 0) {
        AG_HAL::panic("UAVCAN Flow subscriber start problem\n\r");
        return;
    }
}

//updates driver states based on received HereFlow messages
void AG_OpticalFlow_HereFlow::handle_measurement(AG_UAVCAN* ap_uavcan, uint8_t node_id, const MeasurementCb &cb)
{
    if (_driver == nullptr) {
        return;
    }
    //protect from data coming from duplicate sensors,
    //as we only handle one Here Flow at a time as of now
    if (_ap_uavcan == nullptr) {
        _ap_uavcan = ap_uavcan;
        _node_id = node_id;
    }

    if (_ap_uavcan == ap_uavcan && _node_id == node_id) {
        WITH_SEMAPHORE(_driver->_sem);
        _driver->new_data = true;
        _driver->flowRate = Vector2f(cb.msg->flow_integral[0], cb.msg->flow_integral[1]);
        _driver->bodyRate = Vector2f(cb.msg->rate_gyro_integral[0], cb.msg->rate_gyro_integral[1]);
        _driver->integral_time = cb.msg->integration_interval;
        _driver->surface_quality = cb.msg->quality;
    }
}

void AG_OpticalFlow_HereFlow::update()
{
    _push_state();
}

// Read the sensor
void AG_OpticalFlow_HereFlow::_push_state(void)
{
    WITH_SEMAPHORE(_sem);
    if (!new_data) {
        return;
    }
    struct AG_OpticalFlow::OpticalFlow_state state;
    const Vector2f flowScaler = _flowScaler();
    //setup scaling based on parameters
    float flowScaleFactorX = 1.0f + 0.001f * flowScaler.x;
    float flowScaleFactorY = 1.0f + 0.001f * flowScaler.y;
    float integralToRate = 1.0f / integral_time;
    //Convert to Raw Flow measurement to Flow Rate measurement
    state.flowRate = Vector2f(flowRate.x * flowScaleFactorX,
                                flowRate.y * flowScaleFactorY) * integralToRate;
    state.bodyRate = bodyRate * integralToRate;
    state.surface_quality = surface_quality;
    _applyYaw(state.flowRate);
    _applyYaw(state.bodyRate);
    // hal.console->printf("DRV: %u %f %f\n", state.surface_quality, flowRate.length(), bodyRate.length());
    _update_frontend(state);
    new_data = false;
}

#endif // AP_OPTICALFLOW_HEREFLOW_ENABLED
