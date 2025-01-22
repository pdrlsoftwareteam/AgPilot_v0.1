#pragma once

#include <AG_Param/AG_Param.h>
#include <AG_Math/AG_Math.h>

class MovingBase {
public:
    static const struct AG_Param::GroupInfo var_info[];

    MovingBase(void);

    /* Do not allow copies */
    CLASS_NO_COPY(MovingBase);

    enum class Type : int8_t {
        RelativeToAlternateInstance = 0,
        RelativeToCustomBase        = 1,
    };

    AP_Int8 type;            // an option from MovingBaseType
    AP_Vector3f base_offset; // base position offset from the selected GPS reciever

};
