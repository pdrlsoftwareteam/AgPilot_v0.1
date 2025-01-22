/*
  simple test of RC output interface
  Attention: If your board has safety switch,
  don't forget to push it to enable the PWM output.
 */

#include <AG_HAL/AG_HAL.h>

// we need a boardconfig created so that the io processor's enable
// parameter is available
#if CONFIG_HAL_BOARD == HAL_BOARD_CHIBIOS
#include <AG_BoardConfig/AG_BoardConfig.h>
#include <AG_IOMCU/AG_IOMCU.h>
AG_BoardConfig BoardConfig;
#endif

void setup();
void loop();

const AG_HAL::HAL& hal = AG_HAL::get_HAL();

void setup (void)
{
    hal.console->printf("Starting AG_HAL::RCOutput test\n");
#if CONFIG_HAL_BOARD == HAL_BOARD_CHIBIOS
    BoardConfig.init();
#endif
    for (uint8_t i = 0; i< 14; i++) {
        hal.rcout->enable_ch(i);
    }
}

static uint16_t pwm = 1500;
static int8_t delta = 1;

void loop (void)
{
    for (uint8_t i=0; i < 14; i++) {
        hal.rcout->write(i, pwm);
        pwm += delta;
        if (delta > 0 && pwm >= 2000) {
            delta = -1;
            hal.console->printf("decreasing\n");
        } else if (delta < 0 && pwm <= 1000) {
            delta = 1;
            hal.console->printf("increasing\n");
        }
    }
    hal.scheduler->delay(5);
}

AG_HAL_MAIN();
