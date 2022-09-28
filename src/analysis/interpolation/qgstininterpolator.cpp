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
#include "qgsdualedgetriangulation.h"
#include "NormVecDecorator.h"
#include "LinTriangleInterpolator.h"
#include "qgspoint.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgsvariantutils.h"
#include "qgsfeedback.h"
#include "qgscurve.h"
#include "qgsmulticurve.h"
#include "qgscurvepolygon.h"
#include "qgsmultisurface.h"

QgsTinInterpolator::QgsTinInterpolator( const QList<LayerData> &inputData, TinInterpolation interpolation, QgsFeedback *feedback )
  : QgsInterpolator( inputData )
  , mIsInitialized( false )
  , mFeedback( feedback )
  , mInterpolation( interpolation )
{
}

QgsTinInterpolator::~QgsTinInterpolator()
{
  delete mTriangulation;
  delete mTriangleInterpolator;
}

int QgsTinInterpolator::interpolatePoint( double x, double y, double &result, QgsFeedback * )
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
  if ( !mTriangleInterpolator->calcPoint( x, y, r ) )
  {
    return 2;
  }
  result = r.z();
  return 0;
}

QgsFields QgsTinInterpolator::triangulationFields()
{
  return QgsTriangulation::triangulationFields();
}

void QgsTinInterpolator::setTriangulationSink( QgsFeatureSink *sink )
{
  mTriangulationSink = sink;
}

void QgsTinInterpolator::initialize()
{
  QgsDualEdgeTriangulation *dualEdgeTriangulation = new QgsDualEdgeTriangulation( 100000 );
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
  long long nFeatures = 0;
  long long nProcessedFeatures = 0;
  if ( mFeedback )
  {
    for ( const LayerData &layer :  std::as_const( mLayerData ) )
    {
      if ( layer.source )
      {
        nFeatures += layer.source->featureCount();
      }
    }
  }

  const QgsCoordinateReferenceSystem crs = !mLayerData.empty() ? mLayerData.at( 0 ).source->sourceCrs() : QgsCoordinateReferenceSystem();

  QgsFeature f;
  for ( const LayerData &layer : std::as_const( mLayerData ) )
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

      QgsFeatureIterator fit = layer.source->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( attList ).setDestinationCrs( crs, layer.transformContext ) );

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
    NormVecDecorator *dec = dynamic_cast<NormVecDecorator *>( mTriangulation );
    if ( dec )
    {
      auto ctInterpolator = std::make_unique<CloughTocherInterpolator>();
      dec->estimateFirstDerivatives( mFeedback );
      ctInterpolator->setTriangulation( dec );
      mTriangleInterpolator = ctInterpolator.release();
      dec->setTriangleInterpolator( mTriangleInterpolator );
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

int QgsTinInterpolator::insertData( const QgsFeature &f, QgsInterpolator::ValueSource source, int attr, SourceType type )
{
  QgsGeometry g = f.geometry();
  if ( g.isNull() || g.isEmpty() )
  {
    return 2;
  }

  //check attribute value
  double attributeValue = 0;
  bool attributeConversionOk = false;
  switch ( source )
  {
    case ValueAttribute:
    {
      QVariant attributeVariant = f.attribute( attr );
      if ( QgsVariantUtils::isNull( attributeVariant ) ) //attribute not found, something must be wrong (e.g. NULL value)
      {
        return 3;
      }
      attributeValue = attributeVariant.toDouble( &attributeConversionOk );
      if ( !attributeConversionOk || std::isnan( attributeValue ) ) //don't consider vertices with attributes like 'nan' for the interpolation
      {
        return 4;
      }
      break;
    }

    case ValueM:
      if ( !g.constGet()->isMeasure() )
        return 3;
      else
        break;

    case ValueZ:
      if ( !g.constGet()->is3D() )
        return 3;
      else
        break;
  }


  switch ( type )
  {
    case SourcePoints:
    {
      if ( addPointsFromGeometry( g, source, attributeValue ) != 0 )
        return -1;
      break;
    }

    case SourceBreakLines:
    case SourceStructureLines:
    {
      switch ( QgsWkbTypes::geometryType( g.wkbType() ) )
      {
        case QgsWkbTypes::PointGeometry:
        {
          if ( addPointsFromGeometry( g, source, attributeValue ) != 0 )
            return -1;
          break;
        }

        case QgsWkbTypes::LineGeometry:
        case QgsWkbTypes::PolygonGeometry:
        {
          // need to extract all rings from input geometry
          std::vector<const QgsCurve *> curves;
          if ( QgsWkbTypes::geometryType( g.wkbType() ) == QgsWkbTypes::PolygonGeometry )
          {
            std::vector< const QgsCurvePolygon * > polygons;
            if ( g.isMultipart() )
            {
              const QgsMultiSurface *ms = qgsgeometry_cast< const QgsMultiSurface * >( g.constGet() );
              for ( int i = 0; i < ms->numGeometries(); ++i )
              {
                polygons.emplace_back( qgsgeometry_cast< const QgsCurvePolygon * >( ms->geometryN( i ) ) );
              }
            }
            else
            {
              polygons.emplace_back( qgsgeometry_cast< const QgsCurvePolygon * >( g.constGet() ) );
            }

            for ( const QgsCurvePolygon *polygon : polygons )
            {
              if ( !polygon )
                continue;

              if ( polygon->exteriorRing() )
                curves.emplace_back( polygon->exteriorRing() );

              for ( int i = 0; i < polygon->numInteriorRings(); ++i )
              {
                curves.emplace_back( polygon->interiorRing( i ) );
              }
            }
          }
          else
          {
            if ( g.isMultipart() )
            {
              const QgsMultiCurve *mc = qgsgeometry_cast< const QgsMultiCurve * >( g.constGet() );
              for ( int i = 0; i < mc->numGeometries(); ++i )
              {
                curves.emplace_back( mc->curveN( i ) );
              }
            }
            else
            {
              curves.emplace_back( qgsgeometry_cast< const QgsCurve * >( g.constGet() ) );
            }
          }

          for ( const QgsCurve *curve : curves )
          {
            if ( !curve )
              continue;

            QgsPointSequence linePoints;
            curve->points( linePoints );
            for ( QgsPoint &point : linePoints )
            {
              switch ( source )
              {
                case ValueAttribute:
                  if ( point.is3D() )
                    point.setZ( attributeValue );
                  else
                    point.addZValue( attributeValue );
                  break;

                case ValueM:
                  if ( point.is3D() )
                    point.setZ( point.m() );
                  else
                    point.addZValue( point.m() );
                  break;

                case ValueZ:
                  break;
              }
            }
            mTriangulation->addLine( linePoints, type );
          }
          break;
        }
        case QgsWkbTypes::UnknownGeometry:
        case QgsWkbTypes::NullGeometry:
          break;
      }
      break;
    }
  }

  return 0;
}


int QgsTinInterpolator::addPointsFromGeometry( const QgsGeometry &g, ValueSource source, double attributeValue )
{
  // loop through all vertices and add to triangulation
  for ( auto point = g.vertices_begin(); point != g.vertices_end(); ++point )
  {
    QgsPoint p = *point;
    double z = 0;
    switch ( source )
    {
      case ValueAttribute:
        z = attributeValue;
        break;

      case ValueZ:
        z = p.z();
        break;

      case ValueM:
        z = p.m();
        break;
    }
    if ( mTriangulation->addPoint( QgsPoint( p.x(), p.y(), z ) ) == -100 )
    {
      return -1;
    }
  }
  return 0;
}
