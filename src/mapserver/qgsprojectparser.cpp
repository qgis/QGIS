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
#include "qgsconfigcache.h"
#include "qgsepsgcache.h"
#include "qgsmslayercache.h"
#include "qgslogger.h"
#include "qgsmapserviceexception.h"
#include "qgsrasterlayer.h"
#include "qgsrenderer.h"
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

#include "QFileInfo"
#include "QTextStream"


QgsProjectParser::QgsProjectParser( QDomDocument* xmlDoc, const QString& filePath ): QgsConfigParser(), mXMLDoc( xmlDoc ), mProjectPath( filePath )
{
  mOutputUnits = QgsMapRenderer::Millimeters;
  setLegendParametersFromProject();
  setSelectionColor();
}

QgsProjectParser::~QgsProjectParser()
{
  delete mXMLDoc;
}

int QgsProjectParser::numberOfLayers() const
{
  QList<QDomElement> layerElems = projectLayerElements();
  return layerElems.size();
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
      QgsDebugMsg( QString( "add layer %1 to map" ).arg( layer->id() ) );
      layerMap.insert( layer->id(), layer );
    }
#if QGSMSDEBUG
    else
    {
      QString buf;
      QTextStream s( &buf );
      layerIt->save( s, 0 );
      QgsDebugMsg( QString( "layer %1 not found" ).arg( buf ) );
    }
#endif
  }

  //According to the WMS spec, there can be only one toplevel layer.
  //So we create an artificial one here to be in accordance with the schema
  QString projTitle = projectTitle();
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

  QDomElement legendElem = mXMLDoc->documentElement().firstChildElement( "legend" );

  addLayers( doc, layerParentElem, legendElem, layerMap, nonIdentifiableLayers );

  parentElement.appendChild( layerParentElem );
  combineExtentAndCrsOfGroupChildren( layerParentElem, doc );
}

void QgsProjectParser::addLayers( QDomDocument &doc,
                                  QDomElement &parentElem,
                                  const QDomElement &legendElem,
                                  const QMap<QString, QgsMapLayer *> &layerMap,
                                  const QStringList &nonIdentifiableLayers ) const
{
  QDomNodeList legendChildren = legendElem.childNodes();
  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    QDomElement currentChildElem = legendChildren.at( i ).toElement();

    QDomElement layerElem = doc.createElement( "Layer" );

    if ( currentChildElem.tagName() == "legendgroup" )
    {
      layerElem.setAttribute( "queryable", "1" );
      QString name = currentChildElem.attribute( "name" );
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
        QString project = convertToAbsolutePath( currentChildElem.attribute( "project" ) );
        QgsDebugMsg( QString( "Project path: %1" ).arg( project ) );
        QString embeddedGroupName = currentChildElem.attribute( "name" );
        QgsProjectParser* p = dynamic_cast<QgsProjectParser*>( QgsConfigCache::instance()->searchConfiguration( project ) );
        if ( p )
        {
          QStringList pIdDisabled = p->identifyDisabledLayers();

          QDomElement embeddedGroupElem;
          QList<QDomElement> pLegendElems = p->legendGroupElements();
          QList<QDomElement>::const_iterator pLegendIt = pLegendElems.constBegin();
          for ( ; pLegendIt != pLegendElems.constEnd(); ++pLegendIt )
          {
            if ( pLegendIt->attribute( "name" ) == embeddedGroupName )
            {
              embeddedGroupElem = *pLegendIt;
              break;
            }
          }

          QList<QDomElement> pLayerElems = p->projectLayerElements();
          QMap<QString, QgsMapLayer *> pLayerMap;
          QList<QDomElement>::const_iterator pLayerIt = pLayerElems.constBegin();
          for ( ; pLayerIt != pLayerElems.constEnd(); ++pLayerIt )
          {
            pLayerMap.insert( layerId( *pLayerIt ), p->createLayerFromElement( *pLayerIt ) );
          }

          p->addLayers( doc, layerElem, embeddedGroupElem, pLayerMap, pIdDisabled );
        }
      }
      else //normal (not embedded) legend group
      {
        addLayers( doc, layerElem, currentChildElem, layerMap, nonIdentifiableLayers );
      }

      // combine bounding boxes of children (groups/layers)
      combineExtentAndCrsOfGroupChildren( layerElem, doc );
    }
    else if ( currentChildElem.tagName() == "legendlayer" )
    {
      QString id = layerIdFromLegendLayer( currentChildElem );

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
      QDomText nameText = doc.createTextNode( currentLayer->name() );
      nameElem.appendChild( nameText );
      layerElem.appendChild( nameElem );

      QDomElement titleElem = doc.createElement( "Title" );
      QDomText titleText = doc.createTextNode( currentLayer->name() );
      titleElem.appendChild( titleText );
      layerElem.appendChild( titleElem );

      //CRS
      QStringList crsList = createCRSListForLayer( currentLayer );
      appendCRSElementsToLayer( layerElem, doc, crsList );

      //Ex_GeographicBoundingBox
      appendExGeographicBoundingBox( layerElem, doc, currentLayer->extent(), currentLayer->crs() );

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
      QgsDebugMsg( "unexpected child element" );
      continue;
    }

    parentElem.appendChild( layerElem );
  }
}

