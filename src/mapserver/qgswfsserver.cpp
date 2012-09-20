/***************************************************************************
                              qgswfsserver.cpp
                              -------------------
  begin                : February 27, 2012
  copyright            : (C) 2012 by René-Luc D'Hont & Marco Hugentobler
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswfsserver.h"
#include "qgsconfigparser.h"
#include "qgscrscache.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsmaptopixel.h"
#include "qgspallabeling.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsscalecalculator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsfilter.h"
#include "qgslogger.h"
#include "qgsmapserviceexception.h"
#include "qgssldparser.h"
#include "qgssymbol.h"
#include "qgssymbolv2.h"
#include "qgsrenderer.h"
#include "qgslegendmodel.h"
#include "qgscomposerlegenditem.h"
#include "qgsrequesthandler.h"
#include <QImage>
#include <QPainter>
#include <QStringList>
#include <QTextStream>
#include <QDir>

//for printing
#include "qgscomposition.h"
#include <QBuffer>
#include <QPrinter>
#include <QSvgGenerator>
#include <QUrl>
#include <QPaintEngine>

QgsWFSServer::QgsWFSServer( QMap<QString, QString> parameters )
    : mParameterMap( parameters )
    , mConfigParser( 0 )
{
}

QgsWFSServer::~QgsWFSServer()
{
}

QgsWFSServer::QgsWFSServer()
{
}

QDomDocument QgsWFSServer::getCapabilities()
{
  QgsDebugMsg( "Entering." );
  QDomDocument doc;
  //wfs:WFS_Capabilities element
  QDomElement wfsCapabilitiesElement = doc.createElement( "WFS_Capabilities"/*wms:WFS_Capabilities*/ );
  wfsCapabilitiesElement.setAttribute( "xmlns", "http://www.opengis.net/wfs" );
  wfsCapabilitiesElement.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
  wfsCapabilitiesElement.setAttribute( "xsi:schemaLocation", "http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.0.0/wfs.xsd" );
  wfsCapabilitiesElement.setAttribute( "xmlns:ogc", "http://www.opengis.net/ogc" );
  wfsCapabilitiesElement.setAttribute( "xmlns:gml", "http://www.opengis.net/gml" );
  wfsCapabilitiesElement.setAttribute( "xmlns:ows", "http://www.opengis.net/ows" );
  wfsCapabilitiesElement.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
  wfsCapabilitiesElement.setAttribute( "version", "1.0.0" );
  wfsCapabilitiesElement.setAttribute( "updateSequence", "0" );
  doc.appendChild( wfsCapabilitiesElement );

  if ( mConfigParser )
  {
    mConfigParser->serviceCapabilities( wfsCapabilitiesElement, doc );
  }

  //wfs:Capability element
  QDomElement capabilityElement = doc.createElement( "Capability"/*wfs:Capability*/ );
  wfsCapabilitiesElement.appendChild( capabilityElement );

  //wfs:Request element
  QDomElement requestElement = doc.createElement( "Request"/*wfs:Request*/ );
  capabilityElement.appendChild( requestElement );
  //wfs:GetCapabilities
  QDomElement getCapabilitiesElement = doc.createElement( "GetCapabilities"/*wfs:GetCapabilities*/ );
  requestElement.appendChild( getCapabilitiesElement );
  QDomElement capabilitiesFormatElement = doc.createElement( "Format" );/*wfs:Format*/
  getCapabilitiesElement.appendChild( capabilitiesFormatElement );
  QDomText capabilitiesFormatText = doc.createTextNode( "text/xml" );
  capabilitiesFormatElement.appendChild( capabilitiesFormatText );

  QDomElement dcpTypeElement = doc.createElement( "DCPType"/*wfs:DCPType*/ );
  getCapabilitiesElement.appendChild( dcpTypeElement );
  QDomElement httpElement = doc.createElement( "HTTP"/*wfs:HTTP*/ );
  dcpTypeElement.appendChild( httpElement );

  //Prepare url
  //Some client requests already have http://<SERVER_NAME> in the REQUEST_URI variable
  QString hrefString;
  QString requestUrl = getenv( "REQUEST_URI" );
  QUrl mapUrl( requestUrl );
  mapUrl.setHost( QString( getenv( "SERVER_NAME" ) ) );

  //Add non-default ports to url
  QString portString = getenv( "SERVER_PORT" );
  if ( !portString.isEmpty() )
  {
    bool portOk;
    int portNumber = portString.toInt( &portOk );
    if ( portOk )
    {
      if ( portNumber != 80 )
      {
        mapUrl.setPort( portNumber );
      }
    }
  }

  if ( QString( getenv( "HTTPS" ) ).compare( "on", Qt::CaseInsensitive ) == 0 )
  {
    mapUrl.setScheme( "https" );
  }
  else
  {
    mapUrl.setScheme( "http" );
  }

  QList<QPair<QString, QString> > queryItems = mapUrl.queryItems();
  QList<QPair<QString, QString> >::const_iterator queryIt = queryItems.constBegin();
  for ( ; queryIt != queryItems.constEnd(); ++queryIt )
  {
    if ( queryIt->first.compare( "REQUEST", Qt::CaseInsensitive ) == 0 )
    {
      mapUrl.removeQueryItem( queryIt->first );
    }
    else if ( queryIt->first.compare( "VERSION", Qt::CaseInsensitive ) == 0 )
    {
      mapUrl.removeQueryItem( queryIt->first );
    }
    else if ( queryIt->first.compare( "SERVICE", Qt::CaseInsensitive ) == 0 )
    {
      mapUrl.removeQueryItem( queryIt->first );
    }
    else if ( queryIt->first.compare( "_DC", Qt::CaseInsensitive ) == 0 )
    {
      mapUrl.removeQueryItem( queryIt->first );
    }
  }
  hrefString = mapUrl.toString();

  //only Get supported for the moment
  QDomElement getElement = doc.createElement( "Get"/*wfs:Get*/ );
  httpElement.appendChild( getElement );
  QDomElement olResourceElement = doc.createElement( "OnlineResource"/*wfs:OnlineResource*/ );
  olResourceElement.setAttribute( "xlink:type", "simple" );
  requestUrl.truncate( requestUrl.indexOf( "?" ) + 1 );
  olResourceElement.setAttribute( "xlink:href", hrefString );
  getElement.appendChild( olResourceElement );

  //wfs:DescribeFeatureType
  QDomElement describeFeatureTypeElement = doc.createElement( "DescribeFeatureType"/*wfs:DescribeFeatureType*/ );
  requestElement.appendChild( describeFeatureTypeElement );
  QDomElement schemaDescriptionLanguageElement = doc.createElement( "SchemaDescriptionLanguage"/*wfs:SchemaDescriptionLanguage*/ );
  describeFeatureTypeElement.appendChild( schemaDescriptionLanguageElement );
  QDomElement xmlSchemaElement = doc.createElement( "XMLSCHEMA"/*wfs:XMLSCHEMA*/ );
  schemaDescriptionLanguageElement.appendChild( xmlSchemaElement );
  QDomElement describeFeatureTypeDhcTypeElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  describeFeatureTypeElement.appendChild( describeFeatureTypeDhcTypeElement );

  //wfs:GetFeature
  QDomElement getFeatureElement = doc.createElement( "GetFeature"/*wfs:GetFeature*/ );
  requestElement.appendChild( getFeatureElement );
  QDomElement getFeatureFormatElement = doc.createElement( "ResultFormat" );/*wfs:ResultFormat*/
  getFeatureElement.appendChild( getFeatureFormatElement );
  QDomElement gmlFormatElement = doc.createElement( "GML2" );/*wfs:GML2*/
  getFeatureFormatElement.appendChild( gmlFormatElement );
  QDomElement geojsonFormatElement = doc.createElement( "GeoJSON" );/*wfs:GeoJSON*/
  getFeatureFormatElement.appendChild( geojsonFormatElement );
  QDomElement getFeatureDhcTypeElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  getFeatureElement.appendChild( getFeatureDhcTypeElement );

  //wfs:FeatureTypeList element
  QDomElement featureTypeListElement = doc.createElement( "FeatureTypeList"/*wfs:FeatureTypeList*/ );
  wfsCapabilitiesElement.appendChild( featureTypeListElement );
  //wfs:Operations element
  QDomElement operationsElement = doc.createElement( "Operations"/*wfs:Operations*/ );
  featureTypeListElement.appendChild( operationsElement );
  //wfs:Query element
  QDomElement queryElement = doc.createElement( "Query"/*wfs:Query*/ );
  operationsElement.appendChild( queryElement );
  /*
   * Adding layer liste in featureTypeListElement
   */
  if ( mConfigParser )
  {
    mConfigParser->featureTypeList( featureTypeListElement, doc );
  }

  /*
   * Adding ogc:Filter_Capabilities in capabilityElement
   */
  //ogc:Filter_Capabilities element
  QDomElement filterCapabilitiesElement = doc.createElement( "ogc:Filter_Capabilities"/*ogc:Filter_Capabilities*/ );
  capabilityElement.appendChild( filterCapabilitiesElement );
  QDomElement spatialCapabilitiesElement = doc.createElement( "ogc:Spatial_Capabilities"/*ogc:Spatial_Capabilities*/ );
  filterCapabilitiesElement.appendChild( spatialCapabilitiesElement );
  QDomElement spatialOperatorsElement = doc.createElement( "ogc:Spatial_Operators"/*ogc:Spatial_Operators*/ );
  spatialCapabilitiesElement.appendChild( spatialOperatorsElement );
  QDomElement ogcBboxElement = doc.createElement( "ogc:BBOX"/*ogc:BBOX*/ );
  spatialOperatorsElement.appendChild( ogcBboxElement );
  QDomElement scalarCapabilitiesElement = doc.createElement( "ogc:Scalar_Capabilities"/*ogc:Scalar_Capabilities*/ );
  filterCapabilitiesElement.appendChild( scalarCapabilitiesElement );
  QDomElement comparisonOperatorsElement = doc.createElement( "ogc:Comparison_Operators"/*ogc:Comparison_Operators*/ );
  scalarCapabilitiesElement.appendChild( comparisonOperatorsElement );
  QDomElement simpleComparisonsElement = doc.createElement( "ogc:Simple_Comparisons"/*ogc:Simple_Comparisons*/ );
  comparisonOperatorsElement.appendChild( simpleComparisonsElement );
  return doc;
}

