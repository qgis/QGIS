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
#include "qgsfields.h"
#include "qgsexpression.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsmaplayer.h"
#include "qgsmaptopixel.h"
#include "qgsmessagelog.h"
#include "qgspallabeling.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsscalecalculator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsmapserviceexception.h"
#include "qgssymbol.h"
#include "qgsrequesthandler.h"
#include "qgsogcutils.h"
#include "qgsaccesscontrol.h"
#include "qgsjsonutils.h"

#include <QImage>
#include <QPainter>
#include <QStringList>
#include <QTextStream>
#include <QDir>
#include <QSharedPointer>

//for printing
#include "qgscomposition.h"
#include <QBuffer>
#include <QPrinter>
#include <QSvgGenerator>
#include <QUrl>
#include <QPaintEngine>

#ifndef Q_OS_WIN
#include <netinet/in.h>
#else
#include <winsock.h>
#endif

static const QString WFS_NAMESPACE = QStringLiteral( "http://www.opengis.net/wfs" );
static const QString GML_NAMESPACE = QStringLiteral( "http://www.opengis.net/gml" );
static const QString OGC_NAMESPACE = QStringLiteral( "http://www.opengis.net/ogc" );
static const QString QGS_NAMESPACE = QStringLiteral( "http://www.qgis.org/gml" );

QgsWfsServer::QgsWfsServer(
  const QString& configFilePath
  , const QgsServerSettings& settings
  , QMap<QString, QString> &parameters
  , QgsWfsProjectParser* cp
  , QgsRequestHandler* rh
  , QgsAccessControl* accessControl
)
    : QgsOWSServer(
      configFilePath
      , settings
      , parameters
      , rh
      , accessControl
    )
    , mWithGeom( true )
    , mConfigParser( cp )
{
}

QgsWfsServer::QgsWfsServer()
    : QgsOWSServer(
      QString()
      , QgsServerSettings()
      , QMap<QString, QString>()
      , nullptr
      , nullptr
    )
    , mWithGeom( true )
    , mConfigParser( nullptr )
{
}


void QgsWfsServer::executeRequest()
{
  if ( !mConfigParser && !mRequestHandler )
  {
    return;
  }

  //request type
  QString request = mParameters.value( QStringLiteral( "REQUEST" ) );
  if ( request.isEmpty() )
  {
    //do some error handling
    QgsMessageLog::logMessage( QStringLiteral( "unable to find 'REQUEST' parameter, exiting..." ) );
    mRequestHandler->setServiceException( QgsMapServiceException( QStringLiteral( "OperationNotSupported" ), QStringLiteral( "Please check the value of the REQUEST parameter" ) ) );
    return;
  }

  if ( request.compare( QLatin1String( "GetCapabilities" ), Qt::CaseInsensitive ) == 0 )
  {
    QDomDocument capabilitiesDocument;
    try
    {
      capabilitiesDocument = getCapabilities();
    }
    catch ( QgsMapServiceException& ex )
    {
      mRequestHandler->setServiceException( ex );
      return;
    }
    QgsMessageLog::logMessage( QStringLiteral( "Setting GetCapabilities response" ) );
    mRequestHandler->setGetCapabilitiesResponse( capabilitiesDocument );
    return;
  }
  else if ( request.compare( QLatin1String( "DescribeFeatureType" ), Qt::CaseInsensitive ) == 0 )
  {
    QDomDocument describeDocument;
    try
    {
      describeDocument = describeFeatureType();
    }
    catch ( QgsMapServiceException& ex )
    {
      mRequestHandler->setServiceException( ex );
      return;
    }
    QgsMessageLog::logMessage( QStringLiteral( "Setting GetCapabilities response" ) );
    mRequestHandler->setGetCapabilitiesResponse( describeDocument );
    return;
  }
  else if ( request.compare( QLatin1String( "GetFeature" ), Qt::CaseInsensitive ) == 0 )
  {
    //output format for GetFeature
    QString outputFormat = mParameters.value( QStringLiteral( "OUTPUTFORMAT" ) );
    try
    {
      getFeature( *mRequestHandler, outputFormat );
    }
    catch ( QgsMapServiceException& ex )
    {
      mRequestHandler->setServiceException( ex );
    }

    return;
  }
  else if ( request.compare( QLatin1String( "Transaction" ), Qt::CaseInsensitive ) == 0 )
  {
    QDomDocument transactionDocument;
    try
    {
      transactionDocument = transaction( mParameters.value( QStringLiteral( "REQUEST_BODY" ) ) );
    }
    catch ( QgsMapServiceException& ex )
    {
      mRequestHandler->setServiceException( ex );
      return;
    }
    QgsMessageLog::logMessage( QStringLiteral( "Setting Transaction response" ) );
    mRequestHandler->setGetCapabilitiesResponse( transactionDocument );
    return;
  }
}

QDomDocument QgsWfsServer::getCapabilities()
{
  QgsMessageLog::logMessage( QStringLiteral( "Entering." ) );
  QDomDocument doc;

  //wfs:WFS_Capabilities element
  QDomElement wfsCapabilitiesElement = doc.createElement( QStringLiteral( "WFS_Capabilities" )/*wms:WFS_Capabilities*/ );
  wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns" ), WFS_NAMESPACE );
  wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
  wfsCapabilitiesElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), WFS_NAMESPACE + " http://schemas.opengis.net/wfs/1.0.0/WFS-capabilities.xsd" );
  wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:ogc" ), OGC_NAMESPACE );
  wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:gml" ), GML_NAMESPACE );
  wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:ows" ), QStringLiteral( "http://www.opengis.net/ows" ) );
  wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
  wfsCapabilitiesElement.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0.0" ) );
  wfsCapabilitiesElement.setAttribute( QStringLiteral( "updateSequence" ), QStringLiteral( "0" ) );
  doc.appendChild( wfsCapabilitiesElement );

  if ( mConfigParser )
  {
    mConfigParser->serviceCapabilities( wfsCapabilitiesElement, doc );
  }

  //wfs:Capability element
  QDomElement capabilityElement = doc.createElement( QStringLiteral( "Capability" )/*wfs:Capability*/ );
  wfsCapabilitiesElement.appendChild( capabilityElement );

  //wfs:Request element
  QDomElement requestElement = doc.createElement( QStringLiteral( "Request" )/*wfs:Request*/ );
  capabilityElement.appendChild( requestElement );
  //wfs:GetCapabilities
  QDomElement getCapabilitiesElement = doc.createElement( QStringLiteral( "GetCapabilities" )/*wfs:GetCapabilities*/ );
  requestElement.appendChild( getCapabilitiesElement );

  QDomElement dcpTypeElement = doc.createElement( QStringLiteral( "DCPType" )/*wfs:DCPType*/ );
  getCapabilitiesElement.appendChild( dcpTypeElement );
  QDomElement httpElement = doc.createElement( QStringLiteral( "HTTP" )/*wfs:HTTP*/ );
  dcpTypeElement.appendChild( httpElement );

  //Prepare url
  QString hrefString;
  if ( mConfigParser )
  {
    hrefString = mConfigParser->wfsServiceUrl();
    if ( hrefString.isEmpty() )
    {
      hrefString = mConfigParser->serviceUrl();
    }
  }
  if ( hrefString.isEmpty() )
  {
    hrefString = serviceUrl();
  }

  //only Get supported for the moment
  QDomElement getElement = doc.createElement( QStringLiteral( "Get" )/*wfs:Get*/ );
  httpElement.appendChild( getElement );
  getElement.setAttribute( QStringLiteral( "onlineResource" ), hrefString );
  QDomElement getCapabilitiesDhcTypePostElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  getCapabilitiesDhcTypePostElement.firstChild().firstChild().toElement().setTagName( QStringLiteral( "Post" ) );
  getCapabilitiesElement.appendChild( getCapabilitiesDhcTypePostElement );

  //wfs:DescribeFeatureType
  QDomElement describeFeatureTypeElement = doc.createElement( QStringLiteral( "DescribeFeatureType" )/*wfs:DescribeFeatureType*/ );
  requestElement.appendChild( describeFeatureTypeElement );
  QDomElement schemaDescriptionLanguageElement = doc.createElement( QStringLiteral( "SchemaDescriptionLanguage" )/*wfs:SchemaDescriptionLanguage*/ );
  describeFeatureTypeElement.appendChild( schemaDescriptionLanguageElement );
  QDomElement xmlSchemaElement = doc.createElement( QStringLiteral( "XMLSCHEMA" )/*wfs:XMLSCHEMA*/ );
  schemaDescriptionLanguageElement.appendChild( xmlSchemaElement );
  QDomElement describeFeatureTypeDhcTypeElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  describeFeatureTypeElement.appendChild( describeFeatureTypeDhcTypeElement );
  QDomElement describeFeatureTypeDhcTypePostElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  describeFeatureTypeDhcTypePostElement.firstChild().firstChild().toElement().setTagName( QStringLiteral( "Post" ) );
  describeFeatureTypeElement.appendChild( describeFeatureTypeDhcTypePostElement );

  //wfs:GetFeature
  QDomElement getFeatureElement = doc.createElement( QStringLiteral( "GetFeature" )/*wfs:GetFeature*/ );
  requestElement.appendChild( getFeatureElement );
  QDomElement getFeatureFormatElement = doc.createElement( QStringLiteral( "ResultFormat" ) );/*wfs:ResultFormat*/
  getFeatureElement.appendChild( getFeatureFormatElement );
  QDomElement gmlFormatElement = doc.createElement( QStringLiteral( "GML2" ) );/*wfs:GML2*/
  getFeatureFormatElement.appendChild( gmlFormatElement );
  QDomElement gml3FormatElement = doc.createElement( QStringLiteral( "GML3" ) );/*wfs:GML3*/
  getFeatureFormatElement.appendChild( gml3FormatElement );
  QDomElement geojsonFormatElement = doc.createElement( QStringLiteral( "GeoJSON" ) );/*wfs:GeoJSON*/
  getFeatureFormatElement.appendChild( geojsonFormatElement );
  QDomElement getFeatureDhcTypeGetElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  getFeatureElement.appendChild( getFeatureDhcTypeGetElement );
  QDomElement getFeatureDhcTypePostElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  getFeatureDhcTypePostElement.firstChild().firstChild().toElement().setTagName( QStringLiteral( "Post" ) );
  getFeatureElement.appendChild( getFeatureDhcTypePostElement );

  //wfs:Transaction
  QDomElement transactionElement = doc.createElement( QStringLiteral( "Transaction" )/*wfs:Transaction*/ );
  requestElement.appendChild( transactionElement );
  QDomElement transactionDhcTypeElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  transactionDhcTypeElement.firstChild().firstChild().toElement().setTagName( QStringLiteral( "Post" ) );
  transactionElement.appendChild( transactionDhcTypeElement );

  //wfs:FeatureTypeList element
  QDomElement featureTypeListElement = doc.createElement( QStringLiteral( "FeatureTypeList" )/*wfs:FeatureTypeList*/ );
  wfsCapabilitiesElement.appendChild( featureTypeListElement );
  //wfs:Operations element
  QDomElement operationsElement = doc.createElement( QStringLiteral( "Operations" )/*wfs:Operations*/ );
  featureTypeListElement.appendChild( operationsElement );
  //wfs:Query element
  QDomElement queryElement = doc.createElement( QStringLiteral( "Query" )/*wfs:Query*/ );
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
  QDomElement filterCapabilitiesElement = doc.createElement( QStringLiteral( "ogc:Filter_Capabilities" )/*ogc:Filter_Capabilities*/ );
  wfsCapabilitiesElement.appendChild( filterCapabilitiesElement );
  QDomElement spatialCapabilitiesElement = doc.createElement( QStringLiteral( "ogc:Spatial_Capabilities" )/*ogc:Spatial_Capabilities*/ );
  filterCapabilitiesElement.appendChild( spatialCapabilitiesElement );
  QDomElement spatialOperatorsElement = doc.createElement( QStringLiteral( "ogc:Spatial_Operators" )/*ogc:Spatial_Operators*/ );
  spatialCapabilitiesElement.appendChild( spatialOperatorsElement );
  spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:BBOX" )/*ogc:BBOX*/ ) );
  spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Disjoint" )/*ogc:Disjoint*/ ) );
  spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Intersect" )/*ogc:Intersects*/ ) );
  spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Touches" )/*ogc:Touches*/ ) );
  spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Crosses" )/*ogc:Crosses*/ ) );
  spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Contains" )/*ogc:Contains*/ ) );
  spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Overlaps" )/*ogc:Overlaps*/ ) );
  spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Within" )/*ogc:Within*/ ) );
  QDomElement scalarCapabilitiesElement = doc.createElement( QStringLiteral( "ogc:Scalar_Capabilities" )/*ogc:Scalar_Capabilities*/ );
  filterCapabilitiesElement.appendChild( scalarCapabilitiesElement );
  QDomElement comparisonOperatorsElement = doc.createElement( QStringLiteral( "ogc:Comparison_Operators" )/*ogc:Comparison_Operators*/ );
  scalarCapabilitiesElement.appendChild( comparisonOperatorsElement );
  comparisonOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Simple_Comparisons" )/*ogc:Simple_Comparisons*/ ) );
  comparisonOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Between" )/*ogc:Between*/ ) );
  comparisonOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Like" )/*ogc:Like*/ ) );

  return doc;
}

