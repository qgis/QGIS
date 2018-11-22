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
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterrenderer.h"
#include "qgsmaplayerstylemanager.h"

QgsLayerRestorer::QgsLayerRestorer( const QList<QgsMapLayer *> &layers )
{
  for ( QgsMapLayer *layer : layers )
  {
    QgsLayerSettings settings;
    settings.name = layer->name();

    QString style = layer->styleManager()->currentStyle();
    settings.mNamedStyle = layer->styleManager()->currentStyle();

    // set a custom property allowing to keep in memory if a SLD file has
    // been loaded for rendering
    layer->setCustomProperty( "readSLD", false );

    QString errMsg;
    QDomDocument sldDoc;
    layer->exportSldStyle( sldDoc, errMsg );
    ( void )settings.mSldStyle.setContent( sldDoc.toString(), true ); // for namespace processing

    if ( layer->type() == QgsMapLayer::LayerType::VectorLayer )
    {
      QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( layer );

      if ( vLayer )
      {
        settings.mOpacity = vLayer->opacity();
        settings.mSelectedFeatureIds = vLayer->selectedFeatureIds();
        settings.mFilter = vLayer->subsetString();
      }
    }
    else if ( layer->type() == QgsMapLayer::LayerType::RasterLayer )
    {
      QgsRasterLayer *rLayer = qobject_cast<QgsRasterLayer *>( layer );

      if ( rLayer )
      {
        settings.mOpacity = rLayer->renderer()->opacity();
      }
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

    // if a SLD file has been loaded for rendering, we restore the previous one
    QString errMsg;
    QDomElement root = settings.mSldStyle.firstChildElement( "StyledLayerDescriptor" );
    QDomElement el = root.firstChildElement( "NamedLayer" );
    if ( layer->customProperty( "readSLD", false ).toBool() )
    {
      layer->readSld( el, errMsg );
    }
    layer->removeCustomProperty( "readSLD" );

    if ( layer->type() == QgsMapLayer::LayerType::VectorLayer )
    {
      QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( layer );

      if ( vLayer )
      {
        vLayer->setOpacity( settings.mOpacity );
        vLayer->selectByIds( settings.mSelectedFeatureIds );
        vLayer->setSubsetString( settings.mFilter );
      }
    }
    else if ( layer->type() == QgsMapLayer::LayerType::RasterLayer )
    {
      QgsRasterLayer *rLayer = qobject_cast<QgsRasterLayer *>( layer );

      if ( rLayer )
      {
        rLayer->renderer()->setOpacity( settings.mOpacity );
      }
    }
  }
}
