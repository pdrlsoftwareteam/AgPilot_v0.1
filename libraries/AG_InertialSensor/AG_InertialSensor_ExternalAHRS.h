#pragma once

#include <AG_ExternalAHRS/AG_ExternalAHRS.h>

#if HAL_EXTERNAL_AHRS_ENABLED

#include "AG_InertialSensor.h"
#include "AG_InertialSensor_Backend.h"

class AG_InertialSensor_ExternalAHRS : public AG_InertialSensor_Backend
{
public:
    AG_InertialSensor_ExternalAHRS(AG_InertialSensor &imu, uint8_t serial_port);

    /* update accel and gyro state */
    bool update() override;
    void start() override;
    void accumulate() override;

    void handle_external(const AG_ExternalAHRS::ins_data_message_t &pkt) override;
    bool get_output_banner(char* banner, uint8_t banner_len) override;

private:
    uint8_t gyro_instance;
    uint8_t accel_instance;
    const uint8_t serial_port;
    bool started;
};
#endif // HAL_EXTERNAL_AHRS_ENABLED

