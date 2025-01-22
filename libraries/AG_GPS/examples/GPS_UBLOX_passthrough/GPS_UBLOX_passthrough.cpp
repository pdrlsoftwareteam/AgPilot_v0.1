/*
 *       GPS UBlox passthrough sketch
 *       Code by DIYDrones.com
 */

#include <stdlib.h>
#include <AG_Common/AG_Common.h>
#include <AG_HAL/AG_HAL.h>

void setup();
void loop();

const AG_HAL::HAL& hal = AG_HAL::get_HAL();

void setup()
{
    // initialise console uart to 38400 baud
    hal.console->begin(38400);

    // initialise gps uart to 38400 baud
    hal.serial(3)->begin(38400);
}

void loop()
{
    // send characters received from the console to the GPS
    while (hal.console->available()) {
        hal.serial(3)->write(hal.console->read());
    }
    // send GPS characters to the console
    while (hal.serial(3)->available()) {
        hal.console->write(hal.serial(3)->read());
    }
}

AG_HAL_MAIN();
