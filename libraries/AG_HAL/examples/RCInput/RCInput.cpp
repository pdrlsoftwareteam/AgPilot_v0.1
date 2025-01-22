/*
  simple test of RC input interface
 */
#include <AG_Common/AG_Common.h>
#include <AG_HAL/AG_HAL.h>

// we need a boardconfig created so that the io processor's enable
// parameter is available
#if HAL_WITH_IO_MCU
#include <AG_BoardConfig/AG_BoardConfig.h>
#include <AG_IOMCU/AG_IOMCU.h>
AG_BoardConfig BoardConfig;
#endif

void setup();
void loop();

const AG_HAL::HAL& hal = AG_HAL::get_HAL();

#define MAX_CHANNELS 16

static uint8_t max_channels_display = 8;  // Set to 0 for display numbers of channels detected.
static uint16_t last_value[MAX_CHANNELS];

void setup(void)
{
    hal.console->printf("Starting RCInput test\n");
#if HAL_WITH_IO_MCU
    BoardConfig.init();
#endif
}

void read_channels(void);
void read_channels(void)
{
    uint8_t nchannels = hal.rcin->num_channels();  // Get the numbers channels detected by RC_INPUT.
    if (nchannels == 0) {
        hal.console->printf("No channels detected\n");
        return;
    }
    if (max_channels_display == 0) {
        hal.console->printf("Channels detected: %2u\n", nchannels);
        hal.console->printf("Set max_channels_display > 0 to display channels values\n");
        return;
    }

    if (nchannels > MAX_CHANNELS) {
        nchannels = MAX_CHANNELS;
    }

    bool changed = false;
    for (uint8_t i = 0; i < nchannels; i++) {
        uint16_t v = hal.rcin->read(i);
        if (last_value[i] != v) {
            changed = true;
            last_value[i] = v;
        }
    }
    if (max_channels_display > nchannels) {
        max_channels_display = nchannels;
    }

    if (changed) {
        for (uint8_t i = 0; i < max_channels_display; i++) {
            hal.console->printf("%2u:%04u ", (unsigned)i+1, (unsigned)last_value[i]);
        }
        hal.console->printf("\n");
    }
}

void loop(void) {
    read_channels();
    hal.scheduler->delay(100);
}

AG_HAL_MAIN();
