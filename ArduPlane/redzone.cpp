/*
 * redzone.cpp
 *
 *  Ported from ArduCopter/redzone.cpp to ArduPlane.
 */

#include "redzone.h"
// Define static members outside the class
Red_Zone::NFZ_EntryState* Red_Zone::nfz_states = nullptr;
Red_Zone::NFZ_EntryState_poly* Red_Zone::nfz_states_poly = nullptr;

size_t Red_Zone::nfz_states_size = 0;
size_t Red_Zone::nfz_states_size_poly = 0;
Red_Zone::Red_Zone ()
{
  // TODO Auto-generated constructor stub
}

Red_Zone::~Red_Zone ()
{
  // TODO Auto-generated destructor stub
}

// Convert degrees to radians
double Red_Zone::deg2rad(double deg) {
  return deg * (M_PI / 180.0);
}


// Haversine distance calculation
double Red_Zone::haversine_distance(double lat1, double lon1, double lat2, double lon2) {
  double dlat = deg2rad(lat2 - lat1);
  double dlon = deg2rad(lon2 - lon1);

  lat1 = deg2rad(lat1);
  lat2 = deg2rad(lat2);

  double a = sin(dlat/2) * sin(dlat/2) +
      cos(lat1) * cos(lat2) *
      sin(dlon/2) * sin(dlon/2);

  double c = 2 * atan2(sqrt(a), sqrt(1 - a));

  return EARTH_RADIUS_M * c;
}
// Compare floats safely
bool Red_Zone::isSame(double a, double b) {
  return std::fabs(a - b) < EPSILON;
}

// Check if point is on a segment
bool Red_Zone::isPointOnSegment(double lat1, double lon1, double lat2, double lon2, double plat, double plon) {
  double cross = (plat - lat1) * (lon2 - lon1) - (plon - lon1) * (lat2 - lat1);
  if (std::fabs(cross) > 1e-10)
    return false;

  double dot = (plon - lon1) * (lon2 - lon1) + (plat - lat1) * (lat2 - lat1);
  double len_sq = (lon2 - lon1) * (lon2 - lon1) + (lat2 - lat1) * (lat2 - lat1);

  bool on_segment = dot >= 0 && dot <= len_sq;

  if (on_segment) {
      printf("\nPoint appears on segment from (%.6f, %.6f) to (%.6f, %.6f)\n", lat1, lon1, lat2, lon2);
  }

  return on_segment;
}


// Main inside-polygon check
bool Red_Zone::isInsidePolygon(const std::vector<double>& lat,const std::vector<double>& lon,int n,const Point& p)
 {
  // Check if point matches any vertex
  for (int i = 0; i < n; ++i) {
      if (isSame(p.lat_test, lat[i]) && isSame(p.lon_test, lon[i])) {
	  return true;
      }
  }

  // Check if point lies on any edge
  for (int i = 0; i < n; ++i) {
      int j = (i + 1) % n;
      if (isPointOnSegment(lat[i], lon[i], lat[j], lon[j], p.lat_test, p.lon_test)) {
	  return true;
      }
  }

  // Ray casting logic
  bool inside = false;
  for (int i = 0, j = n - 1; i < n; j = i++) {
      if ((lat[i] > p.lat_test) != (lat[j] > p.lat_test)) {
	  double intersect_lon = (lon[j] - lon[i]) * (p.lat_test - lat[i]) / (lat[j] - lat[i] + EPSILON) + lon[i];
	  if (p.lon_test < intersect_lon) {
	      inside = !inside;
	  }
      }
  }

  return inside;
}
Red_Zone::zone_breach Red_Zone::check_redzone()
{
    const auto& circles = AP_PDRL_COMMANDER::getInstance()->getAllCircles();
    const auto& polygons = AP_PDRL_COMMANDER::getInstance()->getAllPolygons();

    location curr_loc;
    curr_loc.lat = plane.current_loc.lat * 1.0e-7;
    curr_loc.lng = plane.current_loc.lng * 1.0e-7;
    curr_loc.alt = plane.current_loc.alt;

    printf("lat: %lf\tlng: %lf\talt: %d\n", curr_loc.lat, curr_loc.lng, curr_loc.alt);

    for (size_t i = 0; i < circles.size(); ++i) {
        const auto& nfz = circles[i];

        double distance = haversine_distance(curr_loc.lat, curr_loc.lng, nfz.lat, nfz.lng);
        bool inside_radius = distance <= nfz.radius_m;
        bool altitude_ok = static_cast<int>(curr_loc.alt) < static_cast<int>(nfz.alt_max);

        if (inside_radius &&
            nfz_states[i].was_inside_radius &&
            nfz_states[i].was_altitude_ok &&
            !altitude_ok) {

            printf("Zone %zu: Safe horizontal entry, now altitude breach!\n", i);
            return zone_breach::ALT_BREACH;
        }

        if (inside_radius &&
            !nfz_states[i].was_inside_radius &&
            !altitude_ok) {

            printf("Zone %zu: Entered NFZ with high altitude — critical!\n", i);
            return zone_breach::CIRCULAR_BREACH_RED;
        }

        // Save state
        nfz_states[i].was_inside_radius = inside_radius;
        nfz_states[i].was_altitude_ok = altitude_ok;
    }

    for (size_t j = 0; j < polygons.size(); ++j) {
        const auto& nfz = polygons[j];

        Point testPoint = {curr_loc.lat, curr_loc.lng};

        // Count valid points
        int num_points = 0;
        for (int i = 0; i < nfz.total_point; ++i) {
            if (std::fabs(nfz.lat_arr[i]) > EPSILON && std::fabs(nfz.lng_arr[i]) > EPSILON) {
                num_points = i + 1;
            }
        }

        bool is_inside_now = isInsidePolygon(nfz.lat_arr, nfz.lng_arr, num_points, testPoint);
        bool altitude_ok = static_cast<int>(curr_loc.alt) < static_cast<int>(nfz.alt_max);

        if (is_inside_now &&
            nfz_states_poly[j].was_inside_polygon &&
            nfz_states_poly[j].was_altitude_ok &&
            !altitude_ok) {

            printf("Zone %zu: Safe polygon entry, now altitude breach!\n", j);
            return zone_breach::ALT_BREACH;
        }

        if (is_inside_now &&
            !nfz_states_poly[j].was_inside_polygon &&
            !altitude_ok) {

            printf("Zone %zu: Entered polygon NFZ with high altitude — critical!\n", j);
            return zone_breach::POLY_BREACH_RED;
        }

        nfz_states_poly[j].was_inside_polygon = is_inside_now;
        nfz_states_poly[j].was_altitude_ok = altitude_ok;
    }

    return zone_breach::NO_BREACH;
}

