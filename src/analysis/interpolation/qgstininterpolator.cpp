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
#include "qgsfeatureiterator.h"
#include "CloughTocherInterpolator.h"
#include "DualEdgeTriangulation.h"
#include "NormVecDecorator.h"
#include "LinTriangleInterpolator.h"
#include "Line3D.h"
#include "qgspoint.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgswkbptr.h"
#include "qgsfeedback.h"

QgsTINInterpolator::QgsTINInterpolator( const QList<LayerData> &inputData, TINInterpolation interpolation, QgsFeedback *feedback )
  : QgsInterpolator( inputData )
  , mIsInitialized( false )
  , mFeedback( feedback )
  , mInterpolation( interpolation )
{
}

QgsTINInterpolator::~QgsTINInterpolator()
{
  delete mTriangulation;
  delete mTriangleInterpolator;
}

int QgsTINInterpolator::interpolatePoint( double x, double y, double &result, QgsFeedback * )
{
  if ( !mIsInitialized )
  {
    initialize();
  }

  if ( !mTriangleInterpolator )
  {
    return 1;
  }

  QgsPoint r( 0, 0, 0 );
  if ( !mTriangleInterpolator->calcPoint( x, y, &r ) )
  {
    return 2;
  }
  result = r.z();
  return 0;
}

QgsFields QgsTINInterpolator::triangulationFields()
{
  return Triangulation::triangulationFields();
}

void QgsTINInterpolator::setTriangulationSink( QgsFeatureSink *sink )
{
  mTriangulationSink = sink;
}

void QgsTINInterpolator::initialize()
{
  DualEdgeTriangulation *dualEdgeTriangulation = new DualEdgeTriangulation( 100000, nullptr );
  if ( mInterpolation == CloughTocher )
  {
    NormVecDecorator *dec = new NormVecDecorator();
    dec->addTriangulation( dualEdgeTriangulation );
    mTriangulation = dec;
  }
  else
  {
    mTriangulation = dualEdgeTriangulation;
  }

  //get number of features if we use a progress bar
  int nFeatures = 0;
  int nProcessedFeatures = 0;
  if ( mFeedback )
  {
    for ( const LayerData &layer :  qgis::as_const( mLayerData ) )
    {
      if ( layer.source )
      {
        nFeatures += layer.source->featureCount();
      }
    }
  }

  QgsFeature f;
  for ( const LayerData &layer : qgis::as_const( mLayerData ) )
  {
    if ( layer.source )
    {
      QgsAttributeList attList;
      switch ( layer.valueSource )
      {
        case QgsInterpolator::ValueAttribute:
          attList.push_back( layer.interpolationAttribute );
          break;

        case QgsInterpolator::ValueM:
        case QgsInterpolator::ValueZ:
          break;
      }

      QgsFeatureIterator fit = layer.source->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( attList ) );

      while ( fit.nextFeature( f ) )
      {
        if ( mFeedback )
        {
          if ( mFeedback->isCanceled() )
          {
            break;
          }
          if ( nFeatures > 0 )
            mFeedback->setProgress( 100.0 * static_cast< double >( nProcessedFeatures ) / nFeatures );
        }
        insertData( f, layer.valueSource, layer.interpolationAttribute, layer.sourceType );
        ++nProcessedFeatures;
      }
    }
  }

  if ( mInterpolation == CloughTocher )
  {
    CloughTocherInterpolator *ctInterpolator = new CloughTocherInterpolator();
    NormVecDecorator *dec = dynamic_cast<NormVecDecorator *>( mTriangulation );
    if ( dec )
    {
      dec->estimateFirstDerivatives( mFeedback );
      ctInterpolator->setTriangulation( dec );
      dec->setTriangleInterpolator( ctInterpolator );
      mTriangleInterpolator = ctInterpolator;
    }
  }
  else //linear
  {
    mTriangleInterpolator = new LinTriangleInterpolator( dualEdgeTriangulation );
  }
  mIsInitialized = true;

  //debug
  if ( mTriangulationSink )
  {
    dualEdgeTriangulation->saveTriangulation( mTriangulationSink, mFeedback );
  }
}

