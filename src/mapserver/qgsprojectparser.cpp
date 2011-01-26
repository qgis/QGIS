/***************************************************************************
                              qgsprojectparser.cpp
                              --------------------
  begin                : May 27, 2010
  copyright            : (C) 2010 by Marco Hugentobler
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

#include "qgsprojectparser.h"
#include "qgsepsgcache.h"
#include "qgsmslayercache.h"
#include "qgsmapserverlogger.h"
#include "qgsmapserviceexception.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"

#include "qgscomposition.h"
#include "qgscomposerarrow.h"
#include "qgscomposerattributetable.h"
#include "qgscomposerlabel.h"
#include "qgscomposerlegend.h"
#include "qgscomposermap.h"
#include "qgscomposerpicture.h"
#include "qgscomposerscalebar.h"
#include "qgscomposershape.h"


QgsProjectParser::QgsProjectParser( QDomDocument* xmlDoc ): QgsConfigParser(), mXMLDoc( xmlDoc )
{
  mOutputUnits = QgsMapRenderer::Millimeters;
  setLegendParametersFromProject();
}

QgsProjectParser::~QgsProjectParser()
{
  delete mXMLDoc;
}

void QgsProjectParser::layersAndStylesCapabilities( QDomElement& parentElement, QDomDocument& doc ) const
{
  QList<QDomElement> layerElems = projectLayerElements();

  QStringList nonIdentifiableLayers = identifyDisabledLayers();

  if ( layerElems.size() < 1 )
  {
    return;
  }

  QMap<QString, QgsMapLayer *> layerMap;

  QList<QDomElement>::const_iterator layerIt = layerElems.constBegin();
  for ( ; layerIt != layerElems.constEnd(); ++layerIt )
  {
    QgsMapLayer *layer = createLayerFromElement( *layerIt );
    if ( layer )
    {
      QgsMSDebugMsg( QString( "add layer %1 to map" ).arg( layer->getLayerID() ) );
      layerMap.insert( layer->getLayerID(), layer );
    }
#if QGSMSDEBUG
    else
    {
      QString buf;
      QTextStream s( &buf );
      layerIt->save( s, 0 );
      QgsMSDebugMsg( QString( "layer %1 not found" ).arg( buf ) );
    }
#endif
  }

  //According to the WMS spec, there can be only one toplevel layer.
  //So we create an artificial one here to be in accordance with the schema
  QString projTitle = projectTitle();
  QDomElement layerParentElem = doc.createElement( "Layer" );
  QDomElement layerParentNameElem = doc.createElement( "Name" );
  QDomText layerParentNameText = doc.createTextNode( projTitle );
  layerParentNameElem.appendChild( layerParentNameText );
  layerParentElem.appendChild( layerParentNameElem );
  QDomElement layerParentTitleElem = doc.createElement( "Title" );
  QDomText layerParentTitleText = doc.createTextNode( projTitle );
  layerParentTitleElem.appendChild( layerParentTitleText );
  layerParentElem.appendChild( layerParentTitleElem );

  //Map rectangle. If not empty, this will be set for every layer (instead of the bbox that comes from the data)
  QgsRectangle mapExtent = mapRectangle();
  QgsCoordinateReferenceSystem mapCRS;
  if ( !mapExtent.isEmpty() )
  {
    int epsg = mapEpsg();
    if ( epsg != -1 )
    {
      mapCRS.createFromEpsg( epsg );
      appendExGeographicBoundingBox( layerParentElem, doc, mapExtent, mapCRS );
    }
  }

  QDomElement legendElem = mXMLDoc->documentElement().firstChildElement( "legend" );

  addLayers( doc, layerParentElem, legendElem, layerMap, nonIdentifiableLayers, mapExtent, mapCRS );

  parentElement.appendChild( layerParentElem );
}

void QgsProjectParser::addLayers( QDomDocument &doc,
                                  QDomElement &parentElem,
                                  const QDomElement &legendElem,
                                  const QMap<QString, QgsMapLayer *> &layerMap,
                                  const QStringList &nonIdentifiableLayers,
                                  const QgsRectangle &mapExtent,
                                  const QgsCoordinateReferenceSystem &mapCRS ) const
{
  QDomNodeList legendChildren = legendElem.childNodes();
  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    QDomElement currentChildElem = legendChildren.at( i ).toElement();

    QDomElement layerElem = doc.createElement( "Layer" );

    if ( currentChildElem.tagName() == "legendgroup" )
    {
      QString name = currentChildElem.attribute( "name" );
      QDomElement nameElem = doc.createElement( "Name" );
      QDomText nameText = doc.createTextNode( name );
      nameElem.appendChild( nameText );
      layerElem.appendChild( nameElem );

      QDomElement titleElem = doc.createElement( "Title" );
      QDomText titleText = doc.createTextNode( name );
      titleElem.appendChild( titleText );
      layerElem.appendChild( titleElem );

      addLayers( doc, layerElem, currentChildElem, layerMap, nonIdentifiableLayers, mapExtent, mapCRS );

      // combine bounding boxes of childs (groups/layers)

      QgsRectangle combinedGeographicBBox;
      QSet<int> combinedCRSSet;
      bool firstBBox = true;
      bool firstCRSSet = true;

      QDomNodeList layerChildren = layerElem.childNodes();
      for ( int j = 0; j < layerChildren.size(); ++j )
      {
        QDomElement childElem = layerChildren.at( j ).toElement();

        if ( childElem.tagName() != "Layer" )
          continue;

        QgsRectangle bbox;
        if ( exGeographicBoundingBox( childElem, bbox ) )
        {
          if ( firstBBox )
          {
            combinedGeographicBBox = bbox;
            firstBBox = false;
          }
          else
          {
            combinedGeographicBBox.combineExtentWith( &bbox );
          }
        }

        //combine crs set
        QSet<int> crsSet;
        if ( crsSetForLayer( childElem, crsSet ) )
        {
          if ( firstCRSSet )
          {
            combinedCRSSet = crsSet;
            firstCRSSet = false;
          }
          else
          {
            combinedCRSSet.intersect( crsSet );
          }
        }
      }

      appendCRSElementsToLayer( layerElem, doc, combinedCRSSet.toList() );

      const QgsCoordinateReferenceSystem& groupCRS = QgsEPSGCache::instance()->searchCRS( 4326 );
      appendExGeographicBoundingBox( layerElem, doc, combinedGeographicBBox, groupCRS );
    }
    else if ( currentChildElem.tagName() == "legendlayer" )
    {
      QString id = layerIdFromLegendLayer( currentChildElem );
      QgsMapLayer *currentLayer = layerMap[ id ];
      if ( !currentLayer )
      {
        QgsMSDebugMsg( QString( "layer %1 not found" ).arg( id ) );
        continue;
      }

      if ( nonIdentifiableLayers.contains( currentLayer->getLayerID() ) )
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
      QDomText nameText = doc.createTextNode( currentLayer->name() );
      nameElem.appendChild( nameText );
      layerElem.appendChild( nameElem );

      QDomElement titleElem = doc.createElement( "Title" );
      QDomText titleText = doc.createTextNode( currentLayer->name() );
      titleElem.appendChild( titleText );
      layerElem.appendChild( titleElem );

      QDomElement abstractElem = doc.createElement( "Abstract" );
      layerElem.appendChild( abstractElem );

      //CRS
      QList<int> crsList = createCRSListForLayer( currentLayer );
      appendCRSElementsToLayer( layerElem, doc, crsList );

      //Ex_GeographicBoundingBox
      if ( mapExtent.isEmpty() )
      {
        appendExGeographicBoundingBox( layerElem, doc, currentLayer->extent(), currentLayer->srs() );
      }
      else
      {
        appendExGeographicBoundingBox( layerElem, doc, mapExtent, mapCRS );
      }

      //only one default style in project file mode
      QDomElement styleElem = doc.createElement( "Style" );
      QDomElement styleNameElem = doc.createElement( "Name" );
      QDomText styleNameText = doc.createTextNode( "default" );
      styleNameElem.appendChild( styleNameText );
      QDomElement styleTitleElem = doc.createElement( "Title" );
      QDomText styleTitleText = doc.createTextNode( "default" );
      styleTitleElem.appendChild( styleTitleText );
      styleElem.appendChild( styleNameElem );
      styleElem.appendChild( styleTitleElem );
      layerElem.appendChild( styleElem );
    }
    else
    {
      QgsMSDebugMsg( "unexpected child element" );
      continue;
    }


#if QGSMSDEBUG
    QString buf;
    QTextStream s( &buf );
    layerElem.save( s, 0 );
    QgsMSDebugMsg( QString( "adding layer: %1" ).arg( buf ) );
#endif

    parentElem.appendChild( layerElem );
  }
}


QList<QgsMapLayer*> QgsProjectParser::mapLayerFromStyle( const QString& lName, const QString& styleName, bool allowCaching ) const
{
  QList<QgsMapLayer*> layerList;
  bool layerFound = false;

  //first assume lName refers to a leaf layer
  QMap< QString, QDomElement > layerElemMap = projectLayerElementsByName();
  QMap< QString, QDomElement >::const_iterator layerElemIt = layerElemMap.find( lName );
  if ( layerElemIt != layerElemMap.constEnd() )
  {
    QgsMapLayer* layer = createLayerFromElement( layerElemIt.value() );
    if ( layer )
    {
      layerList.push_back( layer );
    }
    layerFound = true;
  }

  if ( layerFound )
  {
    return layerList;
  }

  //Check if layer name refers to the top level group for the project.
  //The project group is not contained in the groupLayerRelationship list
  //because the list (and the qgis legend) does not support nested groups
  if ( lName == projectTitle() )
  {
    QList<QDomElement> layerElemList = projectLayerElements();
    QList<QDomElement>::const_iterator layerElemIt = layerElemList.constBegin();
    for ( ; layerElemIt != layerElemList.constEnd(); ++layerElemIt )
    {
      layerList.push_back( createLayerFromElement( *layerElemIt ) );
    }
    return layerList;
  }

  //maybe the layer is a goup. Check if lName is contained in the group list
  QMap< QString, QDomElement > idLayerMap = projectLayerElementsById();

  QList<QDomElement> legendGroups = legendGroupElements();
  QList<QDomElement>::const_iterator groupIt = legendGroups.constBegin();
  for ( ; groupIt != legendGroups.constEnd(); ++groupIt )
  {
    if ( groupIt->attribute( "name" ) == lName )
    {
      QDomNodeList layerFileList = groupIt->elementsByTagName( "legendlayerfile" );
      for ( int i = 0; i < layerFileList.size(); ++i )
      {
        QMap< QString, QDomElement >::const_iterator layerEntry = idLayerMap.find( layerFileList.at( i ).toElement().attribute( "layerid" ) );
        if ( layerEntry != idLayerMap.constEnd() )
        {
          layerList.push_back( createLayerFromElement( layerEntry.value() ) );
        }
      }
    }
  }

  return layerList;
}

int QgsProjectParser::layersAndStyles( QStringList& layers, QStringList& styles ) const
{
  layers.clear();
  styles.clear();

  QList<QDomElement> layerElemList = projectLayerElements();
  QList<QDomElement>::const_iterator elemIt = layerElemList.constBegin();

  QString currentLayerName;

  for ( ; elemIt != layerElemList.constEnd(); ++elemIt )
  {
    currentLayerName = layerName( *elemIt );
    if ( !currentLayerName.isNull() )
    {
      layers << currentLayerName;
      styles << "default";
    }
  }
  return 0;
}

QDomDocument QgsProjectParser::getStyle( const QString& styleName, const QString& layerName ) const
{
  QDomDocument doc;
  return doc; //soon...
}

QgsMapRenderer::OutputUnits QgsProjectParser::outputUnits() const
{
  return QgsMapRenderer::Millimeters;
}

QStringList QgsProjectParser::identifyDisabledLayers() const
{
  QStringList disabledList;
  if ( !mXMLDoc )
  {
    return disabledList;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
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

QSet<int> QgsProjectParser::supportedOutputCrsSet() const
{
  QSet<int> epsgSet;
  if ( !mXMLDoc )
  {
    return epsgSet;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return epsgSet;
  }
  QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return epsgSet;
  }
  QDomElement wmsEpsgElem = propertiesElem.firstChildElement( "WMSEpsgList" );
  if ( wmsEpsgElem.isNull() )
  {
    return epsgSet;
  }
  QDomNodeList valueList = wmsEpsgElem.elementsByTagName( "value" );
  bool conversionOk;
  for ( int i = 0; i < valueList.size(); ++i )
  {
    int epsgNr = valueList.at( i ).toElement().text().toInt( &conversionOk );
    if ( conversionOk )
    {
      epsgSet.insert( epsgNr );
    }
  }
  return epsgSet;
}

QMap< QString, QMap< int, QString > > QgsProjectParser::layerAliasInfo() const
{
  QMap< QString, QMap< int, QString > > resultMap;

  QList<QDomElement> layerElems = projectLayerElements();
  QList<QDomElement>::const_iterator layerIt = layerElems.constBegin();
  for ( ; layerIt != layerElems.constEnd(); ++layerIt )
  {
    QDomNodeList aNodeList = layerIt->elementsByTagName( "aliases" );
    if ( aNodeList.size() > 0 )
    {
      QMap<int, QString> aliasMap;
      QDomNodeList aliasNodeList = aNodeList.at( 0 ).toElement().elementsByTagName( "alias" );
      for ( int i = 0; i < aliasNodeList.size(); ++i )
      {
        QDomElement aliasElem = aliasNodeList.at( i ).toElement();
        aliasMap.insert( aliasElem.attribute( "index" ).toInt(), aliasElem.attribute( "name" ) );
      }
      resultMap.insert( layerId( *layerIt ) , aliasMap );
    }
  }

  return resultMap;
}

QMap< QString, QSet<QString> > QgsProjectParser::hiddenAttributes() const
{
  QMap< QString, QSet<QString> > resultMap;
  QList<QDomElement> layerElems = projectLayerElements();
  QList<QDomElement>::const_iterator layerIt = layerElems.constBegin();
  for ( ; layerIt != layerElems.constEnd(); ++layerIt )
  {
    QDomNodeList editTypesList = layerIt->elementsByTagName( "edittypes" );
    if ( editTypesList.size() > 0 )
    {
      QSet< QString > hiddenAttributes;
      QDomElement editTypesElem = editTypesList.at( 0 ).toElement();
      QDomNodeList editTypeList = editTypesElem.elementsByTagName( "edittype" );
      for ( int i = 0; i < editTypeList.size(); ++i )
      {
        QDomElement editTypeElem = editTypeList.at( i ).toElement();
        if ( editTypeElem.attribute( "type" ).toInt() == QgsVectorLayer::Hidden )
        {
          hiddenAttributes.insert( editTypeElem.attribute( "name" ) );
        }
      }

      if ( hiddenAttributes.size() > 0 )
      {
        resultMap.insert( layerId( *layerIt ), hiddenAttributes );
      }
    }
  }

  return resultMap;
}

QgsRectangle QgsProjectParser::mapRectangle() const
{
  if ( !mXMLDoc )
  {
    return QgsRectangle();
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return QgsRectangle();
  }

  QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return QgsRectangle();
  }

  QDomElement extentElem = propertiesElem.firstChildElement( "WMSExtent" );
  if ( extentElem.isNull() )
  {
    return QgsRectangle();
  }

  QDomNodeList valueNodeList = extentElem.elementsByTagName( "value" );
  if ( valueNodeList.size() < 4 )
  {
    return QgsRectangle();
  }

  //order of value elements must be xmin, ymin, xmax, ymax
  double xmin = valueNodeList.at( 0 ).toElement().text().toDouble();
  double ymin = valueNodeList.at( 1 ).toElement().text().toDouble();
  double xmax = valueNodeList.at( 2 ).toElement().text().toDouble();
  double ymax = valueNodeList.at( 3 ).toElement().text().toDouble();
  return QgsRectangle( xmin, ymin, xmax, ymax );
}

int QgsProjectParser::mapEpsg() const
{
  if ( !mXMLDoc )
  {
    return -1;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return -1;
  }

  QDomElement mapCanvasElem = qgisElem.firstChildElement( "mapcanvas" );
  if ( mapCanvasElem.isNull() )
  {
    return -1;
  }

  QDomElement srsElem = mapCanvasElem.firstChildElement( "destinationsrs" );
  if ( srsElem.isNull() )
  {
    return -1;
  }

  QDomNodeList authIdNodes = srsElem.elementsByTagName( "authid" );
  if ( authIdNodes.size() < 1 )
  {
    return -1;
  }

  QStringList authIdSplit = authIdNodes.at( 0 ).toElement().text().split( ":" );
  if ( authIdSplit.size() < 2 )
  {
    return -1;
  }
  return authIdSplit.at( 1 ).toInt();
}

QString QgsProjectParser::projectTitle() const
{
  if ( !mXMLDoc )
  {
    return QString();
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return QString();
  }

  QDomElement titleElem = qgisElem.firstChildElement( "title" );
  if ( titleElem.isNull() )
  {
    return QString();
  }

  return titleElem.text();
}

QList<QDomElement> QgsProjectParser::projectLayerElements() const
{
  QList<QDomElement> layerElemList;
  if ( !mXMLDoc )
  {
    return layerElemList;
  }

  QDomNodeList layerNodeList = mXMLDoc->elementsByTagName( "maplayer" );
  int nNodes = layerNodeList.size();
  for ( int i = 0; i < nNodes; ++i )
  {
    layerElemList.push_back( layerNodeList.at( i ).toElement() );
  }
  return layerElemList;
}

QList<QDomElement> QgsProjectParser::legendGroupElements() const
{
  QList<QDomElement> groupList;
  if ( !mXMLDoc )
  {
    return groupList;
  }

  QDomElement legendElement = mXMLDoc->documentElement().firstChildElement( "legend" );
  if ( legendElement.isNull() )
  {
    return groupList;
  }

  QDomNodeList groupNodeList = legendElement.elementsByTagName( "legendgroup" );
  for ( int i = 0; i < groupNodeList.size(); ++i )
  {
    groupList.push_back( groupNodeList.at( i ).toElement() );
  }

  return groupList;
}

QMap< QString, QDomElement > QgsProjectParser::projectLayerElementsById() const
{
  QMap< QString, QDomElement > layerMap;
  if ( !mXMLDoc )
  {
    return layerMap;
  }

  QDomNodeList layerNodeList = mXMLDoc->elementsByTagName( "maplayer" );
  QDomElement currentElement;
  int nNodes = layerNodeList.size();
  for ( int i = 0; i < nNodes; ++i )
  {
    currentElement = layerNodeList.at( i ).toElement();
    layerMap.insert( layerId( currentElement ), currentElement );
  }
  return layerMap;
}

QMap< QString, QDomElement > QgsProjectParser::projectLayerElementsByName() const
{
  QMap< QString, QDomElement > layerMap;
  if ( !mXMLDoc )
  {
    return layerMap;
  }

  QDomNodeList layerNodeList = mXMLDoc->elementsByTagName( "maplayer" );
  QDomElement currentElement;
  int nNodes = layerNodeList.size();
  for ( int i = 0; i < nNodes; ++i )
  {
    currentElement = layerNodeList.at( i ).toElement();
    layerMap.insert( layerName( currentElement ), currentElement );
  }
  return layerMap;
}

QgsMapLayer* QgsProjectParser::createLayerFromElement( const QDomElement& elem ) const
{
  if ( elem.isNull() )
  {
    return 0;
  }

  QDomElement dataSourceElem = elem.firstChildElement( "datasource" );
  if ( dataSourceElem.isNull() )
  {
    return 0;
  }

  QString uri = dataSourceElem.text();
  QString id = layerId( elem );
  if ( id.isNull() )
  {
    return 0;
  }

  QgsMapLayer* layer = QgsMSLayerCache::instance()->searchLayer( uri, id );
  if ( layer )
  {
    //reading symbology every time is necessary because it could have been changed by a user SLD based request
    QString error;
    layer->readSymbology( elem, error );
    return layer;
  }

  QString type = elem.attribute( "type" );
  if ( type == "vector" )
  {
    layer = new QgsVectorLayer();
  }
  else if ( type == "raster" )
  {
    layer = new QgsRasterLayer();
  }

  if ( layer )
  {
    layer->readXML( const_cast<QDomElement&>( elem ) ); //should be changed to const in QgsMapLayer
    layer->setLayerName( layerName( elem ) );
    QgsMSLayerCache::instance()->insertLayer( uri, id, layer );
  }
  return layer;
}

QString QgsProjectParser::layerId( const QDomElement& layerElem ) const
{
  if ( layerElem.isNull() )
  {
    return QString();
  }

  QDomElement idElem = layerElem.firstChildElement( "id" );
  if ( idElem.isNull() )
  {
    return QString();
  }
  return idElem.text();
}

QString QgsProjectParser::layerName( const QDomElement& layerElem ) const
{
  if ( layerElem.isNull() )
  {
    return QString();
  }

  QDomElement nameElem = layerElem.firstChildElement( "layername" );
  if ( nameElem.isNull() )
  {
    return QString();
  }
  return nameElem.text().replace( "," , "%60" ); //commas are not allowed in layer names
}

void QgsProjectParser::setLegendParametersFromProject()
{
  if ( !mXMLDoc )
  {
    return;
  }

  QDomElement documentElem = mXMLDoc->documentElement();
  if ( documentElem.isNull() )
  {
    return;
  }

  QDomElement composerElem = documentElem.firstChildElement( "Composer" );
  if ( composerElem.isNull() )
  {
    return;
  }
  QDomElement composerLegendElem = composerElem.firstChildElement( "ComposerLegend" );
  if ( composerLegendElem.isNull() )
  {
    return;
  }

  mLegendBoxSpace = composerLegendElem.attribute( "boxSpace" ).toDouble();
  mLegendLayerSpace = composerLegendElem.attribute( "layerSpace" ).toDouble();
  mLegendSymbolSpace = composerLegendElem.attribute( "symbolSpace" ).toDouble();
  mLegendIconLabelSpace = composerLegendElem.attribute( "iconLabelSpace" ).toDouble();
  mLegendSymbolWidth = composerLegendElem.attribute( "symbolWidth" ).toDouble();
  mLegendSymbolHeight = composerLegendElem.attribute( "symbolHeight" ).toDouble();
  mLegendLayerFont.fromString( composerLegendElem.attribute( "layerFont" ) );
  mLegendItemFont.fromString( composerLegendElem.attribute( "itemFont" ) );
}

QList< GroupLayerInfo > QgsProjectParser::groupLayerRelationshipFromProject() const
{
  QList< GroupLayerInfo > resultList;
  if ( !mXMLDoc )
  {
    return resultList;
  }

  QDomElement documentElem = mXMLDoc->documentElement();
  if ( documentElem.isNull() )
  {
    return resultList;
  }
  QDomElement legendElem = documentElem.firstChildElement( "legend" );
  if ( legendElem.isNull() )
  {
    return resultList;
  }

  QDomNodeList legendChildren = legendElem.childNodes();
  QDomElement currentChildElem;
  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    QList< QString > layerIdList;
    currentChildElem = legendChildren.at( i ).toElement();
    if ( currentChildElem.isNull() )
    {
      continue;
    }
    else if ( currentChildElem.tagName() == "legendlayer" )
    {
      layerIdList.push_back( layerIdFromLegendLayer( currentChildElem ) );
      resultList.push_back( qMakePair( QString(), layerIdList ) );
    }
    else if ( currentChildElem.tagName() == "legendgroup" )
    {
      QDomNodeList childLayerList = currentChildElem.elementsByTagName( "legendlayer" );
      QString groupName = currentChildElem.attribute( "name" ).replace( "," , "%60" );
      QString currentLayerId;

      for ( int j = 0; j < childLayerList.size(); ++j )
      {
        layerIdList.push_back( layerIdFromLegendLayer( childLayerList.at( j ).toElement() ) );
      }
      resultList.push_back( qMakePair( groupName, layerIdList ) );
    }
  }

  return resultList;
}

QString QgsProjectParser::layerIdFromLegendLayer( const QDomElement& legendLayer ) const
{
  if ( legendLayer.isNull() )
  {
    return QString();
  }

  QDomNodeList legendLayerFileList = legendLayer.elementsByTagName( "legendlayerfile" );
  if ( legendLayerFileList.size() < 1 )
  {
    return QString();
  }

  return legendLayerFileList.at( 0 ).toElement().attribute( "layerid" );
}

QgsComposition* QgsProjectParser::initComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, QList< QgsComposerMap*>& mapList, QList< QgsComposerLabel* >& labelList ) const
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

  QgsComposition* composition = new QgsComposition( mapRenderer ); //set resolution, paper size from composer element attributes
  if ( !composition->readXML( compositionElem, *mXMLDoc ) )
  {
    delete composition;
    return 0;
  }

  //go through all the item elements and add them to the composition (and to the lists)
  QDomNodeList itemNodes = composerElem.childNodes();
  for ( int i = 0; i < itemNodes.size(); ++i )
  {
    QDomElement currentElem = itemNodes.at( i ).toElement();
    QString elemName = currentElem.tagName();
    if ( elemName == "ComposerMap" )
    {
      QgsComposerMap* map = new QgsComposerMap( composition );
      map->readXML( currentElem, *mXMLDoc );
      composition->addItem( map );
      mapList.push_back( map );
    }
    else if ( elemName == "ComposerLabel" )
    {
      QgsComposerLabel* label = new QgsComposerLabel( composition );
      label->readXML( currentElem, *mXMLDoc );
      composition->addItem( label );
      labelList.push_back( label );
    }
    else if ( elemName == "ComposerLegend" )
    {
      //legend needs to be loaded indirectly to have generic content
      //and to avoid usage of x-server with pixmap icons
      QgsComposerLegend* legend = new QgsComposerLegend( composition );
      legend->_readXML( currentElem.firstChildElement( "ComposerItem" ), *mXMLDoc );
      legend->updateLegend();
      composition->addItem( legend );
    }
    else if ( elemName == "ComposerShape" )
    {
      QgsComposerShape* shape = new QgsComposerShape( composition );
      shape->readXML( currentElem, *mXMLDoc );
      composition->addItem( shape );
    }
    else if ( elemName == "ComposerArrow" )
    {
      QgsComposerArrow* arrow = new QgsComposerArrow( composition );
      arrow->readXML( currentElem, *mXMLDoc );
      composition->addItem( arrow );
    }
    else if ( elemName == "ComposerAttributeTable" )
    {
      QgsComposerAttributeTable* table = new QgsComposerAttributeTable( composition );
      table->readXML( currentElem, *mXMLDoc );
      composition->addItem( table );
    }
  }

  //scalebars and pictures need to be loaded after the maps to receive the correct size / rotation
  for ( int i = 0; i < itemNodes.size(); ++i )
  {
    QDomElement currentElem = itemNodes.at( i ).toElement();
    QString elemName = currentElem.tagName();
    if ( elemName == "ComposerPicture" )
    {
      QgsComposerPicture* picture = new QgsComposerPicture( composition );
      picture->readXML( currentElem, *mXMLDoc );
      composition->addItem( picture );
    }
    else if ( elemName == "ComposerScaleBar" )
    {
      QgsComposerScaleBar* bar = new QgsComposerScaleBar( composition );
      bar->readXML( currentElem, *mXMLDoc );
      composition->addItem( bar );
    }
  }

  return composition;
}

void QgsProjectParser::printCapabilities( QDomElement& parentElement, QDomDocument& doc ) const
{
  if ( !mXMLDoc )
  {
    return;
  }

  QDomNodeList composerNodeList = mXMLDoc->elementsByTagName( "Composer" );
  if ( composerNodeList.size() < 1 )
  {
    return;
  }

  QDomElement composerTemplatesElem = doc.createElement( "ComposerTemplates" );
  composerTemplatesElem.setAttribute( "xmlns:wms", "http://www.opengis.net/wms" );
  composerTemplatesElem.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
  composerTemplatesElem.setAttribute( "xsi:type", "wms:_ExtendedCapabilities" );

  for ( int i = 0; i < composerNodeList.size(); ++i )
  {
    QDomElement composerTemplateElem = doc.createElement( "ComposerTemplate" );
    QDomElement currentComposerElem = composerNodeList.at( i ).toElement();
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
      QDomElement clabel = composerLabelList.at( j ).toElement();
      QString id = clabel.attribute( "id" );
      if ( id.isEmpty() ) //only export labels with ids for text replacement
      {
        continue;
      }
      QDomElement composerLabelElem = doc.createElement( "ComposerLabel" );
      composerLabelElem.setAttribute( "name", id );
      composerTemplateElem.appendChild( composerLabelElem );
    }

    composerTemplatesElem.appendChild( composerTemplateElem );
  }
  parentElement.appendChild( composerTemplatesElem );
}

QDomElement QgsProjectParser::composerByName( const QString& composerName ) const
{
  QDomElement composerElem;
  if ( !mXMLDoc )
  {
    return composerElem;
  }

  QDomNodeList composerNodeList = mXMLDoc->elementsByTagName( "Composer" );
  for ( int i = 0; i < composerNodeList.size(); ++i )
  {
    QDomElement currentComposerElem = composerNodeList.at( i ).toElement();
    if ( currentComposerElem.attribute( "title" ) == composerName )
    {
      return currentComposerElem;
    }
  }

  return composerElem;
}

void QgsProjectParser::serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const
{
  QDomElement propertiesElem = mXMLDoc->documentElement().firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    QgsConfigParser::serviceCapabilities( parentElement, doc );
    return;
  }

  QDomElement serviceCapabilityElem = propertiesElem.firstChildElement( "WMSServiceCapabilities" );
  if ( serviceCapabilityElem.isNull() || serviceCapabilityElem.text().compare( "true", Qt::CaseInsensitive ) != 0 )
  {
    QgsConfigParser::serviceCapabilities( parentElement, doc );
    return;
  }

  //Service name is always WMS
  QDomElement wmsNameElem = doc.createElement( "Name" );
  QDomText wmsNameText = doc.createTextNode( "WMS" );
  wmsNameElem.appendChild( wmsNameText );
  parentElement.appendChild( wmsNameElem );

  //WMS title
  QDomElement titleElem = propertiesElem.firstChildElement( "WMSServiceTitle" );
  if ( !titleElem.isNull() )
  {
    QDomElement wmsTitleElem = doc.createElement( "Title" );
    QDomText wmsTitleText = doc.createTextNode( titleElem.text() );
    wmsTitleElem.appendChild( wmsTitleText );
    parentElement.appendChild( wmsTitleElem );
  }

  //WMS abstract
  QDomElement abstractElem = propertiesElem.firstChildElement( "WMSServiceAbstract" );
  if ( !abstractElem.isNull() )
  {
    QDomElement wmsAbstractElem = doc.createElement( "Abstract" );
    QDomText wmsAbstractText = doc.createTextNode( abstractElem.text() );
    wmsAbstractElem.appendChild( wmsAbstractText );
    parentElement.appendChild( wmsAbstractElem );
  }

  //Contact information
  QDomElement contactInfoElem = doc.createElement( "ContactInformation" );

  //Contact person primary
  QDomElement contactPersonPrimaryElem = doc.createElement( "ContactPersonPrimary" );

  //Contact person
  QDomElement contactPersonElem = propertiesElem.firstChildElement( "WMSContactPerson" );
  if ( !contactPersonElem.isNull() )
  {
    QDomElement wmsContactPersonElem = doc.createElement( "ContactPerson" );
    QDomText contactPersonText = doc.createTextNode( contactPersonElem.text() );
    wmsContactPersonElem.appendChild( contactPersonText );
    contactPersonPrimaryElem.appendChild( wmsContactPersonElem );
  }

  //Contact organisation
  QDomElement contactOrganisationElem = propertiesElem.firstChildElement( "WMSContactOrganisation" );
  if ( !contactOrganisationElem.isNull() )
  {
    QDomElement wmsContactOrganisationElem = doc.createElement( "ContactOrganization" );
    QDomText contactOrganisationText = doc.createTextNode( contactOrganisationElem.text() );
    wmsContactOrganisationElem.appendChild( contactOrganisationText );
    contactPersonPrimaryElem.appendChild( wmsContactOrganisationElem );
  }

  contactInfoElem.appendChild( contactPersonPrimaryElem );

  //Contact address
  //QDomElement contactAddressElem = doc.createElement( "ContactAddress" );
  //contactInfoElem.appendChild( contactAddressElem );

  //phone
  QDomElement phoneElem = propertiesElem.firstChildElement( "WMSContactPhone" );
  if ( !phoneElem.isNull() )
  {
    QDomElement wmsPhoneElem = doc.createElement( "ContactVoiceTelephone" );
    QDomText wmsPhoneText = doc.createTextNode( phoneElem.text() );
    wmsPhoneElem.appendChild( wmsPhoneText );
    contactInfoElem.appendChild( wmsPhoneElem );
  }

  //mail
  QDomElement mailElem = propertiesElem.firstChildElement( "WMSContactMail" );
  if ( !mailElem.isNull() )
  {
    QDomElement wmsMailElem = doc.createElement( "ContactElectronicMailAddress" );
    QDomText wmsMailText = doc.createTextNode( mailElem.text() );
    wmsMailElem.appendChild( wmsMailText );
    contactInfoElem.appendChild( wmsMailElem );
  }

  parentElement.appendChild( contactInfoElem );
}

