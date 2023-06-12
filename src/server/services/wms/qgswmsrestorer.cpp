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
      case Qgis::LayerType::Vector:
      {
        QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( layer );

        if ( vLayer )
        {
          settings.mOpacity = vLayer->opacity();
          settings.mSelectedFeatureIds = vLayer->selectedFeatureIds();
          settings.mFilter = vLayer->subsetString();
          settings.mLabelingOpacity = -1;
          // Labeling opacity
          if ( vLayer->labelsEnabled() && vLayer->labeling() )
          {
            QgsAbstractVectorLayerLabeling *labeling { vLayer->labeling() };
            if ( QgsVectorLayerSimpleLabeling *simpleLabeling = static_cast<QgsVectorLayerSimpleLabeling *>( labeling ) )
            {
              settings.mLabelingOpacity =  simpleLabeling->settings( ).format().opacity();
            }
            else if ( QgsRuleBasedLabeling *ruleLabeling = static_cast<QgsRuleBasedLabeling *>( labeling ) )
            {
              settings.mLabelingOpacity =  ruleLabeling->settings( ).format().opacity();
            }
          }
        }
        break;
      }
      case Qgis::LayerType::Raster:
      {
        QgsRasterLayer *rLayer = qobject_cast<QgsRasterLayer *>( layer );

        if ( rLayer )
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
        break;
    }

    mLayerSettings[layer] = settings;
  }
}

QgsLayerRestorer::~QgsLayerRestorer()
{
  for ( auto it = mLayerSettings.constBegin(); it != mLayerSettings.constEnd(); it++ )
  {
    QgsMapLayer *layer = it.key();

    // Firstly check if a SLD file has been loaded for rendering and removed it
    const QString sldStyleName { layer->customProperty( "sldStyleName", "" ).toString() };
    if ( !sldStyleName.isEmpty() )
    {
      layer->styleManager()->removeStyle( sldStyleName );
      layer->removeCustomProperty( "sldStyleName" );
    }

    // Then restore the previous style
    const QgsLayerSettings settings = it.value();
    layer->styleManager()->setCurrentStyle( settings.mNamedStyle );
    layer->setName( it.value().name );

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
          if ( settings.mLabelingOpacity != -1 )
          {
            QgsAbstractVectorLayerLabeling *labeling { vLayer->labeling() };
            if ( QgsVectorLayerSimpleLabeling *simpleLabeling = static_cast<QgsVectorLayerSimpleLabeling *>( labeling ) )
            {
              std::unique_ptr<QgsPalLayerSettings> labelSettings = std::make_unique<QgsPalLayerSettings>( simpleLabeling->settings( ) );
              QgsTextFormat format { labelSettings->format() };
              format.setOpacity( settings.mLabelingOpacity );
              labelSettings->setFormat( format );
              simpleLabeling->setSettings( labelSettings.release() );
            }
            else if ( QgsRuleBasedLabeling *ruleLabeling = static_cast<QgsRuleBasedLabeling *>( labeling ) )
            {
              std::unique_ptr<QgsPalLayerSettings> labelSettings = std::make_unique<QgsPalLayerSettings>( simpleLabeling->settings( ) );
              QgsTextFormat format { labelSettings->format() };
              format.setOpacity( settings.mLabelingOpacity );
              labelSettings->setFormat( format );
              ruleLabeling->setSettings( labelSettings.release() );
            }
          }
        }
        break;
      }
      case Qgis::LayerType::Raster:
      {
        QgsRasterLayer *rLayer = qobject_cast<QgsRasterLayer *>( layer );

        if ( rLayer )
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
