/*
 * Location.cpp
 */

#include "Location.h"

#include <AP_AHRS/AP_AHRS.h>
#include <AP_Terrain/AP_Terrain.h>
#include <AP_Math/AP_Math.h>

/// constructors
Location::Location()
{
    zero();
}

const Location definitely_zero{};
bool Location::is_zero(void) const
{
    return !memcmp(this, &definitely_zero, sizeof(*this));
}

void Location::zero(void)
{
    memset(this, 0, sizeof(*this));
}

// Construct location using position (NEU) from ekf_origin for the given altitude frame
Location::Location(int32_t latitude, int32_t longitude, int32_t alt_in_cm, AltFrame frame)
{
    zero();
    lat = latitude;
    lng = longitude;
    set_alt_cm(alt_in_cm, frame);
}

Location::Location(const Vector3f &ekf_offset_neu, AltFrame frame)
{
    zero();

    // store alt and alt frame
    set_alt_cm(ekf_offset_neu.z, frame);

    // calculate lat, lon
    Location ekf_origin;
    if (AP::ahrs().get_origin(ekf_origin)) {
        lat = ekf_origin.lat;
        lng = ekf_origin.lng;
        offset(ekf_offset_neu.x * 0.01, ekf_offset_neu.y * 0.01);
    }
}

Location::Location(const Vector3d &ekf_offset_neu, AltFrame frame)
{
    zero();

    // store alt and alt frame
    set_alt_cm(ekf_offset_neu.z, frame);

    // calculate lat, lon
    Location ekf_origin;
    if (AP::ahrs().get_origin(ekf_origin)) {
        lat = ekf_origin.lat;
        lng = ekf_origin.lng;
        offset(ekf_offset_neu.x * 0.01, ekf_offset_neu.y * 0.01);
    }
}

void Location::set_alt_cm(int32_t alt_cm, AltFrame frame)
{
    alt = alt_cm;
    relative_alt = false;
    terrain_alt = false;
    origin_alt = false;
    switch (frame) {
        case AltFrame::ABSOLUTE:
            // do nothing
            break;
        case AltFrame::ABOVE_HOME:
            relative_alt = true;
            break;
        case AltFrame::ABOVE_ORIGIN:
            origin_alt = true;
            break;
        case AltFrame::ABOVE_TERRAIN:
            // we mark it as a relative altitude, as it doesn't have
            // home alt added
            relative_alt = true;
            terrain_alt = true;
            break;
    }
}

// converts altitude to new frame
bool Location::change_alt_frame(AltFrame desired_frame)
{
    int32_t new_alt_cm;
    if (!get_alt_cm(desired_frame, new_alt_cm)) {
        return false;
    }
    set_alt_cm(new_alt_cm, desired_frame);
    return true;
}

// get altitude frame
Location::AltFrame Location::get_alt_frame() const
{
    if (terrain_alt) {
        return AltFrame::ABOVE_TERRAIN;
    }
    if (origin_alt) {
        return AltFrame::ABOVE_ORIGIN;
    }
    if (relative_alt) {
        return AltFrame::ABOVE_HOME;
    }
    return AltFrame::ABSOLUTE;
}

