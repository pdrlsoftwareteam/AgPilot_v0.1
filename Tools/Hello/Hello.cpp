/*
  simple hello world sketch
  Andrew Tridgell September 2011
*/

#include <AG_HAL/AG_HAL.h>

void setup();
void loop();

const AG_HAL::HAL& hal = AG_HAL::get_HAL();

void setup()
{
    hal.console->printf("hello world\n");
}

void loop()
{
    hal.scheduler->delay(1000);
    hal.console->printf("*\n");
}

AG_HAL_MAIN();
