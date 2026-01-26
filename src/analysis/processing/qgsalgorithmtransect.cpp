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

#include "qgslinestring.h"
#include "qgsmultilinestring.h"

///@cond PRIVATE

QString QgsTransectAlgorithm::name() const
{
  return u"transect"_s;
}

QString QgsTransectAlgorithm::displayName() const
{
  return QObject::tr( "Transect" );
}

QString QgsTransectAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates transects on vertices for (multi)linestrings.\n" )
         + QObject::tr( "A transect is a line oriented from an angle (by default perpendicular) to the input polylines (at vertices)." )
         + u"\n\n"_s
         + QObject::tr( "Field(s) from feature(s) are returned in the transect with these new fields:\n" )
         + QObject::tr( "- TR_FID: ID of the original feature\n" )
         + QObject::tr( "- TR_ID: ID of the transect. Each transect have an unique ID\n" )
         + QObject::tr( "- TR_SEGMENT: ID of the segment of the linestring\n" )
         + QObject::tr( "- TR_ANGLE: Angle in degrees from the original line at the vertex\n" )
         + QObject::tr( "- TR_LENGTH: Total length of the transect returned\n" )
         + QObject::tr( "- TR_ORIENT: Side of the transect (only on the left or right of the line, or both side)\n" );
}

QgsTransectAlgorithm *QgsTransectAlgorithm::createInstance() const
{
  return new QgsTransectAlgorithm();
}

void QgsTransectAlgorithm::addAlgorithmParams()
{
  // No additional parameters for the basic transect algorithm (vertex-based only)
}

bool QgsTransectAlgorithm::prepareAlgorithmTransectParameters( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * )
{
  // No additional preparation needed for basic transect algorithm
  return true;
}

std::vector<QgsPoint> QgsTransectAlgorithm::generateSamplingPoints( const QgsLineString &line, const QVariantMap &, QgsProcessingContext & )
{
  std::vector<QgsPoint> samplingPoints;

  for ( auto it = line.vertices_begin(); it != line.vertices_end(); ++it )
    samplingPoints.push_back( *it );

  return samplingPoints;
}

double QgsTransectAlgorithm::calculateAzimuth( const QgsLineString &line, const QgsPoint &, int pointIndex )
{
  return line.vertexAngle( QgsVertexId( 0, 0, pointIndex ) );
}

///@endcond
