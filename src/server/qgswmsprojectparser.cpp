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
#include "qgsmaplayerstylemanager.h"
#include "qgsmapserviceexception.h"
#include "qgspallabeling.h"
#include "qgsrendererv2.h"
#include "qgsvectorlayer.h"

#include "qgscomposition.h"
#include "qgscomposerarrow.h"
#include "qgscomposerattributetable.h"
#include "qgscomposerlabel.h"
#include "qgscomposerlegend.h"
#include "qgscomposermap.h"
#include "qgscomposerframe.h"
#include "qgscomposerhtml.h"
#include "qgscomposerpicture.h"
#include "qgscomposerscalebar.h"
#include "qgscomposershape.h"
#include "qgslayertreegroup.h"

#include <QFileInfo>
#include <QTextDocument>

// style name to use for the unnamed style of layers (must not be empty name in WMS)
// this implies that a layer style called "default" will not be usable in WMS server
#define EMPTY_STYLE_NAME   "default"

QgsWMSProjectParser::QgsWMSProjectParser( const QString& filePath )
    : QgsWMSConfigParser()
{
  mProjectParser = QgsConfigCache::instance()->serverConfiguration( filePath );
  mLegendLayerFont.fromString( mProjectParser->firstComposerLegendElement().attribute( "layerFont" ) );
  mLegendItemFont.fromString( mProjectParser->firstComposerLegendElement().attribute( "itemFont" ) );
  createTextAnnotationItems();
  createSvgAnnotationItems();
}

QgsWMSProjectParser::~QgsWMSProjectParser()
{
  cleanupTextAnnotationItems();
  cleanupSvgAnnotationItems();
}

void QgsWMSProjectParser::layersAndStylesCapabilities( QDomElement& parentElement, QDomDocument& doc, const QString& version, bool fullProjectSettings ) const
{
  QStringList nonIdentifiableLayers = identifyDisabledLayers();

  if ( mProjectParser->projectLayerElements().size() < 1 && mProjectParser->legendGroupElements().size() < 1 )
  {
    return;
  }

  if ( fullProjectSettings )
  {
    addDrawingOrder( parentElement, doc );
  }

  QMap<QString, QgsMapLayer *> layerMap;
  mProjectParser->projectLayerMap( layerMap );

  //According to the WMS spec, there can be only one toplevel layer.
  //So we create an artificial one here to be in accordance with the schema
  QString projTitle = mProjectParser->projectTitle();
  QDomElement layerParentElem = doc.createElement( "Layer" );
  layerParentElem.setAttribute( "queryable", "1" );
  QDomElement layerParentNameElem = doc.createElement( "Name" );
  QDomText layerParentNameText = doc.createTextNode( projTitle );
  layerParentNameElem.appendChild( layerParentNameText );
  layerParentElem.appendChild( layerParentNameElem );
  QDomElement layerParentTitleElem = doc.createElement( "Title" );
  QDomText layerParentTitleText = doc.createTextNode( projTitle );
  layerParentTitleElem.appendChild( layerParentTitleText );
  layerParentElem.appendChild( layerParentTitleElem );

  QDomElement legendElem = mProjectParser->legendElem();

  addLayers( doc, layerParentElem, legendElem, layerMap, nonIdentifiableLayers, version, fullProjectSettings );

  parentElement.appendChild( layerParentElem );
  mProjectParser->combineExtentAndCrsOfGroupChildren( layerParentElem, doc, true );
}

