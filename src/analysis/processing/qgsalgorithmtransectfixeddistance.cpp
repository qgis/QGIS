/***************************************************************************
                         qgsalgorithmtransectfixeddistance.cpp
                         -------------------------------------
    begin                : September 2024
    copyright            : (C) 2024 by Loïc Bartoletti
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmtransectfixeddistance.h"
#include "qgsmultilinestring.h"
#include "qgslinestring.h"

///@cond PRIVATE

QString QgsTransectFixedDistanceAlgorithm::name() const
{
  return QStringLiteral( "transectfixeddistance" );
}

QString QgsTransectFixedDistanceAlgorithm::displayName() const
{
  return QObject::tr( "Transect (fixed distance)" );
}

QStringList QgsTransectFixedDistanceAlgorithm::tags() const
{
  return QObject::tr( "transect,station,lines,extend,fixed,interval,distance" ).split( ',' );
}

QString QgsTransectFixedDistanceAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsTransectFixedDistanceAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

void QgsTransectFixedDistanceAlgorithm::initAlgorithm( const QVariantMap & )
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

  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "INTERVAL" ), QObject::tr( "Fixed sampling interval" ), Qgis::ProcessingNumberParameterType::Double, 10.0, false, 0 ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Transect" ), Qgis::ProcessingSourceType::VectorLine ) );
}

QString QgsTransectFixedDistanceAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates transects at fixed distance intervals along (multi)linestrings.\n" )
         + QObject::tr( "A transect is a line oriented from an angle (by default perpendicular) to the input polylines at regular intervals." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Field(s) from feature(s) are returned in the transect with these new fields:\n" )
         + QObject::tr( "- TR_FID: ID of the original feature\n" )
         + QObject::tr( "- TR_ID: ID of the transect. Each transect have an unique ID\n" )
         + QObject::tr( "- TR_SEGMENT: ID of the segment of the linestring\n" )
         + QObject::tr( "- TR_ANGLE: Angle in degrees from the original line at the vertex\n" )
         + QObject::tr( "- TR_LENGTH: Total length of the transect returned\n" )
         + QObject::tr( "- TR_ORIENT: Side of the transect (only on the left or right of the line, or both side)\n" );
}

QString QgsTransectFixedDistanceAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates transects at fixed distance intervals along (multi)linestrings." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsTransectFixedDistanceAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsTransectFixedDistanceAlgorithm *QgsTransectFixedDistanceAlgorithm::createInstance() const
{
  return new QgsTransectFixedDistanceAlgorithm();
}

QVariantMap QgsTransectFixedDistanceAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const Side orientation = static_cast<QgsTransectFixedDistanceAlgorithm::Side>( parameterAsInt( parameters, QStringLiteral( "SIDE" ), context ) );
  const double angle = fabs( parameterAsDouble( parameters, QStringLiteral( "ANGLE" ), context ) );
  const bool dynamicAngle = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "ANGLE" ) );
  QgsProperty angleProperty;
  if ( dynamicAngle )
    angleProperty = parameters.value( QStringLiteral( "ANGLE" ) ).value<QgsProperty>();

  double length = parameterAsDouble( parameters, QStringLiteral( "LENGTH" ), context );
  const bool dynamicLength = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "LENGTH" ) );
  QgsProperty lengthProperty;
  if ( dynamicLength )
    lengthProperty = parameters.value( QStringLiteral( "LENGTH" ) ).value<QgsProperty>();

  const double interval = parameterAsDouble( parameters, QStringLiteral( "INTERVAL" ), context );

  if ( orientation == QgsTransectFixedDistanceAlgorithm::Both )
    length /= 2.0;

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

    if ( dynamicLength || dynamicAngle )
    {
      expressionContext.setFeature( feat );
    }

    double evaluatedLength = length;
    if ( dynamicLength )
      evaluatedLength = lengthProperty.valueAsDouble( context.expressionContext(), length );
    double evaluatedAngle = angle;
    if ( dynamicAngle )
      evaluatedAngle = angleProperty.valueAsDouble( context.expressionContext(), angle );

    inputGeometry.convertToMultiType();
    const QgsMultiLineString *multiLine = static_cast<const QgsMultiLineString *>( inputGeometry.constGet() );

    for ( int part = 0; part < multiLine->numGeometries(); ++part )
    {
      const QgsLineString *lineString = multiLine->lineStringN( part );
      if ( !lineString )
        continue;

      QgsLineString line = *lineString;
      std::vector<QgsPoint> samplingPoints;

      // Sample points at fixed intervals
      double totalLength = line.length();
      for ( double d = 0; d <= totalLength; d += interval )
      {
        QgsPoint *pt = line.interpolatePoint( d );
        samplingPoints.push_back( *pt );
      }

      for ( int i = 0; i < static_cast<int>( samplingPoints.size() ); ++i )
      {
        const QgsPoint &pt = samplingPoints[i];
        double azimuth = 0;

        QgsPoint segPt;
        QgsVertexId vid;
        line.closestSegment( pt, segPt, vid, nullptr, Qgis::DEFAULT_SEGMENT_EPSILON );
        QgsVertexId prev( vid.part, vid.ring, vid.vertex - 1 );
        azimuth = line.vertexAt( prev ).azimuth( line.vertexAt( vid ) ) * M_PI / 180.0;

        QgsFeature outFeat;
        QgsAttributes attrs = feat.attributes();
        attrs << current << number << i + 1 << evaluatedAngle
              << ( ( orientation == QgsTransectFixedDistanceAlgorithm::Both ) ? evaluatedLength * 2 : evaluatedLength )
              << static_cast<int>( orientation );
        outFeat.setAttributes( attrs );
        outFeat.setGeometry( calcTransect( pt, azimuth, evaluatedLength, orientation, evaluatedAngle ) );
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

QgsGeometry QgsTransectFixedDistanceAlgorithm::calcTransect( const QgsPoint &point, const double angleAtVertex, const double length, const QgsTransectFixedDistanceAlgorithm::Side orientation, const double angle )
{
  QgsPoint pLeft;  // left point of the line
  QgsPoint pRight; // right point of the line

  QgsPolyline line;

  if ( ( orientation == QgsTransectFixedDistanceAlgorithm::Right ) || ( orientation == QgsTransectFixedDistanceAlgorithm::Both ) )
  {
    pLeft = point.project( length, angle + 180.0 / M_PI * angleAtVertex );
    if ( orientation != QgsTransectFixedDistanceAlgorithm::Both )
      pRight = point;
  }

  if ( ( orientation == QgsTransectFixedDistanceAlgorithm::Left ) || ( orientation == QgsTransectFixedDistanceAlgorithm::Both ) )
  {
    pRight = point.project( -length, angle + 180.0 / M_PI * angleAtVertex );
    if ( orientation != QgsTransectFixedDistanceAlgorithm::Both )
      pLeft = point;
  }

  line.append( pLeft );
  line.append( pRight );

  return QgsGeometry::fromPolyline( line );
}

///@endcond