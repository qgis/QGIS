/***************************************************************************
                              qgswfsgecapabilities.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
                         (C) 2012 by René-Luc D'Hont    (original code)
                         (C) 2014 by Alessandro Pasotti (original code)
                         (C) 2017 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswfsutils.h"
#include "qgsserverprojectutils.h"
#include "qgswfsgetcapabilities.h"

#include "qgsproject.h"
#include "qgsexception.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgscoordinatereferencesystem.h"

namespace QgsWfs
{

  /**
   * Output WFS  GetCapabilities response
   */
  void writeGetCapabilities( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
                             const QgsServerRequest &request, QgsServerResponse &response )
  {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
#endif
    QDomDocument doc;
    const QDomDocument *capabilitiesDocument = nullptr;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsServerCacheManager *cacheManager = serverIface->cacheManager();
    if ( cacheManager && cacheManager->getCachedDocument( &doc, project, request, accessControl ) )
    {
      capabilitiesDocument = &doc;
    }
    else //capabilities xml not in cache. Create a new one
    {
      doc = createGetCapabilitiesDocument( serverIface, project, version, request );

      if ( cacheManager )
      {
        cacheManager->setCachedDocument( &doc, project, request, accessControl );
      }
      capabilitiesDocument = &doc;
    }
#else
    doc = createGetCapabilitiesDocument( serverIface, project, version, request );
    capabilitiesDocument = &doc;
#endif
    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
    response.write( capabilitiesDocument->toByteArray() );
  }


  QDomDocument createGetCapabilitiesDocument( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
      const QgsServerRequest &request )
  {
    Q_UNUSED( version )

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
    wfsCapabilitiesElement.setAttribute( QStringLiteral( "version" ), implementationVersion() );
    wfsCapabilitiesElement.setAttribute( QStringLiteral( "updateSequence" ), QStringLiteral( "0" ) );
    doc.appendChild( wfsCapabilitiesElement );

    //ows:ServiceIdentification
    wfsCapabilitiesElement.appendChild( getServiceIdentificationElement( doc, project ) );

    //ows:ServiceProvider
    wfsCapabilitiesElement.appendChild( getServiceProviderElement( doc, project ) );

    //wfs:OperationsMetadata
    wfsCapabilitiesElement.appendChild( getOperationsMetadataElement( doc, project, request, serverIface->serverSettings() ) );

    //wfs:FeatureTypeList
    wfsCapabilitiesElement.appendChild( getFeatureTypeListElement( doc, serverIface, project ) );

    /*
     * Adding ogc:Filter_Capabilities in wfsCapabilitiesElement
     */
    //ogc:Filter_Capabilities element
    QDomElement filterCapabilitiesElement = doc.createElement( QStringLiteral( "ogc:Filter_Capabilities" )/*ogc:Filter_Capabilities*/ );
    wfsCapabilitiesElement.appendChild( filterCapabilitiesElement );
    QDomElement spatialCapabilitiesElement = doc.createElement( QStringLiteral( "ogc:Spatial_Capabilities" )/*ogc:Spatial_Capabilities*/ );
    filterCapabilitiesElement.appendChild( spatialCapabilitiesElement );
    //GeometryOperands
    QStringList geometryOperands;
    geometryOperands << QStringLiteral( "gml:Point" ) << QStringLiteral( "gml:LineString" ) << QStringLiteral( "gml:Polygon" )
                     << QStringLiteral( "gml:Envelope" );
    QDomElement geometryOperandsElem = doc.createElement( QStringLiteral( "ogc:GeometryOperands" ) );
    for ( const QString &geometryOperand : geometryOperands )
    {
      QDomElement geometryOperandElem = doc.createElement( QStringLiteral( "ogc:GeometryOperand" ) );
      QDomText geometryOperandText = doc.createTextNode( geometryOperand );
      geometryOperandElem.appendChild( geometryOperandText );
      geometryOperandsElem.appendChild( geometryOperandElem );
    }
    spatialCapabilitiesElement.appendChild( geometryOperandsElem );
    //SpatialOperators
    QStringList spatialOperators;
    spatialOperators << QStringLiteral( "Equals" ) << QStringLiteral( "Disjoint" ) << QStringLiteral( "Touches" )
                     << QStringLiteral( "Within" ) << QStringLiteral( "Overlaps" ) << QStringLiteral( "Crosses" )
                     << QStringLiteral( "Intersects" ) << QStringLiteral( "Contains" ) << QStringLiteral( "BBOX" );
    QDomElement spatialOperatorsElem = doc.createElement( QStringLiteral( "ogc:SpatialOperators" ) );
    for ( const QString &spatialOperator : spatialOperators )
    {
      QDomElement spatialOperatorElem = doc.createElement( QStringLiteral( "ogc:SpatialOperator" ) );
      spatialOperatorElem.setAttribute( QStringLiteral( "name" ), spatialOperator );
      spatialOperatorsElem.appendChild( spatialOperatorElem );
    }
    spatialCapabilitiesElement.appendChild( spatialOperatorsElem );
    QDomElement scalarCapabilitiesElement = doc.createElement( QStringLiteral( "ogc:Scalar_Capabilities" )/*ogc:Scalar_Capabilities*/ );
    filterCapabilitiesElement.appendChild( scalarCapabilitiesElement );
    QDomElement logicalOperatorsElement = doc.createElement( QStringLiteral( "ogc:LogicalOperators" ) );
    scalarCapabilitiesElement.appendChild( logicalOperatorsElement );
    // ComparisonOperators
    QStringList comparisonOperators;
    comparisonOperators << QStringLiteral( "LessThan" ) << QStringLiteral( "GreaterThan" )
                        << QStringLiteral( "LessThanEqualTo" ) << QStringLiteral( "GreaterThanEqualTo" )
                        << QStringLiteral( "EqualTo" ) << QStringLiteral( "Like" ) << QStringLiteral( "Between" );
    QDomElement comparisonOperatorsElem = doc.createElement( QStringLiteral( "ogc:ComparisonOperators" ) );
    for ( const QString &comparisonOperator : comparisonOperators )
    {
      QDomElement comparisonOperatorElem = doc.createElement( QStringLiteral( "ogc:ComparisonOperator" ) );
      QDomText comparisonOperatorText = doc.createTextNode( comparisonOperator );
      comparisonOperatorElem.appendChild( comparisonOperatorText );
      comparisonOperatorsElem.appendChild( comparisonOperatorElem );
    }
    scalarCapabilitiesElement.appendChild( comparisonOperatorsElem );

    QDomElement idCapabilitiesElement = doc.createElement( QStringLiteral( "ogc:Id_Capabilities" ) );
    QDomElement fidElem = doc.createElement( QStringLiteral( "ogc:FID" ) );
    idCapabilitiesElement.appendChild( fidElem );
    filterCapabilitiesElement.appendChild( idCapabilitiesElement );

    return doc;

  }

  QDomElement getServiceIdentificationElement( QDomDocument &doc, const QgsProject *project )
  {
    //Service element
    QDomElement serviceElem = doc.createElement( QStringLiteral( "ows:ServiceIdentification" ) );

    QString title = QgsServerProjectUtils::owsServiceTitle( *project );
    if ( !title.isEmpty() )
    {
      QDomElement titleElem = doc.createElement( QStringLiteral( "ows:Title" ) );
      QDomText titleText = doc.createTextNode( title );
      titleElem.appendChild( titleText );
      serviceElem.appendChild( titleElem );
    }

    QString abstract = QgsServerProjectUtils::owsServiceAbstract( *project );
    if ( !abstract.isEmpty() )
    {
      QDomElement abstractElem = doc.createElement( QStringLiteral( "ows:Abstract" ) );
      QDomText abstractText = doc.createCDATASection( abstract );
      abstractElem.appendChild( abstractText );
      serviceElem.appendChild( abstractElem );
    }

    QStringList keywords = QgsServerProjectUtils::owsServiceKeywords( *project );
    if ( !keywords.isEmpty() && !keywords.join( QLatin1String( ", " ) ).isEmpty() )
    {
      QDomElement keywordsElem = doc.createElement( QStringLiteral( "ows:Keywords" ) );
      for ( const QString &keyword : keywords )
      {
        if ( !keyword.isEmpty() )
        {
          QDomElement keywordElem = doc.createElement( QStringLiteral( "ows:Keyword" ) );
          QDomText keywordText = doc.createTextNode( keyword );
          keywordElem.appendChild( keywordText );
          keywordsElem.appendChild( keywordElem );
        }
      }
      serviceElem.appendChild( keywordsElem );
    }

    //Service type
    QDomElement serviceTypeElem = doc.createElement( QStringLiteral( "ows:ServiceType" ) );
    QDomText serviceTypeText = doc.createTextNode( "WFS" );
    serviceTypeElem.appendChild( serviceTypeText );
    serviceElem.appendChild( serviceTypeElem );

    //Service type version
    QDomElement serviceTypeVersionElem = doc.createElement( QStringLiteral( "ows:ServiceTypeVersion" ) );
    QDomText serviceTypeVersionText = doc.createTextNode( "1.1.0" );
    serviceTypeVersionElem.appendChild( serviceTypeVersionText );
    serviceElem.appendChild( serviceTypeVersionElem );

    QDomElement feesElem = doc.createElement( QStringLiteral( "ows:Fees" ) );
    QDomText feesText = doc.createTextNode( "None" );
    const QString fees = QgsServerProjectUtils::owsServiceFees( *project );
    if ( !fees.isEmpty() )
    {
      feesText = doc.createTextNode( fees );
    }
    feesElem.appendChild( feesText );
    serviceElem.appendChild( feesElem );

    QDomElement accessConstraintsElem = doc.createElement( QStringLiteral( "ows:AccessConstraints" ) );
    const QString accessConstraints = QgsServerProjectUtils::owsServiceAccessConstraints( *project );
    QDomText accessConstraintsText = doc.createTextNode( "None" );
    if ( !accessConstraints.isEmpty() )
    {
      accessConstraintsText = doc.createTextNode( accessConstraints );
    }
    accessConstraintsElem.appendChild( accessConstraintsText );
    serviceElem.appendChild( accessConstraintsElem );

    return serviceElem;

  }

  QDomElement getServiceProviderElement( QDomDocument &doc, const QgsProject *project )
  {
    //Service element
    QDomElement serviceElem = doc.createElement( QStringLiteral( "ows:ServiceProvider" ) );

    //ProviderName
    const QString contactOrganization = QgsServerProjectUtils::owsServiceContactOrganization( *project );
    if ( !contactOrganization.isEmpty() )
    {
      QDomElement providerNameElem = doc.createElement( QStringLiteral( "ows:ProviderName" ) );
      QDomText providerNameText = doc.createTextNode( contactOrganization );
      providerNameElem.appendChild( providerNameText );
      serviceElem.appendChild( providerNameElem );
    }

    const QString contactPerson = QgsServerProjectUtils::owsServiceContactPerson( *project );
    const QString contactPosition = QgsServerProjectUtils::owsServiceContactPosition( *project );
    if ( !contactPerson.isEmpty() ||
         !contactPosition.isEmpty() )
    {
      //Contact information
      QDomElement serviceContactElem = doc.createElement( QStringLiteral( "ows:ServiceContact" ) );

      if ( !contactPerson.isEmpty() )
      {
        QDomElement individualNameElem = doc.createElement( QStringLiteral( "ows:IndividualName" ) );
        QDomText individualNameText = doc.createTextNode( contactPerson );
        individualNameElem.appendChild( individualNameText );
        serviceContactElem.appendChild( individualNameElem );
      }

      if ( !contactPosition.isEmpty() )
      {
        QDomElement positionNameElem = doc.createElement( QStringLiteral( "ows:PositionName" ) );
        QDomText positionNameText = doc.createTextNode( contactPosition );
        positionNameElem.appendChild( positionNameText );
        serviceContactElem.appendChild( positionNameElem );
      }

      const QString contactMail = QgsServerProjectUtils::owsServiceContactMail( *project );
      const QString contactPhone = QgsServerProjectUtils::owsServiceContactPhone( *project );
      const QString onlineResource = QgsServerProjectUtils::owsServiceOnlineResource( *project );
      if ( !contactMail.isEmpty() ||
           !contactPhone.isEmpty() ||
           !onlineResource.isEmpty() )
      {
        //Contact information
        QDomElement contactInfoElem = doc.createElement( QStringLiteral( "ows:ContactInfo" ) );

        if ( !contactPhone.isEmpty() )
        {
          QDomElement phoneElem = doc.createElement( QStringLiteral( "ows:Phone" ) );
          QDomElement voiceElem = doc.createElement( QStringLiteral( "ows:Voice" ) );
          QDomText voiceText = doc.createTextNode( contactPhone );
          voiceElem.appendChild( voiceText );
          phoneElem.appendChild( voiceElem );
          contactInfoElem.appendChild( phoneElem );
        }

        if ( !contactMail.isEmpty() )
        {
          QDomElement addressElem = doc.createElement( QStringLiteral( "ows:Address" ) );
          QDomElement mailElem = doc.createElement( QStringLiteral( "ows:ElectronicMailAddress" ) );
          QDomText mailText = doc.createTextNode( contactMail );
          mailElem.appendChild( mailText );
          addressElem.appendChild( mailElem );
          contactInfoElem.appendChild( addressElem );
        }

        if ( !onlineResource.isEmpty() )
        {
          QDomElement onlineResourceElem = doc.createElement( QStringLiteral( "ows:OnlineResource" ) );
          onlineResourceElem.setAttribute( "xlink:href", onlineResource );
          contactInfoElem.appendChild( onlineResourceElem );
        }
      }

      QDomElement roleElem = doc.createElement( QStringLiteral( "ows:Role" ) );
      QDomText roleText = doc.createTextNode( "PointOfContact" );
      roleElem.appendChild( roleText );
      serviceContactElem.appendChild( roleElem );

      serviceElem.appendChild( serviceContactElem );
    }

    return serviceElem;

  }

  QDomElement getParameterElement( QDomDocument &doc, const QString &name, const QStringList &values )
  {
    QDomElement parameterElement = doc.createElement( QStringLiteral( "ows:Parameter" ) );
    parameterElement.setAttribute( QStringLiteral( "name" ), name );

    for ( const QString &v : values )
    {
      QDomElement valueElement = doc.createElement( QStringLiteral( "ows:Value" ) );
      QDomText valueText = doc.createTextNode( v );
      valueElement.appendChild( valueText );
      parameterElement.appendChild( valueElement );
    }

    return parameterElement;
  }

  QDomElement getOperationsMetadataElement( QDomDocument &doc, const QgsProject *project, const QgsServerRequest &request, const QgsServerSettings *settings )
  {
    QDomElement oprationsElement = doc.createElement( QStringLiteral( "ows:OperationsMetadata" ) );

    // Prepare url
    QString hrefString = serviceUrl( request, project, *settings );

    QDomElement operationElement = doc.createElement( QStringLiteral( "ows:Operation" ) );
    QDomElement dcpElement = doc.createElement( QStringLiteral( "ows:DCP" ) );
    QDomElement httpElement = doc.createElement( QStringLiteral( "ows:HTTP" ) );
    QDomElement getElement = doc.createElement( QStringLiteral( "ows:Get" ) );
    getElement.setAttribute( QStringLiteral( "xlink:href" ), hrefString );
    httpElement.appendChild( getElement );

    QDomElement postElement = doc.createElement( QStringLiteral( "ows:Post" ) );
    postElement.setAttribute( QStringLiteral( "xlink:href" ), hrefString );
    httpElement.appendChild( postElement );

    dcpElement.appendChild( httpElement );
    operationElement.appendChild( dcpElement );

    // GetCapabilities
    QDomElement getCapabilitiesElement = operationElement.cloneNode().toElement();
    getCapabilitiesElement.setAttribute( QStringLiteral( "name" ), QStringLiteral( "GetCapabilities" ) );
    // GetCapabilities service
    QDomElement serviceParameterElement = getParameterElement( doc, QStringLiteral( "service" ),
                                          QStringList() << QStringLiteral( "WFS" ) );
    getCapabilitiesElement.appendChild( serviceParameterElement );
    // GetCapabilities AcceptVersions
    QDomElement acceptVersionsParameterElement = getParameterElement( doc, QStringLiteral( "AcceptVersions" ),
        QStringList() << QStringLiteral( "1.1.0" ) << QStringLiteral( "1.0.0" ) );
    getCapabilitiesElement.appendChild( acceptVersionsParameterElement );
    // GetCapabilities AcceptFormats
    QDomElement acceptFormatsParameterElement = getParameterElement( doc, QStringLiteral( "AcceptFormats" ),
        QStringList() << QStringLiteral( "text/xml" ) );
    getCapabilitiesElement.appendChild( acceptFormatsParameterElement );
    // Add
    oprationsElement.appendChild( getCapabilitiesElement );

    // DescribeFeatureType
    QDomElement describeFeatureTypeElement = operationElement.cloneNode().toElement();
    describeFeatureTypeElement.setAttribute( QStringLiteral( "name" ), QStringLiteral( "DescribeFeatureType" ) );
    // DescribeFeatureType outputFormat
    QDomElement dftOutputFormatParameterElement = getParameterElement( doc, QStringLiteral( "outputFormat" ),
        QStringList() << QStringLiteral( "XMLSCHEMA" )
        << QStringLiteral( "text/xml; subtype=gml/2.1.2" )
        << QStringLiteral( "text/xml; subtype=gml/3.1.1" ) );
    describeFeatureTypeElement.appendChild( dftOutputFormatParameterElement );
    // Add
    oprationsElement.appendChild( describeFeatureTypeElement );

    // GetFeature
    QDomElement getFeatureElement = operationElement.cloneNode().toElement();
    getFeatureElement.setAttribute( QStringLiteral( "name" ), QStringLiteral( "GetFeature" ) );
    // GetFeature outputFormat
    QDomElement gfOutputFormatParameterElement = getParameterElement( doc, QStringLiteral( "outputFormat" ),
        QStringList() << QStringLiteral( "text/xml; subtype=gml/2.1.2" )
        << QStringLiteral( "text/xml; subtype=gml/3.1.1" )
        << QStringLiteral( "application/vnd.geo+json" ) );
    getFeatureElement.appendChild( gfOutputFormatParameterElement );
    // GetFeature resultType
    QDomElement resultTypeParameterElement = getParameterElement( doc, QStringLiteral( "resultType" ),
        QStringList() << QStringLiteral( "results" ) << QStringLiteral( "hits" ) );
    getFeatureElement.appendChild( resultTypeParameterElement );
    // Add
    oprationsElement.appendChild( getFeatureElement );

    // Transaction
    QDomElement transactionElement = operationElement.cloneNode().toElement();
    transactionElement.setAttribute( QStringLiteral( "name" ), QStringLiteral( "Transaction" ) );
    // GetFeature inputFormat
    QDomElement inputFormatParameterElement = getParameterElement( doc, QStringLiteral( "inputFormat" ),
        QStringList() << QStringLiteral( "text/xml; subtype=gml/2.1.2" )
        << QStringLiteral( "text/xml; subtype=gml/3.1.1" )
        << QStringLiteral( "application/vnd.geo+json" ) );
    transactionElement.appendChild( inputFormatParameterElement );
    // Add
    oprationsElement.appendChild( transactionElement );

    return oprationsElement;

  }

  QDomElement getFeatureTypeListElement( QDomDocument &doc, QgsServerInterface *serverIface, const QgsProject *project )
  {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
#else
    ( void )serverIface;
#endif

    //wfs:FeatureTypeList element
    QDomElement featureTypeListElement = doc.createElement( QStringLiteral( "FeatureTypeList" )/*wfs:FeatureTypeList*/ );
    //wfs:Operations element
    QDomElement operationsElement = doc.createElement( QStringLiteral( "Operations" )/*wfs:Operations*/ );
    featureTypeListElement.appendChild( operationsElement );
    //wfs:Query element
    QDomElement operationElement = doc.createElement( QStringLiteral( "Operation" ) );
    QDomText queryText = doc.createTextNode( "Query" );
    operationElement.appendChild( queryText );
    operationsElement.appendChild( operationElement );

    const QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *project );
    const QStringList wfstUpdateLayersId = QgsServerProjectUtils::wfstUpdateLayerIds( *project );
    const QStringList wfstInsertLayersId = QgsServerProjectUtils::wfstInsertLayerIds( *project );
    const QStringList wfstDeleteLayersId = QgsServerProjectUtils::wfstDeleteLayerIds( *project );
    for ( const QString &wfsLayerId : wfsLayerIds )
    {
      QgsMapLayer *layer = project->mapLayer( wfsLayerId );
      if ( !layer )
      {
        continue;
      }
      if ( layer->type() != QgsMapLayerType::VectorLayer )
      {
        continue;
      }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( accessControl && !accessControl->layerReadPermission( layer ) )
      {
        continue;
      }
#endif
      QDomElement layerElem = doc.createElement( QStringLiteral( "FeatureType" ) );

      //create Name
      QDomElement nameElem = doc.createElement( QStringLiteral( "Name" ) );
      QDomText nameText = doc.createTextNode( layerTypeName( layer ) );
      nameElem.appendChild( nameText );
      layerElem.appendChild( nameElem );

      //create Title
      QDomElement titleElem = doc.createElement( QStringLiteral( "Title" ) );
      QString title = layer->title();
      if ( title.isEmpty() )
      {
        title = layer->name();
      }
      QDomText titleText = doc.createTextNode( title );
      titleElem.appendChild( titleText );
      layerElem.appendChild( titleElem );

      //create Abstract
      QString abstract = layer->abstract();
      if ( !abstract.isEmpty() )
      {
        QDomElement abstractElem = doc.createElement( QStringLiteral( "Abstract" ) );
        QDomText abstractText = doc.createTextNode( abstract );
        abstractElem.appendChild( abstractText );
        layerElem.appendChild( abstractElem );
      }

      //create keywords
      QString keywords = layer->keywordList();
      if ( !keywords.isEmpty() )
      {
        QDomElement keywordsElem = doc.createElement( QStringLiteral( "ows:Keywords" ) );
        for ( const QString &keyword : keywords.split( ',' ) )
        {
          if ( !keyword.trimmed().isEmpty() )
          {
            QDomElement keywordElem = doc.createElement( QStringLiteral( "ows:Keyword" ) );
            QDomText keywordText = doc.createTextNode( keyword.trimmed() );
            keywordElem.appendChild( keywordText );
            keywordsElem.appendChild( keywordElem );
          }
        }
        layerElem.appendChild( keywordsElem );
      }

      //create DefaultSRS element
      const QString defaultSrs = layer->crs().authid();
      QDomElement srsElem = doc.createElement( QStringLiteral( "DefaultSRS" ) );
      QDomText srsText = doc.createTextNode( defaultSrs );
      srsElem.appendChild( srsText );
      layerElem.appendChild( srsElem );

      //create OtherSRS elements
      const QStringList outputCrsList = QgsServerProjectUtils::wmsOutputCrsList( *project );
      for ( const QString &crs : outputCrsList )
      {
        if ( crs == defaultSrs )
          continue;
        QDomElement otherSrsElem = doc.createElement( QStringLiteral( "OtherSRS" ) );
        QDomText otherSrsText = doc.createTextNode( crs );
        otherSrsElem.appendChild( otherSrsText );
        layerElem.appendChild( otherSrsElem );
      }

      //wfs:Operations element
      QDomElement operationsElement = doc.createElement( QStringLiteral( "Operations" )/*wfs:Operations*/ );
      //wfs:Query element
      QDomElement operationElement = doc.createElement( QStringLiteral( "Operation" ) );
      QDomText queryText = doc.createTextNode( QStringLiteral( "Query" ) );
      operationElement.appendChild( queryText );
      operationsElement.appendChild( operationElement );

      if ( wfstUpdateLayersId.contains( layer->id() ) ||
           wfstInsertLayersId.contains( layer->id() ) ||
           wfstDeleteLayersId.contains( layer->id() ) )
      {
        QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
        QgsVectorDataProvider *provider = vlayer->dataProvider();
        if ( ( provider->capabilities() & QgsVectorDataProvider::AddFeatures ) && wfstInsertLayersId.contains( layer->id() ) )
        {
          //wfs:Insert element
          QDomElement operationElement = doc.createElement( QStringLiteral( "Operation" ) );
          QDomText insertText = doc.createTextNode( QStringLiteral( "Insert" )/*wfs:Insert*/ );
          operationElement.appendChild( insertText );
          operationsElement.appendChild( operationElement );
        }

        if ( ( provider->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) &&
             ( provider->capabilities() & QgsVectorDataProvider::ChangeGeometries ) &&
             wfstUpdateLayersId.contains( layer->id() ) )
        {
          //wfs:Update element
          QDomElement operationElement = doc.createElement( QStringLiteral( "Operation" ) );
          QDomText updateText = doc.createTextNode( QStringLiteral( "Update" )/*wfs:Update*/ );
          operationElement.appendChild( updateText );
          operationsElement.appendChild( operationElement );
        }

        if ( ( provider->capabilities() & QgsVectorDataProvider::DeleteFeatures ) && wfstDeleteLayersId.contains( layer->id() ) )
        {
          //wfs:Delete element
          QDomElement operationElement = doc.createElement( QStringLiteral( "Operation" ) );
          QDomText deleteText = doc.createTextNode( QStringLiteral( "Delete" )/*wfs:Delete*/ );
          operationElement.appendChild( deleteText );
          operationsElement.appendChild( operationElement );
        }
      }

      layerElem.appendChild( operationsElement );

      //create WGS84BoundingBox
      QgsRectangle layerExtent = layer->extent();
      //transform the layers native CRS into WGS84
      QgsCoordinateReferenceSystem wgs84 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( geoEpsgCrsAuthId() );
      int wgs84precision = 6;
      QgsRectangle wgs84BoundingRect;
      if ( !layerExtent.isNull() )
      {
        QgsCoordinateTransform exGeoTransform( layer->crs(), wgs84, project );
        try
        {
          wgs84BoundingRect = exGeoTransform.transformBoundingBox( layerExtent );
        }
        catch ( const QgsCsException & )
        {
          wgs84BoundingRect = QgsRectangle();
        }
      }

      //create WGS84BoundingBox element
      QDomElement bBoxElement = doc.createElement( QStringLiteral( "ows:WGS84BoundingBox" ) );
      bBoxElement.setAttribute( QStringLiteral( "dimensions" ), QStringLiteral( "2" ) );
      QDomElement lCornerElement = doc.createElement( QStringLiteral( "ows:LowerCorner" ) );
      QDomText lCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( wgs84BoundingRect.xMinimum(), wgs84precision ), wgs84precision ) + " " + qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( wgs84BoundingRect.yMinimum(), wgs84precision ), wgs84precision ) );
      lCornerElement.appendChild( lCornerText );
      bBoxElement.appendChild( lCornerElement );
      QDomElement uCornerElement = doc.createElement( QStringLiteral( "ows:UpperCorner" ) );
      QDomText uCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( wgs84BoundingRect.xMaximum(), wgs84precision ), wgs84precision ) + " " + qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( wgs84BoundingRect.yMaximum(), wgs84precision ), wgs84precision ) );
      uCornerElement.appendChild( uCornerText );
      bBoxElement.appendChild( uCornerElement );
      layerElem.appendChild( bBoxElement );

      // layer metadata URL
      QString metadataUrl = layer->metadataUrl();
      if ( !metadataUrl.isEmpty() )
      {
        QDomElement metaUrlElem = doc.createElement( QStringLiteral( "MetadataURL" ) );
        QString metadataUrlType = layer->metadataUrlType();
        metaUrlElem.setAttribute( QStringLiteral( "type" ), metadataUrlType );
        QString metadataUrlFormat = layer->metadataUrlFormat();
        metaUrlElem.setAttribute( QStringLiteral( "format" ), metadataUrlFormat );
        QDomText metaUrlText = doc.createTextNode( metadataUrl );
        metaUrlElem.appendChild( metaUrlText );
        layerElem.appendChild( metaUrlElem );
      }

      featureTypeListElement.appendChild( layerElem );
    }

    return featureTypeListElement;
  }

} // namespace QgsWfs



