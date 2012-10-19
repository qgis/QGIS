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

#ifndef Q_WS_WIN
#include <netinet/in.h>
#else
#include <winsock.h>
#endif

static const QString WFS_NAMESPACE = "http://www.opengis.net/wfs";
static const QString GML_NAMESPACE = "http://www.opengis.net/gml";
static const QString OGC_NAMESPACE = "http://www.opengis.net/ogc";
static const QString QGS_NAMESPACE = "http://www.qgis.org/gml";

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
  wfsCapabilitiesElement.setAttribute( "xmlns", WFS_NAMESPACE );
  wfsCapabilitiesElement.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
  wfsCapabilitiesElement.setAttribute( "xsi:schemaLocation", WFS_NAMESPACE + " http://schemas.opengis.net/wfs/1.0.0/wfs.xsd" );
  wfsCapabilitiesElement.setAttribute( "xmlns:ogc", OGC_NAMESPACE );
  wfsCapabilitiesElement.setAttribute( "xmlns:gml", GML_NAMESPACE );
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

  //wfs:Transaction
  QDomElement transactionElement = doc.createElement( "Transaction"/*wfs:Transaction*/ );
  requestElement.appendChild( transactionElement );
  QDomElement transactionDhcTypeElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  transactionDhcTypeElement.firstChild().firstChild().toElement().setTagName( "Post" );
  transactionElement.appendChild( transactionDhcTypeElement );

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
  wfsCapabilitiesElement.appendChild( filterCapabilitiesElement );
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
  schemaElement.setAttribute( "xmlns:ogc", OGC_NAMESPACE );
  schemaElement.setAttribute( "xmlns:gml", GML_NAMESPACE );
  schemaElement.setAttribute( "xmlns:qgs", QGS_NAMESPACE );
  schemaElement.setAttribute( "targetNamespace", QGS_NAMESPACE );
  schemaElement.setAttribute( "elementFromDefault", "qualified" );
  schemaElement.setAttribute( "version", "1.0" );
  doc.appendChild( schemaElement );

  //xsd:import
  QDomElement importElement = doc.createElement( "import"/*xsd:import*/ );
  importElement.setAttribute( "namespace",  GML_NAMESPACE );
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
    typeName = "";
  }
  mConfigParser->describeFeatureType( typeName, schemaElement, doc );
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

  QList<QgsMapLayer*> layerList;
  QgsMapLayer* currentLayer = 0;

  layerList = mConfigParser->mapLayerFromStyle( mTypeName, "" );
  currentLayer = layerList.at( 0 );

  QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>( currentLayer );
  if ( layer && wfsLayersId.contains( layer->id() ) )
  {
    //is there alias info for this vector layer?
    QMap< int, QString > layerAliasInfo;
    const QMap< QString, QString >& aliasMap = layer->attributeAliases();
    QMap< QString, QString >::const_iterator aliasIt = aliasMap.constBegin();
    for ( ; aliasIt != aliasMap.constEnd(); ++aliasIt )
    {
      int attrIndex = layer->fieldNameIndex( aliasIt.key() );
      if ( attrIndex != -1 )
      {
        layerAliasInfo.insert( attrIndex, aliasIt.value() );
      }
    }

    //excluded attributes for this layer
    const QSet<QString>& layerExcludedAttributes = layer->excludeAttributesWFS();

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

    if ( bboxOk )
      searchRect.set( minx, miny, maxx, maxy );
    else
      searchRect.set( searchRect.xMinimum() - 0.000001
                      , searchRect.yMinimum() - 0.000001
                      , searchRect.xMaximum() + 0.000001
                      , searchRect.yMaximum() + 0.000001 );
    QgsCoordinateReferenceSystem layerCrs = layer->crs();

    startGetFeature( request, format, layerCrs, &searchRect );

    if ( fidOk )
    {
      provider->featureAtId( fid.toInt(), feature, mWithGeom, attrIndexes );
      sendGetFeature( request, format, &feature, 0, layerCrs, fields, layerExcludedAttributes );
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
              sendGetFeature( request, format, &feature, featureCounter, layerCrs, fields, layerExcludedAttributes );
              ++featureCounter;
            }
          }
          else
          {
            sendGetFeature( request, format, &feature, featureCounter, layerCrs, fields, layerExcludedAttributes );
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
          sendGetFeature( request, format, &feature, featureCounter, layerCrs, fields, layerExcludedAttributes );
          ++featureCounter;
        }
      }
    }
    else
    {
      provider->select( attrIndexes, searchRect, mWithGeom, true );
      while ( provider->nextFeature( feature ) && featureCounter < maxFeat )
      {
        sendGetFeature( request, format, &feature, featureCounter, layerCrs, fields, layerExcludedAttributes );
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

void QgsWFSServer::startGetFeature( QgsRequestHandler& request, const QString& format, QgsCoordinateReferenceSystem& crs, QgsRectangle* rect )
{
  QByteArray result;
  QString fcString;
  if ( format == "GeoJSON" )
  {
    fcString = "{\"type\": \"FeatureCollection\",\n";
    fcString += " \"bbox\": [ " + QString::number( rect->xMinimum(), 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) ) + ", " + QString::number( rect->yMinimum(), 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) ) + ", " + QString::number( rect->xMaximum(), 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) ) + ", " + QString::number( rect->yMaximum(), 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) ) + "],\n";
    fcString += " \"features\": [\n";
    result = fcString.toUtf8();
    request.startGetFeatureResponse( &result, format );
  }
  else
  {
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
        mapUrl.addQueryItem( queryIt->first, "DescribeFeatureType" );
      }
      else if ( queryIt->first.compare( "FORMAT", Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( "OUTPUTFORMAT", Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( "BBOX", Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( "FEATUREID", Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( "FILTER", Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( "MAXFEATURES", Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( "PROPERTYNAME", Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( "_DC", Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
    }
    mapUrl.addQueryItem( "OUTPUTFORMAT", "XMLSCHEMA" );
    hrefString = mapUrl.toString();

    //wfs:FeatureCollection valid
    fcString = "<wfs:FeatureCollection";
    fcString += " xmlns:wfs=\"" + WFS_NAMESPACE + "\"";
    fcString += " xmlns:ogc=\"" + OGC_NAMESPACE + "\"";
    fcString += " xmlns:gml=\"" + GML_NAMESPACE + "\"";
    fcString += " xmlns:ows=\"http://www.opengis.net/ows\"";
    fcString += " xmlns:xlink=\"http://www.w3.org/1999/xlink\"";
    fcString += " xmlns:qgs=\"" + QGS_NAMESPACE + "\"";
    fcString += " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"";
    fcString += " xsi:schemaLocation=\"" + WFS_NAMESPACE + " http://schemas.opengis.net/wfs/1.0.0/wfs.xsd " + QGS_NAMESPACE + " " + hrefString.replace( "&", "&amp;" ) + "\"";
    fcString += ">";
    result = fcString.toUtf8();
    request.startGetFeatureResponse( &result, format );

    QDomDocument doc;
    QDomElement bbElem = doc.createElement( "gml:boundedBy" );
    QDomElement boxElem = createBoxGML2( rect, doc );
    if ( !boxElem.isNull() )
    {
      if ( crs.isValid() )
      {
        boxElem.setAttribute( "srsName", crs.authid() );
      }
      bbElem.appendChild( boxElem );
      doc.appendChild( bbElem );
    }
    result = doc.toByteArray();
    request.sendGetFeatureResponse( &result );
  }
  fcString = "";
}

void QgsWFSServer::sendGetFeature( QgsRequestHandler& request, const QString& format, QgsFeature* feat, int featIdx, QgsCoordinateReferenceSystem& crs, QMap< int, QgsField > fields, QSet<QString> excludedAttributes ) /*const*/
{
  QByteArray result;
  if ( format == "GeoJSON" )
  {
    QString fcString;
    if ( featIdx == 0 )
      fcString += "  ";
    else
      fcString += " ,";
    fcString += createFeatureGeoJSON( feat, crs, fields, excludedAttributes );
    fcString += "\n";

    result = fcString.toUtf8();
    request.sendGetFeatureResponse( &result );
    fcString = "";
  }
  else
  {
    QDomDocument gmlDoc;
    QDomElement featureElement = createFeatureGML2( feat, gmlDoc, crs, fields, excludedAttributes );
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

QDomDocument QgsWFSServer::transaction( const QString& requestBody )
{
  // Getting  the transaction document
  QDomDocument doc;
  QString errorMsg;
  if ( !doc.setContent( requestBody, true, &errorMsg ) )
  {
    throw QgsMapServiceException( "RequestNotWellFormed", errorMsg );
  }

  QDomElement docElem = doc.documentElement();
  QDomNodeList docChildNodes = docElem.childNodes();

  // Re-organize the transaction document
  QDomDocument mDoc;
  QDomElement mDocElem = mDoc.createElement( "myTransactionDocument" );
  mDocElem.setAttribute( "xmlns", QGS_NAMESPACE );
  mDocElem.setAttribute( "xmlns:wfs", WFS_NAMESPACE );
  mDocElem.setAttribute( "xmlns:gml", GML_NAMESPACE );
  mDocElem.setAttribute( "xmlns:ogc", OGC_NAMESPACE );
  mDocElem.setAttribute( "xmlns:qgs", QGS_NAMESPACE );
  mDocElem.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
  mDoc.appendChild( mDocElem );

  QDomElement actionElem;
  QString actionName;
  QDomElement typeNameElem;
  QString typeName;

  for ( int i = docChildNodes.count(); 0 < i; --i )
  {
    actionElem = docChildNodes.at( i - 1 ).toElement();
    actionName = actionElem.localName();

    if ( actionName == "Insert" )
    {
      QDomElement featureElem = actionElem.firstChild().toElement();
      typeName = featureElem.localName();
    }
    else if ( actionName == "Update" )
    {
      typeName = actionElem.attribute( "typeName" );
    }
    else if ( actionName == "Delete" )
    {
      typeName = actionElem.attribute( "typeName" );
    }

    QDomNodeList typeNameList = mDocElem.elementsByTagName( typeName );
    if ( typeNameList.count() == 0 )
    {
      typeNameElem = mDoc.createElement( typeName );
      mDocElem.appendChild( typeNameElem );
    }
    else
      typeNameElem = typeNameList.at( 0 ).toElement();

    typeNameElem.appendChild( actionElem );
  }

  // It's time to make the transaction
  // Create the response document
  QDomDocument resp;
  //wfs:WFS_TransactionRespone element
  QDomElement respElem = resp.createElement( "WFS_TransactionResponse"/*wfs:WFS_TransactionResponse*/ );
  respElem.setAttribute( "xmlns", WFS_NAMESPACE );
  respElem.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
  respElem.setAttribute( "xsi:schemaLocation", WFS_NAMESPACE + " http://schemas.opengis.net/wfs/1.0.0/wfs.xsd" );
  respElem.setAttribute( "xmlns:ogc", OGC_NAMESPACE );
  respElem.setAttribute( "version", "1.0.0" );
  resp.appendChild( respElem );

  // Store the created feature id
  QgsFeatureIds insertResults;
  // Get the WFS layers id
  QStringList wfsLayersId = mConfigParser->wfsLayers();

  QList<QgsMapLayer*> layerList;
  QgsMapLayer* currentLayer = 0;

  // Loop through the layer transaction elements
  docChildNodes = mDocElem.childNodes();
  for ( int i = 0; i < docChildNodes.count(); ++i )
  {
    // Get the vector layer
    typeNameElem = docChildNodes.at( i ).toElement();
    typeName = typeNameElem.tagName();

    layerList = mConfigParser->mapLayerFromStyle( typeName, "" );
    currentLayer = layerList.at( 0 );

    QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( currentLayer );
    // it's a vectorlayer and definy by the administrator as a WFS layer
    if ( layer && wfsLayersId.contains( layer->id() ) )
    {
      // Get the provider and it's capabilities
      QgsVectorDataProvider* provider = layer->dataProvider();
      if ( !provider )
      {
        continue;
      }
      int cap = provider->capabilities();

      // Start the update transaction
      layer->startEditing();
      if (( cap & QgsVectorDataProvider::ChangeAttributeValues ) && ( cap & QgsVectorDataProvider::ChangeGeometries ) )
      {
        // Loop through the update elements for this layer
        QDomNodeList upNodeList = typeNameElem.elementsByTagNameNS( WFS_NAMESPACE, "Update" );
        for ( int j = 0; j < upNodeList.count(); ++j )
        {
          actionElem = upNodeList.at( j ).toElement();

          // Get the Feature Ids for this filter on the layer
          QDomElement filterElem = actionElem.elementsByTagName( "Filter" ).at( 0 ).toElement();
          QgsFeatureIds fids = getFeatureIdsFromFilter( filterElem, layer );

          // Loop through the property elements
          // Store properties and the geometry element
          QDomNodeList propertyNodeList = actionElem.elementsByTagName( "Property" );
          QMap<QString, QString> propertyMap;
          QDomElement propertyElem;
          QDomElement nameElem;
          QDomElement valueElem;
          QDomElement geometryElem;

          for ( int l = 0; l < propertyNodeList.count(); ++l )
          {
            propertyElem = propertyNodeList.at( l ).toElement();
            nameElem = propertyElem.elementsByTagName( "Name" ).at( 0 ).toElement();
            valueElem = propertyElem.elementsByTagName( "Value" ).at( 0 ).toElement();
            if ( nameElem.text() != "geometry" )
            {
              propertyMap.insert( nameElem.text(), valueElem.text() );
            }
            else
            {
              geometryElem = valueElem;
            }
          }

          // Update the features
          const QgsFieldMap& fields = provider->fields();
          QgsFieldMap::const_iterator fieldIt;
          QMap<QString, int> fieldMap = provider->fieldNameMap();
          QMap<QString, int>::const_iterator fieldMapIt;
          QString fieldName;
          bool conversionSuccess;

          QgsFeatureIds::const_iterator fidIt = fids.constBegin();
          for ( ; fidIt != fids.constEnd(); ++fidIt )
          {
            QMap< QString, QString >::const_iterator it = propertyMap.constBegin();
            for ( ; it != propertyMap.constEnd(); ++it )
            {
              fieldName = it.key();
              fieldMapIt = fieldMap.find( fieldName );
              if ( fieldMapIt == fieldMap.constEnd() )
              {
                continue;
              }
              fieldIt = fields.find( fieldMapIt.value() );
              if ( fieldIt == fields.constEnd() )
              {
                continue;
              }
              if ( fieldIt.value().type() == 2 )
                layer->changeAttributeValue( *fidIt, fieldIt.key(), it.value().toInt( &conversionSuccess ) );
              else if ( fieldIt.value().type() == 6 )
                layer->changeAttributeValue( *fidIt, fieldIt.key(), it.value().toDouble( &conversionSuccess ) );
              else
                layer->changeAttributeValue( *fidIt, fieldIt.key(), it.value() );
            }

            if ( !geometryElem.isNull() )
            {
              if ( !layer->changeGeometry( *fidIt, QgsGeometry::fromGML2( geometryElem ) ) )
                throw QgsMapServiceException( "RequestNotWellFormed", "Error in change geometry" );
            }
          }
        }
      }
      // Commit the changes of the update elements
      if ( !layer->commitChanges() )
      {
        QDomElement trElem = doc.createElement( "TransactionResult" );
        QDomElement stElem = doc.createElement( "Status" );
        QDomElement successElem = doc.createElement( "PARTIAL" );
        stElem.appendChild( successElem );
        trElem.appendChild( stElem );
        respElem.appendChild( trElem );

        QDomElement locElem = doc.createElement( "Locator" );
        locElem.appendChild( doc.createTextNode( "Update" ) );
        trElem.appendChild( locElem );

        QDomElement mesElem = doc.createElement( "Message" );
        mesElem.appendChild( doc.createTextNode( layer->commitErrors().join( "\n  " ) ) );
        trElem.appendChild( mesElem );

        return resp;
      }
      // Start the delete transaction
      layer->startEditing();
      if (( cap & QgsVectorDataProvider::DeleteFeatures ) )
      {
        // Loop through the delete elements
        QDomNodeList delNodeList = typeNameElem.elementsByTagNameNS( WFS_NAMESPACE, "Delete" );
        for ( int j = 0; j < delNodeList.count(); ++j )
        {
          actionElem = delNodeList.at( j ).toElement();
          QDomElement filterElem = actionElem.firstChild().toElement();
          // Get Feature Ids for the Filter element
          QgsFeatureIds fids = getFeatureIdsFromFilter( filterElem, layer );
          layer->setSelectedFeatures( fids );
          layer->deleteSelectedFeatures();
        }
      }
      // Commit the changes of the delete elements
      if ( !layer->commitChanges() )
      {
        QDomElement trElem = doc.createElement( "TransactionResult" );
        QDomElement stElem = doc.createElement( "Status" );
        QDomElement successElem = doc.createElement( "PARTIAL" );
        stElem.appendChild( successElem );
        trElem.appendChild( stElem );
        respElem.appendChild( trElem );

        QDomElement locElem = doc.createElement( "Locator" );
        locElem.appendChild( doc.createTextNode( "Delete" ) );
        trElem.appendChild( locElem );

        QDomElement mesElem = doc.createElement( "Message" );
        mesElem.appendChild( doc.createTextNode( layer->commitErrors().join( "\n  " ) ) );
        trElem.appendChild( mesElem );

        return resp;
      }
      // Start the insert transaction
      layer->startEditing();
      // Store the inserted features
      QgsFeatureList inFeatList;
      if ( cap & QgsVectorDataProvider::AddFeatures )
      {
        // Get Layer Field Information
        const QgsFieldMap& fields = provider->fields();
        QgsFieldMap::const_iterator fieldIt;
        QMap<QString, int> fieldMap = provider->fieldNameMap();
        QMap<QString, int>::const_iterator fieldMapIt;
        QString fieldName;

        // Loop through the insert elements
        QDomNodeList inNodeList = typeNameElem.elementsByTagNameNS( WFS_NAMESPACE, "Insert" );
        for ( int j = 0; j < inNodeList.count(); ++j )
        {
          actionElem = inNodeList.at( j ).toElement();
          // Loop through the feature element
          QDomNodeList featNodes = actionElem.childNodes();
          for ( int l = 0; l < featNodes.count(); l++ )
          {
            // Create feature for this layer
            QgsFeature* f = new QgsFeature();

            QDomElement featureElem = featNodes.at( l ).toElement();

            QDomNode currentAttributeChild = featureElem.firstChild();

            while ( !currentAttributeChild.isNull() )
            {
              QDomElement currentAttributeElement = currentAttributeChild.toElement();
              QString attrName = currentAttributeElement.localName();

              if ( attrName != "boundedBy" )
              {
                if ( attrName != "geometry" ) //a normal attribute
                {
                  fieldMapIt = fieldMap.find( attrName );
                  if ( fieldMapIt == fieldMap.constEnd() )
                  {
                    continue;
                  }
                  fieldIt = fields.find( fieldMapIt.value() );
                  if ( fieldIt == fields.constEnd() )
                  {
                    continue;
                  }
                  QString attrValue = currentAttributeElement.text();
                  int attrType = fieldIt.value().type();
                  if ( attrType == 2 )
                    f->addAttribute( fieldIt.key(), attrValue.toInt() );
                  else if ( attrType == 6 )
                    f->addAttribute( fieldIt.key(), attrValue.toDouble() );
                  else
                    f->addAttribute( fieldIt.key(), attrValue );
                }
                else //a geometry attribute
                {
                  f->setGeometry( QgsGeometry::fromGML2( currentAttributeElement ) );
                }
              }
              currentAttributeChild = currentAttributeChild.nextSibling();
            }
            // Add the feature to th layer
            // and store it to put it's Feature Id in the response
            layer->addFeature( *f, true );
            inFeatList << *f;
          }
        }
      }
      // Commit the changes of the insert elements
      if ( !layer->commitChanges() )
      {
        QDomElement trElem = doc.createElement( "TransactionResult" );
        QDomElement stElem = doc.createElement( "Status" );
        QDomElement successElem = doc.createElement( "PARTIAL" );
        stElem.appendChild( successElem );
        trElem.appendChild( stElem );
        respElem.appendChild( trElem );

        QDomElement locElem = doc.createElement( "Locator" );
        locElem.appendChild( doc.createTextNode( "Insert" ) );
        trElem.appendChild( locElem );

        QDomElement mesElem = doc.createElement( "Message" );
        mesElem.appendChild( doc.createTextNode( layer->commitErrors().join( "\n  " ) ) );
        trElem.appendChild( mesElem );

        return resp;
      }
      // Get the Feature Ids of the inserted feature
      for ( int j = 0; j < inFeatList.size(); j++ )
      {
        insertResults.insert( inFeatList[j].id() );
      }
    }
  }

  // Put the Feature Ids of the inserted feature
  if ( insertResults.size() > 0 )
  {
    QDomElement irsElem = doc.createElement( "InsertResults" );
    QgsFeatureIds::const_iterator irIt = insertResults.constBegin();
    for ( ; irIt != insertResults.constEnd(); ++irIt )
    {
      QDomElement irElem = doc.createElement( "InsertResult" );
      QDomElement fiElem = doc.createElement( "ogc:FeatureId" );
      fiElem.setAttribute( "fid", *irIt );
      irElem.appendChild( fiElem );
      irsElem.appendChild( irElem );
    }
    respElem.appendChild( irsElem );
  }

  // Set the transaction reposne for success
  QDomElement trElem = doc.createElement( "TransactionResult" );
  QDomElement stElem = doc.createElement( "Status" );
  QDomElement successElem = doc.createElement( "SUCCESS" );
  stElem.appendChild( successElem );
  trElem.appendChild( stElem );
  respElem.appendChild( trElem );

  return resp;
}

QgsFeatureIds QgsWFSServer::getFeatureIdsFromFilter( QDomElement filter, QgsVectorLayer* layer )
{
  QgsFeatureIds fids;

  QgsVectorDataProvider* provider = layer->dataProvider();
  QDomElement filterFirstElem = filter.firstChild().toElement();

  if ( filterFirstElem.localName() == "FeatureId" )
  {
    bool conversionSuccess;
    fids.insert( filterFirstElem.attribute( "fid" ).toInt( &conversionSuccess ) );
  }
  else
  {
    QgsFeature feature;
    QgsFilter* mFilter = QgsFilter::createFilterFromXml( filter, layer );
    while ( provider->nextFeature( feature ) )
    {
      if ( mFilter )
      {
        if ( mFilter->evaluate( feature ) )
        {
          fids.insert( feature.id() );
        }
      }
    }
  }

  return fids;
}

QString QgsWFSServer::createFeatureGeoJSON( QgsFeature* feat, QgsCoordinateReferenceSystem &, QMap< int, QgsField > fields, QSet<QString> excludedAttributes ) /*const*/
{
  QString fStr = "{\"type\": \"Feature\",\n";

  fStr += "   \"id\": ";
  fStr +=  QString::number( feat->id() );
  fStr += ",\n";

  QgsGeometry* geom = feat->geometry();
  if ( geom && mWithGeom )
  {
    QgsRectangle box = geom->boundingBox();

    fStr += " \"bbox\": [ " + QString::number( box.xMinimum(), 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) ) + ", " + QString::number( box.yMinimum(), 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) ) + ", " + QString::number( box.xMaximum(), 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) ) + ", " + QString::number( box.yMaximum(), 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) ) + "],\n";

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
    //skip attribute if it is excluded from WFS publication
    if ( excludedAttributes.contains( attributeName ) )
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

QDomElement QgsWFSServer::createFeatureGML2( QgsFeature* feat, QDomDocument& doc, QgsCoordinateReferenceSystem& crs, QMap< int, QgsField > fields, QSet<QString> excludedAttributes ) /*const*/
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
    QgsGeometry* geom = feat->geometry();

    QDomElement geomElem = doc.createElement( "qgs:geometry" );
    QDomElement gmlElem = geom->exportToGML2( doc );
    if ( !gmlElem.isNull() )
    {
      QgsRectangle box = geom->boundingBox();
      QDomElement bbElem = doc.createElement( "gml:boundedBy" );
      QDomElement boxElem = createBoxGML2( &box, doc );

      if ( crs.isValid() )
      {
        boxElem.setAttribute( "srsName", crs.authid() );
        gmlElem.setAttribute( "srsName", crs.authid() );
      }

      bbElem.appendChild( boxElem );
      typeNameElement.appendChild( bbElem );

      geomElem.appendChild( gmlElem );
      typeNameElement.appendChild( geomElem );
    }
  }

  //read all attribute values from the feature
  QgsAttributeMap featureAttributes = feat->attributeMap();
  for ( QgsAttributeMap::const_iterator it = featureAttributes.begin(); it != featureAttributes.end(); ++it )
  {

    QString attributeName = fields[it.key()].name();
    //skip attribute if is explicitely excluded from WFS publication
    if ( excludedAttributes.contains( attributeName ) )
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

QDomElement QgsWFSServer::createBoxGML2( QgsRectangle* box, QDomDocument& doc ) /*const*/
{
  if ( !box )
  {
    return QDomElement();
  }

  QDomElement boxElem = doc.createElement( "gml:Box" );
  QVector<QgsPoint> v;
  QgsPoint p1;
  p1.set( box->xMinimum(), box->yMinimum() );
  v.append( p1 );
  QgsPoint p2;
  p2.set( box->xMaximum(), box->yMaximum() );
  v.append( p2 );
  QDomElement coordElem = createCoordinateGML2( v, doc );
  boxElem.appendChild( coordElem );

  return boxElem;
}

QDomElement QgsWFSServer::createCoordinateGML2( const QVector<QgsPoint> points, QDomDocument& doc ) const
{
  QDomElement coordElem = doc.createElement( "gml:coordinates" );
  coordElem.setAttribute( "cs", "," );
  coordElem.setAttribute( "ts", " " );

  QString coordString;
  QVector<QgsPoint>::const_iterator pointIt = points.constBegin();
  for ( ; pointIt != points.constEnd(); ++pointIt )
  {
    if ( pointIt != points.constBegin() )
    {
      coordString += " ";
    }
    coordString += QString::number( pointIt->x(), 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
    coordString += ",";
    coordString += QString::number( pointIt->y(), 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
  }

  QDomText coordText = doc.createTextNode( coordString );
  coordElem.appendChild( coordText );
  return coordElem;
}
