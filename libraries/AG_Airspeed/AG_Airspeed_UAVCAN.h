#pragma once

#include <AG_HAL/AG_HAL_Boards.h>

#ifndef AP_AIRSPEED_UAVCAN_ENABLED
#define AP_AIRSPEED_UAVCAN_ENABLED HAL_ENABLE_LIBUAVCAN_DRIVERS
#endif

#if AP_AIRSPEED_UAVCAN_ENABLED

#include "AG_Airspeed_Backend.h"

#include <AG_UAVCAN/AG_UAVCAN.h>

class AirspeedCb;
class HygrometerCb;

class AG_Airspeed_UAVCAN : public AG_Airspeed_Backend {
public:
    AG_Airspeed_UAVCAN(AG_Airspeed &_frontend, uint8_t _instance);

    bool init(void) override;

    // return the current differential_pressure in Pascal
    bool get_differential_pressure(float &pressure) override;

    // temperature not available via analog backend
    bool get_temperature(float &temperature) override;

#if AP_AIRSPEED_HYGROMETER_ENABLE
    // get hygrometer data
    bool get_hygrometer(uint32_t &last_sample_ms, float &temperature, float &humidity) override;
#endif

    static void subscribe_msgs(AG_UAVCAN* ap_uavcan);

    static AG_Airspeed_Backend* probe(AG_Airspeed &_fronted, uint8_t _instance, uint32_t previous_devid);

private:

    static void handle_airspeed(AG_UAVCAN* ap_uavcan, uint8_t node_id, const AirspeedCb &cb);
    static void handle_hygrometer(AG_UAVCAN* ap_uavcan, uint8_t node_id, const HygrometerCb &cb);

    static AG_Airspeed_UAVCAN* get_uavcan_backend(AG_UAVCAN* ap_uavcan, uint8_t node_id);

    float _pressure; // Pascal
    float _temperature; // Celcius
    uint32_t _last_sample_time_ms;

    // hygrometer data
    struct {
        float temperature;
        float humidity;
        uint32_t last_sample_ms;
    } _hygrometer;

    HAL_Semaphore _sem_airspeed;

    // Module Detection Registry
    static struct DetectedModules {
        AG_UAVCAN* ap_uavcan;
        uint8_t node_id;
        AG_Airspeed_UAVCAN *driver;
    } _detected_modules[AIRSPEED_MAX_SENSORS];

    static HAL_Semaphore _sem_registry;
    bool _have_temperature;
};


#endif  // AP_AIRSPEED_UAVCAN_ENABLED