QDomDocument QgsWFSServer::describeFeatureType()
{
  QgsDebugMsg( "Entering." );
  QDomDocument doc;
  //xsd:schema
  QDomElement schemaElement = doc.createElement( "schema"/*xsd:schema*/ );
  schemaElement.setAttribute( "xmlns", "http://www.w3.org/2001/XMLSchema" );
  schemaElement.setAttribute( "xmlns:xsd", "http://www.w3.org/2001/XMLSchema" );
  schemaElement.setAttribute( "xmlns:ogc", "http://www.opengis.net/ogc" );
  schemaElement.setAttribute( "xmlns:gml", "http://www.opengis.net/gml" );
  schemaElement.setAttribute( "xmlns:qgs", "http://www.qgis.org/gml" );
  schemaElement.setAttribute( "targetNamespace", "http://www.qgis.org/gml" );
  doc.appendChild( schemaElement );

  //xsd:import
  QDomElement importElement = doc.createElement( "import"/*xsd:import*/ );
  importElement.setAttribute( "namespace", "http://www.opengis.net/gml" );
  importElement.setAttribute( "schemaLocation", "http://schemas.opengis.net/gml/2.1.2/feature.xsd" );
  schemaElement.appendChild( importElement );

  //read TYPENAME
  QString typeName;
  QMap<QString, QString>::const_iterator type_name_it = mParameterMap.find( "TYPENAME" );
  if ( type_name_it != mParameterMap.end() )
  {
    typeName = type_name_it.value();
  }
  else
  {
    return doc;
  }

  QStringList wfsLayersId = mConfigParser->wfsLayers();
  QMap< QString, QMap< int, QString > > aliasInfo = mConfigParser->layerAliasInfo();
  QMap< QString, QSet<QString> > hiddenAttributes = mConfigParser->hiddenAttributes();

  QList<QgsMapLayer*> layerList;
  QgsMapLayer* currentLayer = 0;

  layerList = mConfigParser->mapLayerFromStyle( typeName, "" );
  currentLayer = layerList.at( 0 );

  QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>( currentLayer );
  if ( layer && wfsLayersId.contains( layer->id() ) )
  {
    //is there alias info for this vector layer?
    QMap< int, QString > layerAliasInfo;
    QMap< QString, QMap< int, QString > >::const_iterator aliasIt = aliasInfo.find( currentLayer->id() );
    if ( aliasIt != aliasInfo.constEnd() )
    {
      layerAliasInfo = aliasIt.value();
    }

    //hidden attributes for this layer
    QSet<QString> layerHiddenAttributes;
    QMap< QString, QSet<QString> >::const_iterator hiddenIt = hiddenAttributes.find( currentLayer->id() );
    if ( hiddenIt != hiddenAttributes.constEnd() )
    {
      layerHiddenAttributes = hiddenIt.value();
    }

    //do a select with searchRect and go through all the features
    QgsVectorDataProvider* provider = layer->dataProvider();
    if ( !provider )
    {
      return doc;
    }

    typeName = typeName.replace( QString( " " ), QString( "_" ) );

    //xsd:element
    QDomElement elementElem = doc.createElement( "element"/*xsd:element*/ );
    elementElem.setAttribute( "name", typeName );
    elementElem.setAttribute( "type", "qgs:" + typeName + "Type" );
    elementElem.setAttribute( "substitutionGroup", "gml:_Feature" );
    schemaElement.appendChild( elementElem );

    //xsd:complexType
    QDomElement complexTypeElem = doc.createElement( "complexType"/*xsd:complexType*/ );
    complexTypeElem.setAttribute( "name", typeName + "Type" );
    schemaElement.appendChild( complexTypeElem );

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
    geomElem.setAttribute( "type", "gml:GeometryPropertyType" );
    geomElem.setAttribute( "minOccurs", "0" );
    geomElem.setAttribute( "maxOccurs", "1" );
    sequenceElem.appendChild( geomElem );

    const QgsFieldMap& fields = provider->fields();
    for ( QgsFieldMap::const_iterator it = fields.begin(); it != fields.end(); ++it )
    {

      QString attributeName = it.value().name();
      //skip attribute if it has edit type 'hidden'
      if ( layerHiddenAttributes.contains( attributeName ) )
      {
        continue;
      }

      //xsd:element
      QDomElement geomElem = doc.createElement( "element"/*xsd:element*/ );
      geomElem.setAttribute( "name", attributeName );
      if ( it.value().type() == 2 )
        geomElem.setAttribute( "type", "integer" );
      else if ( it.value().type() == 6 )
        geomElem.setAttribute( "type", "double" );
      else
        geomElem.setAttribute( "type", "string" );

      sequenceElem.appendChild( geomElem );

      //check if the attribute name should be replaced with an alias
      QMap<int, QString>::const_iterator aliasIt = layerAliasInfo.find( it.key() );
      if ( aliasIt != layerAliasInfo.constEnd() )
      {
        geomElem.setAttribute( "alias", aliasIt.value() );
      }

    }
  }

  return doc;
}

