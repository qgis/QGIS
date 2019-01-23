/***************************************************************************
                         qgsalgorithmsplitlineantimeridian.cpp
                         -------------------------------------
    begin                : January 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgsalgorithmsplitlineantimeridian.h"
#include "qgscurve.h"
#include "qgslinestring.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgsgeometrycollection.h"

///@cond PRIVATE

QString QgsSplitGeometryAtAntimeridianAlgorithm::name() const
{
  return QStringLiteral( "antimeridiansplit" );
}

QString QgsSplitGeometryAtAntimeridianAlgorithm::displayName() const
{
  return QObject::tr( "Geodesic line split at antimeridian" );
}

QStringList QgsSplitGeometryAtAntimeridianAlgorithm::tags() const
{
  return QObject::tr( "break,cut,dateline,180,-180,longitude,geographic,ellipsoid" ).split( ',' );
}

QString QgsSplitGeometryAtAntimeridianAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsSplitGeometryAtAntimeridianAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsSplitGeometryAtAntimeridianAlgorithm::shortDescription() const
{
  return QObject::tr( "Splits lines into multiple geodesic segments when the line crosses the antimeridian (±180 degrees longitude)." );
}

QString QgsSplitGeometryAtAntimeridianAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm splits a line into multiple geodesic segments, whenever the line crosses the antimeridian (±180 degrees longitude).\n\n"
                      "Splitting at the antimeridian helps the visual display of the lines in some projections. The returned "
                      "geometry will always be a multi-part geometry.\n\n"
                      "Whenever line segments in the input geometry cross the antimeridian, they will be "
                      "split into two segments, with the latitude of the breakpoint being determined using a geodesic "
                      "line connecting the points either side of this segment. The current project ellipsoid setting will "
                      "be used when calculating this breakpoint.\n\n"
                      "If the input geometry contains M or Z values, these will be linearly interpolated for the new vertices "
                      "created at the antimeridian." );
}

QList<int> QgsSplitGeometryAtAntimeridianAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine;
}

QgsProcessing::SourceType QgsSplitGeometryAtAntimeridianAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorLine;
}

QgsSplitGeometryAtAntimeridianAlgorithm *QgsSplitGeometryAtAntimeridianAlgorithm::createInstance() const
{
  return new QgsSplitGeometryAtAntimeridianAlgorithm();
}

QString QgsSplitGeometryAtAntimeridianAlgorithm::outputName() const
{
  return QObject::tr( "Split" );
}

QgsWkbTypes::Type QgsSplitGeometryAtAntimeridianAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  return QgsWkbTypes::multiType( inputWkbType );
}

QgsCoordinateReferenceSystem QgsSplitGeometryAtAntimeridianAlgorithm::outputCrs( const QgsCoordinateReferenceSystem &inputCrs ) const
{
  mDa.setSourceCrs( inputCrs, mTransformContext );
  return inputCrs;
}

bool QgsSplitGeometryAtAntimeridianAlgorithm::prepareAlgorithm( const QVariantMap &, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  if ( context.project() )
  {
    mDa.setEllipsoid( context.project()->ellipsoid() );
  }

  mTransformContext = context.transformContext();
  return true;
}

QgsFeatureList QgsSplitGeometryAtAntimeridianAlgorithm::processFeature( const QgsFeature &f, QgsProcessingContext &, QgsProcessingFeedback * )
{
  if ( !f.hasGeometry() )
  {
    return QgsFeatureList() << f;
  }
  else
  {
    QgsFeature feat = f;
    feat.setGeometry( mDa.splitGeometryAtAntimeridian( f.geometry() ) );
    return QgsFeatureList() << feat;
  }
}


///@endcond