void QgsProjectParser::combineExtentAndCrsOfGroupChildren( QDomElement& groupElem, QDomDocument& doc ) const
{
  QgsRectangle combinedGeographicBBox;
  QSet<QString> combinedCRSSet;
  bool firstBBox = true;
  bool firstCRSSet = true;

  QDomNodeList layerChildren = groupElem.childNodes();
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
    QSet<QString> crsSet;
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

  appendCRSElementsToLayer( groupElem, doc, combinedCRSSet.toList() );

  const QgsCoordinateReferenceSystem& groupCRS = QgsEPSGCache::instance()->searchCRS( GEO_EPSG_CRS_ID );
  appendExGeographicBoundingBox( groupElem, doc, combinedGeographicBBox, groupCRS );
}

QList<QgsMapLayer*> QgsProjectParser::mapLayerFromStyle( const QString& lName, const QString& styleName, bool allowCaching ) const
{
  Q_UNUSED( styleName );
  Q_UNUSED( allowCaching );
  QList<QgsMapLayer*> layerList;

  //first assume lName refers to a leaf layer
  QMap< QString, QDomElement > layerElemMap = projectLayerElementsByName();
  QMap< QString, QDomElement >::const_iterator layerElemIt = layerElemMap.find( lName );
  if ( layerElemIt != layerElemMap.constEnd() )
  {
    QgsMapLayer* layer = createLayerFromElement( layerElemIt.value() );
    if ( layer )
    {
      layerList.push_back( layer );
      return layerList;
    }
  }

  //Check if layer name refers to the top level group for the project.
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
      if ( groupIt->attribute( "embedded" ) == "1" ) //requested group is embedded from another project
      {
        QString project = convertToAbsolutePath( groupIt->attribute( "project" ) );
        QgsDebugMsg( QString( "Project path: %1" ).arg( project ) );
        QgsProjectParser* p = dynamic_cast<QgsProjectParser*>( QgsConfigCache::instance()->searchConfiguration( project ) );
        if ( p )
        {
          QList<QDomElement> pGroupElems = p->legendGroupElements();
          QList<QDomElement>::const_iterator pGroupIt = pGroupElems.constBegin();
          QDomElement embeddedGroupElem;

          for ( ; pGroupIt != pGroupElems.constEnd(); ++pGroupIt )
          {
            if ( pGroupIt->attribute( "name" ) == lName )
            {
              embeddedGroupElem = *pGroupIt;
              break;
            }
          }

          if ( !embeddedGroupElem.isNull() )
          {
            //add all the layers under the group
            QMap< QString, QDomElement > pLayerElems = p->projectLayerElementsById();
            QDomNodeList pLayerNodes = embeddedGroupElem.elementsByTagName( "legendlayer" );
            for ( int i = 0; i < pLayerNodes.size(); ++i )
            {
              QString pLayerId = pLayerNodes.at( i ).toElement().firstChildElement( "filegroup" ).firstChildElement( "legendlayerfile" ).attribute( "layerid" );
              QgsMapLayer* pLayer = p->createLayerFromElement( pLayerElems[pLayerId] );
              if ( pLayer )
              {
                layerList.push_back( pLayer );
              }
            }
          }
        }
      }
      else //normal (not embedded) group
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
      return layerList;
    }
  }

  //maybe the layer is embedded from another project
  QMap< QString, QDomElement >::const_iterator layerIt = idLayerMap.constBegin();
  for ( ; layerIt != idLayerMap.constEnd(); ++layerIt )
  {
    if ( layerIt.value().attribute( "embedded" ) == "1" )
    {
      QString id = layerIt.value().attribute( "id" );
      QString project = layerIt.value().attribute( "project" );
      QgsDebugMsg( QString( "Project path: %1" ).arg( project ) );

      //get config parser from cache
      QgsProjectParser* otherParser = dynamic_cast<QgsProjectParser*>( QgsConfigCache::instance()->searchConfiguration( project ) );
      if ( otherParser )
      {
        //get element by id
        QMap< QString, QDomElement > otherLayerElems = otherParser->projectLayerElementsById();
        QMap< QString, QDomElement >::const_iterator otherLayerIt = otherLayerElems.find( id );
        if ( otherLayerIt != otherLayerElems.constEnd() )
        {
          if ( otherLayerIt.value().firstChildElement( "layername" ).text() == lName )
          {
            layerList.push_back( otherParser->createLayerFromElement( otherLayerIt.value() ) );
            return layerList;
          }
        }
      }
    }
  }

  //layer still not found. Check if it is a single layer contained in a embedded layer group
  groupIt = legendGroups.constBegin();
  for ( ; groupIt != legendGroups.constEnd(); ++groupIt )
  {
    if ( groupIt->attribute( "embedded" ) == "1" )
    {
      QString project = convertToAbsolutePath( groupIt->attribute( "project" ) );
      QgsDebugMsg( QString( "Project path: %1" ).arg( project ) );
      QgsProjectParser* p = dynamic_cast<QgsProjectParser*>( QgsConfigCache::instance()->searchConfiguration( project ) );
      if ( p )
      {
        QMap< QString, QDomElement > pLayers = p->projectLayerElementsByName();
        QMap< QString, QDomElement >::const_iterator pLayerIt = pLayers.find( lName );
        if ( pLayerIt != pLayers.constEnd() )
        {
          QgsMapLayer* layer = p->createLayerFromElement( pLayerIt.value() );
          if ( layer )
          {
            layerList.push_back( layer );
            return layerList;
          }
        }
      }
    }
  }

  //layer not found, return empty list
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
  Q_UNUSED( styleName );
  Q_UNUSED( layerName );
  return QDomDocument();
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