int QgsWFSServer::getFeature( QgsRequestHandler& request, const QString& format )
{
  QgsDebugMsg( "Info format is:" + format );

  //read TYPENAME
  QMap<QString, QString>::const_iterator type_name_it = mParameterMap.find( "TYPENAME" );
  if ( type_name_it != mParameterMap.end() )
  {
    mTypeName = type_name_it.value();
  }
  else
  {
    return 1;
  }

  QStringList wfsLayersId = mConfigParser->wfsLayers();
  QMap< QString, QMap< int, QString > > aliasInfo = mConfigParser->layerAliasInfo();
  QMap< QString, QSet<QString> > hiddenAttributes = mConfigParser->hiddenAttributes();

  QList<QgsMapLayer*> layerList;
  QgsMapLayer* currentLayer = 0;

  layerList = mConfigParser->mapLayerFromStyle( mTypeName, "" );
  currentLayer = layerList.at( 0 );

  QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>( currentLayer );
  if ( layer && wfsLayersId.contains( layer->id() ) )
  {
    //is there alias info for this vector layer?
    QMap< int, QString > layerAliasInfo;
    QMap< QString, QMap< int, QString > >::const_iterator aliasIt = aliasInfo.find( currentLayer->id() );
    if ( aliasIt != aliasInfo.constEnd() )
    {
      layerAliasInfo = aliasIt.value();
    }

    //hidden attributes for this layer
    QSet<QString> layerHiddenAttributes;
    QMap< QString, QSet<QString> >::const_iterator hiddenIt = hiddenAttributes.find( currentLayer->id() );
    if ( hiddenIt != hiddenAttributes.constEnd() )
    {
      layerHiddenAttributes = hiddenIt.value();
    }

    //do a select with searchRect and go through all the features
    QgsVectorDataProvider* provider = layer->dataProvider();
    if ( !provider )
    {
      return 2;
    }

    QgsFeature feature;
    QgsAttributeMap featureAttributes;
    const QgsFieldMap& fields = provider->fields();

    //map extent
    QgsRectangle searchRect = layer->extent();

    //read FEATUREDID
    bool fidOk = false;
    QString fid;
    QMap<QString, QString>::const_iterator fidIt = mParameterMap.find( "FEATUREID" );
    if ( fidIt != mParameterMap.end() )
    {
      fidOk = true;
      fid = fidIt.value();
    }

    //read FILTER
    bool filterOk = false;
    QDomDocument filter;
    QMap<QString, QString>::const_iterator filterIt = mParameterMap.find( "FILTER" );
    if ( filterIt != mParameterMap.end() )
    {
      try
      {
        QString errorMsg;
        if ( !filter.setContent( filterIt.value(), true, &errorMsg ) )
        {
          QgsDebugMsg( "soap request parse error" );
          QgsDebugMsg( "error message: " + errorMsg );
          QgsDebugMsg( "the xml string was:" );
          QgsDebugMsg( filterIt.value() );
        }
        else
        {
          filterOk = true;
        }
      }
      catch ( QgsMapServiceException& e )
      {
        Q_UNUSED( e );
        filterOk = false;
      }
    }


    bool conversionSuccess;
    double minx, miny, maxx, maxy;
    bool bboxOk = false;
    //read BBOX
    QMap<QString, QString>::const_iterator bbIt = mParameterMap.find( "BBOX" );
    if ( bbIt == mParameterMap.end() )
    {
      minx = 0; miny = 0; maxx = 0; maxy = 0;
    }
    else
    {
      bboxOk = true;
      QString bbString = bbIt.value();
      minx = bbString.section( ",", 0, 0 ).toDouble( &conversionSuccess );
      if ( !conversionSuccess ) {bboxOk = false;}
      miny = bbString.section( ",", 1, 1 ).toDouble( &conversionSuccess );
      if ( !conversionSuccess ) {bboxOk = false;}
      maxx = bbString.section( ",", 2, 2 ).toDouble( &conversionSuccess );
      if ( !conversionSuccess ) {bboxOk = false;}
      maxy = bbString.section( ",", 3, 3 ).toDouble( &conversionSuccess );
      if ( !conversionSuccess ) {bboxOk = false;}
    }

    //read MAXFEATURES
    long maxFeat = layer->featureCount();
    long featureCounter = 0;
    QMap<QString, QString>::const_iterator mfIt = mParameterMap.find( "MAXFEATURES" );
    if ( mfIt != mParameterMap.end() )
    {
      QString mfString = mfIt.value();
      bool mfOk;
      maxFeat = mfString.toLong( &mfOk, 10 );
      if ( !mfOk ) { maxFeat = layer->featureCount(); }
    }

    //read PROPERTYNAME
    mWithGeom = true;
    QgsAttributeList attrIndexes = provider->attributeIndexes();
    QMap<QString, QString>::const_iterator pnIt = mParameterMap.find( "PROPERTYNAME" );
    if ( pnIt != mParameterMap.end() )
    {
      QStringList attrList = pnIt.value().split( "," );
      if ( attrList.size() > 0 )
      {
        mWithGeom = false;
        QStringList::const_iterator alstIt;
        QList<int> idxList;
        QMap<QString, int> fieldMap = provider->fieldNameMap();
        QMap<QString, int>::const_iterator fieldIt;
        QString fieldName;
        for ( alstIt = attrList.begin(); alstIt != attrList.end(); ++alstIt )
        {
          fieldName = *alstIt;
          fieldIt = fieldMap.find( fieldName );
          if ( fieldIt != fieldMap.end() )
          {
            idxList.append( fieldIt.value() );
          }
          else if ( fieldName == "geometry" )
          {
            mWithGeom = true;
          }
        }
        if ( idxList.size() > 0 || mWithGeom )
        {
          attrIndexes = idxList;
        }
        else
        {
          mWithGeom = true;
        }
      }
    }

    QgsCoordinateReferenceSystem layerCrs = layer->crs();

    startGetFeature( request, format );

    if ( fidOk )
    {
      provider->featureAtId( fid.toInt(), feature, mWithGeom, attrIndexes );
      sendGetFeature( request, format, &feature, 0, layerCrs, fields, layerHiddenAttributes );
    }
    else if ( filterOk )
    {
      provider->select( attrIndexes, searchRect, mWithGeom, true );
      try
      {
        QgsFilter* mFilter = QgsFilter::createFilterFromXml( filter.firstChild().toElement().firstChild().toElement(), layer );
        while ( provider->nextFeature( feature ) && featureCounter < maxFeat )
        {
          if ( mFilter )
          {
            if ( mFilter->evaluate( feature ) )
            {
              sendGetFeature( request, format, &feature, featureCounter, layerCrs, fields, layerHiddenAttributes );
              ++featureCounter;
            }
          }
          else
          {
            sendGetFeature( request, format, &feature, featureCounter, layerCrs, fields, layerHiddenAttributes );
            ++featureCounter;
          }
        }
        delete mFilter;
      }
      catch ( QgsMapServiceException& e )
      {
        Q_UNUSED( e );

        while ( provider->nextFeature( feature ) && featureCounter < maxFeat )
        {
          sendGetFeature( request, format, &feature, featureCounter, layerCrs, fields, layerHiddenAttributes );
          ++featureCounter;
        }
      }
    }
    else
    {
      if ( bboxOk )
        searchRect.set( minx, miny, maxx, maxy );
      provider->select( attrIndexes, searchRect, mWithGeom, true );
      while ( provider->nextFeature( feature ) && featureCounter < maxFeat )
      {
        sendGetFeature( request, format, &feature, featureCounter, layerCrs, fields, layerHiddenAttributes );
        ++featureCounter;
      }
    }

    endGetFeature( request, format );
  }
  else
  {
    return 2;
  }
  return 0;
}

