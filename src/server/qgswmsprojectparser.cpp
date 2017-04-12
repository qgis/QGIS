/***************************************************************************
                              qgswmsprojectparser.cpp
                              -----------------------
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

#include "qgswmsprojectparser.h"
#include "qgsconfigcache.h"
#include "qgsconfigparserutils.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmapserviceexception.h"
#include "qgspallabeling.h"
#include "qgsrenderer.h"
#include "qgsvectorlayer.h"
#include "qgsmapsettings.h"

#include "qgscomposition.h"
#include "qgscomposerarrow.h"
#include "qgscomposerlabel.h"
#include "qgscomposerlegend.h"
#include "qgscomposermap.h"
#include "qgscomposerframe.h"
#include "qgscomposerhtml.h"
#include "qgscomposerpicture.h"
#include "qgscomposerscalebar.h"
#include "qgscomposershape.h"
#include "qgslayertreeutils.h"
#include "qgslayertreegroup.h"
#include "qgslayertreelayer.h"
#include "qgslayertree.h"
#include "qgsaccesscontrol.h"

#include <QFileInfo>
#include <QTextDocument>

// style name to use for the unnamed style of layers (must not be empty name in WMS)
// this implies that a layer style called "default" will not be usable in WMS server
#define EMPTY_STYLE_NAME   "default"

QgsWmsProjectParser::QgsWmsProjectParser(
  const QString &filePath
  , const QgsAccessControl *accessControl
)
  : QgsWmsConfigParser()
  , mAccessControl( accessControl )
{
  mProjectParser = QgsConfigCache::instance()->serverConfiguration( filePath );
  mLegendLayerFont.fromString( mProjectParser->firstComposerLegendElement().attribute( QStringLiteral( "layerFont" ) ) );
  mLegendItemFont.fromString( mProjectParser->firstComposerLegendElement().attribute( QStringLiteral( "itemFont" ) ) );
  createTextAnnotationItems();
  createSvgAnnotationItems();
}

QgsWmsProjectParser::~QgsWmsProjectParser()
{
  cleanupTextAnnotationItems();
  cleanupSvgAnnotationItems();
  delete mProjectParser;
}

void QgsWmsProjectParser::layersAndStylesCapabilities( QDomElement &parentElement, QDomDocument &doc, const QString &version, const QString &serviceUrl, bool fullProjectSettings ) const
{
  QStringList nonIdentifiableLayers = identifyDisabledLayers();

  if ( mProjectParser->projectLayerElements().size() < 1 && mProjectParser->legendGroupElements().size() < 1 )
  {
    return;
  }

  QMap<QString, QgsMapLayer *> layerMap;
  mProjectParser->projectLayerMap( layerMap );

  //According to the WMS spec, there can be only one toplevel layer.
  //So we create an artificial one here to be in accordance with the schema
  QString projTitle = mProjectParser->projectTitle();
  QDomElement layerParentElem = doc.createElement( QStringLiteral( "Layer" ) );
  layerParentElem.setAttribute( QStringLiteral( "queryable" ), QStringLiteral( "1" ) );

  QDomElement layerParentNameElem = doc.createElement( QStringLiteral( "Name" ) );
  //WMS Name
  QDomElement nameElem = mProjectParser->propertiesElem().firstChildElement( QStringLiteral( "WMSRootName" ) );
  if ( !nameElem.isNull() && !nameElem.text().isEmpty() )
  {
    QDomText layerParentNameText = doc.createTextNode( nameElem.text() );
    layerParentNameElem.appendChild( layerParentNameText );
  }
  else
  {
    QDomText layerParentNameText = doc.createTextNode( projTitle );
    layerParentNameElem.appendChild( layerParentNameText );
  }
  layerParentElem.appendChild( layerParentNameElem );
  // Why not use WMSServiceTitle ?
  QDomElement layerParentTitleElem = doc.createElement( QStringLiteral( "Title" ) );
  QDomText layerParentTitleText = doc.createTextNode( projTitle );
  layerParentTitleElem.appendChild( layerParentTitleText );
  layerParentElem.appendChild( layerParentTitleElem );

  if ( fullProjectSettings )
  {
    // Layer tree name
    QDomElement treeNameElem = doc.createElement( QStringLiteral( "TreeName" ) );
    QDomText treeNameText = doc.createTextNode( projTitle );
    treeNameElem.appendChild( treeNameText );
    layerParentElem.appendChild( treeNameElem );
  }

  QDomElement legendElem = mProjectParser->legendElem();

  QHash<QString, QString> idNameMap;
  QStringList layerIDList;

  addLayers( doc, layerParentElem, legendElem, projectLayerTreeGroup(), layerMap, nonIdentifiableLayers, version, serviceUrl, fullProjectSettings, idNameMap, layerIDList );

  parentElement.appendChild( layerParentElem );
  mProjectParser->combineExtentAndCrsOfGroupChildren( layerParentElem, doc, true );

  if ( fullProjectSettings )
  {
    addDrawingOrder( parentElement, doc, idNameMap, layerIDList );
  }
}

QList<QgsMapLayer *> QgsWmsProjectParser::mapLayerFromStyle( const QString &lName, const QString &styleName, bool useCache ) const
{
  QMap< int, QgsMapLayer * > layers;

  //first check if the layer name refers an unpublished layer / group
  if ( mProjectParser->restrictedLayers().contains( lName ) )
  {
    return QList<QgsMapLayer *>();
  }

  // can't use layer cache if we are going to apply a non-default style
  if ( !styleName.isEmpty() && styleName != EMPTY_STYLE_NAME )
    useCache = false;

  //does lName refer to a leaf layer
  const QHash< QString, QDomElement > &projectLayerElements = mProjectParser->useLayerIds() ? mProjectParser->projectLayerElementsById() : mProjectParser->projectLayerElementsByName();
  QHash< QString, QDomElement >::const_iterator layerElemIt = projectLayerElements.find( lName );
  if ( layerElemIt != projectLayerElements.constEnd() )
  {
    QgsMapLayer *ml = mProjectParser->createLayerFromElement( layerElemIt.value(), useCache );
    if ( !styleName.isEmpty() && styleName != EMPTY_STYLE_NAME )
    {
      // try to apply the specified style
      if ( !ml->styleManager()->setCurrentStyle( styleName != EMPTY_STYLE_NAME ? styleName : QString() ) )
        throw QgsMapServiceException( QStringLiteral( "StyleNotDefined" ), QStringLiteral( "Style \"%1\" does not exist for layer \"%2\"" ).arg( styleName, lName ) );
    }
    return QList<QgsMapLayer *>() << ml;
  }

  //group or project name
  QDomElement groupElement;
  if ( lName == mProjectParser->projectTitle() )
  {
    groupElement = mProjectParser->legendElem();
  }
  else
  {
    QDomElement nameElem = mProjectParser->propertiesElem().firstChildElement( QStringLiteral( "WMSRootName" ) );
    if ( !nameElem.isNull() && lName == nameElem.text() )
    {
      groupElement = mProjectParser->legendElem();
    }
    else
    {
      const QList<QDomElement> &legendGroupElements = mProjectParser->legendGroupElements();
      QList<QDomElement>::const_iterator groupIt = legendGroupElements.constBegin();
      for ( ; groupIt != legendGroupElements.constEnd(); ++groupIt )
      {
        if ( groupIt->attribute( QStringLiteral( "name" ) ) == lName )
        {
          groupElement = *groupIt;
          break;
        }
        else if ( groupIt->attribute( QStringLiteral( "shortName" ) ) == lName )
        {
          groupElement = *groupIt;
          break;
        }
      }
    }
  }

  if ( !groupElement.isNull() )
  {
    addLayersFromGroup( groupElement, layers, useCache );
    return QgsConfigParserUtils::layerMapToList( layers, mProjectParser->updateLegendDrawingOrder() );
  }

  //still not found. Check if it is a single embedded layer (embedded layers are not contained in mProjectLayerElementsByName)
  QDomElement legendElement = mProjectParser->legendElem();
  QDomNodeList legendLayerList = legendElement.elementsByTagName( QStringLiteral( "legendlayer" ) );
  for ( int i = 0; i < legendLayerList.size(); ++i )
  {
    QDomElement legendLayerElem = legendLayerList.at( i ).toElement();
    if ( legendLayerElem.attribute( QStringLiteral( "name" ) ) == lName )
    {
      mProjectParser->layerFromLegendLayer( legendLayerElem, layers, useCache );
    }
  }

  //Still not found. Probably it is a layer or a subgroup in an embedded group
  //go through all groups
  //check if they are embedded
  //if yes, request leaf layers and groups from project parser
  const QList<QDomElement> &legendGroupElements = mProjectParser->legendGroupElements();
  QList<QDomElement>::const_iterator legendIt = legendGroupElements.constBegin();
  for ( ; legendIt != legendGroupElements.constEnd(); ++legendIt )
  {
    if ( legendIt->attribute( QStringLiteral( "embedded" ) ) == QLatin1String( "1" ) )
    {
      QString project = mProjectParser->convertToAbsolutePath( legendIt->attribute( QStringLiteral( "project" ) ) );
      QgsWmsProjectParser *p = dynamic_cast<QgsWmsProjectParser *>( QgsConfigCache::instance()->wmsConfiguration(
                                 project
                                 , mAccessControl
                               ) );
      if ( p )
      {
        QgsServerProjectParser *pp = p->mProjectParser;
        const QHash< QString, QDomElement > &pLayerByName = pp->projectLayerElementsByName();
        QHash< QString, QDomElement >::const_iterator pLayerNameIt = pLayerByName.find( lName );
        if ( pLayerNameIt != pLayerByName.constEnd() )
        {
          return ( QList<QgsMapLayer *>() << pp->createLayerFromElement( pLayerNameIt.value(), useCache ) );
        }

        const QList<QDomElement> &legendGroupElements = pp->legendGroupElements();
        QList<QDomElement>::const_iterator pLegendGroupIt = legendGroupElements.constBegin();
        for ( ; pLegendGroupIt != legendGroupElements.constEnd(); ++pLegendGroupIt )
        {
          if ( pLegendGroupIt->attribute( QStringLiteral( "name" ) ) == lName )
          {
            addLayersFromGroup( *pLegendGroupIt, layers, useCache );
            break;
          }
        }
      }
    }
  }

  if ( layers.isEmpty() )
    throw QgsMapServiceException( QStringLiteral( "LayerNotDefined" ), QStringLiteral( "Layer \"%1\" does not exist" ).arg( lName ) );

  return layers.values();
}

void QgsWmsProjectParser::addLayersFromGroup( const QDomElement &legendGroupElem, QMap< int, QgsMapLayer *> &layers, bool useCache ) const
{
  if ( legendGroupElem.isNull() )
  {
    return;
  }

  if ( legendGroupElem.attribute( QStringLiteral( "embedded" ) ) == QLatin1String( "1" ) ) //embedded group
  {
    QString groupName = legendGroupElem.attribute( QStringLiteral( "name" ) );
    int drawingOrder = mProjectParser->updateLegendDrawingOrder() ? legendGroupElem.attribute( QStringLiteral( "drawingOrder" ), QStringLiteral( "-1" ) ).toInt() : -1;

    QString project = mProjectParser->convertToAbsolutePath( legendGroupElem.attribute( QStringLiteral( "project" ) ) );
    QgsWmsProjectParser *p = dynamic_cast<QgsWmsProjectParser *>( QgsConfigCache::instance()->wmsConfiguration(
                               project
                               , mAccessControl
                             ) );
    if ( p )
    {
      QgsServerProjectParser *pp = p->mProjectParser;
      const QList<QDomElement> &legendGroups = pp->legendGroupElements();
      QList<QDomElement>::const_iterator legendIt = legendGroups.constBegin();
      for ( ; legendIt != legendGroups.constEnd(); ++legendIt )
      {
        if ( legendIt->attribute( QStringLiteral( "name" ) ) == groupName )
        {
          QMap< int, QgsMapLayer *> embeddedGroupLayers;
          p->addLayersFromGroup( *legendIt, embeddedGroupLayers, useCache );

          //reverse order because it will be reversed again afterwards in insertMulti
          QList< QgsMapLayer * > embeddedLayerList = QgsConfigParserUtils::layerMapToList( embeddedGroupLayers, pp->updateLegendDrawingOrder() );

          QList< QgsMapLayer * >::const_iterator layerIt = embeddedLayerList.constBegin();
          for ( ; layerIt != embeddedLayerList.constEnd(); ++layerIt )
          {
            layers.insertMulti( drawingOrder, *layerIt );
          }
        }
      }
    }
  }
  else //normal group
  {
    QMap< int, QDomElement > layerOrderList;
    QDomNodeList groupElemChildren = legendGroupElem.childNodes();
    // for rendering layers has to be add from bottom (end) to top (start)
    for ( int i = groupElemChildren.size() - 1; i >= 0 ; --i )
    {
      QDomElement elem = groupElemChildren.at( i ).toElement();
      if ( elem.tagName() == QLatin1String( "legendgroup" ) )
      {
        addLayersFromGroup( elem, layers, useCache );
      }
      else if ( elem.tagName() == QLatin1String( "legendlayer" ) )
      {
        mProjectParser->layerFromLegendLayer( elem, layers, useCache );
      }
    }
  }
}


QStringList QgsWmsProjectParser::wfsLayerNames() const
{
  return mProjectParser->wfsLayerNames();
}

double QgsWmsProjectParser::legendBoxSpace() const
{
  QDomElement legendElem = mProjectParser->firstComposerLegendElement();
  return legendElem.isNull() ? 2.0 : legendElem.attribute( QStringLiteral( "boxSpace" ) ).toDouble();
}

double QgsWmsProjectParser::legendLayerSpace() const
{
  QDomElement legendElem = mProjectParser->firstComposerLegendElement();
  return legendElem.isNull() ? 3.0 : legendElem.attribute( QStringLiteral( "layerSpace" ) ).toDouble();
}

double QgsWmsProjectParser::legendLayerTitleSpace() const
{
  QDomElement legendElem = mProjectParser->firstComposerLegendElement();
  return legendElem.isNull() ? 3.0 : legendElem.attribute( QStringLiteral( "layerTitleSpace" ) ).toDouble();
}

double QgsWmsProjectParser::legendSymbolSpace() const
{
  QDomElement legendElem = mProjectParser->firstComposerLegendElement();
  return legendElem.isNull() ? 2.0 : legendElem.attribute( QStringLiteral( "symbolSpace" ) ).toDouble();
}

double QgsWmsProjectParser::legendIconLabelSpace() const
{
  QDomElement legendElem = mProjectParser->firstComposerLegendElement();
  return legendElem.isNull() ? 2.0 : legendElem.attribute( QStringLiteral( "iconLabelSpace" ) ).toDouble();
}

double QgsWmsProjectParser::legendSymbolWidth() const
{
  QDomElement legendElem = mProjectParser->firstComposerLegendElement();
  return legendElem.isNull() ? 7.0 : legendElem.attribute( QStringLiteral( "symbolWidth" ) ).toDouble();
}

double QgsWmsProjectParser::legendSymbolHeight() const
{
  QDomElement legendElem = mProjectParser->firstComposerLegendElement();
  return legendElem.isNull() ? 4.0 : legendElem.attribute( QStringLiteral( "symbolHeight" ) ).toDouble();
}

QFont QgsWmsProjectParser::legendLayerFont() const
{
  return mLegendLayerFont;
}

QFont QgsWmsProjectParser::legendItemFont() const
{
  return mLegendItemFont;
}

double QgsWmsProjectParser::imageQuality() const
{
  double imageQuality = -1;
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( !propertiesElem.isNull() )
  {
    QDomElement imageQualityElem = propertiesElem.firstChildElement( QStringLiteral( "WMSImageQuality" ) );
    if ( !imageQualityElem.isNull() )
    {
      imageQuality = imageQualityElem.text().toInt();
    }
  }
  return imageQuality;
}

int QgsWmsProjectParser::wmsPrecision() const
{
  int WMSPrecision = -1;
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( !propertiesElem.isNull() )
  {
    QDomElement WMSPrecisionElem = propertiesElem.firstChildElement( QStringLiteral( "WMSPrecision" ) );
    if ( !WMSPrecisionElem.isNull() )
    {
      WMSPrecision = WMSPrecisionElem.text().toInt();
    }
  }
  return WMSPrecision;
}

bool QgsWmsProjectParser::wmsInspireActivated() const
{
  bool inspireActivated = false;
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( !propertiesElem.isNull() )
  {
    QDomElement inspireElem = propertiesElem.firstChildElement( QStringLiteral( "WMSInspire" ) );
    if ( !inspireElem.isNull() )
    {
      QDomElement inspireActivatedElem = inspireElem.firstChildElement( QStringLiteral( "activated" ) );
      if ( !inspireActivatedElem.isNull() )
      {
        inspireActivated = QVariant( inspireActivatedElem.text() ).toBool();
      }
    }
  }
  return inspireActivated;
}

QgsComposition *QgsWmsProjectParser::initComposition( const QString &composerTemplate, const QgsMapSettings &mapSettings, QList< QgsComposerMap * > &mapList, QList< QgsComposerLegend * > &legendList, QList< QgsComposerLabel * > &labelList, QList<const QgsComposerHtml *> &htmlList ) const
{
  //Create composition from xml
  QDomElement composerElem = composerByName( composerTemplate );
  if ( composerElem.isNull() )
  {
    throw QgsMapServiceException( QStringLiteral( "Error" ), QStringLiteral( "Composer template not found" ) );
  }

  QDomElement compositionElem = composerElem.firstChildElement( QStringLiteral( "Composition" ) );
  if ( compositionElem.isNull() )
  {
    return nullptr;
  }

  QgsComposition *composition = new QgsComposition( QgsProject::instance() ); //set resolution, paper size from composer element attributes
  if ( !composition->readXml( compositionElem, *( mProjectParser->xmlDocument() ) ) )
  {
    delete composition;
    return nullptr;
  }

  composition->addItemsFromXml( compositionElem, *( mProjectParser->xmlDocument() ) );

  labelList.clear();
  mapList.clear();
  legendList.clear();
  htmlList.clear();

  QList<QgsComposerItem * > itemList;
  composition->composerItems( itemList );
  QList<QgsComposerItem *>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    QgsComposerLabel *label = qobject_cast< QgsComposerLabel *>( *itemIt );
    if ( label )
    {
      labelList.push_back( label );
      continue;
    }
    QgsComposerMap *map = qobject_cast< QgsComposerMap *>( *itemIt );
    if ( map )
    {
      if ( !map->keepLayerSet() )
        map->setLayers( mapSettings.layers() );
      mapList.push_back( map );
      continue;
    }
    QgsComposerLegend *legend = qobject_cast< QgsComposerLegend *>( *itemIt );
    if ( legend )
    {
      QgsLegendModel *model = legend->model();
#if 0
      QgsLayerTreeGroup *root = model->rootGroup();
      QStringList layerIds = root->findLayerIds();
      throw QgsMapServiceException( "Error", "Composer legend layerIds " + layerIds.join( " ," ) );
#endif
      if ( legend->autoUpdateModel() )
      {
        model->setRootGroup( projectLayerTreeGroup() );
      }
      // if the legend has no map
      // we will load all layers
      const QgsComposerMap *map = legend->composerMap();
      if ( !map )
      {
        QgsLayerTree *root = model->rootGroup();
        QStringList layerIds = root->findLayerIds();
        // for each layer find in the layer tree
        // load it if the layer id is not QgsProject
        Q_FOREACH ( const QString &layerId, layerIds )
        {
          QgsMapLayer *layer = QgsProject::instance()->mapLayer( layerId );
          if ( layer )
          {
            continue;
          }

          QgsLayerTreeLayer *nodeLayer = root->findLayer( layerId );
          if ( !nodeLayer )
          {
            continue;
          }
          layer = nodeLayer->layer();
          if ( !layer )
          {
            const QHash< QString, QDomElement > &projectLayerElements = mProjectParser->projectLayerElementsById();
            QHash< QString, QDomElement >::const_iterator layerElemIt = projectLayerElements.find( layerId );
            if ( layerElemIt != projectLayerElements.constEnd() )
            {
              layer = mProjectParser->createLayerFromElement( layerElemIt.value(), true );
            }
          }
          QgsProject::instance()->addMapLayer( layer );
        }
        legend->updateLegend();
      }
      legendList.push_back( legend );
      continue;
    }
    QgsComposerPicture *pic = qobject_cast< QgsComposerPicture *>( *itemIt );
    if ( pic )
    {
      pic->setPicturePath( mProjectParser->convertToAbsolutePath( ( pic )->picturePath() ) );
      continue;
    }

    // an html item will be a composer frame and if it is we can try to get
    // its multiframe parent and then try to cast that to a composer html
    const QgsComposerFrame *frame = qobject_cast<const QgsComposerFrame *>( *itemIt );
    if ( frame )
    {
      const QgsComposerMultiFrame *multiFrame = frame->multiFrame();
      const QgsComposerHtml *composerHtml = qobject_cast<const QgsComposerHtml *>( multiFrame );
      if ( composerHtml )
      {
        htmlList.push_back( composerHtml );
        continue;
      }
    }
  }

  return composition;
}

void QgsWmsProjectParser::printCapabilities( QDomElement &parentElement, QDomDocument &doc ) const
{
  if ( !mProjectParser->xmlDocument() )
  {
    return;
  }

  QList<QDomElement> composerElemList = mProjectParser->publishedComposerElements();
  if ( composerElemList.size() < 1 )
  {
    return;
  }

  QDomElement composerTemplatesElem = doc.createElement( QStringLiteral( "ComposerTemplates" ) );

  QList<QDomElement>::const_iterator composerElemIt = composerElemList.constBegin();
  for ( ; composerElemIt != composerElemList.constEnd(); ++composerElemIt )
  {
    QDomElement composerTemplateElem = doc.createElement( QStringLiteral( "ComposerTemplate" ) );
    QDomElement currentComposerElem = *composerElemIt;
    if ( currentComposerElem.isNull() )
    {
      continue;
    }

    composerTemplateElem.setAttribute( QStringLiteral( "name" ), currentComposerElem.attribute( QStringLiteral( "title" ) ) );

    //get paper width and hight in mm from composition
    QDomElement compositionElem = currentComposerElem.firstChildElement( QStringLiteral( "Composition" ) );
    if ( compositionElem.isNull() )
    {
      continue;
    }
    composerTemplateElem.setAttribute( QStringLiteral( "width" ), compositionElem.attribute( QStringLiteral( "paperWidth" ) ) );
    composerTemplateElem.setAttribute( QStringLiteral( "height" ), compositionElem.attribute( QStringLiteral( "paperHeight" ) ) );


    //add available composer maps and their size in mm
    QDomNodeList composerMapList = currentComposerElem.elementsByTagName( QStringLiteral( "ComposerMap" ) );
    for ( int j = 0; j < composerMapList.size(); ++j )
    {
      QDomElement cmap = composerMapList.at( j ).toElement();
      QDomElement citem = cmap.firstChildElement( QStringLiteral( "ComposerItem" ) );
      if ( citem.isNull() )
      {
        continue;
      }

      QDomElement composerMapElem = doc.createElement( QStringLiteral( "ComposerMap" ) );
      composerMapElem.setAttribute( QStringLiteral( "name" ), "map" + cmap.attribute( QStringLiteral( "id" ) ) );
      composerMapElem.setAttribute( QStringLiteral( "width" ), citem.attribute( QStringLiteral( "width" ) ) );
      composerMapElem.setAttribute( QStringLiteral( "height" ), citem.attribute( QStringLiteral( "height" ) ) );
      composerTemplateElem.appendChild( composerMapElem );
    }

    //add available composer labels
    QDomNodeList composerLabelList = currentComposerElem.elementsByTagName( QStringLiteral( "ComposerLabel" ) );
    for ( int j = 0; j < composerLabelList.size(); ++j )
    {
      QDomElement citem = composerLabelList.at( j ).firstChildElement( QStringLiteral( "ComposerItem" ) );
      QString id = citem.attribute( QStringLiteral( "id" ) );
      if ( id.isEmpty() ) //only export labels with ids for text replacement
      {
        continue;
      }
      QDomElement composerLabelElem = doc.createElement( QStringLiteral( "ComposerLabel" ) );
      composerLabelElem.setAttribute( QStringLiteral( "name" ), id );
      composerTemplateElem.appendChild( composerLabelElem );
    }

    //add available composer HTML
    QDomNodeList composerHtmlList = currentComposerElem.elementsByTagName( QStringLiteral( "ComposerHtml" ) );
    for ( int j = 0; j < composerHtmlList.size(); ++j )
    {
      QDomElement citem = composerHtmlList.at( j ).firstChildElement( QStringLiteral( "ComposerFrame" ) ).firstChildElement( QStringLiteral( "ComposerItem" ) );
      QString id = citem.attribute( QStringLiteral( "id" ) );
      if ( id.isEmpty() ) //only export labels with ids for text replacement
      {
        continue;
      }
      QDomElement composerHtmlElem = doc.createElement( QStringLiteral( "ComposerHtml" ) );
      composerHtmlElem.setAttribute( QStringLiteral( "name" ), id );
      composerTemplateElem.appendChild( composerHtmlElem );
    }

    composerTemplatesElem.appendChild( composerTemplateElem );
  }
  parentElement.appendChild( composerTemplatesElem );
}

void QgsWmsProjectParser::inspireCapabilities( QDomElement &parentElement, QDomDocument &doc ) const
{
  if ( !wmsInspireActivated() )
    return;

  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return;
  }

  QDomElement inspireElem = propertiesElem.firstChildElement( QStringLiteral( "WMSInspire" ) );
  if ( inspireElem.isNull() )
  {
    return;
  }

  QDomElement inspireCapabilitiesElem = doc.createElement( QStringLiteral( "inspire_vs:ExtendedCapabilities" ) );

  QDomElement inspireMetadataUrlElem = inspireElem.firstChildElement( QStringLiteral( "metadataUrl" ) );
  if ( !inspireMetadataUrlElem.isNull() )
  {
    QDomElement inspireCommonMetadataUrlElem = doc.createElement( QStringLiteral( "inspire_common:MetadataUrl" ) );
    inspireCommonMetadataUrlElem.setAttribute( QStringLiteral( "xsi:type" ), QStringLiteral( "inspire_common:resourceLocatorType" ) );

    QDomElement inspireCommonMetadataUrlUrlElem = doc.createElement( QStringLiteral( "inspire_common:URL" ) );
    inspireCommonMetadataUrlUrlElem.appendChild( doc.createTextNode( inspireMetadataUrlElem.text() ) );
    inspireCommonMetadataUrlElem.appendChild( inspireCommonMetadataUrlUrlElem );

    QDomElement inspireMetadataUrlTypeElem = inspireElem.firstChildElement( QStringLiteral( "metadataUrlType" ) );
    if ( !inspireMetadataUrlTypeElem.isNull() )
    {
      QDomElement inspireCommonMetadataUrlMediaTypeElem = doc.createElement( QStringLiteral( "inspire_common:MediaType" ) );
      inspireCommonMetadataUrlMediaTypeElem.appendChild( doc.createTextNode( inspireMetadataUrlTypeElem.text() ) );
      inspireCommonMetadataUrlElem.appendChild( inspireCommonMetadataUrlMediaTypeElem );
    }

    inspireCapabilitiesElem.appendChild( inspireCommonMetadataUrlElem );
  }
  else
  {
    QDomElement inspireCommonResourceTypeElem = doc.createElement( QStringLiteral( "inspire_common:ResourceType" ) );
    inspireCommonResourceTypeElem.appendChild( doc.createTextNode( QStringLiteral( "service" ) ) );
    inspireCapabilitiesElem.appendChild( inspireCommonResourceTypeElem );

    QDomElement inspireCommonSpatialDataServiceTypeElem = doc.createElement( QStringLiteral( "inspire_common:SpatialDataServiceType" ) );
    inspireCommonSpatialDataServiceTypeElem.appendChild( doc.createTextNode( QStringLiteral( "view" ) ) );
    inspireCapabilitiesElem.appendChild( inspireCommonSpatialDataServiceTypeElem );

    QDomElement inspireTemporalReferenceElem = inspireElem.firstChildElement( QStringLiteral( "temporalReference" ) );
    if ( !inspireTemporalReferenceElem.isNull() )
    {
      QDomElement inspireCommonTemporalReferenceElem = doc.createElement( QStringLiteral( "inspire_common:TemporalReference" ) );
      QDomElement inspireCommonDateOfLastRevisionElem = doc.createElement( QStringLiteral( "inspire_common:DateOfLastRevision" ) );
      inspireCommonDateOfLastRevisionElem.appendChild( doc.createTextNode( inspireTemporalReferenceElem.text() ) );
      inspireCommonTemporalReferenceElem.appendChild( inspireCommonDateOfLastRevisionElem );
      inspireCapabilitiesElem.appendChild( inspireCommonTemporalReferenceElem );
    }

    QDomElement inspireCommonMetadataPointOfContactElem = doc.createElement( QStringLiteral( "inspire_common:MetadataPointOfContact" ) );

    QDomElement contactOrganizationElem = propertiesElem.firstChildElement( QStringLiteral( "WMSContactOrganization" ) );
    QDomElement inspireCommonOrganisationNameElem = doc.createElement( QStringLiteral( "inspire_common:OrganisationName" ) );
    if ( !contactOrganizationElem.isNull() )
    {
      inspireCommonOrganisationNameElem.appendChild( doc.createTextNode( contactOrganizationElem.text() ) );
    }
    inspireCommonMetadataPointOfContactElem.appendChild( inspireCommonOrganisationNameElem );

    QDomElement contactMailElem = propertiesElem.firstChildElement( QStringLiteral( "WMSContactMail" ) );
    QDomElement inspireCommonEmailAddressElem = doc.createElement( QStringLiteral( "inspire_common:EmailAddress" ) );
    if ( !contactMailElem.isNull() )
    {
      inspireCommonEmailAddressElem.appendChild( doc.createTextNode( contactMailElem.text() ) );
    }
    inspireCommonMetadataPointOfContactElem.appendChild( inspireCommonEmailAddressElem );

    inspireCapabilitiesElem.appendChild( inspireCommonMetadataPointOfContactElem );

    QDomElement inspireMetadataDateElem = inspireElem.firstChildElement( QStringLiteral( "metadataDate" ) );
    if ( !inspireMetadataDateElem.isNull() )
    {
      QDomElement inspireCommonMetadataDateElem = doc.createElement( QStringLiteral( "inspire_common:MetadataDate" ) );
      inspireCommonMetadataDateElem.appendChild( doc.createTextNode( inspireMetadataDateElem.text() ) );
      inspireCapabilitiesElem.appendChild( inspireCommonMetadataDateElem );
    }
  }

  QDomElement inspireLanguageElem = inspireElem.firstChildElement( QStringLiteral( "language" ) );
  if ( !inspireLanguageElem.isNull() )
  {
    QDomElement inspireCommonSupportedLanguagesElem = doc.createElement( QStringLiteral( "inspire_common:SupportedLanguages" ) );
    inspireCommonSupportedLanguagesElem.setAttribute( QStringLiteral( "xsi:type" ), QStringLiteral( "inspire_common:supportedLanguagesType" ) );

    QDomElement inspireCommonLanguageElem = doc.createElement( QStringLiteral( "inspire_common:Language" ) );
    inspireCommonLanguageElem.appendChild( doc.createTextNode( inspireLanguageElem.text() ) );

    QDomElement inspireCommonDefaultLanguageElem = doc.createElement( QStringLiteral( "inspire_common:DefaultLanguage" ) );
    inspireCommonDefaultLanguageElem.appendChild( inspireCommonLanguageElem );
    inspireCommonSupportedLanguagesElem.appendChild( inspireCommonDefaultLanguageElem );

#if 0
    /* Supported language has to be different from default one */
    QDomElement inspireCommonSupportedLanguageElem = doc.createElement( "inspire_common:SupportedLanguage" );
    inspireCommonSupportedLanguageElem.appendChild( inspireCommonLanguageElem.cloneNode().toElement() );
    inspireCommonSupportedLanguagesElem.appendChild( inspireCommonSupportedLanguageElem );
