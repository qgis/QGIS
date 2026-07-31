/***************************************************************************
                         qgsalgorithmexecutesql.h
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
  return QObject::tr( "Runs a query with SQL syntax." );
}

QString QgsExecuteSqlAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm runs a query with SQL syntax.\n]n"
    "Input data sources are identified with 'input1', 'input2', ..., 'inputN'' "
    "and a simple query will look like: 'SELECT * FROM input1'.\n]n"
    "The result of the query will be added as a new layer."
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
  addParameter(
    new QgsProcessingParameterMultipleLayers( u"INPUT_DATASOURCES"_s, QObject::tr( "Input data sources (called input1, .., inputN in the query)" ), Qgis::ProcessingSourceType::Vector, QVariant(), true )
  );

  addParameter( new QgsProcessingParameterString( u"INPUT_QUERY"_s, QObject::tr( "SQL query" ) ) );

  addParameter( new QgsProcessingParameterString( u"INPUT_UID_FIELD"_s, QObject::tr( "Unique identifier field" ), QVariant(), false, true ) );

  addParameter( new QgsProcessingParameterString( u"INPUT_GEOMETRY_FIELD"_s, QObject::tr( "Geometry field" ), QVariant(), false, true ) );

  QStringList geometryTypeOptions;
  geometryTypeOptions.reserve( static_cast<int>( mGeometryTypes.size() ) );
  for ( const std::pair<Qgis::WkbType, QString> &typePair : std::as_const( mGeometryTypes ) )
  {
    geometryTypeOptions.append( typePair.second );
  }

  addParameter( new QgsProcessingParameterEnum( u"INPUT_GEOMETRY_TYPE"_s, QObject::tr( "Geometry type" ), geometryTypeOptions, false, 0 ) );

  addParameter( new QgsProcessingParameterCrs( u"INPUT_GEOMETRY_CRS"_s, QObject::tr( "CRS" ), QVariant(), true ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "SQL Output" ) ) );
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
    // So, we write them to disk is this is the case.
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
    throw QgsProcessingException( virtualLayer.dataProvider() ? virtualLayer.dataProvider()->error().message() : QObject::tr( "Invalid virtual layer" ) );
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
