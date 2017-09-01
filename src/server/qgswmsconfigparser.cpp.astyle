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

#include "qgis_server.h"

#include "qgswmsconfigparser.h"
#include "qgsmaplayer.h"
#include "qgspallabeling.h"
#include "qgsproject.h"
#include "qgsmapserviceexception.h"

#include "qgscomposerlabel.h"
#include "qgscomposerlegend.h"
#include "qgscomposermap.h"
#include "qgscomposermapgrid.h"
#include "qgscomposerhtml.h"
#include "qgscomposerframe.h"
#include "qgscomposition.h"
#include "qgsmapsettings.h"

#include "qgslayertreegroup.h"
#include "qgslayertreelayer.h"
#include "qgslayertree.h"

#include "qgsrenderer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerlabeling.h"


QgsWmsConfigParser::QgsWmsConfigParser()
{
}

QgsComposition *QgsWmsConfigParser::createPrintComposition( const QString &composerTemplate, const QgsMapSettings &mapSettings, const QMap< QString, QString > &parameterMap, QStringList &highlightLayers ) const
{
  QList<QgsComposerMap *> composerMaps;
  QList<QgsComposerLegend *> composerLegends;
  QList<QgsComposerLabel *> composerLabels;
  QList<const QgsComposerHtml *> composerHtmls;

  QgsComposition *c = initComposition( composerTemplate, mapSettings, composerMaps, composerLegends, composerLabels, composerHtmls );
  if ( !c )
  {
    return nullptr;
  }

  QString dpi = parameterMap.value( QStringLiteral( "DPI" ) );
  if ( !dpi.isEmpty() )
  {
    c->setPrintResolution( dpi.toInt() );
  }

  //replace composer map parameters
  Q_FOREACH ( QgsComposerMap *currentMap, composerMaps )
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

    // Change CRS of map set to "project CRS" to match requested CRS
    // (if map has a valid preset crs then we keep this crs and don't use the
    // requested crs for this map item)
    if ( mapSettings.destinationCrs().isValid() && !currentMap->presetCrs().isValid() )
      currentMap->setCrs( mapSettings.destinationCrs() );

    QStringList coordList = extent.split( QStringLiteral( "," ) );
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
    QString version = parameterMap.value( QStringLiteral( "VERSION" ) );
    if ( version == QLatin1String( "1.3.0" ) && mapSettings.destinationCrs().hasAxisInverted() )
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
      Q_FOREACH ( QgsMapLayer *layer, currentMap->layers() )
        layerSet << layer->id();
    }
    else
    {
      QString layers = parameterMap.value( mapId + ":LAYERS" );
      QString styles = parameterMap.value( mapId + ":STYLES" );


      if ( layers.isEmpty() )
      {
        layers = parameterMap.value( QStringLiteral( "LAYERS" ) );
        styles = parameterMap.value( QStringLiteral( "STYLES" ) );
      }

      QStringList wmsLayerList = layers.split( QStringLiteral( "," ) );
      QStringList wmsStyleList;

      if ( !styles.isEmpty() )
      {
        wmsStyleList = styles.split( QStringLiteral( "," ) );
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

    QList<QgsMapLayer *> layers;
    Q_FOREACH ( const QString &layerId, layerSet )
    {
      if ( QgsMapLayer *layer = QgsProject::instance()->mapLayer( layerId ) )
        layers << layer;
    }

    currentMap->setLayers( layers );
    currentMap->setKeepLayerSet( true );

    //remove highlight layers from the composer legends
    QList< QgsComposerLegend * >::iterator legendIt = composerLegends.begin();
    for ( ; legendIt != composerLegends.end(); ++legendIt )
    {
      if ( ( *legendIt )->autoUpdateModel() )
      {
        setLayerIdsToLegendModel( ( *legendIt )->model(), bkLayerSet, currentMap->scale() );
      }
    }

    //grid space x / y
    currentMap->grid()->setIntervalX( parameterMap.value( mapId + ":GRID_INTERVAL_X" ).toDouble() );
    currentMap->grid()->setIntervalY( parameterMap.value( mapId + ":GRID_INTERVAL_Y" ).toDouble() );
  }
//update legend
// if it has an auto-update model
  Q_FOREACH ( QgsComposerLegend *currentLegend, composerLegends )
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
      const QgsComposerMap *map = currentLegend->composerMap();
      if ( !map )
      {
        continue;
      }

      // get model and layer tree root of the legend
      QgsLegendModel *model = currentLegend->model();
      QStringList layerSet;
      Q_FOREACH ( QgsMapLayer *layer, map->layers() )
        layerSet << layer->id();
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
    QgsComposerHtml *html = const_cast<QgsComposerHtml *>( currentHtml );
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

QStringList QgsWmsConfigParser::addHighlightLayers( const QMap<QString, QString> &parameterMap, QStringList &layerSet, const QString &parameterPrefix )
{
  QStringList highlightLayers, geomSplit, symbolSplit, labelSplit, labelFontSplit, labelSizeSplit,
              labelWeightSplit, labelColorSplit, labelBufferColorSplit, labelBufferSizeSplit;
  highlightParameters( parameterMap, parameterPrefix, geomSplit, symbolSplit, labelSplit, labelFontSplit, labelSizeSplit, labelWeightSplit,
                       labelColorSplit, labelBufferColorSplit, labelBufferSizeSplit );

  if ( geomSplit.isEmpty() || symbolSplit.isEmpty() )
  {
    return highlightLayers;
  }

  QString crsString = parameterMap.contains( QStringLiteral( "CRS" ) ) ? parameterMap.value( QStringLiteral( "CRS" ) ) : parameterMap.value( QStringLiteral( "SRS" ) );

  int nHighlights = std::min( geomSplit.size(), symbolSplit.size() );
  for ( int i = 0; i < nHighlights; ++i )
  {
    //create geometry
    QgsGeometry geom( QgsGeometry::fromWkt( geomSplit.at( i ) ) );
    if ( !geom )
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
    std::unique_ptr<QgsFeatureRenderer> renderer( QgsFeatureRenderer::loadSld( sldDoc.documentElement(), geom.type(), errorMsg ) );
    if ( !renderer )
    {
      continue;
    }

    //add label settings
    QString labelString;
    if ( i < labelSplit.size() )
    {
      labelString = labelSplit.at( i );
    }

    std::unique_ptr<QgsVectorLayer> layer( createHighlightLayer( i, crsString, geom, labelString, labelSizeSplit, labelColorSplit, labelWeightSplit, labelFontSplit,
                                           labelBufferSizeSplit, labelBufferColorSplit ) );
    if ( !layer )
    {
      continue;
    }

    layer->setRenderer( renderer.release() );
    layerSet.prepend( layer->id() );
    highlightLayers.append( layer->id() );
    QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << layer.release() );
  }
  return highlightLayers;
}

