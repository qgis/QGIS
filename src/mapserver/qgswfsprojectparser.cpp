/***************************************************************************
                              qgswfsprojectparser.cpp
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

#include "qgswfsprojectparser.h"
#include "qgsconfigparserutils.h"
#include "qgsvectordataprovider.h"

QgsWFSProjectParser::QgsWFSProjectParser( QDomDocument* xmlDoc, const QString& filePath ):
    mProjectParser( xmlDoc, filePath )
{
}

QgsWFSProjectParser::~QgsWFSProjectParser()
{
}

void QgsWFSProjectParser::serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const
{
  QDomElement serviceElem = doc.createElement( "Service" );

  QDomElement propertiesElem = mProjectParser.propertiesElem();
  if ( propertiesElem.isNull() )
  {
    QgsConfigParserUtils::fallbackServiceCapabilities( parentElement, doc );
    return;
  }

  QDomElement serviceCapabilityElem = propertiesElem.firstChildElement( "WMSServiceCapabilities" );
  if ( serviceCapabilityElem.isNull() || serviceCapabilityElem.text().compare( "true", Qt::CaseInsensitive ) != 0 )
  {
    QgsConfigParserUtils::fallbackServiceCapabilities( parentElement, doc );
    return;
  }

  QDomElement wmsNameElem = doc.createElement( "Name" );
  QDomText wmsNameText = doc.createTextNode( "WFS" );
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

  //keyword list
  QDomElement keywordListElem = propertiesElem.firstChildElement( "WMSKeywordList" );
  if ( !keywordListElem.isNull() && !keywordListElem.text().isEmpty() )
  {
    QDomNodeList keywordList = keywordListElem.elementsByTagName( "value" );
    QStringList keywords;
    for ( int i = 0; i < keywordList.size(); ++i )
    {
      keywords << keywordList.at( i ).toElement().text();
    }

    if ( keywordList.size() > 0 )
    {
      QDomElement wfsKeywordElem = doc.createElement( "Keywords" );
      QDomText keywordText = doc.createTextNode( keywords.join( ", " ) );
      wfsKeywordElem.appendChild( keywordText );
      serviceElem.appendChild( wfsKeywordElem );
    }
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

  //Fees
  QDomElement feesElem = propertiesElem.firstChildElement( "WMSFees" );
  if ( !feesElem.isNull() )
  {
    QDomElement wmsFeesElem = doc.createElement( "Fees" );
    QDomText wmsFeesText = doc.createTextNode( feesElem.text() );
    wmsFeesElem.appendChild( wmsFeesText );
    serviceElem.appendChild( wmsFeesElem );
  }

  //AccessConstraints
  QDomElement accessConstraintsElem = propertiesElem.firstChildElement( "WMSAccessConstraints" );
  if ( !accessConstraintsElem.isNull() )
  {
    QDomElement wmsAccessConstraintsElem = doc.createElement( "AccessConstraints" );
    QDomText wmsAccessConstraintsText = doc.createTextNode( accessConstraintsElem.text() );
    wmsAccessConstraintsElem.appendChild( wmsAccessConstraintsText );
    serviceElem.appendChild( wmsAccessConstraintsElem );
  }
  parentElement.appendChild( serviceElem );
}

QString QgsWFSProjectParser::serviceUrl() const
{
  return mProjectParser.serviceUrl();
}

QString QgsWFSProjectParser::wfsServiceUrl() const
{
  QString url;

  if ( !mProjectParser.xmlDocument() )
  {
    return url;
  }

  QDomElement propertiesElem = mProjectParser.propertiesElem();
  if ( !propertiesElem.isNull() )
  {
    QDomElement wfsUrlElem = propertiesElem.firstChildElement( "WFSUrl" );
    if ( !wfsUrlElem.isNull() )
    {
      url = wfsUrlElem.text();
    }
  }
  return url;
}

void QgsWFSProjectParser::featureTypeList( QDomElement& parentElement, QDomDocument& doc ) const
{
  const QList<QDomElement>& projectLayerElements = mProjectParser.projectLayerElements();
  if ( projectLayerElements.size() < 1 )
  {
    return;
  }

  QStringList wfsLayersId = mProjectParser.wfsLayers();
  QStringList wfstUpdateLayersId = wfstUpdateLayers();
  QStringList wfstInsertLayersId = wfstInsertLayers();
  QStringList wfstDeleteLayersId = wfstDeleteLayers();

  QMap<QString, QgsMapLayer *> layerMap;

  foreach ( const QDomElement &elem, projectLayerElements )
  {
    QString type = elem.attribute( "type" );
    if ( type == "vector" )
    {
      //addJoinLayersForElement( elem ); //todo: fixme
      QgsMapLayer *layer = mProjectParser.createLayerFromElement( elem );
      if ( layer && wfsLayersId.contains( layer->id() ) )
      {
        QgsDebugMsg( QString( "add layer %1 to map" ).arg( layer->id() ) );
        layerMap.insert( layer->id(), layer );

        QDomElement layerElem = doc.createElement( "FeatureType" );
        QDomElement nameElem = doc.createElement( "Name" );
        //We use the layer name even though it might not be unique.
        //Because the id sometimes contains user/pw information and the name is more descriptive
        QString typeName = layer->name();
        typeName = typeName.replace( " ", "_" );
        QDomText nameText = doc.createTextNode( typeName );
        nameElem.appendChild( nameText );
        layerElem.appendChild( nameElem );

        QDomElement titleElem = doc.createElement( "Title" );
        QString titleName = layer->title();
        if ( titleName.isEmpty() )
        {
          titleName = layer->name();
        }
        QDomText titleText = doc.createTextNode( titleName );
        titleElem.appendChild( titleText );
        layerElem.appendChild( titleElem );

        QDomElement abstractElem = doc.createElement( "Abstract" );
        QString abstractName = layer->abstract();
        if ( abstractName.isEmpty() )
        {
          abstractName = "";
        }
        QDomText abstractText = doc.createTextNode( abstractName );
        abstractElem.appendChild( abstractText );
        layerElem.appendChild( abstractElem );

        //keyword list
        if ( !layer->keywordList().isEmpty() )
        {
          QDomElement keywordsElem = doc.createElement( "Keywords" );
          QDomText keywordsText = doc.createTextNode( layer->keywordList() );
          keywordsElem.appendChild( keywordsText );
          layerElem.appendChild( keywordsElem );
        }

        //appendExGeographicBoundingBox( layerElem, doc, layer->extent(), layer->crs() );

        QDomElement srsElem = doc.createElement( "SRS" );
        QDomText srsText = doc.createTextNode( layer->crs().authid() );
        srsElem.appendChild( srsText );
        layerElem.appendChild( srsElem );

        //wfs:Operations element
        QDomElement operationsElement = doc.createElement( "Operations"/*wfs:Operations*/ );
        //wfs:Query element
        QDomElement queryElement = doc.createElement( "Query"/*wfs:Query*/ );
        operationsElement.appendChild( queryElement );

        QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( layer );
        QgsVectorDataProvider* provider = vlayer->dataProvider();
        if (( provider->capabilities() & QgsVectorDataProvider::AddFeatures ) && wfstInsertLayersId.contains( layer->id() ) )
        {
          //wfs:Insert element
          QDomElement insertElement = doc.createElement( "Insert"/*wfs:Insert*/ );
          operationsElement.appendChild( insertElement );
        }
        if (( provider->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) &&
            ( provider->capabilities() & QgsVectorDataProvider::ChangeGeometries ) &&
            wfstUpdateLayersId.contains( layer->id() ) )
        {
          //wfs:Update element
          QDomElement updateElement = doc.createElement( "Update"/*wfs:Update*/ );
          operationsElement.appendChild( updateElement );
        }
        if (( provider->capabilities() & QgsVectorDataProvider::DeleteFeatures ) && wfstDeleteLayersId.contains( layer->id() ) )
        {
          //wfs:Delete element
          QDomElement deleteElement = doc.createElement( "Delete"/*wfs:Delete*/ );
          operationsElement.appendChild( deleteElement );
        }

        layerElem.appendChild( operationsElement );

        QgsRectangle layerExtent = layer->extent();
        QDomElement bBoxElement = doc.createElement( "LatLongBoundingBox" );
        bBoxElement.setAttribute( "minx", QString::number( layerExtent.xMinimum() ) );
        bBoxElement.setAttribute( "miny", QString::number( layerExtent.yMinimum() ) );
        bBoxElement.setAttribute( "maxx", QString::number( layerExtent.xMaximum() ) );
        bBoxElement.setAttribute( "maxy", QString::number( layerExtent.yMaximum() ) );
        layerElem.appendChild( bBoxElement );

        // layer metadata URL
        QString metadataUrl = layer->metadataUrl();
        if ( !metadataUrl.isEmpty() )
        {
          QDomElement metaUrlElem = doc.createElement( "MetadataURL" );
          QString metadataUrlType = layer->metadataUrlType();
          metaUrlElem.setAttribute( "type", metadataUrlType );
          QString metadataUrlFormat = layer->metadataUrlFormat();
          if ( metadataUrlFormat == "text/xml" )
          {
            metaUrlElem.setAttribute( "format", "XML" );
          }
          else
          {
            metaUrlElem.setAttribute( "format", "TXT" );
          }
          QDomText metaUrlText = doc.createTextNode( metadataUrl );
          metaUrlElem.appendChild( metaUrlText );
          layerElem.appendChild( metaUrlElem );
        }

        parentElement.appendChild( layerElem );
      }
    }
  }
  return;
}

