/*
  simple test of UART interfaces
 */

#include <AG_HAL/AG_HAL.h>
#include <AG_Beacon/AG_Beacon_Marvelmind.h>
#include <AG_Beacon/AG_Beacon.h>
#include <AG_SerialManager/AG_SerialManager.h>
#include <stdio.h>

void setup();
void loop();
void set_object_value_and_report(const void *object_pointer,
                      const struct AG_Param::GroupInfo *group_info,
                      const char *name, float value);

const AG_HAL::HAL& hal = AG_HAL::get_HAL();
static AG_SerialManager serial_manager;
AG_Beacon beacon;

// try to set the object value but provide diagnostic if it failed
void set_object_value_and_report(const void *object_pointer,
                      const struct AG_Param::GroupInfo *group_info,
                      const char *name, float value)
{
    if (!AG_Param::set_object_value(object_pointer, group_info, name, value)) {
        printf("WARNING: AG_Param::set object value \"%s::%s\" Failed.\n",
                            group_info->name, name);
    }
}

void setup(void)
{
    set_object_value_and_report(&beacon, beacon.var_info, "_TYPE", 2.0f);
    set_object_value_and_report(&serial_manager, serial_manager.var_info, "0_PROTOCOL", 13.0f);
    serial_manager.init();
    beacon.init();
}

void loop(void)
{
    static int count = 0;
    beacon.update();
    Vector3f pos;
    float accuracy = 0.0f;
    beacon.get_vehicle_position_ned(pos, accuracy);
    if (pos.x > 0.001f) {
        printf("%f %f %f\n", static_cast<double>(pos.x), static_cast<double>(pos.y), static_cast<double>(pos.z));
        count++;
    }
    hal.scheduler->delay(1000);
    if (count == 3)
        exit(0);
}

AG_HAL_MAIN();