QList<QgsMapLayer*> QgsWMSProjectParser::mapLayerFromStyle( const QString& lName, const QString& styleName, bool useCache ) const
{
  QMap< int, QgsMapLayer* > layers;

  //first check if the layer name refers an unpublished layer / group
  if ( mProjectParser->restrictedLayers().contains( lName ) )
  {
    return QList<QgsMapLayer*>();
  }

  // can't use layer cache if we are going to apply a non-default style
  if ( !styleName.isEmpty() )
    useCache = false;

  //does lName refer to a leaf layer
  const QHash< QString, QDomElement > &projectLayerElements = mProjectParser->useLayerIDs() ? mProjectParser->projectLayerElementsById() : mProjectParser->projectLayerElementsByName();
  QHash< QString, QDomElement >::const_iterator layerElemIt = projectLayerElements.find( lName );
  if ( layerElemIt != projectLayerElements.constEnd() )
  {
    QgsMapLayer* ml = mProjectParser->createLayerFromElement( layerElemIt.value(), useCache );
    if ( !styleName.isEmpty() )
    {
      // try to apply the specified style
      if ( !ml->styleManager()->setCurrentStyle( styleName != EMPTY_STYLE_NAME ? styleName : QString() ) )
        throw QgsMapServiceException( "StyleNotDefined", QString( "Style \"%1\" does not exist for layer \"%2\"" ).arg( styleName ).arg( lName ) );
    }
    return QList<QgsMapLayer*>() << ml;
  }

  //group or project name
  QDomElement groupElement;
  if ( lName == mProjectParser->projectTitle() )
  {
    groupElement = mProjectParser->legendElem();
  }
  else
  {
    const QList<QDomElement>& legendGroupElements = mProjectParser->legendGroupElements();
    QList<QDomElement>::const_iterator groupIt = legendGroupElements.constBegin();
    for ( ; groupIt != legendGroupElements.constEnd(); ++groupIt )
    {
      if ( groupIt->attribute( "name" ) == lName )
      {
        groupElement = *groupIt;
        break;
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
  QDomNodeList legendLayerList = legendElement.elementsByTagName( "legendlayer" );
  for ( int i = 0; i < legendLayerList.size(); ++i )
  {
    QDomElement legendLayerElem = legendLayerList.at( i ).toElement();
    if ( legendLayerElem.attribute( "name" ) == lName )
    {
      mProjectParser->layerFromLegendLayer( legendLayerElem, layers, useCache );
    }
  }

  //Still not found. Probably it is a layer or a subgroup in an embedded group
  //go through all groups
  //check if they are embedded
  //if yes, request leaf layers and groups from project parser
  const QList<QDomElement>& legendGroupElements = mProjectParser->legendGroupElements();
  QList<QDomElement>::const_iterator legendIt = legendGroupElements.constBegin();
  for ( ; legendIt != legendGroupElements.constEnd(); ++legendIt )
  {
    if ( legendIt->attribute( "embedded" ) == "1" )
    {
      QString project = mProjectParser->convertToAbsolutePath( legendIt->attribute( "project" ) );
      QgsWMSProjectParser* p = dynamic_cast<QgsWMSProjectParser*>( QgsConfigCache::instance()->wmsConfiguration( project ) );
      if ( p )
      {
        QgsServerProjectParser* pp = p->mProjectParser;
        const QHash< QString, QDomElement >& pLayerByName = pp->projectLayerElementsByName();
        QHash< QString, QDomElement >::const_iterator pLayerNameIt = pLayerByName.find( lName );
        if ( pLayerNameIt != pLayerByName.constEnd() )
        {
          return ( QList<QgsMapLayer*>() << pp->createLayerFromElement( pLayerNameIt.value(), useCache ) );
        }

        const QList<QDomElement>& legendGroupElements = pp->legendGroupElements();
        QList<QDomElement>::const_iterator pLegendGroupIt = legendGroupElements.constBegin();
        for ( ; pLegendGroupIt != legendGroupElements.constEnd(); ++pLegendGroupIt )
        {
          if ( pLegendGroupIt->attribute( "name" ) == lName )
          {
            addLayersFromGroup( *pLegendGroupIt, layers, useCache );
            break;
          }
        }
      }
    }
  }

  if ( layers.count() == 0 )
    throw QgsMapServiceException( "LayerNotDefined", QString( "Layer \"%1\" does not exist" ).arg( lName ) );

  return layers.values();
}

void QgsWMSProjectParser::addLayersFromGroup( const QDomElement& legendGroupElem, QMap< int, QgsMapLayer*>& layers, bool useCache ) const
{
  if ( legendGroupElem.isNull() )
  {
    return;
  }

  if ( legendGroupElem.attribute( "embedded" ) == "1" ) //embedded group
  {
    QString groupName = legendGroupElem.attribute( "name" );
    int drawingOrder = mProjectParser->updateLegendDrawingOrder() ? legendGroupElem.attribute( "drawingOrder", "-1" ).toInt() : -1;

    QString project = mProjectParser->convertToAbsolutePath( legendGroupElem.attribute( "project" ) );
    QgsWMSProjectParser* p = dynamic_cast<QgsWMSProjectParser*>( QgsConfigCache::instance()->wmsConfiguration( project ) );
    if ( p )
    {
      QgsServerProjectParser* pp = p->mProjectParser;
      const QList<QDomElement>& legendGroups = pp->legendGroupElements();
      QList<QDomElement>::const_iterator legendIt = legendGroups.constBegin();
      for ( ; legendIt != legendGroups.constEnd(); ++legendIt )
      {
        if ( legendIt->attribute( "name" ) == groupName )
        {
          QMap< int, QgsMapLayer*> embeddedGroupLayers;
          p->addLayersFromGroup( *legendIt, embeddedGroupLayers, useCache );

          //reverse order because it will be reversed again afterwards in insertMulti
          QList< QgsMapLayer* > embeddedLayerList = QgsConfigParserUtils::layerMapToList( embeddedGroupLayers, pp->updateLegendDrawingOrder() );

          QList< QgsMapLayer* >::const_iterator layerIt = embeddedLayerList.constBegin();
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
    for ( int i = 0; i < groupElemChildren.size(); ++i )
    {
      QDomElement elem = groupElemChildren.at( i ).toElement();
      if ( elem.tagName() == "legendgroup" )
      {
        addLayersFromGroup( elem, layers, useCache );
      }
      else if ( elem.tagName() == "legendlayer" )
      {
        mProjectParser->layerFromLegendLayer( elem, layers, useCache );
      }
    }
  }
}

QString QgsWMSProjectParser::serviceUrl() const
{
  return mProjectParser->serviceUrl();
}

QStringList QgsWMSProjectParser::wfsLayerNames() const
{
  return mProjectParser->wfsLayerNames();
}

double QgsWMSProjectParser::legendBoxSpace() const
{
  QDomElement legendElem = mProjectParser->firstComposerLegendElement();
  return legendElem.isNull() ? 2.0 : legendElem.attribute( "boxSpace" ).toDouble();
}

double QgsWMSProjectParser::legendLayerSpace() const
{
  QDomElement legendElem = mProjectParser->firstComposerLegendElement();
  return legendElem.isNull() ? 3.0 : legendElem.attribute( "layerSpace" ).toDouble();
}

double QgsWMSProjectParser::legendLayerTitleSpace() const
{
  QDomElement legendElem = mProjectParser->firstComposerLegendElement();
  return legendElem.isNull() ? 3.0 : legendElem.attribute( "layerTitleSpace" ).toDouble();
}

double QgsWMSProjectParser::legendSymbolSpace() const
{
  QDomElement legendElem = mProjectParser->firstComposerLegendElement();
  return legendElem.isNull() ? 2.0 : legendElem.attribute( "symbolSpace" ).toDouble();
}

double QgsWMSProjectParser::legendIconLabelSpace() const
{
  QDomElement legendElem = mProjectParser->firstComposerLegendElement();
  return legendElem.isNull() ? 2.0 : legendElem.attribute( "iconLabelSpace" ).toDouble();
}

double QgsWMSProjectParser::legendSymbolWidth() const
{
  QDomElement legendElem = mProjectParser->firstComposerLegendElement();
  return legendElem.isNull() ? 7.0 : legendElem.attribute( "symbolWidth" ).toDouble();
}

double QgsWMSProjectParser::legendSymbolHeight() const
{
  QDomElement legendElem = mProjectParser->firstComposerLegendElement();
  return legendElem.isNull() ? 4.0 : legendElem.attribute( "symbolHeight" ).toDouble();
}

const QFont& QgsWMSProjectParser::legendLayerFont() const
{
  return mLegendLayerFont;
}

const QFont& QgsWMSProjectParser::legendItemFont() const
{
  return mLegendItemFont;
}

double QgsWMSProjectParser::maxWidth() const
{
  double maxWidth = -1;
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( !propertiesElem.isNull() )
  {
    QDomElement maxWidthElem = propertiesElem.firstChildElement( "WMSMaxWidth" );
    if ( !maxWidthElem.isNull() )
    {
      maxWidth = maxWidthElem.text().toInt();
    }
  }
  return maxWidth;
}

double QgsWMSProjectParser::maxHeight() const
{
  double maxHeight = -1;
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( !propertiesElem.isNull() )
  {
    QDomElement maxWidthElem = propertiesElem.firstChildElement( "WMSMaxHeight" );
    if ( !maxWidthElem.isNull() )
    {
      maxHeight = maxWidthElem.text().toInt();
    }
  }
  return maxHeight;
}

double QgsWMSProjectParser::imageQuality() const
{
  double imageQuality = -1;
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( !propertiesElem.isNull() )
  {
    QDomElement imageQualityElem = propertiesElem.firstChildElement( "WMSImageQuality" );
    if ( !imageQualityElem.isNull() )
    {
      imageQuality = imageQualityElem.text().toInt();
    }
  }
  return imageQuality;
}

int QgsWMSProjectParser::WMSPrecision() const
{
  int WMSPrecision = -1;
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( !propertiesElem.isNull() )
  {
    QDomElement WMSPrecisionElem = propertiesElem.firstChildElement( "WMSPrecision" );
    if ( !WMSPrecisionElem.isNull() )
    {
      WMSPrecision = WMSPrecisionElem.text().toInt();
    }
  }
  return WMSPrecision;
}

QgsComposition* QgsWMSProjectParser::initComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, QList< QgsComposerMap* >& mapList, QList< QgsComposerLegend* >& legendList, QList< QgsComposerLabel* >& labelList, QList<const QgsComposerHtml *>& htmlList ) const
{
  //Create composition from xml
  QDomElement composerElem = composerByName( composerTemplate );
  if ( composerElem.isNull() )
  {
    throw QgsMapServiceException( "Error", "Composer template not found" );
  }

  QDomElement compositionElem = composerElem.firstChildElement( "Composition" );
  if ( compositionElem.isNull() )
  {
    return 0;
  }

  QgsComposition* composition = new QgsComposition( mapRenderer->mapSettings() ); //set resolution, paper size from composer element attributes
  if ( !composition->readXML( compositionElem, *( mProjectParser->xmlDocument() ) ) )
  {
    delete composition;
    return 0;
  }

  composition->addItemsFromXML( compositionElem, *( mProjectParser->xmlDocument() ) );

  labelList.clear();
  mapList.clear();
  legendList.clear();
  htmlList.clear();

  QList<QgsComposerItem* > itemList;
  composition->composerItems( itemList );
  QList<QgsComposerItem *>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    QgsComposerLabel* label = dynamic_cast< QgsComposerLabel *>( *itemIt );
    if ( label )
    {
      labelList.push_back( label );
      continue;
    }
    QgsComposerMap* map = dynamic_cast< QgsComposerMap *>( *itemIt );
    if ( map )
    {
      mapList.push_back( map );
      continue;
    }
    QgsComposerLegend* legend = dynamic_cast< QgsComposerLegend *>( *itemIt );
    if ( legend )
    {
#if 0
      QgsLegendModelV2* model = legend->modelV2();
      QgsLayerTreeGroup* root = model->rootGroup();
      QStringList layerIds = root->findLayerIds();
      throw QgsMapServiceException( "Error", "Composer legend layerIds " + layerIds.join( " ," ) );
#endif
      if ( legend->autoUpdateModel() )
      {
        QgsLegendModelV2* model = legend->modelV2();
        model->setRootGroup( projectLayerTreeGroup() );
      }
      legendList.push_back( legend );
      continue;
    }
    QgsComposerPicture* pic = dynamic_cast< QgsComposerPicture *>( *itemIt );
    if ( pic )
    {
      pic->setPicturePath( mProjectParser->convertToAbsolutePath(( pic )->picturePath() ) );
      continue;
    }

    // an html item will be a composer frame and if it is we can try to get
    // its multiframe parent and then try to cast that to a composer html
    const QgsComposerFrame* frame = dynamic_cast<const QgsComposerFrame *>( *itemIt );
    if ( frame )
    {
      const QgsComposerMultiFrame * multiFrame = frame->multiFrame();
      const QgsComposerHtml* composerHtml = dynamic_cast<const QgsComposerHtml *>( multiFrame );
      if ( composerHtml )
      {
        htmlList.push_back( composerHtml );
        continue;
      }
    }
  }

  return composition;
}

void QgsWMSProjectParser::printCapabilities( QDomElement& parentElement, QDomDocument& doc ) const
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

  QDomElement composerTemplatesElem = doc.createElement( "ComposerTemplates" );

  QList<QDomElement>::const_iterator composerElemIt = composerElemList.constBegin();
  for ( ; composerElemIt != composerElemList.constEnd(); ++composerElemIt )
  {
    QDomElement composerTemplateElem = doc.createElement( "ComposerTemplate" );
    QDomElement currentComposerElem = *composerElemIt;
    if ( currentComposerElem.isNull() )
    {
      continue;
    }

    composerTemplateElem.setAttribute( "name", currentComposerElem.attribute( "title" ) );

    //get paper width and hight in mm from composition
    QDomElement compositionElem = currentComposerElem.firstChildElement( "Composition" );
    if ( compositionElem.isNull() )
    {
      continue;
    }
    composerTemplateElem.setAttribute( "width", compositionElem.attribute( "paperWidth" ) );
    composerTemplateElem.setAttribute( "height", compositionElem.attribute( "paperHeight" ) );


    //add available composer maps and their size in mm
    QDomNodeList composerMapList = currentComposerElem.elementsByTagName( "ComposerMap" );
    for ( int j = 0; j < composerMapList.size(); ++j )
    {
      QDomElement cmap = composerMapList.at( j ).toElement();
      QDomElement citem = cmap.firstChildElement( "ComposerItem" );
      if ( citem.isNull() )
      {
        continue;
      }

      QDomElement composerMapElem = doc.createElement( "ComposerMap" );
      composerMapElem.setAttribute( "name", "map" + cmap.attribute( "id" ) );
      composerMapElem.setAttribute( "width", citem.attribute( "width" ) );
      composerMapElem.setAttribute( "height", citem.attribute( "height" ) );
      composerTemplateElem.appendChild( composerMapElem );
    }

    //add available composer labels
    QDomNodeList composerLabelList = currentComposerElem.elementsByTagName( "ComposerLabel" );
    for ( int j = 0; j < composerLabelList.size(); ++j )
    {
      QDomElement citem = composerLabelList.at( j ).firstChildElement( "ComposerItem" );
      QString id = citem.attribute( "id" );
      if ( id.isEmpty() ) //only export labels with ids for text replacement
      {
        continue;
      }
      QDomElement composerLabelElem = doc.createElement( "ComposerLabel" );
      composerLabelElem.setAttribute( "name", id );
      composerTemplateElem.appendChild( composerLabelElem );
    }

    //add available composer HTML
    QDomNodeList composerHtmlList = currentComposerElem.elementsByTagName( "ComposerHtml" );
    for ( int j = 0; j < composerHtmlList.size(); ++j )
    {
      QDomElement citem = composerHtmlList.at( j ).firstChildElement( "ComposerFrame" ).firstChildElement( "ComposerItem" );
      QString id = citem.attribute( "id" );
      if ( id.isEmpty() ) //only export labels with ids for text replacement
      {
        continue;
      }
      QDomElement composerHtmlElem = doc.createElement( "ComposerHtml" );
      composerHtmlElem.setAttribute( "name", id );
      composerTemplateElem.appendChild( composerHtmlElem );
    }

    composerTemplatesElem.appendChild( composerTemplateElem );
  }
  parentElement.appendChild( composerTemplatesElem );
}

QList< QPair< QString, QgsLayerCoordinateTransform > > QgsWMSProjectParser::layerCoordinateTransforms() const
{
  return mProjectParser->layerCoordinateTransforms();
}

void QgsWMSProjectParser::owsGeneralAndResourceList( QDomElement& parentElement, QDomDocument& doc, const QString& strHref ) const
{
  // set parentElement id
  QFileInfo projectFileInfo( mProjectParser->projectPath() );
  parentElement.setAttribute( "id", "ows-context-" + projectFileInfo.baseName() );

  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    QFile wmsService( "wms_metadata.xml" );
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
  QDomElement generalElem = doc.createElement( "General" );

  QDomElement windowElem = doc.createElement( "Window" );
  windowElem.setAttribute( "height", "600" );
  windowElem.setAttribute( "width", "800" );
  generalElem.appendChild( windowElem );

  //WMS title
  QDomElement titleElem = propertiesElem.firstChildElement( "WMSServiceTitle" );
  if ( !titleElem.isNull() )
  {
    QDomElement wmsTitleElem = doc.createElement( "ows:Title" );
    QDomText wmsTitleText = doc.createTextNode( titleElem.text() );
    wmsTitleElem.appendChild( wmsTitleText );
    generalElem.appendChild( wmsTitleElem );
  }

  //WMS abstract
  QDomElement abstractElem = propertiesElem.firstChildElement( "WMSServiceAbstract" );
  if ( !abstractElem.isNull() )
  {
    QDomElement wmsAbstractElem = doc.createElement( "ows:Abstract" );
    QDomText wmsAbstractText = doc.createTextNode( abstractElem.text() );
    wmsAbstractElem.appendChild( wmsAbstractText );
    generalElem.appendChild( wmsAbstractElem );
  }

  //keyword list
  QDomElement keywordListElem = propertiesElem.firstChildElement( "WMSKeywordList" );
  if ( !keywordListElem.isNull() && !keywordListElem.text().isEmpty() )
  {
    bool siaFormat = featureInfoFormatSIA2045();

    QDomElement keywordsElem = doc.createElement( "ows:Keywords" );
    QDomNodeList keywordList = keywordListElem.elementsByTagName( "value" );
    for ( int i = 0; i < keywordList.size(); ++i )
    {
      QDomElement keywordElem = doc.createElement( "ows:Keyword" );
      QDomText keywordText = doc.createTextNode( keywordList.at( i ).toElement().text() );
      keywordElem.appendChild( keywordText );
      if ( siaFormat )
      {
        keywordElem.setAttribute( "vocabulary", "SIA_Geo405" );
      }
      keywordsElem.appendChild( keywordElem );
    }

    if ( keywordList.size() > 0 )
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

  QDomElement resourceListElem = doc.createElement( "ResourceList" );

  addOWSLayers( doc, resourceListElem, legendElem, layerMap, nonIdentifiableLayers, strHref, combinedBBox, "" );

  parentElement.appendChild( resourceListElem );

  QgsRectangle mapRect = mProjectParser->mapRectangle();
  if ( !mapRect.isEmpty() )
  {
    combinedBBox = mapRect;
  }
  const QgsCoordinateReferenceSystem& projectCrs = mProjectParser->projectCRS();
  QDomElement bboxElem = doc.createElement( "ows:BoundingBox" );
  bboxElem.setAttribute( "crs", projectCrs.authid() );
  if ( projectCrs.axisInverted() )
  {
    combinedBBox.invert();
  }
  QDomElement lowerCornerElem = doc.createElement( "ows:LowerCorner" );
  QDomText lowerCornerText = doc.createTextNode( QString::number( combinedBBox.xMinimum() ) + " " +  QString::number( combinedBBox.yMinimum() ) );
  lowerCornerElem.appendChild( lowerCornerText );
  bboxElem.appendChild( lowerCornerElem );
  QDomElement upperCornerElem = doc.createElement( "ows:UpperCorner" );
  QDomText upperCornerText = doc.createTextNode( QString::number( combinedBBox.xMaximum() ) + " " +  QString::number( combinedBBox.yMaximum() ) );
  upperCornerElem.appendChild( upperCornerText );
  bboxElem.appendChild( upperCornerElem );
  generalElem.appendChild( bboxElem );
}

QStringList QgsWMSProjectParser::identifyDisabledLayers() const
{
  QStringList disabledList;

  const QDomDocument* projectDoc = mProjectParser->xmlDocument();
  if ( !projectDoc )
  {
    return disabledList;
  }

  QDomElement qgisElem = projectDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return disabledList;
  }
  QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return disabledList;
  }
  QDomElement identifyElem = propertiesElem.firstChildElement( "Identify" );
  if ( identifyElem.isNull() )
  {
    return disabledList;
  }
  QDomElement disabledLayersElem = identifyElem.firstChildElement( "disabledLayers" );
  if ( disabledLayersElem.isNull() )
  {
    return disabledList;
  }
  QDomNodeList valueList = disabledLayersElem.elementsByTagName( "value" );
  for ( int i = 0; i < valueList.size(); ++i )
  {
    disabledList << valueList.at( i ).toElement().text();
  }
  return disabledList;
}

void QgsWMSProjectParser::addDrawingOrder( QDomElement& parentElem, QDomDocument& doc ) const
{
  const QDomDocument* projectDoc = mProjectParser->xmlDocument();
  if ( !projectDoc )
  {
    return;
  }

  //find legend section
  QDomElement legendElement = projectDoc->documentElement().firstChildElement( "legend" );
  if ( legendElement.isNull() )
  {
    return;
  }

  bool useDrawingOrder = legendElement.attribute( "updateDrawingOrder" ) == "false";
  QMap<int, QString> orderedLayerNames;

  QDomNodeList legendChildren = legendElement.childNodes();
  QDomElement childElem;
  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    addDrawingOrder( legendChildren.at( i ).toElement(), useDrawingOrder, orderedLayerNames );
  }

  QStringList layerList;
  QMap<int, QString>::const_iterator nameIt = orderedLayerNames.constBegin();
  for ( ; nameIt != orderedLayerNames.constEnd(); ++nameIt )
  {
    layerList.prepend( nameIt.value() );
  }

  QDomElement layerDrawingOrderElem = doc.createElement( "LayerDrawingOrder" );
  QDomText drawingOrderText = doc.createTextNode( layerList.join( "," ) );
  layerDrawingOrderElem.appendChild( drawingOrderText );
  parentElem.appendChild( layerDrawingOrderElem );
}

void QgsWMSProjectParser::addDrawingOrderEmbeddedGroup( QDomElement groupElem, bool useDrawingOrder, QMap<int, QString>& orderedLayerList ) const
{
  if ( groupElem.isNull() )
  {
    return;
  }

  QString project = mProjectParser->convertToAbsolutePath( groupElem.attribute( "project" ) );
  if ( project.isEmpty() )
  {
    return;
  }

  int embedDrawingOrder = groupElem.attribute( "drawingOrder", "-1" ).toInt();
  QgsWMSProjectParser* p = dynamic_cast<QgsWMSProjectParser*>( QgsConfigCache::instance()->wmsConfiguration( project ) );
  if ( !p )
  {
    return;
  }

  const QDomDocument* doc = p->mProjectParser->xmlDocument();
  if ( !doc )
  {
    return;
  }

  //find requested group
  QString groupName = groupElem.attribute( "name" );
  QDomElement embeddedGroupElem; //group element in source project file
  QDomNodeList groupList = doc->elementsByTagName( "legendgroup" );
  for ( int i = 0; i < groupList.size(); ++i )
  {
    if ( groupList.at( i ).toElement().attribute( "name" ) == groupName )
    {
      embeddedGroupElem = groupList.at( i ).toElement();
      break;
    }
  }

  if ( embeddedGroupElem.isNull() ) //group does not exist in project file
  {
    return;
  }

  //legend or custom drawing order in embedded project?
  bool updateDrawingOrder = true;
  QDomNodeList legendNode = doc->elementsByTagName( "legend" );
  if ( legendNode.size() > 0 )
  {
    updateDrawingOrder = ( legendNode.at( 0 ).toElement().attribute( "updateDrawingOrder" ) == "true" );
  }

  QDomNodeList layerNodeList = embeddedGroupElem.elementsByTagName( "legendlayer" );
  QDomElement layerElem;
  QMap<int, QString > layerNames;
  QString layerName;
  for ( int i = 0; i < layerNodeList.size(); ++i )
  {
    layerElem = layerNodeList.at( i ).toElement();
    layerName = mProjectParser->useLayerIDs() ? layerElem.attribute( "id" ) : layerElem.attribute( "name" );

    int layerDrawingOrder = updateDrawingOrder ? -1 : layerElem.attribute( "drawingOrder", "-1" ).toInt();
    if ( layerDrawingOrder == -1 )
    {
      layerNames.insert( layerNames.size(), layerName );
    }
    else
    {
      orderedLayerList.insert( orderedLayerList.size(), layerName );
    }
  }

  if ( useDrawingOrder )
  {
    QMapIterator<int, QString > layerNamesIt( layerNames );
    layerNamesIt.toBack();
    while ( layerNamesIt.hasPrevious() )
    {
      layerNamesIt.previous();
      orderedLayerList.insertMulti( embedDrawingOrder, layerNamesIt.value() );
    }
  }
  else
  {
    QMap<int, QString >::const_iterator layerNamesIt = layerNames.constBegin();
    for ( ; layerNamesIt != layerNames.constEnd(); ++layerNamesIt )
    {
      orderedLayerList.insert( orderedLayerList.size(), layerNamesIt.value() );
    }
  }
}

void QgsWMSProjectParser::addDrawingOrder( QDomElement elem, bool useDrawingOrder, QMap<int, QString>& orderedLayerList ) const
{
  if ( elem.isNull() )
  {
    return;
  }

  if ( elem.tagName() == "legendgroup" )
  {
    if ( elem.attribute( "embedded" ) == "1" )
    {
      addDrawingOrderEmbeddedGroup( elem, useDrawingOrder, orderedLayerList );
    }
    else
    {
      QDomNodeList groupChildren = elem.childNodes();
      for ( int i = 0; i < groupChildren.size(); ++i )
      {
        addDrawingOrder( groupChildren.at( i ).toElement(), useDrawingOrder, orderedLayerList );
      }
    }
  }
  else if ( elem.tagName() == "legendlayer" )
  {
    QString layerName = mProjectParser->useLayerIDs()
                        ? mProjectParser->layerIdFromLegendLayer( elem )
                        : elem.attribute( "name" );

    if ( useDrawingOrder )
    {
      int drawingOrder = elem.attribute( "drawingOrder", "-1" ).toInt();
      orderedLayerList.insert( drawingOrder, layerName );
    }
    else
    {
      orderedLayerList.insert( orderedLayerList.size(), layerName );
    }
  }
}


void QgsWMSProjectParser::addLayerStyles( QgsMapLayer* currentLayer, QDomDocument& doc, QDomElement& layerElem, const QString& version ) const
{
  foreach ( QString styleName, currentLayer->styleManager()->styles() )
  {
    if ( styleName.isEmpty() )
      styleName = EMPTY_STYLE_NAME;

    QDomElement styleElem = doc.createElement( "Style" );
    QDomElement styleNameElem = doc.createElement( "Name" );
    QDomText styleNameText = doc.createTextNode( styleName );
    styleNameElem.appendChild( styleNameText );
    QDomElement styleTitleElem = doc.createElement( "Title" );
    QDomText styleTitleText = doc.createTextNode( styleName );
    styleTitleElem.appendChild( styleTitleText );
    styleElem.appendChild( styleNameElem );
    styleElem.appendChild( styleTitleElem );

    // QString LegendURL for explicit layerbased GetLegendGraphic request
    QDomElement getLayerLegendGraphicElem = doc.createElement( "LegendURL" );
    QString hrefString = currentLayer->legendUrl();
    bool customHrefString;
    if ( !hrefString.isEmpty() )
    {
      customHrefString = true;
    }
    else
    {
      customHrefString = false;
      hrefString = serviceUrl();
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
        getLayerLegendGraphicFormats << "image/png"; // << "jpeg" << "image/jpeg"

      }
      else
      {
        getLayerLegendGraphicFormats << currentLayer->legendUrlFormat();
      }

      for ( int i = 0; i < getLayerLegendGraphicFormats.size(); ++i )
      {
        QDomElement getLayerLegendGraphicFormatElem = doc.createElement( "Format" );
        QString getLayerLegendGraphicFormat = getLayerLegendGraphicFormats[i];
        QDomText getLayerLegendGraphicFormatText = doc.createTextNode( getLayerLegendGraphicFormat );
        getLayerLegendGraphicFormatElem.appendChild( getLayerLegendGraphicFormatText );
        getLayerLegendGraphicElem.appendChild( getLayerLegendGraphicFormatElem );
      }

      // no parameters on custom hrefUrl, because should link directly to graphic
      if ( !customHrefString )
      {
        QUrl mapUrl( hrefString );
        mapUrl.addQueryItem( "SERVICE", "WMS" );
        mapUrl.addQueryItem( "VERSION", version );
        mapUrl.addQueryItem( "REQUEST", "GetLegendGraphic" );
        mapUrl.addQueryItem( "LAYER", mProjectParser->useLayerIDs() ? currentLayer->id() : currentLayer->name() );
        mapUrl.addQueryItem( "FORMAT", "image/png" );
        mapUrl.addQueryItem( "STYLE", styleNameText.data() );
        if ( version == "1.3.0" )
        {
          mapUrl.addQueryItem( "SLD_VERSION", "1.1.0" );
        }
        hrefString = mapUrl.toString();
      }

      QDomElement getLayerLegendGraphicORElem = doc.createElement( "OnlineResource" );
      getLayerLegendGraphicORElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
      getLayerLegendGraphicORElem.setAttribute( "xlink:type", "simple" );
      getLayerLegendGraphicORElem.setAttribute( "xlink:href", hrefString );
      getLayerLegendGraphicElem.appendChild( getLayerLegendGraphicORElem );
      styleElem.appendChild( getLayerLegendGraphicElem );
    }
    layerElem.appendChild( styleElem );
  }
}