int QgsTINInterpolator::insertData( const QgsFeature &f, bool zCoord, int attr, SourceType type )
{
  QgsGeometry g = f.geometry();
  {
    if ( g.isNull() || g.isEmpty() )
    {
      return 2;
    }
  }

  //check attribute value
  double attributeValue = 0;
  bool attributeConversionOk = false;
  if ( !zCoord )
  {
    QVariant attributeVariant = f.attribute( attr );
    if ( !attributeVariant.isValid() ) //attribute not found, something must be wrong (e.g. NULL value)
    {
      return 3;
    }
    attributeValue = attributeVariant.toDouble( &attributeConversionOk );
    if ( !attributeConversionOk || std::isnan( attributeValue ) ) //don't consider vertices with attributes like 'nan' for the interpolation
    {
      return 4;
    }
  }

  //parse WKB. It is ugly, but we cannot use the methods with QgsPointXY because they don't contain z-values for 25D types
  bool hasZValue = false;
  double x, y, z;
  QByteArray wkb( g.exportToWkb() );
  QgsConstWkbPtr currentWkbPtr( wkb );
  currentWkbPtr.readHeader();
  //maybe a structure or break line
  Line3D *line = nullptr;

  QgsWkbTypes::Type wkbType = g.wkbType();
  switch ( wkbType )
  {
    case QgsWkbTypes::Point25D:
      hasZValue = true;
      FALLTHROUGH;
    case QgsWkbTypes::Point:
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
      QgsPoint *point = new QgsPoint( x, y, z );
      if ( mTriangulation->addPoint( point ) == -100 )
      {
        return -1;
      }
      break;
    }
    case QgsWkbTypes::MultiPoint25D:
      hasZValue = true;
      FALLTHROUGH;
    case QgsWkbTypes::MultiPoint:
    {
      int nPoints;
      currentWkbPtr >> nPoints;
      for ( int index = 0; index < nPoints; ++index )
      {
        currentWkbPtr.readHeader();
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
    case QgsWkbTypes::LineString25D:
      hasZValue = true;
      FALLTHROUGH;
    case QgsWkbTypes::LineString:
    {
      if ( type != SourcePoints )
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

        if ( type == SourcePoints )
        {
          //todo: handle error code -100
          mTriangulation->addPoint( new QgsPoint( x, y, z ) );
        }
        else
        {
          line->insertPoint( new QgsPoint( x, y, z ) );
        }
      }

      if ( type != SourcePoints )
      {
        mTriangulation->addLine( line, type == SourceBreakLines );
      }
      break;
    }
    case QgsWkbTypes::MultiLineString25D:
      hasZValue = true;
      FALLTHROUGH;
    case QgsWkbTypes::MultiLineString:
    {
      int nLines;
      currentWkbPtr >> nLines;
      for ( int index = 0; index < nLines; ++index )
      {
        if ( type != SourcePoints )
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

          if ( type == SourcePoints )
          {
            //todo: handle error code -100
            mTriangulation->addPoint( new QgsPoint( x, y, z ) );
          }
          else
          {
            line->insertPoint( new QgsPoint( x, y, z ) );
          }
        }
        if ( type != SourcePoints )
        {
          mTriangulation->addLine( line, type == SourceBreakLines );
        }
      }
      break;
    }
    case QgsWkbTypes::Polygon25D:
      hasZValue = true;
      FALLTHROUGH;
    case QgsWkbTypes::Polygon:
    {
      int nRings;
      currentWkbPtr >> nRings;
      for ( int index = 0; index < nRings; ++index )
      {
        if ( type != SourcePoints )
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
          if ( type == SourcePoints )
          {
            //todo: handle error code -100
            mTriangulation->addPoint( new QgsPoint( x, y, z ) );
          }
          else
          {
            line->insertPoint( new QgsPoint( x, y, z ) );
          }
        }

        if ( type != SourcePoints )
        {
          mTriangulation->addLine( line, type == SourceBreakLines );
        }
      }
      break;
    }

    case QgsWkbTypes::MultiPolygon25D:
      hasZValue = true;
      FALLTHROUGH;
    case QgsWkbTypes::MultiPolygon:
    {
      int nPolys;
      currentWkbPtr >> nPolys;
      for ( int index = 0; index < nPolys; ++index )
      {
        currentWkbPtr.readHeader();
        int nRings;
        currentWkbPtr >> nRings;
        for ( int index2 = 0; index2 < nRings; ++index2 )
        {
          if ( type != SourcePoints )
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
            if ( type == SourcePoints )
            {
              //todo: handle error code -100
              mTriangulation->addPoint( new QgsPoint( x, y, z ) );
            }
            else
            {
              line->insertPoint( new QgsPoint( x, y, z ) );
            }
          }
          if ( type != SourcePoints )
          {
            mTriangulation->addLine( line, type == SourceBreakLines );
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

