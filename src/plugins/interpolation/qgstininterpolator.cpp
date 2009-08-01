/***************************************************************************
                              qgstininterpolator.cpp
                              ----------------------
  begin                : March 10, 2008
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

#include "qgstininterpolator.h"
#include "DualEdgeTriangulation.h"
#include "LinTriangleInterpolator.h"
#include "Point3D.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsvectorlayer.h"
#include <QProgressDialog>

#ifndef Q_OS_MACX
#include <cmath>
#else
#include <math.h>
#endif
#ifdef WIN32
#include <float.h>
#define isnan(f) _isnan(f)
#endif

QgsTINInterpolator::QgsTINInterpolator( const QList<LayerData>& inputData, bool showProgressDialog ): QgsInterpolator( inputData ), mTriangulation( 0 ), \
    mTriangleInterpolator( 0 ), mIsInitialized( false ), mShowProgressDialog( showProgressDialog ), mExportTriangulationToFile( false )
{
}

QgsTINInterpolator::~QgsTINInterpolator()
{
  delete mTriangulation;
  delete mTriangleInterpolator;
}

int QgsTINInterpolator::interpolatePoint( double x, double y, double& result )
{
  if ( !mIsInitialized )
  {
    initialize();
  }

  if ( !mTriangleInterpolator )
  {
    return 1;
  }

  Point3D r;
  if ( !mTriangleInterpolator->calcPoint( x, y, &r ) )
  {
    return 2;
  }
  result = r.getZ();
  return 0;
}

void QgsTINInterpolator::initialize()
{
  DualEdgeTriangulation* theDualEdgeTriangulation = new DualEdgeTriangulation( 100000, 0 );
  mTriangulation = theDualEdgeTriangulation;

  //get number of features if we use a progress bar
  int nFeatures = 0;
  int nProcessedFeatures = 0;
  if ( mShowProgressDialog )
  {
    QList<LayerData>::iterator layerDataIt = mLayerData.begin();
    for ( ; layerDataIt != mLayerData.end(); ++layerDataIt )
    {
      if ( layerDataIt->vectorLayer )
      {
        nFeatures += layerDataIt->vectorLayer->featureCount();
      }
    }
  }

  QProgressDialog* theProgressDialog = 0;
  if ( mShowProgressDialog )
  {
    theProgressDialog = new QProgressDialog( QObject::tr( "Building triangulation..." ), QObject::tr( "Abort" ), 0, nFeatures, 0 );
    theProgressDialog->setWindowModality( Qt::WindowModal );
  }


  QgsFeature f;
  QList<LayerData>::iterator layerDataIt = mLayerData.begin();
  for ( ; layerDataIt != mLayerData.end(); ++layerDataIt )
  {
    if ( layerDataIt->vectorLayer )
    {
      QgsAttributeList attList;
      if ( !layerDataIt->zCoordInterpolation )
      {
        attList.push_back( layerDataIt->interpolationAttribute );
      }
      layerDataIt->vectorLayer->select( attList );
      while ( layerDataIt->vectorLayer->nextFeature( f ) )
      {
        if ( mShowProgressDialog )
        {
          if ( theProgressDialog->wasCanceled() )
          {
            break;
          }
          theProgressDialog->setValue( nProcessedFeatures );
        }
        insertData( &f, layerDataIt->zCoordInterpolation, layerDataIt->interpolationAttribute, layerDataIt->mInputType );
        ++nProcessedFeatures;
      }
    }
  }

  delete theProgressDialog;
  mTriangleInterpolator = new LinTriangleInterpolator( theDualEdgeTriangulation );
  mIsInitialized = true;

  //debug
  if ( mExportTriangulationToFile )
  {
    theDualEdgeTriangulation->saveAsShapefile( mTriangulationFilePath );
  }
}

int QgsTINInterpolator::insertData( QgsFeature* f, bool zCoord, int attr, InputType type )
{
  if ( !f )
  {
    return 1;
  }

  QgsGeometry* g = f->geometry();
  {
    if ( !g )
    {
      return 2;
    }
  }

  //check attribute value
  double attributeValue = 0;
  bool attributeConversionOk = false;
  if ( !zCoord )
  {
    QgsAttributeMap attMap = f->attributeMap();
    QgsAttributeMap::const_iterator att_it = attMap.find( attr );
    if ( att_it == attMap.end() ) //attribute not found, something must be wrong (e.g. NULL value)
    {
      return 3;
    }
    attributeValue = att_it.value().toDouble( &attributeConversionOk );
    if ( !attributeConversionOk || isnan( attributeValue ) ) //don't consider vertices with attributes like 'nan' for the interpolation
    {
      return 4;
    }
  }

  //parse WKB. We cannot use the methods with QgsPoint because they don't contain z-values for 25D types
  bool hasZValue = false;
  double x, y, z;
  unsigned char* currentWkbPtr = g->asWkb();

  QGis::WkbType wkbType = g->wkbType();
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
      hasZValue = true;
    case QGis::WKBPoint:
    {
      currentWkbPtr += ( 1 + sizeof( int ) );
      x = *(( double * )( currentWkbPtr ) );
      currentWkbPtr += sizeof( double );
      y = *(( double * )( currentWkbPtr ) );
      if ( zCoord && hasZValue )
      {
        currentWkbPtr += sizeof( double );
        z = *(( double * )( currentWkbPtr ) );
      }
      else
      {
        z = attributeValue;
      }
      Point3D* thePoint = new Point3D( x, y, z );
      if ( mTriangulation->addPoint( thePoint ) == -100 )
      {
        return -1;
      }
      break;
    }
    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {
      //maybe a structure or break line
      Line3D* line = 0;
      if ( type != POINTS )
      {
        line = new Line3D();
      }
      currentWkbPtr += ( 1 + sizeof( int ) );
      int* npoints = ( int* )currentWkbPtr;
      currentWkbPtr += sizeof( int );
      for ( int index = 0;index < *npoints;++index )
      {
        x = *(( double * )( currentWkbPtr ) );
        currentWkbPtr += sizeof( double );
        y = *(( double * )( currentWkbPtr ) );
        currentWkbPtr += sizeof( double );
        if ( zCoord && hasZValue ) //skip z-coordinate for 25D geometries
        {
          z = *(( double * )( currentWkbPtr ) );
        }
        else
        {
          z = attributeValue;
        }
        if ( hasZValue )
        {
          currentWkbPtr += sizeof( double );
        }

        if ( type == POINTS )
        {
          //todo: handle error code -100
          mTriangulation->addPoint( new Point3D( x, y, z ) );
        }
        else
        {
          line->insertPoint( new Point3D( x, y, z ) );
        }
      }

      if ( type != POINTS )
      {
        mTriangulation->addLine( line, type == BREAK_LINES );
      }
      break;
    }
    default:
      //todo: add the same for multiline, polygon, multipolygon
      break;
  }

  return 0;
}

