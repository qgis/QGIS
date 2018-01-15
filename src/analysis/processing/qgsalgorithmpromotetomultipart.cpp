/***************************************************************************
                         qgsalgorithmpromotetomultipart.cpp
                         ---------------------
    begin                : April 2017
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

#include "qgsalgorithmpromotetomultipart.h"

///@cond PRIVATE

QgsProcessingAlgorithm::Flags QgsPromoteToMultipartAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

QString QgsPromoteToMultipartAlgorithm::name() const
{
  return QStringLiteral( "promotetomulti" );
}

QString QgsPromoteToMultipartAlgorithm::displayName() const
{
  return QObject::tr( "Promote to multipart" );
}

QStringList QgsPromoteToMultipartAlgorithm::tags() const
{
  return QObject::tr( "multi,single,multiple,convert,force,parts" ).split( ',' );
}

QString QgsPromoteToMultipartAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsPromoteToMultipartAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsPromoteToMultipartAlgorithm::outputName() const
{
  return QObject::tr( "Multiparts" );
}

QString QgsPromoteToMultipartAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a vector layer with singlepart geometries and generates a new one in which all geometries are "
                      "multipart. Input features which are already multipart features will remain unchanged." ) +
         QStringLiteral( "\n\n" ) +
         QObject::tr( "This algorithm can be used to force geometries to multipart types in order to be compatibility with data providers "
                      "with strict singlepart/multipart compatibility checks." ) +
         QStringLiteral( "\n\n" ) +
         QObject::tr( "See the 'Collect geometries' or 'Aggregate' algorithms for alternative options." );
}

QgsPromoteToMultipartAlgorithm *QgsPromoteToMultipartAlgorithm::createInstance() const
{
  return new QgsPromoteToMultipartAlgorithm();
}

QgsWkbTypes::Type QgsPromoteToMultipartAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  return QgsWkbTypes::multiType( inputWkbType );
}

QgsFeature QgsPromoteToMultipartAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() && !f.geometry().isMultipart() )
  {
    QgsGeometry g = f.geometry();
    g.convertToMultiType();
    f.setGeometry( g );
  }
  return f;
}

///@endcond


