/**
  * 半正矢公式
  * 参考资料：http://www.movable-type.co.uk/scripts/latlong.html
  */

#ifndef HAVERSINE_HPP
#define HAVERSINE_HPP

#include <math.h>


const double EARTH_RADIUS = 6372797.560856;
const double DEGREES_TO_RADIANS = M_PI / 180;

double haversine(
    double first_latitude,
    double first_longitude,
    double second_latitude,
    double second_longitude
) {
    double first_latitude_in_radians = first_latitude * DEGREES_TO_RADIANS;
    double second_latitude_in_radians = second_latitude * DEGREES_TO_RADIANS;
    double latitude_delta_in_radians = (second_latitude - first_latitude) * DEGREES_TO_RADIANS;
    double longitude_delta_in_radians = (second_longitude - first_longitude) * DEGREES_TO_RADIANS;
    
    // the square of half the chord length between the points
    double a = sin(latitude_delta_in_radians / 2) * sin(latitude_delta_in_radians / 2) + cos(first_latitude_in_radians) * cos(second_latitude_in_radians) * sin(longitude_delta_in_radians / 2) * sin(longitude_delta_in_radians / 2);
    
    // angular distance in radians
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    
    return EARTH_RADIUS * c;
}

#endif