QStringList QgsWFSProjectParser::wfstUpdateLayers() const
{
  QStringList publiedIds = mProjectParser.wfsLayers();
  QStringList wfsList;
  if ( !mProjectParser.xmlDocument() )
  {
    return wfsList;
  }

  QDomElement propertiesElem = mProjectParser.propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstLayersElem = propertiesElem.firstChildElement( "WFSTLayers" );
  if ( wfstLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstUpdateLayersElem = wfstLayersElem.firstChildElement( "Update" );
  if ( wfstUpdateLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomNodeList valueList = wfstUpdateLayersElem.elementsByTagName( "value" );
  for ( int i = 0; i < valueList.size(); ++i )
  {
    QString id = valueList.at( i ).toElement().text();
    if ( publiedIds.contains( id ) )
      wfsList << id;
  }
  return wfsList;
}

QStringList QgsWFSProjectParser::wfstInsertLayers() const
{
  QStringList updateIds = wfstUpdateLayers();
  QStringList wfsList;
  if ( !mProjectParser.xmlDocument() )
  {
    return wfsList;
  }

  QDomElement propertiesElem = mProjectParser.propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstLayersElem = propertiesElem.firstChildElement( "WFSTLayers" );
  if ( wfstLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstInsertLayersElem = wfstLayersElem.firstChildElement( "Insert" );
  if ( wfstInsertLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomNodeList valueList = wfstInsertLayersElem.elementsByTagName( "value" );
  for ( int i = 0; i < valueList.size(); ++i )
  {
    QString id = valueList.at( i ).toElement().text();
    if ( updateIds.contains( id ) )
      wfsList << id;
  }
  return wfsList;
}

QStringList QgsWFSProjectParser::wfstDeleteLayers() const
{
  QStringList insertIds = wfstInsertLayers();
  QStringList wfsList;
  if ( !mProjectParser.xmlDocument() )
  {
    return wfsList;
  }

  QDomElement propertiesElem = mProjectParser.propertiesElem();
  if ( propertiesElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstLayersElem = propertiesElem.firstChildElement( "WFSTLayers" );
  if ( wfstLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomElement wfstDeleteLayersElem = wfstLayersElem.firstChildElement( "Delete" );
  if ( wfstDeleteLayersElem.isNull() )
  {
    return wfsList;
  }
  QDomNodeList valueList = wfstDeleteLayersElem.elementsByTagName( "value" );
  for ( int i = 0; i < valueList.size(); ++i )
  {
    QString id = valueList.at( i ).toElement().text();
    if ( insertIds.contains( id ) )
      wfsList << id;
  }
  return wfsList;
}
