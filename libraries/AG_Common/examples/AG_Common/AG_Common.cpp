//
// Unit tests for the AG_Common code
//

#include <AG_Common/AG_Common.h>
#include <AG_HAL/AG_HAL.h>

void setup();
void loop();
void test_high_low_byte(void);

const AG_HAL::HAL& hal = AG_HAL::get_HAL();

void test_high_low_byte(void)
{

    // test each value from 0 to 300
    for (uint16_t i = 0; i <= 300; i++) {
        uint8_t high = HIGHBYTE(i);
        uint8_t low = LOWBYTE(i);
        hal.console->printf("\ni:%u high:%u low:%u", (unsigned int)i, (unsigned int)high, (unsigned int)low);
    }

    // test values from 300 to 65400 at increments of 200
    for (uint16_t i = 301; i <= 65400; i += 200) {
        uint8_t high = HIGHBYTE(i);
        uint8_t low = LOWBYTE(i);
        hal.console->printf("\ni:%u high:%u low:%u", (unsigned int)i, (unsigned int)high, (unsigned int)low);
    }
}

/*
 *  euler angle tests
 */
void setup(void)
{
    hal.console->printf("AG_Common tests\n\n");

    test_high_low_byte();
}

void loop(void)
{
    // do nothing
}

AG_HAL_MAIN();
