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
#include "qgsexpression.h"
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
#include "qgssymbolv2.h"
#include "qgslegendmodel.h"
#include "qgscomposerlegenditem.h"
#include "qgsrequesthandler.h"
#include "qgsogcutils.h"

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
  wfsCapabilitiesElement.setAttribute( "xsi:schemaLocation", WFS_NAMESPACE + " http://schemas.opengis.net/wfs/1.0.0/WFS-capabilities.xsd" );
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

  QDomElement dcpTypeElement = doc.createElement( "DCPType"/*wfs:DCPType*/ );
  getCapabilitiesElement.appendChild( dcpTypeElement );
  QDomElement httpElement = doc.createElement( "HTTP"/*wfs:HTTP*/ );
  dcpTypeElement.appendChild( httpElement );
  
  //Prepare url
  QString hrefString = mConfigParser->wfsServiceUrl();
  if ( hrefString.isEmpty() )
  {
    hrefString = mConfigParser->serviceUrl();
  }
  if ( hrefString.isEmpty() )
  {
    hrefString = serviceUrl();
  }

  //only Get supported for the moment
  QDomElement getElement = doc.createElement( "Get"/*wfs:Get*/ );
  httpElement.appendChild( getElement );
  getElement.setAttribute( "onlineResource", hrefString );
  QDomElement getCapabilitiesDhcTypePostElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  getCapabilitiesDhcTypePostElement.firstChild().firstChild().toElement().setTagName( "Post" );
  getCapabilitiesElement.appendChild( getCapabilitiesDhcTypePostElement );

  //wfs:DescribeFeatureType
  QDomElement describeFeatureTypeElement = doc.createElement( "DescribeFeatureType"/*wfs:DescribeFeatureType*/ );
  requestElement.appendChild( describeFeatureTypeElement );
  QDomElement schemaDescriptionLanguageElement = doc.createElement( "SchemaDescriptionLanguage"/*wfs:SchemaDescriptionLanguage*/ );
  describeFeatureTypeElement.appendChild( schemaDescriptionLanguageElement );
  QDomElement xmlSchemaElement = doc.createElement( "XMLSCHEMA"/*wfs:XMLSCHEMA*/ );
  schemaDescriptionLanguageElement.appendChild( xmlSchemaElement );
  QDomElement describeFeatureTypeDhcTypeElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  describeFeatureTypeElement.appendChild( describeFeatureTypeDhcTypeElement );
  QDomElement describeFeatureTypeDhcTypePostElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  describeFeatureTypeDhcTypePostElement.firstChild().firstChild().toElement().setTagName( "Post" );
  describeFeatureTypeElement.appendChild( describeFeatureTypeDhcTypePostElement );

  //wfs:GetFeature
  QDomElement getFeatureElement = doc.createElement( "GetFeature"/*wfs:GetFeature*/ );
  requestElement.appendChild( getFeatureElement );
  QDomElement getFeatureFormatElement = doc.createElement( "ResultFormat" );/*wfs:ResultFormat*/
  getFeatureElement.appendChild( getFeatureFormatElement );
  QDomElement gmlFormatElement = doc.createElement( "GML2" );/*wfs:GML2*/
  getFeatureFormatElement.appendChild( gmlFormatElement );
  QDomElement gml3FormatElement = doc.createElement( "GML3" );/*wfs:GML3*/
  getFeatureFormatElement.appendChild( gml3FormatElement );
  QDomElement geojsonFormatElement = doc.createElement( "GeoJSON" );/*wfs:GeoJSON*/
  getFeatureFormatElement.appendChild( geojsonFormatElement );
  QDomElement getFeatureDhcTypeGetElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  getFeatureElement.appendChild( getFeatureDhcTypeGetElement );
  QDomElement getFeatureDhcTypePostElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  getFeatureDhcTypePostElement.firstChild().firstChild().toElement().setTagName( "Post" );
  getFeatureElement.appendChild( getFeatureDhcTypePostElement );

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
  spatialOperatorsElement.appendChild( doc.createElement( "ogc:BBOX"/*ogc:BBOX*/ ) );
  spatialOperatorsElement.appendChild( doc.createElement( "ogc:Disjoint"/*ogc:Disjoint*/ ) );
  spatialOperatorsElement.appendChild( doc.createElement( "ogc:Intersect"/*ogc:Intersects*/ ) );
  spatialOperatorsElement.appendChild( doc.createElement( "ogc:Touches"/*ogc:Touches*/ ) );
  spatialOperatorsElement.appendChild( doc.createElement( "ogc:Crosses"/*ogc:Crosses*/ ) );
  spatialOperatorsElement.appendChild( doc.createElement( "ogc:Contains"/*ogc:Contains*/ ) );
  spatialOperatorsElement.appendChild( doc.createElement( "ogc:Overlaps"/*ogc:Overlaps*/ ) );
  spatialOperatorsElement.appendChild( doc.createElement( "ogc:Within"/*ogc:Within*/ ) );
  QDomElement scalarCapabilitiesElement = doc.createElement( "ogc:Scalar_Capabilities"/*ogc:Scalar_Capabilities*/ );
  filterCapabilitiesElement.appendChild( scalarCapabilitiesElement );
  QDomElement comparisonOperatorsElement = doc.createElement( "ogc:Comparison_Operators"/*ogc:Comparison_Operators*/ );
  scalarCapabilitiesElement.appendChild( comparisonOperatorsElement );
  comparisonOperatorsElement.appendChild( doc.createElement( "ogc:Simple_Comparisons"/*ogc:Simple_Comparisons*/ ) );
  comparisonOperatorsElement.appendChild( doc.createElement( "ogc:Between"/*ogc:Between*/ ) );
  comparisonOperatorsElement.appendChild( doc.createElement( "ogc:Like"/*ogc:Like*/ ) );
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
  schemaElement.setAttribute( "elementFormDefault", "qualified" );
  schemaElement.setAttribute( "version", "1.0" );
  doc.appendChild( schemaElement );

  //xsd:import
  QDomElement importElement = doc.createElement( "import"/*xsd:import*/ );
  importElement.setAttribute( "namespace",  GML_NAMESPACE );
  importElement.setAttribute( "schemaLocation", "http://schemas.opengis.net/gml/2.1.2/feature.xsd" );
  schemaElement.appendChild( importElement );

  //defining typename
  QString typeName = "";

  QDomDocument queryDoc;
  QString errorMsg;
  if ( queryDoc.setContent( mParameterMap.value( "REQUEST_BODY" ), true, &errorMsg ) )
  {
    //read doc
    QDomElement queryDocElem = queryDoc.documentElement();
    QDomNodeList docChildNodes = queryDocElem.childNodes();
    if ( docChildNodes.size() )
    {
      for ( int i = 0; i < docChildNodes.size(); i++ )
      {
        QDomElement docChildElem = docChildNodes.at( i ).toElement();
        if ( docChildElem.tagName() == "TypeName" )
        {
          if ( typeName == "" )
            typeName = docChildElem.text();
          else
            typeName += "," + docChildElem.text();
        }
      }
    }
    mConfigParser->describeFeatureType( typeName, schemaElement, doc );
  }
  else
  {
    //read TYPENAME
    QMap<QString, QString>::const_iterator type_name_it = mParameterMap.find( "TYPENAME" );
    if ( type_name_it != mParameterMap.end() )
    {
      typeName = type_name_it.value();
    }
    mConfigParser->describeFeatureType( typeName, schemaElement, doc );
  }
  return doc;
}