void QgsWFSServer::startGetFeature( QgsRequestHandler& request, const QString& format )
{
  QByteArray result;
  QString fcString;
  if ( format == "GeoJSON" )
  {
    fcString = "{\"type\": \"FeatureCollection\",\n";
    fcString += " \"features\": [\n";
    result = fcString.toUtf8();
    request.startGetFeatureResponse( &result, format );
  }
  else
  {
    //wfs:FeatureCollection
    fcString = "<wfs:FeatureCollection";
    fcString += " xmlns=\"http://www.opengis.net/wfs\"";
    fcString += " xmlns:wfs=\"http://www.opengis.net/wfs\"";
    fcString += " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"";
    fcString += " xsi:schemaLocation=\"http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.0.0/wfs.xsd\"";
    fcString += " xmlns:ogc=\"http://www.opengis.net/ogc\"";
    fcString += " xmlns:gml=\"http://www.opengis.net/gml\"";
    fcString += " xmlns:ows=\"http://www.opengis.net/ows\"";
    fcString += " xmlns:xlink=\"http://www.w3.org/1999/xlink\"";
    fcString += " xmlns:qgs=\"http://www.qgis.org/gml\"";
    fcString += ">";
    result = fcString.toUtf8();
    request.startGetFeatureResponse( &result, format );
  }
  fcString = "";
}

