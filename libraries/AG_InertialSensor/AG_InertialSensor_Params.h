#pragma once

#include <AG_Param/AG_Param.h>
#include <AG_Math/AG_Math.h>
#include "AG_InertialSensor_tempcal.h"
#include "AG_InertialSensor_config.h"

class AG_InertialSensor_Params {
public:
    static const struct AG_Param::GroupInfo var_info[];

    AG_InertialSensor_Params(void);

    /* Do not allow copies */
    CLASS_NO_COPY(AG_InertialSensor_Params);

    AP_Int32 _accel_id;
    AP_Vector3f _accel_scale;
    AP_Vector3f _accel_offset;
    AP_Vector3f _accel_pos;
    AP_Float caltemp_accel;

    AP_Int32 _gyro_id;
    AP_Vector3f _gyro_offset;
    AP_Float caltemp_gyro;

    AP_Int8 _use;

#if HAL_INS_TEMPERATURE_CAL_ENABLE
    AG_InertialSensor_TCal tcal;
#endif
};
