#include "AG_Baro_UAVCAN.h"

#if AP_BARO_UAVCAN_ENABLED

#include <AG_CANManager/AG_CANManager.h>
#include <AG_UAVCAN/AG_UAVCAN.h>

#include <uavcan/equipment/air_data/StaticPressure.hpp>
#include <uavcan/equipment/air_data/StaticTemperature.hpp>

extern const AG_HAL::HAL& hal;

#define LOG_TAG "Baro"

//UAVCAN Frontend Registry Binder
UC_REGISTRY_BINDER(PressureCb, uavcan::equipment::air_data::StaticPressure);
UC_REGISTRY_BINDER(TemperatureCb, uavcan::equipment::air_data::StaticTemperature);

AG_Baro_UAVCAN::DetectedModules AG_Baro_UAVCAN::_detected_modules[];
HAL_Semaphore AG_Baro_UAVCAN::_sem_registry;

/*
  constructor - registers instance at top Baro driver
 */
AG_Baro_UAVCAN::AG_Baro_UAVCAN(AG_Baro &baro) :
    AG_Baro_Backend(baro)
{}

void AG_Baro_UAVCAN::subscribe_msgs(AG_UAVCAN* ap_uavcan)
{
    if (ap_uavcan == nullptr) {
        return;
    }

    auto* node = ap_uavcan->get_node();

    uavcan::Subscriber<uavcan::equipment::air_data::StaticPressure, PressureCb> *pressure_listener;
    pressure_listener = new uavcan::Subscriber<uavcan::equipment::air_data::StaticPressure, PressureCb>(*node);
    // Msg Handler
    const int pressure_listener_res = pressure_listener->start(PressureCb(ap_uavcan, &handle_pressure));
    if (pressure_listener_res < 0) {
        AG_HAL::panic("UAVCAN Baro subscriber start problem\n\r");
    }

    uavcan::Subscriber<uavcan::equipment::air_data::StaticTemperature, TemperatureCb> *temperature_listener;
    temperature_listener = new uavcan::Subscriber<uavcan::equipment::air_data::StaticTemperature, TemperatureCb>(*node);
    // Msg Handler
    const int temperature_listener_res = temperature_listener->start(TemperatureCb(ap_uavcan, &handle_temperature));
    if (temperature_listener_res < 0) {
        AG_HAL::panic("UAVCAN Baro subscriber start problem\n\r");
    }
}

AG_Baro_Backend* AG_Baro_UAVCAN::probe(AG_Baro &baro)
{
    WITH_SEMAPHORE(_sem_registry);

    AG_Baro_UAVCAN* backend = nullptr;
    for (uint8_t i = 0; i < BARO_MAX_DRIVERS; i++) {
        if (_detected_modules[i].driver == nullptr && _detected_modules[i].ap_uavcan != nullptr) {
            backend = new AG_Baro_UAVCAN(baro);
            if (backend == nullptr) {
                AP::can().log_text(AG_CANManager::LOG_ERROR,
                            LOG_TAG,
                            "Failed register UAVCAN Baro Node %d on Bus %d\n",
                            _detected_modules[i].node_id,
                            _detected_modules[i].ap_uavcan->get_driver_index());
            } else {
                _detected_modules[i].driver = backend;
                backend->_pressure = 0;
                backend->_pressure_count = 0;
                backend->_ap_uavcan = _detected_modules[i].ap_uavcan;
                backend->_node_id = _detected_modules[i].node_id;

                backend->_instance = backend->_frontend.register_sensor();
                backend->set_bus_id(backend->_instance, AG_HAL::Device::make_bus_id(AG_HAL::Device::BUS_TYPE_UAVCAN,
                                                                                    _detected_modules[i].ap_uavcan->get_driver_index(),
                                                                                    backend->_node_id, 0));

                AP::can().log_text(AG_CANManager::LOG_INFO,
                            LOG_TAG,
                            "Registered UAVCAN Baro Node %d on Bus %d\n",
                            _detected_modules[i].node_id,
                            _detected_modules[i].ap_uavcan->get_driver_index());
            }
            break;
        }
    }
    return backend;
}

AG_Baro_UAVCAN* AG_Baro_UAVCAN::get_uavcan_backend(AG_UAVCAN* ap_uavcan, uint8_t node_id, bool create_new)
{
    if (ap_uavcan == nullptr) {
        return nullptr;
    }
    for (uint8_t i = 0; i < BARO_MAX_DRIVERS; i++) {
        if (_detected_modules[i].driver != nullptr &&
            _detected_modules[i].ap_uavcan == ap_uavcan && 
            _detected_modules[i].node_id == node_id) {
            return _detected_modules[i].driver;
        }
    }
    
    if (create_new) {
        bool already_detected = false;
        //Check if there's an empty spot for possible registeration
        for (uint8_t i = 0; i < BARO_MAX_DRIVERS; i++) {
            if (_detected_modules[i].ap_uavcan == ap_uavcan && _detected_modules[i].node_id == node_id) {
                //Already Detected
                already_detected = true;
                break;
            }
        }
        if (!already_detected) {
            for (uint8_t i = 0; i < BARO_MAX_DRIVERS; i++) {
                if (_detected_modules[i].ap_uavcan == nullptr) {
                    _detected_modules[i].ap_uavcan = ap_uavcan;
                    _detected_modules[i].node_id = node_id;
                    break;
                }
            }
        }
    }

    return nullptr;
}


void AG_Baro_UAVCAN::_update_and_wrap_accumulator(float *accum, float val, uint8_t *count, const uint8_t max_count)
{
    *accum += val;
    *count += 1;
    if (*count == max_count) {
        *count = max_count / 2;
        *accum = *accum / 2;
    }
}

void AG_Baro_UAVCAN::handle_pressure(AG_UAVCAN* ap_uavcan, uint8_t node_id, const PressureCb &cb)
{
    AG_Baro_UAVCAN* driver;
    {
        WITH_SEMAPHORE(_sem_registry);
        driver = get_uavcan_backend(ap_uavcan, node_id, true);
        if (driver == nullptr) {
            return;
        }
    }
    {
        WITH_SEMAPHORE(driver->_sem_baro);
        _update_and_wrap_accumulator(&driver->_pressure, cb.msg->static_pressure, &driver->_pressure_count, 32);
        driver->new_pressure = true;
    }
}

void AG_Baro_UAVCAN::handle_temperature(AG_UAVCAN* ap_uavcan, uint8_t node_id, const TemperatureCb &cb)
{
    AG_Baro_UAVCAN* driver;
    {
        WITH_SEMAPHORE(_sem_registry);
        driver = get_uavcan_backend(ap_uavcan, node_id, false);
        if (driver == nullptr) {
            return;
        }
    }
    {
        WITH_SEMAPHORE(driver->_sem_baro);
        driver->_temperature = KELVIN_TO_C(cb.msg->static_temperature);
    }
}

// Read the sensor
void AG_Baro_UAVCAN::update(void)
{
    float pressure = 0;

    WITH_SEMAPHORE(_sem_baro);
    if (new_pressure) {
        if (_pressure_count != 0) {
            pressure = _pressure / _pressure_count;
            _pressure_count = 0;
            _pressure = 0;
        }
        _copy_to_frontend(_instance, pressure, _temperature);


        _frontend.set_external_temperature(_temperature);
        new_pressure = false;
    }
}

#endif // AP_BARO_UAVCAN_ENABLED