#endif

    inspireCapabilitiesElem.appendChild( inspireCommonSupportedLanguagesElem );

    QDomElement inspireCommonResponseLanguageElem = doc.createElement( QStringLiteral( "inspire_common:ResponseLanguage" ) );
    inspireCommonResponseLanguageElem.appendChild( inspireCommonLanguageElem.cloneNode().toElement() );
    inspireCapabilitiesElem.appendChild( inspireCommonResponseLanguageElem );
  }

  parentElement.appendChild( inspireCapabilitiesElem );
}

QList< QPair< QString, QgsDatumTransformStore::Entry > > QgsWmsProjectParser::layerCoordinateTransforms() const
{
  return mProjectParser->layerCoordinateTransforms();
}

void QgsWmsProjectParser::owsGeneralAndResourceList( QDomElement &parentElement, QDomDocument &doc, const QString &strHref ) const
{
  // set parentElement id
  QFileInfo projectFileInfo( mProjectParser->projectPath() );
  parentElement.setAttribute( QStringLiteral( "id" ), "ows-context-" + projectFileInfo.baseName() );

  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    QFile wmsService( QStringLiteral( "wms_metadata.xml" ) );
    if ( wmsService.open( QIODevice::ReadOnly ) )
    {
      QDomDocument externServiceDoc;
      QString parseError;
      int errorLineNo;
      if ( externServiceDoc.setContent( &wmsService, false, &parseError, &errorLineNo ) )
      {
        wmsService.close();
        QDomElement service = externServiceDoc.firstChildElement();
        parentElement.appendChild( service );
      }
    }
    return;
  }

  // OWSContext General element
  QDomElement generalElem = doc.createElement( QStringLiteral( "General" ) );

  QDomElement windowElem = doc.createElement( QStringLiteral( "Window" ) );
  windowElem.setAttribute( QStringLiteral( "height" ), QStringLiteral( "600" ) );
  windowElem.setAttribute( QStringLiteral( "width" ), QStringLiteral( "800" ) );
  generalElem.appendChild( windowElem );

  //WMS title
  //why not use project title ?
  QDomElement titleElem = propertiesElem.firstChildElement( QStringLiteral( "WMSServiceTitle" ) );
  if ( !titleElem.isNull() )
  {
    QDomElement wmsTitleElem = doc.createElement( QStringLiteral( "ows:Title" ) );
    QDomText wmsTitleText = doc.createTextNode( titleElem.text() );
    wmsTitleElem.appendChild( wmsTitleText );
    generalElem.appendChild( wmsTitleElem );
  }

  //WMS abstract
  QDomElement abstractElem = propertiesElem.firstChildElement( QStringLiteral( "WMSServiceAbstract" ) );
  if ( !abstractElem.isNull() )
  {
    QDomElement wmsAbstractElem = doc.createElement( QStringLiteral( "ows:Abstract" ) );
    QDomText wmsAbstractText = doc.createTextNode( abstractElem.text() );
    wmsAbstractElem.appendChild( wmsAbstractText );
    generalElem.appendChild( wmsAbstractElem );
  }

  //keyword list
  QDomElement keywordListElem = propertiesElem.firstChildElement( QStringLiteral( "WMSKeywordList" ) );
  if ( !keywordListElem.isNull() && !keywordListElem.text().isEmpty() )
  {
    bool siaFormat = featureInfoFormatSIA2045();

    QDomElement keywordsElem = doc.createElement( QStringLiteral( "ows:Keywords" ) );
    QDomNodeList keywordList = keywordListElem.elementsByTagName( QStringLiteral( "value" ) );
    for ( int i = 0; i < keywordList.size(); ++i )
    {
      QDomElement keywordElem = doc.createElement( QStringLiteral( "ows:Keyword" ) );
      QDomText keywordText = doc.createTextNode( keywordList.at( i ).toElement().text() );
      keywordElem.appendChild( keywordText );
      if ( siaFormat )
      {
        keywordElem.setAttribute( QStringLiteral( "vocabulary" ), QStringLiteral( "SIA_Geo405" ) );
      }
      keywordsElem.appendChild( keywordElem );
    }

    if ( !keywordList.isEmpty() )
    {
      generalElem.appendChild( keywordsElem );
    }
  }

  parentElement.appendChild( generalElem );

  // OWSContext ResourceList element
  QStringList nonIdentifiableLayers = identifyDisabledLayers();
  if ( mProjectParser->projectLayerElements().size() < 1 )
  {
    return;
  }

  QgsRectangle combinedBBox;
  QMap<QString, QgsMapLayer *> layerMap;
  mProjectParser->projectLayerMap( layerMap );

  QDomElement legendElem = mProjectParser->legendElem();

  QDomElement resourceListElem = doc.createElement( QStringLiteral( "ResourceList" ) );

  addOWSLayers( doc, resourceListElem, legendElem, layerMap, nonIdentifiableLayers, strHref, combinedBBox, QLatin1String( "" ) );

  parentElement.appendChild( resourceListElem );

  QgsRectangle mapRect = mProjectParser->mapRectangle();
  if ( !mapRect.isEmpty() )
  {
    combinedBBox = mapRect;
  }
  QgsCoordinateReferenceSystem projectCrs = mProjectParser->projectCrs();
  QDomElement bboxElem = doc.createElement( QStringLiteral( "ows:BoundingBox" ) );
  bboxElem.setAttribute( QStringLiteral( "crs" ), projectCrs.authid() );
  if ( projectCrs.hasAxisInverted() )
  {
    combinedBBox.invert();
  }
  QDomElement lowerCornerElem = doc.createElement( QStringLiteral( "ows:LowerCorner" ) );
  QDomText lowerCornerText = doc.createTextNode( QString::number( combinedBBox.xMinimum() ) + " " +  QString::number( combinedBBox.yMinimum() ) );
  lowerCornerElem.appendChild( lowerCornerText );
  bboxElem.appendChild( lowerCornerElem );
  QDomElement upperCornerElem = doc.createElement( QStringLiteral( "ows:UpperCorner" ) );
  QDomText upperCornerText = doc.createTextNode( QString::number( combinedBBox.xMaximum() ) + " " +  QString::number( combinedBBox.yMaximum() ) );
  upperCornerElem.appendChild( upperCornerText );
  bboxElem.appendChild( upperCornerElem );
  generalElem.appendChild( bboxElem );
}

