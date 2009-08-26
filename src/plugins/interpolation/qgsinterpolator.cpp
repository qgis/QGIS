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
#include "qgsgeometry.h"
#ifndef Q_OS_MACX
#include <cmath>
#else
#include <math.h>
#endif
#ifdef WIN32
#include <float.h>
#define isnan(f) _isnan(f)
#endif

QgsInterpolator::QgsInterpolator( const QList<LayerData>& layerData ): mDataIsCached( false ), mLayerData( layerData )
{

}

QgsInterpolator::QgsInterpolator()
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

    QgsVectorDataProvider* provider = v_it->vectorLayer->dataProvider();
    if ( !provider )
    {
      return 2;
    }

    QgsAttributeList attList;
    if ( !v_it->zCoordInterpolation )
    {
      attList.push_back( v_it->interpolationAttribute );
    }

    provider->select( attList );

    QgsFeature theFeature;
    double attributeValue = 0.0;
    bool attributeConversionOk = false;

    while ( provider->nextFeature( theFeature ) )
    {
      if ( !v_it->zCoordInterpolation )
      {
        QgsAttributeMap attMap = theFeature.attributeMap();
        QgsAttributeMap::const_iterator att_it = attMap.find( v_it->interpolationAttribute );
        if ( att_it == attMap.end() ) //attribute not found, something must be wrong (e.g. NULL value)
        {
          continue;
        }
        attributeValue = att_it.value().toDouble( &attributeConversionOk );
        if ( !attributeConversionOk || isnan( attributeValue ) ) //don't consider vertices with attributes like 'nan' for the interpolation
        {
          continue;
        }
      }

      if ( addVerticesToCache( theFeature.geometry(), v_it->zCoordInterpolation, attributeValue ) != 0 )
      {
        return 3;
      }
    }
  }

  return 0;
}

int QgsInterpolator::addVerticesToCache( QgsGeometry* geom, bool zCoord, double attributeValue )
{
  if ( !geom )
  {
    return 1;
  }

  bool hasZValue = false;
  unsigned char* currentWkbPtr = geom->asWkb();
  vertexData theVertex; //the current vertex

  QGis::WkbType wkbType = geom->wkbType();
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
      hasZValue = true;
    case QGis::WKBPoint:
    {
      currentWkbPtr += ( 1 + sizeof( int ) );
      theVertex.x = *(( double * )( currentWkbPtr ) );
      currentWkbPtr += sizeof( double );
      theVertex.y = *(( double * )( currentWkbPtr ) );
      if ( zCoord && hasZValue )
      {
        currentWkbPtr += sizeof( double );
        theVertex.z = *(( double * )( currentWkbPtr ) );
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
    case QGis::WKBLineString:
    {
      currentWkbPtr += ( 1 + sizeof( int ) );
      int* npoints = ( int* )currentWkbPtr;
      currentWkbPtr += sizeof( int );
      for ( int index = 0;index < *npoints;++index )
      {
        theVertex.x = *(( double * )( currentWkbPtr ) );
        currentWkbPtr += sizeof( double );
        theVertex.y = *(( double * )( currentWkbPtr ) );
        currentWkbPtr += sizeof( double );
        if ( zCoord && hasZValue ) //skip z-coordinate for 25D geometries
        {
          theVertex.z = *(( double * )( currentWkbPtr ) );
          currentWkbPtr += sizeof( double );
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
    case QGis::WKBPolygon:
    {
      int* nrings = ( int* )( mGeometry + 5 );
      int* npoints;
      unsigned char* ptr = mGeometry + 9;
      for ( int index = 0;index < *nrings;++index )
      {
        npoints = ( int* )ptr;
        ptr += sizeof( int );
        for ( int index2 = 0;index2 < *npoints;++index2 )
        {
          tempx = ( double* )ptr;
          ptr += sizeof( double );
          tempy = ( double* )ptr;
          if ( point.sqrDist( *tempx, *tempy ) < actdist )
          {
            x = *tempx;
            y = *tempy;
            actdist = point.sqrDist( *tempx, *tempy );
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
          ptr += sizeof( double );
          if ( hasZValue ) //skip z-coordinate for 25D geometries
          {
            ptr += sizeof( double );
          }
          ++vertexcounter;
        }
      }
      break;
    }
    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
    {
      unsigned char* ptr = mGeometry + 5;
      int* npoints = ( int* )ptr;
      ptr += sizeof( int );
      for ( int index = 0;index < *npoints;++index )
      {
        ptr += ( 1 + sizeof( int ) ); //skip endian and point type
        tempx = ( double* )ptr;
        tempy = ( double* )( ptr + sizeof( double ) );
        if ( point.sqrDist( *tempx, *tempy ) < actdist )
        {
          x = *tempx;
          y = *tempy;
          actdist = point.sqrDist( *tempx, *tempy );
          vertexnr = index;
        }
        ptr += ( 2 * sizeof( double ) );
        if ( hasZValue ) //skip z-coordinate for 25D geometries
        {
          ptr += sizeof( double );
        }
      }
      break;
    }
    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      unsigned char* ptr = mGeometry + 5;
      int* nlines = ( int* )ptr;
      int* npoints = 0;
      ptr += sizeof( int );
      for ( int index = 0;index < *nlines;++index )
      {
        ptr += ( sizeof( int ) + 1 );
        npoints = ( int* )ptr;
        ptr += sizeof( int );
        for ( int index2 = 0;index2 < *npoints;++index2 )
        {
          tempx = ( double* )ptr;
          ptr += sizeof( double );
          tempy = ( double* )ptr;
          ptr += sizeof( double );
          if ( point.sqrDist( *tempx, *tempy ) < actdist )
          {
            x = *tempx;
            y = *tempy;
            actdist = point.sqrDist( *tempx, *tempy );
            vertexnr = vertexcounter;

            if ( index2 == 0 )//assign the rubber band indices
            {
              beforeVertex = -1;
            }
            else
            {
              beforeVertex = vertexnr - 1;
            }
            if ( index2 == ( *npoints ) - 1 )
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
            ptr += sizeof( double );
          }
          ++vertexcounter;
        }
      }
      break;
    }
    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      unsigned char* ptr = mGeometry + 5;
      int* npolys = ( int* )ptr;
      int* nrings;
      int* npoints;
      ptr += sizeof( int );
      for ( int index = 0;index < *npolys;++index )
      {
        ptr += ( 1 + sizeof( int ) ); //skip endian and polygon type
        nrings = ( int* )ptr;
        ptr += sizeof( int );
        for ( int index2 = 0;index2 < *nrings;++index2 )
        {
          npoints = ( int* )ptr;
          ptr += sizeof( int );
          for ( int index3 = 0;index3 < *npoints;++index3 )
          {
            tempx = ( double* )ptr;
            ptr += sizeof( double );
            tempy = ( double* )ptr;
            if ( point.sqrDist( *tempx, *tempy ) < actdist )
            {
              x = *tempx;
              y = *tempy;
              actdist = point.sqrDist( *tempx, *tempy );
              vertexnr = vertexcounter;

              //assign the rubber band indices
              if ( index3 == 0 )
              {
                beforeVertex = vertexcounter + ( *npoints - 2 );
                afterVertex = vertexcounter + 1;
              }
              else if ( index3 == ( *npoints - 1 ) )
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
            ptr += sizeof( double );
            if ( hasZValue ) //skip z-coordinate for 25D geometries
            {
              ptr += sizeof( double );
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
