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
#include "qgsmaplayerregistry.h"
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

#include "qgsrendererv2.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

QgsWMSConfigParser::QgsWMSConfigParser()
{

}

QgsWMSConfigParser::~QgsWMSConfigParser()
{

}

QgsComposition* QgsWMSConfigParser::createPrintComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, const QMap< QString, QString >& parameterMap ) const
{
  QStringList highlightLayers;
  return createPrintComposition( composerTemplate, mapRenderer, parameterMap, highlightLayers );
}

QgsComposition* QgsWMSConfigParser::createPrintComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, const QMap< QString, QString >& parameterMap, QStringList& highlightLayers ) const
{
  QList<QgsComposerMap*> composerMaps;
  QList<QgsComposerLegend*> composerLegends;
  QList<QgsComposerLabel*> composerLabels;
  QList<const QgsComposerHtml*> composerHtmls;

  QgsComposition* c = initComposition( composerTemplate, mapRenderer, composerMaps, composerLegends, composerLabels, composerHtmls );
  if ( !c )
  {
    return nullptr;
  }

  QString dpi = parameterMap.value( "DPI" );
  if ( !dpi.isEmpty() )
  {
    c->setPrintResolution( dpi.toInt() );
  }

  //replace composer map parameters
  Q_FOREACH ( QgsComposerMap* currentMap, composerMaps )
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
      c->removeItem( currentMap );
      delete currentMap;
      continue;
    }

    QStringList coordList = extent.split( "," );
    if ( coordList.size() < 4 )
    {
      c->removeItem( currentMap );
      delete currentMap;
      continue; //need at least four coordinates
    }

    bool xMinOk, yMinOk, xMaxOk, yMaxOk;
    double xmin = coordList.at( 0 ).toDouble( &xMinOk );
    double ymin = coordList.at( 1 ).toDouble( &yMinOk );
    double xmax = coordList.at( 2 ).toDouble( &xMaxOk );
    double ymax = coordList.at( 3 ).toDouble( &yMaxOk );
    if ( !xMinOk || !yMinOk || !xMaxOk || !yMaxOk )
    {
      c->removeItem( currentMap );
      delete currentMap;
      continue;
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
    QStringList layerSet;
    if ( currentMap->keepLayerSet() )
    {
      layerSet = currentMap->layerSet();
    }
    else
    {
      QString layers = parameterMap.value( mapId + ":LAYERS" );
      QString styles = parameterMap.value( mapId + ":STYLES" );


      if ( layers.isEmpty() )
      {
        layers = parameterMap.value( "LAYERS" );
        styles = parameterMap.value( "STYLES" );
      }

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
            layerSet.prepend( layer->id() );
          }
        }
      }
    }

    //save layer list prior to adding highlight layers
    QStringList bkLayerSet = layerSet;

    //add highlight layers
    highlightLayers.append( addHighlightLayers( parameterMap, layerSet, mapId + ":" ) );

    currentMap->setLayerSet( layerSet );
    currentMap->setKeepLayerSet( true );

    //remove highlight layers from the composer legends
    QList< QgsComposerLegend* >::iterator legendIt = composerLegends.begin();
    for ( ; legendIt != composerLegends.end(); ++legendIt )
    {
      if (( *legendIt )->autoUpdateModel() )
      {
        setLayerIdsToLegendModel(( *legendIt )->modelV2(), bkLayerSet, currentMap->scale() );
      }
    }

    //grid space x / y
    currentMap->grid()->setIntervalX( parameterMap.value( mapId + ":GRID_INTERVAL_X" ).toDouble() );
    currentMap->grid()->setIntervalY( parameterMap.value( mapId + ":GRID_INTERVAL_Y" ).toDouble() );
  }
//update legend
// if it has an auto-update model
  Q_FOREACH ( QgsComposerLegend* currentLegend, composerLegends )
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
      QStringList layerSet = map->layerSet();
      setLayerIdsToLegendModel( model, layerSet, map->scale() );
    }
  }

//replace label text
  Q_FOREACH ( QgsComposerLabel *currentLabel, composerLabels )
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
  Q_FOREACH ( const QgsComposerHtml *currentHtml, composerHtmls )
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

