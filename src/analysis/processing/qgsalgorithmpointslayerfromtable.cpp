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
  return QStringLiteral( "createpointslayerfromtable" );
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
  return QStringLiteral( "vectorcreation" );
}

QString QgsPointsLayerFromTableAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm generates a points layer based on the values from an input table." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "The table must contain a field with the X coordinate of each point and another "
                        "one with the Y coordinate, as well as optional fields with Z and M values. A CRS "
                        "for the output layer has to be specified, and the coordinates in the table are "
                        "assumed to be expressed in the units used by that CRS. The attributes table of "
                        "the resulting layer will be the input table." );
}

QgsPointsLayerFromTableAlgorithm *QgsPointsLayerFromTableAlgorithm::createInstance() const
{
  return new QgsPointsLayerFromTableAlgorithm();
}

void QgsPointsLayerFromTableAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ),
                QList< int >() << QgsProcessing::TypeVector ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "XFIELD" ), QObject::tr( "X field" ), QVariant(),
                QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "YFIELD" ), QObject::tr( "Y field" ), QVariant(),
                QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "ZFIELD" ), QObject::tr( "Z field" ), QVariant(),
                QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, false, true ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "MFIELD" ), QObject::tr( "M field" ), QVariant(),
                QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, false, true ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "TARGET_CRS" ), QObject::tr( "Target CRS" ), QStringLiteral( "EPSG:4326" ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Points from table" ), QgsProcessing::TypeVectorPoint ) );
}

QVariantMap QgsPointsLayerFromTableAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > featureSource( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !featureSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const QgsFields fields = featureSource->fields();
  const int xFieldIndex = fields.lookupField( parameterAsString( parameters, QStringLiteral( "XFIELD" ), context ) );
  const int yFieldIndex = fields.lookupField( parameterAsString( parameters, QStringLiteral( "YFIELD" ), context ) );

  QString fieldName = parameterAsString( parameters, QStringLiteral( "ZFIELD" ), context );
  int zFieldIndex = -1;
  if ( !fieldName.isEmpty() )
    zFieldIndex = fields.lookupField( fieldName );

  fieldName = parameterAsString( parameters, QStringLiteral( "MFIELD" ), context );
  int mFieldIndex = -1;
  if ( !fieldName.isEmpty() )
    mFieldIndex = fields.lookupField( fieldName );

  QgsWkbTypes::Type outputWkbType = QgsWkbTypes::Point;
  if ( zFieldIndex >= 0 )
    outputWkbType = QgsWkbTypes::addZ( outputWkbType );
  if ( mFieldIndex >= 0 )
    outputWkbType = QgsWkbTypes::addM( outputWkbType );

  const QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, QStringLiteral( "TARGET_CRS" ), context );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, outputWkbType, crs, QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  const double step = featureSource->featureCount() > 0 ? 100.0 / featureSource->featureCount() : 1;

  QgsFeatureRequest req;
  req.setFlags( QgsFeatureRequest::NoGeometry );
  QgsFeatureIterator fi = featureSource->getFeatures( req, QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );
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

    if ( ! QgsVariantUtils::isNull( attrs.at( xFieldIndex ) ) && ! QgsVariantUtils::isNull( attrs.at( yFieldIndex ) ) && xOk && yOk )
    {
      QgsPoint point( x, y );

      if ( zFieldIndex >= 0 && ! QgsVariantUtils::isNull( attrs.at( zFieldIndex ) ) )
        point.addZValue( attrs.at( zFieldIndex ).toDouble() );

      if ( mFieldIndex >= 0 && ! QgsVariantUtils::isNull( attrs.at( mFieldIndex ) ) )
        point.addMValue( attrs.at( mFieldIndex ).toDouble() );

      f.setGeometry( QgsGeometry( point.clone() ) );
    }

    if ( !sink->addFeature( f ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
