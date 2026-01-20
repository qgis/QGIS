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
  return u"polygonize"_s;
}

QString QgsPolygonizeAlgorithm::displayName() const
{
  return QObject::tr( "Polygonize" );
}

QString QgsPolygonizeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a polygon layer from the input lines layer." );
}

QString QgsPolygonizeAlgorithm::shortDescription() const
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
  return u"vectorgeometry"_s;
}

void QgsPolygonizeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) ) );
  addParameter( new QgsProcessingParameterBoolean( u"KEEP_FIELDS"_s, QObject::tr( "Keep table structure of line layer" ), false ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Polygons" ), Qgis::ProcessingSourceType::VectorPolygon ) );
  addOutput( new QgsProcessingOutputNumber( u"NUM_POLYGONS"_s, QObject::tr( "Number of polygons" ) ) );
}

QgsPolygonizeAlgorithm *QgsPolygonizeAlgorithm::createInstance() const
{
  return new QgsPolygonizeAlgorithm();
}

QVariantMap QgsPolygonizeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  QgsFields fields = QgsFields();
  if ( parameterAsBoolean( parameters, u"KEEP_FIELDS"_s, context ) )
    fields = source->fields();

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, Qgis::WkbType::Polygon, source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

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
  const QgsGeometry polygons = QgsGeometry::polygonize( QVector<QgsGeometry>() << lines );
  if ( polygons.isEmpty() )
    feedback->reportError( QObject::tr( "No polygons were created." ) );

  feedback->setProgress( 50 );

  if ( !polygons.isEmpty() )
  {
    const QgsGeometryCollection *collection = qgsgeometry_cast<const QgsGeometryCollection *>( polygons.constGet() );
    const int numGeometries = collection ? collection->numGeometries() : 1;
    step = numGeometries > 0 ? 50.0 / numGeometries : 1;

    int part = 0;
    for ( auto partIt = polygons.const_parts_begin(); partIt != polygons.const_parts_end(); ++partIt, ++part )
    {
      if ( feedback->isCanceled() )
        break;

      QgsFeature outFeat;
      outFeat.setGeometry( QgsGeometry( ( *partIt )->clone() ) );
      if ( !sink->addFeature( outFeat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
      feedback->setProgress( 50 + part * step );
      polygonCount += 1;
    }
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  outputs.insert( u"NUM_POLYGONS"_s, polygonCount );
  return outputs;
}

///@endcond
