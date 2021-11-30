/***************************************************************************
                              qgswmsrestorer.cpp
                              ------------------
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

#include "qgswmsrestorer.h"
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
      case QgsMapLayerType::VectorTileLayer:
      case QgsMapLayerType::PluginLayer:
      case QgsMapLayerType::AnnotationLayer:
      case QgsMapLayerType::PointCloudLayer:
      case QgsMapLayerType::GroupLayer:
        break;
    }

    mLayerSettings[layer] = settings;
  }
}

QgsLayerRestorer::~QgsLayerRestorer()
{
  for ( QgsMapLayer *layer : mLayerSettings.keys() )
  {
    // Firstly check if a SLD file has been loaded for rendering and removed it
    const QString sldStyleName { layer->customProperty( "sldStyleName", "" ).toString() };
    if ( !sldStyleName.isEmpty() )
    {
      layer->styleManager()->removeStyle( sldStyleName );
      layer->removeCustomProperty( "sldStyleName" );
    }

    // Then restore the previous style
    const QgsLayerSettings settings = mLayerSettings[layer];
    layer->styleManager()->setCurrentStyle( settings.mNamedStyle );
    layer->setName( mLayerSettings[layer].name );

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
      case QgsMapLayerType::VectorTileLayer:
      case QgsMapLayerType::PluginLayer:
      case QgsMapLayerType::AnnotationLayer:
      case QgsMapLayerType::PointCloudLayer:
      case QgsMapLayerType::GroupLayer:
        break;
    }
  }
}

namespace QgsWms
{
  QgsWmsRestorer::QgsWmsRestorer( const QgsWmsRenderContext &context )
    : mLayerRestorer( context.layers() )
  {
  }
}
