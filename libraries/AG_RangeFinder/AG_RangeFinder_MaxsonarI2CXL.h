#pragma once

#include "AG_RangeFinder.h"
#include "AG_RangeFinder_Backend.h"

#ifndef AP_RANGEFINDER_MAXSONARI2CXL_ENABLED
#define AP_RANGEFINDER_MAXSONARI2CXL_ENABLED AP_RANGEFINDER_BACKEND_DEFAULT_ENABLED
#endif

#if AP_RANGEFINDER_MAXSONARI2CXL_ENABLED

#include <AG_HAL/I2CDevice.h>

#define AP_RANGE_FINDER_MAXSONARI2CXL_DEFAULT_ADDR   0x70
#define AP_RANGE_FINDER_MAXSONARI2CXL_COMMAND_TAKE_RANGE_READING 0x51

class AG_RangeFinder_MaxsonarI2CXL : public AG_RangeFinder_Backend
{
public:
    // static detection function
    static AG_RangeFinder_Backend *detect(RangeFinder::RangeFinder_State &_state,
                                          AG_RangeFinder_Params &_params,
                                          AG_HAL::OwnPtr<AG_HAL::I2CDevice> dev);

    // update state
    void update(void) override;

protected:

    MAV_DISTANCE_SENSOR _get_mav_distance_sensor_type() const override {
        return MAV_DISTANCE_SENSOR_ULTRASOUND;
    }

private:
    // constructor
    AG_RangeFinder_MaxsonarI2CXL(RangeFinder::RangeFinder_State &_state,
    								AG_RangeFinder_Params &_params,
                                 AG_HAL::OwnPtr<AG_HAL::I2CDevice> dev);

    bool _init(void);
    void _timer(void);

    uint16_t distance;
    bool new_distance;
    
    // start a reading
    bool start_reading(void);
    bool get_reading(uint16_t &reading_cm);
    AG_HAL::OwnPtr<AG_HAL::I2CDevice> _dev;
};

#endif  // AP_RANGEFINDER_MAXSONARI2CXL_ENABLED
