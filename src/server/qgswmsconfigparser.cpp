/***************************************************************************
                              qgswmsconfigparser.cpp
                              ----------------------
  begin                : March 25, 2014
  copyright            : (C) 2014 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswmsconfigparser.h"
#include "qgsmaplayer.h"
#include "qgsmapserviceexception.h"

#include "qgscomposerlabel.h"
#include "qgscomposerlegend.h"
#include "qgscomposermap.h"
#include "qgscomposermapgrid.h"
#include "qgscomposerhtml.h"
#include "qgscomposerframe.h"
#include "qgscomposition.h"

#include "qgslayertreegroup.h"
#include "qgslayertreelayer.h"

QgsWMSConfigParser::QgsWMSConfigParser()
{

}

QgsWMSConfigParser::~QgsWMSConfigParser()
{

}

QgsComposition* QgsWMSConfigParser::createPrintComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, const QMap< QString, QString >& parameterMap ) const
{
  QList<QgsComposerMap*> composerMaps;
  QList<QgsComposerLegend*> composerLegends;
  QList<QgsComposerLabel*> composerLabels;
  QList<const QgsComposerHtml*> composerHtmls;

  QgsComposition* c = initComposition( composerTemplate, mapRenderer, composerMaps, composerLegends, composerLabels, composerHtmls );
  if ( !c )
  {
    return 0;
  }

  QString dpi = parameterMap.value( "DPI" );
  if ( !dpi.isEmpty() )
  {
    c->setPrintResolution( dpi.toInt() );
  }

  //replace composer map parameters
  foreach ( QgsComposerMap* currentMap, composerMaps )
  {
    if ( !currentMap )
    {
      continue;
    }

    QString mapId = "MAP" + QString::number( currentMap->id() );

    QString extent = parameterMap.value( mapId + ":EXTENT" );
    if ( extent.isEmpty() ) //map extent is mandatory
    {
      //remove map from composition if not referenced by the request
      c->removeItem( currentMap ); delete currentMap; continue;
    }

    QStringList coordList = extent.split( "," );
    if ( coordList.size() < 4 )
    {
      c->removeItem( currentMap ); delete currentMap; continue; //need at least four coordinates
    }

    bool xMinOk, yMinOk, xMaxOk, yMaxOk;
    double xmin = coordList.at( 0 ).toDouble( &xMinOk );
    double ymin = coordList.at( 1 ).toDouble( &yMinOk );
    double xmax = coordList.at( 2 ).toDouble( &xMaxOk );
    double ymax = coordList.at( 3 ).toDouble( &yMaxOk );
    if ( !xMinOk || !yMinOk || !xMaxOk || !yMaxOk )
    {
      c->removeItem( currentMap ); delete currentMap; continue;
    }

    QgsRectangle r( xmin, ymin, xmax, ymax );

    //Change x- and y- of extent for WMS 1.3.0 if axis inverted
    QString version = parameterMap.value( "VERSION" );
    if ( version == "1.3.0" && mapRenderer && mapRenderer->destinationCrs().axisInverted() )
    {
      r.invert();
    }
    currentMap->setNewExtent( r );

    //scale
    QString scaleString = parameterMap.value( mapId + ":SCALE" );
    if ( !scaleString.isEmpty() )
    {
      bool scaleOk;
      double scale = scaleString.toDouble( &scaleOk );
      if ( scaleOk )
      {
        currentMap->setNewScale( scale );
      }
    }

    //rotation
    QString rotationString = parameterMap.value( mapId + ":ROTATION" );
    if ( !rotationString.isEmpty() )
    {
      bool rotationOk;
      double rotation = rotationString.toDouble( &rotationOk );
      if ( rotationOk )
      {
        currentMap->setMapRotation( rotation );
      }
    }

    //layers / styles
    QString layers = parameterMap.value( mapId + ":LAYERS" );
    QString styles = parameterMap.value( mapId + ":STYLES" );
    if ( !layers.isEmpty() )
    {
      QStringList layerSet;
      QStringList wmsLayerList = layers.split( "," );
      QStringList wmsStyleList;

      if ( !styles.isEmpty() )
      {
        wmsStyleList = styles.split( "," );
      }

      for ( int i = 0; i < wmsLayerList.size(); ++i )
      {
        QString styleName;
        if ( wmsStyleList.size() > i )
        {
          styleName = wmsStyleList.at( i );
        }

        foreach ( QgsMapLayer *layer, mapLayerFromStyle( wmsLayerList.at( i ), styleName ) )
        {
          if ( layer )
          {
            layerSet.push_back( layer->id() );
          }
        }
      }

      currentMap->setLayerSet( layerSet );
      currentMap->setKeepLayerSet( true );
    }

    //grid space x / y
    currentMap->grid()->setIntervalX( parameterMap.value( mapId + ":GRID_INTERVAL_X" ).toDouble() );
    currentMap->grid()->setIntervalY( parameterMap.value( mapId + ":GRID_INTERVAL_Y" ).toDouble() );
  }
  //update legend
  // if it has an auto-update model
  foreach ( QgsComposerLegend* currentLegend, composerLegends )
  {
    if ( !currentLegend )
    {
      continue;
    }

    if ( currentLegend->autoUpdateModel() || currentLegend->legendFilterByMapEnabled() )
    {
      // the legend has an auto-update model or
      // has to be filter by map
      // we will update it with map's layers
      const QgsComposerMap* map = currentLegend->composerMap();
      if ( !map )
      {
        continue;
      }

      // get model and layer tree root of the legend
      QgsLegendModelV2* model = currentLegend->modelV2();
      QgsLayerTreeGroup* root = model->rootGroup();


      // get layerIds find in the layer tree root
      QStringList layerIds = root->findLayerIds();
      // get map layerIds
      QStringList layerSet = map->layerSet();

      // get map scale
      double scale = map->scale();

      // foreach layer find in the layer tree
      // remove it if the layer id is not in map layerIds
      foreach ( QString layerId, layerIds )
      {
        QgsLayerTreeLayer* nodeLayer = root->findLayer( layerId );
        if ( !nodeLayer )
        {
          continue;
        }
        if ( !layerSet.contains( layerId ) )
        {
          qobject_cast<QgsLayerTreeGroup*>( nodeLayer->parent() )->removeChildNode( nodeLayer );
        }
        else
        {
          QgsMapLayer* layer = nodeLayer->layer();
          if ( layer->hasScaleBasedVisibility() )
          {
            if ( layer->minimumScale() > scale )
              qobject_cast<QgsLayerTreeGroup*>( nodeLayer->parent() )->removeChildNode( nodeLayer );
            else if ( layer->maximumScale() < scale )
              qobject_cast<QgsLayerTreeGroup*>( nodeLayer->parent() )->removeChildNode( nodeLayer );
          }
        }
      }
      root->removeChildrenGroupWithoutLayers();
    }
  }

  //replace label text
  foreach ( QgsComposerLabel *currentLabel, composerLabels )
  {
    QString title = parameterMap.value( currentLabel->id().toUpper() );

    if ( title.isEmpty() )
    {
      //remove exported labels referenced in the request
      //but with empty string
      if ( parameterMap.contains( currentLabel->id().toUpper() ) )
      {
        c->removeItem( currentLabel );
        delete currentLabel;
      }
      continue;
    }

    currentLabel->setText( title );
  }

  //replace html url
  foreach ( const QgsComposerHtml *currentHtml, composerHtmls )
  {
    QgsComposerHtml * html = const_cast<QgsComposerHtml *>( currentHtml );
    QgsComposerFrame *htmlFrame = html->frame( 0 );
    QString htmlId = htmlFrame->id();
    QString url = parameterMap.value( htmlId.toUpper() );

    if ( url.isEmpty() )
    {
      //remove exported Htmls referenced in the request
      //but with empty string
      if ( parameterMap.contains( htmlId.toUpper() ) )
      {
        c->removeMultiFrame( html );
        delete currentHtml;
      }
      else
      {
        html->update();
      }
      continue;
    }

    QUrl newUrl( url );
    html->setUrl( newUrl );
    html->update();
  }

  return c;
}
