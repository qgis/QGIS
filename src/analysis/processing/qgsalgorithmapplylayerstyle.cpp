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
  return u"setlayerstyle"_s;
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
  return u"cartography"_s;
}

QString QgsApplyLayerStyleAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm applies the style to a layer. The style must be defined as QML file." );
}

QString QgsApplyLayerStyleAlgorithm::shortDescription() const
{
  return QObject::tr( "Applies the style from a QML file to a layer." );
}

QgsApplyLayerStyleAlgorithm *QgsApplyLayerStyleAlgorithm::createInstance() const
{
  return new QgsApplyLayerStyleAlgorithm();
}

void QgsApplyLayerStyleAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( u"INPUT"_s, QObject::tr( "Layer" ) ) );
  addParameter( new QgsProcessingParameterFile( u"STYLE"_s, QObject::tr( "Style file" ), Qgis::ProcessingFileParameterBehavior::File, u"qml"_s ) );
  addOutput( new QgsProcessingOutputMapLayer( u"OUTPUT"_s, QObject::tr( "Styled" ) ) );
}

bool QgsApplyLayerStyleAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, u"INPUT"_s, context );
  const QString style = parameterAsFile( parameters, u"STYLE"_s, context );

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
  results.insert( u"OUTPUT"_s, mLayerId );
  return results;
}

///@endcond
