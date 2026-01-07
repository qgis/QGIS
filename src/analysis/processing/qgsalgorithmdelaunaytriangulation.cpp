/***************************************************************************
                         qgsalgorithmdelaunaytriangulation.cpp
                         ---------------------
    begin                : July 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmdelaunaytriangulation.h"

#include "qgsattributes.h"
#include "qgsmultipoint.h"
#include "qgsspatialindex.h"

///@cond PRIVATE

QString QgsDelaunayTriangulationAlgorithm::name() const
{
  return u"delaunaytriangulation"_s;
}

QString QgsDelaunayTriangulationAlgorithm::displayName() const
{
  return QObject::tr( "Delaunay triangulation" );
}

QStringList QgsDelaunayTriangulationAlgorithm::tags() const
{
  return QObject::tr( "delaunay,triangulation,polygons,voronoi" ).split( ',' );
}

QString QgsDelaunayTriangulationAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsDelaunayTriangulationAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsDelaunayTriangulationAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a polygon layer with the Delaunay triangulation corresponding to a points layer." );
}

QString QgsDelaunayTriangulationAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a polygon layer with the Delaunay triangulation corresponding to a points layer." );
}

QgsDelaunayTriangulationAlgorithm *QgsDelaunayTriangulationAlgorithm::createInstance() const
{
  return new QgsDelaunayTriangulationAlgorithm();
}

void QgsDelaunayTriangulationAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) ) );
  auto toleranceParam = std::make_unique<QgsProcessingParameterNumber>( u"TOLERANCE"_s, QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Double, 0, true, 0 );
  toleranceParam->setHelp( QObject::tr( "Specifies an optional snapping tolerance which can be used to improve the robustness of the triangulation" ) );
  addParameter( toleranceParam.release() );
  addParameter( new QgsProcessingParameterBoolean( u"ADD_ATTRIBUTES"_s, QObject::tr( "Add point IDs to output" ), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Delaunay triangulation" ), Qgis::ProcessingSourceType::VectorPolygon ) );
}

QVariantMap QgsDelaunayTriangulationAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  if ( source->featureCount() < 3 )
    throw QgsProcessingException( QObject::tr( "Input layer should contain at least 3 points." ) );

  const double tolerance = parameterAsDouble( parameters, u"TOLERANCE"_s, context );
  const bool addAttributes = parameterAsBool( parameters, u"ADD_ATTRIBUTES"_s, context );

  QgsFields fields;
  if ( addAttributes )
  {
    fields.append( QgsField( u"POINTA"_s, QMetaType::Type::LongLong ) );
    fields.append( QgsField( u"POINTB"_s, QMetaType::Type::LongLong ) );
    fields.append( QgsField( u"POINTC"_s, QMetaType::Type::LongLong ) );
  }
  else
  {
    fields.append( QgsField( u"id"_s, QMetaType::Type::LongLong ) );
  }

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, Qgis::WkbType::Polygon, source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QgsFeatureIterator it = source->getFeatures( QgsFeatureRequest().setNoAttributes(), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );

  QgsGeometry allPoints;

  long long i = 0;
  const double step = source->featureCount() > 0 ? 50.0 / source->featureCount() : 1;

  const QgsSpatialIndex index( it, [&]( const QgsFeature &f ) -> bool {
    i++;
    if ( feedback->isCanceled() )
      return false;

    feedback->setProgress( i * step );

    if ( !f.hasGeometry() )
      return true;

    const QgsAbstractGeometry *geom = f.geometry().constGet();
    if ( QgsWkbTypes::isMultiType( geom->wkbType() ) )
    {
      const QgsMultiPoint mp( *qgsgeometry_cast< const QgsMultiPoint * >( geom ) );
      for ( auto pit = mp.const_parts_begin(); pit != mp.const_parts_end(); ++pit )
      {
        allPoints.addPartV2( qgsgeometry_cast< const QgsPoint * >( *pit )->clone(), Qgis::WkbType::Point );
      }
    }
    else
    {
      allPoints.addPartV2( qgsgeometry_cast< const QgsPoint * >( geom )->clone(), Qgis::WkbType::Point );
    }

    return true; }, QgsSpatialIndex::FlagStoreFeatureGeometries );

  const QgsGeometry triangulation = allPoints.delaunayTriangulation( tolerance );

  if ( !triangulation.isEmpty() )
  {
    const QVector<QgsGeometry> collection = triangulation.asGeometryCollection();
    for ( int i = 0; i < collection.length(); i++ )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }
      QgsFeature f;
      f.setFields( fields );
      f.setGeometry( collection[i] );
      if ( addAttributes )
      {
        const QList<QgsFeatureId> nearest = index.nearestNeighbor( collection[i], 3 );
        QgsAttributes attrs;
        for ( int j = 0; j < 3; j++ )
        {
          attrs << nearest.at( j );
        }
        f.setAttributes( attrs );
      }
      else
      {
        f.setAttributes( QgsAttributes() << i );
      }

      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
      feedback->setProgress( 50 + i * step );
    }
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

///@endcond
