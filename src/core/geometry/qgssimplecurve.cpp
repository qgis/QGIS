/***************************************************************************
                       qgssimplecurve.h
                         ------------
    begin                : May 2026
    copyright            : (C) 2026 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssimplecurve.h"

QgsPoint QgsSimpleCurve::pointN( int i ) const
{
  if ( i < 0 || i >= mX.size() )
  {
    return QgsPoint();
  }

  double x = mX.at( i );
  double y = mY.at( i );
  double z = std::numeric_limits<double>::quiet_NaN(); // TODO: QgsCircularString had 0 here!
  double m = std::numeric_limits<double>::quiet_NaN(); // TODO: QgsCircularString had 0 here!

  bool hasZ = is3D();
  if ( hasZ )
  {
    z = mZ.at( i );
  }
  bool hasM = isMeasure();
  if ( hasM )
  {
    m = mM.at( i );
  }

  Qgis::WkbType t = Qgis::WkbType::Point;
  if ( mWkbType == Qgis::WkbType::LineString25D )
  {
    t = Qgis::WkbType::Point25D;
  }
  else if ( hasZ && hasM )
  {
    t = Qgis::WkbType::PointZM;
  }
  else if ( hasZ )
  {
    t = Qgis::WkbType::PointZ;
  }
  else if ( hasM )
  {
    t = Qgis::WkbType::PointM;
  }
  return QgsPoint( t, x, y, z, m );
}

int QgsSimpleCurve::numPoints() const
{
  return mX.size();
}

int QgsSimpleCurve::nCoordinates() const
{
  return mX.size();
}
