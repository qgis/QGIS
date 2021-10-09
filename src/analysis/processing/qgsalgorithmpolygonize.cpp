/***************************************************************************
                         qgsalgorithmdpolygonize.cpp
                         ---------------------
    begin                : May 2020
    copyright            : (C) 2020 by Alexander Bruy
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

#include "qgsalgorithmpolygonize.h"
#include "qgsgeometrycollection.h"

///@cond PRIVATE

QString QgsPolygonizeAlgorithm::name() const
{
  return QStringLiteral( "polygonize" );
}

QString QgsPolygonizeAlgorithm::displayName() const
{
  return QObject::tr( "Polygonize" );
}

QString QgsPolygonizeAlgorithm::shortHelpString() const
{
  return QObject::tr( "Creates a polygon layer from the input lines layer." );
}

QStringList QgsPolygonizeAlgorithm::tags() const
{
  return QObject::tr( "create,lines,polygons,convert" ).split( ',' );
}

QString QgsPolygonizeAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsPolygonizeAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

void QgsPolygonizeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorLine ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "KEEP_FIELDS" ),
                QObject::tr( "Keep table structure of line layer" ), false, true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ),
                QObject::tr( "Polygons" ), QgsProcessing::TypeVectorPolygon ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "NUM_POLYGONS" ), QObject::tr( "Number of polygons" ) ) );
}

QgsPolygonizeAlgorithm *QgsPolygonizeAlgorithm::createInstance() const
{
  return new QgsPolygonizeAlgorithm();
}

QVariantMap QgsPolygonizeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  QgsFields fields = QgsFields();
  if ( parameterAsBoolean( parameters, QStringLiteral( "KEEP_FIELDS" ), context ) )
    fields = source->fields();

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::Polygon, source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  int polygonCount = 0;

  feedback->pushInfo( QObject::tr( "Collecting lines…" ) );
  const int i = 0;
  double step = source->featureCount() > 0 ? 40.0 / source->featureCount() : 1;
  QgsFeature f;
  QgsFeatureIterator features = source->getFeatures( QgsFeatureRequest().setNoAttributes() );
  QVector<QgsGeometry> linesList;
  linesList.reserve( source->featureCount() );
  while ( features.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;

    if ( f.hasGeometry() )
      linesList << f.geometry();

    feedback->setProgress( i * step );
  }
  feedback->setProgress( 40 );

  feedback->pushInfo( QObject::tr( "Noding lines…" ) );
  const QgsGeometry lines = QgsGeometry::unaryUnion( linesList );
  if ( feedback->isCanceled() )
    return QVariantMap();
  feedback->setProgress( 45 );

  feedback->pushInfo( QObject::tr( "Polygonizing…" ) );
  const QgsGeometry polygons = QgsGeometry::polygonize( QVector< QgsGeometry >() << lines );
  if ( polygons.isEmpty() )
    feedback->reportError( QObject::tr( "No polygons were created." ) );

  feedback->setProgress( 50 );

  if ( !polygons.isEmpty() )
  {
    const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( polygons.constGet() );
    step = collection->numGeometries() > 0 ? 50.0 / collection->numGeometries() : 1;
    for ( int part = 0; part < collection->numGeometries(); ++part )
    {
      if ( feedback->isCanceled() )
        break;

      QgsFeature outFeat;
      outFeat.setGeometry( QgsGeometry( collection->geometryN( part )->clone() ) );
      if ( !sink->addFeature( outFeat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      feedback->setProgress( 50 + i * step );
      polygonCount += 1;
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  outputs.insert( QStringLiteral( "NUM_POLYGONS" ), polygonCount );
  return outputs;
}

///@endcond
