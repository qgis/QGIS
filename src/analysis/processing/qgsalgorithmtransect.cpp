/***************************************************************************
                         qgsalgorithmtransect.cpp
                         -------------------------
    begin                : October 2017
    copyright            : (C) 2017 by Loïc Bartoletti
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

#include "qgsalgorithmtransect.h"
#include "qgsmultilinestring.h"
#include "qgslinestring.h"

///@cond PRIVATE

QString QgsTransectAlgorithm::name() const
{
  return QStringLiteral( "transect" );
}

QString QgsTransectAlgorithm::displayName() const
{
  return QObject::tr( "Transect" );
}

QStringList QgsTransectAlgorithm::tags() const
{
  return QObject::tr( "transect,station,lines,extend," ).split( ',' );
}

QString QgsTransectAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsTransectAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QgsProcessingAlgorithm::Flags QgsTransectAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsTransectAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorLine ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "LENGTH" ), QObject::tr( "Length of the transect " ), QgsProcessingParameterNumber::Double,
                5.0, false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "ANGLE" ), QObject::tr( "Angle in degrees from the original line at the vertices" ), QgsProcessingParameterNumber::Double,
                90.0, false, 0, 360 ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "SIDE" ), QObject::tr( "Side to create the transects" ), QStringList() << QObject::tr( "Left" ) << QObject::tr( "Right" ) << QObject::tr( "Both" ), false ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Transect" ), QgsProcessing::TypeVectorLine ) );

}

QString QgsTransectAlgorithm::shortHelpString() const
{

  return QObject::tr( "This algorithm creates transects on vertices for (multi)linestring.\n" ) +
         QObject::tr( "A transect is a line oriented from an angle (by default perpendicular) to the input polylines (at vertices)." ) +
         QStringLiteral( "\n\n" )  +
         QObject::tr( "Field(s) from feature(s) are returned in the transect with these new fields:\n" ) +
         QObject::tr( "- TR_FID: ID of the original feature\n" ) +
         QObject::tr( "- TR_ID: ID of the transect. Each transect have an unique ID\n" ) +
         QObject::tr( "- TR_SEGMENT: ID of the segment of the linestring\n" ) +
         QObject::tr( "- TR_ANGLE: Angle in degrees from the original line at the vertex\n" ) +
         QObject::tr( "- TR_LENGTH: Total length of the transect returned\n" ) +
         QObject::tr( "- TR_ORIENT: Side of the transect (only on the left or right of the line, or both side)\n" );

}

QgsTransectAlgorithm *QgsTransectAlgorithm::createInstance() const
{
  return new QgsTransectAlgorithm();
}

QVariantMap QgsTransectAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Side orientation = static_cast< QgsTransectAlgorithm::Side >( parameterAsInt( parameters, QStringLiteral( "SIDE" ), context ) );
  double angle = fabs( parameterAsDouble( parameters, QStringLiteral( "ANGLE" ), context ) );
  double length = parameterAsDouble( parameters, QStringLiteral( "LENGTH" ), context );

  if ( orientation == QgsTransectAlgorithm::Both )
    length /= 2.0;

  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QgsFields fields = source->fields();

  fields.append( QgsField( QStringLiteral( "TR_FID" ), QVariant::Int, QString(), 20 ) );
  fields.append( QgsField( QStringLiteral( "TR_ID" ), QVariant::Int, QString(), 20 ) );
  fields.append( QgsField( QStringLiteral( "TR_SEGMENT" ), QVariant::Int, QString(), 20 ) );
  fields.append( QgsField( QStringLiteral( "TR_ANGLE" ), QVariant::Double, QString(), 5, 2 ) );
  fields.append( QgsField( QStringLiteral( "TR_LENGTH" ), QVariant::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "TR_ORIENT" ), QVariant::Int, QString(), 1 ) );

  QgsWkbTypes::Type outputWkb = QgsWkbTypes::LineString;
  if ( QgsWkbTypes::hasZ( source->wkbType() ) )
    outputWkb = QgsWkbTypes::addZ( outputWkb );
  if ( QgsWkbTypes::hasM( source->wkbType() ) )
    outputWkb = QgsWkbTypes::addM( outputWkb );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields,
                                          outputWkb, source->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();

  QgsFeatureIterator features = source->getFeatures( );

  int current = -1;
  int number = 0;
  double step =  source->featureCount() > 0 ? 100.0 / source->featureCount() : 1;
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

    inputGeometry.convertToMultiType();
    const QgsMultiLineString *multiLine = static_cast< const QgsMultiLineString *  >( inputGeometry.constGet() );
    for ( int id = 0; id < multiLine->numGeometries(); ++id )
    {
      const QgsLineString *line = static_cast< const QgsLineString * >( multiLine->geometryN( id ) );
      QgsAbstractGeometry::vertex_iterator it = line->vertices_begin();
      while ( it != line->vertices_end() )
      {
        QgsVertexId vertexId = it.vertexId();
        int i = vertexId.vertex;
        QgsFeature outFeat;
        QgsAttributes attrs = feat.attributes();
        attrs << current << number << i + 1 << angle <<
              ( ( orientation == QgsTransectAlgorithm::Both ) ? length * 2 : length ) <<
              orientation;
        outFeat.setAttributes( attrs );
        double angleAtVertex = line->vertexAngle( vertexId );
        outFeat.setGeometry( calcTransect( *it, angleAtVertex, length, orientation, angle ) );
        sink->addFeature( outFeat, QgsFeatureSink::FastInsert );
        number++;
        it++;
      }
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


QgsGeometry QgsTransectAlgorithm::calcTransect( const QgsPoint &point, const double angleAtVertex, const double length, const QgsTransectAlgorithm::Side orientation, const double angle )
{
  QgsPoint pLeft; // left point of the line
  QgsPoint pRight; // right point of the line

  QgsPolyline line;

  if ( ( orientation == QgsTransectAlgorithm::Right ) || ( orientation == QgsTransectAlgorithm::Both ) )
  {
    pLeft = point.project( length, angle + 180.0 / M_PI * angleAtVertex );
    if ( orientation != QgsTransectAlgorithm::Both )
      pRight = point;
  }

  if ( ( orientation == QgsTransectAlgorithm::Left ) || ( orientation == QgsTransectAlgorithm::Both ) )
  {
    pRight = point.project( -length, angle + 180.0 / M_PI * angleAtVertex );
    if ( orientation != QgsTransectAlgorithm::Both )
      pLeft = point;
  }

  line.append( pLeft );
  line.append( pRight );

  return QgsGeometry::fromPolyline( line );
}

///@endcond