QgsVectorLayer *QgsWmsConfigParser::createHighlightLayer( int i, const QString &crsString, const QgsGeometry &geom, const QString &labelString, const QStringList &labelSizeSplit, const QStringList &labelColorSplit,
    const QStringList &labelWeightSplit, const QStringList &labelFontSplit, const QStringList &labelBufferSizeSplit,
    const QStringList &labelBufferColorSplit )
{
  if ( !geom )
  {
    return 0;
  }

  QgsWkbTypes::GeometryType geomType = geom.type();
  QString typeName = QString( QgsWkbTypes::displayString( geom.wkbType() ) ).replace( QLatin1String( "WKB" ), QLatin1String( "" ) );
  QString url = typeName + "?crs=" + crsString;
  if ( !labelString.isEmpty() )
  {
    url += QLatin1String( "&field=label:string" );
    if ( geomType == QgsWkbTypes::PolygonGeometry )
    {
      url += QLatin1String( "&field=x:double&field=y:double&field=hali:string&field=vali:string" );
    }
  }

  QgsVectorLayer *layer = new QgsVectorLayer( url, "highlight_" + QString::number( i ), QStringLiteral( "memory" ) );
  if ( !layer->isValid() )
  {
    delete layer;
    return 0;
  }

  QgsPalLayerSettings settings;
  settings.fieldName = "label";

  //give highest priority to highlight layers and make sure the labels are always drawn
  settings.priority = 10;
  settings.displayAll = true;

  QgsTextFormat textFormat;
  QgsTextBufferSettings bufferSettings;

  QString fontFamily;
  int fontSize = 12;
  int fontWeight = -1;

  QgsPropertyCollection &ddp = settings.dataDefinedProperties();

  QgsFeature fet( layer->pendingFields() );
  if ( !labelString.isEmpty() )
  {
    fet.setAttribute( 0, labelString );
    if ( geomType == QgsWkbTypes::PolygonGeometry )
    {
      QgsGeometry point = geom.pointOnSurface();
      if ( point )
      {
        QgsPointXY pt = point.asPoint();
        fet.setAttribute( 1, pt.x() );
        fet.setAttribute( 2, pt.y() );
        fet.setAttribute( 3, "Center" );
        fet.setAttribute( 4, "Half" );
      }
    }

    //fontsize?
    if ( i < labelSizeSplit.size() )
    {
      fontSize = labelSizeSplit.at( i ).toInt();
    }
    //font color
    if ( i < labelColorSplit.size() )
    {
      QColor c( labelColorSplit.at( i ) );
      textFormat.setColor( c );
    }
    //font weight
    if ( i < labelWeightSplit.size() )
    {
      fontWeight = labelWeightSplit.at( i ).toInt();
    }

    //font family list
    if ( i < labelFontSplit.size() )
    {
      fontFamily = labelFontSplit.at( i );
    }

    //buffer
    if ( i < labelBufferSizeSplit.size() )
    {
      bufferSettings.setSize( labelBufferSizeSplit.at( i ).toInt() );
    }

    //buffer color
    if ( i <  labelBufferColorSplit.size() )
    {
      QColor c( labelBufferColorSplit.at( i ) );
      bufferSettings.setColor( c );
    }

    //placement
    switch ( geomType )
    {
      case QgsWkbTypes::PointGeometry:
        settings.placement = QgsPalLayerSettings::AroundPoint;
        settings.dist = 2;
        settings.placementFlags = 0;
        break;
      case QgsWkbTypes::PolygonGeometry:
        settings.placement = QgsPalLayerSettings::AroundPoint;
        ddp.setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( "x" ) );
        ddp.setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( "y" ) );
        ddp.setProperty( QgsPalLayerSettings::Hali, QgsProperty::fromField( "hali" ) );
        ddp.setProperty( QgsPalLayerSettings::Vali, QgsProperty::fromValue( "vali" ) );
        break;
      default:
        settings.placement = QgsPalLayerSettings::Line; //parallel placement for line
        settings.dist = 2;
        settings.placementFlags = 10;
    }
  }

  textFormat.setFont( QFont( fontFamily, fontSize, fontWeight ) );
  textFormat.setSize( fontSize );
  textFormat.setBuffer( bufferSettings );
  settings.setFormat( textFormat );
  layer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );

  fet.setGeometry( geom );
  layer->dataProvider()->addFeatures( QgsFeatureList() << fet );
  return layer;
}

