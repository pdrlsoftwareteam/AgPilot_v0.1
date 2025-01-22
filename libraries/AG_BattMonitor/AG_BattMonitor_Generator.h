#pragma once

#include <AG_Generator/AG_Generator.h>

#if HAL_GENERATOR_ENABLED

#include "AG_BattMonitor.h"
#include "AG_BattMonitor_Backend.h"

// Sub class for generator electrical
class AG_BattMonitor_Generator_Elec : public AG_BattMonitor_Backend
{
public:

    // Inherit constructor
    using AG_BattMonitor_Backend::AG_BattMonitor_Backend;

    void init(void) override {};

    // Read the battery voltage and current
    void read(void) override;

    bool has_current(void) const override;

    bool has_consumed_energy(void) const override;

    // Override backend update_failsafes.  No point in failsafing twice so generator failsafes are only updated from the electrical instance of the generator drivers
    AG_BattMonitor::Failsafe update_failsafes() override;
};

// Sub class for generator fuel
class AG_BattMonitor_Generator_FuelLevel : public AG_BattMonitor_Backend
{
public:

    // Inherit constructor
    using AG_BattMonitor_Backend::AG_BattMonitor_Backend;

    void init(void) override;

    // Read the fuel level
    void read(void) override;

    // This is where we tell the battery monitor 'we have current' if we want to report a fuel level remaining
    bool has_current(void) const override;

    // This is where we tell the battery monitor 'we have consummed energy' if we want to report a fuel level remaining
    bool has_consumed_energy(void) const override;
};
#endif