QDomDocument QgsWfsServer::describeFeatureType()
{
  QgsMessageLog::logMessage( QStringLiteral( "Entering." ) );
  QDomDocument doc;

  //xsd:schema
  QDomElement schemaElement = doc.createElement( QStringLiteral( "schema" )/*xsd:schema*/ );
  schemaElement.setAttribute( QStringLiteral( "xmlns" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema" ) );
  schemaElement.setAttribute( QStringLiteral( "xmlns:xsd" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema" ) );
  schemaElement.setAttribute( QStringLiteral( "xmlns:ogc" ), OGC_NAMESPACE );
  schemaElement.setAttribute( QStringLiteral( "xmlns:gml" ), GML_NAMESPACE );
  schemaElement.setAttribute( QStringLiteral( "xmlns:qgs" ), QGS_NAMESPACE );
  schemaElement.setAttribute( QStringLiteral( "targetNamespace" ), QGS_NAMESPACE );
  schemaElement.setAttribute( QStringLiteral( "elementFormDefault" ), QStringLiteral( "qualified" ) );
  schemaElement.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  doc.appendChild( schemaElement );

  //xsd:import
  QDomElement importElement = doc.createElement( QStringLiteral( "import" )/*xsd:import*/ );
  importElement.setAttribute( QStringLiteral( "namespace" ),  GML_NAMESPACE );
  importElement.setAttribute( QStringLiteral( "schemaLocation" ), QStringLiteral( "http://schemas.opengis.net/gml/2.1.2/feature.xsd" ) );
  schemaElement.appendChild( importElement );

  //defining typename
  QString typeName = QLatin1String( "" );

  QDomDocument queryDoc;
  QString errorMsg;
  if ( queryDoc.setContent( mParameters.value( QStringLiteral( "REQUEST_BODY" ) ), true, &errorMsg ) )
  {
    //read doc
    QDomElement queryDocElem = queryDoc.documentElement();
    QDomNodeList docChildNodes = queryDocElem.childNodes();
    if ( docChildNodes.size() )
    {
      for ( int i = 0; i < docChildNodes.size(); i++ )
      {
        QDomElement docChildElem = docChildNodes.at( i ).toElement();
        if ( docChildElem.tagName() == QLatin1String( "TypeName" ) )
        {
          if ( typeName == QLatin1String( "" ) )
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
    QMap<QString, QString>::const_iterator type_name_it = mParameters.constFind( QStringLiteral( "TYPENAME" ) );
    if ( type_name_it != mParameters.constEnd() )
    {
      typeName = type_name_it.value();
    }
    mConfigParser->describeFeatureType( typeName, schemaElement, doc );
  }

  return doc;
}

int QgsWfsServer::getFeature( QgsRequestHandler& request, const QString& format )
{
  QgsMessageLog::logMessage( "Info format is:" + format );

  QStringList wfsLayersId = mConfigParser->wfsLayers();

  QList<QgsMapLayer*> layerList;
  QgsMapLayer* currentLayer = nullptr;
  QgsCoordinateReferenceSystem layerCrs;
  QgsRectangle searchRect( 0, 0, 0, 0 );

  mErrors = QStringList();
  mTypeNames = QStringList();

  long maxFeatures = 0;
  bool hasFeatureLimit = false;
  long startIndex = 0;
  long featureCounter = 0;
  int layerPrec = 8;
  long featCounter = 0;

  QgsExpressionContext expressionContext;
  expressionContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope( QgsProject::instance() );

  QDomDocument doc;
  QString errorMsg;

  //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
  //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
  QScopedPointer< QgsOWSServerFilterRestorer > filterRestorer( new QgsOWSServerFilterRestorer() );

  if ( doc.setContent( mParameters.value( QStringLiteral( "REQUEST_BODY" ) ), true, &errorMsg ) )
  {
    QDomElement docElem = doc.documentElement();
    if ( docElem.hasAttribute( QStringLiteral( "maxFeatures" ) ) )
    {
      hasFeatureLimit = true;
      maxFeatures = docElem.attribute( QStringLiteral( "maxFeatures" ) ).toLong();
    }
    if ( docElem.hasAttribute( QStringLiteral( "startIndex" ) ) )
    {
      startIndex = docElem.attribute( QStringLiteral( "startIndex" ) ).toLong();
    }

    QDomNodeList queryNodes = docElem.elementsByTagName( QStringLiteral( "Query" ) );
    QDomElement queryElem;
    for ( int i = 0; i < queryNodes.size(); i++ )
    {
      queryElem = queryNodes.at( 0 ).toElement();
      mTypeName = queryElem.attribute( QStringLiteral( "typeName" ), QLatin1String( "" ) );
      if ( mTypeName.contains( QLatin1String( ":" ) ) )
      {
        mTypeName = mTypeName.section( QStringLiteral( ":" ), 1, 1 );
      }
      mTypeNames << mTypeName;
    }
    for ( int i = 0; i < queryNodes.size(); i++ )
    {
      queryElem = queryNodes.at( 0 ).toElement();
      mTypeName = queryElem.attribute( QStringLiteral( "typeName" ), QLatin1String( "" ) );
      if ( mTypeName.contains( QLatin1String( ":" ) ) )
      {
        mTypeName = mTypeName.section( QStringLiteral( ":" ), 1, 1 );
      }

      layerList = mConfigParser->mapLayerFromTypeName( mTypeName );
      if ( layerList.size() < 1 )
      {
        mErrors << QStringLiteral( "The layer for the TypeName '%1' is not found" ).arg( mTypeName );
        continue;
      }

      currentLayer = layerList.at( 0 );
      QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( currentLayer );
      if ( layer && wfsLayersId.contains( layer->id() ) )
      {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
        if ( !mAccessControl->layerReadPermission( currentLayer ) )
        {
          throw QgsMapServiceException( QStringLiteral( "Security" ), QStringLiteral( "Feature access permission denied" ) );
        }
        applyAccessControlLayerFilters( currentLayer, filterRestorer->originalFilters() );
#endif

        expressionContext << QgsExpressionContextUtils::layerScope( layer );

        //is there alias info for this vector layer?
        QMap< int, QString > layerAliasInfo;
        QgsStringMap aliasMap = layer->attributeAliases();
        QgsStringMap::const_iterator aliasIt = aliasMap.constBegin();
        for ( ; aliasIt != aliasMap.constEnd(); ++aliasIt )
        {
          int attrIndex = layer->fields().lookupField( aliasIt.key() );
          if ( attrIndex != -1 )
          {
            layerAliasInfo.insert( attrIndex, aliasIt.value() );
          }
        }

        //excluded attributes for this layer
        const QSet<QString>& layerExcludedAttributes = layer->excludeAttributesWfs();

        //get layer precision
        layerPrec = mConfigParser->wfsLayerPrecision( layer->id() );

        //do a select with searchRect and go through all the features
        QgsVectorDataProvider* provider = layer->dataProvider();
        if ( !provider )
        {
          mErrors << QStringLiteral( "The layer's provider for the TypeName '%1' is not found" ).arg( mTypeName );
          continue;
        }

        QgsFeature feature;

        mWithGeom = true;

        //Using pending attributes and pending fields
        QgsAttributeList attrIndexes = layer->pendingAllAttributesList();

        QDomNodeList queryChildNodes = queryElem.childNodes();
        if ( queryChildNodes.size() )
        {
          QStringList::const_iterator alstIt;
          QList<int> idxList;
          QgsFields fields = layer->pendingFields();
          QString fieldName;
          QDomElement propertyElem;
          for ( int q = 0; q < queryChildNodes.size(); q++ )
          {
            QDomElement queryChildElem = queryChildNodes.at( q ).toElement();
            if ( queryChildElem.tagName() == QLatin1String( "PropertyName" ) )
            {
              fieldName = queryChildElem.text();
              if ( fieldName.contains( QLatin1String( ":" ) ) )
              {
                fieldName = fieldName.section( QStringLiteral( ":" ), 1, 1 );
              }
              int fieldNameIdx = fields.lookupField( fieldName );
              if ( fieldNameIdx > -1 )
              {
                idxList.append( fieldNameIdx );
              }
            }
          }
          if ( !idxList.isEmpty() )
          {
            attrIndexes = idxList;
          }
        }

        //map extent
        searchRect = layer->extent();
        searchRect.set( searchRect.xMinimum() - 1. / pow( 10., layerPrec )
                        , searchRect.yMinimum() - 1. / pow( 10., layerPrec )
                        , searchRect.xMaximum() + 1. / pow( 10., layerPrec )
                        , searchRect.yMaximum() + 1. / pow( 10., layerPrec ) );
        layerCrs = layer->crs();

        QgsFeatureRequest fReq;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
        fReq.setFlags( QgsFeatureRequest::ExactIntersect | ( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );
        mAccessControl->filterFeatures( layer, fReq );

        QStringList attributes = QStringList();
        Q_FOREACH ( int idx, attrIndexes )
        {
          attributes.append( layer->pendingFields().field( idx ).name() );
        }
        fReq.setSubsetOfAttributes(
          mAccessControl->layerAttributes( layer, attributes ),
          layer->pendingFields() );
#endif

        QgsFeatureIterator fit = layer->getFeatures( fReq );

        QDomNodeList filterNodes = queryElem.elementsByTagName( QStringLiteral( "Filter" ) );
        if ( !filterNodes.isEmpty() )
        {
          QDomElement filterElem = filterNodes.at( 0 ).toElement();
          QDomNodeList fidNodes = filterElem.elementsByTagName( QStringLiteral( "FeatureId" ) );
          if ( !fidNodes.isEmpty() )
          {
            QDomElement fidElem;
            QString fid = QLatin1String( "" );
            for ( int f = 0; f < fidNodes.size(); f++ )
            {
              fidElem = fidNodes.at( f ).toElement();
              fid = fidElem.attribute( QStringLiteral( "fid" ) );
              if ( fid.contains( QLatin1String( "." ) ) )
              {
                if ( fid.section( QStringLiteral( "." ), 0, 0 ) != mTypeName )
                  continue;
                fid = fid.section( QStringLiteral( "." ), 1, 1 );
              }

              //Need to be test for propertyname
              layer->getFeatures( QgsFeatureRequest()
                                  .setFilterFid( fid.toInt() )
                                  .setFlags( QgsFeatureRequest::ExactIntersect | ( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) )
                                  .setSubsetOfAttributes( attrIndexes )
                                ).nextFeature( feature );

              if ( featureCounter == 0 )
                startGetFeature( request, format, layerPrec, layerCrs, &searchRect );

              setGetFeature( request, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes );

              fid = QLatin1String( "" );
              ++featCounter;
              ++featureCounter;
            }
          }
          else if ( filterElem.firstChildElement().tagName() == QLatin1String( "BBOX" ) )
          {
            QDomElement bboxElem = filterElem.firstChildElement();
            QDomElement childElem = bboxElem.firstChildElement();

            QgsFeatureRequest req;
            req.setFlags( QgsFeatureRequest::ExactIntersect | ( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );

            while ( !childElem.isNull() )
            {
              if ( childElem.tagName() == QLatin1String( "Box" ) )
              {
                req.setFilterRect( QgsOgcUtils::rectangleFromGMLBox( childElem ) );
              }
              else if ( childElem.tagName() != QLatin1String( "PropertyName" ) )
              {
                QgsGeometry geom = QgsOgcUtils::geometryFromGML( childElem );
                req.setFilterRect( geom.boundingBox() );
              }
              childElem = childElem.nextSiblingElement();
            }
            req.setSubsetOfAttributes( attrIndexes );

            QgsFeatureIterator fit = layer->getFeatures( req );
            while ( fit.nextFeature( feature ) && ( !hasFeatureLimit || featureCounter < maxFeatures + startIndex ) )
            {
              if ( featureCounter == startIndex )
                startGetFeature( request, format, layerPrec, layerCrs, &searchRect );

              if ( featureCounter >= startIndex )
              {
                setGetFeature( request, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes );
                ++featCounter;
              }
              ++featureCounter;
            }
          }
          else
          {
            QSharedPointer<QgsExpression> filter( QgsOgcUtils::expressionFromOgcFilter( filterElem ) );
            if ( filter )
            {
              if ( filter->hasParserError() )
              {
                throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), filter->parserErrorString() );
              }
              QgsFeatureRequest req;
              if ( filter->needsGeometry() )
              {
                req.setFlags( QgsFeatureRequest::NoFlags );
              }
              else
              {
                req.setFlags( QgsFeatureRequest::ExactIntersect | ( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );
              }
              req.setFilterExpression( filter->expression() );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
              mAccessControl->filterFeatures( layer, req );

              QStringList attributes = QStringList();
              Q_FOREACH ( int idx, attrIndexes )
              {
                attributes.append( layer->pendingFields().field( idx ).name() );
              }
              req.setSubsetOfAttributes(
                mAccessControl->layerAttributes( layer, attributes ),
                layer->pendingFields() );
#endif
              QgsFeatureIterator fit = layer->getFeatures( req );
              while ( fit.nextFeature( feature ) && ( !hasFeatureLimit || featureCounter < maxFeatures + startIndex ) )
              {
                expressionContext.setFeature( feature );

                QVariant res = filter->evaluate( &expressionContext );
                if ( filter->hasEvalError() )
                {
                  throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), filter->evalErrorString() );
                }
                if ( res.toInt() != 0 )
                {
                  if ( featureCounter == startIndex )
                    startGetFeature( request, format, layerPrec, layerCrs, &searchRect );

                  if ( featureCounter >= startIndex )
                  {
                    setGetFeature( request, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes );
                    ++featCounter;
                  }
                  ++featureCounter;
                }
              }
            }
          }
        }
        else
        {
          while ( fit.nextFeature( feature ) && ( !hasFeatureLimit || featureCounter < maxFeatures + startIndex ) )
          {
            if ( featureCounter == startIndex )
              startGetFeature( request, format, layerPrec, layerCrs, &searchRect );

            if ( featureCounter >= startIndex )
            {
              setGetFeature( request, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes );
              ++featCounter;
            }
            ++featureCounter;
          }
        }
      }
      else
      {
        mErrors << QStringLiteral( "The layer for the TypeName '%1' is not a WFS layer" ).arg( mTypeName );
      }

    }

    //force restoration of original layer filters
    filterRestorer.reset();

    QgsMessageLog::logMessage( mErrors.join( QStringLiteral( "\n" ) ) );

    QgsProject::instance()->removeAllMapLayers();
    if ( featureCounter <= startIndex )
      startGetFeature( request, format, layerPrec, layerCrs, &searchRect );
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
  QMap<QString, QString>::const_iterator feature_id_it = mParameters.constFind( QStringLiteral( "FEATUREID" ) );
  if ( feature_id_it != mParameters.constEnd() )
  {
    featureIdOk = true;
    featureIdList = feature_id_it.value().split( QStringLiteral( "," ) );
    QStringList typeNameList;
    Q_FOREACH ( const QString &fidStr, featureIdList )
    {
      // testing typename in the WFS featureID
      if ( !fidStr.contains( QLatin1String( "." ) ) )
        throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), QStringLiteral( "FEATUREID has to have  TYPENAME in the values" ) );

      QString typeName = fidStr.section( QStringLiteral( "." ), 0, 0 );
      if ( !typeNameList.contains( typeName ) )
        typeNameList << typeName;
    }

    mTypeName = typeNameList.join( QStringLiteral( "," ) );
  }

  if ( !featureIdOk )
  {
    //read TYPENAME
    QMap<QString, QString>::const_iterator type_name_it = mParameters.constFind( QStringLiteral( "TYPENAME" ) );
    if ( type_name_it != mParameters.constEnd() )
    {
      mTypeName = type_name_it.value();
    }
    else
    {
      throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), QStringLiteral( "TYPENAME is MANDATORY" ) );
    }

    //read FILTER
    QMap<QString, QString>::const_iterator filterIt = mParameters.constFind( QStringLiteral( "FILTER" ) );
    if ( filterIt != mParameters.constEnd() )
    {
      QString errorMsg;
      if ( !filter.setContent( filterIt.value(), true, &errorMsg ) )
      {
        throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), QStringLiteral( "error message: %1. The XML string was: %2" ).arg( errorMsg, filterIt.value() ) );
      }
      else
      {
        filterOk = true;
      }
    }

    //read EXP_FILTER
    if ( !filterOk )
    {
      QMap<QString, QString>::const_iterator expFilterIt = mParameters.constFind( QStringLiteral( "EXP_FILTER" ) );
      if ( expFilterIt != mParameters.constEnd() )
      {
        expFilterOk = true;
        expFilter = expFilterIt.value();
      }
    }

    //read BBOX
    if ( !filterOk )
    {
      QMap<QString, QString>::const_iterator bbIt = mParameters.constFind( QStringLiteral( "BBOX" ) );
      if ( bbIt == mParameters.constEnd() )
      {
        minx = 0;
        miny = 0;
        maxx = 0;
        maxy = 0;
      }
      else
      {
        bool conversionSuccess;
        bboxOk = true;
        QString bbString = bbIt.value();
        minx = bbString.section( QStringLiteral( "," ), 0, 0 ).toDouble( &conversionSuccess );
        bboxOk &= conversionSuccess;
        miny = bbString.section( QStringLiteral( "," ), 1, 1 ).toDouble( &conversionSuccess );
        bboxOk &= conversionSuccess;
        maxx = bbString.section( QStringLiteral( "," ), 2, 2 ).toDouble( &conversionSuccess );
        bboxOk &= conversionSuccess;
        maxy = bbString.section( QStringLiteral( "," ), 3, 3 ).toDouble( &conversionSuccess );
        bboxOk &= conversionSuccess;
      }
    }
  }

  //read MAXFEATURES
  QMap<QString, QString>::const_iterator mfIt = mParameters.constFind( QStringLiteral( "MAXFEATURES" ) );
  if ( mfIt != mParameters.constEnd() )
  {
    QString mfString = mfIt.value();
    bool mfOk;
    hasFeatureLimit = true;
    maxFeatures = mfString.toLong( &mfOk, 10 );
  }

  //read STARTINDEX
  QMap<QString, QString>::const_iterator siIt = mParameters.constFind( QStringLiteral( "STARTINDEX" ) );
  if ( siIt != mParameters.constEnd() )
  {
    QString siString = siIt.value();
    bool siOk;
    startIndex = siString.toLong( &siOk, 10 );
  }

  //read PROPERTYNAME
  mWithGeom = true;
  mPropertyName = QStringLiteral( "*" );
  QMap<QString, QString>::const_iterator pnIt = mParameters.constFind( QStringLiteral( "PROPERTYNAME" ) );
  if ( pnIt != mParameters.constEnd() )
  {
    mPropertyName = pnIt.value();
  }
  mGeometryName = QLatin1String( "" );
  QMap<QString, QString>::const_iterator gnIt = mParameters.constFind( QStringLiteral( "GEOMETRYNAME" ) );
  if ( gnIt != mParameters.constEnd() )
  {
    mGeometryName = gnIt.value().toUpper();
  }

  mTypeNames = mTypeName.split( QStringLiteral( "," ) );
  Q_FOREACH ( const QString &tnStr, mTypeNames )
  {
    mTypeName = tnStr;
    layerList = mConfigParser->mapLayerFromTypeName( tnStr );
    if ( layerList.size() < 1 )
    {
      mErrors << QStringLiteral( "The layer for the TypeName '%1' is not found" ).arg( tnStr );
      continue;
    }

    currentLayer = layerList.at( 0 );

    QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( currentLayer );
    if ( layer && wfsLayersId.contains( layer->id() ) )
    {
      expressionContext << QgsExpressionContextUtils::layerScope( layer );

      //is there alias info for this vector layer?
      QMap< int, QString > layerAliasInfo;
      QgsStringMap aliasMap = layer->attributeAliases();
      QgsStringMap::const_iterator aliasIt = aliasMap.constBegin();
      for ( ; aliasIt != aliasMap.constEnd(); ++aliasIt )
      {
        int attrIndex = layer->fields().lookupField( aliasIt.key() );
        if ( attrIndex != -1 )
        {
          layerAliasInfo.insert( attrIndex, aliasIt.value() );
        }
      }

      //excluded attributes for this layer
      const QSet<QString>& layerExcludedAttributes = layer->excludeAttributesWfs();

      //get layer precision
      int layerPrec = mConfigParser->wfsLayerPrecision( layer->id() );

      //do a select with searchRect and go through all the features
      QgsVectorDataProvider* provider = layer->dataProvider();
      if ( !provider )
      {
        mErrors << QStringLiteral( "The layer's provider for the TypeName '%1' is not found" ).arg( tnStr );
        continue;
      }

      QgsFeature feature;

      //map extent
      searchRect = layer->extent();

      //Using pending attributes and pending fields
      QgsAttributeList attrIndexes = layer->pendingAllAttributesList();
      if ( mPropertyName != QLatin1String( "*" ) )
      {
        QStringList attrList = mPropertyName.split( QStringLiteral( "," ) );
        if ( !attrList.isEmpty() )
        {
          QStringList::const_iterator alstIt;
          QList<int> idxList;
          QgsFields fields = layer->pendingFields();
          QString fieldName;
          for ( alstIt = attrList.begin(); alstIt != attrList.end(); ++alstIt )
          {
            fieldName = *alstIt;
            int fieldNameIdx = fields.lookupField( fieldName );
            if ( fieldNameIdx > -1 )
            {
              idxList.append( fieldNameIdx );
            }
          }
          if ( !idxList.isEmpty() )
          {
            attrIndexes = idxList;
          }
        }
      }

      if ( bboxOk )
        searchRect.set( minx, miny, maxx, maxy );
      else
        searchRect.set( searchRect.xMinimum() - 1. / pow( 10., layerPrec ),
                        searchRect.yMinimum() - 1. / pow( 10., layerPrec ),
                        searchRect.xMaximum() + 1. / pow( 10., layerPrec ),
                        searchRect.yMaximum() + 1. / pow( 10., layerPrec ) );
      layerCrs = layer->crs();

      if ( featureIdOk )
      {
        Q_FOREACH ( const QString &fidStr, featureIdList )
        {
          if ( !fidStr.startsWith( tnStr ) )
            continue;
          //Need to be test for propertyname
          layer->getFeatures( QgsFeatureRequest()
                              .setFilterFid( fidStr.section( QStringLiteral( "." ), 1, 1 ).toInt() )
                              .setFlags( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry )
                              .setSubsetOfAttributes( attrIndexes )
                            ).nextFeature( feature );

          if ( featureCounter == 0 )
            startGetFeature( request, format, layerPrec, layerCrs, &searchRect );

          setGetFeature( request, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes );
          ++featCounter;
          ++featureCounter;
        }
      }
      else if ( expFilterOk )
      {
        QgsFeatureRequest req;
        if ( layer->wkbType() != QgsWkbTypes::NoGeometry )
        {
          if ( bboxOk )
          {
            req.setFilterRect( searchRect );
            req.setFlags( QgsFeatureRequest::ExactIntersect | ( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );
          }
          else
          {
            req.setFlags( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry );
          }
        }
        else
        {
          req.setFlags( QgsFeatureRequest::NoGeometry );
          mWithGeom = false;
        }
        req.setSubsetOfAttributes( attrIndexes );
        QgsFeatureIterator fit = layer->getFeatures( req );
        QSharedPointer<QgsExpression> filter( new QgsExpression( expFilter ) );
        if ( filter )
        {
          if ( filter->hasParserError() )
          {
            throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), QStringLiteral( "Expression filter error message: %1." ).arg( filter->parserErrorString() ) );
          }
          while ( fit.nextFeature( feature ) && ( !hasFeatureLimit || featureCounter < maxFeatures + startIndex ) )
          {
            expressionContext.setFeature( feature );
            QVariant res = filter->evaluate( &expressionContext );
            if ( filter->hasEvalError() )
            {
              throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), QStringLiteral( "Expression filter eval error message: %1." ).arg( filter->evalErrorString() ) );
            }
            if ( res.toInt() != 0 )
            {
              if ( featureCounter == startIndex )
                startGetFeature( request, format, layerPrec, layerCrs, &searchRect );

              if ( featureCounter >= startIndex )
              {
                setGetFeature( request, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes );
                ++featCounter;
              }
              ++featureCounter;
            }
          }
        }
      }
      else if ( filterOk )
      {
        QDomElement filterElem = filter.firstChildElement();
        QDomNodeList fidNodes = filterElem.elementsByTagName( QStringLiteral( "FeatureId" ) );
        if ( !fidNodes.isEmpty() )
        {
          QDomElement fidElem;
          QString fid = QLatin1String( "" );
          for ( int f = 0; f < fidNodes.size(); f++ )
          {
            fidElem = fidNodes.at( f ).toElement();
            fid = fidElem.attribute( QStringLiteral( "fid" ) );
            if ( fid.contains( QLatin1String( "." ) ) )
            {
              if ( fid.section( QStringLiteral( "." ), 0, 0 ) != mTypeName )
                continue;
              fid = fid.section( QStringLiteral( "." ), 1, 1 );
            }

            //Need to be test for propertyname
            layer->getFeatures( QgsFeatureRequest()
                                .setFilterFid( fid.toInt() )
                                .setFlags( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry )
                                .setSubsetOfAttributes( attrIndexes )
                              ).nextFeature( feature );

            if ( featureCounter == 0 )
              startGetFeature( request, format, layerPrec, layerCrs, &searchRect );

            setGetFeature( request, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes );

            fid = QLatin1String( "" );
            ++featCounter;
            ++featureCounter;
          }
        }
        else if ( filterElem.firstChildElement().tagName() == QLatin1String( "BBOX" ) )
        {
          QDomElement bboxElem = filterElem.firstChildElement();
          QDomElement childElem = bboxElem.firstChildElement();

          QgsFeatureRequest req;
          req.setFlags( QgsFeatureRequest::ExactIntersect | ( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );

          while ( !childElem.isNull() )
          {
            if ( childElem.tagName() == QLatin1String( "Box" ) )
            {
              req.setFilterRect( QgsOgcUtils::rectangleFromGMLBox( childElem ) );
            }
            else if ( childElem.tagName() != QLatin1String( "PropertyName" ) )
            {
              QgsGeometry geom = QgsOgcUtils::geometryFromGML( childElem );
              req.setFilterRect( geom.boundingBox() );
            }
            childElem = childElem.nextSiblingElement();
          }
          req.setSubsetOfAttributes( attrIndexes );

          QgsFeatureIterator fit = layer->getFeatures( req );
          while ( fit.nextFeature( feature ) && ( !hasFeatureLimit || featureCounter < maxFeatures + startIndex ) )
          {
            if ( featureCounter == startIndex )
              startGetFeature( request, format, layerPrec, layerCrs, &searchRect );

            if ( featureCounter >= startIndex )
            {
              setGetFeature( request, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes );
              ++featCounter;
            }
            ++featureCounter;
          }
        }
        else
        {
          QSharedPointer<QgsExpression> filter( QgsOgcUtils::expressionFromOgcFilter( filterElem ) );
          if ( filter )
          {
            if ( filter->hasParserError() )
            {
              throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), QStringLiteral( "OGC expression filter error message: %1." ).arg( filter->parserErrorString() ) );
            }
            QgsFeatureRequest req;
            if ( layer->wkbType() != QgsWkbTypes::NoGeometry )
            {
              if ( bboxOk )
              {
                req.setFilterRect( searchRect ).setFlags( QgsFeatureRequest::ExactIntersect | ( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );
              }
              else
              {
                req.setFlags( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry );
              }
            }
            else
            {
              req.setFlags( QgsFeatureRequest::NoGeometry );
              mWithGeom = false;
            }
            req.setSubsetOfAttributes( attrIndexes );
            QgsFeatureIterator fit = layer->getFeatures( req );
            while ( fit.nextFeature( feature ) && ( !hasFeatureLimit || featureCounter < maxFeatures + startIndex ) )
            {
              expressionContext.setFeature( feature );
              QVariant res = filter->evaluate( &expressionContext );
              if ( filter->hasEvalError() )
              {
                throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), QStringLiteral( "OGC expression filter eval error message: %1." ).arg( filter->evalErrorString() ) );
              }
              if ( res.toInt() != 0 )
              {
                if ( featureCounter == startIndex )
                  startGetFeature( request, format, layerPrec, layerCrs, &searchRect );

                if ( featureCounter >= startIndex )
                {
                  setGetFeature( request, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes );
                  ++featCounter;
                }
                ++featureCounter;
              }
            }
          }
        }
      }
      else
      {
        //throw QgsMapServiceException( "RequestNotWellFormed", QString( "attrIndexes length: %1." ).arg( attrIndexes.count() ) );
        QgsFeatureRequest req;
        if ( layer->wkbType() != QgsWkbTypes::NoGeometry )
        {
          if ( bboxOk )
          {
            req.setFilterRect( searchRect ).setFlags( QgsFeatureRequest::ExactIntersect | ( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) );
          }
          else
          {
            req.setFlags( mWithGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry );
          }
        }
        else
        {
          req.setFlags( QgsFeatureRequest::NoGeometry );
          mWithGeom = false;
        }
        req.setSubsetOfAttributes( attrIndexes );
        QgsFeatureIterator fit = layer->getFeatures( req );
        while ( fit.nextFeature( feature ) && ( !hasFeatureLimit || featureCounter < maxFeatures + startIndex ) )
        {
          mErrors << QStringLiteral( "The feature %2 of layer for the TypeName '%1'" ).arg( tnStr ).arg( featureCounter );
          if ( featureCounter == startIndex )
            startGetFeature( request, format, layerPrec, layerCrs, &searchRect );

          if ( featureCounter >= startIndex )
          {
            setGetFeature( request, format, &feature, featCounter, layerPrec, layerCrs, attrIndexes, layerExcludedAttributes );
            ++featCounter;
          }
          ++featureCounter;
        }
      }

    }
    else
    {
      mErrors << QStringLiteral( "The layer for the TypeName '%1' is not a WFS layer" ).arg( tnStr );
    }

  }

  QgsProject::instance()->removeAllMapLayers();
  if ( featureCounter <= startIndex )
    startGetFeature( request, format, layerPrec, layerCrs, &searchRect );
  endGetFeature( request, format );

  return 0;
}

