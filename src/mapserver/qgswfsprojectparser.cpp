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
#include "qgsmaplayerregistry.h"
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

void QgsWFSProjectParser::describeFeatureType( const QString& aTypeName, QDomElement& parentElement, QDomDocument& doc ) const
{
  const QList<QDomElement>& projectLayerElements = mProjectParser.projectLayerElements();
  if ( projectLayerElements.size() < 1 )
  {
    return;
  }

  QStringList wfsLayersId = mProjectParser.wfsLayers();
  QStringList typeNameList;
  if ( aTypeName != "" )
  {
    QStringList typeNameSplit = aTypeName.split( "," );
    foreach ( const QString &str, typeNameSplit )
    {
      if ( str.contains( ":" ) )
        typeNameList << str.section( ":", 1, 1 );
      else
        typeNameList << str;
    }
  }

  foreach ( const QDomElement &elem, projectLayerElements )
  {
    QString type = elem.attribute( "type" );
    if ( type == "vector" )
    {
      QgsMapLayer *mLayer = mProjectParser.createLayerFromElement( elem );
      QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>( mLayer );
      if ( !layer )
        continue;

      QString typeName = layer->name();
      typeName = typeName.replace( " ", "_" );

      if ( wfsLayersId.contains( layer->id() ) && ( aTypeName == "" || typeNameList.contains( typeName ) ) )
      {
        //do a select with searchRect and go through all the features
        QgsVectorDataProvider* provider = layer->dataProvider();
        if ( !provider )
        {
          continue;
        }

        //hidden attributes for this layer
        const QSet<QString>& layerExcludedAttributes = layer->excludeAttributesWFS();

        //xsd:element
        QDomElement elementElem = doc.createElement( "element"/*xsd:element*/ );
        elementElem.setAttribute( "name", typeName );
        elementElem.setAttribute( "type", "qgs:" + typeName + "Type" );
        elementElem.setAttribute( "substitutionGroup", "gml:_Feature" );
        parentElement.appendChild( elementElem );

        //xsd:complexType
        QDomElement complexTypeElem = doc.createElement( "complexType"/*xsd:complexType*/ );
        complexTypeElem.setAttribute( "name", typeName + "Type" );
        parentElement.appendChild( complexTypeElem );

        //xsd:complexType
        QDomElement complexContentElem = doc.createElement( "complexContent"/*xsd:complexContent*/ );
        complexTypeElem.appendChild( complexContentElem );

        //xsd:extension
        QDomElement extensionElem = doc.createElement( "extension"/*xsd:extension*/ );
        extensionElem.setAttribute( "base", "gml:AbstractFeatureType" );
        complexContentElem.appendChild( extensionElem );

        //xsd:sequence
        QDomElement sequenceElem = doc.createElement( "sequence"/*xsd:sequence*/ );
        extensionElem.appendChild( sequenceElem );

        //xsd:element
        QDomElement geomElem = doc.createElement( "element"/*xsd:element*/ );
        geomElem.setAttribute( "name", "geometry" );
        QGis::WkbType wkbType = layer->wkbType();
        if ( wkbType != QGis::WKBNoGeometry )
        {
          switch ( wkbType )
          {
            case QGis::WKBPoint25D:
            case QGis::WKBPoint:
              geomElem.setAttribute( "type", "gml:PointPropertyType" );
              break;
            case QGis::WKBLineString25D:
            case QGis::WKBLineString:
              geomElem.setAttribute( "type", "gml:LineStringPropertyType" );
              break;
            case QGis::WKBPolygon25D:
            case QGis::WKBPolygon:
              geomElem.setAttribute( "type", "gml:PolygonPropertyType" );
              break;
            case QGis::WKBMultiPoint25D:
            case QGis::WKBMultiPoint:
              geomElem.setAttribute( "type", "gml:MultiPointPropertyType" );
              break;
            case QGis::WKBMultiLineString25D:
            case QGis::WKBMultiLineString:
              geomElem.setAttribute( "type", "gml:MultiLineStringPropertyType" );
              break;
            case QGis::WKBMultiPolygon25D:
            case QGis::WKBMultiPolygon:
              geomElem.setAttribute( "type", "gml:MultiPolygonPropertyType" );
              break;
            default:
              geomElem.setAttribute( "type", "gml:GeometryPropertyType" );
              break;
          }
          geomElem.setAttribute( "minOccurs", "0" );
          geomElem.setAttribute( "maxOccurs", "1" );
          sequenceElem.appendChild( geomElem );
        }

        //const QgsFields& fields = provider->fields();
        const QgsFields& fields = layer->pendingFields();
        for ( int idx = 0; idx < fields.count(); ++idx )
        {

          QString attributeName = fields[idx].name();
          //skip attribute if excluded from WFS publication
          if ( layerExcludedAttributes.contains( attributeName ) )
          {
            continue;
          }

          //xsd:element
          QDomElement geomElem = doc.createElement( "element"/*xsd:element*/ );
          geomElem.setAttribute( "name", attributeName );
          QVariant::Type attributeType = fields[idx].type();
          if ( attributeType == QVariant::Int )
            geomElem.setAttribute( "type", "integer" );
          else if ( attributeType == QVariant::Double )
            geomElem.setAttribute( "type", "double" );
          else
            geomElem.setAttribute( "type", "string" );

          sequenceElem.appendChild( geomElem );

          QString alias = layer->attributeAlias( idx );
          if ( !alias.isEmpty() )
          {
            geomElem.setAttribute( "alias", alias );
          }
        }
      }
    }
  }
  QgsMapLayerRegistry::instance()->removeAllMapLayers();
  return;
}

QStringList QgsWFSProjectParser::wfsLayers() const
{
  return mProjectParser.wfsLayers();
}

QList<QgsMapLayer*> QgsWFSProjectParser::mapLayerFromTypeName( const QString& aTypeName, bool useCache ) const
{
  Q_UNUSED( useCache );

  QList<QgsMapLayer*> layerList;
  const QList<QDomElement>& projectLayerElements = mProjectParser.projectLayerElements();

  if ( projectLayerElements.size() < 1 )
  {
    return layerList;
  }
  QStringList wfsLayersId = wfsLayers();

  QStringList typeNameList;
  if ( aTypeName != "" )
  {
    QStringList typeNameSplit = aTypeName.split( "," );
    foreach ( const QString &str, typeNameSplit )
    {
      if ( str.contains( ":" ) )
        typeNameList << str.section( ":", 1, 1 );
      else
        typeNameList << str;
    }
  }

  foreach ( const QDomElement &elem, projectLayerElements )
  {
    QString type = elem.attribute( "type" );
    if ( type == "vector" )
    {
      QgsMapLayer *mLayer = mProjectParser.createLayerFromElement( elem );
      QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>( mLayer );
      if ( !layer )
        continue;

      QString typeName = layer->name();
      typeName = typeName.replace( " ", "_" );

      if ( wfsLayersId.contains( layer->id() ) && ( aTypeName == "" || typeNameList.contains( typeName ) ) )
        layerList.push_back( mLayer );
    }
  }
  return layerList;
}
