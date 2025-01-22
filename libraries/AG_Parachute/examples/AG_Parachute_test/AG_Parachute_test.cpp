/*
 *       Example of AG_Parachute library.
 *       DIYDrones.com
 */

#include <AG_Common/AG_Common.h>
#include <AG_Math/AG_Math.h>            // ArduPilot Mega Vector/Matrix math Library
#include <AG_Param/AG_Param.h>
#include <AG_HAL/AG_HAL.h>
#include <RC_Channel/RC_Channel.h>
#include <AG_Relay/AG_Relay.h>
#include <AG_Parachute/AG_Parachute.h>
#include <AG_Notify/AG_Notify.h>
#include <StorageManager/StorageManager.h>

void setup();
void loop();

const AG_HAL::HAL& hal = AG_HAL::get_HAL();

// Relay
static AG_Relay relay;

#if HAL_PARACHUTE_ENABLED
// Parachute
static AG_Parachute parachute;
#endif

void setup()
{
    hal.console->printf("AG_Parachute library test\n");
}

void loop()
{
    // print message to user
    hal.console->printf("this example tests compilation only");
    hal.scheduler->delay(5000);
}

AG_HAL_MAIN();
