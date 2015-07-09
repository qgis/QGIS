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
#include "CloughTocherInterpolator.h"
#include "DualEdgeTriangulation.h"
#include "NormVecDecorator.h"
#include "LinTriangleInterpolator.h"
#include "Point3D.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgswkbptr.h"
#include <QProgressDialog>

QgsTINInterpolator::QgsTINInterpolator( const QList<LayerData>& inputData, TIN_INTERPOLATION interpolation, bool showProgressDialog )
    : QgsInterpolator( inputData )
    , mTriangulation( 0 )
    , mTriangleInterpolator( 0 )
    , mIsInitialized( false )
    , mShowProgressDialog( showProgressDialog )
    , mExportTriangulationToFile( false )
    , mInterpolation( interpolation )
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
  if ( mInterpolation == CloughTocher )
  {
    NormVecDecorator* dec = new NormVecDecorator();
    dec->addTriangulation( theDualEdgeTriangulation );
    mTriangulation = dec;
  }
  else
  {
    mTriangulation = theDualEdgeTriangulation;
  }

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

      QgsFeatureIterator fit = layerDataIt->vectorLayer->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( attList ) );

      while ( fit.nextFeature( f ) )
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

  if ( mInterpolation == CloughTocher )
  {
    CloughTocherInterpolator* ctInterpolator = new CloughTocherInterpolator();
    NormVecDecorator* dec = dynamic_cast<NormVecDecorator*>( mTriangulation );
    if ( dec )
    {
      QProgressDialog* progressDialog = 0;
      if ( mShowProgressDialog ) //show a progress dialog because it can take a long time...
      {
        progressDialog = new QProgressDialog();
        progressDialog->setLabelText( QObject::tr( "Estimating normal derivatives..." ) );
      }
      dec->estimateFirstDerivatives( progressDialog );
      delete progressDialog;
      ctInterpolator->setTriangulation( dec );
      dec->setTriangleInterpolator( ctInterpolator );
      mTriangleInterpolator = ctInterpolator;
    }
  }
  else //linear
  {
    mTriangleInterpolator = new LinTriangleInterpolator( theDualEdgeTriangulation );
  }
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

  const QgsGeometry* g = f->constGeometry();
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
    QVariant attributeVariant = f->attribute( attr );
    if ( !attributeVariant.isValid() ) //attribute not found, something must be wrong (e.g. NULL value)
    {
      return 3;
    }
    attributeValue = attributeVariant.toDouble( &attributeConversionOk );
    if ( !attributeConversionOk || qIsNaN( attributeValue ) ) //don't consider vertices with attributes like 'nan' for the interpolation
    {
      return 4;
    }
  }

  //parse WKB. It is ugly, but we cannot use the methods with QgsPoint because they don't contain z-values for 25D types
  bool hasZValue = false;
  double x, y, z;
  QgsConstWkbPtr currentWkbPtr( g->asWkb() + 1 + sizeof( int ) );
  //maybe a structure or break line
  Line3D* line = 0;

  QGis::WkbType wkbType = g->wkbType();
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
      hasZValue = true;
    case QGis::WKBPoint:
    {
      currentWkbPtr >> x >> y;
      if ( zCoord && hasZValue )
      {
        currentWkbPtr >> z;
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
    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
    {
      int nPoints;
      currentWkbPtr >> nPoints;
      for ( int index = 0; index < nPoints; ++index )
      {
        currentWkbPtr += 1 + sizeof( int );
        currentWkbPtr >> x >> y;
        if ( hasZValue ) //skip z-coordinate for 25D geometries
        {
          currentWkbPtr >> z;
        }
        else
        {
          z = attributeValue;
        }
      }
      break;
    }
    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {
      if ( type != POINTS )
      {
        line = new Line3D();
      }
      int nPoints;
      currentWkbPtr >> nPoints;
      for ( int index = 0; index < nPoints; ++index )
      {
        currentWkbPtr >> x >> y;
        if ( zCoord && hasZValue ) //skip z-coordinate for 25D geometries
        {
          currentWkbPtr >> z;
        }
        else
        {
          z = attributeValue;
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
    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      int nLines;
      currentWkbPtr >> nLines;
      for ( int index = 0; index < nLines; ++index )
      {
        if ( type != POINTS )
        {
          line = new Line3D();
        }
        int nPoints;
        currentWkbPtr >> nPoints;
        for ( int index2 = 0; index2 < nPoints; ++index2 )
        {
          currentWkbPtr >> x >> y;
          if ( hasZValue ) //skip z-coordinate for 25D geometries
          {
            currentWkbPtr >> z;
          }
          else
          {
            z = attributeValue;
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
      }
      break;
    }
    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
    {
      int nRings;
      currentWkbPtr >> nRings;
      for ( int index = 0; index < nRings; ++index )
      {
        if ( type != POINTS )
        {
          line = new Line3D();
        }

        int nPoints;
        currentWkbPtr >> nPoints;
        for ( int index2 = 0; index2 < nPoints; ++index2 )
        {
          currentWkbPtr >> x >> y;
          if ( hasZValue ) //skip z-coordinate for 25D geometries
          {
            currentWkbPtr >> z;
          }
          else
          {
            z = attributeValue;
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
      }
      break;
    }

    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      int nPolys;
      currentWkbPtr >> nPolys;
      for ( int index = 0; index < nPolys; ++index )
      {
        currentWkbPtr += 1 + sizeof( int );
        int nRings;
        currentWkbPtr >> nRings;
        for ( int index2 = 0; index2 < nRings; ++index2 )
        {
          if ( type != POINTS )
          {
            line = new Line3D();
          }
          int nPoints;
          currentWkbPtr >> nPoints;
          for ( int index3 = 0; index3 < nPoints; ++index3 )
          {
            currentWkbPtr >> x >> y;
            if ( hasZValue ) //skip z-coordinate for 25D geometries
            {
              currentWkbPtr >> z;
            }
            else
            {
              z = attributeValue;
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
        }
      }
      break;
    }
    default:
      //should not happen...
      break;
  }

  return 0;
}