void QgsWMSProjectParser::addLayers( QDomDocument &doc,
                                     QDomElement &parentLayer,
                                     const QDomElement &legendElem,
                                     const QMap<QString, QgsMapLayer *> &layerMap,
                                     const QStringList &nonIdentifiableLayers,
                                     QString version, //1.1.1 or 1.3.0
                                     bool fullProjectSettings ) const
{
  QDomNodeList legendChildren = legendElem.childNodes();
  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    QDomElement currentChildElem = legendChildren.at( i ).toElement();
    QDomElement layerElem = doc.createElement( "Layer" );
    if ( fullProjectSettings )
    {
      layerElem.setAttribute( "visible", !( currentChildElem.attribute( "checked" ) == "Qt::Unchecked" ) );
    }


    if ( currentChildElem.tagName() == "legendgroup" )
    {
      layerElem.setAttribute( "queryable", "1" );
      QString name = currentChildElem.attribute( "name" );
      if ( mProjectParser->restrictedLayers().contains( name ) ) //unpublished group
      {
        continue;
      }

      QDomElement nameElem = doc.createElement( "Name" );
      QDomText nameText = doc.createTextNode( name );
      nameElem.appendChild( nameText );
      layerElem.appendChild( nameElem );

      QDomElement titleElem = doc.createElement( "Title" );
      QDomText titleText = doc.createTextNode( name );
      titleElem.appendChild( titleText );
      layerElem.appendChild( titleElem );

      if ( currentChildElem.attribute( "embedded" ) == "1" )
      {
        //add layers from other project files and embed into this group
        QString project = mProjectParser->convertToAbsolutePath( currentChildElem.attribute( "project" ) );
        QgsDebugMsg( QString( "Project path: %1" ).arg( project ) );
        QString embeddedGroupName = currentChildElem.attribute( "name" );
        QgsWMSProjectParser* p = dynamic_cast<QgsWMSProjectParser*>( QgsConfigCache::instance()->wmsConfiguration( project ) );
        if ( p )
        {
          QgsServerProjectParser* pp = p->mProjectParser;
          const QList<QDomElement>& embeddedGroupElements = pp->legendGroupElements();
          QStringList pIdDisabled = p->identifyDisabledLayers();

          QDomElement embeddedGroupElem;
          foreach ( const QDomElement &elem, embeddedGroupElements )
          {
            if ( elem.attribute( "name" ) == embeddedGroupName )
            {
              embeddedGroupElem = elem;
              break;
            }
          }

          QMap<QString, QgsMapLayer *> pLayerMap;
          const QList<QDomElement>& embeddedProjectLayerElements = pp->projectLayerElements();
          foreach ( const QDomElement &elem, embeddedProjectLayerElements )
          {
            pLayerMap.insert( pp->layerId( elem ), pp->createLayerFromElement( elem ) );
          }

          p->addLayers( doc, layerElem, embeddedGroupElem, pLayerMap, pIdDisabled, version, fullProjectSettings );
        }
      }
      else //normal (not embedded) legend group
      {
        addLayers( doc, layerElem, currentChildElem, layerMap, nonIdentifiableLayers, version, fullProjectSettings );
      }

      // combine bounding boxes of children (groups/layers)
      mProjectParser->combineExtentAndCrsOfGroupChildren( layerElem, doc );
    }
    else if ( currentChildElem.tagName() == "legendlayer" )
    {
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

      if ( mProjectParser->restrictedLayers().contains( mProjectParser->useLayerIDs() ? currentLayer->id() : currentLayer->name() ) ) //unpublished layer
      {
        continue;
      }
      // queryable layer
      if ( nonIdentifiableLayers.contains( currentLayer->id() ) )
      {
        layerElem.setAttribute( "queryable", "0" );
      }
      else
      {
        layerElem.setAttribute( "queryable", "1" );
      }

      QDomElement nameElem = doc.createElement( "Name" );
      //We use the layer name even though it might not be unique.
      //Because the id sometimes contains user/pw information and the name is more descriptive
      QDomText nameText = doc.createTextNode( mProjectParser->useLayerIDs() ? currentLayer->id() : currentLayer->name() );
      nameElem.appendChild( nameText );
      layerElem.appendChild( nameElem );

      QDomElement titleElem = doc.createElement( "Title" );
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
        QDomElement abstractElem = doc.createElement( "Abstract" );
        QDomText abstractText = doc.createTextNode( abstract );
        abstractElem.appendChild( abstractText );
        layerElem.appendChild( abstractElem );
      }

      //keyword list
      if ( !currentLayer->keywordList().isEmpty() )
      {
        QStringList keywordStringList = currentLayer->keywordList().split( "," );
        bool siaFormat = featureInfoFormatSIA2045();

        QDomElement keywordListElem = doc.createElement( "KeywordList" );
        for ( int i = 0; i < keywordStringList.size(); ++i )
        {
          QDomElement keywordElem = doc.createElement( "Keyword" );
          QDomText keywordText = doc.createTextNode( keywordStringList.at( i ).trimmed() );
          keywordElem.appendChild( keywordText );
          if ( siaFormat )
          {
            keywordElem.setAttribute( "vocabulary", "SIA_Geo405" );
          }
          keywordListElem.appendChild( keywordElem );
        }
        layerElem.appendChild( keywordListElem );
      }

      //vector layer without geometry
      bool geometryLayer = true;
      if ( currentLayer->type() == QgsMapLayer::VectorLayer )
      {
        QgsVectorLayer* vLayer = dynamic_cast<QgsVectorLayer*>( currentLayer );
        if ( vLayer )
        {
          if ( vLayer->wkbType() == QGis::WKBNoGeometry )
          {
            geometryLayer = false;
          }
        }
      }

      //CRS
      if ( geometryLayer )
      {
        QStringList crsList = QgsConfigParserUtils::createCRSListForLayer( currentLayer );
        QgsConfigParserUtils::appendCRSElementsToLayer( layerElem, doc, crsList, mProjectParser->supportedOutputCrsList() );

        //Ex_GeographicBoundingBox
        QgsConfigParserUtils::appendLayerBoundingBoxes( layerElem, doc, currentLayer->extent(), currentLayer->crs() );
      }

      // add details about supported styles of the layer
      addLayerStyles( currentLayer, doc, layerElem, version );

      //min/max scale denominatormScaleBasedVisibility
      if ( currentLayer->hasScaleBasedVisibility() )
      {
        if ( version == "1.1.1" )
        {
          double OGC_PX_M = 0.00028; // OGC reference pixel size in meter, also used by qgis
          double SCALE_TO_SCALEHINT = OGC_PX_M * sqrt( 2.0 );

          QDomElement scaleHintElem = doc.createElement( "ScaleHint" );
          scaleHintElem.setAttribute( "min", QString::number( currentLayer->minimumScale() * SCALE_TO_SCALEHINT ) );
          scaleHintElem.setAttribute( "max", QString::number( currentLayer->maximumScale() * SCALE_TO_SCALEHINT ) );
          layerElem.appendChild( scaleHintElem );
        }
        else
        {
          QString minScaleString = QString::number( currentLayer->minimumScale() );
          QDomElement minScaleElem = doc.createElement( "MinScaleDenominator" );
          QDomText minScaleText = doc.createTextNode( minScaleString );
          minScaleElem.appendChild( minScaleText );
          layerElem.appendChild( minScaleElem );

          QString maxScaleString = QString::number( currentLayer->maximumScale() );
          QDomElement maxScaleElem = doc.createElement( "MaxScaleDenominator" );
          QDomText maxScaleText = doc.createTextNode( maxScaleString );
          maxScaleElem.appendChild( maxScaleText );
          layerElem.appendChild( maxScaleElem );
        }
      }

      // layer attribution
      QString dataUrl = currentLayer->dataUrl();
      if ( !dataUrl.isEmpty() )
      {
        QDomElement dataUrlElem = doc.createElement( "DataURL" );
        QDomElement dataUrlFormatElem = doc.createElement( "Format" );
        QString dataUrlFormat = currentLayer->dataUrlFormat();
        QDomText dataUrlFormatText = doc.createTextNode( dataUrlFormat );
        dataUrlFormatElem.appendChild( dataUrlFormatText );
        dataUrlElem.appendChild( dataUrlFormatElem );
        QDomElement dataORElem = doc.createElement( "OnlineResource" );
        dataORElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
        dataORElem.setAttribute( "xlink:type", "simple" );
        dataORElem.setAttribute( "xlink:href", dataUrl );
        dataUrlElem.appendChild( dataORElem );
        layerElem.appendChild( dataUrlElem );
      }

      // layer attribution
      QString attribution = currentLayer->attribution();
      if ( !attribution.isEmpty() )
      {
        QDomElement attribElem = doc.createElement( "Attribution" );
        QDomElement attribTitleElem = doc.createElement( "Title" );
        QDomText attribText = doc.createTextNode( attribution );
        attribTitleElem.appendChild( attribText );
        attribElem.appendChild( attribTitleElem );
        QString attributionUrl = currentLayer->attributionUrl();
        if ( !attributionUrl.isEmpty() )
        {
          QDomElement attribORElem = doc.createElement( "OnlineResource" );
          attribORElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
          attribORElem.setAttribute( "xlink:type", "simple" );
          attribORElem.setAttribute( "xlink:href", attributionUrl );
          attribElem.appendChild( attribORElem );
        }
        layerElem.appendChild( attribElem );
      }

      // layer metadata URL
      QString metadataUrl = currentLayer->metadataUrl();
      if ( !metadataUrl.isEmpty() )
      {
        QDomElement metaUrlElem = doc.createElement( "MetadataURL" );
        QString metadataUrlType = currentLayer->metadataUrlType();
        if ( version == "1.1.1" )
        {
          metaUrlElem.setAttribute( "type", metadataUrlType );
        }
        else if ( metadataUrlType == "FGDC" )
        {
          metaUrlElem.setAttribute( "type", "FGDC:1998" );
        }
        else if ( metadataUrlType == "TC211" )
        {
          metaUrlElem.setAttribute( "type", "ISO19115:2003" );
        }
        else
        {
          metaUrlElem.setAttribute( "type", metadataUrlType );
        }
        QString metadataUrlFormat = currentLayer->metadataUrlFormat();
        if ( !metadataUrlFormat.isEmpty() )
        {
          QDomElement metaUrlFormatElem = doc.createElement( "Format" );
          QDomText metaUrlFormatText = doc.createTextNode( metadataUrlFormat );
          metaUrlFormatElem.appendChild( metaUrlFormatText );
          metaUrlElem.appendChild( metaUrlFormatElem );
        }
        QDomElement metaUrlORElem = doc.createElement( "OnlineResource" );
        metaUrlORElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
        metaUrlORElem.setAttribute( "xlink:type", "simple" );
        metaUrlORElem.setAttribute( "xlink:href", metadataUrl );
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


void QgsWMSProjectParser::addOWSLayerStyles( QgsMapLayer* currentLayer, QDomDocument& doc, QDomElement& layerElem ) const
{
  foreach ( QString styleName, currentLayer->styleManager()->styles() )
  {
    if ( styleName.isEmpty() )
      styleName = EMPTY_STYLE_NAME;

    QDomElement styleListElem = doc.createElement( "StyleList" );
    //only one default style in project file mode
    QDomElement styleElem = doc.createElement( "Style" );
    styleElem.setAttribute( "current", "true" );
    QDomElement styleNameElem = doc.createElement( "Name" );
    QDomText styleNameText = doc.createTextNode( styleName );
    styleNameElem.appendChild( styleNameText );
    QDomElement styleTitleElem = doc.createElement( "Title" );
    QDomText styleTitleText = doc.createTextNode( styleName );
    styleTitleElem.appendChild( styleTitleText );
    styleElem.appendChild( styleNameElem );
    styleElem.appendChild( styleTitleElem );
    styleListElem.appendChild( styleElem );
    layerElem.appendChild( styleListElem );
  }
}



void QgsWMSProjectParser::addOWSLayers( QDomDocument &doc,
                                        QDomElement &parentElem,
                                        const QDomElement &legendElem,
                                        const QMap<QString, QgsMapLayer *> &layerMap,
                                        const QStringList &nonIdentifiableLayers,
                                        const QString& strHref,
                                        QgsRectangle& combinedBBox,
                                        QString strGroup ) const
{
  const QgsCoordinateReferenceSystem& projectCrs = mProjectParser->projectCRS();
  QDomNodeList legendChildren = legendElem.childNodes();
  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    QDomElement currentChildElem = legendChildren.at( i ).toElement();

    if ( currentChildElem.tagName() == "legendgroup" )
    {
      QString name = currentChildElem.attribute( "name" );
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

      if ( currentChildElem.attribute( "embedded" ) == "1" )
      {
        //add layers from other project files and embed into this group
        QString project = mProjectParser->convertToAbsolutePath( currentChildElem.attribute( "project" ) );
        QgsDebugMsg( QString( "Project path: %1" ).arg( project ) );
        QString embeddedGroupName = currentChildElem.attribute( "name" );
        QgsWMSProjectParser* p = dynamic_cast<QgsWMSProjectParser*>( QgsConfigCache::instance()->wmsConfiguration( project ) );
        if ( p )
        {
          QgsServerProjectParser* pp = p->mProjectParser;
          const QList<QDomElement>& embeddedGroupElements = pp->legendGroupElements();
          QStringList pIdDisabled = p->identifyDisabledLayers();

          QDomElement embeddedGroupElem;
          foreach ( const QDomElement &elem, embeddedGroupElements )
          {
            if ( elem.attribute( "name" ) == embeddedGroupName )
            {
              embeddedGroupElem = elem;
              break;
            }
          }

          QMap<QString, QgsMapLayer *> pLayerMap;
          const QList<QDomElement>& embeddedProjectLayerElements = pp->projectLayerElements();
          foreach ( const QDomElement &elem, embeddedProjectLayerElements )
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
    else if ( currentChildElem.tagName() == "legendlayer" )
    {
      QDomElement layerElem = doc.createElement( "Layer" );
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

      if ( mProjectParser->restrictedLayers().contains( mProjectParser->useLayerIDs() ? currentLayer->id() : currentLayer->name() ) ) //unpublished layer
      {
        continue;
      }
      if ( nonIdentifiableLayers.contains( currentLayer->id() ) )
      {
        layerElem.setAttribute( "queryable", "false" );
      }
      else
      {
        layerElem.setAttribute( "queryable", "true" );
      }

      // is the layer visible ?
      if ( currentChildElem.firstChildElement().firstChildElement().attribute( "visible" ) == "1" )
      {
        layerElem.setAttribute( "hidden", "false" );
      }
      else
      {
        layerElem.setAttribute( "hidden", "true" );
      }

      if ( !strGroup.isEmpty() )
      {
        layerElem.setAttribute( "group", strGroup );
      }
      // Because Layer transparency is used for the rendering
      // OWSContext Layer opacity is set to 1
      layerElem.setAttribute( "opacity", 1 );

      QString lyrname = mProjectParser->useLayerIDs() ? currentLayer->id() : currentLayer->name();
      layerElem.setAttribute( "name", lyrname );

      // define an id based on layer name
      layerElem.setAttribute( "id", lyrname.replace( QRegExp( "[\\W]" ), "_" ) );

      QDomElement titleElem = doc.createElement( "ows:Title" );
      QString titleName = currentLayer->title();
      if ( titleName.isEmpty() )
      {
        titleName = currentLayer->name();
      }
      QDomText titleText = doc.createTextNode( titleName );
      titleElem.appendChild( titleText );
      layerElem.appendChild( titleElem );

      QDomElement formatElem = doc.createElement( "ows:OutputFormat" );
      QDomText formatText = doc.createTextNode( "image/png" );
      formatElem.appendChild( formatText );
      layerElem.appendChild( formatElem );

      QDomElement serverElem = doc.createElement( "Server" );
      serverElem.setAttribute( "service", "urn:ogc:serviceType:WMS" );
      serverElem.setAttribute( "version", "1.3.0" );
      serverElem.setAttribute( "default", "true" );
      QDomElement orServerElem = doc.createElement( "OnlineResource" );
      orServerElem.setAttribute( "xlink:href", strHref );
      serverElem.appendChild( orServerElem );
      layerElem.appendChild( serverElem );

      QString abstract = currentLayer->abstract();
      if ( !abstract.isEmpty() )
      {
        QDomElement abstractElem = doc.createElement( "ows:Abstract" );
        QDomText abstractText = doc.createTextNode( abstract );
        abstractElem.appendChild( abstractText );
        layerElem.appendChild( abstractElem );
      }

      //min/max scale denominatormScaleBasedVisibility
      if ( currentLayer->hasScaleBasedVisibility() )
      {
        QString minScaleString = QString::number( currentLayer->minimumScale() );
        QString maxScaleString = QString::number( currentLayer->maximumScale() );
        QDomElement minScaleElem = doc.createElement( "sld:MinScaleDenominator" );
        QDomText minScaleText = doc.createTextNode( minScaleString );
        minScaleElem.appendChild( minScaleText );
        layerElem.appendChild( minScaleElem );
        QDomElement maxScaleElem = doc.createElement( "sld:MaxScaleDenominator" );
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
      const QgsCoordinateReferenceSystem& layerCrs = currentLayer->crs();
      QgsCoordinateTransform t( layerCrs, projectCrs );

      //transform
      QgsRectangle BBox = t.transformBoundingBox( currentLayer->extent() );
      if ( combinedBBox.isEmpty() )
      {
        combinedBBox = BBox;
      }
      else
      {
        combinedBBox.combineExtentWith( &BBox );
      }

      addOWSLayerStyles( currentLayer, doc, layerElem );

      //keyword list
      if ( !currentLayer->keywordList().isEmpty() )
      {
        QStringList keywordStringList = currentLayer->keywordList().split( "," );
        bool siaFormat = featureInfoFormatSIA2045();

        QDomElement keywordsElem = doc.createElement( "ows:Keywords" );
        for ( int i = 0; i < keywordStringList.size(); ++i )
        {
          QDomElement keywordElem = doc.createElement( "ows:Keyword" );
          QDomText keywordText = doc.createTextNode( keywordStringList.at( i ).trimmed() );
          keywordElem.appendChild( keywordText );
          if ( siaFormat )
          {
            keywordElem.setAttribute( "vocabulary", "SIA_Geo405" );
          }
          keywordsElem.appendChild( keywordElem );
        }
        layerElem.appendChild( keywordsElem );
      }

      // layer data URL
      QString dataUrl = currentLayer->dataUrl();
      if ( !dataUrl.isEmpty() )
      {
        QDomElement dataUrlElem = doc.createElement( "DataURL" );
        QString dataUrlFormat = currentLayer->dataUrlFormat();
        dataUrlElem.setAttribute( "format", dataUrlFormat );
        QDomElement dataORElem = doc.createElement( "OnlineResource" );
        dataORElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
        dataORElem.setAttribute( "xlink:type", "simple" );
        dataORElem.setAttribute( "xlink:href", dataUrl );
        dataUrlElem.appendChild( dataORElem );
        layerElem.appendChild( dataUrlElem );
      }

      // layer metadata URL
      QString metadataUrl = currentLayer->metadataUrl();
      if ( !metadataUrl.isEmpty() )
      {
        QDomElement metaUrlElem = doc.createElement( "MetadataURL" );
        QString metadataUrlFormat = currentLayer->metadataUrlFormat();
        metaUrlElem.setAttribute( "format", metadataUrlFormat );
        QDomElement metaUrlORElem = doc.createElement( "OnlineResource" );
        metaUrlORElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
        metaUrlORElem.setAttribute( "xlink:type", "simple" );
        metaUrlORElem.setAttribute( "xlink:href", metadataUrl );
        metaUrlElem.appendChild( metaUrlORElem );
        layerElem.appendChild( metaUrlElem );
      }

      parentElem.appendChild( layerElem );
    }
    else
    {
      QgsDebugMsg( "unexpected child element" );
      continue;
    }

  }
}

int QgsWMSProjectParser::layersAndStyles( QStringList& layers, QStringList& styles ) const
{
  layers.clear();
  styles.clear();

  const QList<QDomElement>& projectLayerElements = mProjectParser->projectLayerElements();
  QList<QDomElement>::const_iterator elemIt = projectLayerElements.constBegin();

  QString currentLayerName;

  for ( ; elemIt != projectLayerElements.constEnd(); ++elemIt )
  {
    currentLayerName = mProjectParser->layerName( *elemIt );
    if ( !currentLayerName.isNull() )
    {
      layers << currentLayerName;
      styles << QString();
    }
  }
  return 0;
}

QDomDocument QgsWMSProjectParser::getStyle( const QString& styleName, const QString& layerName ) const
{
  Q_UNUSED( styleName );
  QStringList layerList;
  layerList.append( layerName );
  return getStyles( layerList );
}

QDomDocument QgsWMSProjectParser::getStyles( QStringList& layerList ) const
{
  QDomDocument myDocument = QDomDocument();

  QDomNode header = myDocument.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" );
  myDocument.appendChild( header );

  // Create the root element
  QDomElement root = myDocument.createElementNS( "http://www.opengis.net/sld", "StyledLayerDescriptor" );
  root.setAttribute( "version", "1.1.0" );
  root.setAttribute( "xsi:schemaLocation", "http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd" );
  root.setAttribute( "xmlns:ogc", "http://www.opengis.net/ogc" );
  root.setAttribute( "xmlns:se", "http://www.opengis.net/se" );
  root.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
  root.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
  myDocument.appendChild( root );

  for ( int i = 0; i < layerList.size(); i++ )
  {
    QString layerName;
    layerName = layerList.at( i );
    // don't use a cache - we may be changing styles
    QList<QgsMapLayer*> currentLayerList = mapLayerFromStyle( layerName, "", false );
    if ( currentLayerList.size() < 1 )
    {
      throw QgsMapServiceException( "Error", QString( "The layer for the TypeName '%1' is not found" ).arg( layerName ) );
    }
    for ( int j = 0; j < currentLayerList.size(); j++ )
    {
      QgsMapLayer* currentLayer = currentLayerList.at( j );
      QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>( currentLayer );
      if ( !layer )
      {
        throw QgsMapServiceException( "Error", QString( "Could not get style because:\n%1" ).arg( "Non-vector layers not supported yet" ) );
      }
      // Create the NamedLayer element
      QDomElement namedLayerNode = myDocument.createElement( "NamedLayer" );
      root.appendChild( namedLayerNode );

      // store the Name element
      QDomElement nameNode = myDocument.createElement( "se:Name" );
      nameNode.appendChild( myDocument.createTextNode( layerName ) );
      namedLayerNode.appendChild( nameNode );

      foreach ( QString styleName, layer->styleManager()->styles() )
      {
        if ( layer->hasGeometryType() )
        {
          layer->styleManager()->setCurrentStyle( styleName );
          if ( styleName.isEmpty() )
            styleName = EMPTY_STYLE_NAME;
          QDomElement styleElem = layer->rendererV2()->writeSld( myDocument, styleName );
          namedLayerNode.appendChild( styleElem );
        }
      }
    }
  }
  return myDocument;
}

QDomDocument QgsWMSProjectParser::describeLayer( QStringList& layerList, const QString& hrefString ) const
{
  QDomDocument myDocument = QDomDocument();

  QDomNode header = myDocument.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" );
  myDocument.appendChild( header );

  // Create the root element
  QDomElement root = myDocument.createElementNS( "http://www.opengis.net/sld", "DescribeLayerResponse" );
  root.setAttribute( "xsi:schemaLocation", "http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/DescribeLayer.xsd" );
  root.setAttribute( "xmlns:ows", "http://www.opengis.net/ows" );
  root.setAttribute( "xmlns:se", "http://www.opengis.net/se" );
  root.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
  root.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
  myDocument.appendChild( root );

  // store the Version element
  QDomElement versionNode = myDocument.createElement( "Version" );
  versionNode.appendChild( myDocument.createTextNode( "1.1.0" ) );
  root.appendChild( versionNode );

  //Prepare url
  QString wfsHrefString = mProjectParser->wfsServiceUrl();
  if ( wfsHrefString.isEmpty() )
  {
    wfsHrefString = hrefString;
  }
  QString wcsHrefString = mProjectParser->wcsServiceUrl();
  if ( wcsHrefString.isEmpty() )
  {
    wcsHrefString = hrefString;
  }

  //WFS layers
  QStringList wfsLayers = wfsLayerNames();
  //WCS layers
  QStringList wcsLayers = mProjectParser->wcsLayerNames();

  for ( int i = 0; i < layerList.size(); i++ )
  {
    QString layerName;
    layerName = layerList.at( i );
    // don't use a cache - we may be changing styles
    QList<QgsMapLayer*> currentLayerList = mapLayerFromStyle( layerName, "", false );
    if ( currentLayerList.size() < 1 )
    {
      throw QgsMapServiceException( "InvalidParameterValue", QString( "The layer '%1' is not found" ).arg( layerName ) );
    }
    for ( int j = 0; j < currentLayerList.size(); j++ )
    {
      QgsMapLayer* currentLayer = currentLayerList.at( j );
      QString layerTypeName = mProjectParser->useLayerIDs() ? currentLayer->id() : currentLayer->name();

      // Create the NamedLayer element
      QDomElement layerNode = myDocument.createElement( "LayerDescription" );
      root.appendChild( layerNode );

      // store the owsType element
      QDomElement typeNode = myDocument.createElement( "owsType" );
      // store the se:OnlineResource element
      QDomElement oResNode = myDocument.createElement( "se:OnlineResource" );
      oResNode.setAttribute( "xlink:type", "simple" );
      // store the TypeName element
      QDomElement nameNode = myDocument.createElement( "TypeName" );
      if ( currentLayer->type() == QgsMapLayer::VectorLayer )
      {
        typeNode.appendChild( myDocument.createTextNode( "wfs" ) );

        if ( wfsLayers.indexOf( layerTypeName ) != -1 )
        {
          oResNode.setAttribute( "xlink:href", wfsHrefString );
        }

        // store the se:FeatureTypeName element
        QDomElement typeNameNode = myDocument.createElement( "se:FeatureTypeName" );
        typeNameNode.appendChild( myDocument.createTextNode( layerTypeName ) );
        nameNode.appendChild( typeNameNode );
      }
      else if ( currentLayer->type() == QgsMapLayer::RasterLayer )
      {
        typeNode.appendChild( myDocument.createTextNode( "wcs" ) );

        if ( wcsLayers.indexOf( layerTypeName ) != -1 )
        {
          oResNode.setAttribute( "xlink:href", wcsHrefString );
        }

        // store the se:CoverageTypeName element
        QDomElement typeNameNode = myDocument.createElement( "se:CoverageTypeName" );
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

QgsMapRenderer::OutputUnits QgsWMSProjectParser::outputUnits() const
{
  return QgsMapRenderer::Millimeters;
}

bool QgsWMSProjectParser::featureInfoWithWktGeometry() const
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
  QDomElement wktElem = propertiesElem.firstChildElement( "WMSAddWktGeometry" );
  if ( wktElem.isNull() )
  {
    return false;
  }

  return ( wktElem.text().compare( "true", Qt::CaseInsensitive ) == 0 );
}

QHash<QString, QString> QgsWMSProjectParser::featureInfoLayerAliasMap() const
{
  QHash<QString, QString> aliasMap;
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return aliasMap;
  }

  //WMSFeatureInfoAliasLayers
  QStringList aliasLayerStringList;
  QDomElement featureInfoAliasLayersElem = propertiesElem.firstChildElement( "WMSFeatureInfoAliasLayers" );
  if ( featureInfoAliasLayersElem.isNull() )
  {
    return aliasMap;
  }
  QDomNodeList aliasLayerValueList = featureInfoAliasLayersElem.elementsByTagName( "value" );
  for ( int i = 0; i < aliasLayerValueList.size(); ++i )
  {
    aliasLayerStringList << aliasLayerValueList.at( i ).toElement().text();
  }

  //WMSFeatureInfoLayerAliases
  QStringList layerAliasStringList;
  QDomElement featureInfoLayerAliasesElem = propertiesElem.firstChildElement( "WMSFeatureInfoLayerAliases" );
  if ( featureInfoLayerAliasesElem.isNull() )
  {
    return aliasMap;
  }
  QDomNodeList layerAliasesValueList = featureInfoLayerAliasesElem.elementsByTagName( "value" );
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

QString QgsWMSProjectParser::featureInfoDocumentElement( const QString& defaultValue ) const
{
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return defaultValue;
  }
  QDomElement featureInfoDocumentElem = propertiesElem.firstChildElement( "WMSFeatureInfoDocumentElement" );
  if ( featureInfoDocumentElem.isNull() )
  {
    return defaultValue;
  }
  return featureInfoDocumentElem.text();
}

QString QgsWMSProjectParser::featureInfoDocumentElementNS() const
{
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return "";
  }
  QDomElement featureInfoDocumentNSElem = propertiesElem.firstChildElement( "WMSFeatureInfoDocumentElementNS" );
  if ( featureInfoDocumentNSElem.isNull() )
  {
    return "";
  }
  return featureInfoDocumentNSElem.text();
}

QString QgsWMSProjectParser::featureInfoSchema() const
{
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return "";
  }
  QDomElement featureInfoSchemaElem = propertiesElem.firstChildElement( "WMSFeatureInfoSchema" );
  if ( featureInfoSchemaElem.isNull() )
  {
    return "";
  }
  return featureInfoSchemaElem.text();
}


bool QgsWMSProjectParser::featureInfoFormatSIA2045() const
{
  QDomElement propertiesElem = mProjectParser->propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return false;
  }

  QDomElement sia2045Elem = propertiesElem.firstChildElement( "WMSInfoFormatSIA2045" );
  if ( sia2045Elem.isNull() )
  {
    return false;
  }

  if ( sia2045Elem.text().compare( "enabled", Qt::CaseInsensitive ) == 0
       || sia2045Elem.text().compare( "true", Qt::CaseInsensitive ) == 0 )
  {
    return true;
  }
  return false;
}

void QgsWMSProjectParser::drawOverlays( QPainter* p, int dpi, int width, int height ) const
{
  Q_UNUSED( width );
  Q_UNUSED( height );

  //consider DPI
  double scaleFactor = dpi / 88.0; //assume 88 as standard dpi
  QgsRectangle prjExtent = mProjectParser->projectExtent();

  //text annotations
  QList< QPair< QTextDocument*, QDomElement > >::const_iterator textIt = mTextAnnotationItems.constBegin();
  for ( ; textIt != mTextAnnotationItems.constEnd(); ++textIt )
  {
    QDomElement annotationElem = textIt->second;
    if ( annotationElem.isNull() )
    {
      continue;
    }

    int itemWidth = annotationElem.attribute( "frameWidth", "0" ).toInt();
    int itemHeight = annotationElem.attribute( "frameHeight", "0" ).toInt();

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
  QList< QPair< QSvgRenderer*, QDomElement > >::const_iterator svgIt = mSvgAnnotationElems.constBegin();
  QDomElement annotationElem;
  for ( ; svgIt != mSvgAnnotationElems.constEnd(); ++svgIt )
  {
    annotationElem = svgIt->second;
    int itemWidth = annotationElem.attribute( "frameWidth", "0" ).toInt() * scaleFactor;
    int itemHeight = annotationElem.attribute( "frameHeight", "0" ).toInt() * scaleFactor;

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
      double widthRatio = ( double )( itemWidth ) / ( double )( viewBox.width() );
      double heightRatio = ( double )( itemHeight ) / ( double )( viewBox.height() );
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

void QgsWMSProjectParser::loadLabelSettings( QgsLabelingEngineInterface* lbl ) const
{
  QgsPalLabeling* pal = dynamic_cast<QgsPalLabeling*>( lbl );
  if ( pal )
  {
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

    //pal::Pal default positions for candidates;
    int candPoint, candLine, candPoly;
    pal->numCandidatePositions( candPoint, candLine, candPoly );

    //mCandPoint
    QDomElement candPointElem = palElem.firstChildElement( "CandidatesPoint" );
    if ( !candPointElem.isNull() )
    {
      candPoint = candPointElem.text().toInt();
    }

    //mCandLine
    QDomElement candLineElem = palElem.firstChildElement( "CandidatesLine" );
    if ( !candLineElem.isNull() )
    {
      candLine = candLineElem.text().toInt();
    }

    //mCandPolygon
    QDomElement candPolyElem = palElem.firstChildElement( "CandidatesPolygon" );
    if ( !candPolyElem.isNull() )
    {
      candPoly = candPolyElem.text().toInt();
    }

    pal->setNumCandidatePositions( candPoint, candLine, candPoly );

    //mShowingCandidates
    QDomElement showCandElem = palElem.firstChildElement( "ShowingCandidates" );
    if ( !showCandElem.isNull() )
    {
      pal->setShowingCandidates( showCandElem.text().compare( "true", Qt::CaseInsensitive ) == 0 );
    }

    //mShowingAllLabels
    QDomElement showAllLabelsElem = palElem.firstChildElement( "ShowingAllLabels" );
    if ( !showAllLabelsElem.isNull() )
    {
      pal->setShowingAllLabels( showAllLabelsElem.text().compare( "true", Qt::CaseInsensitive ) == 0 );
    }

    //mShowingPartialsLabels
    QDomElement showPartialsLabelsElem = palElem.firstChildElement( "ShowingPartialsLabels" );
    if ( !showPartialsLabelsElem.isNull() )
    {
      pal->setShowingPartialsLabels( showPartialsLabelsElem.text().compare( "true", Qt::CaseInsensitive ) == 0 );
    }

    //mDrawOutlineLabels
    // TODO: This should probably always be true (already default) for WMS, regardless of any project setting.
    //       Not much sense to output text-as-text, when text-as-outlines gives better results.

    //save settings into global project instance (QgsMapRendererCustomPainterJob reads label settings from there)
    pal->saveEngineSettings();
  }
}

int QgsWMSProjectParser::nLayers() const
{
  return mProjectParser->numberOfLayers();
}

void QgsWMSProjectParser::serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const
{
  mProjectParser->serviceCapabilities( parentElement, doc, "WMS", featureInfoFormatSIA2045() );
}

QDomElement QgsWMSProjectParser::composerByName( const QString& composerName ) const
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
    if ( currentComposerElem.attribute( "title" ) == composerName )
    {
      return currentComposerElem;
    }
  }

  return composerElem;
}

QgsLayerTreeGroup* QgsWMSProjectParser::projectLayerTreeGroup() const
{
  const QDomDocument* projectDoc = mProjectParser->xmlDocument();
  if ( !projectDoc )
  {
    return 0;
  }

  QDomElement qgisElem = projectDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return 0;
  }
  QDomElement layerTreeElem = qgisElem.firstChildElement( "layer-tree-group" );
  if ( layerTreeElem.isNull() )
  {
    return 0;
  }
  return QgsLayerTreeGroup::readXML( layerTreeElem );
}

bool QgsWMSProjectParser::annotationPosition( const QDomElement& elem, double scaleFactor, double& xPos, double& yPos )
{
  Q_UNUSED( scaleFactor );

  xPos = elem.attribute( "canvasPosX" ).toDouble() / scaleFactor;
  yPos = elem.attribute( "canvasPosY" ).toDouble() / scaleFactor;
  return true;
}

void QgsWMSProjectParser::drawAnnotationRectangle( QPainter* p, const QDomElement& elem, double scaleFactor, double xPos, double yPos, int itemWidth, int itemHeight )
{
  Q_UNUSED( scaleFactor );
  if ( !p )
  {
    return;
  }

  QColor backgroundColor( elem.attribute( "frameBackgroundColor", "#000000" ) );
  backgroundColor.setAlpha( elem.attribute( "frameBackgroundColorAlpha", "255" ).toInt() );
  p->setBrush( QBrush( backgroundColor ) );
  QColor frameColor( elem.attribute( "frameColor", "#000000" ) );
  frameColor.setAlpha( elem.attribute( "frameColorAlpha", "255" ).toInt() );
  QPen framePen( frameColor );
  framePen.setWidth( elem.attribute( "frameBorderWidth", "1" ).toInt() );
  p->setPen( framePen );

  p->drawRect( QRectF( xPos, yPos, itemWidth, itemHeight ) );
}

void QgsWMSProjectParser::createTextAnnotationItems()
{
  cleanupTextAnnotationItems();

  const QDomDocument* xmlDoc = mProjectParser->xmlDocument();
  if ( !xmlDoc )
  {
    return;
  }

  //text annotations
  QDomElement qgisElem = xmlDoc->documentElement();
  QDomNodeList textAnnotationList = qgisElem.elementsByTagName( "TextAnnotationItem" );
  QDomElement textAnnotationElem;
  QDomElement annotationElem;
  for ( int i = 0; i < textAnnotationList.size(); ++i )
  {
    textAnnotationElem = textAnnotationList.at( i ).toElement();
    annotationElem = textAnnotationElem.firstChildElement( "AnnotationItem" );
    if ( !annotationElem.isNull() && annotationElem.attribute( "mapPositionFixed" ) != "1" )
    {
      QTextDocument* textDoc = new QTextDocument();
      textDoc->setHtml( textAnnotationElem.attribute( "document" ) );
      mTextAnnotationItems.push_back( qMakePair( textDoc, annotationElem ) );
    }
  }
}

void QgsWMSProjectParser::createSvgAnnotationItems()
{
  mSvgAnnotationElems.clear();
  const QDomDocument* xmlDoc = mProjectParser->xmlDocument();
  if ( !xmlDoc )
  {
    return;
  }

  QDomElement qgisElem = xmlDoc->documentElement();
  QDomNodeList svgAnnotationList = qgisElem.elementsByTagName( "SVGAnnotationItem" );
  QDomElement svgAnnotationElem;
  QDomElement annotationElem;
  for ( int i = 0; i < svgAnnotationList.size(); ++i )
  {
    svgAnnotationElem = svgAnnotationList.at( i ).toElement();
    annotationElem = svgAnnotationElem.firstChildElement( "AnnotationItem" );
    if ( !annotationElem.isNull() && annotationElem.attribute( "mapPositionFixed" ) != "1" )
    {
      QSvgRenderer* svg = new QSvgRenderer();
      if ( svg->load( mProjectParser->convertToAbsolutePath( svgAnnotationElem.attribute( "file" ) ) ) )
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

void QgsWMSProjectParser::cleanupSvgAnnotationItems()
{
  QList< QPair< QSvgRenderer*, QDomElement > >::const_iterator it = mSvgAnnotationElems.constBegin();
  for ( ; it != mSvgAnnotationElems.constEnd(); ++it )
  {
    delete it->first;
  }
  mSvgAnnotationElems.clear();
}

void QgsWMSProjectParser::cleanupTextAnnotationItems()
{
  QList< QPair< QTextDocument*, QDomElement > >::const_iterator it = mTextAnnotationItems.constBegin();
  for ( ; it != mTextAnnotationItems.constEnd(); ++it )
  {
    delete it->first;
  }
  mTextAnnotationItems.clear();
}

QString QgsWMSProjectParser::getCapaServiceUrl( QDomDocument& doc ) const
{
  QString url;
  QDomNodeList getCapNodeList = doc.elementsByTagName( "GetCapabilities" );
  if ( getCapNodeList.count() > 0 )
  {
    QDomElement getCapElem = getCapNodeList.at( 0 ).toElement();
    QDomNodeList getCapORNodeList = getCapElem.elementsByTagName( "OnlineResource" );
    if ( getCapORNodeList.count() > 0 )
    {
      url = getCapORNodeList.at( 0 ).toElement().attribute( "xlink:href", "" );
    }

  }

  return url;
}