void QgsWFSServer::sendGetFeature( QgsRequestHandler& request, const QString& format, QgsFeature* feat, int featIdx, QgsCoordinateReferenceSystem& crs, QMap< int, QgsField > fields, QSet<QString> hiddenAttributes ) /*const*/
{
  QByteArray result;
  if ( format == "GeoJSON" )
  {
    QString fcString;
    if ( featIdx == 0 )
      fcString += "  ";
    else
      fcString += " ,";
    fcString += createFeatureGeoJSON( feat, crs, fields, hiddenAttributes );
    fcString += "\n";

    result = fcString.toUtf8();
    request.sendGetFeatureResponse( &result );
    fcString = "";
  }
  else
  {
    QDomDocument gmlDoc;
    QDomElement featureElement = createFeatureElem( feat, gmlDoc, crs, fields, hiddenAttributes );
    gmlDoc.appendChild( featureElement );

    result = gmlDoc.toByteArray();
    request.sendGetFeatureResponse( &result );
    gmlDoc.removeChild( featureElement );
  }
}

void QgsWFSServer::endGetFeature( QgsRequestHandler& request, const QString& format )
{
  QByteArray result;
  QString fcString;
  if ( format == "GeoJSON" )
  {
    fcString += " ]\n";
    fcString += "}";

    result = fcString.toUtf8();
    request.endGetFeatureResponse( &result );
    fcString = "";
  }
  else
  {
    fcString = "</wfs:FeatureCollection>";
    result = fcString.toUtf8();
    request.endGetFeatureResponse( &result );
    fcString = "";
  }
}