QStringList QgsWmsProjectParser::identifyDisabledLayers() const
{
  QStringList disabledList;

  const QDomDocument *projectDoc = mProjectParser->xmlDocument();
  if ( !projectDoc )
  {
    return disabledList;
  }

  QDomElement qgisElem = projectDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return disabledList;
  }
  QDomElement propertiesElem = qgisElem.firstChildElement( QStringLiteral( "properties" ) );
  if ( propertiesElem.isNull() )
  {
    return disabledList;
  }
  QDomElement identifyElem = propertiesElem.firstChildElement( QStringLiteral( "Identify" ) );
  if ( identifyElem.isNull() )
  {
    return disabledList;
  }
  QDomElement disabledLayersElem = identifyElem.firstChildElement( QStringLiteral( "disabledLayers" ) );
  if ( disabledLayersElem.isNull() )
  {
    return disabledList;
  }
  QDomNodeList valueList = disabledLayersElem.elementsByTagName( QStringLiteral( "value" ) );
  for ( int i = 0; i < valueList.size(); ++i )
  {
    disabledList << valueList.at( i ).toElement().text();
  }
  return disabledList;
}

void QgsWmsProjectParser::addDrawingOrder( QDomElement &parentElem, QDomDocument &doc, const QHash<QString, QString> &idNameMap, const QStringList &layerIDList ) const
{
  QStringList layerList( mProjectParser->customLayerOrder() );

  if ( layerList.isEmpty() )
  {
    layerList = layerIDList;
  }

  if ( !mProjectParser->useLayerIds() )
  {
    int i = 0;
    while ( i < layerList.size() )
    {
      if ( idNameMap.contains( layerList[i] ) )
      {
        layerList[i] = idNameMap[ layerList[i] ];
        ++i;
      }
      else
      {
        QgsDebugMsg( "layer not found" );
        layerList.removeAt( i );
      }
    }
  }

  if ( !layerList.isEmpty() )
  {
    QStringList reversedList;
    reversedList.reserve( layerList.size() );
    for ( int i = layerList.size() - 1; i >= 0; --i )
      reversedList << layerList[ i ];

    QDomElement layerDrawingOrderElem = doc.createElement( QStringLiteral( "LayerDrawingOrder" ) );
    QDomText drawingOrderText = doc.createTextNode( reversedList.join( QStringLiteral( "," ) ) );
    layerDrawingOrderElem.appendChild( drawingOrderText );
    parentElem.appendChild( layerDrawingOrderElem );
  }
}

