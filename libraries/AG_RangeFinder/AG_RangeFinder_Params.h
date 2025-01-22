#pragma once

#include <AG_Param/AG_Param.h>
#include <AG_Math/AG_Math.h>

class AG_RangeFinder_Params {
public:
    static const struct AG_Param::GroupInfo var_info[];

    AG_RangeFinder_Params(void);

    /* Do not allow copies */
    CLASS_NO_COPY(AG_RangeFinder_Params);

    AP_Vector3f pos_offset; // position offset in body frame
    AP_Float scaling;
    AP_Float offset;
    AP_Int16 powersave_range;
    AP_Int16 min_distance_cm;
    AP_Int16 max_distance_cm;
    AP_Int8  type;
    AP_Int8  pin;
    AP_Int8  ratiometric;
    AP_Int8  stop_pin;
    AP_Int8  function;
    AP_Int8  ground_clearance_cm;
    AP_Int8  address;
    AP_Int8  orientation;
};