QStringList QgsProjectParser::supportedOutputCrsList() const
{
  QStringList crsList;
  if ( !mXMLDoc )
  {
    return crsList;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return crsList;
  }
  QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
  if ( propertiesElem.isNull() )
  {
    return crsList;
  }
  QDomElement wmsCrsElem = propertiesElem.firstChildElement( "WMSCrsList" );
  if ( !wmsCrsElem.isNull() )
  {
    QDomNodeList valueList = wmsCrsElem.elementsByTagName( "value" );
    for ( int i = 0; i < valueList.size(); ++i )
    {
      crsList.append( valueList.at( i ).toElement().text() );
    }
  }
  else
  {
    QDomElement wmsEpsgElem = propertiesElem.firstChildElement( "WMSEpsgList" );
    if ( wmsEpsgElem.isNull() )
    {
      return crsList;
    }
    QDomNodeList valueList = wmsEpsgElem.elementsByTagName( "value" );
    bool conversionOk;
    for ( int i = 0; i < valueList.size(); ++i )
    {
      int epsgNr = valueList.at( i ).toElement().text().toInt( &conversionOk );
      if ( conversionOk )
      {
        crsList.append( QString( "EPSG:%1" ).arg( epsgNr ) );
      }
    }
  }

  return crsList;
}