void QgsWmsProjectParser::addLayerStyles( QgsMapLayer *currentLayer, QDomDocument &doc, QDomElement &layerElem, const QString &version, const QString &serviceUrl ) const
{
  Q_FOREACH ( QString styleName, currentLayer->styleManager()->styles() )
  {
    if ( styleName.isEmpty() )
      styleName = EMPTY_STYLE_NAME;

    QDomElement styleElem = doc.createElement( QStringLiteral( "Style" ) );
    QDomElement styleNameElem = doc.createElement( QStringLiteral( "Name" ) );
    QDomText styleNameText = doc.createTextNode( styleName );
    styleNameElem.appendChild( styleNameText );
    QDomElement styleTitleElem = doc.createElement( QStringLiteral( "Title" ) );
    QDomText styleTitleText = doc.createTextNode( styleName );
    styleTitleElem.appendChild( styleTitleText );
    styleElem.appendChild( styleNameElem );
    styleElem.appendChild( styleTitleElem );

    // QString LegendURL for explicit layerbased GetLegendGraphic request
    QDomElement getLayerLegendGraphicElem = doc.createElement( QStringLiteral( "LegendURL" ) );
    QString hrefString = currentLayer->legendUrl();
    bool customHrefString;
    if ( !hrefString.isEmpty() )
    {
      customHrefString = true;
    }
    else
    {
      customHrefString = false;
      hrefString = serviceUrl;
    }
    if ( hrefString.isEmpty() )
    {
      hrefString = getCapaServiceUrl( doc );
    }
    if ( !hrefString.isEmpty() )
    {
      QStringList getLayerLegendGraphicFormats;
      if ( !customHrefString )
      {
        getLayerLegendGraphicFormats << QStringLiteral( "image/png" ); // << "jpeg" << "image/jpeg"

      }
      else
      {
        getLayerLegendGraphicFormats << currentLayer->legendUrlFormat();
      }

      for ( int i = 0; i < getLayerLegendGraphicFormats.size(); ++i )
      {
        QDomElement getLayerLegendGraphicFormatElem = doc.createElement( QStringLiteral( "Format" ) );
        QString getLayerLegendGraphicFormat = getLayerLegendGraphicFormats[i];
        QDomText getLayerLegendGraphicFormatText = doc.createTextNode( getLayerLegendGraphicFormat );
        getLayerLegendGraphicFormatElem.appendChild( getLayerLegendGraphicFormatText );
        getLayerLegendGraphicElem.appendChild( getLayerLegendGraphicFormatElem );
      }

      // no parameters on custom hrefUrl, because should link directly to graphic
      if ( !customHrefString )
      {
        QString layerName =  currentLayer->name();
        if ( mProjectParser && mProjectParser->useLayerIds() )
          layerName = currentLayer->id();
        else if ( !currentLayer->shortName().isEmpty() )
          layerName = currentLayer->shortName();
        QUrl mapUrl( hrefString );
        mapUrl.addQueryItem( QStringLiteral( "SERVICE" ), QStringLiteral( "WMS" ) );
        mapUrl.addQueryItem( QStringLiteral( "VERSION" ), version );
        mapUrl.addQueryItem( QStringLiteral( "REQUEST" ), QStringLiteral( "GetLegendGraphic" ) );
        mapUrl.addQueryItem( QStringLiteral( "LAYER" ), layerName );
        mapUrl.addQueryItem( QStringLiteral( "FORMAT" ), QStringLiteral( "image/png" ) );
        mapUrl.addQueryItem( QStringLiteral( "STYLE" ), styleNameText.data() );
        if ( version == QLatin1String( "1.3.0" ) )
        {
          mapUrl.addQueryItem( QStringLiteral( "SLD_VERSION" ), QStringLiteral( "1.1.0" ) );
        }
        hrefString = mapUrl.toString();
      }

      QDomElement getLayerLegendGraphicORElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
      getLayerLegendGraphicORElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
      getLayerLegendGraphicORElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
      getLayerLegendGraphicORElem.setAttribute( QStringLiteral( "xlink:href" ), hrefString );
      getLayerLegendGraphicElem.appendChild( getLayerLegendGraphicORElem );
      styleElem.appendChild( getLayerLegendGraphicElem );
    }
    layerElem.appendChild( styleElem );
  }
}


