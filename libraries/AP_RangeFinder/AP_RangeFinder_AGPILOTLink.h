#pragma once

#include "AP_RangeFinder.h"
#include "AP_RangeFinder_Backend.h"

#ifndef AP_RANGEFINDER_AGPILOTLINK_ENABLED
#define AP_RANGEFINDER_AGPILOTLINK_ENABLED AP_RANGEFINDER_BACKEND_DEFAULT_ENABLED
#endif

#if AP_RANGEFINDER_AGPILOTLINK_ENABLED

// Data timeout
#define AP_RANGEFINDER_AGPILOTLINK_TIMEOUT_MS 500

class AP_RangeFinder_AGPILOTLink : public AP_RangeFinder_Backend
{

public:

    // constructor
    using AP_RangeFinder_Backend::AP_RangeFinder_Backend;

    // Assume that if the user set the RANGEFINDER_TYPE parameter to AGPILOTLink,
    // there is an attached AGPILOTLink rangefinder
    static bool detect() { return true; }

    // update state
    void update(void) override;

    // Get update from mavlink
    void handle_msg(const mavlink_message_t &msg) override;

    int16_t max_distance_cm() const override;
    int16_t min_distance_cm() const override;

protected:

    AGPILOT_DISTANCE_SENSOR _get_mav_distance_sensor_type() const override {
        return sensor_type;
    }

private:

    // stored data from packet:
    uint16_t distance_cm;
    uint16_t _max_distance_cm;
    uint16_t _min_distance_cm;

    // start a reading
    static bool start_reading(void);
    static bool get_reading(uint16_t &reading_cm);

    AGPILOT_DISTANCE_SENSOR sensor_type = AGPILOT_DISTANCE_SENSOR_UNKNOWN;
};

#endif