/// get altitude in desired frame
bool Location::get_alt_cm(AltFrame desired_frame, int32_t &ret_alt_cm) const
{
#if CONFIG_HAL_BOARD == HAL_BOARD_SITL
    if (!initialised()) {
        AP_HAL::panic("Should not be called on invalid location: Location cannot be (0, 0, 0)");
    }
#endif
    Location::AltFrame frame = get_alt_frame();

    // shortcut if desired and underlying frame are the same
    if (desired_frame == frame) {
        ret_alt_cm = alt;
        return true;
    }

    // check for terrain altitude
    float alt_terr_cm = 0;
    if (frame == AltFrame::ABOVE_TERRAIN || desired_frame == AltFrame::ABOVE_TERRAIN) {
#if AP_TERRAIN_AVAILABLE
        AP_Terrain *terrain = AP::terrain();
        if (terrain == nullptr) {
            return false;
        }
        if (!terrain->height_amsl(*this, alt_terr_cm)) {
            return false;
        }
        // convert terrain alt to cm
        alt_terr_cm *= 100.0f;
#else
        return false;
#endif
    }

    // convert alt to absolute
    int32_t alt_abs = 0;
    switch (frame) {
        case AltFrame::ABSOLUTE:
            alt_abs = alt;
            break;
        case AltFrame::ABOVE_HOME:
            if (!AP::ahrs().home_is_set()) {
                return false;
            }
            alt_abs = alt + AP::ahrs().get_home().alt;
            break;
        case AltFrame::ABOVE_ORIGIN:
            {
                // fail if we cannot get ekf origin
                Location ekf_origin;
                if (!AP::ahrs().get_origin(ekf_origin)) {
                    return false;
                }
                alt_abs = alt + ekf_origin.alt;
            }
            break;
        case AltFrame::ABOVE_TERRAIN:
            alt_abs = alt + alt_terr_cm;
            break;
    }

    // convert absolute to desired frame
    switch (desired_frame) {
        case AltFrame::ABSOLUTE:
            ret_alt_cm = alt_abs;
            return true;
        case AltFrame::ABOVE_HOME:
            if (!AP::ahrs().home_is_set()) {
                return false;
            }
            ret_alt_cm = alt_abs - AP::ahrs().get_home().alt;
            return true;
        case AltFrame::ABOVE_ORIGIN:
            {
                // fail if we cannot get ekf origin
                Location ekf_origin;
                if (!AP::ahrs().get_origin(ekf_origin)) {
                    return false;
                }
                ret_alt_cm = alt_abs - ekf_origin.alt;
                return true;
            }
        case AltFrame::ABOVE_TERRAIN:
            ret_alt_cm = alt_abs - alt_terr_cm;
            return true;
    }
    return false;  // LCOV_EXCL_LINE  - not reachable
}

bool Location::get_vector_xy_from_origin_NE(Vector2f &vec_ne) const
{
    Location ekf_origin;
    if (!AP::ahrs().get_origin(ekf_origin)) {
        return false;
    }
    vec_ne.x = (lat-ekf_origin.lat) * LATLON_TO_CM;
    vec_ne.y = diff_longitude(lng,ekf_origin.lng) * LATLON_TO_CM * longitude_scale((lat+ekf_origin.lat)/2);
    return true;
}

bool Location::get_vector_from_origin_NEU(Vector3f &vec_neu) const
{
    // convert lat, lon
    if (!get_vector_xy_from_origin_NE(vec_neu.xy())) {
        return false;
    }

    // convert altitude
    int32_t alt_above_origin_cm = 0;
    if (!get_alt_cm(AltFrame::ABOVE_ORIGIN, alt_above_origin_cm)) {
        return false;
    }
    vec_neu.z = alt_above_origin_cm;

    return true;
}

// return horizontal distance in meters between two locations
ftype Location::get_distance(const Location &loc2) const
{
    ftype dlat = (ftype)(loc2.lat - lat);
    ftype dlng = ((ftype)diff_longitude(loc2.lng,lng)) * longitude_scale((lat+loc2.lat)/2);
    return norm(dlat, dlng) * LOCATION_SCALING_FACTOR;
}

// return the altitude difference in meters taking into account alt frame.
bool Location::get_alt_distance(const Location &loc2, ftype &distance) const
{
    int32_t alt1, alt2;
    if (!get_alt_cm(AltFrame::ABSOLUTE, alt1) || !loc2.get_alt_cm(AltFrame::ABSOLUTE, alt2)) {
        return false;
    }
    distance = (alt1 - alt2) * 0.01;
    return true;
}

/*
  return the distance in meters in North/East plane as a N/E vector
  from loc1 to loc2
 */
Vector2f Location::get_distance_NE(const Location &loc2) const
{
    return Vector2f((loc2.lat - lat) * LOCATION_SCALING_FACTOR,
                    diff_longitude(loc2.lng,lng) * LOCATION_SCALING_FACTOR * longitude_scale((loc2.lat+lat)/2));
}

// return the distance in meters in North/East/Down plane as a N/E/D vector to loc2, NOT CONSIDERING ALT FRAME!
Vector3f Location::get_distance_NED(const Location &loc2) const
{
    return Vector3f((loc2.lat - lat) * LOCATION_SCALING_FACTOR,
                    diff_longitude(loc2.lng,lng) * LOCATION_SCALING_FACTOR * longitude_scale((lat+loc2.lat)/2),
                    (alt - loc2.alt) * 0.01);
}

