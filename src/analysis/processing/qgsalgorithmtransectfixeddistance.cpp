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

#include "qgslinestring.h"
#include "qgsmultilinestring.h"

///@cond PRIVATE

QString QgsTransectFixedDistanceAlgorithm::name() const
{
  return u"transectfixeddistance"_s;
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
         + u"\n\n"_s
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
  addParameter( new QgsProcessingParameterDistance( u"INTERVAL"_s, QObject::tr( "Fixed sampling interval" ), 10.0, u"INPUT"_s, false, 0.0001 ) );
  addParameter( new QgsProcessingParameterBoolean( u"INCLUDE_START"_s, QObject::tr( "Include transect at start of line" ), true ) );
}

bool QgsTransectFixedDistanceAlgorithm::prepareAlgorithmTransectParameters( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mInterval = parameterAsDouble( parameters, u"INTERVAL"_s, context );
  mIncludeStartPoint = parameterAsBool( parameters, u"INCLUDE_START"_s, context );
  return true;
}

std::vector<QgsPoint> QgsTransectFixedDistanceAlgorithm::generateSamplingPoints( const QgsLineString &line, const QVariantMap &, QgsProcessingContext & )
{
  std::vector<QgsPoint> samplingPoints;

  Qgis::WkbType pointType = Qgis::WkbType::Point;
  if ( line.is3D() )
    pointType = Qgis::WkbType::PointZ;
  if ( line.isMeasure() )
    pointType = QgsWkbTypes::addM( pointType );

  if ( mIncludeStartPoint )
  {
    samplingPoints.push_back( line.startPoint() );
  }

  line.visitPointsByRegularDistance( mInterval, [&]( double x, double y, double z, double m, double, double, double, double, double, double, double, double ) -> bool {
    samplingPoints.emplace_back( pointType, x, y, z, m );
    return true;
  } );

  return samplingPoints;
}

double QgsTransectFixedDistanceAlgorithm::calculateAzimuth( const QgsLineString &line, const QgsPoint &point, int )
{
  // For fixed distance sampling, find closest segment
  QgsPoint segPt;
  QgsVertexId vid;
  const double sqrDist = line.closestSegment( point, segPt, vid, nullptr, Qgis::DEFAULT_SEGMENT_EPSILON );

  // Validate closestSegment found a valid segment
  if ( sqrDist < 0 || vid.vertex < 1 )
  {
    // Fallback: use azimuth from first to second vertex if line has at least 2 vertices
    if ( line.numPoints() >= 2 )
    {
      return line.pointN( 0 ).azimuth( line.pointN( 1 ) ) * M_PI / 180.0;
    }
    return 0.0;
  }

  QgsVertexId prev( vid.part, vid.ring, vid.vertex - 1 );
  return line.vertexAt( prev ).azimuth( line.vertexAt( vid ) ) * M_PI / 180.0;
}

///@endcond
