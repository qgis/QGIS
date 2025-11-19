/***************************************************************************
                         qgsalgorithmtransectfixeddistance.cpp
                         -------------------------------------
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

QgsTransectFixedDistanceAlgorithm *QgsTransectFixedDistanceAlgorithm::createInstance() const
{
  return new QgsTransectFixedDistanceAlgorithm();
}

void QgsTransectFixedDistanceAlgorithm::addAlgorithmParams()
{
  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "INTERVAL" ), QObject::tr( "Fixed sampling interval" ), 10.0, QStringLiteral( "INPUT" ), false, 0 ) );
}

bool QgsTransectFixedDistanceAlgorithm::prepareAlgorithmTransectParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mInterval = parameterAsDouble( parameters, QStringLiteral( "INTERVAL" ), context );
  return true;
}

std::vector<QgsPoint> QgsTransectFixedDistanceAlgorithm::generateSamplingPoints( const QgsLineString &line, const QVariantMap &, QgsProcessingContext & )
{
  std::vector<QgsPoint> samplingPoints;

  // Sample points at fixed intervals
  double totalLength = line.length();
  for ( double d = 0; d <= totalLength; d += mInterval )
  {
    QgsPoint *pt = line.interpolatePoint( d );
    samplingPoints.push_back( *pt );
  }

  return samplingPoints;
}

double QgsTransectFixedDistanceAlgorithm::calculateAzimuth( const QgsLineString &line, const QgsPoint &point, int )
{
  // For fixed distance sampling, find closest segment
  QgsPoint segPt;
  QgsVertexId vid;
  line.closestSegment( point, segPt, vid, nullptr, Qgis::DEFAULT_SEGMENT_EPSILON );
  QgsVertexId prev( vid.part, vid.ring, vid.vertex - 1 );
  return line.vertexAt( prev ).azimuth( line.vertexAt( vid ) ) * M_PI / 180.0;
}

///@endcond
