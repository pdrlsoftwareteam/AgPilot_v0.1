/*
  simple test of Storage API
 */

#include <AG_HAL/AG_HAL.h>

void setup();
void loop();

const AG_HAL::HAL& hal = AG_HAL::get_HAL();

void setup(void) 
{
    /*
      init Storage API
     */
    AG_HAL::Storage *st = hal.storage;

    hal.console->printf("Starting AG_HAL::Storage test\r\n");
    st->init();

    /*
      Calculate XOR of the full conent of memory
      Do it by block of 8 bytes
    */
    unsigned char buff[8], XOR_res = 0;

    for (uint32_t i = 0; i < HAL_STORAGE_SIZE; i += 8) {
        st->read_block((void *) buff, i, 8);
        for(uint32_t j = 0; j < 8; j++) {
            XOR_res ^= buff[j];
        }
    }

    /*
      print XORed result
     */
    hal.console->printf("XORed ememory: %u\r\n", (unsigned) XOR_res);
}

// In main loop do nothing
void loop(void) 
{	
    hal.scheduler->delay(1000);
}

AG_HAL_MAIN();