// return the distance in meters in North/East/Down plane as a N/E/D vector to loc2
Vector3d Location::get_distance_NED_double(const Location &loc2) const
{
    return Vector3d((loc2.lat - lat) * double(LOCATION_SCALING_FACTOR),
                    diff_longitude(loc2.lng,lng) * LOCATION_SCALING_FACTOR * longitude_scale((lat+loc2.lat)/2),
                    (alt - loc2.alt) * 0.01);
}

Vector2d Location::get_distance_NE_double(const Location &loc2) const
{
    return Vector2d((loc2.lat - lat) * double(LOCATION_SCALING_FACTOR),
                    diff_longitude(loc2.lng,lng) * double(LOCATION_SCALING_FACTOR) * longitude_scale((lat+loc2.lat)/2));
}

Vector2F Location::get_distance_NE_ftype(const Location &loc2) const
{
    return Vector2F((loc2.lat - lat) * ftype(LOCATION_SCALING_FACTOR),
                    diff_longitude(loc2.lng,lng) * ftype(LOCATION_SCALING_FACTOR) * longitude_scale((lat+loc2.lat)/2));
}

// extrapolate latitude/longitude given distances (in meters) north and east
void Location::offset_latlng(int32_t &lat, int32_t &lng, ftype ofs_north, ftype ofs_east)
{
    const int32_t dlat = ofs_north * LOCATION_SCALING_FACTOR_INV;
    const int64_t dlng = (ofs_east * LOCATION_SCALING_FACTOR_INV) / longitude_scale(lat+dlat/2);
    lat += dlat;
    lat = limit_lattitude(lat);
    lng = wrap_longitude(dlng+lng);
}

// extrapolate latitude/longitude given distances (in meters) north and east
void Location::offset(ftype ofs_north, ftype ofs_east)
{
    offset_latlng(lat, lng, ofs_north, ofs_east);
}

/*
 *  extrapolate latitude/longitude given bearing and distance
 * Note that this function is accurate to about 1mm at a distance of
 * 100m. This function has the advantage that it works in relative
 * positions, so it keeps the accuracy even when dealing with small
 * distances and floating point numbers
 */
void Location::offset_bearing(ftype bearing_deg, ftype distance)
{
    const ftype ofs_north = cosF(radians(bearing_deg)) * distance;
    const ftype ofs_east  = sinF(radians(bearing_deg)) * distance;
    offset(ofs_north, ofs_east);
}

// extrapolate latitude/longitude given bearing, pitch and distance
void Location::offset_bearing_and_pitch(ftype bearing_deg, ftype pitch_deg, ftype distance)
{
    const ftype ofs_north =  cosF(radians(pitch_deg)) * cosF(radians(bearing_deg)) * distance;
    const ftype ofs_east  =  cosF(radians(pitch_deg)) * sinF(radians(bearing_deg)) * distance;
    offset(ofs_north, ofs_east);
    const int32_t dalt =  sinF(radians(pitch_deg)) * distance *100.0f;
    alt += dalt; 
}


ftype Location::longitude_scale(int32_t lat)
{
    ftype scale = cosF(lat * (1.0e-7 * DEG_TO_RAD));
    return MAX(scale, 0.01);
}

/*
 * convert invalid waypoint with useful data. return true if location changed
 */
bool Location::sanitize(const Location &defaultLoc)
{
    bool has_changed = false;
    // convert lat/lng=0 to mean current point
    if (lat == 0 && lng == 0) {
        lat = defaultLoc.lat;
        lng = defaultLoc.lng;
        has_changed = true;
    }

    // convert relative alt=0 to mean current alt
    if (alt == 0 && relative_alt) {
        int32_t defaultLoc_alt;
        if (defaultLoc.get_alt_cm(get_alt_frame(), defaultLoc_alt)) {
            alt = defaultLoc_alt;
            has_changed = true;
        }
    }

    // limit lat/lng to appropriate ranges
    if (!check_latlng()) {
        lat = defaultLoc.lat;
        lng = defaultLoc.lng;
        has_changed = true;
    }

    return has_changed;
}

// make sure we know what size the Location object is:
assert_storage_size<Location, 16> _assert_storage_size_Location;


// return bearing in radians from location to loc2, return is 0 to 2*Pi
ftype Location::get_bearing(const Location &loc2) const
{
    const int32_t off_x = diff_longitude(loc2.lng,lng);
    const int32_t off_y = (loc2.lat - lat) / loc2.longitude_scale((lat+loc2.lat)/2);
    ftype bearing = (M_PI*0.5) + atan2F(-off_y, off_x);
    if (bearing < 0) {
        bearing += 2*M_PI;
    }
    return bearing;
}

