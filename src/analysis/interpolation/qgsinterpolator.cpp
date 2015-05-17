/***************************************************************************
                              qgsinterpolator.cpp
                              -------------------
  begin                : Marco 10, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsinterpolator.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"

QgsInterpolator::QgsInterpolator( const QList<LayerData>& layerData )
    : mDataIsCached( false )
    , mLayerData( layerData )
{

}

QgsInterpolator::QgsInterpolator()
    : mDataIsCached( false )
{

}

QgsInterpolator::~QgsInterpolator()
{

}

int QgsInterpolator::cacheBaseData()
{
  if ( mLayerData.size() < 1 )
  {
    return 0;
  }

  //reserve initial memory for 100000 vertices
  mCachedBaseData.clear();
  mCachedBaseData.reserve( 100000 );

  QList<LayerData>::iterator v_it = mLayerData.begin();

  for ( ; v_it != mLayerData.end(); ++v_it )
  {
    if ( v_it->vectorLayer == 0 )
    {
      continue;
    }

    QgsVectorLayer* vlayer = v_it->vectorLayer;
    if ( !vlayer )
    {
      return 2;
    }

    QgsAttributeList attList;
    if ( !v_it->zCoordInterpolation )
    {
      attList.push_back( v_it->interpolationAttribute );
    }


    double attributeValue = 0.0;
    bool attributeConversionOk = false;

    QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( attList ) );

    QgsFeature theFeature;
    while ( fit.nextFeature( theFeature ) )
    {
      if ( !v_it->zCoordInterpolation )
      {
        QVariant attributeVariant = theFeature.attribute( v_it->interpolationAttribute );
        if ( !attributeVariant.isValid() ) //attribute not found, something must be wrong (e.g. NULL value)
        {
          continue;
        }
        attributeValue = attributeVariant.toDouble( &attributeConversionOk );
        if ( !attributeConversionOk || qIsNaN( attributeValue ) ) //don't consider vertices with attributes like 'nan' for the interpolation
        {
          continue;
        }
      }

      if ( addVerticesToCache( theFeature.constGeometry(), v_it->zCoordInterpolation, attributeValue ) != 0 )
      {
        return 3;
      }
    }
  }

  return 0;
}

int QgsInterpolator::addVerticesToCache( const QgsGeometry *geom, bool zCoord, double attributeValue )
{
  if ( !geom )
    return 1;

  bool hasZValue = false;
  QgsConstWkbPtr currentWkbPtr( geom->asWkb() + 1 + sizeof( int ) );
  vertexData theVertex; //the current vertex

  QGis::WkbType wkbType = geom->wkbType();
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
      hasZValue = true;
      //intentional fall-through
    case QGis::WKBPoint:
    {
      currentWkbPtr >> theVertex.x >> theVertex.y;
      if ( zCoord && hasZValue )
      {
        currentWkbPtr >> theVertex.z;
      }
      else
      {
        theVertex.z = attributeValue;
      }
      mCachedBaseData.push_back( theVertex );
      break;
    }
    case QGis::WKBLineString25D:
      hasZValue = true;
      //intentional fall-through
    case QGis::WKBLineString:
    {
      int nPoints;
      currentWkbPtr >> nPoints;
      for ( int index = 0; index < nPoints; ++index )
      {
        currentWkbPtr >> theVertex.x >> theVertex.y;
        if ( zCoord && hasZValue ) //skip z-coordinate for 25D geometries
        {
          currentWkbPtr >> theVertex.z;
        }
        else
        {
          theVertex.z = attributeValue;
        }
        mCachedBaseData.push_back( theVertex );
      }
      break;
    }
#if 0
    case QGis::WKBPolygon25D:
      hasZValue = true;
      //intentional fall-through
    case QGis::WKBPolygon:
    {
      int nRings;
      wkbPtr >> nRings;
      for ( int index = 0; index < nRings; ++index )
      {
        int nPoints;
        wkbPtr >> nPoints;
        for ( int index2 = 0; index2 < *npoints; ++index2 )
        {
          double x, y;
          wkbPtr >> x >> y;
          if ( point.sqrDist( x, y ) < actdist )
          {
            actdist = point.sqrDist( x, y );
            vertexnr = vertexcounter;
            //assign the rubber band indices
            if ( index2 == 0 )
            {
              beforeVertex = vertexcounter + ( *npoints - 2 );
              afterVertex = vertexcounter + 1;
            }
            else if ( index2 == ( *npoints - 1 ) )
            {
              beforeVertex = vertexcounter - 1;
              afterVertex = vertexcounter - ( *npoints - 2 );
            }
            else
            {
              beforeVertex = vertexcounter - 1;
              afterVertex = vertexcounter + 1;
            }
          }
          if ( hasZValue ) //skip z-coordinate for 25D geometries
          {
            wkbPtr += sizeof( double );
          }
          ++vertexcounter;
        }
      }
      break;
    }
    case QGis::WKBMultiPoint25D:
      hasZValue = true;
      //intentional fall-through
    case QGis::WKBMultiPoint:
    {
      int nPoints;
      wkbPtr >> nPoints;
      for ( int index = 0; index < nPoints; ++index )
      {
        wkbPtr +=  1 + sizeof( int ); //skip endian and point type

        double x, y;
        wkbPtr >> x >> y;
        if ( point.sqrDist( x, y ) < actdist )
        {
          actdist = point.sqrDist( x, y );
          vertexnr = index;
        }
        if ( hasZValue ) //skip z-coordinate for 25D geometries
        {
          wkbPtr += sizeof( double );
        }
      }
      break;
    }
    case QGis::WKBMultiLineString25D:
      hasZValue = true;
      //intentional fall-through
    case QGis::WKBMultiLineString:
    {
      int nLines;
      wkbPtr >> nLines;
      for ( int index = 0; index < nLines; ++index )
      {
        int nPoints;
        wkbPtr >> nPoints;
        for ( int index2 = 0; index2 < nPoints; ++index2 )
        {
          double x, y;
          wkbPtr >> x >> y;
          if ( point.sqrDist( x, y ) < actdist )
          {
            actdist = point.sqrDist( x, y );
            vertexnr = vertexcounter;

            if ( index2 == 0 )//assign the rubber band indices
            {
              beforeVertex = -1;
            }
            else
            {
              beforeVertex = vertexnr - 1;
            }
            if ( index2 == nPoints - 1 )
            {
              afterVertex = -1;
            }
            else
            {
              afterVertex = vertexnr + 1;
            }
          }
          if ( hasZValue ) //skip z-coordinate for 25D geometries
          {
            wkbPtr += sizeof( double );
          }
          ++vertexcounter;
        }
      }
      break;
    }
    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
      //intentional fall-through
    case QGis::WKBMultiPolygon:
    {
      int nPolys;
      wkbPtr >> nPolys;
      for ( int index = 0; index < nPolys; ++index )
      {
        wkbPtr += 1 + sizeof( int ); //skip endian and polygon type
        int nRings;
        wkbPtr >> nRings;
        for ( int index2 = 0; index2 < nRings; ++index2 )
        {
          int nPoints;
          wkbPtr >> nPoints;
          for ( int index3 = 0; index3 < nPoints; ++index3 )
          {
            double x, y;
            wkbPtr >> x >> y;
            if ( point.sqrDist( x, y ) < actdist )
            {
              actdist = point.sqrDist( x, y );
              vertexnr = vertexcounter;

              //assign the rubber band indices
              if ( index3 == 0 )
              {
                beforeVertex = vertexcounter + ( nPoints - 2 );
                afterVertex = vertexcounter + 1;
              }
              else if ( index3 == ( *npoints - 1 ) )
              {
                beforeVertex = vertexcounter - 1;
                afterVertex = vertexcounter - ( nPoints - 2 );
              }
              else
              {
                beforeVertex = vertexcounter - 1;
                afterVertex = vertexcounter + 1;
              }
            }
            if ( hasZValue ) //skip z-coordinate for 25D geometries
            {
              wkbPtr += sizeof( double );
            }
            ++vertexcounter;
          }
        }
      }
      break;
    }
#endif //0
    default:
      break;
  }
  mDataIsCached = true;
  return 0;
}
