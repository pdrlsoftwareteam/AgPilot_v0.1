/*
 *       Example of AC_Notify library .
 *       DIYDrones.com
 */

#include <AG_HAL/AG_HAL.h>
#include <AG_Notify/AG_Notify.h>          // Notify library
#include <AG_Notify/AG_BoardLED.h>        // Board LED library

void setup();
void loop();

const AG_HAL::HAL& hal = AG_HAL::get_HAL();

// create board led object
AG_BoardLED board_led;

void setup()
{
    hal.console->printf("AG_Notify library test\n");

    // initialise the board leds
    board_led.init();

    // turn on initialising notification
    AG_Notify::flags.initialising = true;
    AG_Notify::flags.gps_status = 1;
    AG_Notify::flags.armed = 1;
    AG_Notify::flags.pre_arm_check = 1;
}

void loop()
{
    hal.scheduler->delay(1000);
}

AG_HAL_MAIN();
