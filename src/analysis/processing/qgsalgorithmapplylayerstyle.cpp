/***************************************************************************
                         qgsalgorithmapplylayerstyle.cpp
                         ---------------------
    begin                : December 2019
    copyright            : (C) 2017 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmapplylayerstyle.h"

///@cond PRIVATE

QString QgsApplyLayerStyleAlgorithm::name() const
{
  return QStringLiteral( "setlayerstyle" );
}

QString QgsApplyLayerStyleAlgorithm::displayName() const
{
  return QObject::tr( "Set layer style" );
}

QStringList QgsApplyLayerStyleAlgorithm::tags() const
{
  return QObject::tr( "change,layer,style,qml" ).split( ',' );
}

QString QgsApplyLayerStyleAlgorithm::group() const
{
  return QObject::tr( "Cartography" );
}

QString QgsApplyLayerStyleAlgorithm::groupId() const
{
  return QStringLiteral( "cartography" );
}

QString QgsApplyLayerStyleAlgorithm::shortHelpString() const
{
  return QObject::tr( "Applies the style to a layer. The style must be defined as QML file." );
}

QgsApplyLayerStyleAlgorithm *QgsApplyLayerStyleAlgorithm::createInstance() const
{
  return new QgsApplyLayerStyleAlgorithm();
}

void QgsApplyLayerStyleAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "INPUT" ), QObject::tr( "Layer" ) ) );
  addParameter( new QgsProcessingParameterFile( QStringLiteral( "STYLE" ), QObject::tr( "Style file" ),  QgsProcessingParameterFile::File, QStringLiteral( "qml" ) ) );
  addOutput( new QgsProcessingOutputMapLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Styled" ) ) );
}

bool QgsApplyLayerStyleAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, QStringLiteral( "INPUT" ), context );
  const QString style = parameterAsFile( parameters, QStringLiteral( "STYLE" ), context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  mLayerId = layer->id();

  bool ok = false;
  const QString msg = layer->loadNamedStyle( style, ok );
  if ( !ok )
  {
    throw QgsProcessingException( QObject::tr( "Failed to apply style. Error: %1" ).arg( msg ) );
  }
  layer->triggerRepaint();

  return true;
}

QVariantMap QgsApplyLayerStyleAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  Q_UNUSED( parameters );
  Q_UNUSED( context );

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), mLayerId );
  return results;
}

///@endcond
