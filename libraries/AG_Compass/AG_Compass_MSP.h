#pragma once

#include "AG_Compass_config.h"

#if AP_COMPASS_MSP_ENABLED

#include "AG_Compass.h"
#include "AG_Compass_Backend.h"
#include <AG_MSP/msp.h>

class AG_Compass_MSP : public AG_Compass_Backend
{
public:
    AG_Compass_MSP(uint8_t msp_instance);

    void read(void) override;

private:
    void handle_msp(const MSP::msp_compass_data_message_t &pkt) override;
    uint8_t msp_instance;
    uint8_t instance;
};

#endif // AP_COMPASS_MSP_ENABLED
