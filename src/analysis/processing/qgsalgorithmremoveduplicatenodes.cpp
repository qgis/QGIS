/***************************************************************************
                         qgsalgorithmremoveduplicatenodes.cpp
                         ---------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsalgorithmremoveduplicatenodes.h"

///@cond PRIVATE

QgsProcessingAlgorithm::Flags QgsAlgorithmRemoveDuplicateNodes::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

QString QgsAlgorithmRemoveDuplicateNodes::name() const
{
  return QStringLiteral( "removeduplicatenodes" );
}

QString QgsAlgorithmRemoveDuplicateNodes::displayName() const
{
  return QObject::tr( "Remove duplicate nodes" );
}

QStringList QgsAlgorithmRemoveDuplicateNodes::tags() const
{
  return QObject::tr( "points,valid,overlapping" ).split( ',' );
}

QString QgsAlgorithmRemoveDuplicateNodes::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsAlgorithmRemoveDuplicateNodes::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsAlgorithmRemoveDuplicateNodes::outputName() const
{
  return QObject::tr( "Cleaned" );
}

QString QgsAlgorithmRemoveDuplicateNodes::shortHelpString() const
{
  return QObject::tr( "This algorithm removes duplicate nodes from features, wherever removing the nodes does "
                      "not result in a degenerate geometry.\n\n"
                      "The tolerance parameter specifies the tolerance for coordinates when determining whether "
                      "vertices are identical.\n\n"
                      "By default, z values are not considered when detecting duplicate nodes. E.g. two nodes "
                      "with the same x and y coordinate but different z values will still be considered "
                      "duplicate and one will be removed. If the Use Z Value parameter is true, then the z values are "
                      "also tested and nodes with the same x and y but different z will be maintained.\n\n"
                      "Note that duplicate nodes are not tested between different parts of a multipart geometry. E.g. "
                      "a multipoint geometry with overlapping points will not be changed by this method." );
}

QgsAlgorithmRemoveDuplicateNodes *QgsAlgorithmRemoveDuplicateNodes::createInstance() const
{
  return new QgsAlgorithmRemoveDuplicateNodes();
}

void QgsAlgorithmRemoveDuplicateNodes::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TOLERANCE" ),
                QObject::tr( "Tolerance" ), QgsProcessingParameterNumber::Double,
                0.000001, false, 0, 10000000.0 ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "USE_Z_VALUE" ),
                QObject::tr( "Use Z Value" ), false ) );
}

bool QgsAlgorithmRemoveDuplicateNodes::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsDouble( parameters, QStringLiteral( "TOLERANCE" ), context );
  mUseZValues = parameterAsBool( parameters, QStringLiteral( "USE_Z_VALUE" ), context );
  return true;
}

QgsFeature QgsAlgorithmRemoveDuplicateNodes::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry geometry = f.geometry();
    geometry.removeDuplicateNodes( mTolerance, mUseZValues );
    f.setGeometry( geometry );
  }
  return f;
}

///@endcond