void QgsWmsProjectParser::addLayers( QDomDocument &doc,
                                     QDomElement &parentLayer,
                                     const QDomElement &legendElem,
                                     QgsLayerTreeGroup *layerTreeGroup,
                                     const QMap<QString, QgsMapLayer *> &layerMap,
                                     const QStringList &nonIdentifiableLayers,
                                     const QString &version, //1.1.1 or 1.3.0
                                     const QString &serviceUrl,
                                     bool fullProjectSettings,
                                     QHash<QString, QString> &idNameMap,
                                     QStringList &layerIDList ) const
{
  QDomNodeList legendChildren = legendElem.childNodes();
  QList< QgsLayerTreeNode * > layerTreeGroupChildren = layerTreeGroup->children();
  int g = 0; // index of the last child layer tree group
  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    QDomElement currentChildElem = legendChildren.at( i ).toElement();
    QDomElement layerElem = doc.createElement( QStringLiteral( "Layer" ) );
    if ( fullProjectSettings )
    {
      layerElem.setAttribute( QStringLiteral( "visible" ), currentChildElem.attribute( QStringLiteral( "checked" ) ) != QLatin1String( "Qt::Unchecked" ) );
    }

    if ( currentChildElem.tagName() == QLatin1String( "legendgroup" ) )
    {
      layerElem.setAttribute( QStringLiteral( "queryable" ), QStringLiteral( "1" ) );
      QString name = currentChildElem.attribute( QStringLiteral( "name" ) );
      if ( mProjectParser->restrictedLayers().contains( name ) ) //unpublished group
      {
        continue;
      }
      // find layer tree group
      QgsLayerTreeGroup *ltGroup = layerTreeGroup->findGroup( name );
      if ( layerTreeGroupChildren.length() >= g && layerTreeGroupChildren.length() <= i )
      {
        for ( int j = g; j < i + 1; ++j )
        {
          QgsLayerTreeNode *layerTreeChildNode = layerTreeGroupChildren.at( j );
          if ( layerTreeChildNode->nodeType() != QgsLayerTreeNode::NodeGroup )
            continue;
          QgsLayerTreeGroup *layerTreeChildGroup = static_cast<QgsLayerTreeGroup *>( layerTreeChildNode );
          if ( layerTreeChildGroup->name() != currentChildElem.attribute( QStringLiteral( "name" ) ) )
            continue;
          ltGroup = layerTreeChildGroup;
          g = j;
          break;
        }
      }

      if ( !ltGroup )
      {
        QgsDebugMsg( QString( "Skipping group %1, it could not be found" ).arg( name ) );
        continue;
      }
      QString shortName = ltGroup->customProperty( QStringLiteral( "wmsShortName" ) ).toString();
      QString title = ltGroup->customProperty( QStringLiteral( "wmsTitle" ) ).toString();

      QDomElement nameElem = doc.createElement( QStringLiteral( "Name" ) );
      QDomText nameText;
      if ( !shortName.isEmpty() )
        nameText = doc.createTextNode( shortName );
      else
        nameText = doc.createTextNode( name );
      nameElem.appendChild( nameText );
      layerElem.appendChild( nameElem );

      QDomElement titleElem = doc.createElement( QStringLiteral( "Title" ) );
      QDomText titleText;
      if ( !title.isEmpty() )
        titleText = doc.createTextNode( title );
      else
        titleText = doc.createTextNode( name );
      titleElem.appendChild( titleText );
      layerElem.appendChild( titleElem );

      QString abstract = ltGroup->customProperty( QStringLiteral( "wmsAbstract" ) ).toString();
      if ( !abstract.isEmpty() )
      {
        QDomElement abstractElem = doc.createElement( QStringLiteral( "Abstract" ) );
        QDomText abstractText = doc.createTextNode( abstract );
        abstractElem.appendChild( abstractText );
        layerElem.appendChild( abstractElem );
      }

      if ( fullProjectSettings )
      {
        // Layer tree name
        QDomElement treeNameElem = doc.createElement( QStringLiteral( "TreeName" ) );
        QDomText treeNameText = doc.createTextNode( name );
        treeNameElem.appendChild( treeNameText );
        layerElem.appendChild( treeNameElem );
      }

      if ( currentChildElem.attribute( QStringLiteral( "embedded" ) ) == QLatin1String( "1" ) )
      {
        //add layers from other project files and embed into this group
        QString project = mProjectParser->convertToAbsolutePath( currentChildElem.attribute( QStringLiteral( "project" ) ) );
        QgsDebugMsg( QString( "Project path: %1" ).arg( project ) );
        QString embeddedGroupName = currentChildElem.attribute( QStringLiteral( "name" ) );
        QgsWmsProjectParser *p = dynamic_cast<QgsWmsProjectParser *>( QgsConfigCache::instance()->wmsConfiguration(
                                   project
                                   , mAccessControl
                                 ) );
        if ( p )
        {
          QgsServerProjectParser *pp = p->mProjectParser;
          const QList<QDomElement> &embeddedGroupElements = pp->legendGroupElements();
          QgsLayerTreeGroup *embeddedLayerTreeGroup = p->projectLayerTreeGroup();
          QStringList pIdDisabled = p->identifyDisabledLayers();

          QDomElement embeddedGroupElem;
          Q_FOREACH ( const QDomElement &elem, embeddedGroupElements )
          {
            if ( elem.attribute( QStringLiteral( "name" ) ) == embeddedGroupName )
            {
              embeddedGroupElem = elem;
              break;
            }
          }

          QMap<QString, QgsMapLayer *> pLayerMap;
          const QList<QDomElement> &embeddedProjectLayerElements = pp->projectLayerElements();
          Q_FOREACH ( const QDomElement &elem, embeddedProjectLayerElements )
          {
            pLayerMap.insert( pp->layerId( elem ), pp->createLayerFromElement( elem ) );
          }

          p->addLayers( doc, layerElem, embeddedGroupElem, embeddedLayerTreeGroup->findGroup( name ), pLayerMap, pIdDisabled, version, serviceUrl, fullProjectSettings, idNameMap, layerIDList );
        }
      }
      else //normal (not embedded) legend group
      {
        addLayers( doc, layerElem, currentChildElem, ltGroup, layerMap, nonIdentifiableLayers, version, serviceUrl, fullProjectSettings, idNameMap, layerIDList );
      }

      // combine bounding boxes of children (groups/layers)
      mProjectParser->combineExtentAndCrsOfGroupChildren( layerElem, doc );
    }
    else if ( currentChildElem.tagName() == QLatin1String( "legendlayer" ) )
    {
      QString id = mProjectParser->layerIdFromLegendLayer( currentChildElem );

      if ( !layerMap.contains( id ) )
      {
        QgsDebugMsg( QString( "layer %1 not found in map - layer cache too small?" ).arg( id ) );
        continue;
      }

      QgsMapLayer *currentLayer = layerMap[ id ];
      if ( !currentLayer )
      {
        QgsDebugMsg( QString( "layer %1 not found" ).arg( id ) );
        continue;
      }

      QString layerName =  currentLayer->name();
      if ( mProjectParser->useLayerIds() )
        layerName = currentLayer->id();
      else if ( !currentLayer->shortName().isEmpty() )
        layerName = currentLayer->shortName();
      if ( mProjectParser->restrictedLayers().contains( layerName ) ) //unpublished layer
      {
        continue;
      }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( !mAccessControl->layerReadPermission( currentLayer ) )
      {
        continue;
      }
#endif

      // queryable layer
      if ( nonIdentifiableLayers.contains( currentLayer->id() ) )
      {
        layerElem.setAttribute( QStringLiteral( "queryable" ), QStringLiteral( "0" ) );
      }
      else
      {
        layerElem.setAttribute( QStringLiteral( "queryable" ), QStringLiteral( "1" ) );
      }

      QDomElement nameElem = doc.createElement( QStringLiteral( "Name" ) );
      QDomText nameText = doc.createTextNode( layerName );
      nameElem.appendChild( nameText );
      layerElem.appendChild( nameElem );

      layerIDList << id;
      idNameMap.insert( id, currentLayer->name() );

      QDomElement titleElem = doc.createElement( QStringLiteral( "Title" ) );
      QString titleName = currentLayer->title();
      if ( titleName.isEmpty() )
      {
        titleName = currentLayer->name();
      }
      QDomText titleText = doc.createTextNode( titleName );
      titleElem.appendChild( titleText );
      layerElem.appendChild( titleElem );

      QString abstract = currentLayer->abstract();
      if ( !abstract.isEmpty() )
      {
        QDomElement abstractElem = doc.createElement( QStringLiteral( "Abstract" ) );
        QDomText abstractText = doc.createTextNode( abstract );
        abstractElem.appendChild( abstractText );
        layerElem.appendChild( abstractElem );
      }

      //keyword list
      if ( !currentLayer->keywordList().isEmpty() )
      {
        QStringList keywordStringList = currentLayer->keywordList().split( QStringLiteral( "," ) );
        bool siaFormat = featureInfoFormatSIA2045();

        QDomElement keywordListElem = doc.createElement( QStringLiteral( "KeywordList" ) );
        for ( int i = 0; i < keywordStringList.size(); ++i )
        {
          QDomElement keywordElem = doc.createElement( QStringLiteral( "Keyword" ) );
          QDomText keywordText = doc.createTextNode( keywordStringList.at( i ).trimmed() );
          keywordElem.appendChild( keywordText );
          if ( siaFormat )
          {
            keywordElem.setAttribute( QStringLiteral( "vocabulary" ), QStringLiteral( "SIA_Geo405" ) );
          }
          keywordListElem.appendChild( keywordElem );
        }
        layerElem.appendChild( keywordListElem );
      }

      //vector layer without geometry
      bool geometryLayer = true;
      if ( currentLayer->type() == QgsMapLayer::VectorLayer )
      {
        QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( currentLayer );
        if ( vLayer )
        {
          if ( vLayer->wkbType() == QgsWkbTypes::NoGeometry )
          {
            geometryLayer = false;
          }
        }
      }

      //CRS
      if ( geometryLayer )
      {
        QStringList crsList = QgsConfigParserUtils::createCrsListForLayer( currentLayer );
        QgsConfigParserUtils::appendCrsElementsToLayer( layerElem, doc, crsList, mProjectParser->supportedOutputCrsList() );

        //Ex_GeographicBoundingBox
        QgsConfigParserUtils::appendLayerBoundingBoxes( layerElem, doc, currentLayer->extent(), currentLayer->crs(), crsList, mProjectParser->supportedOutputCrsList() );
      }

      // add details about supported styles of the layer
      addLayerStyles( currentLayer, doc, layerElem, version, serviceUrl );

      //min/max scale denominatormScaleBasedVisibility
      if ( currentLayer->hasScaleBasedVisibility() )
      {
        if ( version == QLatin1String( "1.1.1" ) )
        {
          double OGC_PX_M = 0.00028; // OGC reference pixel size in meter, also used by qgis
          double SCALE_TO_SCALEHINT = OGC_PX_M * sqrt( 2.0 );

          QDomElement scaleHintElem = doc.createElement( QStringLiteral( "ScaleHint" ) );
          scaleHintElem.setAttribute( QStringLiteral( "min" ), QString::number( currentLayer->minimumScale() * SCALE_TO_SCALEHINT ) );
          scaleHintElem.setAttribute( QStringLiteral( "max" ), QString::number( currentLayer->maximumScale() * SCALE_TO_SCALEHINT ) );
          layerElem.appendChild( scaleHintElem );
        }
        else
        {
          QString minScaleString = QString::number( currentLayer->minimumScale() );
          QDomElement minScaleElem = doc.createElement( QStringLiteral( "MinScaleDenominator" ) );
          QDomText minScaleText = doc.createTextNode( minScaleString );
          minScaleElem.appendChild( minScaleText );
          layerElem.appendChild( minScaleElem );

          QString maxScaleString = QString::number( currentLayer->maximumScale() );
          QDomElement maxScaleElem = doc.createElement( QStringLiteral( "MaxScaleDenominator" ) );
          QDomText maxScaleText = doc.createTextNode( maxScaleString );
          maxScaleElem.appendChild( maxScaleText );
          layerElem.appendChild( maxScaleElem );
        }
      }

      // layer attribution
      QString dataUrl = currentLayer->dataUrl();
      if ( !dataUrl.isEmpty() )
      {
        QDomElement dataUrlElem = doc.createElement( QStringLiteral( "DataURL" ) );
        QDomElement dataUrlFormatElem = doc.createElement( QStringLiteral( "Format" ) );
        QString dataUrlFormat = currentLayer->dataUrlFormat();
        QDomText dataUrlFormatText = doc.createTextNode( dataUrlFormat );
        dataUrlFormatElem.appendChild( dataUrlFormatText );
        dataUrlElem.appendChild( dataUrlFormatElem );
        QDomElement dataORElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
        dataORElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
        dataORElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
        dataORElem.setAttribute( QStringLiteral( "xlink:href" ), dataUrl );
        dataUrlElem.appendChild( dataORElem );
        layerElem.appendChild( dataUrlElem );
      }

      // layer attribution
      QString attribution = currentLayer->attribution();
      if ( !attribution.isEmpty() )
      {
        QDomElement attribElem = doc.createElement( QStringLiteral( "Attribution" ) );
        QDomElement attribTitleElem = doc.createElement( QStringLiteral( "Title" ) );
        QDomText attribText = doc.createTextNode( attribution );
        attribTitleElem.appendChild( attribText );
        attribElem.appendChild( attribTitleElem );
        QString attributionUrl = currentLayer->attributionUrl();
        if ( !attributionUrl.isEmpty() )
        {
          QDomElement attribORElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
          attribORElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
          attribORElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
          attribORElem.setAttribute( QStringLiteral( "xlink:href" ), attributionUrl );
          attribElem.appendChild( attribORElem );
        }
        layerElem.appendChild( attribElem );
      }

      // layer metadata URL
      QString metadataUrl = currentLayer->metadataUrl();
      if ( !metadataUrl.isEmpty() )
      {
        QDomElement metaUrlElem = doc.createElement( QStringLiteral( "MetadataURL" ) );
        QString metadataUrlType = currentLayer->metadataUrlType();
        if ( version == QLatin1String( "1.1.1" ) )
        {
          metaUrlElem.setAttribute( QStringLiteral( "type" ), metadataUrlType );
        }
        else if ( metadataUrlType == QLatin1String( "FGDC" ) )
        {
          metaUrlElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "FGDC:1998" ) );
        }
        else if ( metadataUrlType == QLatin1String( "TC211" ) )
        {
          metaUrlElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "ISO19115:2003" ) );
        }
        else
        {
          metaUrlElem.setAttribute( QStringLiteral( "type" ), metadataUrlType );
        }
        QString metadataUrlFormat = currentLayer->metadataUrlFormat();
        if ( !metadataUrlFormat.isEmpty() )
        {
          QDomElement metaUrlFormatElem = doc.createElement( QStringLiteral( "Format" ) );
          QDomText metaUrlFormatText = doc.createTextNode( metadataUrlFormat );
          metaUrlFormatElem.appendChild( metaUrlFormatText );
          metaUrlElem.appendChild( metaUrlFormatElem );
        }
        QDomElement metaUrlORElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
        metaUrlORElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
        metaUrlORElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
        metaUrlORElem.setAttribute( QStringLiteral( "xlink:href" ), metadataUrl );
        metaUrlElem.appendChild( metaUrlORElem );
        layerElem.appendChild( metaUrlElem );
      }

      if ( fullProjectSettings )
      {
        mProjectParser->addLayerProjectSettings( layerElem, doc, currentLayer );
      }
    }
    else
    {
      QgsDebugMsg( "unexpected child element" );
      continue;
    }

    parentLayer.appendChild( layerElem );
  }
}


void QgsWmsProjectParser::addOWSLayerStyles( QgsMapLayer *currentLayer, QDomDocument &doc, QDomElement &layerElem ) const
{
  Q_FOREACH ( QString styleName, currentLayer->styleManager()->styles() )
  {
    if ( styleName.isEmpty() )
      styleName = EMPTY_STYLE_NAME;

    QDomElement styleListElem = doc.createElement( QStringLiteral( "StyleList" ) );
    //only one default style in project file mode
    QDomElement styleElem = doc.createElement( QStringLiteral( "Style" ) );
    styleElem.setAttribute( QStringLiteral( "current" ), QStringLiteral( "true" ) );
    QDomElement styleNameElem = doc.createElement( QStringLiteral( "Name" ) );
    QDomText styleNameText = doc.createTextNode( styleName );
    styleNameElem.appendChild( styleNameText );
    QDomElement styleTitleElem = doc.createElement( QStringLiteral( "Title" ) );
    QDomText styleTitleText = doc.createTextNode( styleName );
    styleTitleElem.appendChild( styleTitleText );
    styleElem.appendChild( styleNameElem );
    styleElem.appendChild( styleTitleElem );
    styleListElem.appendChild( styleElem );
    layerElem.appendChild( styleListElem );
  }
}


