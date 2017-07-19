#ifndef UTILS_H
#define UTILS_H

class QgsLineString;
class QgsPolygonV2;

#include "map3d.h"

class Utils
{
  public:

    static void clampAltitudes( QgsLineString *lineString, AltitudeClamping altClamp, AltitudeBinding altBind, const QgsPoint &centroid, float height, const Map3D &map );
    static bool clampAltitudes( QgsPolygonV2 *polygon, AltitudeClamping altClamp, AltitudeBinding altBind, float height, const Map3D &map );

};

#endif // UTILS_H
