/***************************************************************************
                              qgslayerrestorer.cpp
                              --------------------
  begin                : April 24, 2017
  copyright            : (C) 2017 by Paul Blottiere
  email                : paul.blottiere@oslandia.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayerrestorer.h"
#include "qgsmessagelog.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterrenderer.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsreadwritecontext.h"

QgsLayerRestorer::QgsLayerRestorer( const QList<QgsMapLayer *> &layers )
{
  for ( QgsMapLayer *layer : layers )
  {
    QgsLayerSettings settings;
    settings.name = layer->name();

    settings.mNamedStyle = layer->styleManager()->currentStyle();

    // set a custom property allowing to keep in memory if a SLD file has
    // been loaded for rendering
    layer->setCustomProperty( "readSLD", false );

    QString errMsg;
    QDomDocument styleDoc( QStringLiteral( "style" ) );
    QDomElement styleXml = styleDoc.createElement( QStringLiteral( "style" ) );
    styleDoc.appendChild( styleXml );
    if ( !layer->writeStyle( styleXml, styleDoc, errMsg, QgsReadWriteContext() ) )
    {
      QgsMessageLog::logMessage( QStringLiteral( "QGIS Style has not been added to layer restorer for layer %1: %2" ).arg( layer->name(), errMsg ) );
    }
    ( void )settings.mQgisStyle.setContent( styleDoc.toString() );

    switch ( layer->type() )
    {
      case QgsMapLayerType::VectorLayer:
      {
        QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( layer );

        if ( vLayer )
        {
          settings.mOpacity = vLayer->opacity();
          settings.mSelectedFeatureIds = vLayer->selectedFeatureIds();
          settings.mFilter = vLayer->subsetString();
        }
        break;
      }
      case QgsMapLayerType::RasterLayer:
      {
        QgsRasterLayer *rLayer = qobject_cast<QgsRasterLayer *>( layer );

        if ( rLayer )
        {
          settings.mOpacity = rLayer->renderer()->opacity();
        }
        break;
      }

      case QgsMapLayerType::MeshLayer:
      case QgsMapLayerType::PluginLayer:
        break;
    }

    mLayerSettings[layer] = settings;
  }
}

QgsLayerRestorer::~QgsLayerRestorer()
{
  for ( QgsMapLayer *layer : mLayerSettings.keys() )
  {
    QgsLayerSettings settings = mLayerSettings[layer];
    layer->styleManager()->setCurrentStyle( settings.mNamedStyle );
    layer->setName( mLayerSettings[layer].name );

    // if a SLD file has been loaded for rendering, we restore the previous style
    if ( layer->customProperty( "readSLD", false ).toBool() )
    {
      QString errMsg;
      QDomElement root = settings.mQgisStyle.documentElement();
      QgsReadWriteContext context = QgsReadWriteContext();
      if ( !layer->readStyle( root, errMsg, context ) )
      {
        QgsMessageLog::logMessage( QStringLiteral( "QGIS Style has not been read from layer restorer for layer %1: %2" ).arg( layer->name(), errMsg ) );
      }
    }
    layer->removeCustomProperty( "readSLD" );

    switch ( layer->type() )
    {
      case QgsMapLayerType::VectorLayer:
      {
        QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( layer );

        if ( vLayer )
        {
          vLayer->setOpacity( settings.mOpacity );
          vLayer->selectByIds( settings.mSelectedFeatureIds );
          vLayer->setSubsetString( settings.mFilter );
        }
        break;
      }
      case QgsMapLayerType::RasterLayer:
      {
        QgsRasterLayer *rLayer = qobject_cast<QgsRasterLayer *>( layer );

        if ( rLayer )
        {
          rLayer->renderer()->setOpacity( settings.mOpacity );
        }
        break;
      }

      case QgsMapLayerType::MeshLayer:
      case QgsMapLayerType::PluginLayer:
        break;
    }
  }
}