QStringList QgsWMSConfigParser::addHighlightLayers( const QMap<QString, QString>& parameterMap, QStringList& layerSet, const QString& parameterPrefix )
{
  QStringList highlightLayers, geomSplit, symbolSplit, labelSplit, labelFontSplit, labelSizeSplit,
  labelWeightSplit, labelColorSplit, labelBufferColorSplit, labelBufferSizeSplit;
  highlightParameters( parameterMap, parameterPrefix, geomSplit, symbolSplit, labelSplit, labelFontSplit, labelSizeSplit, labelWeightSplit,
                       labelColorSplit, labelBufferColorSplit, labelBufferSizeSplit );

  if ( geomSplit.isEmpty() || symbolSplit.isEmpty() )
  {
    return highlightLayers;
  }

  QString crsString = parameterMap.contains( "CRS" ) ? parameterMap.value( "CRS" ) : parameterMap.value( "SRS" );

  int nHighlights = qMin( geomSplit.size(), symbolSplit.size() );
  for ( int i = 0; i < nHighlights; ++i )
  {
    //create geometry
    QScopedPointer<QgsGeometry> geom( QgsGeometry::fromWkt( geomSplit.at( i ) ) );
    if ( !geom.data() )
    {
      continue;
    }

    //create renderer from sld
    QDomDocument sldDoc;
    if ( !sldDoc.setContent( symbolSplit[i], true ) )
    {
      continue;
    }

    QString errorMsg;
    QScopedPointer<QgsFeatureRendererV2> renderer( QgsFeatureRendererV2::loadSld( sldDoc.documentElement(), geom.data()->type(), errorMsg ) );
    if ( !renderer.data() )
    {
      continue;
    }

    //add label settings
    QString labelString;
    if ( i < labelSplit.size() )
    {
      labelString = labelSplit.at( i );
    }

    QScopedPointer<QgsVectorLayer> layer( createHighlightLayer( i, crsString, geom.take(), labelString, labelSizeSplit, labelColorSplit, labelWeightSplit, labelFontSplit,
                                          labelBufferSizeSplit, labelBufferColorSplit ) );
    if ( !layer.data() )
    {
      continue;
    }

    layer->setRendererV2( renderer.take() );
    layerSet.prepend( layer.data()->id() );
    highlightLayers.append( layer.data()->id() );
    QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer *>() << layer.take() );
  }
  return highlightLayers;
}

QgsVectorLayer* QgsWMSConfigParser::createHighlightLayer( int i, const QString& crsString, QgsGeometry* geom, const QString& labelString, const QStringList& labelSizeSplit, const QStringList& labelColorSplit,
    const QStringList& labelWeightSplit, const QStringList& labelFontSplit, const QStringList& labelBufferSizeSplit,
    const QStringList& labelBufferColorSplit )
{
  if ( !geom )
  {
    return 0;
  }

  QGis::GeometryType geomType = geom->type();
  QString typeName = QString( QGis::featureType( geom->wkbType() ) ).replace( "WKB", "" );
  QString url = typeName + "?crs=" + crsString;
  if ( !labelString.isEmpty() )
  {
    url += "&field=label:string";
    if ( geomType == QGis::Polygon )
    {
      url += "&field=x:double&field=y:double&field=hali:string&field=vali:string";
    }
  }

  QgsVectorLayer* layer = new QgsVectorLayer( url, "highlight_" + QString::number( i ), "memory" );
  if ( !layer->isValid() )
  {
    delete layer;
    return 0;
  }

  QgsFeature fet( layer->pendingFields() );
  if ( !labelString.isEmpty() )
  {
    fet.setAttribute( 0, labelString );
    if ( geomType == QGis::Polygon )
    {
      QgsGeometry* point = geom->pointOnSurface();
      if ( point )
      {
        QgsPoint pt = point->asPoint();
        fet.setAttribute( 1, pt.x() );
        fet.setAttribute( 2, pt.y() );
        fet.setAttribute( 3, "Center" );
        fet.setAttribute( 4, "Half" );
      }
      delete point;
    }

    layer->setCustomProperty( "labeling/fieldName", "label" );
    layer->setCustomProperty( "labeling/enabled", "true" );
    layer->setCustomProperty( "labeling", "pal" );
    //give highest priority to highlight layers and make sure the labels are always drawn
    layer->setCustomProperty( "labeling/priority", "10" );
    layer->setCustomProperty( "labeling/displayAll", "true" );

    //fontsize?
    if ( i < labelSizeSplit.size() )
    {
      layer->setCustomProperty( "labeling/fontSize", labelSizeSplit.at( i ) );
    }
    //font color
    if ( i < labelColorSplit.size() )
    {
      QColor c( labelColorSplit.at( i ) );
      layer->setCustomProperty( "labeling/textColorR", c.red() );
      layer->setCustomProperty( "labeling/textColorG", c.green() );
      layer->setCustomProperty( "labeling/textColorB", c.blue() );
    }
    //font weight
    if ( i < labelWeightSplit.size() )
    {
      layer->setCustomProperty( "labeling/fontWeight", labelWeightSplit.at( i ) );
    }

    //font family list
    if ( i < labelFontSplit.size() )
    {
      layer->setCustomProperty( "labeling/fontFamily", labelFontSplit.at( i ) );
    }

    //buffer
    if ( i < labelBufferSizeSplit.size() )
    {
      layer->setCustomProperty( "labeling/bufferSize", labelBufferSizeSplit.at( i ) );
    }

    //buffer color
    if ( i <  labelBufferColorSplit.size() )
    {
      QColor c( labelBufferColorSplit.at( i ) );
      layer->setCustomProperty( "labeling/bufferColorR", c.red() );
      layer->setCustomProperty( "labeling/bufferColorG", c.green() );
      layer->setCustomProperty( "labeling/bufferColorB", c.blue() );
    }

    //placement
    int placement = 0;
    switch ( geomType )
    {
      case QGis::Point:
        placement = 0;
        layer->setCustomProperty( "labeling/dist", 2 );
        layer->setCustomProperty( "labeling/placementFlags", 0 );
        break;
      case QGis::Polygon:
        layer->setCustomProperty( "labeling/dataDefinedProperty9", 1 );
        layer->setCustomProperty( "labeling/dataDefinedProperty10", 2 );
        layer->setCustomProperty( "labeling/dataDefinedProperty11", 3 );
        layer->setCustomProperty( "labeling/dataDefinedProperty12", 4 );
        break;
      default:
        placement = 2; //parallel placement for line
        layer->setCustomProperty( "labeling/dist", 2 );
        layer->setCustomProperty( "labeling/placementFlags", 10 );
    }
    layer->setCustomProperty( "labeling/placement", placement );
  }

  fet.setGeometry( geom );
  layer->dataProvider()->addFeatures( QgsFeatureList() << fet );
  return layer;
}