void Red_Zone::update()
{

  if(!plane.arming.is_armed())
    {
      return;
    }
  size_t NFZ_size = AP_PDRL_COMMANDER::getInstance()->send_NFZ_count();
  if (nfz_states == nullptr || nfz_states_size != NFZ_size) {
      delete[] nfz_states; // free old if allocated
      nfz_states = new NFZ_EntryState[NFZ_size](); // zero-initialize
      nfz_states_size = NFZ_size;
  }

  size_t NFZ_size_poly = AP_PDRL_COMMANDER::getInstance()->send_NFZ_count_poly();
  if (nfz_states_poly == nullptr || nfz_states_size_poly != NFZ_size_poly) {
      delete[] nfz_states_poly; // free old if allocated
      nfz_states_poly = new NFZ_EntryState_poly[NFZ_size_poly](); // zero-initialize
      nfz_states_size_poly = NFZ_size_poly;
  }

  static uint32_t t = AP_HAL::millis();
  zone_breach ret = check_redzone();
  switch(ret)
  {
    case zone_breach::NO_BREACH:
      {
	if(AP_HAL::millis() - t > 1000)
	  {
	    plane.failsafe_NFZ_on_event(zone_breach::NO_BREACH);
	    t = AP_HAL::millis();
	  }
      }
      break;

    case zone_breach::ALT_BREACH:
      {
	if(AP_HAL::millis() - t > 1000)
	  {
	    plane.failsafe_NFZ_on_event(zone_breach::ALT_BREACH);
	    t = AP_HAL::millis();
	  }
      }
      break;

    case zone_breach::CIRCULAR_BREACH_YELLOW:
      {
	if(AP_HAL::millis() - t > 1000)
	  {
	    t = AP_HAL::millis();
	  }
      }
      break;

    case zone_breach::CIRCULAR_BREACH_RED:
      {
	if(AP_HAL::millis() - t > 1000)
	  {
	    plane.failsafe_NFZ_on_event(zone_breach::CIRCULAR_BREACH_RED);
	    t = AP_HAL::millis();
	  }
      }
      break;

    case zone_breach::POLY_BREACH_YELLOW:
      {
	if(AP_HAL::millis() - t > 1000)
	  {
	    plane.failsafe_NFZ_on_event(zone_breach::POLY_BREACH_YELLOW);
	    t = AP_HAL::millis();
	  }
      }
      break;

    case zone_breach::POLY_BREACH_RED:
      {
	if(AP_HAL::millis() - t > 1000)
	  {
	    plane.failsafe_NFZ_on_event(zone_breach::POLY_BREACH_RED);
	    t = AP_HAL::millis();
	  }
      }
      break;

    default:
      break;
  }

}
