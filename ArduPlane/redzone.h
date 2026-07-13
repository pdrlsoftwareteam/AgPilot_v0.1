/*
 * redzone.h
 *
 *  Ported from ArduCopter/redzone.h to ArduPlane.
 */
#include "Plane.h"
#include <GCS_MAVLink/GCS.h>
#include "mode.h"
#include "../libraries/AP_PDRL_Commander/AP_PDRL_Commander.h"

#ifndef ARDUPLANE_REDZONE_H_
#define ARDUPLANE_REDZONE_H_
#define EARTH_RADIUS_M 6371000.0  // in meters
const double EPSILON = 1e-6;
struct Point {
    double lat_test; // y
    double lon_test; // x
};

class Red_Zone
{
public:
  friend class Plane;
  friend class AP_Arming_Plane;
  enum class zone_breach {
    NO_BREACH = 0,
	ALT_BREACH = 1,
	CIRCULAR_BREACH_YELLOW = 2,
	CIRCULAR_BREACH_RED = 3,
	POLY_BREACH_YELLOW = 4,
	POLY_BREACH_RED = 5
  };
  struct location{
    double lat;
    double lng;
    uint16_t alt;
  };
  struct NFZ_EntryState {
    bool was_inside_radius;
    bool was_altitude_ok;
  };

  struct NFZ_EntryState_poly {
    bool was_inside_polygon;
    bool was_altitude_ok;
  };
  // Dynamically allocated state pointer (you can move this into a class later)
  static NFZ_EntryState* nfz_states;
  static NFZ_EntryState_poly* nfz_states_poly;
  static size_t nfz_states_size;
  static size_t nfz_states_size_poly;
  Red_Zone ();
  void update();
  bool isSamePoint(double lat1, double lon1, double lat2, double lon2);
  bool isSame(double a, double b);

  bool isPointOnSegment(double lat1, double lon1, double lat2, double lon2, double plat, double plon);
  bool isInsidePolygon(const std::vector<double>& lat,const std::vector<double>& lon,int n,const Point& p);


  zone_breach check_redzone();
  double deg2rad(double);
  double haversine_distance(double , double , double , double );
  virtual
  ~Red_Zone ();
};

#endif /* ARDUPLANE_REDZONE_H_ */
