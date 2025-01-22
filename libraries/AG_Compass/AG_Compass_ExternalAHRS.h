#pragma once

#include "AG_Compass_config.h"

#if AP_COMPASS_EXTERNALAHRS_ENABLED

#include "AG_Compass.h"
#include "AG_Compass_Backend.h"
#include <AG_ExternalAHRS/AG_ExternalAHRS.h>

class AG_Compass_ExternalAHRS : public AG_Compass_Backend
{
public:
    AG_Compass_ExternalAHRS(uint8_t instance);

    void read(void) override;

private:
    void handle_external(const AG_ExternalAHRS::mag_data_message_t &pkt) override;
    uint8_t instance;
};

#endif  // AP_COMPASS_EXTERNALAHRS_ENABLED
