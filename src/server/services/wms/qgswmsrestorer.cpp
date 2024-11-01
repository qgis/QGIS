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
    mLayerSettings.emplace( layer, QgsLayerSettings() );
    QgsLayerSettings &settings = mLayerSettings[layer];

    settings.name = layer->name();
    settings.mNamedStyle = layer->styleManager()->currentStyle();

    switch ( layer->type() )
    {
      case Qgis::LayerType::Vector:
      {
        QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( layer );

        if ( vLayer )
        {
          settings.mOpacity = vLayer->opacity();
          settings.mSelectedFeatureIds = vLayer->selectedFeatureIds();
          settings.mFilter = vLayer->subsetString();
          // Labeling opacity
          if ( vLayer->labelsEnabled() && vLayer->labeling() )
          {
            settings.mLabeling.reset( vLayer->labeling()->clone() );
          }
        }
        break;
      }
      case Qgis::LayerType::Raster:
      {
        QgsRasterLayer *rLayer = qobject_cast<QgsRasterLayer *>( layer );

        if ( rLayer && rLayer->renderer() )
        {
          settings.mOpacity = rLayer->renderer()->opacity();
        }
        break;
      }

      case Qgis::LayerType::Mesh:
      case Qgis::LayerType::VectorTile:
      case Qgis::LayerType::Plugin:
      case Qgis::LayerType::Annotation:
      case Qgis::LayerType::PointCloud:
      case Qgis::LayerType::Group:
      case Qgis::LayerType::TiledScene:
        break;
    }
  }
}

QgsLayerRestorer::~QgsLayerRestorer()
{
  for ( auto it = mLayerSettings.begin(); it != mLayerSettings.end(); it++ )
  {
    QgsMapLayer *layer = it->first;

    // Firstly check if a SLD file has been loaded for rendering and removed it
    const QString sldStyleName { layer->customProperty( "sldStyleName", "" ).toString() };
    if ( !sldStyleName.isEmpty() )
    {
      layer->styleManager()->removeStyle( sldStyleName );
      layer->removeCustomProperty( "sldStyleName" );
    }

    // Then restore the previous style
    QgsLayerSettings &settings = it->second;
    layer->styleManager()->setCurrentStyle( settings.mNamedStyle );
    layer->setName( settings.name );

    switch ( layer->type() )
    {
      case Qgis::LayerType::Vector:
      {
        QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( layer );

        if ( vLayer )
        {
          vLayer->setOpacity( settings.mOpacity );
          vLayer->selectByIds( settings.mSelectedFeatureIds );
          vLayer->setSubsetString( settings.mFilter );
          if ( settings.mLabeling )
          {
            vLayer->setLabeling( settings.mLabeling.release() );
          }
        }
        break;
      }
      case Qgis::LayerType::Raster:
      {
        QgsRasterLayer *rLayer = qobject_cast<QgsRasterLayer *>( layer );

        if ( rLayer && rLayer->renderer() )
        {
          rLayer->renderer()->setOpacity( settings.mOpacity );
        }
        break;
      }

      case Qgis::LayerType::Mesh:
      case Qgis::LayerType::VectorTile:
      case Qgis::LayerType::Plugin:
      case Qgis::LayerType::Annotation:
      case Qgis::LayerType::PointCloud:
      case Qgis::LayerType::Group:
      case Qgis::LayerType::TiledScene:
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
} // namespace QgsWms
