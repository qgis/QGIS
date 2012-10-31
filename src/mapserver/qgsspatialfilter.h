/***************************************************************************
                              qgsspatialfilter.h
                              ---------------------
  begin                : Oct 19, 2012
  copyright            : (C) 2012 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSPATIALFILTER_H
#define QGSSPATIALFILTER_H

#include <qgsfilter.h>
#include <qgsgeometry.h>
#include <QDomElement>

/**A filter for spatial filter (bbox, intersects, within, disjoint)
Sample xml fragment:
<Filter xmlns="http://www.opengis.net/ogc">
<BBOX>
<gml:Box xmlns:gml="http://www.opengis.net/gml" srsName="EPSG:4326">
<gml:coordinates decimal="." cs="," ts=" ">135.45,-47.425 157.95,-36.175</gml:coordinates>
</gml:Box>
</BBOX>
</Filter>
*/
class QgsSpatialFilter: public QgsFilter
{
  public:
    enum SPATIAL_TYPE
    {
      BBOX,
      CONTAINS,
      CROSSES,
      EQUALS,
      DISJOINT,
      INTERSECTS,
      OVERLAPS,
      TOUCHES,
      WITHIN,
      UNKNOWN
    };

    QgsSpatialFilter();
    QgsSpatialFilter( SPATIAL_TYPE st, QgsGeometry* geom );
    ~QgsSpatialFilter();

    /**Evaluates a feature against the filter.
     @return true if the filter applies for the feature*/
    bool evaluate( const QgsFeature& f ) const;

    //setters and getters
    SPATIAL_TYPE spatialType() const {return mSpatialType;}
    void setSpatialType( SPATIAL_TYPE t ) {mSpatialType = t;}

    //setters and getters
    QgsGeometry* geometry() const {return mGeom;}
    void setGeometry( QgsGeometry* g ) {mGeom = g;}

  private:
    SPATIAL_TYPE mSpatialType;
    QgsGeometry* mGeom;
};

#endif //QGSSPATIALFILTER_H
