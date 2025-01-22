#pragma once

#include "AG_BattMonitor_SMBus.h"

#if AP_BATTERY_SMBUS_SUI_ENABLED

// Base SUI class
class AG_BattMonitor_SMBus_SUI : public AG_BattMonitor_SMBus
{
public:

    // Constructor
    AG_BattMonitor_SMBus_SUI(AG_BattMonitor &mon,
                             AG_BattMonitor::BattMonitor_State &mon_state,
                             AG_BattMonitor_Params &params,
                             uint8_t cell_count
                            );

    void init(void) override;

private:
    void timer(void) override;
    void read_cell_voltages();
    void update_health();

    // read_block_bare - returns number of characters read if successful, zero if unsuccessful
    bool read_block_bare(uint8_t reg, uint8_t* data, uint8_t len) const;

    const uint8_t cell_count;
    bool phase_voltages;
    uint32_t last_volt_read_us;
};

#endif  // AP_BATTERY_SMBUS_SUI_ENABLED
