#include "AG_Airspeed_UAVCAN.h"

#if AP_AIRSPEED_UAVCAN_ENABLED

#include <AG_CANManager/AG_CANManager.h>
#include <AG_UAVCAN/AG_UAVCAN.h>

#include <uavcan/equipment/air_data/RawAirData.hpp>
#if AP_AIRSPEED_HYGROMETER_ENABLE
#include <dronecan/sensors/hygrometer/Hygrometer.hpp>
#endif
extern const AG_HAL::HAL& hal;

#define LOG_TAG "AirSpeed"

// Frontend Registry Binders
UC_REGISTRY_BINDER(AirspeedCb, uavcan::equipment::air_data::RawAirData);

#if AP_AIRSPEED_HYGROMETER_ENABLE
UC_REGISTRY_BINDER(HygrometerCb, dronecan::sensors::hygrometer::Hygrometer);
#endif

AG_Airspeed_UAVCAN::DetectedModules AG_Airspeed_UAVCAN::_detected_modules[];
HAL_Semaphore AG_Airspeed_UAVCAN::_sem_registry;

// constructor
AG_Airspeed_UAVCAN::AG_Airspeed_UAVCAN(AG_Airspeed &_frontend, uint8_t _instance) :
    AG_Airspeed_Backend(_frontend, _instance)
{}

void AG_Airspeed_UAVCAN::subscribe_msgs(AG_UAVCAN* ap_uavcan)
{
    if (ap_uavcan == nullptr) {
        return;
    }

    auto* node = ap_uavcan->get_node();

    uavcan::Subscriber<uavcan::equipment::air_data::RawAirData, AirspeedCb> *airspeed_listener;
    airspeed_listener = new uavcan::Subscriber<uavcan::equipment::air_data::RawAirData, AirspeedCb>(*node);

    const int airspeed_listener_res = airspeed_listener->start(AirspeedCb(ap_uavcan, &handle_airspeed));
    if (airspeed_listener_res < 0) {
        AG_HAL::panic("DroneCAN Airspeed subscriber error \n");
    }

#if AP_AIRSPEED_HYGROMETER_ENABLE
    uavcan::Subscriber<dronecan::sensors::hygrometer::Hygrometer, HygrometerCb> *hygrometer_listener;
    hygrometer_listener = new uavcan::Subscriber<dronecan::sensors::hygrometer::Hygrometer, HygrometerCb>(*node);
    const int hygrometer_listener_res = hygrometer_listener->start(HygrometerCb(ap_uavcan, &handle_hygrometer));
    if (hygrometer_listener_res < 0) {
        AG_HAL::panic("DroneCAN Hygrometer subscriber error\n");
    }
#endif
}

AG_Airspeed_Backend* AG_Airspeed_UAVCAN::probe(AG_Airspeed &_frontend, uint8_t _instance, uint32_t previous_devid)
{
    WITH_SEMAPHORE(_sem_registry);

    AG_Airspeed_UAVCAN* backend = nullptr;

    for (uint8_t i = 0; i < AIRSPEED_MAX_SENSORS; i++) {
        if (_detected_modules[i].driver == nullptr && _detected_modules[i].ap_uavcan != nullptr) {
            const auto bus_id = AG_HAL::Device::make_bus_id(AG_HAL::Device::BUS_TYPE_UAVCAN,
                                                            _detected_modules[i].ap_uavcan->get_driver_index(),
                                                            _detected_modules[i].node_id, 0);
            if (previous_devid != 0 && previous_devid != bus_id) {
                // match with previous ID only
                continue;
            }
            backend = new AG_Airspeed_UAVCAN(_frontend, _instance);
            if (backend == nullptr) {
                AP::can().log_text(AG_CANManager::LOG_INFO,
                                   LOG_TAG,
                                   "Failed register UAVCAN Airspeed Node %d on Bus %d\n",
                                   _detected_modules[i].node_id,
                                   _detected_modules[i].ap_uavcan->get_driver_index());
            } else {
                _detected_modules[i].driver = backend;
                AP::can().log_text(AG_CANManager::LOG_INFO,
                                   LOG_TAG,
                                   "Registered UAVCAN Airspeed Node %d on Bus %d\n",
                                   _detected_modules[i].node_id,
                                   _detected_modules[i].ap_uavcan->get_driver_index());
                backend->set_bus_id(bus_id);
            }
            break;
        }
    }

    return backend;
}

