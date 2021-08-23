/***************************************************************************
                         qgsalgorithmexplode.cpp
                         ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmexplode.h"
#include "qgscurve.h"
#include "qgslinestring.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgsgeometrycollection.h"

///@cond PRIVATE

QString QgsExplodeAlgorithm::name() const
{
  return QStringLiteral( "explodelines" );
}

QString QgsExplodeAlgorithm::displayName() const
{
  return QObject::tr( "Explode lines" );
}

QStringList QgsExplodeAlgorithm::tags() const
{
  return QObject::tr( "segments,parts" ).split( ',' );
}

QString QgsExplodeAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsExplodeAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsExplodeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a lines layer and creates a new one in which each line is replaced by a set of "
                      "lines representing the segments in the original line. Each line in the resulting layer contains only a "
                      "start and an end point, with no intermediate nodes between them.\n\n"
                      "If the input layer consists of CircularStrings or CompoundCurves, the output layer will be of the "
                      "same type and contain only single curve segments." );
}

QList<int> QgsExplodeAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine;
}

QgsProcessing::SourceType QgsExplodeAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorLine;
}

QgsExplodeAlgorithm *QgsExplodeAlgorithm::createInstance() const
{
  return new QgsExplodeAlgorithm();
}

QString QgsExplodeAlgorithm::outputName() const
{
  return QObject::tr( "Exploded" );
}

QgsWkbTypes::Type QgsExplodeAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  return QgsWkbTypes::singleType( inputWkbType );
}

QgsFeatureList QgsExplodeAlgorithm::processFeature( const QgsFeature &f, QgsProcessingContext &, QgsProcessingFeedback * )
{
  if ( !f.hasGeometry() )
  {
    return QgsFeatureList() << f;
  }
  else
  {
    const std::vector<QgsGeometry> parts = extractAsParts( f.geometry() );
    QgsFeature outputFeature;
    QgsFeatureList features;
    features.reserve( parts.size() );
    for ( const QgsGeometry &part : parts )
    {
      outputFeature.setAttributes( f.attributes() );
      outputFeature.setGeometry( part );
      features << outputFeature;
    }
    return features;
  }
}

QgsFeatureSink::SinkFlags QgsExplodeAlgorithm::sinkFlags() const
{
  return QgsFeatureSink::RegeneratePrimaryKey;
}

std::vector<QgsGeometry> QgsExplodeAlgorithm::extractAsParts( const QgsGeometry &geometry ) const
{
  if ( geometry.isMultipart() )
  {
    std::vector<QgsGeometry> parts;
    const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( geometry.constGet() );
    for ( int part = 0; part < collection->numGeometries(); ++part )
    {
      std::vector<QgsGeometry> segments = curveAsSingleSegments( qgsgeometry_cast< const QgsCurve * >( collection->geometryN( part ) ) );
      parts.reserve( parts.size() + segments.size() );
      std::move( std::begin( segments ), std::end( segments ), std::back_inserter( parts ) );
    }
    return parts;
  }
  else
  {
    return curveAsSingleSegments( qgsgeometry_cast< const QgsCurve * >( geometry.constGet() ) );
  }
}

std::vector<QgsGeometry> QgsExplodeAlgorithm::curveAsSingleSegments( const QgsCurve *curve, bool useCompoundCurves ) const
{
  std::vector<QgsGeometry> parts;
  if ( !curve )
    return parts;
  switch ( QgsWkbTypes::flatType( curve->wkbType() ) )
  {
    case QgsWkbTypes::LineString:
    {
      const QgsLineString *line = qgsgeometry_cast< const QgsLineString * >( curve );
      for ( int i = 0; i < line->numPoints() - 1; ++i )
      {
        const QgsPoint ptA = line->pointN( i );
        const QgsPoint ptB = line->pointN( i + 1 );
        std::unique_ptr< QgsLineString > ls = std::make_unique< QgsLineString >( QVector< QgsPoint >() << ptA << ptB );
        if ( !useCompoundCurves )
        {
          parts.emplace_back( QgsGeometry( std::move( ls ) ) );
        }
        else
        {
          std::unique_ptr< QgsCompoundCurve > cc = std::make_unique< QgsCompoundCurve >();
          cc->addCurve( ls.release() );
          parts.emplace_back( QgsGeometry( std::move( cc ) ) );
        }
      }
      break;
    }

    case QgsWkbTypes::CircularString:
    {
      const QgsCircularString *string = qgsgeometry_cast< const QgsCircularString * >( curve );
      for ( int i = 0; i < string->numPoints() - 2; i += 2 )
      {
        const QgsPoint ptA = string->pointN( i );
        const QgsPoint ptB = string->pointN( i + 1 );
        const QgsPoint ptC = string->pointN( i + 2 );
        std::unique_ptr< QgsCircularString > cs = std::make_unique< QgsCircularString >();
        cs->setPoints( QgsPointSequence() << ptA << ptB << ptC );
        if ( !useCompoundCurves )
        {
          parts.emplace_back( QgsGeometry( std::move( cs ) ) );
        }
        else
        {
          std::unique_ptr< QgsCompoundCurve > cc = std::make_unique< QgsCompoundCurve >();
          cc->addCurve( cs.release() );
          parts.emplace_back( QgsGeometry( std::move( cc ) ) );
        }
      }
      break;
    }

    case QgsWkbTypes::CompoundCurve:
    {
      const QgsCompoundCurve *compoundCurve = qgsgeometry_cast< QgsCompoundCurve * >( curve );
      for ( int i = 0; i < compoundCurve->nCurves(); ++i )
      {
        std::vector<QgsGeometry> segments = curveAsSingleSegments( compoundCurve->curveAt( i ), true );
        parts.reserve( parts.size() + segments.size() );
        std::move( std::begin( segments ), std::end( segments ), std::back_inserter( parts ) );
      }
      break;
    }

    default:
      break;

  }
  return parts;
}

///@endcond



