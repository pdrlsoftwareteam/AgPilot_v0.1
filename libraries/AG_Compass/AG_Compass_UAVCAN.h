#pragma once

#include "AG_Compass.h"

#if AP_COMPASS_UAVCAN_ENABLED

#include "AG_Compass_Backend.h"

#include <AG_UAVCAN/AG_UAVCAN.h>

class MagCb;
class Mag2Cb;

class AG_Compass_UAVCAN : public AG_Compass_Backend {
public:
    AG_Compass_UAVCAN(AG_UAVCAN* ap_uavcan, uint8_t node_id, uint8_t sensor_id, uint32_t devid);

    void        read(void) override;

    static void subscribe_msgs(AG_UAVCAN* ap_uavcan);
    static AG_Compass_Backend* probe(uint8_t index);
    static uint32_t get_detected_devid(uint8_t index) { return _detected_modules[index].devid; }
    static void handle_magnetic_field(AG_UAVCAN* ap_uavcan, uint8_t node_id, const MagCb &cb);
    static void handle_magnetic_field_2(AG_UAVCAN* ap_uavcan, uint8_t node_id, const Mag2Cb &cb);

private:
    bool init();

    // callback for UAVCAN messages
    void handle_mag_msg(const Vector3f &mag);

    static AG_Compass_UAVCAN* get_uavcan_backend(AG_UAVCAN* ap_uavcan, uint8_t node_id, uint8_t sensor_id);

    uint8_t  _instance;

    AG_UAVCAN* _ap_uavcan;
    uint8_t _node_id;
    uint8_t _sensor_id;
    uint32_t _devid;

    // Module Detection Registry
    static struct DetectedModules {
        AG_UAVCAN* ap_uavcan;
        uint8_t node_id;
        uint8_t sensor_id;
        AG_Compass_UAVCAN *driver;
        uint32_t devid;
    } _detected_modules[COMPASS_MAX_BACKEND];

    static HAL_Semaphore _sem_registry;
};

#endif  // AP_COMPASS_UAVCAN_ENABLED