AG_Airspeed_UAVCAN* AG_Airspeed_UAVCAN::get_uavcan_backend(AG_UAVCAN* ap_uavcan, uint8_t node_id)
{
    if (ap_uavcan == nullptr) {
        return nullptr;
    }

    for (uint8_t i = 0; i < AIRSPEED_MAX_SENSORS; i++) {
        if (_detected_modules[i].driver != nullptr &&
            _detected_modules[i].ap_uavcan == ap_uavcan &&
            _detected_modules[i].node_id == node_id ) {
            return _detected_modules[i].driver;
        }
    }

    bool detected = false;
    for (uint8_t i = 0; i < AIRSPEED_MAX_SENSORS; i++) {
        if (_detected_modules[i].ap_uavcan == ap_uavcan && _detected_modules[i].node_id == node_id) {
            // detected
            detected = true;
            break;
        }
    }

    if (!detected) {
        for (uint8_t i = 0; i < AIRSPEED_MAX_SENSORS; i++) {
            if (_detected_modules[i].ap_uavcan == nullptr) {
                _detected_modules[i].ap_uavcan = ap_uavcan;
                _detected_modules[i].node_id = node_id;
                break;
            }
        }
    }

    return nullptr;
}

void AG_Airspeed_UAVCAN::handle_airspeed(AG_UAVCAN* ap_uavcan, uint8_t node_id, const AirspeedCb &cb)
{
    WITH_SEMAPHORE(_sem_registry);

    AG_Airspeed_UAVCAN* driver = get_uavcan_backend(ap_uavcan, node_id);

    if (driver != nullptr) {
        WITH_SEMAPHORE(driver->_sem_airspeed);
        driver->_pressure = cb.msg->differential_pressure;
        if (!isnan(cb.msg->static_air_temperature) &&
            cb.msg->static_air_temperature > 0) {
            driver->_temperature = KELVIN_TO_C(cb.msg->static_air_temperature);
            driver->_have_temperature = true;
        }
        driver->_last_sample_time_ms = AG_HAL::millis();
    }
}

#if AP_AIRSPEED_HYGROMETER_ENABLE
void AG_Airspeed_UAVCAN::handle_hygrometer(AG_UAVCAN* ap_uavcan, uint8_t node_id, const HygrometerCb &cb)
{
    WITH_SEMAPHORE(_sem_registry);

    AG_Airspeed_UAVCAN* driver = get_uavcan_backend(ap_uavcan, node_id);

    if (driver != nullptr) {
        WITH_SEMAPHORE(driver->_sem_airspeed);
        driver->_hygrometer.temperature = KELVIN_TO_C(cb.msg->temperature);
        driver->_hygrometer.humidity = cb.msg->humidity;
        driver->_hygrometer.last_sample_ms = AG_HAL::millis();
    }
}
#endif // AP_AIRSPEED_HYGROMETER_ENABLE

bool AG_Airspeed_UAVCAN::init()
{
    // always returns true
    return true;
}

bool AG_Airspeed_UAVCAN::get_differential_pressure(float &pressure)
{
    WITH_SEMAPHORE(_sem_airspeed);

    if ((AG_HAL::millis() - _last_sample_time_ms) > 250) {
        return false;
    }

    pressure = _pressure;

    return true;
}

bool AG_Airspeed_UAVCAN::get_temperature(float &temperature)
{
    if (!_have_temperature) {
        return false;
    }
    WITH_SEMAPHORE(_sem_airspeed);

    if ((AG_HAL::millis() - _last_sample_time_ms) > 100) {
        return false;
    }

    temperature = _temperature;

    return true;
}

#if AP_AIRSPEED_HYGROMETER_ENABLE
/*
  return hygrometer data if available
 */
bool AG_Airspeed_UAVCAN::get_hygrometer(uint32_t &last_sample_ms, float &temperature, float &humidity)
{
    if (_hygrometer.last_sample_ms == 0) {
        return false;
    }
    WITH_SEMAPHORE(_sem_airspeed);
    last_sample_ms = _hygrometer.last_sample_ms;
    temperature = _hygrometer.temperature;
    humidity = _hygrometer.humidity;
    return true;
}
#endif // AP_AIRSPEED_HYGROMETER_ENABLE

#endif // AP_AIRSPEED_UAVCAN_ENABLED