void QgsWmsProjectParser::addOWSLayers( QDomDocument &doc,
                                        QDomElement &parentElem,
                                        const QDomElement &legendElem,
                                        const QMap<QString, QgsMapLayer *> &layerMap,
                                        const QStringList &nonIdentifiableLayers,
                                        const QString &strHref,
                                        QgsRectangle &combinedBBox,
                                        const QString &strGroup ) const
{
  QgsCoordinateReferenceSystem projectCrs = mProjectParser->projectCrs();
  QDomNodeList legendChildren = legendElem.childNodes();
  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    QDomElement currentChildElem = legendChildren.at( i ).toElement();

    if ( currentChildElem.tagName() == QLatin1String( "legendgroup" ) )
    {
      QString name = currentChildElem.attribute( QStringLiteral( "name" ) );
      if ( mProjectParser->restrictedLayers().contains( name ) ) //unpublished group
      {
        continue;
      }
      QString group;
      if ( strGroup.isEmpty() )
      {
        group = name;
      }
      else
      {
        group = strGroup + "/" + name;
      }

      if ( currentChildElem.attribute( QStringLiteral( "embedded" ) ) == QLatin1String( "1" ) )
      {
        //add layers from other project files and embed into this group
        QString project = mProjectParser->convertToAbsolutePath( currentChildElem.attribute( QStringLiteral( "project" ) ) );
        QgsDebugMsg( QString( "Project path: %1" ).arg( project ) );
        QString embeddedGroupName = currentChildElem.attribute( QStringLiteral( "name" ) );
        QgsWmsProjectParser *p = dynamic_cast<QgsWmsProjectParser *>( QgsConfigCache::instance()->wmsConfiguration(
                                   project
                                   , mAccessControl
                                 ) );
        if ( p )
        {
          QgsServerProjectParser *pp = p->mProjectParser;
          const QList<QDomElement> &embeddedGroupElements = pp->legendGroupElements();
          QStringList pIdDisabled = p->identifyDisabledLayers();

          QDomElement embeddedGroupElem;
          Q_FOREACH ( const QDomElement &elem, embeddedGroupElements )
          {
            if ( elem.attribute( QStringLiteral( "name" ) ) == embeddedGroupName )
            {
              embeddedGroupElem = elem;
              break;
            }
          }

          QMap<QString, QgsMapLayer *> pLayerMap;
          const QList<QDomElement> &embeddedProjectLayerElements = pp->projectLayerElements();
          Q_FOREACH ( const QDomElement &elem, embeddedProjectLayerElements )
          {
            pLayerMap.insert( pp->layerId( elem ), pp->createLayerFromElement( elem ) );
          }

          p->addOWSLayers( doc, parentElem, embeddedGroupElem, pLayerMap, pIdDisabled, strHref, combinedBBox, group );
        }
      }
      else //normal (not embedded) legend group
      {
        addOWSLayers( doc, parentElem, currentChildElem, layerMap, nonIdentifiableLayers, strHref, combinedBBox, group );
      }

      // combine bounding boxes of children (groups/layers)
      // combineExtentAndCrsOfGroupChildren( layerElem, doc );
    }
    else if ( currentChildElem.tagName() == QLatin1String( "legendlayer" ) )
    {
      QDomElement layerElem = doc.createElement( QStringLiteral( "Layer" ) );
      QString id = mProjectParser->layerIdFromLegendLayer( currentChildElem );

      if ( !layerMap.contains( id ) )
      {
        QgsDebugMsg( QString( "layer %1 not found in map - layer cache to small?" ).arg( id ) );
        continue;
      }

      QgsMapLayer *currentLayer = layerMap[ id ];
      if ( !currentLayer )
      {
        QgsDebugMsg( QString( "layer %1 not found" ).arg( id ) );
        continue;
      }

      QString layerName =  currentLayer->name();
      if ( mProjectParser->useLayerIds() )
        layerName = currentLayer->id();
      else if ( !currentLayer->shortName().isEmpty() )
        layerName = currentLayer->shortName();
      if ( mProjectParser->restrictedLayers().contains( layerName ) ) //unpublished layer
      {
        continue;
      }
      if ( nonIdentifiableLayers.contains( currentLayer->id() ) )
      {
        layerElem.setAttribute( QStringLiteral( "queryable" ), QStringLiteral( "false" ) );
      }
      else
      {
        layerElem.setAttribute( QStringLiteral( "queryable" ), QStringLiteral( "true" ) );
      }

      // is the layer visible ?
      if ( currentChildElem.firstChildElement().firstChildElement().attribute( QStringLiteral( "visible" ) ) == QLatin1String( "1" ) )
      {
        layerElem.setAttribute( QStringLiteral( "hidden" ), QStringLiteral( "false" ) );
      }
      else
      {
        layerElem.setAttribute( QStringLiteral( "hidden" ), QStringLiteral( "true" ) );
      }

      if ( !strGroup.isEmpty() )
      {
        layerElem.setAttribute( QStringLiteral( "group" ), strGroup );
      }
      // Because Layer transparency is used for the rendering
      // OWSContext Layer opacity is set to 1
      layerElem.setAttribute( QStringLiteral( "opacity" ), 1 );

      QString lyrname =  currentLayer->name();
      if ( mProjectParser->useLayerIds() )
        lyrname = currentLayer->id();
      else if ( !currentLayer->shortName().isEmpty() )
        lyrname = currentLayer->shortName();
      layerElem.setAttribute( QStringLiteral( "name" ), lyrname );

      // define an id based on layer name
      layerElem.setAttribute( QStringLiteral( "id" ), lyrname.replace( QRegExp( "[\\W]" ), QStringLiteral( "_" ) ) );

      QDomElement titleElem = doc.createElement( QStringLiteral( "ows:Title" ) );
      QString titleName = currentLayer->title();
      if ( titleName.isEmpty() )
      {
        titleName = currentLayer->name();
      }
      QDomText titleText = doc.createTextNode( titleName );
      titleElem.appendChild( titleText );
      layerElem.appendChild( titleElem );

      QDomElement formatElem = doc.createElement( QStringLiteral( "ows:OutputFormat" ) );
      QDomText formatText = doc.createTextNode( QStringLiteral( "image/png" ) );
      formatElem.appendChild( formatText );
      layerElem.appendChild( formatElem );

      QDomElement serverElem = doc.createElement( QStringLiteral( "Server" ) );
      serverElem.setAttribute( QStringLiteral( "service" ), QStringLiteral( "urn:ogc:serviceType:WMS" ) );
      serverElem.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.3.0" ) );
      serverElem.setAttribute( QStringLiteral( "default" ), QStringLiteral( "true" ) );
      QDomElement orServerElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
      orServerElem.setAttribute( QStringLiteral( "xlink:href" ), strHref );
      serverElem.appendChild( orServerElem );
      layerElem.appendChild( serverElem );

      QString abstract = currentLayer->abstract();
      if ( !abstract.isEmpty() )
      {
        QDomElement abstractElem = doc.createElement( QStringLiteral( "ows:Abstract" ) );
        QDomText abstractText = doc.createTextNode( abstract );
        abstractElem.appendChild( abstractText );
        layerElem.appendChild( abstractElem );
      }

      //min/max scale denominatormScaleBasedVisibility
      if ( currentLayer->hasScaleBasedVisibility() )
      {
        QString minScaleString = QString::number( currentLayer->minimumScale() );
        QString maxScaleString = QString::number( currentLayer->maximumScale() );
        QDomElement minScaleElem = doc.createElement( QStringLiteral( "sld:MinScaleDenominator" ) );
        QDomText minScaleText = doc.createTextNode( minScaleString );
        minScaleElem.appendChild( minScaleText );
        layerElem.appendChild( minScaleElem );
        QDomElement maxScaleElem = doc.createElement( QStringLiteral( "sld:MaxScaleDenominator" ) );
        QDomText maxScaleText = doc.createTextNode( maxScaleString );
        maxScaleElem.appendChild( maxScaleText );
        layerElem.appendChild( maxScaleElem );
      }

      /*
      //CRS
      QStringList crsList = createCRSListForLayer( currentLayer );
      appendCRSElementsToLayer( layerElem, doc, crsList );

      //Ex_GeographicBoundingBox
      appendLayerBoundingBoxes( layerElem, doc, currentLayer->extent(), currentLayer->crs() );
      */
      //get project crs
      QgsCoordinateReferenceSystem layerCrs = currentLayer->crs();
      QgsCoordinateTransform t( layerCrs, projectCrs );

      //transform
      QgsRectangle BBox = t.transformBoundingBox( currentLayer->extent() );
      if ( combinedBBox.isEmpty() )
      {
        combinedBBox = BBox;
      }
      else
      {
        combinedBBox.combineExtentWith( BBox );
      }

      addOWSLayerStyles( currentLayer, doc, layerElem );

      //keyword list
      if ( !currentLayer->keywordList().isEmpty() )
      {
        QStringList keywordStringList = currentLayer->keywordList().split( QStringLiteral( "," ) );
        bool siaFormat = featureInfoFormatSIA2045();

        QDomElement keywordsElem = doc.createElement( QStringLiteral( "ows:Keywords" ) );
        for ( int i = 0; i < keywordStringList.size(); ++i )
        {
          QDomElement keywordElem = doc.createElement( QStringLiteral( "ows:Keyword" ) );
          QDomText keywordText = doc.createTextNode( keywordStringList.at( i ).trimmed() );
          keywordElem.appendChild( keywordText );
          if ( siaFormat )
          {
            keywordElem.setAttribute( QStringLiteral( "vocabulary" ), QStringLiteral( "SIA_Geo405" ) );
          }
          keywordsElem.appendChild( keywordElem );
        }
        layerElem.appendChild( keywordsElem );
      }

      // layer data URL
      QString dataUrl = currentLayer->dataUrl();
      if ( !dataUrl.isEmpty() )
      {
        QDomElement dataUrlElem = doc.createElement( QStringLiteral( "DataURL" ) );
        QString dataUrlFormat = currentLayer->dataUrlFormat();
        dataUrlElem.setAttribute( QStringLiteral( "format" ), dataUrlFormat );
        QDomElement dataORElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
        dataORElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
        dataORElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
        dataORElem.setAttribute( QStringLiteral( "xlink:href" ), dataUrl );
        dataUrlElem.appendChild( dataORElem );
        layerElem.appendChild( dataUrlElem );
      }

      // layer metadata URL
      QString metadataUrl = currentLayer->metadataUrl();
      if ( !metadataUrl.isEmpty() )
      {
        QDomElement metaUrlElem = doc.createElement( QStringLiteral( "MetadataURL" ) );
        QString metadataUrlFormat = currentLayer->metadataUrlFormat();
        metaUrlElem.setAttribute( QStringLiteral( "format" ), metadataUrlFormat );
        QDomElement metaUrlORElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
        metaUrlORElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
        metaUrlORElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
        metaUrlORElem.setAttribute( QStringLiteral( "xlink:href" ), metadataUrl );
        metaUrlElem.appendChild( metaUrlORElem );
        layerElem.appendChild( metaUrlElem );
      }

      if ( parentElem.hasChildNodes() )
      {
        parentElem.insertBefore( layerElem, parentElem.firstChild() );
      }
      else
      {
        parentElem.appendChild( layerElem );
      }
    }
    else
    {
      QgsDebugMsg( "unexpected child element" );
      continue;
    }

  }
}

int QgsWmsProjectParser::layersAndStyles( QStringList &layers, QStringList &styles ) const
{
  layers.clear();
  styles.clear();

  const QList<QDomElement> &projectLayerElements = mProjectParser->projectLayerElements();
  QList<QDomElement>::const_iterator elemIt = projectLayerElements.constBegin();

  QString currentLayerName;

  for ( ; elemIt != projectLayerElements.constEnd(); ++elemIt )
  {
    currentLayerName = mProjectParser->layerShortName( *elemIt );
    if ( currentLayerName.isEmpty() )
      currentLayerName = mProjectParser->layerName( *elemIt );
    if ( !currentLayerName.isEmpty() )
    {
      layers << currentLayerName;
      styles << QString();
    }
  }
  return 0;
}

QDomDocument QgsWmsProjectParser::getStyle( const QString &styleName, const QString &layerName ) const
{
  Q_UNUSED( styleName );
  QStringList layerList;
  layerList.append( layerName );
  return getStyles( layerList );
}

