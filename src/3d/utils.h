#ifndef UTILS_H
#define UTILS_H

class QgsLineString;
class QgsPolygonV2;

#include "map3d.h"


//! how to handle altitude of vector features
enum AltitudeClamping
{
  AltClampAbsolute,   //!< Z_final = z_geometry
  AltClampRelative,   //!< Z_final = z_terrain + z_geometry
  AltClampTerrain,    //!< Z_final = z_terrain
};


//! how to handle clamping of vertices of individual features
enum AltitudeBinding
{
  AltBindVertex,      //!< Clamp every vertex of feature
  AltBindCentroid,    //!< Clamp just centroid of feature
};


class Utils
{
  public:

    static QString altClampingToString( AltitudeClamping altClamp );
    static AltitudeClamping altClampingFromString( const QString &str );

    static QString altBindingToString( AltitudeBinding altBind );
    static AltitudeBinding altBindingFromString( const QString &str );

    static void clampAltitudes( QgsLineString *lineString, AltitudeClamping altClamp, AltitudeBinding altBind, const QgsPoint &centroid, float height, const Map3D &map );
    static bool clampAltitudes( QgsPolygonV2 *polygon, AltitudeClamping altClamp, AltitudeBinding altBind, float height, const Map3D &map );

    static QString matrix4x4toString( const QMatrix4x4 &m );
    static QMatrix4x4 stringToMatrix4x4( const QString &str );
};

#endif // UTILS_H
