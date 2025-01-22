#pragma once

#include <AG_WPNav/AG_WPNav.h>
#include <AG_Avoidance/AG_OAGathPlanner.h>
#include <AG_Avoidance/AG_OABendyRuler.h>

class AG_WPNav_OA : public AG_WPNav
{

public:
    /// Constructor
    AG_WPNav_OA(const AG_InertialNav& inav, const AG_AHRS_View& ahrs, AG_PosControl& pos_control, const AG_AttitudeControl& attitude_control);

    // returns object avoidance adjusted wp location using location class
    // returns false if unable to convert from target vector to global coordinates
    bool get_oa_wp_destination(Location& destination) const override;

    /// set_wp_destination waypoint using position vector (distance from ekf origin in cm)
    ///     terrain_alt should be true if destination.z is a desired altitude above terrain
    ///     returns false on failure (likely caused by missing terrain data)
    bool set_wp_destination(const Vector3f& destination, bool terrain_alt = false) override;

    /// get horizontal distance to destination in cm
    /// always returns distance to final destination (i.e. does not use oa adjusted destination)
    float get_wp_distance_to_destination() const override;

    /// get bearing to next waypoint in centi-degrees
    /// always returns bearing to final destination (i.e. does not use oa adjusted destination)
    int32_t get_wp_bearing_to_destination() const override;

    /// true when we have come within RADIUS cm of the final destination
    bool reached_wp_destination() const override;

    /// run the wp controller
    bool update_wpnav() override;

protected:

    // oa path planning variables
    AG_OAGathPlanner::OA_RetState _oa_state;    // state of object avoidance, if OA_SUCCESS we use _oa_destination to avoid obstacles
    Vector3f    _origin_oabak;          // backup of _origin so it can be restored when oa completes
    Vector3f    _destination_oabak;     // backup of _destination so it can be restored when oa completes
    bool        _terrain_alt_oabak;     // true if backup origin and destination z-axis are terrain altitudes
    Location    _oa_destination;        // intermediate destination during avoidance
};
