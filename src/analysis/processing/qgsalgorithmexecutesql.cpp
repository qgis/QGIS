/***************************************************************************
                         qgsalgorithmexecutesql.cpp
                         ---------------------------------
    begin                : August 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#include "qgsalgorithmexecutesql.h"

#include "qgsfeaturerequest.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include "qgsvirtuallayerdefinition.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE
QgsExecuteSqlAlgorithm::QgsExecuteSqlAlgorithm()
{
  mGeometryTypes = {
    { Qgis::WkbType::Unknown, QObject::tr( "Autodetect" ) },
    { Qgis::WkbType::NoGeometry, QObject::tr( "No geometry" ) },
    { Qgis::WkbType::Point, QObject::tr( "Point" ) },
    { Qgis::WkbType::LineString, QObject::tr( "LineString" ) },
    { Qgis::WkbType::Polygon, QObject::tr( "Polygon" ) },
    { Qgis::WkbType::MultiPoint, QObject::tr( "MultiPoint" ) },
    { Qgis::WkbType::MultiLineString, QObject::tr( "MultiLineString" ) },
    { Qgis::WkbType::MultiPolygon, QObject::tr( "MultiPolygon" ) }
  };
}

QString QgsExecuteSqlAlgorithm::name() const
{
  return u"executesql"_s;
}

QString QgsExecuteSqlAlgorithm::displayName() const
{
  return QObject::tr( "Execute SQL" );
}

QStringList QgsExecuteSqlAlgorithm::tags() const
{
  return QObject::tr( "virtual,query,sql" ).split( ',' );
}

QString QgsExecuteSqlAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsExecuteSqlAlgorithm::groupId() const
{
  return u"vectorgeneral"_s;
}

QString QgsExecuteSqlAlgorithm::shortDescription() const
{
  return QObject::tr( "Runs an SQL query on vector layers using virtual layers." );
}

QString QgsExecuteSqlAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm executes an SQL query on input vector layers using QGIS virtual layers.\n\n"
    "Input layers are made available inside the query using aliases 'input1', 'input2', ..., 'inputN', corresponding to the order of layers supplied.\n\n"
    "The query engine uses SQLite and SpatiaLite syntax, allowing spatial functions like ST_Intersects, ST_Buffer, and attribute aggregation. "
    "Additionally, QGIS variable expressions in the format [% @var %] will be evaluated before running the query.\n\n"
    "The result of the query will be materialized and stored in a new layer."
  );
}

QgsExecuteSqlAlgorithm *QgsExecuteSqlAlgorithm::createInstance() const
{
  return new QgsExecuteSqlAlgorithm();
}

Qgis::ProcessingAlgorithmFlags QgsExecuteSqlAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading;
}

void QgsExecuteSqlAlgorithm::initAlgorithm( const QVariantMap & )
{
  auto inputDataSources = std::make_unique<
    QgsProcessingParameterMultipleLayers>( u"INPUT_DATASOURCES"_s, QObject::tr( "Input data sources (called input1, .., inputN in the query)" ), Qgis::ProcessingSourceType::Vector, QVariant(), true );
  inputDataSources->setHelp(
    QObject::tr( "Input vector layers to query. Inside the SQL statement, these layers are referenced as 'input1', 'input2', ..., 'inputN' according to their order in this list." )
  );
  addParameter( inputDataSources.release() );

  auto inputQuery = std::make_unique<QgsProcessingParameterString>( u"INPUT_QUERY"_s, QObject::tr( "SQL query" ) );
  inputQuery->setHelp(
    QObject::tr( "The SQL query to execute using SQLite/SpatiaLite syntax. Example: 'SELECT * FROM input1 WHERE area > 100'. Expressions enclosed in [% %] will be expanded before execution." )
  );
  inputQuery->setMetadata( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"executesql"_s } } ) } } );

  addParameter( inputQuery.release() );

  auto inputUidField = std::make_unique<QgsProcessingParameterString>( u"INPUT_UID_FIELD"_s, QObject::tr( "Unique identifier field" ), QVariant(), false, true );
  inputUidField->setHelp(
    QObject::tr( "Defines the field to be used as a unique integer identifier (primary key) for output features. If left empty, an autoincrementing ID field will be automatically generated." )
  );
  addParameter( inputUidField.release() );

  auto inputGeometryField = std::make_unique<QgsProcessingParameterString>( u"INPUT_GEOMETRY_FIELD"_s, QObject::tr( "Geometry field" ), QVariant(), false, true );
  inputGeometryField->setHelp( QObject::tr( "Specifies the name of the column in the query output that contains the feature geometries (e.g. 'geometry' or 'geom')." ) );
  addParameter( inputGeometryField.release() );

  QStringList geometryTypeOptions;
  geometryTypeOptions.reserve( static_cast<int>( mGeometryTypes.size() ) );
  for ( const std::pair<Qgis::WkbType, QString> &typePair : std::as_const( mGeometryTypes ) )
  {
    geometryTypeOptions.append( typePair.second );
  }

  auto inputGeometryType = std::make_unique<QgsProcessingParameterEnum>( u"INPUT_GEOMETRY_TYPE"_s, QObject::tr( "Geometry type" ), geometryTypeOptions, false, 0 );
  inputGeometryType->setHelp( QObject::tr( "Explicitly defines the geometry type of the query result. If set to 'Autodetect', the algorithm will attempt to infer the type from the result features." ) );
  addParameter( inputGeometryType.release() );

  auto inputGeometryCrs = std::make_unique<QgsProcessingParameterCrs>( u"INPUT_GEOMETRY_CRS"_s, QObject::tr( "CRS" ), QVariant(), true );
  inputGeometryCrs->setHelp( QObject::tr( "Specifies the coordinate reference system (CRS) for the output geometry. If left empty, the algorithm will attempt to infer the CRS from the input layers." ) );
  addParameter( inputGeometryCrs.release() );

  auto output = std::make_unique<QgsProcessingParameterFeatureSink>( u"OUTPUT"_s, QObject::tr( "SQL Output" ) );
  output->setHelp( QObject::tr( "Specifies the destination layer for the features returned by the SQL query." ) );
  addParameter( output.release() );
}

QVariantMap QgsExecuteSqlAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, u"INPUT_DATASOURCES"_s, context );
  const QString query = parameterAsString( parameters, u"INPUT_QUERY"_s, context );
  const QString uniqueIdentifierField = parameterAsString( parameters, u"INPUT_UID_FIELD"_s, context );
  const QString geometryField = parameterAsString( parameters, u"INPUT_GEOMETRY_FIELD"_s, context );

  const int geometryTypeIndex = parameterAsEnum( parameters, u"INPUT_GEOMETRY_TYPE"_s, context );
  const Qgis::WkbType geometryType = ( geometryTypeIndex >= 0 && geometryTypeIndex < static_cast<int>( mGeometryTypes.size() ) ) ? mGeometryTypes.at( geometryTypeIndex ).first : Qgis::WkbType::Unknown;

  const QgsCoordinateReferenceSystem geometryCrs = parameterAsCrs( parameters, u"INPUT_GEOMETRY_CRS"_s, context );

  QgsVirtualLayerDefinition layerDefinition;
  int layerIndex = 1;
  for ( QgsMapLayer *layer : std::as_const( layers ) )
  {
    QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( !vectorLayer || !vectorLayer->isValid() )
    {
      continue;
    }

    // Issue https://github.com/qgis/QGIS/issues/24041
    // When using this algorithm from the graphic modeler, it may try to
    // access (thanks the QgsVirtualLayerProvider) to memory layer that
    // belongs to temporary QgsMapLayerStore, not project.
    // So, we write them to disk if this is the case.
    if ( context.project() && !context.project()->mapLayer( vectorLayer->id() ) )
    {
      const QString basename = u"memorylayer."_s + QgsVectorFileWriter::supportedFormatExtensions().value( 0 );
      const QString temporaryPath = QgsProcessingUtils::generateTempFilename( basename, &context );

      QgsVectorFileWriter::SaveVectorOptions saveOptions;
      saveOptions.fileEncoding = vectorLayer->dataProvider()->encoding();
      QgsVectorFileWriter::writeAsVectorFormatV3( vectorLayer, temporaryPath, context.transformContext(), saveOptions );
      layerDefinition.addSource( u"input%1"_s.arg( layerIndex ), temporaryPath, u"ogr"_s );
    }
    else
    {
      layerDefinition.addSource( u"input%1"_s.arg( layerIndex ), vectorLayer->id() );
    }
    layerIndex++;
  }

  if ( query.trimmed().isEmpty() )
  {
    throw QgsProcessingException( QObject::tr( "Empty SQL. Please enter valid SQL expression and try again." ) );
  }

  QgsExpressionContext localContext = createExpressionContext( parameters, context );
  const QString expandedQuery = QgsExpression::replaceExpressionText( query, &localContext );

  feedback->pushInfo( QObject::tr( "Executing query:" ) );
  feedback->pushCommandInfo( expandedQuery );

  layerDefinition.setQuery( expandedQuery );

  if ( !uniqueIdentifierField.isEmpty() )
  {
    layerDefinition.setUid( uniqueIdentifierField );
  }

  if ( geometryType == Qgis::WkbType::NoGeometry )
  {
    layerDefinition.setGeometryWkbType( Qgis::WkbType::NoGeometry );
  }
  else
  {
    if ( !geometryField.isEmpty() )
    {
      layerDefinition.setGeometryField( geometryField );
    }
    if ( geometryType != Qgis::WkbType::Unknown )
    {
      layerDefinition.setGeometryWkbType( geometryType );
    }
    if ( geometryCrs.isValid() )
    {
      layerDefinition.setGeometrySrid( geometryCrs.postgisSrid() );
    }
  }

  QgsVectorLayer virtualLayer( layerDefinition.toString(), u"temp_vlayer"_s, u"virtual"_s );
  if ( !virtualLayer.isValid() )
  {
    throw QgsProcessingException( virtualLayer.dataProvider() ? virtualLayer.dataProvider()->error().summary() : QObject::tr( "Invalid virtual layer" ) );
  }

  if ( virtualLayer.wkbType() == Qgis::WkbType::Unknown )
  {
    throw QgsProcessingException( QObject::tr( "Cannot find geometry field" ) );
  }

  QString destinationId;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, destinationId, virtualLayer.fields(), virtualLayer.wkbType(), virtualLayer.crs() ) );
  if ( !sink )
  {
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );
  }

  QgsFeatureIterator featureIterator = virtualLayer.getFeatures();
  const double progressStep = virtualLayer.featureCount() > 0 ? 100.0 / virtualLayer.featureCount() : 0.0;
  long long currentFeatureIndex = 0;
  QgsFeature inputFeature;
  while ( featureIterator.nextFeature( inputFeature ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    sink->addFeature( inputFeature, QgsFeatureSink::Flag::FastInsert );
    feedback->featureAddedToSink( u"OUTPUT"_s );
    feedback->setProgress( static_cast<int>( currentFeatureIndex * progressStep ) );
    currentFeatureIndex++;
  }
  sink->finalize();
  feedback->featureSinkFinalized( u"OUTPUT"_s );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, destinationId );
  return outputs;
}

///@endcond
