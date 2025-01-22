#pragma once

#include "AG_BattMonitor_Backend.h"

#if AP_BATTERY_EFI_ENABLED

class AG_BattMonitor_EFI : public AG_BattMonitor_Backend
{
public:

    // Inherit constructor
    using AG_BattMonitor_Backend::AG_BattMonitor_Backend;

    // update state
    void read(void) override;

    bool has_current(void) const override {
        return true;
    }

    bool has_consumed_energy(void) const override {
        return true;
    }
};
#endif // AP_BATTERY_EFI_ENABLED