QString QgsWFSServer::createFeatureGeoJSON( QgsFeature* feat, QgsCoordinateReferenceSystem &, QMap< int, QgsField > fields, QSet<QString> hiddenAttributes ) /*const*/
{
  QString fStr = "{\"type\": \"Feature\",\n";

  fStr += "   \"id\": ";
  fStr +=  QString::number( feat->id() );
  fStr += ",\n";

  QgsGeometry* geom = feat->geometry();
  if ( geom && mWithGeom )
  {
    fStr += "  \"geometry\": ";
    fStr += geom->exportToGeoJSON();
    fStr += ",\n";
  }

  //read all attribute values from the feature
  fStr += "   \"properties\": {\n";
  QgsAttributeMap featureAttributes = feat->attributeMap();
  int attributeCounter = 0;
  for ( QgsAttributeMap::const_iterator it = featureAttributes.begin(); it != featureAttributes.end(); ++it )
  {
    QString attributeName = fields[it.key()].name();
    //skip attribute if it has edit type 'hidden'
    if ( hiddenAttributes.contains( attributeName ) )
    {
      continue;
    }

    if ( attributeCounter == 0 )
      fStr += "    \"";
    else
      fStr += "   ,\"";
    fStr += attributeName;
    fStr += "\": ";
    if ( it->type() == 6 || it->type() == 2 )
    {
      fStr +=  it->toString();
    }
    else
    {
      fStr += "\"";
      fStr +=  it->toString().replace( QString( "\"" ), QString( "\\\"" ) );
      fStr += "\"";
    }
    fStr += "\n";
    ++attributeCounter;
  }

  fStr += "   }\n";

  fStr += "  }";

  return fStr;
}

