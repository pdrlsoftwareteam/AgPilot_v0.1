#pragma once

#include "AG_Compass.h"

#if AP_COMPASS_SITL_ENABLED

#include "AG_Compass_Backend.h"

#include <AG_Math/vectorN.h>
#include <AG_Math/AG_Math.h>
#include <AG_Declination/AG_Declination.h>
#include <SITL/SITL.h>

#define MAX_SITL_COMPASSES 3

class AG_Compass_SITL : public AG_Compass_Backend {
public:
    AG_Compass_SITL();

    void read(void) override;

private:
    uint8_t _compass_instance[MAX_SITL_COMPASSES];
    uint8_t _num_compass;
    SITL::SIM *_sitl;

    // delay buffer variables
    struct readings_compass {
        uint32_t time;
        Vector3f data;
    };
    uint8_t store_index;
    uint32_t last_store_time;
    static const uint8_t buffer_length = 50;
    VectorN<readings_compass,buffer_length> buffer;

    void _timer();
    uint32_t _last_sample_time;

    void _setup_eliptical_correcion(uint8_t i);
    
    Matrix3f _eliptical_corr;
    Vector3f _last_dia;
    Vector3f _last_odi;
    Vector3f _last_data[MAX_SITL_COMPASSES];
};
#endif // AP_COMPASS_SITL_ENABLED
