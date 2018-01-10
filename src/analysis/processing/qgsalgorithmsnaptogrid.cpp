/***************************************************************************
                         qgsalgorithmsnaptogrid.cpp
                         --------------------------
    begin                : October 2017
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

#include "qgsalgorithmsnaptogrid.h"

///@cond PRIVATE

QgsProcessingAlgorithm::Flags QgsSnapToGridAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

QString QgsSnapToGridAlgorithm::name() const
{
  return QStringLiteral( "snappointstogrid" );
}

QString QgsSnapToGridAlgorithm::displayName() const
{
  return QObject::tr( "Snap points to grid" );
}

QStringList QgsSnapToGridAlgorithm::tags() const
{
  return QObject::tr( "snapped,grid,simplify,round,precision" ).split( ',' );
}

QString QgsSnapToGridAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsSnapToGridAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsSnapToGridAlgorithm::outputName() const
{
  return QObject::tr( "Snapped" );
}

QString QgsSnapToGridAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm modifies the coordinates of geometries in a vector layer, so that all points "
                      "or vertices are snapped to the closest point of the grid.\n\n"
                      "If the snapped geometry cannot be calculated (or is totally collapsed) the feature's "
                      "geometry will be cleared.\n\n"
                      "Note that snapping to grid may generate an invalid geometry in some corner cases.\n\n"
                      "Snapping can be performed on the X, Y, Z or M axis. A grid spacing of 0 for any axis will "
                      "disable snapping for that axis." );
}

QgsSnapToGridAlgorithm *QgsSnapToGridAlgorithm::createInstance() const
{
  return new QgsSnapToGridAlgorithm();
}

void QgsSnapToGridAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "HSPACING" ),
                QObject::tr( "X Grid Spacing" ), QgsProcessingParameterNumber::Double,
                1, false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "VSPACING" ),
                QObject::tr( "Y Grid Spacing" ), QgsProcessingParameterNumber::Double,
                1, false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "ZSPACING" ),
                QObject::tr( "Z Grid Spacing" ), QgsProcessingParameterNumber::Double,
                0, false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MSPACING" ),
                QObject::tr( "M Grid Spacing" ), QgsProcessingParameterNumber::Double,
                0, false, 0 ) );
}

bool QgsSnapToGridAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mIntervalX = parameterAsDouble( parameters, QStringLiteral( "HSPACING" ), context );
  mIntervalY = parameterAsDouble( parameters, QStringLiteral( "VSPACING" ), context );
  mIntervalZ = parameterAsDouble( parameters, QStringLiteral( "ZSPACING" ), context );
  mIntervalM = parameterAsDouble( parameters, QStringLiteral( "MSPACING" ), context );
  return true;
}

QgsFeature QgsSnapToGridAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry outputGeometry = f.geometry().snappedToGrid( mIntervalX, mIntervalY, mIntervalZ, mIntervalM );
    if ( !outputGeometry )
    {
      feedback->reportError( QObject::tr( "Error snapping geometry %1" ).arg( feature.id() ) );
    }
    f.setGeometry( outputGeometry );
  }
  return f;
}

///@endcond