QDomElement QgsWFSServer::createFeatureElem( QgsFeature* feat, QDomDocument& doc, QgsCoordinateReferenceSystem& crs, QMap< int, QgsField > fields, QSet<QString> hiddenAttributes ) /*const*/
{
  //gml:FeatureMember
  QDomElement featureElement = doc.createElement( "gml:featureMember"/*wfs:FeatureMember*/ );

  //qgs:%TYPENAME%
  QDomElement typeNameElement = doc.createElement( "qgs:" + mTypeName.replace( QString( " " ), QString( "_" ) )/*qgs:%TYPENAME%*/ );
  typeNameElement.setAttribute( "fid", QString::number( feat->id() ) );
  featureElement.appendChild( typeNameElement );

  if ( mWithGeom )
  {
    //add geometry column (as gml)
    QDomElement geomElem = doc.createElement( "qgs:geometry" );
    QDomElement gmlElem = createGeometryElem( feat->geometry(), doc );
    if ( !gmlElem.isNull() )
    {
      if ( crs.isValid() )
      {
        gmlElem.setAttribute( "srsName", crs.authid() );
      }
      geomElem.appendChild( gmlElem );
      typeNameElement.appendChild( geomElem );
    }
  }

  //read all attribute values from the feature
  QgsAttributeMap featureAttributes = feat->attributeMap();
  for ( QgsAttributeMap::const_iterator it = featureAttributes.begin(); it != featureAttributes.end(); ++it )
  {

    QString attributeName = fields[it.key()].name();
    //skip attribute if it has edit type 'hidden'
    if ( hiddenAttributes.contains( attributeName ) )
    {
      continue;
    }

    QDomElement fieldElem = doc.createElement( "qgs:" + attributeName.replace( QString( " " ), QString( "_" ) ) );
    QDomText fieldText = doc.createTextNode( it->toString() );
    fieldElem.appendChild( fieldText );
    typeNameElement.appendChild( fieldElem );
  }

  return featureElement;
}

QDomElement QgsWFSServer::createGeometryElem( QgsGeometry* geom, QDomDocument& doc ) /*const*/
{
  if ( !geom )
  {
    return QDomElement();
  }

  QDomElement geomElement;

  QString geomTypeName;
  QGis::WkbType wkbType = geom->wkbType();
  switch ( wkbType )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
      geomElement = createPointElem( geom, doc );
      break;
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
      geomElement = createMultiPointElem( geom, doc );
      break;
    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
      geomElement = createLineStringElem( geom, doc );
      break;
    case QGis::WKBMultiLineString:
    case QGis::WKBMultiLineString25D:
      geomElement = createMultiLineStringElem( geom, doc );
      break;
    case QGis::WKBPolygon:
    case QGis::WKBPolygon25D:
      geomElement = createPolygonElem( geom, doc );
      break;
    case QGis::WKBMultiPolygon:
    case QGis::WKBMultiPolygon25D:
      geomElement = createMultiPolygonElem( geom, doc );
      break;
    default:
      return QDomElement();
  }
  return geomElement;
}

QDomElement QgsWFSServer::createLineStringElem( QgsGeometry* geom, QDomDocument& doc ) const
{
  if ( !geom )
  {
    return QDomElement();
  }

  QDomElement lineStringElem = doc.createElement( "gml:LineString" );
  QDomElement coordElem = createCoordinateElem( geom->asPolyline(), doc );
  lineStringElem.appendChild( coordElem );
  return lineStringElem;
}

QDomElement QgsWFSServer::createMultiLineStringElem( QgsGeometry* geom, QDomDocument& doc ) const
{
  if ( !geom )
  {
    return QDomElement();
  }

  QDomElement multiLineStringElem = doc.createElement( "gml:MultiLineString" );
  QgsMultiPolyline multiline = geom->asMultiPolyline();

  QgsMultiPolyline::const_iterator multiLineIt = multiline.constBegin();
  for ( ; multiLineIt != multiline.constEnd(); ++multiLineIt )
  {
    QgsGeometry* lineGeom = QgsGeometry::fromPolyline( *multiLineIt );
    if ( lineGeom )
    {
      QDomElement lineStringMemberElem = doc.createElement( "gml:lineStringMember" );
      QDomElement lineElem = createLineStringElem( lineGeom, doc );
      lineStringMemberElem.appendChild( lineElem );
      multiLineStringElem.appendChild( lineStringMemberElem );
    }
    delete lineGeom;
  }

  return multiLineStringElem;
}

