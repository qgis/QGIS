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

#include "qgslinestring.h"
#include "qgsmultilinestring.h"

///@cond PRIVATE

QString QgsTransectAlgorithmBase::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsTransectAlgorithmBase::groupId() const
{
  return u"vectorgeometry"_s;
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
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) ) );

  addAlgorithmParams();

  auto length = std::make_unique<QgsProcessingParameterDistance>( u"LENGTH"_s, QObject::tr( "Length of the transect" ), 5.0, u"INPUT"_s, false, 0 );
  length->setIsDynamic( true );
  length->setDynamicPropertyDefinition( QgsPropertyDefinition( u"LENGTH"_s, QObject::tr( "Length of the transect" ), QgsPropertyDefinition::DoublePositive ) );
  length->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( length.release() );

  auto angle = std::make_unique<QgsProcessingParameterNumber>( u"ANGLE"_s, QObject::tr( "Angle in degrees from the original line at the vertices" ), Qgis::ProcessingNumberParameterType::Double, 90.0, false, 0, 360 );
  angle->setIsDynamic( true );
  angle->setDynamicPropertyDefinition( QgsPropertyDefinition( u"ANGLE"_s, QObject::tr( "Angle in degrees" ), QgsPropertyDefinition::Double ) );
  angle->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( angle.release() );

  addParameter( new QgsProcessingParameterEnum( u"SIDE"_s, QObject::tr( "Side to create the transects" ), QStringList() << QObject::tr( "Left" ) << QObject::tr( "Right" ) << QObject::tr( "Both" ), false, 2 ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Transect" ), Qgis::ProcessingSourceType::VectorLine ) );
}

QVariantMap QgsTransectAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mOrientation = static_cast<QgsTransectAlgorithmBase::Side>( parameterAsInt( parameters, u"SIDE"_s, context ) );
  mAngle = fabs( parameterAsDouble( parameters, u"ANGLE"_s, context ) );
  mDynamicAngle = QgsProcessingParameters::isDynamic( parameters, u"ANGLE"_s );
  if ( mDynamicAngle )
    mAngleProperty = parameters.value( u"ANGLE"_s ).value<QgsProperty>();

  mLength = parameterAsDouble( parameters, u"LENGTH"_s, context );
  mDynamicLength = QgsProcessingParameters::isDynamic( parameters, u"LENGTH"_s );
  if ( mDynamicLength )
    mLengthProperty = parameters.value( u"LENGTH"_s ).value<QgsProperty>();

  if ( mOrientation == QgsTransectAlgorithmBase::Both )
    mLength /= 2.0;

  // Let subclass prepare their specific parameters
  if ( !prepareAlgorithmTransectParameters( parameters, context, feedback ) )
    return QVariantMap();

  std::unique_ptr<QgsFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, dynamic_cast<QgsProcessingFeatureSource *>( source.get() ) );

  QgsFields newFields;
  newFields.append( QgsField( u"TR_FID"_s, QMetaType::Type::Int, QString(), 20 ) );
  newFields.append( QgsField( u"TR_ID"_s, QMetaType::Type::Int, QString(), 20 ) );
  newFields.append( QgsField( u"TR_SEGMENT"_s, QMetaType::Type::Int, QString(), 20 ) );
  newFields.append( QgsField( u"TR_ANGLE"_s, QMetaType::Type::Double, QString(), 5, 2 ) );
  newFields.append( QgsField( u"TR_LENGTH"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  newFields.append( QgsField( u"TR_ORIENT"_s, QMetaType::Type::Int, QString(), 1 ) );
  QgsFields fields = QgsProcessingUtils::combineFields( source->fields(), newFields );

  Qgis::WkbType outputWkb = Qgis::WkbType::LineString;
  if ( QgsWkbTypes::hasZ( source->wkbType() ) )
    outputWkb = QgsWkbTypes::addZ( outputWkb );
  if ( QgsWkbTypes::hasM( source->wkbType() ) )
    outputWkb = QgsWkbTypes::addM( outputWkb );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, outputWkb, source->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

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

    // Segmentize curved geometries to convert them to straight line segments
    if ( QgsWkbTypes::isCurvedType( inputGeometry.wkbType() ) )
    {
      inputGeometry = QgsGeometry( inputGeometry.constGet()->segmentize() );
    }

    inputGeometry.convertToMultiType();
    const QgsMultiLineString *multiLine = qgsgeometry_cast<const QgsMultiLineString *>( inputGeometry.constGet() );

    for ( int part = 0; part < multiLine->numGeometries(); ++part )
    {
      const QgsLineString *lineString = multiLine->lineStringN( part );
      if ( !lineString )
        continue;

      // Let subclass generate sampling points using their specific strategy
      std::vector<QgsPoint> samplingPoints = generateSamplingPoints( *lineString, parameters, context );

      for ( int i = 0; i < static_cast<int>( samplingPoints.size() ); ++i )
      {
        const QgsPoint &pt = samplingPoints[i];

        // Let subclass calculate azimuth using their specific method
        double azimuth = calculateAzimuth( *lineString, pt, i );

        QgsFeature outFeat;
        QgsAttributes attrs = feat.attributes();
        attrs << current << number << i + 1 << evaluatedAngle
              << ( ( mOrientation == QgsTransectAlgorithmBase::Both ) ? evaluatedLength * 2 : evaluatedLength )
              << static_cast<int>( mOrientation );
        outFeat.setAttributes( attrs );
        outFeat.setGeometry( calcTransect( pt, azimuth, evaluatedLength, mOrientation, evaluatedAngle ) );
        if ( !sink->addFeature( outFeat, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
        number++;
      }
    }
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

QgsGeometry QgsTransectAlgorithmBase::calcTransect( const QgsPoint &point, const double angleAtVertex, const double length, const QgsTransectAlgorithmBase::Side orientation, const double angle )
{
  QgsPoint pLeft;  // left point of the line
  QgsPoint pRight; // right point of the line

  QgsPolyline line;

  switch ( orientation )
  {
    case QgsTransectAlgorithmBase::Right:
      pLeft = point.project( length, angle + 180.0 / M_PI * angleAtVertex );
      pRight = point;
      break;

    case QgsTransectAlgorithmBase::Left:
      pRight = point.project( -length, angle + 180.0 / M_PI * angleAtVertex );
      pLeft = point;
      break;

    case QgsTransectAlgorithmBase::Both:
      pLeft = point.project( length, angle + 180.0 / M_PI * angleAtVertex );
      pRight = point.project( -length, angle + 180.0 / M_PI * angleAtVertex );
      break;
  }

  line.append( pLeft );
  line.append( pRight );

  return QgsGeometry::fromPolyline( line );
}

///@endcond
