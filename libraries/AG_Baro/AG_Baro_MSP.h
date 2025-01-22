/*
  MSP backend barometer
 */
#pragma once

#include "AG_Baro_Backend.h"

// AP_BARO_MSP_ENABLED is defined in AG_Baro.h

#if AP_BARO_MSP_ENABLED

#define MOVING_AVERAGE_WEIGHT 0.20f // a 5 samples moving average

class AG_Baro_MSP : public AG_Baro_Backend
{
public:
    AG_Baro_MSP(AG_Baro &baro, uint8_t msp_instance);
    void update(void) override;
    void handle_msp(const MSP::msp_baro_data_message_t &pkt) override;

private:
    uint8_t instance;
    uint8_t msp_instance;
    float sum_pressure;
    float sum_temp;
    uint16_t count;
};

#endif // AP_BARO_MSP_ENABLED
