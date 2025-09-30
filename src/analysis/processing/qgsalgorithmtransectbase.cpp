/***************************************************************************
                         qgsalgorithmtransectbase.cpp
                         ----------------------------
    begin                : September 2025
    copyright            : (C) 2025 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmtransectbase.h"
#include "qgsmultilinestring.h"
#include "qgslinestring.h"

///@cond PRIVATE

QString QgsTransectAlgorithmBase::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsTransectAlgorithmBase::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QStringList QgsTransectAlgorithmBase::tags() const
{
  return QObject::tr( "transect,station,lines,extend" ).split( ',' );
}

QString QgsTransectAlgorithmBase::shortDescription() const
{
  return QObject::tr( "Creates transects for (multi)linestrings." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsTransectAlgorithmBase::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

void QgsTransectAlgorithmBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) ) );

  auto length = std::make_unique<QgsProcessingParameterDistance>( QStringLiteral( "LENGTH" ), QObject::tr( "Length of the transect" ), 5.0, QStringLiteral( "INPUT" ), false, 0 );
  length->setIsDynamic( true );
  length->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "LENGTH" ), QObject::tr( "Length of the transect" ), QgsPropertyDefinition::DoublePositive ) );
  length->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( length.release() );

  auto angle = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "ANGLE" ), QObject::tr( "Angle in degrees from the original line at the vertices" ), Qgis::ProcessingNumberParameterType::Double, 90.0, false, 0, 360 );
  angle->setIsDynamic( true );
  angle->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "ANGLE" ), QObject::tr( "Angle in degrees" ), QgsPropertyDefinition::Double ) );
  angle->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( angle.release() );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "SIDE" ), QObject::tr( "Side to create the transects" ), QStringList() << QObject::tr( "Left" ) << QObject::tr( "Right" ) << QObject::tr( "Both" ), false ) );

  // Allow subclasses to add their specific parameters
  addAlgorithmParams();

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Transect" ), Qgis::ProcessingSourceType::VectorLine ) );
}

QVariantMap QgsTransectAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mOrientation = static_cast<QgsTransectAlgorithmBase::Side>( parameterAsInt( parameters, QStringLiteral( "SIDE" ), context ) );
  mAngle = fabs( parameterAsDouble( parameters, QStringLiteral( "ANGLE" ), context ) );
  mDynamicAngle = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "ANGLE" ) );
  if ( mDynamicAngle )
    mAngleProperty = parameters.value( QStringLiteral( "ANGLE" ) ).value<QgsProperty>();

  mLength = parameterAsDouble( parameters, QStringLiteral( "LENGTH" ), context );
  mDynamicLength = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "LENGTH" ) );
  if ( mDynamicLength )
    mLengthProperty = parameters.value( QStringLiteral( "LENGTH" ) ).value<QgsProperty>();

  if ( mOrientation == QgsTransectAlgorithmBase::Both )
    mLength /= 2.0;

  // Let subclass prepare their specific parameters
  if ( !prepareAlgorithmTransectParameters( parameters, context, feedback ) )
    return QVariantMap();

  std::unique_ptr<QgsFeatureSource> source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, dynamic_cast<QgsProcessingFeatureSource *>( source.get() ) );

  QgsFields newFields;
  newFields.append( QgsField( QStringLiteral( "TR_FID" ), QMetaType::Type::Int, QString(), 20 ) );
  newFields.append( QgsField( QStringLiteral( "TR_ID" ), QMetaType::Type::Int, QString(), 20 ) );
  newFields.append( QgsField( QStringLiteral( "TR_SEGMENT" ), QMetaType::Type::Int, QString(), 20 ) );
  newFields.append( QgsField( QStringLiteral( "TR_ANGLE" ), QMetaType::Type::Double, QString(), 5, 2 ) );
  newFields.append( QgsField( QStringLiteral( "TR_LENGTH" ), QMetaType::Type::Double, QString(), 20, 6 ) );
  newFields.append( QgsField( QStringLiteral( "TR_ORIENT" ), QMetaType::Type::Int, QString(), 1 ) );
  QgsFields fields = QgsProcessingUtils::combineFields( source->fields(), newFields );

  Qgis::WkbType outputWkb = Qgis::WkbType::LineString;
  if ( QgsWkbTypes::hasZ( source->wkbType() ) )
    outputWkb = QgsWkbTypes::addZ( outputWkb );
  if ( QgsWkbTypes::hasM( source->wkbType() ) )
    outputWkb = QgsWkbTypes::addM( outputWkb );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, outputWkb, source->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QgsFeatureIterator features = source->getFeatures();

  int current = -1;
  int number = 0;
  const double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 1;
  QgsFeature feat;

  while ( features.nextFeature( feat ) )
  {
    current++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( current * step );
    if ( !feat.hasGeometry() )
      continue;

    QgsGeometry inputGeometry = feat.geometry();

    if ( mDynamicLength || mDynamicAngle )
    {
      expressionContext.setFeature( feat );
    }

    double evaluatedLength = mLength;
    if ( mDynamicLength )
      evaluatedLength = mLengthProperty.valueAsDouble( context.expressionContext(), mLength );
    double evaluatedAngle = mAngle;
    if ( mDynamicAngle )
      evaluatedAngle = mAngleProperty.valueAsDouble( context.expressionContext(), mAngle );

    inputGeometry.convertToMultiType();
    const QgsMultiLineString *multiLine = static_cast<const QgsMultiLineString *>( inputGeometry.constGet() );

    for ( int part = 0; part < multiLine->numGeometries(); ++part )
    {
      const QgsLineString *lineString = multiLine->lineStringN( part );
      if ( !lineString )
        continue;

      QgsLineString line = *lineString;

      // Let subclass generate sampling points using their specific strategy
      std::vector<QgsPoint> samplingPoints = generateSamplingPoints( line, parameters, context );

      for ( int i = 0; i < static_cast<int>( samplingPoints.size() ); ++i )
      {
        const QgsPoint &pt = samplingPoints[i];

        // Let subclass calculate azimuth using their specific method
        double azimuth = calculateAzimuth( line, pt, i );

        QgsFeature outFeat;
        QgsAttributes attrs = feat.attributes();
        attrs << current << number << i + 1 << evaluatedAngle
              << ( ( mOrientation == QgsTransectAlgorithmBase::Both ) ? evaluatedLength * 2 : evaluatedLength )
              << static_cast<int>( mOrientation );
        outFeat.setAttributes( attrs );
        outFeat.setGeometry( calcTransect( pt, azimuth, evaluatedLength, mOrientation, evaluatedAngle ) );
        if ( !sink->addFeature( outFeat, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
        number++;
      }
    }
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

QgsGeometry QgsTransectAlgorithmBase::calcTransect( const QgsPoint &point, const double angleAtVertex, const double length, const QgsTransectAlgorithmBase::Side orientation, const double angle )
{
  QgsPoint pLeft;  // left point of the line
  QgsPoint pRight; // right point of the line

  QgsPolyline line;

  if ( ( orientation == QgsTransectAlgorithmBase::Right ) || ( orientation == QgsTransectAlgorithmBase::Both ) )
  {
    pLeft = point.project( length, angle + 180.0 / M_PI * angleAtVertex );
    if ( orientation != QgsTransectAlgorithmBase::Both )
      pRight = point;
  }

  if ( ( orientation == QgsTransectAlgorithmBase::Left ) || ( orientation == QgsTransectAlgorithmBase::Both ) )
  {
    pRight = point.project( -length, angle + 180.0 / M_PI * angleAtVertex );
    if ( orientation != QgsTransectAlgorithmBase::Both )
      pLeft = point;
  }

  line.append( pLeft );
  line.append( pRight );

  return QgsGeometry::fromPolyline( line );
}

///@endcond