QDomDocument QgsWmsProjectParser::getStyles( QStringList &layerList ) const
{
  QDomDocument myDocument = QDomDocument();

  QDomNode header = myDocument.createProcessingInstruction( QStringLiteral( "xml" ), QStringLiteral( "version=\"1.0\" encoding=\"UTF-8\"" ) );
  myDocument.appendChild( header );

  // Create the root element
  QDomElement root = myDocument.createElementNS( QStringLiteral( "http://www.opengis.net/sld" ), QStringLiteral( "StyledLayerDescriptor" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.1.0" ) );
  root.setAttribute( QStringLiteral( "xsi:schemaLocation" ), QStringLiteral( "http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd" ) );
  root.setAttribute( QStringLiteral( "xmlns:ogc" ), QStringLiteral( "http://www.opengis.net/ogc" ) );
  root.setAttribute( QStringLiteral( "xmlns:se" ), QStringLiteral( "http://www.opengis.net/se" ) );
  root.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
  root.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
  myDocument.appendChild( root );

  for ( int i = 0; i < layerList.size(); i++ )
  {
    QString layerName;
    layerName = layerList.at( i );
    // don't use a cache - we may be changing styles
    QList<QgsMapLayer *> currentLayerList = mapLayerFromStyle( layerName, QLatin1String( "" ), false );
    if ( currentLayerList.size() < 1 )
    {
      throw QgsMapServiceException( QStringLiteral( "Error" ), QStringLiteral( "The layer for the TypeName '%1' is not found" ).arg( layerName ) );
    }
    for ( int j = 0; j < currentLayerList.size(); j++ )
    {
      QgsMapLayer *currentLayer = currentLayerList.at( j );
      QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( currentLayer );
      if ( !layer )
      {
        throw QgsMapServiceException( QStringLiteral( "Error" ), QStringLiteral( "Could not get style because:\n%1" ).arg( QStringLiteral( "Non-vector layers not supported yet" ) ) );
      }
      // Create the NamedLayer element
      QDomElement namedLayerNode = myDocument.createElement( QStringLiteral( "NamedLayer" ) );
      root.appendChild( namedLayerNode );

      // store the Name element
      QDomElement nameNode = myDocument.createElement( QStringLiteral( "se:Name" ) );
      nameNode.appendChild( myDocument.createTextNode( layerName ) );
      namedLayerNode.appendChild( nameNode );

      Q_FOREACH ( QString styleName, layer->styleManager()->styles() )
      {
        if ( layer->hasGeometryType() )
        {
          layer->styleManager()->setCurrentStyle( styleName );
          if ( styleName.isEmpty() )
            styleName = EMPTY_STYLE_NAME;
          QDomElement styleElem = layer->renderer()->writeSld( myDocument, styleName );
          namedLayerNode.appendChild( styleElem );
        }
      }
    }
  }
  return myDocument;
}

QDomDocument QgsWmsProjectParser::describeLayer( QStringList &layerList, const QString &wfsHrefString, const QString &wcsHrefString ) const
{
  QDomDocument myDocument = QDomDocument();

  QDomNode header = myDocument.createProcessingInstruction( QStringLiteral( "xml" ), QStringLiteral( "version=\"1.0\" encoding=\"UTF-8\"" ) );
  myDocument.appendChild( header );

  // Create the root element
  QDomElement root = myDocument.createElementNS( QStringLiteral( "http://www.opengis.net/sld" ), QStringLiteral( "DescribeLayerResponse" ) );
  root.setAttribute( QStringLiteral( "xsi:schemaLocation" ), QStringLiteral( "http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/DescribeLayer.xsd" ) );
  root.setAttribute( QStringLiteral( "xmlns:ows" ), QStringLiteral( "http://www.opengis.net/ows" ) );
  root.setAttribute( QStringLiteral( "xmlns:se" ), QStringLiteral( "http://www.opengis.net/se" ) );
  root.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
  root.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
  myDocument.appendChild( root );

  // store the Version element
  QDomElement versionNode = myDocument.createElement( QStringLiteral( "Version" ) );
  versionNode.appendChild( myDocument.createTextNode( QStringLiteral( "1.1.0" ) ) );
  root.appendChild( versionNode );

  //WFS layers
  QStringList wfsLayers = wfsLayerNames();
  //WCS layers
  QStringList wcsLayers = mProjectParser->wcsLayerNames();

  for ( int i = 0; i < layerList.size(); i++ )
  {
    QString layerName;
    layerName = layerList.at( i );
    // don't use a cache - we may be changing styles
    QList<QgsMapLayer *> currentLayerList = mapLayerFromStyle( layerName, QLatin1String( "" ), false );
    if ( currentLayerList.size() < 1 )
    {
      throw QgsMapServiceException( QStringLiteral( "InvalidParameterValue" ), QStringLiteral( "The layer '%1' is not found" ).arg( layerName ) );
    }
    for ( int j = 0; j < currentLayerList.size(); j++ )
    {
      QgsMapLayer *currentLayer = currentLayerList.at( j );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( !mAccessControl->layerReadPermission( currentLayer ) )
      {
        throw QgsMapServiceException( QStringLiteral( "Security" ), QStringLiteral( "You are not allowed to access to this layer" ) );
      }
#endif

      QString layerTypeName =  currentLayer->name();
      if ( mProjectParser && mProjectParser->useLayerIds() )
        layerTypeName = currentLayer->id();
      else if ( !currentLayer->shortName().isEmpty() )
        layerTypeName = currentLayer->shortName();

      // Create the NamedLayer element
      QDomElement layerNode = myDocument.createElement( QStringLiteral( "LayerDescription" ) );
      root.appendChild( layerNode );

      // store the owsType element
      QDomElement typeNode = myDocument.createElement( QStringLiteral( "owsType" ) );
      // store the se:OnlineResource element
      QDomElement oResNode = myDocument.createElement( QStringLiteral( "se:OnlineResource" ) );
      oResNode.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
      // store the TypeName element
      QDomElement nameNode = myDocument.createElement( QStringLiteral( "TypeName" ) );
      if ( currentLayer->type() == QgsMapLayer::VectorLayer )
      {
        typeNode.appendChild( myDocument.createTextNode( QStringLiteral( "wfs" ) ) );

        if ( wfsLayers.indexOf( layerTypeName ) != -1 )
        {
          oResNode.setAttribute( QStringLiteral( "xlink:href" ), wfsHrefString );
        }

        // store the se:FeatureTypeName element
        QDomElement typeNameNode = myDocument.createElement( QStringLiteral( "se:FeatureTypeName" ) );
        typeNameNode.appendChild( myDocument.createTextNode( layerTypeName ) );
        nameNode.appendChild( typeNameNode );
      }
      else if ( currentLayer->type() == QgsMapLayer::RasterLayer )
      {
        typeNode.appendChild( myDocument.createTextNode( QStringLiteral( "wcs" ) ) );

        if ( wcsLayers.indexOf( layerTypeName ) != -1 )
        {
          oResNode.setAttribute( QStringLiteral( "xlink:href" ), wcsHrefString );
        }

        // store the se:CoverageTypeName element
        QDomElement typeNameNode = myDocument.createElement( QStringLiteral( "se:CoverageTypeName" ) );
        typeNameNode.appendChild( myDocument.createTextNode( layerTypeName ) );
        nameNode.appendChild( typeNameNode );
      }
      layerNode.appendChild( typeNode );
      layerNode.appendChild( oResNode );
      layerNode.appendChild( nameNode );

    }
  }

  return myDocument;
}

QgsUnitTypes::RenderUnit QgsWmsProjectParser::outputUnits() const
{
  return QgsUnitTypes::RenderUnit::RenderMillimeters;
}

bool QgsWmsProjectParser::featureInfoWithWktGeometry() const
{
  if ( !mProjectParser->xmlDocument() )
  {
    return false;
  }

  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return false;
  }
  QDomElement wktElem = propertiesElem.firstChildElement( QStringLiteral( "WMSAddWktGeometry" ) );
  if ( wktElem.isNull() )
  {
    return false;
  }

  return ( wktElem.text().compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 );
}

bool QgsWmsProjectParser::segmentizeFeatureInfoWktGeometry() const
{
  if ( !mProjectParser->xmlDocument() )
  {
    return false;
  }

  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return false;
  }

  QDomElement segmentizeElem = propertiesElem.firstChildElement( QStringLiteral( "WMSSegmentizeFeatureInfoGeometry" ) );
  if ( segmentizeElem.isNull() )
  {
    return false;
  }

  return ( segmentizeElem.text().compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 );
}

QHash<QString, QString> QgsWmsProjectParser::featureInfoLayerAliasMap() const
{
  QHash<QString, QString> aliasMap;
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return aliasMap;
  }

  //WMSFeatureInfoAliasLayers
  QStringList aliasLayerStringList;
  QDomElement featureInfoAliasLayersElem = propertiesElem.firstChildElement( QStringLiteral( "WMSFeatureInfoAliasLayers" ) );
  if ( featureInfoAliasLayersElem.isNull() )
  {
    return aliasMap;
  }
  QDomNodeList aliasLayerValueList = featureInfoAliasLayersElem.elementsByTagName( QStringLiteral( "value" ) );
  for ( int i = 0; i < aliasLayerValueList.size(); ++i )
  {
    aliasLayerStringList << aliasLayerValueList.at( i ).toElement().text();
  }

  //WMSFeatureInfoLayerAliases
  QStringList layerAliasStringList;
  QDomElement featureInfoLayerAliasesElem = propertiesElem.firstChildElement( QStringLiteral( "WMSFeatureInfoLayerAliases" ) );
  if ( featureInfoLayerAliasesElem.isNull() )
  {
    return aliasMap;
  }
  QDomNodeList layerAliasesValueList = featureInfoLayerAliasesElem.elementsByTagName( QStringLiteral( "value" ) );
  for ( int i = 0; i < layerAliasesValueList.size(); ++i )
  {
    layerAliasStringList << layerAliasesValueList.at( i ).toElement().text();
  }

  int nMapEntries = qMin( aliasLayerStringList.size(), layerAliasStringList.size() );
  for ( int i = 0; i < nMapEntries; ++i )
  {
    aliasMap.insert( aliasLayerStringList.at( i ), layerAliasStringList.at( i ) );
  }

  return aliasMap;
}

QString QgsWmsProjectParser::featureInfoDocumentElement( const QString &defaultValue ) const
{
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return defaultValue;
  }
  QDomElement featureInfoDocumentElem = propertiesElem.firstChildElement( QStringLiteral( "WMSFeatureInfoDocumentElement" ) );
  if ( featureInfoDocumentElem.isNull() )
  {
    return defaultValue;
  }
  return featureInfoDocumentElem.text();
}

QString QgsWmsProjectParser::featureInfoDocumentElementNS() const
{
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return QLatin1String( "" );
  }
  QDomElement featureInfoDocumentNSElem = propertiesElem.firstChildElement( QStringLiteral( "WMSFeatureInfoDocumentElementNS" ) );
  if ( featureInfoDocumentNSElem.isNull() )
  {
    return QLatin1String( "" );
  }
  return featureInfoDocumentNSElem.text();
}

QString QgsWmsProjectParser::featureInfoSchema() const
{
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return QLatin1String( "" );
  }
  QDomElement featureInfoSchemaElem = propertiesElem.firstChildElement( QStringLiteral( "WMSFeatureInfoSchema" ) );
  if ( featureInfoSchemaElem.isNull() )
  {
    return QLatin1String( "" );
  }
  return featureInfoSchemaElem.text();
}


bool QgsWmsProjectParser::featureInfoFormatSIA2045() const
{
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return false;
  }

  QDomElement sia2045Elem = propertiesElem.firstChildElement( QStringLiteral( "WMSInfoFormatSIA2045" ) );
  if ( sia2045Elem.isNull() )
  {
    return false;
  }

  if ( sia2045Elem.text().compare( QLatin1String( "enabled" ), Qt::CaseInsensitive ) == 0
       || sia2045Elem.text().compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    return true;
  }
  return false;
}

void QgsWmsProjectParser::drawOverlays( QPainter *p, int dpi, int width, int height ) const
{
  Q_UNUSED( width );
  Q_UNUSED( height );

  //consider DPI
  double scaleFactor = dpi / 88.0; //assume 88 as standard dpi

  //text annotations
  QList< QPair< QTextDocument *, QDomElement > >::const_iterator textIt = mTextAnnotationItems.constBegin();
  for ( ; textIt != mTextAnnotationItems.constEnd(); ++textIt )
  {
    QDomElement annotationElem = textIt->second;
    if ( annotationElem.isNull() )
    {
      continue;
    }

    int itemWidth = annotationElem.attribute( QStringLiteral( "frameWidth" ), QStringLiteral( "0" ) ).toInt();
    int itemHeight = annotationElem.attribute( QStringLiteral( "frameHeight" ), QStringLiteral( "0" ) ).toInt();

    //calculate item position
    double xPos, yPos;
    if ( !annotationPosition( annotationElem, scaleFactor, xPos, yPos ) )
    {
      continue;
    }

    drawAnnotationRectangle( p, annotationElem, scaleFactor, xPos, yPos, itemWidth, itemHeight );

    //draw annotation contents
    p->translate( xPos, yPos );
    p->scale( scaleFactor, scaleFactor );
    textIt->first->drawContents( p, QRectF( 0, 0, itemWidth / scaleFactor, itemHeight / scaleFactor ) );
    p->restore();
  }

  //svg annotations
  QList< QPair< QSvgRenderer *, QDomElement > >::const_iterator svgIt = mSvgAnnotationElems.constBegin();
  QDomElement annotationElem;
  for ( ; svgIt != mSvgAnnotationElems.constEnd(); ++svgIt )
  {
    annotationElem = svgIt->second;
    int itemWidth = annotationElem.attribute( QStringLiteral( "frameWidth" ), QStringLiteral( "0" ) ).toInt() * scaleFactor;
    int itemHeight = annotationElem.attribute( QStringLiteral( "frameHeight" ), QStringLiteral( "0" ) ).toInt() * scaleFactor;

    //calculate item position
    double xPos, yPos;
    if ( !annotationPosition( annotationElem, scaleFactor, xPos, yPos ) )
    {
      continue;
    }

    drawAnnotationRectangle( p, annotationElem, scaleFactor, xPos, yPos, itemWidth, itemHeight );

    //keep width/height ratio of svg
    QRect viewBox = svgIt->first->viewBox();
    if ( viewBox.isValid() )
    {
      double widthRatio = static_cast< double >( itemWidth ) / static_cast< double >( viewBox.width() );
      double heightRatio = static_cast< double >( itemHeight ) / static_cast< double >( viewBox.height() );
      double renderWidth = 0;
      double renderHeight = 0;
      if ( widthRatio <= heightRatio )
      {
        renderWidth = itemWidth;
        renderHeight = viewBox.height() * itemWidth / viewBox.width();
      }
      else
      {
        renderHeight = itemHeight;
        renderWidth = viewBox.width() * itemHeight / viewBox.height();
      }

      svgIt->first->render( p, QRectF( xPos, yPos, renderWidth,
                                       renderHeight ) );
    }
  }
}

