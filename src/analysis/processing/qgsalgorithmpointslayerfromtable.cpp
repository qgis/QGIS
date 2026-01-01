/***************************************************************************
                         qgsalgorithmpointslayerfromtable.cpp
                         ---------------------
    begin                : November 2019
    copyright            : (C) 2019 by Alexander Bruy
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

#include "qgsalgorithmpointslayerfromtable.h"

#include "qgsvariantutils.h"

///@cond PRIVATE

QString QgsPointsLayerFromTableAlgorithm::name() const
{
  return u"createpointslayerfromtable"_s;
}

QString QgsPointsLayerFromTableAlgorithm::displayName() const
{
  return QObject::tr( "Create points layer from table" );
}

QStringList QgsPointsLayerFromTableAlgorithm::tags() const
{
  return QObject::tr( "points,create,values,attributes" ).split( ',' );
}

QString QgsPointsLayerFromTableAlgorithm::group() const
{
  return QObject::tr( "Vector creation" );
}

QString QgsPointsLayerFromTableAlgorithm::groupId() const
{
  return u"vectorcreation"_s;
}

QString QgsPointsLayerFromTableAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm generates a point layer based on the coordinates from an input table." )
         + u"\n\n"_s
         + QObject::tr( "The table must contain a field with the X coordinate of each point and another "
                        "one with the Y coordinate, as well as optional fields with Z and M values. A CRS "
                        "for the output layer has to be specified, and the coordinates in the table are "
                        "assumed to be expressed in the units used by that CRS. The attributes table of "
                        "the resulting layer will be the input table." );
}

QString QgsPointsLayerFromTableAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates a point layer based on the coordinates from an input table." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsPointsLayerFromTableAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsPointsLayerFromTableAlgorithm *QgsPointsLayerFromTableAlgorithm::createInstance() const
{
  return new QgsPointsLayerFromTableAlgorithm();
}

void QgsPointsLayerFromTableAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterField( u"XFIELD"_s, QObject::tr( "X field" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any ) );
  addParameter( new QgsProcessingParameterField( u"YFIELD"_s, QObject::tr( "Y field" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any ) );
  addParameter( new QgsProcessingParameterField( u"ZFIELD"_s, QObject::tr( "Z field" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, false, true ) );
  addParameter( new QgsProcessingParameterField( u"MFIELD"_s, QObject::tr( "M field" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, false, true ) );
  addParameter( new QgsProcessingParameterCrs( u"TARGET_CRS"_s, QObject::tr( "Target CRS" ), u"EPSG:4326"_s ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Points from table" ), Qgis::ProcessingSourceType::VectorPoint ) );
}

QVariantMap QgsPointsLayerFromTableAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> featureSource( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !featureSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const QgsFields fields = featureSource->fields();
  const QString xFieldName = parameterAsString( parameters, u"XFIELD"_s, context );
  const int xFieldIndex = fields.lookupField( xFieldName );
  if ( xFieldIndex < 0 )
    throw QgsProcessingException( QObject::tr( "X field “%1” does not exist" ).arg( xFieldName ) );

  const QString yFieldName = parameterAsString( parameters, u"YFIELD"_s, context );
  const int yFieldIndex = fields.lookupField( yFieldName );
  if ( yFieldIndex < 0 )
    throw QgsProcessingException( QObject::tr( "Y field “%1” does not exist" ).arg( yFieldName ) );

  QString fieldName = parameterAsString( parameters, u"ZFIELD"_s, context );
  int zFieldIndex = -1;
  if ( !fieldName.isEmpty() )
  {
    zFieldIndex = fields.lookupField( fieldName );
    if ( zFieldIndex < 0 )
      throw QgsProcessingException( QObject::tr( "Z field “%1” does not exist" ).arg( fieldName ) );
  }

  fieldName = parameterAsString( parameters, u"MFIELD"_s, context );
  int mFieldIndex = -1;
  if ( !fieldName.isEmpty() )
  {
    mFieldIndex = fields.lookupField( fieldName );
    if ( mFieldIndex < 0 )
      throw QgsProcessingException( QObject::tr( "M field “%1” does not exist" ).arg( fieldName ) );
  }

  Qgis::WkbType outputWkbType = Qgis::WkbType::Point;
  if ( zFieldIndex >= 0 )
    outputWkbType = QgsWkbTypes::addZ( outputWkbType );
  if ( mFieldIndex >= 0 )
    outputWkbType = QgsWkbTypes::addM( outputWkbType );

  const QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, u"TARGET_CRS"_s, context );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, outputWkbType, crs, QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  const double step = featureSource->featureCount() > 0 ? 100.0 / featureSource->featureCount() : 1;

  QgsFeatureRequest req;
  req.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  QgsFeatureIterator fi = featureSource->getFeatures( req, Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  QgsFeature f;
  int current = 0;

  while ( fi.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    const QgsAttributes attrs = f.attributes();

    bool xOk = false;
    bool yOk = false;
    const double x = attrs.at( xFieldIndex ).toDouble( &xOk );
    const double y = attrs.at( yFieldIndex ).toDouble( &yOk );

    if ( !QgsVariantUtils::isNull( attrs.at( xFieldIndex ) ) && !QgsVariantUtils::isNull( attrs.at( yFieldIndex ) ) && xOk && yOk )
    {
      QgsPoint point( x, y );

      if ( zFieldIndex >= 0 && !QgsVariantUtils::isNull( attrs.at( zFieldIndex ) ) )
        point.addZValue( attrs.at( zFieldIndex ).toDouble() );

      if ( mFieldIndex >= 0 && !QgsVariantUtils::isNull( attrs.at( mFieldIndex ) ) )
        point.addMValue( attrs.at( mFieldIndex ).toDouble() );

      f.setGeometry( QgsGeometry( point.clone() ) );
    }

    if ( !sink->addFeature( f ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    feedback->setProgress( current * step );
    current++;
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

///@endcond