bool QgsProjectParser::featureInfoWithWktGeometry() const
{
  if ( !mXMLDoc )
  {
    return false;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return false;
  }
  QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
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

QString QgsProjectParser::mapAuthid() const
{
  if ( !mXMLDoc )
  {
    return QString::null;
  }

  QDomElement qgisElem = mXMLDoc->documentElement();
  if ( qgisElem.isNull() )
  {
    return QString::null;
  }

  QDomElement mapCanvasElem = qgisElem.firstChildElement( "mapcanvas" );
  if ( mapCanvasElem.isNull() )
  {
    return QString::null;
  }

  QDomElement srsElem = mapCanvasElem.firstChildElement( "destinationsrs" );
  if ( srsElem.isNull() )
  {
    return QString::null;
  }

  QDomNodeList authIdNodes = srsElem.elementsByTagName( "authid" );
  if ( authIdNodes.size() < 1 )
  {
    return QString::null;
  }

  return authIdNodes.at( 0 ).toElement().text();
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
  if ( !titleElem.isNull() )
  {
    QString title = titleElem.text();
    if ( !title.isEmpty() )
    {
      return title;
    }
  }

  //no title element or not project title set. Use project filename without extension
  QFileInfo projectFileInfo( mProjectPath );
  return projectFileInfo.baseName();
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
  if ( elem.isNull() || !mXMLDoc )
  {
    return 0;
  }

  QString uri;
  QString absoluteUri;
  QDomElement dataSourceElem = elem.firstChildElement( "datasource" );
  if ( !dataSourceElem.isNull() )
  {
    //convert relative pathes to absolute ones if necessary
    uri = dataSourceElem.text();
    absoluteUri = convertToAbsolutePath( uri );
    if ( uri != absoluteUri )
    {
      QDomText absoluteTextNode = mXMLDoc->createTextNode( absoluteUri );
      dataSourceElem.replaceChild( absoluteTextNode, dataSourceElem.firstChild() );
    }
  }

  QString id = layerId( elem );
  QgsMapLayer* layer = QgsMSLayerCache::instance()->searchLayer( absoluteUri, id );
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
  else if ( elem.attribute( "embedded" ) == "1" ) //layer is embedded from another project file
  {
    QString project = convertToAbsolutePath( elem.attribute( "project" ) );
    QgsDebugMsg( QString( "Project path: %1" ).arg( project ) );
    QgsProjectParser* otherConfig = dynamic_cast<QgsProjectParser*>( QgsConfigCache::instance()->searchConfiguration( project ) );
    if ( !otherConfig )
    {
      return 0;
    }

    QMap< QString, QDomElement > layerMap = otherConfig->projectLayerElementsById();
    QMap< QString, QDomElement >::const_iterator layerIt = layerMap.find( elem.attribute( "id" ) );
    if ( layerIt == layerMap.constEnd() )
    {
      return 0;
    }
    return otherConfig->createLayerFromElement( layerIt.value() );
  }

  if ( layer )
  {
    layer->readXML( const_cast<QDomElement&>( elem ) ); //should be changed to const in QgsMapLayer
    layer->setLayerName( layerName( elem ) );
    QgsMSLayerCache::instance()->insertLayer( absoluteUri, id, layer );
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

      //read full legend from xml
      QgsComposerLegend* legend = new QgsComposerLegend( composition );
      legend->readXML( currentElem, *mXMLDoc );

      //dynamic legend (would be interesting in case of layers dynamically defined in SLD)
      //legend->_readXML( currentElem.firstChildElement( "ComposerItem" ), *mXMLDoc );
      //legend->updateLegend();
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
      //qgis mapserver needs an absolute file path
      picture->setPictureFile( convertToAbsolutePath( picture->pictureFile() ) );
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
  QDomElement serviceElem = doc.createElement( "Service" );

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
  serviceElem.appendChild( wmsNameElem );

  //WMS title
  QDomElement titleElem = propertiesElem.firstChildElement( "WMSServiceTitle" );
  if ( !titleElem.isNull() )
  {
    QDomElement wmsTitleElem = doc.createElement( "Title" );
    QDomText wmsTitleText = doc.createTextNode( titleElem.text() );
    wmsTitleElem.appendChild( wmsTitleText );
    serviceElem.appendChild( wmsTitleElem );
  }

  //WMS abstract
  QDomElement abstractElem = propertiesElem.firstChildElement( "WMSServiceAbstract" );
  if ( !abstractElem.isNull() )
  {
    QDomElement wmsAbstractElem = doc.createElement( "Abstract" );
    QDomText wmsAbstractText = doc.createTextNode( abstractElem.text() );
    wmsAbstractElem.appendChild( wmsAbstractText );
    serviceElem.appendChild( wmsAbstractElem );
  }

  //OnlineResource element is mandatory according to the WMS specification
  QDomElement wmsOnlineResourceElem = propertiesElem.firstChildElement( "WMSOnlineResource" );
  QDomElement onlineResourceElem = doc.createElement( "OnlineResource" );
  onlineResourceElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
  onlineResourceElem.setAttribute( "xlink:type", "simple" );
  if ( !wmsOnlineResourceElem.isNull() )
  {
    onlineResourceElem.setAttribute( "xlink:href", wmsOnlineResourceElem.text() );
  }

  serviceElem.appendChild( onlineResourceElem );

  //Contact information
  QDomElement contactInfoElem = doc.createElement( "ContactInformation" );

  //Contact person primary
  QDomElement contactPersonPrimaryElem = doc.createElement( "ContactPersonPrimary" );

  //Contact person
  QDomElement contactPersonElem = propertiesElem.firstChildElement( "WMSContactPerson" );
  QString contactPersonString;
  if ( !contactPersonElem.isNull() )
  {
    contactPersonString = contactPersonElem.text();
  }
  QDomElement wmsContactPersonElem = doc.createElement( "ContactPerson" );
  QDomText contactPersonText = doc.createTextNode( contactPersonString );
  wmsContactPersonElem.appendChild( contactPersonText );
  contactPersonPrimaryElem.appendChild( wmsContactPersonElem );


  //Contact organisation
  QDomElement contactOrganizationElem = propertiesElem.firstChildElement( "WMSContactOrganization" );
  QString contactOrganizationString;
  if ( !contactOrganizationElem.isNull() )
  {
    contactOrganizationString = contactOrganizationElem.text();
  }
  QDomElement wmsContactOrganizationElem = doc.createElement( "ContactOrganization" );
  QDomText contactOrganizationText = doc.createTextNode( contactOrganizationString );
  wmsContactOrganizationElem.appendChild( contactOrganizationText );
  contactPersonPrimaryElem.appendChild( wmsContactOrganizationElem );
  contactInfoElem.appendChild( contactPersonPrimaryElem );

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

  serviceElem.appendChild( contactInfoElem );
  parentElement.appendChild( serviceElem );
}

QString QgsProjectParser::convertToAbsolutePath( const QString& file ) const
{
  if ( !file.startsWith( "./" ) && !file.startsWith( "../" ) )
  {
    return file;
  }

  QString srcPath = file;
  QString projPath = mProjectPath;

#if defined(Q_OS_WIN)
  srcPath.replace( "\\", "/" );
  projPath.replace( "\\", "/" );

  bool uncPath = projPath.startsWith( "//" );
#endif

  QStringList srcElems = file.split( "/", QString::SkipEmptyParts );
  QStringList projElems = mProjectPath.split( "/", QString::SkipEmptyParts );

#if defined(Q_OS_WIN)
  if ( uncPath )
  {
    projElems.insert( 0, "" );
    projElems.insert( 0, "" );
  }
#endif

  // remove project file element
  projElems.removeLast();

  // append source path elements
  projElems << srcElems;
  projElems.removeAll( "." );

  // resolve ..
  int pos;
  while (( pos = projElems.indexOf( ".." ) ) > 0 )
  {
    // remove preceding element and ..
    projElems.removeAt( pos - 1 );
    projElems.removeAt( pos - 1 );
  }

#if !defined(Q_OS_WIN)
  // make path absolute
  projElems.prepend( "" );
#endif

  return projElems.join( "/" );
}

void QgsProjectParser::setSelectionColor()
{
  int red = 255;
  int green = 255;
  int blue = 0;
  int alpha = 255;

  //overwrite default selection color with settings from the project
  if ( mXMLDoc )
  {
    QDomElement qgisElem = mXMLDoc->documentElement();
    if ( !qgisElem.isNull() )
    {
      QDomElement propertiesElem = qgisElem.firstChildElement( "properties" );
      if ( !propertiesElem.isNull() )
      {
        QDomElement guiElem = propertiesElem.firstChildElement( "Gui" );
        if ( !guiElem.isNull() )
        {
          QDomElement redElem = guiElem.firstChildElement( "SelectionColorRedPart" );
          if ( !redElem.isNull() )
          {
            red = redElem.text().toInt();
          }
          QDomElement greenElem = guiElem.firstChildElement( "SelectionColorGreenPart" );
          if ( !greenElem.isNull() )
          {
            green = greenElem.text().toInt();
          }
          QDomElement blueElem = guiElem.firstChildElement( "SelectionColorBluePart" );
          if ( !blueElem.isNull() )
          {
            blue = blueElem.text().toInt();
          }
          QDomElement alphaElem = guiElem.firstChildElement( "SelectionColorAlphaPart" );
          if ( !alphaElem.isNull() )
          {
            alpha = alphaElem.text().toInt();
          }
        }
      }
    }
  }

  QgsRenderer::setSelectionColor( QColor( red, green, blue, alpha ) );
}

