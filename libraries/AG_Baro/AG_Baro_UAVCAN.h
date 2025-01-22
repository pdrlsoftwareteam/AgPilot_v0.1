#pragma once

#include "AG_Baro_Backend.h"

#if AP_BARO_UAVCAN_ENABLED

#include <AG_UAVCAN/AG_UAVCAN.h>

class PressureCb;
class TemperatureCb;

class AG_Baro_UAVCAN : public AG_Baro_Backend {
public:
    AG_Baro_UAVCAN(AG_Baro &baro);

    void update() override;

    static void subscribe_msgs(AG_UAVCAN* ap_uavcan);
    static AG_Baro_UAVCAN* get_uavcan_backend(AG_UAVCAN* ap_uavcan, uint8_t node_id, bool create_new);
    static AG_Baro_Backend* probe(AG_Baro &baro);

    static void handle_pressure(AG_UAVCAN* ap_uavcan, uint8_t node_id, const PressureCb &cb);
    static void handle_temperature(AG_UAVCAN* ap_uavcan, uint8_t node_id, const TemperatureCb &cb);

private:

    static void _update_and_wrap_accumulator(float *accum, float val, uint8_t *count, const uint8_t max_count);

    uint8_t _instance;

    bool new_pressure;
    float _pressure;
    float _temperature;
    uint8_t  _pressure_count;
    HAL_Semaphore _sem_baro;

    AG_UAVCAN* _ap_uavcan;
    uint8_t _node_id;

    // Module Detection Registry
    static struct DetectedModules {
        AG_UAVCAN* ap_uavcan;
        uint8_t node_id;
        AG_Baro_UAVCAN* driver;
    } _detected_modules[BARO_MAX_DRIVERS];

    static HAL_Semaphore _sem_registry;
};

#endif  // AP_BARO_UAVCAN_ENABLED