QDomElement QgsWFSServer::createPointElem( QgsGeometry* geom, QDomDocument& doc ) const
{
  if ( !geom )
  {
    return QDomElement();
  }

  QDomElement pointElem = doc.createElement( "gml:Point" );
  QgsPoint p = geom->asPoint();
  QVector<QgsPoint> v;
  v.append( p );
  QDomElement coordElem = createCoordinateElem( v, doc );
  pointElem.appendChild( coordElem );
  return pointElem;
}

QDomElement QgsWFSServer::createMultiPointElem( QgsGeometry* geom, QDomDocument& doc ) const
{
  if ( !geom )
  {
    return QDomElement();
  }

  QDomElement multiPointElem = doc.createElement( "gml:MultiPoint" );
  QgsMultiPoint multiPoint = geom->asMultiPoint();

  QgsMultiPoint::const_iterator multiPointIt = multiPoint.constBegin();
  for ( ; multiPointIt != multiPoint.constEnd(); ++multiPointIt )
  {
    QgsGeometry* pointGeom = QgsGeometry::fromPoint( *multiPointIt );
    if ( pointGeom )
    {
      QDomElement multiPointMemberElem = doc.createElement( "gml:pointMember" );
      QDomElement pointElem = createPointElem( pointGeom, doc );
      multiPointMemberElem.appendChild( pointElem );
      multiPointElem.appendChild( multiPointMemberElem );
    }
  }
  return multiPointElem;
}

QDomElement QgsWFSServer::createPolygonElem( QgsGeometry* geom, QDomDocument& doc ) const
{
  if ( !geom )
  {
    return QDomElement();
  }

  QDomElement polygonElem = doc.createElement( "gml:Polygon" );
  QgsPolygon poly = geom->asPolygon();
  for ( int i = 0; i < poly.size(); ++i )
  {
    QString boundaryName;
    if ( i == 0 )
    {
      boundaryName = "outerBoundaryIs";
    }
    else
    {
      boundaryName = "innerBoundaryIs";
    }
    QDomElement boundaryElem = doc.createElementNS( "http://www.opengis.net/gml", boundaryName );
    QDomElement ringElem = doc.createElement( "gml:LinearRing" );
    QDomElement coordElem = createCoordinateElem( poly.at( i ), doc );
    ringElem.appendChild( coordElem );
    boundaryElem.appendChild( ringElem );
    polygonElem.appendChild( boundaryElem );
  }
  return polygonElem;
}

QDomElement QgsWFSServer::createMultiPolygonElem( QgsGeometry* geom, QDomDocument& doc ) const
{
  if ( !geom )
  {
    return QDomElement();
  }
  QDomElement multiPolygonElem = doc.createElement( "gml:MultiPolygon" );
  QgsMultiPolygon multipoly = geom->asMultiPolygon();

  QgsMultiPolygon::const_iterator polyIt = multipoly.constBegin();
  for ( ; polyIt != multipoly.constEnd(); ++polyIt )
  {
    QgsGeometry* polygonGeom = QgsGeometry::fromPolygon( *polyIt );
    if ( polygonGeom )
    {
      QDomElement polygonMemberElem = doc.createElement( "gml:polygonMember" );
      QDomElement polygonElem = createPolygonElem( polygonGeom, doc );
      delete polygonGeom;
      polygonMemberElem.appendChild( polygonElem );
      multiPolygonElem.appendChild( polygonMemberElem );
    }
  }
  return multiPolygonElem;
}

QDomElement QgsWFSServer::createCoordinateElem( const QVector<QgsPoint> points, QDomDocument& doc ) const
{
  QDomElement coordElem = doc.createElement( "gml:coordinates" );
  coordElem.setAttribute( "cs", "," );
  coordElem.setAttribute( "ts", " " );

  //precision 4 for meters / feet, precision 8 for degrees
  int precision = 8;
  /*
  if ( mSourceCRS.mapUnits() == QGis::Meters
       || mSourceCRS.mapUnits() == QGis::Feet )
  {
    precision = 4;
  }
  */

  QString coordString;
  QVector<QgsPoint>::const_iterator pointIt = points.constBegin();
  for ( ; pointIt != points.constEnd(); ++pointIt )
  {
    if ( pointIt != points.constBegin() )
    {
      coordString += " ";
    }
    coordString += QString::number( pointIt->x(), 'f', precision );
    coordString += ",";
    coordString += QString::number( pointIt->y(), 'f', precision );
  }

  QDomText coordText = doc.createTextNode( coordString );
  coordElem.appendChild( coordText );
  return coordElem;
}