void QgsWMSConfigParser::highlightParameters( const QMap<QString, QString>& parameterMap, const QString& parameterPrefix, QStringList& geom, QStringList& symbol,
    QStringList& label, QStringList& labelFont, QStringList& labelSize, QStringList& labelWeight, QStringList& labelColor,
    QStringList& labelBufferColor, QStringList& labelBufferSize )
{
  QString geomParam = parameterMap.value( parameterPrefix + "HIGHLIGHT_GEOM" );
  QString symbolParam = parameterMap.value( parameterPrefix + "HIGHLIGHT_SYMBOL" );
  if ( geomParam.isEmpty() || symbolParam.isEmpty() )
  {
    return;
  }
  QString labelParam = parameterMap.value( parameterPrefix + "HIGHLIGHT_LABELSTRING" );
  QString labelFontParam = parameterMap.value( parameterPrefix + "HIGHLIGHT_LABELFONT" );
  QString labelSizeParam = parameterMap.value( parameterPrefix + "HIGHLIGHT_LABELSIZE" );
  QString labelWeightParam = parameterMap.value( parameterPrefix + "HIGHLIGHT_LABELWEIGHT" );
  QString labelColorParam = parameterMap.value( parameterPrefix + "HIGHLIGHT_LABELCOLOR" );
  QString labelBufferColorParam = parameterMap.value( parameterPrefix + "HIGHLIGHT_LABELBUFFERCOLOR" );
  QString labelBufferSizeParam = parameterMap.value( parameterPrefix + "HIGHLIGHT_LABELBUFFERSIZE" );

  geom = geomParam.split( ";" );
  symbol = symbolParam.split( ";" );

  label.clear();
  labelFont.clear();
  labelSize.clear();
  labelWeight.clear();
  labelColor.clear();
  labelBufferColor.clear();
  labelBufferSize.clear();

  if ( !labelParam.isEmpty() )
  {
    label = labelParam.split( ";" );
  }
  if ( !labelFontParam.isEmpty() )
  {
    labelFont = labelFontParam.split( ";" );
  }
  if ( !labelSizeParam.isEmpty() )
  {
    labelSize = labelSizeParam.split( ";" );
  }
  if ( !labelWeightParam.isEmpty() )
  {
    labelWeight = labelWeightParam.split( ";" );
  }
  if ( !labelColorParam.isEmpty() )
  {
    labelColor = labelColorParam.split( ";" );
  }
  if ( !labelBufferColorParam.isEmpty() )
  {
    labelBufferColor =  labelBufferColorParam.split( ";" );
  }
  if ( !labelBufferSizeParam.isEmpty() )
  {
    labelBufferSize = labelBufferSizeParam.split( ";" );
  }
}

void QgsWMSConfigParser::removeHighlightLayers( const QStringList& layerIds )
{
  QStringList::const_iterator idIt = layerIds.constBegin();
  for ( ; idIt != layerIds.constEnd(); ++idIt )
  {
    QgsMapLayerRegistry::instance()->removeMapLayers( QStringList() << *idIt );
  }
}

void QgsWMSConfigParser::setLayerIdsToLegendModel( QgsLegendModelV2* model, const QStringList& layerSet, double scale )
{
  if ( !model )
  {
    return;
  }

  // get model and layer tree root of the legend
  QgsLayerTreeGroup* root = model->rootGroup();


  // get layerIds find in the layer tree root
  QStringList layerIds = root->findLayerIds();

  // Q_FOREACH layer find in the layer tree
  // remove it if the layer id is not in map layerIds
  Q_FOREACH ( const QString& layerId, layerIds )
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