void QgsWfsServer::startGetFeature( QgsRequestHandler& request, const QString& format, int prec, QgsCoordinateReferenceSystem& crs, QgsRectangle* rect )
{
  QByteArray result;
  QString fcString;
  if ( format == QLatin1String( "GeoJSON" ) )
  {
    if ( crs.isValid() )
    {
      QgsGeometry exportGeom = QgsGeometry::fromRect( *rect );
      QgsCoordinateTransform transform;
      transform.setSourceCrs( crs );
      transform.setDestinationCrs( QgsCoordinateReferenceSystem( 4326, QgsCoordinateReferenceSystem::EpsgCrsId ) );
      try
      {
        if ( exportGeom.transform( transform ) == 0 )
          rect = new QgsRectangle( exportGeom.boundingBox() );
      }
      catch ( QgsException &cse )
      {
        Q_UNUSED( cse );
      }
    }
    fcString = QStringLiteral( "{\"type\": \"FeatureCollection\",\n" );
    fcString += " \"bbox\": [ " + qgsDoubleToString( rect->xMinimum(), prec ) + ", " + qgsDoubleToString( rect->yMinimum(), prec ) + ", " + qgsDoubleToString( rect->xMaximum(), prec ) + ", " + qgsDoubleToString( rect->yMaximum(), prec ) + "],\n";
    fcString += QLatin1String( " \"features\": [\n" );
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
    mapUrl.addQueryItem( QStringLiteral( "SERVICE" ), QStringLiteral( "WFS" ) );
    mapUrl.addQueryItem( QStringLiteral( "VERSION" ), QStringLiteral( "1.0.0" ) );

    QList<QPair<QString, QString> > queryItems = mapUrl.queryItems();
    QList<QPair<QString, QString> >::const_iterator queryIt = queryItems.constBegin();
    for ( ; queryIt != queryItems.constEnd(); ++queryIt )
    {
      if ( queryIt->first.compare( QLatin1String( "REQUEST" ), Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( QLatin1String( "FORMAT" ), Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( QLatin1String( "OUTPUTFORMAT" ), Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( QLatin1String( "BBOX" ), Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( QLatin1String( "FEATUREID" ), Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( QLatin1String( "TYPENAME" ), Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( QLatin1String( "FILTER" ), Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( QLatin1String( "EXP_FILTER" ), Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( QLatin1String( "MAXFEATURES" ), Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( QLatin1String( "STARTINDEX" ), Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( QLatin1String( "PROPERTYNAME" ), Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( QLatin1String( "_DC" ), Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
    }
    mapUrl.addQueryItem( QStringLiteral( "REQUEST" ), QStringLiteral( "DescribeFeatureType" ) );
    mapUrl.addQueryItem( QStringLiteral( "TYPENAME" ), mTypeNames.join( QStringLiteral( "," ) ) );
    mapUrl.addQueryItem( QStringLiteral( "OUTPUTFORMAT" ), QStringLiteral( "XMLSCHEMA" ) );
    hrefString = mapUrl.toString();

    //wfs:FeatureCollection valid
    fcString = QStringLiteral( "<wfs:FeatureCollection" );
    fcString += " xmlns:wfs=\"" + WFS_NAMESPACE + "\"";
    fcString += " xmlns:ogc=\"" + OGC_NAMESPACE + "\"";
    fcString += " xmlns:gml=\"" + GML_NAMESPACE + "\"";
    fcString += QLatin1String( " xmlns:ows=\"http://www.opengis.net/ows\"" );
    fcString += QLatin1String( " xmlns:xlink=\"http://www.w3.org/1999/xlink\"" );
    fcString += " xmlns:qgs=\"" + QGS_NAMESPACE + "\"";
    fcString += QLatin1String( " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" );
    fcString += " xsi:schemaLocation=\"" + WFS_NAMESPACE + " http://schemas.opengis.net/wfs/1.0.0/wfs.xsd " + QGS_NAMESPACE + " " + hrefString.replace( QLatin1String( "&" ), QLatin1String( "&amp;" ) ) + "\"";
    fcString += QLatin1String( ">" );
    result = fcString.toUtf8();
    request.startGetFeatureResponse( &result, format );

    QDomDocument doc;
    QDomElement bbElem = doc.createElement( QStringLiteral( "gml:boundedBy" ) );
    if ( format == QLatin1String( "GML3" ) )
    {
      QDomElement envElem = QgsOgcUtils::rectangleToGMLEnvelope( rect, doc, prec );
      if ( !envElem.isNull() )
      {
        if ( crs.isValid() )
        {
          envElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
        }
        bbElem.appendChild( envElem );
        doc.appendChild( bbElem );
      }
    }
    else
    {
      QDomElement boxElem = QgsOgcUtils::rectangleToGMLBox( rect, doc, prec );
      if ( !boxElem.isNull() )
      {
        if ( crs.isValid() )
        {
          boxElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
        }
        bbElem.appendChild( boxElem );
        doc.appendChild( bbElem );
      }
    }
    result = doc.toByteArray();
    request.setGetFeatureResponse( &result );
  }
  fcString = QLatin1String( "" );
}

void QgsWfsServer::setGetFeature( QgsRequestHandler& request, const QString& format, QgsFeature* feat, int featIdx, int prec, QgsCoordinateReferenceSystem& crs, const QgsAttributeList& attrIndexes, const QSet<QString>& excludedAttributes ) /*const*/
{
  if ( !feat->isValid() )
    return;

  QByteArray result;
  if ( format == QLatin1String( "GeoJSON" ) )
  {
    QString fcString;
    if ( featIdx == 0 )
      fcString += QLatin1String( "  " );
    else
      fcString += QLatin1String( " ," );
    fcString += createFeatureGeoJSON( feat, prec, crs, attrIndexes, excludedAttributes );
    fcString += QLatin1String( "\n" );

    result = fcString.toUtf8();
    request.setGetFeatureResponse( &result );
    fcString = QLatin1String( "" );
  }
  else
  {
    QDomDocument gmlDoc;
    QDomElement featureElement;
    if ( format == QLatin1String( "GML3" ) )
    {
      featureElement = createFeatureGML3( feat, gmlDoc, prec, crs, attrIndexes, excludedAttributes );
      gmlDoc.appendChild( featureElement );
    }
    else
    {
      featureElement = createFeatureGML2( feat, gmlDoc, prec, crs, attrIndexes, excludedAttributes );
      gmlDoc.appendChild( featureElement );
    }

    result = gmlDoc.toByteArray();
    request.setGetFeatureResponse( &result );
    gmlDoc.removeChild( featureElement );
  }
}

void QgsWfsServer::endGetFeature( QgsRequestHandler& request, const QString& format )
{
  QByteArray result;
  QString fcString;
  if ( format == QLatin1String( "GeoJSON" ) )
  {
    fcString += QLatin1String( " ]\n" );
    fcString += QLatin1String( "}" );

    result = fcString.toUtf8();
    request.endGetFeatureResponse( &result );
    fcString = QLatin1String( "" );
  }
  else
  {
    fcString = QStringLiteral( "</wfs:FeatureCollection>\n" );
    result = fcString.toUtf8();
    request.endGetFeatureResponse( &result );
    fcString = QLatin1String( "" );
  }
}

QDomDocument QgsWfsServer::transaction( const QString& requestBody )
{
  // Getting  the transaction document
  QDomDocument doc;

  QString errorMsg;
  if ( !doc.setContent( requestBody, true, &errorMsg ) )
  {
    throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), errorMsg );
  }

  QDomElement docElem = doc.documentElement();
  QDomNodeList docChildNodes = docElem.childNodes();

  // Re-organize the transaction document
  QDomDocument mDoc;
  QDomElement mDocElem = mDoc.createElement( QStringLiteral( "myTransactionDocument" ) );
  mDocElem.setAttribute( QStringLiteral( "xmlns" ), QGS_NAMESPACE );
  mDocElem.setAttribute( QStringLiteral( "xmlns:wfs" ), WFS_NAMESPACE );
  mDocElem.setAttribute( QStringLiteral( "xmlns:gml" ), GML_NAMESPACE );
  mDocElem.setAttribute( QStringLiteral( "xmlns:ogc" ), OGC_NAMESPACE );
  mDocElem.setAttribute( QStringLiteral( "xmlns:qgs" ), QGS_NAMESPACE );
  mDocElem.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
  mDoc.appendChild( mDocElem );

  QDomElement actionElem;
  QString actionName;
  QDomElement typeNameElem;
  QString typeName;

  for ( int i = docChildNodes.count(); 0 < i; --i )
  {
    actionElem = docChildNodes.at( i - 1 ).toElement();
    actionName = actionElem.localName();

    if ( actionName == QLatin1String( "Insert" ) )
    {
      QDomElement featureElem = actionElem.firstChild().toElement();
      typeName = featureElem.localName();
    }
    else if ( actionName == QLatin1String( "Update" ) )
    {
      typeName = actionElem.attribute( QStringLiteral( "typeName" ) );
    }
    else if ( actionName == QLatin1String( "Delete" ) )
    {
      typeName = actionElem.attribute( QStringLiteral( "typeName" ) );
    }

    if ( typeName.contains( QLatin1String( ":" ) ) )
      typeName = typeName.section( QStringLiteral( ":" ), 1, 1 );

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
  QDomElement respElem = resp.createElement( QStringLiteral( "WFS_TransactionResponse" )/*wfs:WFS_TransactionResponse*/ );
  respElem.setAttribute( QStringLiteral( "xmlns" ), WFS_NAMESPACE );
  respElem.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
  respElem.setAttribute( QStringLiteral( "xsi:schemaLocation" ), WFS_NAMESPACE + " http://schemas.opengis.net/wfs/1.0.0/wfs.xsd" );
  respElem.setAttribute( QStringLiteral( "xmlns:ogc" ), OGC_NAMESPACE );
  respElem.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0.0" ) );
  resp.appendChild( respElem );

  // Store the created feature id for WFS
  QStringList insertResults;
  // Get the WFS layers id
  QStringList wfsLayersId = mConfigParser ? mConfigParser->wfsLayers() : QStringList();

  QList<QgsMapLayer*> layerList;
  QgsMapLayer* currentLayer = nullptr;

  // Loop through the layer transaction elements
  docChildNodes = mDocElem.childNodes();
  for ( int i = 0; i < docChildNodes.count(); ++i )
  {
    // Get the vector layer
    typeNameElem = docChildNodes.at( i ).toElement();
    mTypeName = typeNameElem.tagName();

    layerList = mConfigParser->mapLayerFromTypeName( mTypeName );
    // Could be empty!
    if ( layerList.count() > 0 )
    {
      currentLayer = layerList.at( 0 );
    }
    else
    {
      throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), QStringLiteral( "Wrong TypeName: %1" ).arg( mTypeName ) );
    }

    QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( currentLayer );
    // it's a vectorlayer and defined by the administrator as a WFS layer
    if ( layer && wfsLayersId.contains( layer->id() ) )
    {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( actionName == QLatin1String( "Insert" ) )
      {
        if ( !mAccessControl->layerInsertPermission( layer ) )
        {
          throw QgsMapServiceException( QStringLiteral( "Security" ), QStringLiteral( "Feature insert permission denied" ) );
        }
      }
      else if ( actionName == QLatin1String( "Update" ) )
      {
        if ( !mAccessControl->layerUpdatePermission( layer ) )
        {
          throw QgsMapServiceException( QStringLiteral( "Security" ), QStringLiteral( "Feature update permission denied" ) );
        }
      }
      else if ( actionName == QLatin1String( "Delete" ) )
      {
        if ( !mAccessControl->layerDeletePermission( layer ) )
        {
          throw QgsMapServiceException( QStringLiteral( "Security" ), QStringLiteral( "Feature delete permission denied" ) );
        }
      }
#endif

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
        QDomNodeList upNodeList = typeNameElem.elementsByTagNameNS( WFS_NAMESPACE, QStringLiteral( "Update" ) );
        for ( int j = 0; j < upNodeList.count(); ++j )
        {
          if ( !mConfigParser->wfstUpdateLayers().contains( layer->id() ) )
          {
            //no wfs permissions to do updates
            QString errorMsg = "No permissions to do WFS updates on layer '" + layer->name() + "'";
            QgsMessageLog::logMessage( errorMsg, QStringLiteral( "Server" ), QgsMessageLog::CRITICAL );
            addTransactionResult( resp, respElem, QStringLiteral( "FAILED" ), QStringLiteral( "Update" ), errorMsg );
            return resp;
          }

          actionElem = upNodeList.at( j ).toElement();

          // Get the Feature Ids for this filter on the layer
          QDomElement filterElem = actionElem.elementsByTagName( QStringLiteral( "Filter" ) ).at( 0 ).toElement();
          QgsFeatureIds fids = getFeatureIdsFromFilter( filterElem, layer );

          // Loop through the property elements
          // Store properties and the geometry element
          QDomNodeList propertyNodeList = actionElem.elementsByTagName( QStringLiteral( "Property" ) );
          QMap<QString, QString> propertyMap;
          QDomElement propertyElem;
          QDomElement nameElem;
          QDomElement valueElem;
          QDomElement geometryElem;

          for ( int l = 0; l < propertyNodeList.count(); ++l )
          {
            propertyElem = propertyNodeList.at( l ).toElement();
            nameElem = propertyElem.elementsByTagName( QStringLiteral( "Name" ) ).at( 0 ).toElement();
            valueElem = propertyElem.elementsByTagName( QStringLiteral( "Value" ) ).at( 0 ).toElement();
            if ( nameElem.text() != QLatin1String( "geometry" ) )
            {
              propertyMap.insert( nameElem.text(), valueElem.text() );
            }
            else
            {
              geometryElem = valueElem;
            }
          }

          // Update the features
          QgsFields fields = provider->fields();
          QMap<QString, int> fieldMap = provider->fieldNameMap();
          QMap<QString, int>::const_iterator fieldMapIt;
          QString fieldName;
          bool conversionSuccess;

          QgsFeatureIds::const_iterator fidIt = fids.constBegin();
          for ( ; fidIt != fids.constEnd(); ++fidIt )
          {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
            QgsFeatureIterator fit = layer->getFeatures( QgsFeatureRequest( *fidIt ) );
            QgsFeature feature;
            while ( fit.nextFeature( feature ) )
            {
              if ( !mAccessControl->allowToEdit( layer, feature ) )
              {
                throw QgsMapServiceException( QStringLiteral( "Security" ), QStringLiteral( "Feature modify permission denied" ) );
              }
            }
#endif

            QMap< QString, QString >::const_iterator it = propertyMap.constBegin();
            for ( ; it != propertyMap.constEnd(); ++it )
            {
              fieldName = it.key();
              fieldMapIt = fieldMap.find( fieldName );
              if ( fieldMapIt == fieldMap.constEnd() )
              {
                continue;
              }
              QgsField field = fields.at( fieldMapIt.value() );
              if ( field.type() == 2 )
                layer->changeAttributeValue( *fidIt, fieldMapIt.value(), it.value().toInt( &conversionSuccess ) );
              else if ( field.type() == 6 )
                layer->changeAttributeValue( *fidIt, fieldMapIt.value(), it.value().toDouble( &conversionSuccess ) );
              else
                layer->changeAttributeValue( *fidIt, fieldMapIt.value(), it.value() );
            }

            if ( !geometryElem.isNull() )
            {
              QgsGeometry g = QgsOgcUtils::geometryFromGML( geometryElem );
              if ( !layer->changeGeometry( *fidIt, g ) )
              {
                throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), QStringLiteral( "Error in change geometry" ) );
              }
            }

#ifdef HAVE_SERVER_PYTHON_PLUGINS
            fit = layer->getFeatures( QgsFeatureRequest( *fidIt ) );
            while ( fit.nextFeature( feature ) )
            {
              if ( !mAccessControl->allowToEdit( layer, feature ) )
              {
                layer->rollBack();
                throw QgsMapServiceException( QStringLiteral( "Security" ), QStringLiteral( "Feature modify permission denied" ) );
              }
            }
#endif
          }
        }
      }
      // Commit the changes of the update elements
      if ( !layer->commitChanges() )
      {
        addTransactionResult( resp, respElem, QStringLiteral( "PARTIAL" ), QStringLiteral( "Update" ), layer->commitErrors().join( QStringLiteral( "\n  " ) ) );
        return resp;
      }
      // Start the delete transaction
      layer->startEditing();
      if (( cap & QgsVectorDataProvider::DeleteFeatures ) )
      {
        // Loop through the delete elements
        QDomNodeList delNodeList = typeNameElem.elementsByTagNameNS( WFS_NAMESPACE, QStringLiteral( "Delete" ) );
        for ( int j = 0; j < delNodeList.count(); ++j )
        {
          if ( !mConfigParser->wfstDeleteLayers().contains( layer->id() ) )
          {
            //no wfs permissions to do updates
            QString errorMsg = "No permissions to do WFS deletes on layer '" + layer->name() + "'";
            QgsMessageLog::logMessage( errorMsg, QStringLiteral( "Server" ), QgsMessageLog::CRITICAL );
            addTransactionResult( resp, respElem, QStringLiteral( "FAILED" ), QStringLiteral( "Delete" ), errorMsg );
            return resp;
          }

          actionElem = delNodeList.at( j ).toElement();
          QDomElement filterElem = actionElem.firstChild().toElement();
          // Get Feature Ids for the Filter element
          QgsFeatureIds fids = getFeatureIdsFromFilter( filterElem, layer );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
          QgsFeatureIds::const_iterator fidIt = fids.constBegin();
          for ( ; fidIt != fids.constEnd(); ++fidIt )
          {
            QgsFeatureIterator fit = layer->getFeatures( QgsFeatureRequest( *fidIt ) );
            QgsFeature feature;
            while ( fit.nextFeature( feature ) )
            {
              if ( !mAccessControl->allowToEdit( layer, feature ) )
              {
                throw QgsMapServiceException( QStringLiteral( "Security" ), QStringLiteral( "Feature modify permission denied" ) );
              }
            }
          }
#endif

          layer->selectByIds( fids );
          layer->deleteSelectedFeatures();
        }
      }
      // Commit the changes of the delete elements
      if ( !layer->commitChanges() )
      {
        addTransactionResult( resp, respElem, QStringLiteral( "PARTIAL" ), QStringLiteral( "Delete" ), layer->commitErrors().join( QStringLiteral( "\n  " ) ) );
        return resp;
      }

      // Store the inserted features
      QgsFeatureList inFeatList;
      if ( cap & QgsVectorDataProvider::AddFeatures )
      {
        // Get Layer Field Information
        QgsFields fields = provider->fields();
        QMap<QString, int> fieldMap = provider->fieldNameMap();
        QMap<QString, int>::const_iterator fieldMapIt;

        // Loop through the insert elements
        QDomNodeList inNodeList = typeNameElem.elementsByTagNameNS( WFS_NAMESPACE, QStringLiteral( "Insert" ) );
        for ( int j = 0; j < inNodeList.count(); ++j )
        {
          if ( !mConfigParser->wfstInsertLayers().contains( layer->id() ) )
          {
            //no wfs permissions to do updates
            QString errorMsg = "No permissions to do WFS inserts on layer '" + layer->name() + "'";
            QgsMessageLog::logMessage( errorMsg, QStringLiteral( "Server" ), QgsMessageLog::CRITICAL );
            addTransactionResult( resp, respElem, QStringLiteral( "FAILED" ), QStringLiteral( "Insert" ), errorMsg );
            return resp;
          }

          actionElem = inNodeList.at( j ).toElement();
          // Loop through the feature element
          QDomNodeList featNodes = actionElem.childNodes();
          for ( int l = 0; l < featNodes.count(); l++ )
          {
            // Add the feature to the layer
            // and store it to put it's Feature Id in the response
            inFeatList << QgsFeature( fields );

            // Create feature for this layer
            QDomElement featureElem = featNodes.at( l ).toElement();

            QDomNode currentAttributeChild = featureElem.firstChild();

            while ( !currentAttributeChild.isNull() )
            {
              QDomElement currentAttributeElement = currentAttributeChild.toElement();
              QString attrName = currentAttributeElement.localName();

              if ( attrName != QLatin1String( "boundedBy" ) )
              {
                if ( attrName != QLatin1String( "geometry" ) ) //a normal attribute
                {
                  fieldMapIt = fieldMap.find( attrName );
                  if ( fieldMapIt == fieldMap.constEnd() )
                  {
                    continue;
                  }
                  QgsField field = fields.at( fieldMapIt.value() );
                  QString attrValue = currentAttributeElement.text();
                  int attrType = field.type();
                  QgsMessageLog::logMessage( QStringLiteral( "attr: name=%1 idx=%2 value=%3" ).arg( attrName ).arg( fieldMapIt.value() ).arg( attrValue ) );
                  if ( attrType == QVariant::Int )
                    inFeatList.last().setAttribute( fieldMapIt.value(), attrValue.toInt() );
                  else if ( attrType == QVariant::Double )
                    inFeatList.last().setAttribute( fieldMapIt.value(), attrValue.toDouble() );
                  else
                    inFeatList.last().setAttribute( fieldMapIt.value(), attrValue );
                }
                else //a geometry attribute
                {
                  QgsGeometry g = QgsOgcUtils::geometryFromGML( currentAttributeElement );
                  inFeatList.last().setGeometry( g );
                }
              }
              currentAttributeChild = currentAttributeChild.nextSibling();
            }
          }
        }
      }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      QgsFeatureList::iterator featureIt = inFeatList.begin();
      while ( featureIt != inFeatList.end() )
      {
        if ( !mAccessControl->allowToEdit( layer, *featureIt ) )
        {
          throw QgsMapServiceException( QStringLiteral( "Security" ), QStringLiteral( "Feature modify permission denied" ) );
        }
        featureIt++;
      }
#endif

      // add the features
      if ( !provider->addFeatures( inFeatList ) )
      {
        addTransactionResult( resp, respElem, QStringLiteral( "Partial" ), QStringLiteral( "Insert" ), layer->commitErrors().join( QStringLiteral( "\n  " ) ) );
        if ( provider->hasErrors() )
        {
          provider->clearErrors();
        }
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
  if ( !insertResults.isEmpty() )
  {
    Q_FOREACH ( const QString &fidStr, insertResults )
    {
      QDomElement irElem = doc.createElement( QStringLiteral( "InsertResult" ) );
      QDomElement fiElem = doc.createElement( QStringLiteral( "ogc:FeatureId" ) );
      fiElem.setAttribute( QStringLiteral( "fid" ), fidStr );
      irElem.appendChild( fiElem );
      respElem.appendChild( irElem );
    }
  }

  // Set the transaction reposne for success
  QDomElement trElem = doc.createElement( QStringLiteral( "TransactionResult" ) );
  QDomElement stElem = doc.createElement( QStringLiteral( "Status" ) );
  QDomElement successElem = doc.createElement( QStringLiteral( "SUCCESS" ) );
  stElem.appendChild( successElem );
  trElem.appendChild( stElem );
  respElem.appendChild( trElem );

  return resp;
}

QgsFeatureIds QgsWfsServer::getFeatureIdsFromFilter( const QDomElement& filterElem, QgsVectorLayer* layer )
{
  QgsFeatureIds fids;

  QgsVectorDataProvider *provider = layer->dataProvider();
  QDomNodeList fidNodes = filterElem.elementsByTagName( QStringLiteral( "FeatureId" ) );

  if ( fidNodes.size() != 0 )
  {
    QDomElement fidElem;
    QString fid;
    bool conversionSuccess;
    for ( int i = 0; i < fidNodes.size(); ++i )
    {
      fidElem = fidNodes.at( i ).toElement();
      fid = fidElem.attribute( QStringLiteral( "fid" ) );
      if ( fid.contains( QLatin1String( "." ) ) )
        fid = fid.section( QStringLiteral( "." ), 1, 1 );
      fids.insert( fid.toLongLong( &conversionSuccess ) );
    }
  }
  else
  {
    QSharedPointer<QgsExpression> filter( QgsOgcUtils::expressionFromOgcFilter( filterElem ) );
    if ( filter )
    {
      if ( filter->hasParserError() )
      {
        throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), filter->parserErrorString() );
      }
      QgsFeature feature;
      QgsFields fields = provider->fields();
      QgsFeatureIterator fit = layer->getFeatures();
      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( feature, fields );

      while ( fit.nextFeature( feature ) )
      {
        context.setFeature( feature );
        QVariant res = filter->evaluate( &context );
        if ( filter->hasEvalError() )
        {
          throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), filter->evalErrorString() );
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

QString QgsWfsServer::createFeatureGeoJSON( QgsFeature* feat, int prec, QgsCoordinateReferenceSystem& crs, const QgsAttributeList& attrIndexes, const QSet<QString>& excludedAttributes ) /*const*/
{
  QString id = QStringLiteral( "%1.%2" ).arg( mTypeName, FID_TO_STRING( feat->id() ) );

  QgsJSONExporter exporter;
  exporter.setSourceCrs( crs );
  exporter.setPrecision( prec );

  //copy feature so we can modify its geometry as required
  QgsFeature f( *feat );
  QgsGeometry geom = feat->geometry();
  exporter.setIncludeGeometry( false );
  if ( !geom.isEmpty() && mWithGeom && mGeometryName != QLatin1String( "NONE" ) )
  {
    exporter.setIncludeGeometry( true );
    if ( mGeometryName == QLatin1String( "EXTENT" ) )
    {
      QgsRectangle box = geom.boundingBox();
      f.setGeometry( QgsGeometry::fromRect( box ) );
    }
    else if ( mGeometryName == QLatin1String( "CENTROID" ) )
    {
      f.setGeometry( geom.centroid() );
    }
  }

  QgsFields fields = feat->fields();
  QgsAttributeList attrsToExport;
  for ( int i = 0; i < attrIndexes.count(); ++i )
  {
    int idx = attrIndexes[i];
    if ( idx >= fields.count() )
    {
      continue;
    }
    QString attributeName = fields.at( idx ).name();
    //skip attribute if it is excluded from WFS publication
    if ( excludedAttributes.contains( attributeName ) )
    {
      continue;
    }

    attrsToExport << idx;
  }

  exporter.setIncludeAttributes( !attrsToExport.isEmpty() );
  exporter.setAttributes( attrsToExport );

  return exporter.exportFeature( f, QVariantMap(), id );
}

QDomElement QgsWfsServer::createFeatureGML2( QgsFeature* feat, QDomDocument& doc, int prec, QgsCoordinateReferenceSystem& crs, const QgsAttributeList& attrIndexes, const QSet<QString>& excludedAttributes ) /*const*/
{
  //gml:FeatureMember
  QDomElement featureElement = doc.createElement( QStringLiteral( "gml:featureMember" )/*wfs:FeatureMember*/ );

  //qgs:%TYPENAME%
  QDomElement typeNameElement = doc.createElement( "qgs:" + mTypeName /*qgs:%TYPENAME%*/ );
  typeNameElement.setAttribute( QStringLiteral( "fid" ), mTypeName + "." + QString::number( feat->id() ) );
  featureElement.appendChild( typeNameElement );

  if ( mWithGeom && mGeometryName != QLatin1String( "NONE" ) )
  {
    //add geometry column (as gml)
    QgsGeometry geom = feat->geometry();

    QDomElement geomElem = doc.createElement( QStringLiteral( "qgs:geometry" ) );
    QDomElement gmlElem;
    if ( mGeometryName == QLatin1String( "EXTENT" ) )
    {
      QgsGeometry bbox = QgsGeometry::fromRect( geom.boundingBox() );
      gmlElem = QgsOgcUtils::geometryToGML( &bbox , doc, prec );
    }
    else if ( mGeometryName == QLatin1String( "CENTROID" ) )
    {
      QgsGeometry centroid = geom.centroid();
      gmlElem = QgsOgcUtils::geometryToGML( &centroid, doc, prec );
    }
    else
      gmlElem = QgsOgcUtils::geometryToGML( &geom, doc, prec );
    if ( !gmlElem.isNull() )
    {
      QgsRectangle box = geom.boundingBox();
      QDomElement bbElem = doc.createElement( QStringLiteral( "gml:boundedBy" ) );
      QDomElement boxElem = QgsOgcUtils::rectangleToGMLBox( &box, doc, prec );

      if ( crs.isValid() )
      {
        boxElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
        gmlElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
      }

      bbElem.appendChild( boxElem );
      typeNameElement.appendChild( bbElem );

      geomElem.appendChild( gmlElem );
      typeNameElement.appendChild( geomElem );
    }
  }

  //read all attribute values from the feature
  QgsAttributes featureAttributes = feat->attributes();
  QgsFields fields = feat->fields();
  for ( int i = 0; i < attrIndexes.count(); ++i )
  {
    int idx = attrIndexes[i];
    if ( idx >= fields.count() )
    {
      continue;
    }
    QString attributeName = fields.at( idx ).name();
    //skip attribute if it is excluded from WFS publication
    if ( excludedAttributes.contains( attributeName ) )
    {
      continue;
    }

    QDomElement fieldElem = doc.createElement( "qgs:" + attributeName.replace( QStringLiteral( " " ), QStringLiteral( "_" ) ) );
    QDomText fieldText = doc.createTextNode( featureAttributes[idx].toString() );
    fieldElem.appendChild( fieldText );
    typeNameElement.appendChild( fieldElem );
  }

  return featureElement;
}

QDomElement QgsWfsServer::createFeatureGML3( QgsFeature* feat, QDomDocument& doc, int prec, QgsCoordinateReferenceSystem& crs, const QgsAttributeList& attrIndexes, const QSet<QString>& excludedAttributes ) /*const*/
{
  //gml:FeatureMember
  QDomElement featureElement = doc.createElement( QStringLiteral( "gml:featureMember" )/*wfs:FeatureMember*/ );

  //qgs:%TYPENAME%
  QDomElement typeNameElement = doc.createElement( "qgs:" + mTypeName /*qgs:%TYPENAME%*/ );
  typeNameElement.setAttribute( QStringLiteral( "gml:id" ), mTypeName + "." + QString::number( feat->id() ) );
  featureElement.appendChild( typeNameElement );

  if ( mWithGeom && mGeometryName != QLatin1String( "NONE" ) )
  {
    //add geometry column (as gml)
    QgsGeometry geom = feat->geometry();

    QDomElement geomElem = doc.createElement( QStringLiteral( "qgs:geometry" ) );
    QDomElement gmlElem;
    if ( mGeometryName == QLatin1String( "EXTENT" ) )
    {
      QgsGeometry bbox = QgsGeometry::fromRect( geom.boundingBox() );
      gmlElem = QgsOgcUtils::geometryToGML( &bbox, doc, QStringLiteral( "GML3" ), prec );
    }
    else if ( mGeometryName == QLatin1String( "CENTROID" ) )
    {
      QgsGeometry centroid = geom.centroid();
      gmlElem = QgsOgcUtils::geometryToGML( &centroid, doc, QStringLiteral( "GML3" ), prec );
    }
    else
      gmlElem = QgsOgcUtils::geometryToGML( &geom, doc, QStringLiteral( "GML3" ), prec );
    if ( !gmlElem.isNull() )
    {
      QgsRectangle box = geom.boundingBox();
      QDomElement bbElem = doc.createElement( QStringLiteral( "gml:boundedBy" ) );
      QDomElement boxElem = QgsOgcUtils::rectangleToGMLEnvelope( &box, doc, prec );

      if ( crs.isValid() )
      {
        boxElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
        gmlElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
      }

      bbElem.appendChild( boxElem );
      typeNameElement.appendChild( bbElem );

      geomElem.appendChild( gmlElem );
      typeNameElement.appendChild( geomElem );
    }
  }

  //read all attribute values from the feature
  QgsAttributes featureAttributes = feat->attributes();
  QgsFields fields = feat->fields();
  for ( int i = 0; i < attrIndexes.count(); ++i )
  {
    int idx = attrIndexes[i];
    if ( idx >= fields.count() )
    {
      continue;
    }
    QString attributeName = fields.at( idx ).name();
    //skip attribute if it is excluded from WFS publication
    if ( excludedAttributes.contains( attributeName ) )
    {
      continue;
    }

    QDomElement fieldElem = doc.createElement( "qgs:" + attributeName.replace( QStringLiteral( " " ), QStringLiteral( "_" ) ) );
    QDomText fieldText = doc.createTextNode( featureAttributes[idx].toString() );
    fieldElem.appendChild( fieldText );
    typeNameElement.appendChild( fieldElem );
  }

  return featureElement;
}

QString QgsWfsServer::serviceUrl() const
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

  if ( QString( getenv( "HTTPS" ) ).compare( QLatin1String( "on" ), Qt::CaseInsensitive ) == 0 )
  {
    mapUrl.setScheme( QStringLiteral( "https" ) );
  }
  else
  {
    mapUrl.setScheme( QStringLiteral( "http" ) );
  }

  QList<QPair<QString, QString> > queryItems = mapUrl.queryItems();
  QList<QPair<QString, QString> >::const_iterator queryIt = queryItems.constBegin();
  for ( ; queryIt != queryItems.constEnd(); ++queryIt )
  {
    if ( queryIt->first.compare( QLatin1String( "REQUEST" ), Qt::CaseInsensitive ) == 0 )
    {
      mapUrl.removeQueryItem( queryIt->first );
    }
    else if ( queryIt->first.compare( QLatin1String( "VERSION" ), Qt::CaseInsensitive ) == 0 )
    {
      mapUrl.removeQueryItem( queryIt->first );
    }
    else if ( queryIt->first.compare( QLatin1String( "SERVICE" ), Qt::CaseInsensitive ) == 0 )
    {
      mapUrl.removeQueryItem( queryIt->first );
    }
    else if ( queryIt->first.compare( QLatin1String( "_DC" ), Qt::CaseInsensitive ) == 0 )
    {
      mapUrl.removeQueryItem( queryIt->first );
    }
  }
  return mapUrl.toString();
}

void QgsWfsServer::addTransactionResult( QDomDocument& responseDoc, QDomElement& responseElem, const QString& status, const QString& locator, const QString& message )
{
  QDomElement trElem = responseDoc.createElement( QStringLiteral( "TransactionResult" ) );
  QDomElement stElem = responseDoc.createElement( QStringLiteral( "Status" ) );
  QDomElement successElem = responseDoc.createElement( status );
  stElem.appendChild( successElem );
  trElem.appendChild( stElem );
  responseElem.appendChild( trElem );

  QDomElement locElem = responseDoc.createElement( QStringLiteral( "Locator" ) );
  locElem.appendChild( responseDoc.createTextNode( locator ) );
  trElem.appendChild( locElem );

  QDomElement mesElem = responseDoc.createElement( QStringLiteral( "Message" ) );
  mesElem.appendChild( responseDoc.createTextNode( message ) );
  trElem.appendChild( mesElem );
}