/*
  return true if lat and lng match. Ignores altitude and options
 */
bool Location::same_latlon_as(const Location &loc2) const
{
    return (lat == loc2.lat) && (lng == loc2.lng);
}

// return true when lat and lng are within range
bool Location::check_latlng() const
{
    return check_lat(lat) && check_lng(lng);
}

// see if location is past a line perpendicular to
// the line between point1 and point2 and passing through point2.
// If point1 is our previous waypoint and point2 is our target waypoint
// then this function returns true if we have flown past
// the target waypoint
bool Location::past_interval_finish_line(const Location &point1, const Location &point2) const
{
    return this->line_path_proportion(point1, point2) >= 1.0f;
}

/*
  return the proportion we are along the path from point1 to
  point2, along a line parallel to point1<->point2.

  This will be more than 1 if we have passed point2
 */
float Location::line_path_proportion(const Location &point1, const Location &point2) const
{
    const Vector2f vec1 = point1.get_distance_NE(point2);
    const Vector2f vec2 = point1.get_distance_NE(*this);
    const ftype dsquared = sq(vec1.x) + sq(vec1.y);
    if (dsquared < 0.001f) {
        // the two points are very close together
        return 1.0f;
    }
    return (vec1 * vec2) / dsquared;
}

/*
  wrap longitude for -180e7 to 180e7
 */
int32_t Location::wrap_longitude(int64_t lon)
{
    if (lon > 1800000000L) {
        lon = int32_t(lon-3600000000LL);
    } else if (lon < -1800000000L) {
        lon = int32_t(lon+3600000000LL);
    }
    return int32_t(lon);
}

/*
  get lon1-lon2, wrapping at -180e7 to 180e7
 */
int32_t Location::diff_longitude(int32_t lon1, int32_t lon2)
{
    if ((lon1 & 0x80000000) == (lon2 & 0x80000000)) {
        // common case of same sign
        return lon1 - lon2;
    }
    int64_t dlon = int64_t(lon1)-int64_t(lon2);
    if (dlon > 1800000000LL) {
        dlon -= 3600000000LL;
    } else if (dlon < -1800000000LL) {
        dlon += 3600000000LL;
    }
    return int32_t(dlon);
}

/*
  limit lattitude to -90e7 to 90e7
 */
int32_t Location::limit_lattitude(int32_t lat)
{
    if (lat > 900000000L) {
        lat = 1800000000LL - lat;
    } else if (lat < -900000000L) {
        lat = -(1800000000LL + lat);
    }
    return lat;
}

// update altitude and alt-frame base on this location's horizontal position between point1 and point2
// this location's lat,lon is used to calculate the alt of the closest point on the line between point1 and point2
// origin and destination's altitude frames must be the same
// this alt-frame will be updated to match the destination alt frame
void Location::linearly_interpolate_alt(const Location &point1, const Location &point2)
{
    // new target's distance along the original track and then linear interpolate between the original origin and destination altitudes
    set_alt_cm(point1.alt + (point2.alt - point1.alt) * constrain_float(line_path_proportion(point1, point2), 0.0f, 1.0f), point2.get_alt_frame());
}

void Location::calculate_next_waypoint(double current_lat_deg,
                                       double current_lon_deg,
                                       float heading_deg,
                                       float distance_m,
                                       double &next_lat_deg,
                                       double &next_lon_deg)
{
    // Create origin Location
    Location result;
    result.lat = current_lat_deg * 1e7;
    result.lng = current_lon_deg * 1e7;
    result.alt = 0;

    // Convert heading to radians
    float heading_rad = radians(heading_deg);

    // Convert heading+distance to north/east offsets
    float offset_north = distance_m * cosf(heading_rad);
    float offset_east  = distance_m * sinf(heading_rad);

    // Apply offset (in meters)
    result.offset(offset_north, offset_east);

    // Convert back to degrees
    next_lat_deg = result.lat / 1.0e7;
    next_lon_deg = result.lng / 1.0e7;
}


