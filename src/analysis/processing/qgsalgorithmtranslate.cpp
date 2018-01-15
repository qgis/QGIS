/***************************************************************************
                         qgsalgorithmtranslate.cpp
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

#include "qgsalgorithmtranslate.h"

///@cond PRIVATE

QgsProcessingAlgorithm::Flags QgsTranslateAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

QString QgsTranslateAlgorithm::name() const
{
  return QStringLiteral( "translategeometry" );
}

QString QgsTranslateAlgorithm::displayName() const
{
  return QObject::tr( "Translate" );
}

QStringList QgsTranslateAlgorithm::tags() const
{
  return QObject::tr( "move,shift,transform,z,m,values,add" ).split( ',' );
}

QString QgsTranslateAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsTranslateAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsTranslateAlgorithm::outputName() const
{
  return QObject::tr( "Translated" );
}

QString QgsTranslateAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm moves the geometries within a layer, by offsetting them with a specified x and y displacement." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Z and M values present in the geometry can also be translated." );
}

QgsTranslateAlgorithm *QgsTranslateAlgorithm::createInstance() const
{
  return new QgsTranslateAlgorithm();
}

void QgsTranslateAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "DELTA_X" ),
                QObject::tr( "Offset distance (x-axis)" ), QgsProcessingParameterNumber::Double,
                0.0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "DELTA_Y" ),
                QObject::tr( "Offset distance (y-axis)" ), QgsProcessingParameterNumber::Double,
                0.0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "DELTA_Z" ),
                QObject::tr( "Offset distance (z-axis)" ), QgsProcessingParameterNumber::Double,
                0.0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "DELTA_M" ),
                QObject::tr( "Offset distance (m values)" ), QgsProcessingParameterNumber::Double,
                0.0 ) );
}

bool QgsTranslateAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mDeltaX = parameterAsDouble( parameters, QStringLiteral( "DELTA_X" ), context );
  mDeltaY = parameterAsDouble( parameters, QStringLiteral( "DELTA_Y" ), context );
  mDeltaZ = parameterAsDouble( parameters, QStringLiteral( "DELTA_Z" ), context );
  mDeltaM = parameterAsDouble( parameters, QStringLiteral( "DELTA_M" ), context );

  return true;
}

QgsFeature QgsTranslateAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry geometry = f.geometry();
    if ( mDeltaZ != 0 && !geometry.constGet()->is3D() )
      geometry.get()->addZValue( 0 );
    if ( mDeltaM != 0 && !geometry.constGet()->isMeasure() )
      geometry.get()->addMValue( 0 );

    geometry.translate( mDeltaX, mDeltaY, mDeltaZ, mDeltaM );
    f.setGeometry( geometry );
  }
  return f;
}

QgsWkbTypes::Type QgsTranslateAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  QgsWkbTypes::Type wkb = inputWkbType;
  if ( mDeltaZ != 0 )
    wkb = QgsWkbTypes::addZ( wkb );
  if ( mDeltaM != 0 )
    wkb = QgsWkbTypes::addM( wkb );
  return wkb;
}

///@endcond