void QgsWmsConfigParser::highlightParameters( const QMap<QString, QString> &parameterMap, const QString &parameterPrefix, QStringList &geom, QStringList &symbol,
    QStringList &label, QStringList &labelFont, QStringList &labelSize, QStringList &labelWeight, QStringList &labelColor,
    QStringList &labelBufferColor, QStringList &labelBufferSize )
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

  geom = geomParam.split( QStringLiteral( ";" ) );
  symbol = symbolParam.split( QStringLiteral( ";" ) );

  label.clear();
  labelFont.clear();
  labelSize.clear();
  labelWeight.clear();
  labelColor.clear();
  labelBufferColor.clear();
  labelBufferSize.clear();

  if ( !labelParam.isEmpty() )
  {
    label = labelParam.split( QStringLiteral( ";" ) );
  }
  if ( !labelFontParam.isEmpty() )
  {
    labelFont = labelFontParam.split( QStringLiteral( ";" ) );
  }
  if ( !labelSizeParam.isEmpty() )
  {
    labelSize = labelSizeParam.split( QStringLiteral( ";" ) );
  }
  if ( !labelWeightParam.isEmpty() )
  {
    labelWeight = labelWeightParam.split( QStringLiteral( ";" ) );
  }
  if ( !labelColorParam.isEmpty() )
  {
    labelColor = labelColorParam.split( QStringLiteral( ";" ) );
  }
  if ( !labelBufferColorParam.isEmpty() )
  {
    labelBufferColor =  labelBufferColorParam.split( QStringLiteral( ";" ) );
  }
  if ( !labelBufferSizeParam.isEmpty() )
  {
    labelBufferSize = labelBufferSizeParam.split( QStringLiteral( ";" ) );
  }
}

void QgsWmsConfigParser::removeHighlightLayers( const QStringList &layerIds )
{
  QStringList::const_iterator idIt = layerIds.constBegin();
  for ( ; idIt != layerIds.constEnd(); ++idIt )
  {
    QgsProject::instance()->removeMapLayers( QStringList() << *idIt );
  }
}

void QgsWmsConfigParser::setLayerIdsToLegendModel( QgsLegendModel *model, const QStringList &layerSet, double scale )
{
  if ( !model )
  {
    return;
  }

  // get model and layer tree root of the legend
  QgsLayerTree *root = model->rootGroup();


  // get layerIds find in the layer tree root
  QStringList layerIds = root->findLayerIds();

  // Q_FOREACH layer find in the layer tree
  // remove it if the layer id is not in map layerIds
  Q_FOREACH ( const QString &layerId, layerIds )
  {
    QgsLayerTreeLayer *nodeLayer = root->findLayer( layerId );
    if ( !nodeLayer )
    {
      continue;
    }
    if ( !layerSet.contains( layerId ) )
    {
      qobject_cast<QgsLayerTreeGroup *>( nodeLayer->parent() )->removeChildNode( nodeLayer );
    }
    else
    {
      QgsMapLayer *layer = nodeLayer->layer();
      if ( !layer->isInScaleRange( scale ) )
      {
        qobject_cast<QgsLayerTreeGroup *>( nodeLayer->parent() )->removeChildNode( nodeLayer );
      }
    }
  }
  root->removeChildrenGroupWithoutLayers();
}