// heading_deg: direction of survey lines (e.g., 0° = north-south, 90° = east-west)
// side_m: width of the square (length of one side)
// spacing_m: distance between each pass (line separation)
//void Location::generate_square_survey(double center_lat_deg,
//                            double center_lon_deg,
//                            float heading_deg,
//                            float side_m,
//                            float spacing_m,
//                            Location waypoints[6])
//{
//    Location center;
//    center.lat = center_lat_deg * 1e7;
//    center.lng = center_lon_deg * 1e7;
//    center.alt = 0;
//    center.options = 0;
//
//    // Calculate heading vector (direction of flight lines)
//    float hdg_rad = radians(heading_deg);
//    float dx = cosf(hdg_rad);
//    float dy = sinf(hdg_rad);
//
//    // Perpendicular vector (to space lines)
//    float px = -dy;
//    float py = dx;
//
//    // Start from bottom-left corner of square
//    float half_side = side_m / 2.0f;
//
//    for (int i = 0; i < 3; ++i) {
//        float line_offset = (i - 1) * spacing_m;
//
//        // Start point of this line
//        float start_n = -half_side * dx + line_offset * px;
//        float start_e = -half_side * dy + line_offset * py;
//
//        // End point of this line
//        float end_n = half_side * dx + line_offset * px;
//        float end_e = half_side * dy + line_offset * py;
//
//        // Even lines go forward, odd go backward (zig-zag)
//        if (i % 2 == 0) {
//            waypoints[i * 2 + 0].offset(center, start_n, start_e);
//            waypoints[i * 2 + 1].offset(center, end_n, end_e);
//        } else {
//            waypoints[i * 2 + 0].offset(center, end_n, end_e);
//            waypoints[i * 2 + 1].offset(center, start_n, start_e);
//        }
//    }
//}
#include "GCS_MAVLink/GCS.h"
#include "AP_Math/vector2.h" // for Vector2f
#include <cmath>

void Location::plan_ab_survey(Location &loc,
                    double start_lat_deg,
                    double start_lon_deg,
                    float heading_deg,
                    float line_distance_m,
                    float spacing_m,
                    uint8_t num_lines,
					std::vector<Location> &waypoints)
{
    double current_lat = start_lat_deg;
    double current_lon = start_lon_deg;

    // Calculate unit heading vector
    Vector2f heading_vector(cosf(radians(heading_deg)), sinf(radians(heading_deg)));
    Vector2f perpendicular_vector(-heading_vector.y, heading_vector.x); // 90 deg rotation

    for (uint8_t i = 0; i < num_lines; ++i) {
        double next_lat, next_lon;

        // Use forward or reverse direction
        float leg_heading = (i % 2 == 0) ? heading_deg : fmodf(heading_deg + 180.0f, 360.0f);

        loc.calculate_next_waypoint(current_lat, current_lon, leg_heading, line_distance_m, next_lat, next_lon);

        Location wp;
        wp.lat = next_lat * 1e7;
        wp.lng = next_lon * 1e7;
        wp.alt = 0;
        wp.options = 0;
        waypoints.push_back(wp);

        // Move perpendicular (to start next leg)
        if (i < num_lines - 1) {
            Vector2f offset_vec = perpendicular_vector * spacing_m;

            // Convert N/E offset to lat/lon
            double new_lat, new_lon;
            float offset_heading = atan2f(offset_vec.y, offset_vec.x) * (180.0f / M_PI);
            float offset_dist = offset_vec.length();

            loc.calculate_next_waypoint(current_lat, current_lon, offset_heading, offset_dist, new_lat, new_lon);

            current_lat = new_lat;
            current_lon = new_lon;
        }
    }
}

//
//#include "AP_Common/Location.h"
//#include "AP_HAL/AP_HAL.h"
//
//extern const AP_HAL::HAL& hal;
//
//void Location::test_calculate_next_waypoint()
//{
//    Location loc; // just to call the member function
//
//    double current_lat_deg = 19.000000;
//    double current_lon_deg = 72.000000;
//    float heading_deg = 100.0;       // East
//    float distance_m = 100.0;       // 100 meters
//
//    double next_lat_deg = 0;
//    double next_lon_deg = 0;
//
//    loc.calculate_next_waypoint(current_lat_deg,
//                                current_lon_deg,
//                                heading_deg,
//                                distance_m,
//                                next_lat_deg,
//                                next_lon_deg);
//
//    GCS_SEND_TEXT(MAV_SEVERITY_INFO,
//                  "calculated WP: lat=%.7f lon=%.7f",
//				  next_lat_deg, next_lon_deg);
//
////    hal.console->printf("Next waypoint:\n");
////    hal.console->printf("Latitude:  %.7f\n", next_lat_deg);
////    hal.console->printf("Longitude: %.7f\n", next_lon_deg);
//}