int QgsWFSServer::getFeature( QgsRequestHandler& request, const QString& format )
{
  QgsDebugMsg( "Info format is:" + format );

  QStringList wfsLayersId = mConfigParser->wfsLayers();

  QList<QgsMapLayer*> layerList;
  QgsMapLayer* currentLayer = 0;

  mErrors = QStringList();
  mTypeNames = QStringList();

  long maxFeat = 0;
  long maxFeatures = -1;
  long featureCounter = 0;

  QDomDocument doc;
  QString errorMsg;
  if ( doc.setContent( mParameterMap.value( "REQUEST_BODY" ), true, &errorMsg ) )
  {
    QDomElement docElem = doc.documentElement();

    if ( docElem.hasAttribute( "maxFeatures" ) )
      maxFeatures = docElem.attribute( "maxFeatures" ).toLong();

    QDomNodeList queryNodes = docElem.elementsByTagName( "Query" );
    QDomElement queryElem;
    for ( int i = 0; i < queryNodes.size(); i++ )
    {
      queryElem = queryNodes.at( 0 ).toElement();
      mTypeName = queryElem.attribute( "typeName", "" );
      if ( mTypeName.contains( ":" ) )
      {
        mTypeName = mTypeName.section( ":", 1, 1 );
      }
      mTypeNames << mTypeName;
    }
    for ( int i = 0; i < queryNodes.size(); i++ )
    {
      queryElem = queryNodes.at( 0 ).toElement();
      mTypeName = queryElem.attribute( "typeName", "" );
      if ( mTypeName.contains( ":" ) )
      {
        mTypeName = mTypeName.section( ":", 1, 1 );
      }

      layerList = mConfigParser->mapLayerFromTypeName( mTypeName );
      if ( layerList.size() < 1 )
      {
        mErrors << QString( "The layer for the TypeName '%1' is not found" ).arg( mTypeName );
        continue;
      }

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
          mErrors << QString( "The layer's provider for the TypeName '%1' is not found" ).arg( mTypeName );
          continue;
        }

        QgsFeature feature;
        QgsAttributeMap featureAttributes;
        const QgsFields& fields = provider->fields();

        mWithGeom = true;
        QgsAttributeList attrIndexes = provider->attributeIndexes();

        QDomNodeList queryChildNodes = queryElem.childNodes();
        if ( queryChildNodes.size() )
        {
          QStringList::const_iterator alstIt;
          QList<int> idxList;
          QMap<QString, int> fieldMap = provider->fieldNameMap();
          QMap<QString, int>::const_iterator fieldIt;
          QString fieldName;
          QDomElement propertyElem;
          for ( int q = 0; q < queryChildNodes.size(); q++ )
          {
            QDomElement queryChildElem = queryChildNodes.at( q ).toElement();
            if ( queryChildElem.tagName() == "PropertyName" )
            {
              fieldName = queryChildElem.text();
              if ( fieldName.contains( ":" ) )
              {
                fieldName = fieldName.section( ":", 1, 1 );
              }
              fieldIt = fieldMap.find( fieldName );
              if ( fieldIt != fieldMap.end() )
              {
                idxList.append( fieldIt.value() );
              }
            }
          }
          if ( idxList.size() > 0 )
          {
            attrIndexes = idxList;
          }
        }

        //map extent
        QgsRectangle searchRect = layer->extent();
        searchRect.set( searchRect.xMinimum() - 0.000001
                        , searchRect.yMinimum() - 0.000001
                        , searchRect.xMaximum() + 0.000001
                        , searchRect.yMaximum() + 0.000001 );
        QgsCoordinateReferenceSystem layerCrs = layer->crs();

        if ( maxFeatures == -1 )
          maxFeat += layer->featureCount();

        QgsFeatureIterator fit = layer->getFeatures(
                                   QgsFeatureRequest()
                                   .setFilterRect( searchRect )
                                   .setFlags( QgsFeatureRequest::ExactIntersect | ( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) )
                                   .setSubsetOfAttributes( attrIndexes ) );

        long featCounter = 0;
        QDomNodeList filterNodes = queryElem.elementsByTagName( "Filter" );
        if ( filterNodes.size() > 0 )
        {
          QDomElement filterElem = filterNodes.at( 0 ).toElement();
          QDomNodeList fidNodes = filterElem.elementsByTagName( "FeatureId" );
          if ( fidNodes.size() > 0 )
          {
            QDomElement fidElem;
            QString fid = "";
            for ( int f = 0; f < fidNodes.size(); f++ )
            {
              fidElem = fidNodes.at( f ).toElement();
              fid = fidElem.attribute( "fid" );
              if ( fid.contains( "." ) )
              {
                if ( fid.section( ".", 0, 0 ) != mTypeName )
                  continue;
                fid = fid.section( ".", 1, 1 );
              }

              //Need to be test for propertyname
              layer->getFeatures( QgsFeatureRequest()
                                  .setFilterFid( fid.toInt() )
                                  .setFlags( QgsFeatureRequest::ExactIntersect | ( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) )
                                ).nextFeature( feature );

              if ( featureCounter == 0 )
                startGetFeature( request, format, layerCrs, &searchRect );

              sendGetFeature( request, format, &feature, featCounter, layerCrs, fields, layerExcludedAttributes );

              fid = "";
              ++featCounter;
              ++featureCounter;
            }
          }
          else if ( filterElem.firstChildElement().tagName() == "BBOX" )
          {
            QDomElement bboxElem = filterElem.firstChildElement();
            QDomElement childElem = bboxElem.firstChildElement();

            QgsFeatureRequest req;
            req.setSubsetOfAttributes( attrIndexes );
            req.setFlags( QgsFeatureRequest::ExactIntersect | ( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );

            while ( !childElem.isNull() )
            {
              if ( childElem.tagName() == "Box" )
              {
                req.setFilterRect( QgsOgcUtils::rectangleFromGMLBox( childElem ) );
              }
              else if ( childElem.tagName() != "PropertyName" )
              {
                QgsGeometry *geom = QgsOgcUtils::geometryFromGML( childElem );
                req.setFilterRect( geom->boundingBox() );
                delete geom;
              }
              childElem = childElem.nextSiblingElement();
            }

            QgsFeatureIterator fit = layer->getFeatures( req );
            while ( fit.nextFeature( feature ) && featureCounter < maxFeat )
            {
              if ( featureCounter == 0 )
                startGetFeature( request, format, layerCrs, &searchRect );

              sendGetFeature( request, format, &feature, featCounter, layerCrs, fields, layerExcludedAttributes );
              ++featCounter;
              ++featureCounter;
            }
          }
          else
          {
            QgsExpression *mFilter = QgsOgcUtils::expressionFromOgcFilter( filterElem );
            if ( mFilter->hasParserError() )
            {
              throw QgsMapServiceException( "RequestNotWellFormed", mFilter->parserErrorString() );
            }
            if ( mFilter )
            {
              while ( fit.nextFeature( feature ) && featureCounter < maxFeat )
              {
                QVariant res = mFilter->evaluate( &feature, fields );
                if ( mFilter->hasEvalError() )
                {
                  throw QgsMapServiceException( "RequestNotWellFormed", mFilter->evalErrorString() );
                }
                if ( res.toInt() != 0 )
                {
                  if ( featureCounter == 0 )
                    startGetFeature( request, format, layerCrs, &searchRect );

                  sendGetFeature( request, format, &feature, featureCounter, layerCrs, fields, layerExcludedAttributes );
                  ++featureCounter;
                  ++featCounter;
                }
              }
            }
          }
        }
        else
        {
          while ( fit.nextFeature( feature ) && featureCounter < maxFeat )
          {
            if ( featureCounter == 0 )
              startGetFeature( request, format, layerCrs, &searchRect );

            sendGetFeature( request, format, &feature, featCounter, layerCrs, fields, layerExcludedAttributes );
            ++featCounter;
            ++featureCounter;
          }
        }
      }
      else
      {
        mErrors << QString( "The layer for the TypeName '%1' is not a WFS layer" ).arg( mTypeName );
      }

    }

    if ( featureCounter == 0 )
      throw QgsMapServiceException( "RequestNotWellFormed", mErrors.join( ". " ) );
    else
      endGetFeature( request, format );
    return 0;
  }

  // Information about parameters
  // FILTER
  bool filterOk = false;
  QDomDocument filter;
  // EXP_FILTER
  bool expFilterOk = false;
  QString expFilter;
  // BBOX
  bool bboxOk = false;
  double minx = 0.0, miny = 0.0, maxx = 0.0, maxy = 0.0;

  //read FEATUREDID
  bool featureIdOk = false;
  QStringList featureIdList;
  QMap<QString, QString>::const_iterator feature_id_it = mParameterMap.find( "FEATUREID" );
  if ( feature_id_it != mParameterMap.end() )
  {
    featureIdOk = true;
    featureIdList = feature_id_it.value().split( "," );
    QStringList typeNameList;
    foreach ( const QString &fidStr, featureIdList )
    {
      // testing typename in the WFS featureID
      if ( !fidStr.contains( "." ) )
        throw QgsMapServiceException( "RequestNotWellFormed", "FEATUREID has to have  TYPENAME in the values" );

      QString typeName = fidStr.section( ".", 0, 0 );
      if ( !typeNameList.contains( typeName ) )
        typeNameList << typeName;
    }

    mTypeName = typeNameList.join( "," );
  }

  if ( !featureIdOk )
  {
    //read TYPENAME
    QMap<QString, QString>::const_iterator type_name_it = mParameterMap.find( "TYPENAME" );
    if ( type_name_it != mParameterMap.end() )
    {
      mTypeName = type_name_it.value();
    }
    else
    {
      throw QgsMapServiceException( "RequestNotWellFormed", "TYPENAME is MANDATORY" );
    }

    //read FILTER
    QMap<QString, QString>::const_iterator filterIt = mParameterMap.find( "FILTER" );
    if ( filterIt != mParameterMap.end() )
    {
      QString errorMsg;
      if ( !filter.setContent( filterIt.value(), true, &errorMsg ) )
      {
        throw QgsMapServiceException( "RequestNotWellFormed", QString( "error message: %1. The XML string was: %2" ).arg( errorMsg ).arg( filterIt.value() ) );
      }
      else
      {
        filterOk = true;
      }
    }

    //read EXP_FILTER
    if ( !filterOk )
    {
      QMap<QString, QString>::const_iterator expFilterIt = mParameterMap.find( "EXP_FILTER" );
      if ( expFilterIt != mParameterMap.end() )
      {
        expFilterOk = true;
        expFilter = expFilterIt.value();
      }
    }

    //read BBOX
    if ( !filterOk )
    {
      QMap<QString, QString>::const_iterator bbIt = mParameterMap.find( "BBOX" );
      if ( bbIt == mParameterMap.end() )
      {
        minx = 0; miny = 0; maxx = 0; maxy = 0;
      }
      else
      {
        bool conversionSuccess;
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
    }
  }

  //read MAXFEATURES
  QMap<QString, QString>::const_iterator mfIt = mParameterMap.find( "MAXFEATURES" );
  if ( mfIt != mParameterMap.end() )
  {
    QString mfString = mfIt.value();
    bool mfOk;
    maxFeatures = mfString.toLong( &mfOk, 10 );
    maxFeat = mfString.toLong( &mfOk, 10 );
  }

  //read PROPERTYNAME
  mWithGeom = true;
  mPropertyName = "*";
  QMap<QString, QString>::const_iterator pnIt = mParameterMap.find( "PROPERTYNAME" );
  if ( pnIt != mParameterMap.end() )
  {
    mPropertyName = pnIt.value();
  }

  mTypeNames = mTypeName.split( "," );
  foreach ( const QString &tnStr, mTypeNames )
  {
    mTypeName = tnStr;
    layerList = mConfigParser->mapLayerFromTypeName( tnStr );
    if ( layerList.size() < 1 )
    {
      mErrors << QString( "The layer for the TypeName '%1' is not found" ).arg( tnStr );
      continue;
    }

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
        mErrors << QString( "The layer's provider for the TypeName '%1' is not found" ).arg( tnStr );
        continue;
      }

      QgsFeature feature;
      QgsAttributeMap featureAttributes;
      const QgsFields& fields = provider->fields();

      //map extent
      QgsRectangle searchRect = layer->extent();

      QgsAttributeList attrIndexes = provider->attributeIndexes();
      if ( mPropertyName != "*" )
      {
        QStringList attrList = mPropertyName.split( "," );
        if ( attrList.size() > 0 )
        {
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
          }
          if ( idxList.size() > 0 )
          {
            attrIndexes = idxList;
          }
        }
      }

      if ( maxFeatures == -1 )
        maxFeat += layer->featureCount();

      if ( bboxOk )
        searchRect.set( minx, miny, maxx, maxy );
      else
        searchRect.set( searchRect.xMinimum() - 0.000001,
                        searchRect.yMinimum() - 0.000001,
                        searchRect.xMaximum() + 0.000001,
                        searchRect.yMaximum() + 0.000001 );
      QgsCoordinateReferenceSystem layerCrs = layer->crs();

      long featCounter = 0;
      if ( featureIdOk )
      {
        foreach ( const QString &fidStr, featureIdList )
        {
          if ( !fidStr.startsWith( tnStr ) )
            continue;
          //Need to be test for propertyname
          layer->getFeatures( QgsFeatureRequest()
                              .setFilterFid( fidStr.section( ".", 1, 1 ).toInt() )
                              .setFlags( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry )
                            ).nextFeature( feature );

          if ( featureCounter == 0 )
            startGetFeature( request, format, layerCrs, &searchRect );

          sendGetFeature( request, format, &feature, featCounter, layerCrs, fields, layerExcludedAttributes );
          ++featCounter;
          ++featureCounter;
        }
      }
      else if ( expFilterOk )
      {
        QgsFeatureRequest req;
        req.setSubsetOfAttributes( attrIndexes );
        if ( layer->wkbType() != QGis::WKBNoGeometry )
        {
          req.setFilterRect( searchRect )
          .setFlags( QgsFeatureRequest::ExactIntersect | ( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );
        }
        else
        {
          req.setFlags( QgsFeatureRequest::NoGeometry );
          mWithGeom = false;
        }
        QgsFeatureIterator fit = layer->getFeatures( req );
        QgsExpression *mFilter = new QgsExpression( expFilter );
        if ( mFilter->hasParserError() )
        {
          throw QgsMapServiceException( "RequestNotWellFormed", QString( "Expression filter error message: %1." ).arg( mFilter->parserErrorString() ) );
        }
        if ( mFilter )
        {
          while ( fit.nextFeature( feature ) && featureCounter < maxFeat )
          {
            QVariant res = mFilter->evaluate( &feature, fields );
            if ( mFilter->hasEvalError() )
            {
              throw QgsMapServiceException( "RequestNotWellFormed", QString( "Expression filter eval error message: %1." ).arg( mFilter->evalErrorString() ) );
            }
            if ( res.toInt() != 0 )
            {
              if ( featureCounter == 0 )
                startGetFeature( request, format, layerCrs, &searchRect );

              sendGetFeature( request, format, &feature, featCounter, layerCrs, fields, layerExcludedAttributes );
              ++featCounter;
              ++featureCounter;
            }
          }
          delete mFilter;
        }
      }
      else if ( filterOk )
      {
        QDomElement filterElem = filter.firstChildElement();
        QDomNodeList fidNodes = filterElem.elementsByTagName( "FeatureId" );
        if ( fidNodes.size() > 0 )
        {
          QDomElement fidElem;
          QString fid = "";
          for ( int f = 0; f < fidNodes.size(); f++ )
          {
            fidElem = fidNodes.at( f ).toElement();
            fid = fidElem.attribute( "fid" );
            if ( fid.contains( "." ) )
            {
              if ( fid.section( ".", 0, 0 ) != mTypeName )
                continue;
              fid = fid.section( ".", 1, 1 );
            }

            //Need to be test for propertyname
            layer->getFeatures( QgsFeatureRequest()
                                .setFilterFid( fid.toInt() )
                                .setFlags( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry )
                              ).nextFeature( feature );

            if ( featureCounter == 0 )
              startGetFeature( request, format, layerCrs, &searchRect );

            sendGetFeature( request, format, &feature, featCounter, layerCrs, fields, layerExcludedAttributes );

            fid = "";
            ++featCounter;
            ++featureCounter;
          }
        }
        else if ( filterElem.firstChildElement().tagName() == "BBOX" )
        {
          QDomElement bboxElem = filterElem.firstChildElement();
          QDomElement childElem = bboxElem.firstChildElement();

          QgsFeatureRequest req;
          req.setSubsetOfAttributes( attrIndexes );
          req.setFlags( QgsFeatureRequest::ExactIntersect | ( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );

          while ( !childElem.isNull() )
          {
            if ( childElem.tagName() == "Box" )
            {
              req.setFilterRect( QgsOgcUtils::rectangleFromGMLBox( childElem ) );
            }
            else if ( childElem.tagName() != "PropertyName" )
            {
              QgsGeometry* geom = QgsOgcUtils::geometryFromGML( childElem );
              req.setFilterRect( geom->boundingBox() );
              delete geom;
            }
            childElem = childElem.nextSiblingElement();
          }

          QgsFeatureIterator fit = layer->getFeatures( req );
          while ( fit.nextFeature( feature ) && featureCounter < maxFeat )
          {
            if ( featureCounter == 0 )
              startGetFeature( request, format, layerCrs, &searchRect );

            sendGetFeature( request, format, &feature, featCounter, layerCrs, fields, layerExcludedAttributes );
            ++featCounter;
            ++featureCounter;
          }
        }
        else
        {
          QgsExpression *mFilter = QgsOgcUtils::expressionFromOgcFilter( filterElem );
          if ( mFilter->hasParserError() )
          {
            throw QgsMapServiceException( "RequestNotWellFormed", QString( "OGC expression filter error message: %1." ).arg( mFilter->parserErrorString() ) );
          }
          if ( mFilter )
          {
            QgsFeatureRequest req;
            req.setSubsetOfAttributes( attrIndexes );
            if ( layer->wkbType() != QGis::WKBNoGeometry )
            {
              req.setFilterRect( searchRect )
              .setFlags( QgsFeatureRequest::ExactIntersect | ( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );
            }
            else
            {
              req.setFlags( QgsFeatureRequest::NoGeometry );
              mWithGeom = false;
            }
            QgsFeatureIterator fit = layer->getFeatures( req );
            while ( fit.nextFeature( feature ) && featureCounter < maxFeat )
            {
              QVariant res = mFilter->evaluate( &feature, fields );
              if ( mFilter->hasEvalError() )
              {
                throw QgsMapServiceException( "RequestNotWellFormed", QString( "OGC expression filter eval error message: %1." ).arg( mFilter->evalErrorString() ) );
              }
              if ( res.toInt() != 0 )
              {
                if ( featureCounter == 0 )
                  startGetFeature( request, format, layerCrs, &searchRect );

                sendGetFeature( request, format, &feature, featureCounter, layerCrs, fields, layerExcludedAttributes );
                ++featureCounter;
                ++featCounter;
              }
            }
          }
          delete mFilter;
        }
      }
      else
      {
        QgsFeatureRequest req;
        req.setSubsetOfAttributes( attrIndexes );
        if ( layer->wkbType() != QGis::WKBNoGeometry )
        {
          req.setFilterRect( searchRect )
          .setFlags( QgsFeatureRequest::ExactIntersect | ( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );
        }
        else
        {
          req.setFlags( QgsFeatureRequest::NoGeometry );
          mWithGeom = false;
        }
        QgsFeatureIterator fit = layer->getFeatures( req );
        while ( fit.nextFeature( feature ) && featureCounter < maxFeat )
        {
          mErrors << QString( "The feature %2 of layer for the TypeName '%1'" ).arg( tnStr ).arg( featureCounter );
          if ( featureCounter == 0 )
            startGetFeature( request, format, layerCrs, &searchRect );

          sendGetFeature( request, format, &feature, featCounter, layerCrs, fields, layerExcludedAttributes );
          ++featCounter;
          ++featureCounter;
        }
      }

    }
    else
    {
      mErrors << QString( "The layer for the TypeName '%1' is not a WFS layer" ).arg( tnStr );
    }

  }
  if ( featureCounter == 0 )
    throw QgsMapServiceException( "RequestNotWellFormed", QString( "No feature found error messages: %1." ).arg( mErrors.join( ". " ) ) );
  else
    endGetFeature( request, format );

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
    QString hrefString = mConfigParser->wfsServiceUrl();
    if ( hrefString.isEmpty() )
    {
      hrefString = mConfigParser->serviceUrl();
    }
    if ( hrefString.isEmpty() )
    {
      hrefString = serviceUrl();
    }
    QUrl mapUrl( hrefString );
    mapUrl.addQueryItem( "SERVICE", "WFS" );
    mapUrl.addQueryItem( "VERSION", "1.0.0" );

    QList<QPair<QString, QString> > queryItems = mapUrl.queryItems();
    QList<QPair<QString, QString> >::const_iterator queryIt = queryItems.constBegin();
    for ( ; queryIt != queryItems.constEnd(); ++queryIt )
    {
      if ( queryIt->first.compare( "REQUEST", Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
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
      else if ( queryIt->first.compare( "TYPENAME", Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( "FILTER", Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( "EXP_FILTER", Qt::CaseInsensitive ) == 0 )
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
    mapUrl.addQueryItem( "REQUEST", "DescribeFeatureType" );
    mapUrl.addQueryItem( "TYPENAME", mTypeNames.join( "," ) );
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
    if ( format == "GML3" )
    {
      QDomElement envElem = QgsOgcUtils::rectangleToGMLEnvelope( rect, doc );
      if ( !envElem.isNull() )
      {
        if ( crs.isValid() )
        {
          envElem.setAttribute( "srsName", crs.authid() );
        }
        bbElem.appendChild( envElem );
        doc.appendChild( bbElem );
      }
    }
    else
    {
      QDomElement boxElem = QgsOgcUtils::rectangleToGMLBox( rect, doc );
      if ( !boxElem.isNull() )
      {
        if ( crs.isValid() )
        {
          boxElem.setAttribute( "srsName", crs.authid() );
        }
        bbElem.appendChild( boxElem );
        doc.appendChild( bbElem );
      }
    }
    result = doc.toByteArray();
    request.sendGetFeatureResponse( &result );
  }
  fcString = "";
}

void QgsWFSServer::sendGetFeature( QgsRequestHandler& request, const QString& format, QgsFeature* feat, int featIdx, QgsCoordinateReferenceSystem& crs, QgsFields fields, QSet<QString> excludedAttributes ) /*const*/
{
  if ( !feat->isValid() )
    return;

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
    QDomElement featureElement;
    if ( format == "GML3" )
    {
      featureElement = createFeatureGML3( feat, gmlDoc, crs, fields, excludedAttributes );
      gmlDoc.appendChild( featureElement );
    }
    else
    {
      featureElement = createFeatureGML2( feat, gmlDoc, crs, fields, excludedAttributes );
      gmlDoc.appendChild( featureElement );
    }

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

    if ( typeName.contains( ":" ) )
      typeName = typeName.section( ":", 1, 1 );

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

  // Store the created feature id for WFS
  QStringList insertResults;
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
    mTypeName = typeNameElem.tagName();

    layerList = mConfigParser->mapLayerFromTypeName( mTypeName );
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
          const QgsFields& fields = provider->fields();
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
              const QgsField& field = fields[fieldMapIt.value()];
              if ( field.type() == 2 )
                layer->changeAttributeValue( *fidIt, fieldMapIt.value(), it.value().toInt( &conversionSuccess ) );
              else if ( field.type() == 6 )
                layer->changeAttributeValue( *fidIt, fieldMapIt.value(), it.value().toDouble( &conversionSuccess ) );
              else
                layer->changeAttributeValue( *fidIt, fieldMapIt.value(), it.value() );
            }

            if ( !geometryElem.isNull() )
            {
              if ( !layer->changeGeometry( *fidIt, QgsOgcUtils::geometryFromGML( geometryElem ) ) )
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
        const QgsFields& fields = provider->fields();
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
                  const QgsField& field = fields[fieldMapIt.value()];
                  QString attrValue = currentAttributeElement.text();
                  int attrType = field.type();
                  if ( attrType == 2 )
                    f->setAttribute( fieldMapIt.value(), attrValue.toInt() );
                  else if ( attrType == 6 )
                    f->setAttribute( fieldMapIt.value(), attrValue.toDouble() );
                  else
                    f->setAttribute( fieldMapIt.value(), attrValue );
                }
                else //a geometry attribute
                {
                  f->setGeometry( QgsOgcUtils::geometryFromGML( currentAttributeElement ) );
                }
              }
              currentAttributeChild = currentAttributeChild.nextSibling();
            }
            // Add the feature to th layer
            // and store it to put it's Feature Id in the response
            inFeatList.append( *f );
          }
        }
      }
      // Commit the changes of the insert elements
      if ( !provider->addFeatures( inFeatList ) )
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
        QStringList mesErrors;
        mesErrors << QString( "ERROR: %n feature(s) not added." ).arg( inFeatList.size() );
        if ( provider->hasErrors() )
        {
          mesErrors << "\n  Provider errors:" << provider->errors();
          provider->clearErrors();
        }
        mesElem.appendChild( doc.createTextNode( layer->commitErrors().join( "\n  " ) ) );
        trElem.appendChild( mesElem );

        return resp;
      }
      // Get the Feature Ids of the inserted feature
      for ( int j = 0; j < inFeatList.size(); j++ )
      {
        insertResults << mTypeName + "." + QString::number( inFeatList[j].id() );
      }
    }
  }

  // Put the Feature Ids of the inserted feature
  if ( insertResults.size() > 0 )
  {
    foreach ( const QString &fidStr, insertResults )
    {
      QDomElement irElem = doc.createElement( "InsertResult" );
      QDomElement fiElem = doc.createElement( "ogc:FeatureId" );
      fiElem.setAttribute( "fid", fidStr );
      irElem.appendChild( fiElem );
      respElem.appendChild( irElem );
    }
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

QgsFeatureIds QgsWFSServer::getFeatureIdsFromFilter( QDomElement filterElem, QgsVectorLayer* layer )
{
  QgsFeatureIds fids;

  QgsVectorDataProvider *provider = layer->dataProvider();
  QDomNodeList fidNodes = filterElem.elementsByTagName( "FeatureId" );

  if ( fidNodes.size() != 0 )
  {
    QDomElement fidElem;
    QString fid;
    bool conversionSuccess;
    for ( int i = 0; i < fidNodes.size(); ++i )
    {
      fidElem = fidNodes.at( i ).toElement();
      fid = fidElem.attribute( "fid" );
      if ( fid.contains( "." ) )
        fid = fid.section( ".", 1, 1 );
      fids.insert( fid.toInt( &conversionSuccess ) );
    }
  }
  else
  {
    QgsExpression *mFilter = QgsOgcUtils::expressionFromOgcFilter( filterElem );
    if ( mFilter->hasParserError() )
    {
      throw QgsMapServiceException( "RequestNotWellFormed", mFilter->parserErrorString() );
    }
    if ( mFilter )
    {
      QgsFeature feature;
      const QgsFields& fields = provider->fields();
      QgsFeatureIterator fit = layer->getFeatures();
      while ( fit.nextFeature( feature ) )
      {
        QVariant res = mFilter->evaluate( &feature, fields );
        if ( mFilter->hasEvalError() )
        {
          throw QgsMapServiceException( "RequestNotWellFormed", mFilter->evalErrorString() );
        }
        if ( res.toInt() != 0 )
        {
          fids.insert( feature.id() );
        }
      }
    }
  }

  return fids;
}

QString QgsWFSServer::createFeatureGeoJSON( QgsFeature* feat, QgsCoordinateReferenceSystem &, QgsFields fields, QSet<QString> excludedAttributes ) /*const*/
{
  QString fStr = "{\"type\": \"Feature\",\n";

  fStr += "   \"id\": ";
  fStr += "\"" + mTypeName + "." + QString::number( feat->id() ) + "\"";
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
  QgsAttributes featureAttributes = feat->attributes();
  int attributeCounter = 0;
  for ( int i = 0; i < featureAttributes.count(); ++i )
  {
    QString attributeName = fields[i].name();
    //skip attribute if it is excluded from WFS publication
    if ( excludedAttributes.contains( attributeName ) )
    {
      continue;
    }
    QVariant val = featureAttributes[i];

    if ( attributeCounter == 0 )
      fStr += "    \"";
    else
      fStr += "   ,\"";
    fStr += attributeName;
    fStr += "\": ";
    if ( val.type() == 6 || val.type() == 2 )
    {
      fStr +=  val.toString();
    }
    else
    {
      fStr += "\"";
      fStr +=  val.toString().replace( QString( "\"" ), QString( "\\\"" ) );
      fStr += "\"";
    }
    fStr += "\n";
    ++attributeCounter;
  }

  fStr += "   }\n";

  fStr += "  }";

  return fStr;
}

QDomElement QgsWFSServer::createFeatureGML2( QgsFeature* feat, QDomDocument& doc, QgsCoordinateReferenceSystem& crs, QgsFields fields, QSet<QString> excludedAttributes ) /*const*/
{
  //gml:FeatureMember
  QDomElement featureElement = doc.createElement( "gml:featureMember"/*wfs:FeatureMember*/ );

  //qgs:%TYPENAME%
  QDomElement typeNameElement = doc.createElement( "qgs:" + mTypeName /*qgs:%TYPENAME%*/ );
  typeNameElement.setAttribute( "fid", mTypeName + "." + QString::number( feat->id() ) );
  featureElement.appendChild( typeNameElement );

  if ( mWithGeom )
  {
    //add geometry column (as gml)
    QgsGeometry* geom = feat->geometry();

    QDomElement geomElem = doc.createElement( "qgs:geometry" );
    QDomElement gmlElem = QgsOgcUtils::geometryToGML( geom, doc );
    if ( !gmlElem.isNull() )
    {
      QgsRectangle box = geom->boundingBox();
      QDomElement bbElem = doc.createElement( "gml:boundedBy" );
      QDomElement boxElem = QgsOgcUtils::rectangleToGMLBox( &box, doc );

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
  QgsAttributes featureAttributes = feat->attributes();
  for ( int i = 0; i < featureAttributes.count(); ++i )
  {

    QString attributeName = fields[i].name();
    //skip attribute if is explicitly excluded from WFS publication
    if ( excludedAttributes.contains( attributeName ) )
    {
      continue;
    }

    QDomElement fieldElem = doc.createElement( "qgs:" + attributeName.replace( QString( " " ), QString( "_" ) ) );
    QDomText fieldText = doc.createTextNode( featureAttributes[i].toString() );
    fieldElem.appendChild( fieldText );
    typeNameElement.appendChild( fieldElem );
  }

  return featureElement;
}

QDomElement QgsWFSServer::createFeatureGML3( QgsFeature* feat, QDomDocument& doc, QgsCoordinateReferenceSystem& crs, QgsFields fields, QSet<QString> excludedAttributes ) /*const*/
{
  //gml:FeatureMember
  QDomElement featureElement = doc.createElement( "gml:featureMember"/*wfs:FeatureMember*/ );

  //qgs:%TYPENAME%
  QDomElement typeNameElement = doc.createElement( "qgs:" + mTypeName /*qgs:%TYPENAME%*/ );
  typeNameElement.setAttribute( "gml:id", mTypeName + "." + QString::number( feat->id() ) );
  featureElement.appendChild( typeNameElement );

  if ( mWithGeom )
  {
    //add geometry column (as gml)
    QgsGeometry* geom = feat->geometry();

    QDomElement geomElem = doc.createElement( "qgs:geometry" );
    QDomElement gmlElem = QgsOgcUtils::geometryToGML( geom, doc, "GML3" );
    if ( !gmlElem.isNull() )
    {
      QgsRectangle box = geom->boundingBox();
      QDomElement bbElem = doc.createElement( "gml:boundedBy" );
      QDomElement boxElem = QgsOgcUtils::rectangleToGMLEnvelope( &box, doc );

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
  QgsAttributes featureAttributes = feat->attributes();
  for ( int i = 0; i < featureAttributes.count(); ++i )
  {

    QString attributeName = fields[i].name();
    //skip attribute if is explicitly excluded from WFS publication
    if ( excludedAttributes.contains( attributeName ) )
    {
      continue;
    }

    QDomElement fieldElem = doc.createElement( "qgs:" + attributeName.replace( QString( " " ), QString( "_" ) ) );
    QDomText fieldText = doc.createTextNode( featureAttributes[i].toString() );
    fieldElem.appendChild( fieldText );
    typeNameElement.appendChild( fieldElem );
  }

  return featureElement;
}

QString QgsWFSServer::serviceUrl() const
{
  QUrl mapUrl( getenv( "REQUEST_URI" ) );
  mapUrl.setHost( getenv( "SERVER_NAME" ) );

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
  return mapUrl.toString();
}
