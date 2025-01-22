
#include <AG_HAL/AG_HAL.h>
#include <AG_ONVIF/AG_ONVIF.h> 
// #include <DeviceBinding.nsmap>
// #include <MediaBinding.nsmap>
// #include <PTZBinding.nsmap>

void setup();
void loop();
const AG_HAL::HAL& hal = AG_HAL::get_HAL();

#if ENABLE_ONVIF

AG_ONVIF onvif;

void setup()
{
    printf("AG_ONVIF library test\n");
    if (!onvif.init()) {
        AG_HAL::panic("Failed to initialise onvif");
    }
}

void loop()
{
    static float pan = 0.0, tilt = 0.0;
    static bool move_up;
    printf("Sending: %f %f\n", pan, tilt);
    onvif.set_absolutemove(pan, tilt, 0);
    if (pan < 1.0 && move_up) {
        pan += 0.1;
        tilt += 0.1;
    } else if(pan > -1.0 && !move_up) {
        pan -= 0.1;
        tilt -= 0.1;
    }
    if (pan >= 1.0 && move_up) {
        move_up = false;
    }
    if (pan <= -1.0 && !move_up) {
        move_up = true;
    }
    hal.scheduler->delay(10000);
}
#else
void setup() {}
void loop() {}
#endif
AG_HAL_MAIN();
