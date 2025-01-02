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
#include "qgsspatialindex.h"
#include "qgsmultipoint.h"
#include "qgsattributes.h"

///@cond PRIVATE

QString QgsDelaunayTriangulationAlgorithm::name() const
{
  return QStringLiteral( "delaunaytriangulation" );
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
  return QStringLiteral( "vectorgeometry" );
}

QString QgsDelaunayTriangulationAlgorithm::shortHelpString() const
{
  return QObject::tr( "Creates a polygon layer with the Delaunay triangulation corresponding to a points layer." );
}

QgsDelaunayTriangulationAlgorithm *QgsDelaunayTriangulationAlgorithm::createInstance() const
{
  return new QgsDelaunayTriangulationAlgorithm();
}

void QgsDelaunayTriangulationAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) ) );
  std::unique_ptr<QgsProcessingParameterNumber> toleranceParam = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "TOLERANCE" ), QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Double, 0, true, 0 );
  toleranceParam->setHelp( QObject::tr( "Specifies an optional snapping tolerance which can be used to improve the robustness of the triangulation" ) );
  addParameter( toleranceParam.release() );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "ADD_ATTRIBUTES" ), QObject::tr( "Add point IDs to output" ), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Delaunay triangulation" ), Qgis::ProcessingSourceType::VectorPolygon ) );
}

QVariantMap QgsDelaunayTriangulationAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  if ( source->featureCount() < 3 )
    throw QgsProcessingException( QObject::tr( "Input layer should contain at least 3 points." ) );

  const double tolerance = parameterAsDouble( parameters, QStringLiteral( "TOLERANCE" ), context );
  const bool addAttributes = parameterAsBool( parameters, QStringLiteral( "ADD_ATTRIBUTES" ), context );

  QgsFields fields;
  if ( addAttributes )
  {
    fields.append( QgsField( QStringLiteral( "POINTA" ), QMetaType::Type::LongLong ) );
    fields.append( QgsField( QStringLiteral( "POINTB" ), QMetaType::Type::LongLong ) );
    fields.append( QgsField( QStringLiteral( "POINTC" ), QMetaType::Type::LongLong ) );
  }
  else
  {
    fields.append( QgsField( QStringLiteral( "id" ), QMetaType::Type::LongLong ) );
  }

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, Qgis::WkbType::Polygon, source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

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
        allPoints.addPartV2( qgsgeometry_cast< QgsPoint * >( *pit )->clone(), Qgis::WkbType::Point );
      }
    }
    else
    {
      allPoints.addPartV2( qgsgeometry_cast< QgsPoint * >( geom )->clone(), Qgis::WkbType::Point );
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
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      feedback->setProgress( 50 + i * step );
    }
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