void QgsWmsProjectParser::loadLabelSettings() const
{
  int searchMethod, nCandPoint, nCandLine, nCandPoly;
  bool showingCandidates, drawRectOnly, showingShadowRects, showingAllLabels, showingPartialsLabels, drawOutlineLabels;

  readLabelSettings( searchMethod, nCandPoint, nCandLine, nCandPoly, showingCandidates, drawRectOnly, showingShadowRects, showingAllLabels, showingPartialsLabels, drawOutlineLabels );

  QgsProject::instance()->writeEntry( "PAL", "/SearchMethod", searchMethod );
  QgsProject::instance()->writeEntry( "PAL", "/CandidatesPoint", nCandPoint );
  QgsProject::instance()->writeEntry( "PAL", "/CandidatesLine", nCandLine );
  QgsProject::instance()->writeEntry( "PAL", "/CandidatesPolygon", nCandPoly );

  QgsProject::instance()->writeEntry( "PAL", "/ShowingCandidates", showingCandidates );
  QgsProject::instance()->writeEntry( "PAL", "/DrawRectOnly", drawRectOnly );
  QgsProject::instance()->writeEntry( "PAL", "/ShowingShadowRects", showingShadowRects );
  QgsProject::instance()->writeEntry( "PAL", "/ShowingAllLabels", showingAllLabels );
  QgsProject::instance()->writeEntry( "PAL", "/ShowingPartialsLabels", showingPartialsLabels );
  QgsProject::instance()->writeEntry( "PAL", "/DrawOutlineLabels", drawOutlineLabels );
}

void QgsWmsProjectParser::readLabelSettings( int &searchMethod, int &nCandPoint, int &nCandLine, int &nCandPoly, bool &showingCandidates, bool &drawRectOnly, bool &showingShadowRects, bool &showingAllLabels, bool &showingPartialsLabels, bool &drawOutlineLabels ) const
{
  searchMethod = static_cast< int >( QgsPalLabeling::Chain );
  nCandPoint = 8;
  nCandLine = 8;
  nCandPoly = 8;
  showingCandidates = false;
  drawRectOnly = false;
  showingShadowRects = false;
  showingAllLabels = false;
  showingPartialsLabels = true;
  drawOutlineLabels = true;

  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return;
  }

  QDomElement palElem = propertiesElem.firstChildElement( "PAL" );
  if ( palElem.isNull() )
  {
    return;
  }

  QDomElement candPointElem = palElem.firstChildElement( "CandidatesPoint" );
  if ( !candPointElem.isNull() )
  {
    nCandPoint = candPointElem.text().toInt();
  }
  QDomElement candLineElem = palElem.firstChildElement( "CandidatesLine" );
  if ( !candLineElem.isNull() )
  {
    nCandLine = candLineElem.text().toInt();
  }
  QDomElement candPolyElem = palElem.firstChildElement( "CandidatesPolygon" );
  if ( !candPolyElem.isNull() )
  {
    nCandPoly = candPolyElem.text().toInt();
  }

  QDomElement showCandElem = palElem.firstChildElement( "ShowingCandidates" );
  if ( !showCandElem.isNull() )
  {
    showingCandidates = showCandElem.text().compare( "true", Qt::CaseInsensitive ) == 0;
  }

  QDomElement showAllLabelsElem = palElem.firstChildElement( "ShowingAllLabels" );
  if ( !showAllLabelsElem.isNull() )
  {
    showingAllLabels = showAllLabelsElem.text().compare( "true", Qt::CaseInsensitive ) == 0;
  }

  QDomElement showPartialsLabelsElem = palElem.firstChildElement( "ShowingPartialsLabels" );
  if ( !showPartialsLabelsElem.isNull() )
  {
    showingPartialsLabels = showPartialsLabelsElem.text().compare( "true", Qt::CaseInsensitive ) == 0;
  }

  QDomElement drawOutlineLabelsElem = palElem.firstChildElement( "DrawOutlineLabels" );
  if ( !drawOutlineLabelsElem.isNull() )
  {
    drawOutlineLabels = drawOutlineLabelsElem.text().compare( "true", Qt::CaseInsensitive ) == 0;
  }
}

int QgsWmsProjectParser::nLayers() const
{
  return mProjectParser->numberOfLayers();
}

void QgsWmsProjectParser::serviceCapabilities( QDomElement &parentElement, QDomDocument &doc ) const
{
  mProjectParser->serviceCapabilities( parentElement, doc, QStringLiteral( "WMS" ), featureInfoFormatSIA2045() );
}

QDomElement QgsWmsProjectParser::composerByName( const QString &composerName ) const
{
  QDomElement composerElem;
  if ( !mProjectParser->xmlDocument() )
  {
    return composerElem;
  }

  QList<QDomElement> composerElemList = mProjectParser->publishedComposerElements();
  QList<QDomElement>::const_iterator composerIt = composerElemList.constBegin();
  for ( ; composerIt != composerElemList.constEnd(); ++composerIt )
  {
    QDomElement currentComposerElem = *composerIt;
    if ( currentComposerElem.attribute( QStringLiteral( "title" ) ) == composerName )
    {
      return currentComposerElem;
    }
  }

  return composerElem;
}

QgsLayerTree *QgsWmsProjectParser::projectLayerTreeGroup() const
{
  QgsLayerTree *rootGroup = new QgsLayerTree;
  const QDomDocument *projectDoc = mProjectParser->xmlDocument();
  if ( !projectDoc )
  {
    return rootGroup;
  }

  QDomElement qgisElem = projectDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return rootGroup;
  }
  QDomElement layerTreeElem = qgisElem.firstChildElement( QStringLiteral( "layer-tree-group" ) );
  if ( layerTreeElem.isNull() )
  {
    QgsLayerTreeUtils::readOldLegend( rootGroup, mProjectParser->legendElem() );
    return rootGroup;
  }
  return QgsLayerTree::readXml( layerTreeElem );
}

bool QgsWmsProjectParser::annotationPosition( const QDomElement &elem, double scaleFactor, double &xPos, double &yPos )
{
  Q_UNUSED( scaleFactor );

  xPos = elem.attribute( QStringLiteral( "canvasPosX" ) ).toDouble() / scaleFactor;
  yPos = elem.attribute( QStringLiteral( "canvasPosY" ) ).toDouble() / scaleFactor;
  return true;
}

void QgsWmsProjectParser::drawAnnotationRectangle( QPainter *p, const QDomElement &elem, double scaleFactor, double xPos, double yPos, int itemWidth, int itemHeight )
{
  Q_UNUSED( scaleFactor );
  if ( !p )
  {
    return;
  }

  QColor backgroundColor( elem.attribute( QStringLiteral( "frameBackgroundColor" ), QStringLiteral( "#000000" ) ) );
  backgroundColor.setAlpha( elem.attribute( QStringLiteral( "frameBackgroundColorAlpha" ), QStringLiteral( "255" ) ).toInt() );
  p->setBrush( QBrush( backgroundColor ) );
  QColor frameColor( elem.attribute( QStringLiteral( "frameColor" ), QStringLiteral( "#000000" ) ) );
  frameColor.setAlpha( elem.attribute( QStringLiteral( "frameColorAlpha" ), QStringLiteral( "255" ) ).toInt() );
  QPen framePen( frameColor );
  framePen.setWidth( elem.attribute( QStringLiteral( "frameBorderWidth" ), QStringLiteral( "1" ) ).toInt() );
  p->setPen( framePen );

  p->drawRect( QRectF( xPos, yPos, itemWidth, itemHeight ) );
}

void QgsWmsProjectParser::createTextAnnotationItems()
{
  cleanupTextAnnotationItems();

  const QDomDocument *xmlDoc = mProjectParser->xmlDocument();
  if ( !xmlDoc )
  {
    return;
  }

  //text annotations
  QDomElement qgisElem = xmlDoc->documentElement();
  QDomNodeList textAnnotationList = qgisElem.elementsByTagName( QStringLiteral( "TextAnnotationItem" ) );
  QDomElement textAnnotationElem;
  QDomElement annotationElem;
  for ( int i = 0; i < textAnnotationList.size(); ++i )
  {
    textAnnotationElem = textAnnotationList.at( i ).toElement();
    annotationElem = textAnnotationElem.firstChildElement( QStringLiteral( "AnnotationItem" ) );
    if ( !annotationElem.isNull() && annotationElem.attribute( QStringLiteral( "mapPositionFixed" ) ) != QLatin1String( "1" ) )
    {
      QTextDocument *textDoc = new QTextDocument();
      textDoc->setHtml( textAnnotationElem.attribute( QStringLiteral( "document" ) ) );
      mTextAnnotationItems.push_back( qMakePair( textDoc, annotationElem ) );
    }
  }
}

void QgsWmsProjectParser::createSvgAnnotationItems()
{
  mSvgAnnotationElems.clear();
  const QDomDocument *xmlDoc = mProjectParser->xmlDocument();
  if ( !xmlDoc )
  {
    return;
  }

  QDomElement qgisElem = xmlDoc->documentElement();
  QDomNodeList svgAnnotationList = qgisElem.elementsByTagName( QStringLiteral( "SVGAnnotationItem" ) );
  QDomElement svgAnnotationElem;
  QDomElement annotationElem;
  for ( int i = 0; i < svgAnnotationList.size(); ++i )
  {
    svgAnnotationElem = svgAnnotationList.at( i ).toElement();
    annotationElem = svgAnnotationElem.firstChildElement( QStringLiteral( "AnnotationItem" ) );
    if ( !annotationElem.isNull() && annotationElem.attribute( QStringLiteral( "mapPositionFixed" ) ) != QLatin1String( "1" ) )
    {
      QSvgRenderer *svg = new QSvgRenderer();
      if ( svg->load( mProjectParser->convertToAbsolutePath( svgAnnotationElem.attribute( QStringLiteral( "file" ) ) ) ) )
      {
        mSvgAnnotationElems.push_back( qMakePair( svg, annotationElem ) );
      }
      else
      {
        delete svg;
      }
    }
  }
}

void QgsWmsProjectParser::cleanupSvgAnnotationItems()
{
  QList< QPair< QSvgRenderer *, QDomElement > >::const_iterator it = mSvgAnnotationElems.constBegin();
  for ( ; it != mSvgAnnotationElems.constEnd(); ++it )
  {
    delete it->first;
  }
  mSvgAnnotationElems.clear();
}

void QgsWmsProjectParser::cleanupTextAnnotationItems()
{
  QList< QPair< QTextDocument *, QDomElement > >::const_iterator it = mTextAnnotationItems.constBegin();
  for ( ; it != mTextAnnotationItems.constEnd(); ++it )
  {
    delete it->first;
  }
  mTextAnnotationItems.clear();
}

QString QgsWmsProjectParser::getCapaServiceUrl( QDomDocument &doc ) const
{
  QString url;
  QDomNodeList getCapNodeList = doc.elementsByTagName( QStringLiteral( "GetCapabilities" ) );
  if ( getCapNodeList.count() > 0 )
  {
    QDomElement getCapElem = getCapNodeList.at( 0 ).toElement();
    QDomNodeList getCapORNodeList = getCapElem.elementsByTagName( QStringLiteral( "OnlineResource" ) );
    if ( getCapORNodeList.count() > 0 )
    {
      url = getCapORNodeList.at( 0 ).toElement().attribute( QStringLiteral( "xlink:href" ), QLatin1String( "" ) );
    }

  }

  return url;
}

bool QgsWmsProjectParser::allowRequestDefinedDatasources() const
{
  if ( !mProjectParser->xmlDocument() )
  {
    return false;
  }

  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return false;
  }
  QDomElement dsElem = propertiesElem.firstChildElement( "WMSRequestDefinedDataSources" );
  if ( dsElem.isNull() )
  {
    return false;
  }

  return ( dsElem.text().compare( "true", Qt::CaseInsensitive ) == 0 );
}
