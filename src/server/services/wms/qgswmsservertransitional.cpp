/***************************************************************************
                              qgswmsservertransitional.cpp
                              -------------------
  begin                : May 14, 2006
  copyright            : (C) 2006 by Marco Hugentobler & Ionut Iosifescu Enescu
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgswmsservertransitional.h"
#include "qgsfilterrestorer.h"
#include "qgscapabilitiescache.h"
#include "qgscsexception.h"
#include "qgsfields.h"
#include "qgsfieldformatter.h"
#include "qgsfieldformatterregistry.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslegendrenderer.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaptopixel.h"
#include "qgsproject.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrasterlayer.h"
#include "qgsrasterrenderer.h"
#include "qgsscalecalculator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsmapserviceexception.h"
#include "qgssldconfigparser.h"
#include "qgssymbol.h"
#include "qgsrenderer.h"
#include "qgspaintenginehack.h"
#include "qgsogcutils.h"
#include "qgsfeature.h"
#include "qgseditorwidgetregistry.h"
#include "qgsaccesscontrol.h"
#include "qgsfeaturerequest.h"
#include "qgsmaprendererjobproxy.h"

#include <QImage>
#include <QPainter>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextStream>
#include <QDir>

//for printing
#include "qgscomposition.h"
#include <QBuffer>
#include <QPrinter>
#include <QSvgGenerator>
#include <QUrl>
#include <QPaintEngine>

namespace QgsWms
{

  QgsWmsServer::QgsWmsServer(
    const QString& configFilePath
    , const QgsServerSettings& settings
    , QMap<QString, QString> &parameters
    , QgsWmsConfigParser* cp
    , QgsAccessControl* accessControl
  )
      : mParameters( parameters )
      , mOwnsConfigParser( false )
      , mDrawLegendLayerLabel( true )
      , mDrawLegendItemLabel( true )
      , mConfigParser( cp )
      , mConfigFilePath( configFilePath )
      , mAccessControl( accessControl )
      , mSettings( settings )
  {
  }

  QgsWmsServer::QgsWmsServer()
      : mParameters( QMap<QString, QString>() )
      , mOwnsConfigParser( false )
      , mDrawLegendLayerLabel( true )
      , mDrawLegendItemLabel( true )
      , mConfigParser( nullptr )
      , mConfigFilePath( QString() )
      , mAccessControl( nullptr )
  {
  }

  QgsWmsServer::~QgsWmsServer()
  {
  }

  void QgsWmsServer::cleanupAfterRequest()
  {
    if ( mOwnsConfigParser )
    {
      delete mConfigParser;
      mConfigParser = nullptr;
      mOwnsConfigParser = false;
    }
  }

  void QgsWmsServer::appendFormats( QDomDocument &doc, QDomElement &elem, const QStringList &formats )
  {
    Q_FOREACH ( const QString& format, formats )
    {
      QDomElement formatElem = doc.createElement( QStringLiteral( "Format" )/*wms:Format*/ );
      formatElem.appendChild( doc.createTextNode( format ) );
      elem.appendChild( formatElem );
    }
  }

  QDomDocument QgsWmsServer::getCapabilities( const QString& version, bool fullProjectInformation )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Entering." ) );
    QDomDocument doc;
    QDomElement wmsCapabilitiesElement;

    //Prepare url
    QString hrefString;
    if ( mConfigParser )
    {
      hrefString = mConfigParser->serviceUrl();
    }
    if ( hrefString.isEmpty() )
    {
      hrefString = serviceUrl();
    }

    //href needs to be a prefix
    if ( !hrefString.endsWith( "?" ) && !hrefString.endsWith( "&" ) )
    {
      hrefString.append( hrefString.contains( "?" ) ? "&" : "?" );
    }

    if ( version == QLatin1String( "1.1.1" ) )
    {
      doc = QDomDocument( QStringLiteral( "WMT_MS_Capabilities SYSTEM 'http://schemas.opengis.net/wms/1.1.1/WMS_MS_Capabilities.dtd'" ) );  //WMS 1.1.1 needs DOCTYPE  "SYSTEM http://schemas.opengis.net/wms/1.1.1/WMS_MS_Capabilities.dtd"
      addXmlDeclaration( doc );
      wmsCapabilitiesElement = doc.createElement( QStringLiteral( "WMT_MS_Capabilities" )/*wms:WMS_Capabilities*/ );
    }
    else // 1.3.0 as default
    {
      addXmlDeclaration( doc );
      wmsCapabilitiesElement = doc.createElement( QStringLiteral( "WMS_Capabilities" )/*wms:WMS_Capabilities*/ );
      wmsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns" ), QStringLiteral( "http://www.opengis.net/wms" ) );
      wmsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:sld" ), QStringLiteral( "http://www.opengis.net/sld" ) );
      wmsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:qgs" ), QStringLiteral( "http://www.qgis.org/wms" ) );
      wmsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
      QString schemaLocation = QStringLiteral( "http://www.opengis.net/wms" );
      schemaLocation += QLatin1String( " http://schemas.opengis.net/wms/1.3.0/capabilities_1_3_0.xsd" );
      schemaLocation += QLatin1String( " http://www.opengis.net/sld" );
      schemaLocation += QLatin1String( " http://schemas.opengis.net/sld/1.1.0/sld_capabilities.xsd" );
      schemaLocation += QLatin1String( " http://www.qgis.org/wms" );
      if ( mConfigParser && mConfigParser->wmsInspireActivated() )
      {
        wmsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:inspire_common" ), QStringLiteral( "http://inspire.ec.europa.eu/schemas/common/1.0" ) );
        wmsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:inspire_vs" ), QStringLiteral( "http://inspire.ec.europa.eu/schemas/inspire_vs/1.0" ) );
        schemaLocation += QLatin1String( " http://inspire.ec.europa.eu/schemas/inspire_vs/1.0" );
        schemaLocation += QLatin1String( " http://inspire.ec.europa.eu/schemas/inspire_vs/1.0/inspire_vs.xsd" );
      }
      schemaLocation += " " + hrefString + "SERVICE=WMS&REQUEST=GetSchemaExtension";
      wmsCapabilitiesElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), schemaLocation );
    }
    wmsCapabilitiesElement.setAttribute( QStringLiteral( "version" ), version );
    doc.appendChild( wmsCapabilitiesElement );

//todo: add service capabilities
    if ( mConfigParser )
    {
      mConfigParser->serviceCapabilities( wmsCapabilitiesElement, doc );
    }

//wms:Capability element
    QDomElement capabilityElement = doc.createElement( QStringLiteral( "Capability" )/*wms:Capability*/ );
    wmsCapabilitiesElement.appendChild( capabilityElement );
//wms:Request element
    QDomElement requestElement = doc.createElement( QStringLiteral( "Request" )/*wms:Request*/ );
    capabilityElement.appendChild( requestElement );

    QDomElement dcpTypeElement = doc.createElement( QStringLiteral( "DCPType" )/*wms:DCPType*/ );
    QDomElement httpElement = doc.createElement( QStringLiteral( "HTTP" )/*wms:HTTP*/ );
    dcpTypeElement.appendChild( httpElement );

    QDomElement elem;

//wms:GetCapabilities
    elem = doc.createElement( QStringLiteral( "GetCapabilities" )/*wms:GetCapabilities*/ );
    appendFormats( doc, elem, QStringList() << ( version == QLatin1String( "1.1.1" ) ? "application/vnd.ogc.wms_xml" : "text/xml" ) );
    elem.appendChild( dcpTypeElement );
    requestElement.appendChild( elem );

// SOAP platform
//only give this information if it is not a WMS request to be in sync with the WMS capabilities schema
    QMap<QString, QString>::const_iterator service_it = mParameters.constFind( QStringLiteral( "SERVICE" ) );
    if ( service_it != mParameters.constEnd() && service_it.value().compare( QLatin1String( "WMS" ), Qt::CaseInsensitive ) != 0 )
    {
      QDomElement soapElement = doc.createElement( QStringLiteral( "SOAP" )/*wms:SOAP*/ );
      httpElement.appendChild( soapElement );
      QDomElement soapResourceElement = doc.createElement( QStringLiteral( "OnlineResource" )/*wms:OnlineResource*/ );
      soapResourceElement.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
      soapResourceElement.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
      soapResourceElement.setAttribute( QStringLiteral( "xlink:href" ), hrefString );
      soapElement.appendChild( soapResourceElement );
    }

//only Get supported for the moment
    QDomElement getElement = doc.createElement( QStringLiteral( "Get" )/*wms:Get*/ );
    httpElement.appendChild( getElement );
    QDomElement olResourceElement = doc.createElement( QStringLiteral( "OnlineResource" )/*wms:OnlineResource*/ );
    olResourceElement.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
    olResourceElement.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
    olResourceElement.setAttribute( QStringLiteral( "xlink:href" ), hrefString );
    getElement.appendChild( olResourceElement );

//wms:GetMap
    elem = doc.createElement( QStringLiteral( "GetMap" )/*wms:GetMap*/ );
    appendFormats( doc, elem, QStringList() << QStringLiteral( "image/jpeg" ) << QStringLiteral( "image/png" ) << QStringLiteral( "image/png; mode=16bit" ) << QStringLiteral( "image/png; mode=8bit" ) << QStringLiteral( "image/png; mode=1bit" ) << QStringLiteral( "application/dxf" ) );
    elem.appendChild( dcpTypeElement.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElement.appendChild( elem );

//wms:GetFeatureInfo
    elem = doc.createElement( QStringLiteral( "GetFeatureInfo" ) );
    appendFormats( doc, elem, QStringList() << QStringLiteral( "text/plain" ) << QStringLiteral( "text/html" ) << QStringLiteral( "text/xml" ) << QStringLiteral( "application/vnd.ogc.gml" ) << QStringLiteral( "application/vnd.ogc.gml/3.1.1" ) );
    elem.appendChild( dcpTypeElement.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElement.appendChild( elem );

//wms:GetLegendGraphic
    elem = doc.createElement(( version == QLatin1String( "1.1.1" ) ? "GetLegendGraphic" : "sld:GetLegendGraphic" )/*wms:GetLegendGraphic*/ );
    appendFormats( doc, elem, QStringList() << QStringLiteral( "image/jpeg" ) << QStringLiteral( "image/png" ) );
    elem.appendChild( dcpTypeElement.cloneNode().toElement() ); // this is the same as for 'GetCapabilities'
    requestElement.appendChild( elem );

//wms:DescribeLayer
    elem = doc.createElement(( version == QLatin1String( "1.1.1" ) ? "DescribeLayer" : "sld:DescribeLayer" )/*wms:GetLegendGraphic*/ );
    appendFormats( doc, elem, QStringList() << QStringLiteral( "text/xml" ) );
    elem.appendChild( dcpTypeElement.cloneNode().toElement() ); // this is the same as for 'GetCapabilities'
    requestElement.appendChild( elem );

//wms:GetStyles
    elem = doc.createElement(( version == QLatin1String( "1.1.1" ) ? "GetStyles" : "qgs:GetStyles" )/*wms:GetStyles*/ );
    appendFormats( doc, elem, QStringList() << QStringLiteral( "text/xml" ) );
    elem.appendChild( dcpTypeElement.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElement.appendChild( elem );

    if ( fullProjectInformation ) //remove composer templates from GetCapabilities in the long term
    {
      //wms:GetPrint
      elem = doc.createElement( QStringLiteral( "GetPrint" ) /*wms:GetPrint*/ );
      appendFormats( doc, elem, QStringList() << QStringLiteral( "svg" ) << QStringLiteral( "png" ) << QStringLiteral( "pdf" ) );
      elem.appendChild( dcpTypeElement.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
      requestElement.appendChild( elem );
    }

//Exception element is mandatory
    elem = doc.createElement( QStringLiteral( "Exception" ) );
    appendFormats( doc, elem, QStringList() << ( version == QLatin1String( "1.1.1" ) ? "application/vnd.ogc.se_xml" : "XML" ) );
    capabilityElement.appendChild( elem );

//UserDefinedSymbolization element
    if ( version == QLatin1String( "1.3.0" ) )
    {
      elem = doc.createElement( QStringLiteral( "sld:UserDefinedSymbolization" ) );
      elem.setAttribute( QStringLiteral( "SupportSLD" ), QStringLiteral( "1" ) );
      elem.setAttribute( QStringLiteral( "UserLayer" ), QStringLiteral( "0" ) );
      elem.setAttribute( QStringLiteral( "UserStyle" ), QStringLiteral( "1" ) );
      elem.setAttribute( QStringLiteral( "RemoteWFS" ), QStringLiteral( "0" ) );
      elem.setAttribute( QStringLiteral( "InlineFeature" ), QStringLiteral( "0" ) );
      elem.setAttribute( QStringLiteral( "RemoteWCS" ), QStringLiteral( "0" ) );
      capabilityElement.appendChild( elem );

      if ( mConfigParser && mConfigParser->wmsInspireActivated() )
      {
        mConfigParser->inspireCapabilities( capabilityElement, doc );
      }
    }

    if ( mConfigParser && fullProjectInformation ) //remove composer templates from GetCapabilities in the long term
    {
      //Insert <ComposerTemplate> elements derived from wms:_ExtendedCapabilities
      mConfigParser->printCapabilities( capabilityElement, doc );
    }

    if ( mConfigParser && fullProjectInformation )
    {
      //WFS layers
      QStringList wfsLayers = mConfigParser->wfsLayerNames();
      if ( !wfsLayers.isEmpty() )
      {
        QDomElement wfsLayersElem = doc.createElement( QStringLiteral( "WFSLayers" ) );
        QStringList::const_iterator wfsIt = wfsLayers.constBegin();
        for ( ; wfsIt != wfsLayers.constEnd(); ++wfsIt )
        {
          QDomElement wfsLayerElem = doc.createElement( QStringLiteral( "WFSLayer" ) );
          wfsLayerElem.setAttribute( QStringLiteral( "name" ), *wfsIt );
          wfsLayersElem.appendChild( wfsLayerElem );
        }
        capabilityElement.appendChild( wfsLayersElem );
      }
    }

//add the xml content for the individual layers/styles
    QgsMessageLog::logMessage( QStringLiteral( "calling layersAndStylesCapabilities" ) );
    if ( mConfigParser )
    {
      mConfigParser->layersAndStylesCapabilities( capabilityElement, doc, version, fullProjectInformation );
    }
    QgsMessageLog::logMessage( QStringLiteral( "layersAndStylesCapabilities returned" ) );

#if 0
//for debugging: save the document to disk
    QFile capabilitiesFile( QDir::tempPath() + "/capabilities.txt" );
    if ( capabilitiesFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      QTextStream capabilitiesStream( &capabilitiesFile );
      doc.save( capabilitiesStream, 4 );
    }
#endif

    return doc;
  }

  QDomDocument QgsWmsServer::getContext()
  {
    QDomDocument doc;
    addXmlDeclaration( doc );
    QDomElement owsContextElem = doc.createElement( QStringLiteral( "OWSContext" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns" ), QStringLiteral( "http://www.opengis.net/ows-context" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:ows-context" ), QStringLiteral( "http://www.opengis.net/ows-context" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:context" ), QStringLiteral( "http://www.opengis.net/context" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:ows" ), QStringLiteral( "http://www.opengis.net/ows" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:sld" ), QStringLiteral( "http://www.opengis.net/sld" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:ogc" ), QStringLiteral( "http://www.opengis.net/ogc" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:gml" ), QStringLiteral( "http://www.opengis.net/gml" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:kml" ), QStringLiteral( "http://www.opengis.net/kml/2.2" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:ns9" ), QStringLiteral( "http://www.w3.org/2005/Atom" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:xal" ), QStringLiteral( "urn:oasis:names:tc:ciq:xsdschema:xAL:2.0" ) );
    owsContextElem.setAttribute( QStringLiteral( "xmlns:ins" ), QStringLiteral( "http://www.inspire.org" ) );
    owsContextElem.setAttribute( QStringLiteral( "version" ), QStringLiteral( "0.3.1" ) );
    doc.appendChild( owsContextElem );

    if ( mConfigParser )
    {
      //Prepare url
      QString hrefString = mConfigParser->serviceUrl();
      if ( hrefString.isEmpty() )
      {
        hrefString = serviceUrl();
      }
      mConfigParser->owsGeneralAndResourceList( owsContextElem, doc, hrefString );
    }

    return doc;
  }


  static QgsLayerTreeModelLegendNode* _findLegendNodeForRule( QgsLayerTreeModel* legendModel, const QString& rule )
  {
    Q_FOREACH ( QgsLayerTreeLayer* nodeLayer, legendModel->rootGroup()->findLayers() )
    {
      Q_FOREACH ( QgsLayerTreeModelLegendNode* legendNode, legendModel->layerLegendNodes( nodeLayer ) )
      {
        if ( legendNode->data( Qt::DisplayRole ).toString() == rule )
          return legendNode;
      }
    }
    return nullptr;
  }


  static QgsRectangle _parseBBOX( const QString &bboxStr, bool &ok )
  {
    ok = false;

    QStringList lst = bboxStr.split( QStringLiteral( "," ) );
    if ( lst.count() != 4 )
      return QgsRectangle();

    double d[4];
    for ( int i = 0; i < 4; i++ )
    {
      bool ok;
      lst[i].replace( QLatin1String( " " ), QLatin1String( "+" ) );
      d[i] = lst[i].toDouble( &ok );
      if ( !ok )
        return QgsRectangle();
    }

    ok = true;
    if ( d[2] <= d[0] || d[3] <= d[1] )
    {
      throw QgsMapServiceException( "InvalidParameterValue", "BBOX is empty" );
    }
    return QgsRectangle( d[0], d[1], d[2], d[3] );
  }


  QImage* QgsWmsServer::getLegendGraphics()
  {
    if ( !mConfigParser )
    {
      throw QgsException( QStringLiteral( "No config parser" ) );
    }
    if ( !mParameters.contains( QStringLiteral( "LAYER" ) ) && !mParameters.contains( QStringLiteral( "LAYERS" ) ) )
    {
      throw QgsMapServiceException( QStringLiteral( "LayerNotSpecified" ), QStringLiteral( "LAYER is mandatory for GetLegendGraphic operation" ) );
    }
    if ( !mParameters.contains( QStringLiteral( "FORMAT" ) ) )
    {
      throw QgsMapServiceException( QStringLiteral( "FormatNotSpecified" ), QStringLiteral( "FORMAT is mandatory for GetLegendGraphic operation" ) );
    }

    bool contentBasedLegend = false;
    QgsRectangle contentBasedLegendExtent;

    if ( mParameters.contains( QStringLiteral( "BBOX" ) ) )
    {
      contentBasedLegend = true;

      bool bboxOk;
      contentBasedLegendExtent = _parseBBOX( mParameters[QStringLiteral( "BBOX" )], bboxOk );
      if ( !bboxOk || contentBasedLegendExtent.isEmpty() )
        throw QgsMapServiceException( QStringLiteral( "InvalidParameterValue" ), QStringLiteral( "Invalid BBOX parameter" ) );

      if ( mParameters.contains( QStringLiteral( "RULE" ) ) )
        throw QgsMapServiceException( QStringLiteral( "InvalidParameterValue" ), QStringLiteral( "BBOX parameter cannot be combined with RULE" ) );
    }

    QStringList layersList, stylesList;

    readLayersAndStyles( layersList, stylesList );

    if ( layersList.size() < 1 )
    {
      return nullptr;
    }

    //scale
    double scaleDenominator = -1;
    QMap<QString, QString>::const_iterator scaleIt = mParameters.constFind( QStringLiteral( "SCALE" ) );
    if ( scaleIt != mParameters.constEnd() )
    {
      bool conversionSuccess;
      double scaleValue = scaleIt.value().toDouble( &conversionSuccess );
      if ( conversionSuccess )
      {
        scaleDenominator = scaleValue;
      }
    }

    QgsCoordinateReferenceSystem dummyCRS;
    QStringList layerIds = layerSet( layersList, stylesList, dummyCRS, scaleDenominator );
    if ( layerIds.size() < 1 )
    {
      return nullptr;
    }

    //get icon size, spaces between legend items and font from config parser
    double boxSpace, layerSpace, layerTitleSpace, symbolSpace, iconLabelSpace, symbolWidth, symbolHeight;
    QFont layerFont, itemFont;
    QColor layerFontColor, itemFontColor;
    legendParameters( boxSpace, layerSpace, layerTitleSpace, symbolSpace,
                      iconLabelSpace, symbolWidth, symbolHeight, layerFont, itemFont, layerFontColor, itemFontColor );

    QString rule;
    int ruleSymbolWidth = 0, ruleSymbolHeight = 0;
    QMap<QString, QString>::const_iterator ruleIt = mParameters.constFind( QStringLiteral( "RULE" ) );
    if ( ruleIt != mParameters.constEnd() )
    {
      rule = ruleIt.value();

      QMap<QString, QString>::const_iterator widthIt = mParameters.constFind( QStringLiteral( "WIDTH" ) );
      if ( widthIt != mParameters.constEnd() )
      {
        bool conversionSuccess;
        double width = widthIt.value().toDouble( &conversionSuccess );
        if ( conversionSuccess )
        {
          ruleSymbolWidth = width;
        }
      }

      QMap<QString, QString>::const_iterator heightIt = mParameters.constFind( QStringLiteral( "HEIGHT" ) );
      if ( heightIt != mParameters.constEnd() )
      {
        bool conversionSuccess;
        double width = heightIt.value().toDouble( &conversionSuccess );
        if ( conversionSuccess )
        {
          ruleSymbolHeight = width;
        }
      }
    }

    // Checks showFeatureCount parameter
    bool showFeatureCount = false;
    if ( mParameters.contains( QStringLiteral( "SHOWFEATURECOUNT" ) ) )
      showFeatureCount = QVariant( mParameters[ QStringLiteral( "SHOWFEATURECOUNT" )] ).toBool();

    // Create the layer tree root
    QgsLayerTreeGroup rootGroup;
    // Store layers' name to reset them
    QMap<QString, QString> layerNameMap;
    // Create tree layer node for each layer
    Q_FOREACH ( const QString& layerId, layerIds )
    {
      // get layer
      QgsMapLayer *ml = QgsProject::instance()->mapLayer( layerId );
      // create tree layer node
      QgsLayerTreeLayer *layer = rootGroup.addLayer( ml );
      // store the layer's name
      layerNameMap.insert( layerId, ml->name() );
      // set layer name with layer's title to have it in legend
      if ( !ml->title().isEmpty() )
        layer->setName( ml->title() );
      // set show feature count
      if ( showFeatureCount )
        layer->setCustomProperty( QStringLiteral( "showFeatureCount" ), showFeatureCount );
    }
    QgsLayerTreeModel legendModel( &rootGroup );

    if ( scaleDenominator > 0 )
      legendModel.setLegendFilterByScale( scaleDenominator );

    QgsMapSettings contentBasedMapSettings;
    if ( contentBasedLegend )
    {
      HitTest hitTest;
      getMap( contentBasedMapSettings, &hitTest );

      Q_FOREACH ( QgsLayerTreeNode* node, rootGroup.children() )
      {
        Q_ASSERT( QgsLayerTree::isLayer( node ) );
        QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );

        QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( nodeLayer->layer() );
        if ( !vl || !vl->renderer() )
          continue;

        const SymbolSet& usedSymbols = hitTest[vl];
        QList<int> order;
        int i = 0;
        Q_FOREACH ( const QgsLegendSymbolItem& legendItem, vl->renderer()->legendSymbolItemsV2() )
        {
          if ( usedSymbols.contains( legendItem.legacyRuleKey() ) )
            order.append( i );
          ++i;
        }

        // either remove the whole layer or just filter out some items
        if ( order.isEmpty() )
          rootGroup.removeChildNode( nodeLayer );
        else
        {
          QgsMapLayerLegendUtils::setLegendNodeOrder( nodeLayer, order );
          legendModel.refreshLayerLegend( nodeLayer );
        }
      }
    }

    // find out DPI
    QImage* tmpImage = createImage( 1, 1, false );
    if ( !tmpImage )
      return nullptr;
    qreal dpmm = tmpImage->dotsPerMeterX() / 1000.0;
    delete tmpImage;

    // setup legend configuration
    QgsLegendSettings legendSettings;
    legendSettings.setTitle( QString() );
    legendSettings.setBoxSpace( boxSpace );
    legendSettings.rstyle( QgsLegendStyle::Subgroup ).setMargin( QgsLegendStyle::Top, layerSpace );
    // TODO: not available: layer title space
    legendSettings.rstyle( QgsLegendStyle::Symbol ).setMargin( QgsLegendStyle::Top, symbolSpace );
    legendSettings.rstyle( QgsLegendStyle::SymbolLabel ).setMargin( QgsLegendStyle::Left, iconLabelSpace );
    legendSettings.setSymbolSize( QSizeF( symbolWidth, symbolHeight ) );
    legendSettings.rstyle( QgsLegendStyle::Subgroup ).setFont( layerFont );
    legendSettings.rstyle( QgsLegendStyle::SymbolLabel ).setFont( itemFont );
    // TODO: not available: layer font color
    legendSettings.setFontColor( itemFontColor );

    if ( contentBasedLegend )
    {
      legendSettings.setMapScale( contentBasedMapSettings.scale() );
    }

    if ( !rule.isEmpty() )
    {
      //create second image with the right dimensions
      QImage* paintImage = createImage( ruleSymbolWidth, ruleSymbolHeight, false );

      //go through the items a second time for painting
      QPainter p( paintImage );
      p.setRenderHint( QPainter::Antialiasing, true );
      p.scale( dpmm, dpmm );

      QgsLayerTreeModelLegendNode* legendNode = _findLegendNodeForRule( &legendModel, rule );
      if ( legendNode )
      {
        QgsLayerTreeModelLegendNode::ItemContext ctx;
        ctx.painter = &p;
        ctx.labelXOffset = 0;
        ctx.point = QPointF();
        double itemHeight = ruleSymbolHeight / dpmm;
        legendNode->drawSymbol( legendSettings, &ctx, itemHeight );
      }

      QgsProject::instance()->removeAllMapLayers();
      return paintImage;
    }

    QList<QgsLayerTreeNode*> rootChildren = rootGroup.children();
    Q_FOREACH ( QgsLayerTreeNode* node, rootChildren )
    {
      if ( QgsLayerTree::isLayer( node ) )
      {
        QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
        if ( !mAccessControl->layerReadPermission( nodeLayer->layer() ) )
        {
          throw QgsMapServiceException( QStringLiteral( "Security" ), "You are not allowed to access to the layer: " + nodeLayer->layer()->name() );
        }
#endif

        // layer titles - hidden or not
        QgsLegendRenderer::setNodeLegendStyle( nodeLayer, mDrawLegendLayerLabel ? QgsLegendStyle::Subgroup : QgsLegendStyle::Hidden );

        // rule item titles
        if ( !mDrawLegendItemLabel )
        {
          Q_FOREACH ( QgsLayerTreeModelLegendNode* legendNode, legendModel.layerLegendNodes( nodeLayer ) )
          {
            legendNode->setUserLabel( QStringLiteral( " " ) ); // empty string = no override, so let's use one space
          }
        }
        else if ( !mDrawLegendLayerLabel )
        {
          Q_FOREACH ( QgsLayerTreeModelLegendNode* legendNode, legendModel.layerLegendNodes( nodeLayer ) )
          {
            if ( legendNode->isEmbeddedInParent() )
              legendNode->setEmbeddedInParent( false );
          }
        }
      }
    }

    QgsLegendRenderer legendRenderer( &legendModel, legendSettings );
    QSizeF minSize = legendRenderer.minimumSize();
    QSize s( minSize.width() * dpmm, minSize.height() * dpmm );

    QImage* paintImage = createImage( s.width(), s.height(), false );

    QPainter p( paintImage );
    p.setRenderHint( QPainter::Antialiasing, true );
    p.scale( dpmm, dpmm );

    legendRenderer.drawLegend( &p );

    p.end();

    // reset layers' name
    Q_FOREACH ( const QString& layerId, layerIds )
    {
      QgsMapLayer *ml = QgsProject::instance()->mapLayer( layerId );
      ml->setName( layerNameMap[ layerId ] );
    }
    //  clear map layer registry
    QgsProject::instance()->removeAllMapLayers();
    return paintImage;
  }


  void QgsWmsServer::runHitTest( const QgsMapSettings& mapSettings, HitTest& hitTest )
  {
    QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );

    Q_FOREACH ( const QString& layerID, mapSettings.layerIds() )
    {
      QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsProject::instance()->mapLayer( layerID ) );
      if ( !vl || !vl->renderer() )
        continue;

      if ( vl->hasScaleBasedVisibility() && vl->isInScaleRange( mapSettings.scale() ) )
      {
        hitTest[vl] = SymbolSet(); // no symbols -> will not be shown
        continue;
      }

      if ( mapSettings.hasCrsTransformEnabled() )
      {
        QgsCoordinateTransform tr = mapSettings.layerTransform( vl );
        context.setCoordinateTransform( tr );
        context.setExtent( tr.transformBoundingBox( mapSettings.extent(), QgsCoordinateTransform::ReverseTransform ) );
      }

      SymbolSet& usedSymbols = hitTest[vl];
      runHitTestLayer( vl, usedSymbols, context );
    }
  }

  void QgsWmsServer::runHitTestLayer( QgsVectorLayer* vl, SymbolSet& usedSymbols, QgsRenderContext& context )
  {
    QgsFeatureRenderer* r = vl->renderer();
    bool moreSymbolsPerFeature = r->capabilities() & QgsFeatureRenderer::MoreSymbolsPerFeature;
    r->startRender( context, vl->pendingFields() );
    QgsFeature f;
    QgsFeatureRequest request( context.extent() );
    request.setFlags( QgsFeatureRequest::ExactIntersect );
    QgsFeatureIterator fi = vl->getFeatures( request );
    while ( fi.nextFeature( f ) )
    {
      context.expressionContext().setFeature( f );
      if ( moreSymbolsPerFeature )
      {
        Q_FOREACH ( QgsSymbol* s, r->originalSymbolsForFeature( f, context ) )
          usedSymbols.insert( s );
      }
      else
        usedSymbols.insert( r->originalSymbolForFeature( f, context ) );
    }
    r->stopRender( context );
  }


  void QgsWmsServer::legendParameters( double& boxSpace, double& layerSpace, double& layerTitleSpace,
                                       double& symbolSpace, double& iconLabelSpace, double& symbolWidth, double& symbolHeight,
                                       QFont& layerFont, QFont& itemFont, QColor& layerFontColor, QColor& itemFontColor )
  {
    //spaces between legend elements
    QMap<QString, QString>::const_iterator boxSpaceIt = mParameters.constFind( QStringLiteral( "BOXSPACE" ) );
    boxSpace = ( boxSpaceIt == mParameters.constEnd() ) ? mConfigParser->legendBoxSpace() : boxSpaceIt.value().toDouble();
    QMap<QString, QString>::const_iterator layerSpaceIt = mParameters.constFind( QStringLiteral( "LAYERSPACE" ) );
    layerSpace = ( layerSpaceIt == mParameters.constEnd() ) ? mConfigParser->legendLayerSpace() : layerSpaceIt.value().toDouble();
    QMap<QString, QString>::const_iterator layerTitleSpaceIt = mParameters.constFind( QStringLiteral( "LAYERTITLESPACE" ) );
    layerTitleSpace = ( layerTitleSpaceIt == mParameters.constEnd() ) ? mConfigParser->legendLayerTitleSpace() : layerTitleSpaceIt.value().toDouble();
    QMap<QString, QString>::const_iterator symbolSpaceIt = mParameters.constFind( QStringLiteral( "SYMBOLSPACE" ) );
    symbolSpace = ( symbolSpaceIt == mParameters.constEnd() ) ? mConfigParser->legendSymbolSpace() : symbolSpaceIt.value().toDouble();
    QMap<QString, QString>::const_iterator iconLabelSpaceIt = mParameters.constFind( QStringLiteral( "ICONLABELSPACE" ) );
    iconLabelSpace = ( iconLabelSpaceIt == mParameters.constEnd() ) ? mConfigParser->legendIconLabelSpace() : iconLabelSpaceIt.value().toDouble();
    QMap<QString, QString>::const_iterator symbolWidthIt = mParameters.constFind( QStringLiteral( "SYMBOLWIDTH" ) );
    symbolWidth = ( symbolWidthIt == mParameters.constEnd() ) ? mConfigParser->legendSymbolWidth() : symbolWidthIt.value().toDouble();
    QMap<QString, QString>::const_iterator symbolHeightIt = mParameters.constFind( QStringLiteral( "SYMBOLHEIGHT" ) );
    symbolHeight = ( symbolHeightIt == mParameters.constEnd() ) ? mConfigParser->legendSymbolHeight() : symbolHeightIt.value().toDouble();

    //font properties
    layerFont = mConfigParser->legendLayerFont();
    QMap<QString, QString>::const_iterator layerFontFamilyIt = mParameters.constFind( QStringLiteral( "LAYERFONTFAMILY" ) );
    if ( layerFontFamilyIt != mParameters.constEnd() )
    {
      layerFont.setFamily( layerFontFamilyIt.value() );
    }
    QMap<QString, QString>::const_iterator layerFontBoldIt = mParameters.constFind( QStringLiteral( "LAYERFONTBOLD" ) );
    if ( layerFontBoldIt != mParameters.constEnd() )
    {
      layerFont.setBold( layerFontBoldIt.value().compare( QLatin1String( "TRUE" ), Qt::CaseInsensitive ) == 0 );
    }
    QMap<QString, QString>::const_iterator layerFontItalicIt = mParameters.constFind( QStringLiteral( "LAYERFONTITALIC" ) );
    if ( layerFontItalicIt != mParameters.constEnd() )
    {
      layerFont.setItalic( layerFontItalicIt.value().compare( QLatin1String( "TRUE" ), Qt::CaseInsensitive ) == 0 );
    }
    QMap<QString, QString>::const_iterator layerFontSizeIt = mParameters.constFind( QStringLiteral( "LAYERFONTSIZE" ) );
    layerFont.setPointSizeF( layerFontSizeIt != mParameters.constEnd() ? layerFontSizeIt.value().toDouble() : layerFont.pointSizeF() );
    QMap<QString, QString>::const_iterator layerFontColorIt = mParameters.constFind( QStringLiteral( "LAYERFONTCOLOR" ) );
    if ( layerFontColorIt != mParameters.constEnd() )
    {
      layerFontColor.setNamedColor( layerFontColorIt.value() );
    }
    else
    {
      layerFontColor = QColor( 0, 0, 0 );
    }
    QMap<QString, QString>::const_iterator layerTitleIt = mParameters.constFind( QStringLiteral( "LAYERTITLE" ) );
    if ( layerTitleIt != mParameters.constEnd() )
    {
      mDrawLegendLayerLabel = ( layerTitleIt.value().compare( QLatin1String( "TRUE" ), Qt::CaseInsensitive ) == 0 );
    }
    else
    {
      mDrawLegendLayerLabel = true;
    }


    itemFont = mConfigParser->legendItemFont();
    QMap<QString, QString>::const_iterator itemFontFamilyIt = mParameters.constFind( QStringLiteral( "ITEMFONTFAMILY" ) );
    if ( itemFontFamilyIt != mParameters.constEnd() )
    {
      itemFont.setFamily( itemFontFamilyIt.value() );
    }
    QMap<QString, QString>::const_iterator itemFontBoldIt = mParameters.constFind( QStringLiteral( "ITEMFONTBOLD" ) );
    if ( itemFontBoldIt != mParameters.constEnd() )
    {
      itemFont.setBold( itemFontBoldIt.value().compare( QLatin1String( "TRUE" ), Qt::CaseInsensitive ) == 0 );
    }
    QMap<QString, QString>::const_iterator itemFontItalicIt = mParameters.constFind( QStringLiteral( "ITEMFONTITALIC" ) );
    if ( itemFontItalicIt != mParameters.constEnd() )
    {
      itemFont.setItalic( itemFontItalicIt.value().compare( QLatin1String( "TRUE" ), Qt::CaseInsensitive ) == 0 );
    }
    QMap<QString, QString>::const_iterator itemFontSizeIt = mParameters.constFind( QStringLiteral( "ITEMFONTSIZE" ) );
    itemFont.setPointSizeF( itemFontSizeIt != mParameters.constEnd() ? itemFontSizeIt.value().toDouble() : itemFont.pointSizeF() );
    QMap<QString, QString>::const_iterator itemFontColorIt = mParameters.constFind( QStringLiteral( "ITEMFONTCOLOR" ) );
    if ( itemFontColorIt != mParameters.constEnd() )
    {
      itemFontColor.setNamedColor( itemFontColorIt.value() );
    }
    else
    {
      itemFontColor = QColor( 0, 0, 0 );
    }
    QMap<QString, QString>::const_iterator itemLabelIt = mParameters.constFind( QStringLiteral( "RULELABEL" ) );
    if ( itemLabelIt != mParameters.constEnd() )
    {
      mDrawLegendItemLabel = ( itemLabelIt.value().compare( QLatin1String( "TRUE" ), Qt::CaseInsensitive ) == 0 );
    }
    else
    {
      mDrawLegendItemLabel = true;
    }
  }

  QDomDocument QgsWmsServer::getSchemaExtension()
  {
    QDomDocument xsdDoc;

    QFileInfo xsdFileInfo( QStringLiteral( "schemaExtension.xsd" ) );
    if ( !xsdFileInfo.exists() )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Error, xsd file 'schemaExtension.xsd' does not exist" ), QStringLiteral( "Server" ), QgsMessageLog::CRITICAL );
      return xsdDoc;
    }

    QString xsdFilePath = xsdFileInfo.absoluteFilePath();
    QFile xsdFile( xsdFilePath );
    if ( !xsdFile.exists() )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Error, xsd file 'schemaExtension.xsd' does not exist" ), QStringLiteral( "Server" ), QgsMessageLog::CRITICAL );
      return xsdDoc;
    }

    if ( !xsdFile.open( QIODevice::ReadOnly ) )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Error, cannot open xsd file 'schemaExtension.xsd' does not exist" ), QStringLiteral( "Server" ), QgsMessageLog::CRITICAL );
      return xsdDoc;
    }

    QString errorMsg;
    int line, column;
    if ( !xsdDoc.setContent( &xsdFile, true, &errorMsg, &line, &column ) )
    {
      QgsMessageLog::logMessage( "Error parsing file 'schemaExtension.xsd" +
                                 QStringLiteral( "': parse error %1 at row %2, column %3" ).arg( errorMsg ).arg( line ).arg( column ), QStringLiteral( "Server" ), QgsMessageLog::CRITICAL );
      return xsdDoc;
    }
    return xsdDoc;
  }

  QDomDocument QgsWmsServer::getStyle()
  {
    QDomDocument doc;
    if ( !mParameters.contains( QStringLiteral( "STYLE" ) ) )
    {
      throw QgsMapServiceException( QStringLiteral( "StyleNotSpecified" ), QStringLiteral( "Style is mandatory for GetStyle operation" ) );
    }

    if ( !mParameters.contains( QStringLiteral( "LAYER" ) ) )
    {
      throw QgsMapServiceException( QStringLiteral( "LayerNotSpecified" ), QStringLiteral( "Layer is mandatory for GetStyle operation" ) );
    }

    QString styleName = mParameters[ QStringLiteral( "STYLE" )];
    QString layerName = mParameters[ QStringLiteral( "LAYER" )];

    return mConfigParser->getStyle( styleName, layerName );
  }

// GetStyles is defined for WMS1.1.1/SLD1.0 and in WMS 1.3.0 SLD Extension
  QDomDocument QgsWmsServer::getStyles()
  {
    QDomDocument doc;
    if ( !mParameters.contains( QStringLiteral( "LAYERS" ) ) )
    {
      throw QgsMapServiceException( QStringLiteral( "LayerNotSpecified" ), QStringLiteral( "Layers is mandatory for GetStyles operation" ) );
    }

    QStringList layersList = mParameters[ QStringLiteral( "LAYERS" )].split( QStringLiteral( "," ), QString::SkipEmptyParts );
    if ( layersList.size() < 1 )
    {
      throw QgsMapServiceException( QStringLiteral( "LayerNotSpecified" ), QStringLiteral( "Layers is mandatory for GetStyles operation" ) );
    }

    return mConfigParser->getStyles( layersList );
  }

// DescribeLayer is defined for WMS1.1.1/SLD1.0 and in WMS 1.3.0 SLD Extension
  QDomDocument QgsWmsServer::describeLayer()
  {
    if ( !mParameters.contains( QStringLiteral( "SLD_VERSION" ) ) )
    {
      throw QgsMapServiceException( QStringLiteral( "MissingParameterValue" ), QStringLiteral( "SLD_VERSION is mandatory for DescribeLayer operation" ) );
    }
    if ( mParameters[ QStringLiteral( "SLD_VERSION" )] != QLatin1String( "1.1.0" ) )
    {
      throw QgsMapServiceException( QStringLiteral( "InvalidParameterValue" ), QStringLiteral( "SLD_VERSION = %1 is not supported" ).arg( mParameters[ QStringLiteral( "SLD_VERSION" )] ) );
    }

    if ( !mParameters.contains( QStringLiteral( "LAYERS" ) ) )
    {
      throw QgsMapServiceException( QStringLiteral( "MissingParameterValue" ), QStringLiteral( "LAYERS is mandatory for DescribeLayer operation" ) );
    }

    QStringList layersList = mParameters[ QStringLiteral( "LAYERS" )].split( QStringLiteral( "," ), QString::SkipEmptyParts );
    if ( layersList.size() < 1 )
    {
      throw QgsMapServiceException( QStringLiteral( "InvalidParameterValue" ), QStringLiteral( "Layers is empty" ) );
    }

    //Prepare url
    QString hrefString = mConfigParser->serviceUrl();
    if ( hrefString.isEmpty() )
    {
      hrefString = serviceUrl();
    }

    return mConfigParser->describeLayer( layersList, hrefString );
  }

  QByteArray* QgsWmsServer::getPrint( const QString& formatString )
  {
    QStringList layersList, stylesList, layerIdList;
    QgsMapSettings mapSettings;
    QImage* theImage = initializeRendering( layersList, stylesList, layerIdList, mapSettings );
    if ( !theImage )
    {
      return nullptr;
    }
    delete theImage;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    Q_FOREACH ( QgsMapLayer *layer, QgsProject::instance()->mapLayers() )
    {
      if ( !mAccessControl->layerReadPermission( layer ) )
      {
        throw QgsMapServiceException( QStringLiteral( "Security" ), "You are not allowed to access to the layer: " + layer->name() );
      }
    }
#endif

    //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
    //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
    QScopedPointer< QgsOWSServerFilterRestorer > filterRestorer( new QgsOWSServerFilterRestorer( mAccessControl ) );

    applyRequestedLayerFilters( layersList, mapSettings, filterRestorer->originalFilters() );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    applyAccessControlLayersFilters( layersList, filterRestorer->originalFilters() );
#endif

    QStringList selectedLayerIdList = applyFeatureSelections( layersList );

    //GetPrint request needs a template parameter
    if ( !mParameters.contains( QStringLiteral( "TEMPLATE" ) ) )
    {
      clearFeatureSelections( selectedLayerIdList );
      throw QgsMapServiceException( QStringLiteral( "ParameterMissing" ), QStringLiteral( "The TEMPLATE parameter is required for the GetPrint request" ) );
    }

    QList< QPair< QgsVectorLayer*, QgsFeatureRenderer*> > bkVectorRenderers;
    QList< QPair< QgsRasterLayer*, QgsRasterRenderer* > > bkRasterRenderers;
    QList< QPair< QgsVectorLayer*, double > > labelTransparencies;
    QList< QPair< QgsVectorLayer*, double > > labelBufferTransparencies;

    applyOpacities( layersList, bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );

    QStringList highlightLayers;
    QgsComposition* c = mConfigParser->createPrintComposition( mParameters[ QStringLiteral( "TEMPLATE" )], mapSettings, QMap<QString, QString>( mParameters ), highlightLayers );
    if ( !c )
    {
      restoreOpacities( bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );
      clearFeatureSelections( selectedLayerIdList );
      QgsWmsConfigParser::removeHighlightLayers( highlightLayers );
      return nullptr;
    }

    QByteArray* ba = nullptr;
    c->setPlotStyle( QgsComposition::Print );

    //SVG export without a running X-Server is a problem. See e.g. http://developer.qt.nokia.com/forums/viewthread/2038
    if ( formatString.compare( QLatin1String( "svg" ), Qt::CaseInsensitive ) == 0 )
    {
      c->setPlotStyle( QgsComposition::Print );

      QSvgGenerator generator;
      ba = new QByteArray();
      QBuffer svgBuffer( ba );
      generator.setOutputDevice( &svgBuffer );
      int width = ( int )( c->paperWidth() * c->printResolution() / 25.4 ); //width in pixel
      int height = ( int )( c->paperHeight() * c->printResolution() / 25.4 ); //height in pixel
      generator.setSize( QSize( width, height ) );
      generator.setResolution( c->printResolution() ); //because the rendering is done in mm, convert the dpi

      QPainter p( &generator );
      if ( c->printAsRaster() ) //embed one raster into the svg
      {
        QImage img = c->printPageAsRaster( 0 );
        p.drawImage( QRect( 0, 0, width, height ), img, QRectF( 0, 0, img.width(), img.height() ) );
      }
      else
      {
        c->renderPage( &p, 0 );
      }
      p.end();
    }
    else if ( formatString.compare( QLatin1String( "png" ), Qt::CaseInsensitive ) == 0 || formatString.compare( QLatin1String( "jpg" ), Qt::CaseInsensitive ) == 0 )
    {
      QImage image = c->printPageAsRaster( 0 ); //can only return the first page if pixmap is requested

      ba = new QByteArray();
      QBuffer buffer( ba );
      buffer.open( QIODevice::WriteOnly );
      image.save( &buffer, formatString.toLocal8Bit().data(), -1 );
    }
    else if ( formatString.compare( QLatin1String( "pdf" ), Qt::CaseInsensitive ) == 0 )
    {
      QTemporaryFile tempFile;
      if ( !tempFile.open() )
      {
        delete c;
        restoreOpacities( bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );
        clearFeatureSelections( selectedLayerIdList );
        return nullptr;
      }

      c->exportAsPDF( tempFile.fileName() );
      ba = new QByteArray();
      *ba = tempFile.readAll();
    }
    else //unknown format
    {
      restoreOpacities( bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );
      clearFeatureSelections( selectedLayerIdList );
      throw QgsMapServiceException( QStringLiteral( "InvalidFormat" ), "Output format '" + formatString + "' is not supported in the GetPrint request" );
    }

    restoreOpacities( bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );
    clearFeatureSelections( selectedLayerIdList );
    QgsWmsConfigParser::removeHighlightLayers( highlightLayers );

    delete c;
    return ba;
  }

#if 0
  QImage* QgsWMSServer::printCompositionToImage( QgsComposition* c ) const
  {
    int width = ( int )( c->paperWidth() * c->printResolution() / 25.4 ); //width in pixel
    int height = ( int )( c->paperHeight() * c->printResolution() / 25.4 ); //height in pixel
    QImage* image = new QImage( QSize( width, height ), QImage::Format_ARGB32 );
    image->setDotsPerMeterX( c->printResolution() / 25.4 * 1000 );
    image->setDotsPerMeterY( c->printResolution() / 25.4 * 1000 );
    image->fill( 0 );
    QPainter p( image );
    QRectF sourceArea( 0, 0, c->paperWidth(), c->paperHeight() );
    QRectF targetArea( 0, 0, width, height );
    c->render( &p, targetArea, sourceArea );
    p.end();
    return image;
  }
#endif

  QImage* QgsWmsServer::getMap( HitTest* hitTest )
  {
    QgsMapSettings mapSettings;
    return getMap( mapSettings, hitTest );
  }

  QImage* QgsWmsServer::getMap( QgsMapSettings& mapSettings, HitTest* hitTest )
  {
    if ( !checkMaximumWidthHeight() )
    {
      throw QgsMapServiceException( QStringLiteral( "Size error" ), QStringLiteral( "The requested map size is too large" ) );
    }
    QStringList layersList, stylesList, layerIdList;
    QImage* theImage = initializeRendering( layersList, stylesList, layerIdList, mapSettings );

    QStringList layerSetIds = mapSettings.layerIds();

    QStringList highlightLayersId = QgsWmsConfigParser::addHighlightLayers( mParameters, layerSetIds );

    QList<QgsMapLayer *>  layerSet;
    Q_FOREACH ( QString layerSetId, layerSetIds )
    {
      layerSet.append( QgsProject::instance()->mapLayer( layerSetId ) );
    }
    mapSettings.setLayers( layerSet );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    Q_FOREACH ( QgsMapLayer *layer, QgsProject::instance()->mapLayers() )
    {
      if ( !mAccessControl->layerReadPermission( layer ) )
      {
        throw QgsMapServiceException( QStringLiteral( "Security" ), "You are not allowed to access to the layer: " + layer->name() );
      }
    }
#endif

    //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
    //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
    QScopedPointer< QgsOWSServerFilterRestorer > filterRestorer( new QgsOWSServerFilterRestorer( mAccessControl ) );

    applyRequestedLayerFilters( layersList, mapSettings, filterRestorer->originalFilters() );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    applyAccessControlLayersFilters( layersList, filterRestorer->originalFilters() );
#endif

    QStringList selectedLayerIdList = applyFeatureSelections( layersList );

    QList< QPair< QgsVectorLayer*, QgsFeatureRenderer*> > bkVectorRenderers;
    QList< QPair< QgsRasterLayer*, QgsRasterRenderer* > > bkRasterRenderers;
    QList< QPair< QgsVectorLayer*, double > > labelTransparencies;
    QList< QPair< QgsVectorLayer*, double > > labelBufferTransparencies;

    applyOpacities( layersList, bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );

    QScopedPointer<QPainter> painter;
    if ( hitTest )
    {
      runHitTest( mapSettings, *hitTest );
      painter.reset( new QPainter() );
    }
    else
    {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      mAccessControl->resolveFilterFeatures( mapSettings.layers() );
#endif
      QgsMapRendererJobProxy renderJob( mSettings.parallelRendering(),
                                        mSettings.maxThreads()
#ifdef HAVE_SERVER_PYTHON_PLUGINS
                                        , mAccessControl
#endif
                                      );
      renderJob.render( mapSettings, theImage );
      painter.reset( renderJob.takePainter() );
    }

    if ( mConfigParser )
    {
      //draw configuration format specific overlay items
      mConfigParser->drawOverlays( painter.data(), theImage->dotsPerMeterX() / 1000.0 * 25.4, theImage->width(), theImage->height() );
    }

    restoreOpacities( bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );
    clearFeatureSelections( selectedLayerIdList );
    QgsWmsConfigParser::removeHighlightLayers( highlightLayersId );

    if ( !hitTest )
      QgsProject::instance()->removeAllMapLayers();

    painter->end();

    //test if width / height ratio of image is the same as the ratio of WIDTH / HEIGHT parameters. If not, the image has to be scaled (required by WMS spec)
    int widthParam = mParameters.value( "WIDTH", "0" ).toInt();
    int heightParam = mParameters.value( "HEIGHT", "0" ).toInt();
    if ( widthParam != theImage->width() || heightParam != theImage->height() )
    {
      //scale image
      *theImage = theImage->scaled( widthParam, heightParam, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    }

    return theImage;
  }

  static void infoPointToMapCoordinates( int i, int j, QgsPoint* infoPoint, const QgsMapSettings& mapSettings )
  {
    //check if i, j are in the pixel range of the image
    if ( i < 0 || i > mapSettings.outputSize().width() || j < 0 || j > mapSettings.outputSize().height() )
    {
      throw QgsMapServiceException( "InvalidPoint", "I/J parameters not within the pixel range" );
    }

    double xRes = mapSettings.extent().width() / mapSettings.outputSize().width();
    double yRes = mapSettings.extent().height() / mapSettings.outputSize().height();
    infoPoint->setX( mapSettings.extent().xMinimum() + i * xRes + xRes / 2.0 );
    infoPoint->setY( mapSettings.extent().yMaximum() - j * yRes - yRes / 2.0 );
  }

  QDomDocument QgsWmsServer::getFeatureInfo( const QString& version )
  {
    if ( !mConfigParser )
    {
      throw QgsException( QStringLiteral( "No config parser" ) );
    }

    QDomDocument result;
    QStringList layersList, stylesList;
    bool conversionSuccess;

    for ( QMap<QString, QString>::const_iterator it = mParameters.constBegin(); it != mParameters.constEnd(); ++it )
    {
      QgsMessageLog::logMessage( QStringLiteral( "%1 // %2" ).arg( it.key(), it.value() ) );
    }

    readLayersAndStyles( layersList, stylesList );
    initializeSLDParser( layersList, stylesList );

    QgsMapSettings mapSettings;
    QScopedPointer<QImage> outputImage( createImage() );

    configureMapSettings( outputImage.data(), mapSettings );

    QgsMessageLog::logMessage( "mapSettings.extent(): " +  mapSettings.extent().toString() );
    QgsMessageLog::logMessage( QStringLiteral( "mapSettings width = %1 height = %2" ).arg( mapSettings.outputSize().width() ).arg( mapSettings.outputSize().height() ) );
    QgsMessageLog::logMessage( QStringLiteral( "mapSettings.mapUnitsPerPixel() = %1" ).arg( mapSettings.mapUnitsPerPixel() ) );

    //find out the current scale denominator and set it to the SLD parser
    QgsScaleCalculator scaleCalc(( outputImage->logicalDpiX() + outputImage->logicalDpiY() ) / 2, mapSettings.destinationCrs().mapUnits() );
    QgsRectangle mapExtent = mapSettings.extent();
    double scaleDenominator = scaleCalc.calculate( mapExtent, outputImage->width() );
    mConfigParser->setScaleDenominator( scaleDenominator );

    //read FEATURE_COUNT
    int featureCount = 1;
    if ( mParameters.contains( QStringLiteral( "FEATURE_COUNT" ) ) )
    {
      featureCount = mParameters[ QStringLiteral( "FEATURE_COUNT" )].toInt( &conversionSuccess );
      if ( !conversionSuccess )
      {
        featureCount = 1;
      }
    }

    //read QUERY_LAYERS
    if ( !mParameters.contains( QStringLiteral( "QUERY_LAYERS" ) ) )
    {
      throw QgsMapServiceException( QStringLiteral( "ParameterMissing" ), QStringLiteral( "No QUERY_LAYERS" ) );
    }

    QStringList queryLayerList = mParameters[ QStringLiteral( "QUERY_LAYERS" )].split( QStringLiteral( "," ), QString::SkipEmptyParts );
    if ( queryLayerList.size() < 1 )
    {
      throw QgsMapServiceException( QStringLiteral( "InvalidParameterValue" ), QStringLiteral( "Malformed QUERY_LAYERS" ) );
    }

    //read I,J resp. X,Y
    QString iString = mParameters.value( QStringLiteral( "I" ), mParameters.value( QStringLiteral( "X" ) ) );
    int i = iString.toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      i = -1;
    }

    QString jString = mParameters.value( QStringLiteral( "J" ), mParameters.value( QStringLiteral( "Y" ) ) );
    int j = jString.toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      j = -1;
    }

    //In case the output image is distorted (WIDTH/HEIGHT ratio not equal to BBOX width/height), I and J need to be adapted as well
    int widthParam = mParameters.value( "WIDTH", "-1" ).toInt();
    int heightParam = mParameters.value( "HEIGHT", "-1" ).toInt();
    if (( i != -1 && j != -1 && widthParam != -1 && heightParam != -1 ) && ( widthParam != outputImage->width() || heightParam != outputImage->height() ) )
    {
      i *= ( outputImage->width() / ( double )widthParam );
      j *= ( outputImage->height() / ( double )heightParam );
    }

    //Normally, I/J or X/Y are mandatory parameters.
    //However, in order to make attribute only queries via the FILTER parameter, it is allowed to skip them if the FILTER parameter is there

    QScopedPointer<QgsRectangle> featuresRect;
    QScopedPointer<QgsPoint> infoPoint;

    if ( i == -1 || j == -1 )
    {
      if ( mParameters.contains( QStringLiteral( "FILTER" ) ) )
      {
        featuresRect.reset( new QgsRectangle() );
      }
      else
      {
        throw QgsMapServiceException( QStringLiteral( "ParameterMissing" ), QStringLiteral( "I/J parameters are required for GetFeatureInfo" ) );
      }
    }
    else
    {
      infoPoint.reset( new QgsPoint() );
      infoPointToMapCoordinates( i, j, infoPoint.data(), mapSettings );
    }

    //get the layer registered in QgsMapLayerRegistry and apply possible filters
    ( void )layerSet( layersList, stylesList, mapSettings.destinationCrs() );

    //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
    //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
    QScopedPointer< QgsOWSServerFilterRestorer > filterRestorer( new QgsOWSServerFilterRestorer( mAccessControl ) );
    applyRequestedLayerFilters( layersList, mapSettings, filterRestorer->originalFilters() );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    applyAccessControlLayersFilters( layersList, filterRestorer->originalFilters() );
#endif

    QDomElement getFeatureInfoElement;
    QString infoFormat = mParameters.value( QStringLiteral( "INFO_FORMAT" ) );
    if ( infoFormat.startsWith( QLatin1String( "application/vnd.ogc.gml" ) ) )
    {
      getFeatureInfoElement = result.createElement( QStringLiteral( "wfs:FeatureCollection" ) );
      getFeatureInfoElement.setAttribute( QStringLiteral( "xmlns:wfs" ), QStringLiteral( "http://www.opengis.net/wfs" ) );
      getFeatureInfoElement.setAttribute( QStringLiteral( "xmlns:ogc" ), QStringLiteral( "http://www.opengis.net/ogc" ) );
      getFeatureInfoElement.setAttribute( QStringLiteral( "xmlns:gml" ), QStringLiteral( "http://www.opengis.net/gml" ) );
      getFeatureInfoElement.setAttribute( QStringLiteral( "xmlns:ows" ), QStringLiteral( "http://www.opengis.net/ows" ) );
      getFeatureInfoElement.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
      getFeatureInfoElement.setAttribute( QStringLiteral( "xmlns:qgs" ), QStringLiteral( "http://qgis.org/gml" ) );
      getFeatureInfoElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
      getFeatureInfoElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), QStringLiteral( "http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.0.0/wfs.xsd http://qgis.org/gml" ) );
    }
    else
    {
      QString featureInfoElemName = mConfigParser->featureInfoDocumentElement( QStringLiteral( "GetFeatureInfoResponse" ) );
      QString featureInfoElemNS = mConfigParser->featureInfoDocumentElementNS();
      if ( featureInfoElemNS.isEmpty() )
      {
        getFeatureInfoElement = result.createElement( featureInfoElemName );
      }
      else
      {
        getFeatureInfoElement = result.createElementNS( featureInfoElemNS, featureInfoElemName );
      }
      //feature info schema
      QString featureInfoSchema = mConfigParser->featureInfoSchema();
      if ( !featureInfoSchema.isEmpty() )
      {
        getFeatureInfoElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
        getFeatureInfoElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), featureInfoSchema );
      }
    }
    result.appendChild( getFeatureInfoElement );

    QStringList nonIdentifiableLayers = mConfigParser->identifyDisabledLayers();

    //Render context is needed to determine feature visibility for vector layers
    QgsRenderContext renderContext = QgsRenderContext::fromMapSettings( mapSettings );

    bool sia2045 = mConfigParser->featureInfoFormatSIA2045();

    //layers can have assigned a different name for GetCapabilities
    QHash<QString, QString> layerAliasMap = mConfigParser->featureInfoLayerAliasMap();

    QList<QgsMapLayer*> layerList;
    QgsMapLayer* currentLayer = nullptr;
    QStringList::const_iterator layerIt;
    for ( layerIt = queryLayerList.constBegin(); layerIt != queryLayerList.constEnd(); ++layerIt )
    {
      //create maplayers from sld parser (several layers are possible in case of feature info on a group)
      layerList = mConfigParser->mapLayerFromStyle( *layerIt, QLatin1String( "" ) );
      QList<QgsMapLayer*>::iterator layerListIt = layerList.begin();
      for ( ; layerListIt != layerList.end(); ++layerListIt )
      {
        currentLayer = *layerListIt;
        if ( !currentLayer || nonIdentifiableLayers.contains( currentLayer->id() ) )
        {
          continue;
        }
        QgsMapLayer * registeredMapLayer = QgsProject::instance()->mapLayer( currentLayer->id() );
        if ( registeredMapLayer )
        {
          currentLayer = registeredMapLayer;
        }

#ifdef HAVE_SERVER_PYTHON_PLUGINS
        if ( !mAccessControl->layerReadPermission( currentLayer ) )
        {
          throw QgsMapServiceException( QStringLiteral( "Security" ), "You are not allowed to access to the layer: " + currentLayer->name() );
        }
#endif

        //skip layer if not visible at current map scale
        bool useScaleConstraint = ( scaleDenominator > 0 && currentLayer->hasScaleBasedVisibility() );
        if ( useScaleConstraint && ( currentLayer->minimumScale() > scaleDenominator || currentLayer->maximumScale() < scaleDenominator ) )
        {
          continue;
        }

        //switch depending on vector or raster
        QgsVectorLayer* vectorLayer = qobject_cast<QgsVectorLayer*>( currentLayer );

        QDomElement layerElement;
        if ( infoFormat.startsWith( QLatin1String( "application/vnd.ogc.gml" ) ) )
        {
          layerElement = getFeatureInfoElement;
        }
        else
        {
          layerElement = result.createElement( QStringLiteral( "Layer" ) );
          QString layerName =  currentLayer->name();
          if ( mConfigParser && mConfigParser->useLayerIds() )
            layerName = currentLayer->id();
          else if ( !currentLayer->shortName().isEmpty() )
            layerName = currentLayer->shortName();

          //check if the layer is given a different name for GetFeatureInfo output
          QHash<QString, QString>::const_iterator layerAliasIt = layerAliasMap.find( layerName );
          if ( layerAliasIt != layerAliasMap.constEnd() )
          {
            layerName = layerAliasIt.value();
          }
          layerElement.setAttribute( QStringLiteral( "name" ), layerName );
          getFeatureInfoElement.appendChild( layerElement );
          if ( sia2045 ) //the name might not be unique after alias replacement
          {
            layerElement.setAttribute( QStringLiteral( "id" ), currentLayer->id() );
          }
        }

        if ( vectorLayer )
        {
          if ( !featureInfoFromVectorLayer( vectorLayer, infoPoint.data(), featureCount, result, layerElement, mapSettings, renderContext, version, infoFormat, featuresRect.data() ) )
          {
            continue;
          }
        }
        else //raster layer
        {
          if ( infoFormat.startsWith( QLatin1String( "application/vnd.ogc.gml" ) ) )
          {
            layerElement = result.createElement( QStringLiteral( "gml:featureMember" )/*wfs:FeatureMember*/ );
            getFeatureInfoElement.appendChild( layerElement );
          }

          QgsRasterLayer* rasterLayer = qobject_cast<QgsRasterLayer*>( currentLayer );
          if ( rasterLayer )
          {
            if ( !infoPoint.data() )
            {
              continue;
            }
            QgsPoint layerInfoPoint = mapSettings.mapToLayerCoordinates( currentLayer, *( infoPoint.data() ) );
            if ( !featureInfoFromRasterLayer( rasterLayer, mapSettings, &layerInfoPoint, result, layerElement, version, infoFormat ) )
            {
              continue;
            }
          }
          else
          {
            continue;
          }
        }
      }
    }

    if ( featuresRect )
    {
      if ( infoFormat.startsWith( QLatin1String( "application/vnd.ogc.gml" ) ) )
      {
        QDomElement bBoxElem = result.createElement( QStringLiteral( "gml:boundedBy" ) );
        QDomElement boxElem;
        int gmlVersion = infoFormat.startsWith( QLatin1String( "application/vnd.ogc.gml/3" ) ) ? 3 : 2;
        if ( gmlVersion < 3 )
        {
          boxElem = QgsOgcUtils::rectangleToGMLBox( featuresRect.data(), result, 8 );
        }
        else
        {
          boxElem = QgsOgcUtils::rectangleToGMLEnvelope( featuresRect.data(), result, 8 );
        }

        QgsCoordinateReferenceSystem crs = mapSettings.destinationCrs();
        if ( crs.isValid() )
        {
          boxElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
        }
        bBoxElem.appendChild( boxElem );
        getFeatureInfoElement.insertBefore( bBoxElem, QDomNode() ); //insert as first child
      }
      else
      {
        QDomElement bBoxElem = result.createElement( QStringLiteral( "BoundingBox" ) );
        bBoxElem.setAttribute( QStringLiteral( "CRS" ), mapSettings.destinationCrs().authid() );
        bBoxElem.setAttribute( QStringLiteral( "minx" ), qgsDoubleToString( featuresRect->xMinimum(), 8 ) );
        bBoxElem.setAttribute( QStringLiteral( "maxx" ), qgsDoubleToString( featuresRect->xMaximum(), 8 ) );
        bBoxElem.setAttribute( QStringLiteral( "miny" ), qgsDoubleToString( featuresRect->yMinimum(), 8 ) );
        bBoxElem.setAttribute( QStringLiteral( "maxy" ), qgsDoubleToString( featuresRect->yMaximum(), 8 ) );
        getFeatureInfoElement.insertBefore( bBoxElem, QDomNode() ); //insert as first child
      }
    }

    if ( sia2045 && infoFormat.compare( QLatin1String( "text/xml" ), Qt::CaseInsensitive ) == 0 )
    {
      convertFeatureInfoToSIA2045( result );
    }

    //force restoration of original filters
    filterRestorer.reset();

    QgsProject::instance()->removeAllMapLayers();

    return result;
  }

  QImage* QgsWmsServer::initializeRendering( QStringList& layersList, QStringList& stylesList, QStringList& layerIdList, QgsMapSettings& mapSettings )
  {
    if ( !mConfigParser )
    {
      throw QgsException( QStringLiteral( "initializeRendering(): No config parser" ) );
    }

    readLayersAndStyles( layersList, stylesList );
    initializeSLDParser( layersList, stylesList );

    //pass external GML to the SLD parser.
    QString gml = mParameters.value( QStringLiteral( "GML" ) );
    if ( !gml.isEmpty() )
    {
      if ( !mConfigParser->allowRequestDefinedDatasources() )
      {
        throw QgsException( QStringLiteral( "initializeRendering: The project configuration does not allow datasources defined in the request" ) );
      }
      QScopedPointer<QDomDocument> gmlDoc( new QDomDocument() );
      if ( gmlDoc->setContent( gml, true ) )
      {
        QString layerName = gmlDoc->documentElement().attribute( QStringLiteral( "layerName" ) );
        QgsMessageLog::logMessage( "Adding entry with key: " + layerName + " to external GML data" );
        mConfigParser->addExternalGMLData( layerName, gmlDoc.take() );
      }
      else
      {
        throw QgsException( QStringLiteral( "initializeRendering: Error, could not add external GML to QgsSLDParser" ) );
      }
    }

    QScopedPointer<QImage> theImage( createImage() );

    configureMapSettings( theImage.data(), mapSettings );

    //find out the current scale denominater and set it to the SLD parser
    QgsScaleCalculator scaleCalc(( theImage->logicalDpiX() + theImage->logicalDpiY() ) / 2, mapSettings.destinationCrs().mapUnits() );
    QgsRectangle mapExtent = mapSettings.extent();
    mConfigParser->setScaleDenominator( scaleCalc.calculate( mapExtent, theImage->width() ) );

    layerIdList = layerSet( layersList, stylesList, mapSettings.destinationCrs() );
#ifdef QGISDEBUG
    QgsMessageLog::logMessage( QStringLiteral( "Number of layers to be rendered. %1" ).arg( layerIdList.count() ) );
#endif

    QList<QgsMapLayer *>  layers;
    Q_FOREACH ( QString layerId, layerIdList )
    {
      layers.append( QgsProject::instance()->mapLayer( layerId ) );
    }
    mapSettings.setLayers( layers );

    // load label settings
    mConfigParser->loadLabelSettings();

    return theImage.take();
  }

  QImage* QgsWmsServer::createImage( int width, int height, bool useBbox ) const
  {
    bool conversionSuccess;

    if ( width < 0 )
    {
      width = mParameters.value( QStringLiteral( "WIDTH" ), QStringLiteral( "0" ) ).toInt( &conversionSuccess );
      if ( !conversionSuccess )
        width = 0;
    }

    if ( height < 0 )
    {
      height = mParameters.value( QStringLiteral( "HEIGHT" ), QStringLiteral( "0" ) ).toInt( &conversionSuccess );
      if ( !conversionSuccess )
      {
        height = 0;
      }
    }

    //Adapt width / height if the aspect ratio does not correspond with the BBOX.
    //Required by WMS spec. 1.3.
    QString version = mParameters.value( QStringLiteral( "VERSION" ), QStringLiteral( "1.3.0" ) );
    if ( useBbox && version != QLatin1String( "1.1.1" ) )
    {
      bool bboxOk;
      QgsRectangle mapExtent = _parseBBOX( mParameters.value( "BBOX" ), bboxOk );
      QString crs = mParameters.value( QStringLiteral( "CRS" ), mParameters.value( QStringLiteral( "SRS" ) ) );
      if ( crs.compare( "CRS:84", Qt::CaseInsensitive ) == 0 )
      {
        crs = QString( "EPSG:4326" );
        mapExtent.invert();
      }
      QgsCoordinateReferenceSystem outputCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs );
      if ( outputCRS.hasAxisInverted() )
      {
        mapExtent.invert();
      }
      if ( bboxOk )
      {
        double mapWidthHeightRatio = mapExtent.width() / mapExtent.height();
        double imageWidthHeightRatio = ( double )width / ( double )height;
        if ( !qgsDoubleNear( mapWidthHeightRatio, imageWidthHeightRatio, 0.0001 ) )
        {
          // inspired by MapServer, mapdraw.c L115
          double cellsize = ( mapExtent.width() / ( double )width ) * 0.5 + ( mapExtent.height() / ( double )height ) * 0.5;
          width = mapExtent.width() / cellsize;
          height = mapExtent.height() / cellsize;
        }
      }
    }

    if ( width < 0 || height < 0 )
    {
      throw QgsException( QStringLiteral( "createImage: Invalid width / height parameters" ) );
    }

    QImage* theImage = nullptr;

    //Define the image background color in case of map settings is not used
    //is format jpeg?
    QString format = mParameters.value( QStringLiteral( "FORMAT" ) );
    bool jpeg = format.compare( QLatin1String( "jpg" ), Qt::CaseInsensitive ) == 0
                || format.compare( QLatin1String( "jpeg" ), Qt::CaseInsensitive ) == 0
                || format.compare( QLatin1String( "image/jpeg" ), Qt::CaseInsensitive ) == 0;

    //transparent parameter
    bool transparent = mParameters.value( QStringLiteral( "TRANSPARENT" ) ).compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0;

    //background  color
    QString bgColorString = mParameters.value( "BGCOLOR" );
    if ( bgColorString.startsWith( "0x", Qt::CaseInsensitive ) )
    {
      bgColorString.replace( 0, 2, "#" );
    }
    QColor backgroundColor;
    backgroundColor.setNamedColor( bgColorString );
    if ( !backgroundColor.isValid() )
    {
      backgroundColor = QColor( Qt::white );
    }

    //use alpha channel only if necessary because it slows down performance
    if ( transparent && !jpeg )
    {
      theImage = new QImage( width, height, QImage::Format_ARGB32_Premultiplied );
      theImage->fill( 0 );
    }
    else
    {
      theImage = new QImage( width, height, QImage::Format_RGB32 );
      theImage->fill( backgroundColor );
    }

    //apply DPI parameter if present. This is an extension of Qgis Mapserver compared to WMS 1.3.
    //Because of backwards compatibility, this parameter is optional
    double OGC_PX_M = 0.00028; // OGC reference pixel size in meter, also used by qgis
    int dpm = 1 / OGC_PX_M;
    if ( mParameters.contains( QStringLiteral( "DPI" ) ) )
    {
      int dpi = mParameters[ QStringLiteral( "DPI" )].toInt( &conversionSuccess );
      if ( conversionSuccess )
      {
        dpm = dpi / 0.0254;
      }
    }
    theImage->setDotsPerMeterX( dpm );
    theImage->setDotsPerMeterY( dpm );
    return theImage;
  }

  void QgsWmsServer::configureMapSettings( const QPaintDevice* paintDevice, QgsMapSettings& mapSettings ) const
  {
    if ( !paintDevice )
    {
      throw QgsException( QStringLiteral( "configureMapSettings: no paint device" ) );
    }

    mapSettings.datumTransformStore().clear();
    mapSettings.setOutputSize( QSize( paintDevice->width(), paintDevice->height() ) );
    mapSettings.setOutputDpi( paintDevice->logicalDpiX() );

    //map extent
    bool bboxOk = true;
    QgsRectangle mapExtent;
    if ( mParameters.contains( "BBOX" ) )
    {
      mapExtent = _parseBBOX( mParameters.value( QStringLiteral( "BBOX" ), QStringLiteral( "0,0,0,0" ) ), bboxOk );
    }

    if ( !bboxOk )
    {
      //throw a service exception
      throw QgsMapServiceException( QStringLiteral( "InvalidParameterValue" ), QStringLiteral( "Invalid BBOX parameter" ) );
    }

    if ( mParameters.contains( "BBOX" ) && mapExtent.isEmpty() )
    {
      throw QgsMapServiceException( "InvalidParameterValue", "BBOX is empty" );
    }

    QgsUnitTypes::DistanceUnit mapUnits = QgsUnitTypes::DistanceDegrees;

    QString crs = mParameters.value( QStringLiteral( "CRS" ), mParameters.value( QStringLiteral( "SRS" ) ) );
    if ( crs.compare( "CRS:84", Qt::CaseInsensitive ) == 0 )
    {
      crs = QString( "EPSG:4326" );
      mapExtent.invert();
    }

    QgsCoordinateReferenceSystem outputCRS;

    //wms spec says that CRS parameter is mandatory.
    //we don't reject the request if it is not there but disable reprojection on the fly
    if ( crs.isEmpty() )
    {
      //disable on the fly projection
      QgsProject::instance()->writeEntry( QStringLiteral( "SpatialRefSys" ), QStringLiteral( "/ProjectionsEnabled" ), 0 );
    }
    else
    {
      //enable on the fly projection
      QgsMessageLog::logMessage( QStringLiteral( "enable on the fly projection" ) );
      QgsProject::instance()->writeEntry( QStringLiteral( "SpatialRefSys" ), QStringLiteral( "/ProjectionsEnabled" ), 1 );

      //destination SRS
      outputCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs );
      if ( !outputCRS.isValid() )
      {
        QgsMessageLog::logMessage( QStringLiteral( "Error, could not create output CRS from EPSG" ) );
        throw QgsMapServiceException( QStringLiteral( "InvalidCRS" ), QStringLiteral( "Could not create output CRS" ) );
      }

      //then set destinationCrs
      mapSettings.setDestinationCrs( outputCRS );
      mapSettings.setCrsTransformEnabled( true );
      mapUnits = outputCRS.mapUnits();
    }
    mapSettings.setMapUnits( mapUnits );

    // Change x- and y- of BBOX for WMS 1.3.0 if axis inverted
    QString version = mParameters.value( QStringLiteral( "VERSION" ), QStringLiteral( "1.3.0" ) );
    if ( version != QLatin1String( "1.1.1" ) && outputCRS.hasAxisInverted() )
    {
      mapExtent.invert();
    }

    mapSettings.setExtent( mapExtent );

    /* Define the background color
     * Transparent or colored
     */
    //is format jpeg?
    QString format = mParameters.value( QStringLiteral( "FORMAT" ) );
    bool jpeg = format.compare( QLatin1String( "jpg" ), Qt::CaseInsensitive ) == 0
                || format.compare( QLatin1String( "jpeg" ), Qt::CaseInsensitive ) == 0
                || format.compare( QLatin1String( "image/jpeg" ), Qt::CaseInsensitive ) == 0;

    //transparent parameter
    bool transparent = mParameters.value( QStringLiteral( "TRANSPARENT" ) ).compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0;

    //background  color
    QString bgColorString = mParameters.value( "BGCOLOR" );
    if ( bgColorString.startsWith( "0x", Qt::CaseInsensitive ) )
    {
      bgColorString.replace( 0, 2, "#" );
    }
    QColor backgroundColor;
    backgroundColor.setNamedColor( bgColorString );

    //set background color
    if ( transparent && !jpeg )
    {
      mapSettings.setBackgroundColor( QColor( 0, 0, 0, 0 ) );
    }
    else if ( backgroundColor.isValid() )
    {
      mapSettings.setBackgroundColor( backgroundColor );
    }
  }

  void QgsWmsServer::readLayersAndStyles( QStringList& layersList, QStringList& stylesList ) const
  {
    //get layer and style lists from the parameters trying LAYERS and LAYER as well as STYLE and STYLES for GetLegendGraphic compatibility
    layersList = mParameters.value( QStringLiteral( "LAYER" ) ).split( QStringLiteral( "," ), QString::SkipEmptyParts );
    layersList = layersList + mParameters.value( QStringLiteral( "LAYERS" ) ).split( QStringLiteral( "," ), QString::SkipEmptyParts );
    stylesList = mParameters.value( QStringLiteral( "STYLE" ) ).split( QStringLiteral( "," ), QString::SkipEmptyParts );
    stylesList = stylesList + mParameters.value( QStringLiteral( "STYLES" ) ).split( QStringLiteral( "," ), QString::SkipEmptyParts );
  }

  void QgsWmsServer::initializeSLDParser( QStringList& layersList, QStringList& stylesList )
  {
    QString xml = mParameters.value( QStringLiteral( "SLD" ) );
    if ( !xml.isEmpty() )
    {
      //ignore LAYERS and STYLES and take those information from the SLD
      QScopedPointer<QDomDocument> theDocument( new QDomDocument( QStringLiteral( "user.sld" ) ) );
      QString errorMsg;
      int errorLine, errorColumn;

      if ( !theDocument->setContent( xml, true, &errorMsg, &errorLine, &errorColumn ) )
      {
        throw QgsException( QStringLiteral( "SLDParser: Could not create DomDocument from SLD: %1" ).arg( errorMsg ) );
      }

      QgsSLDConfigParser* userSLDParser = new QgsSLDConfigParser( theDocument.take(), mParameters );
      userSLDParser->setFallbackParser( mConfigParser );
      mConfigParser = userSLDParser;
      mOwnsConfigParser = true;
      //now replace the content of layersList and stylesList (if present)
      layersList.clear();
      stylesList.clear();
      QStringList layersSTDList;
      QStringList stylesSTDList;
      if ( mConfigParser->layersAndStyles( layersSTDList, stylesSTDList ) != 0 )
      {
        throw QgsException( QStringLiteral( "SLDParser: no layers and styles found in SLD" ) );
      }
      QStringList::const_iterator layersIt;
      QStringList::const_iterator stylesIt;
      for ( layersIt = layersSTDList.constBegin(), stylesIt = stylesSTDList.constBegin(); layersIt != layersSTDList.constEnd(); ++layersIt, ++stylesIt )
      {
        layersList << *layersIt;
        stylesList << *stylesIt;
      }
    }
  }

  bool QgsWmsServer::featureInfoFromVectorLayer( QgsVectorLayer* layer,
      const QgsPoint* infoPoint,
      int nFeatures,
      QDomDocument& infoDocument,
      QDomElement& layerElement,
      const QgsMapSettings& mapSettings,
      QgsRenderContext& renderContext,
      const QString& version,
      const QString& infoFormat,
      QgsRectangle* featureBBox ) const
  {
    if ( !layer )
    {
      return false;
    }

    //we need a selection rect (0.01 of map width)
    QgsRectangle mapRect = mapSettings.extent();
    QgsRectangle layerRect = mapSettings.mapToLayerCoordinates( layer, mapRect );


    QgsRectangle searchRect;

    //info point could be 0 in case there is only an attribute filter
    if ( infoPoint )
    {
      searchRect = featureInfoSearchRect( layer, mapSettings, renderContext, *infoPoint );
    }
    else if ( mParameters.contains( QStringLiteral( "BBOX" ) ) )
    {
      searchRect = layerRect;
    }

    //do a select with searchRect and go through all the features

    QgsFeature feature;
    QgsAttributes featureAttributes;
    int featureCounter = 0;
    layer->updateFields();
    const QgsFields& fields = layer->pendingFields();
    bool addWktGeometry = mConfigParser && mConfigParser->featureInfoWithWktGeometry();
    bool segmentizeWktGeometry = mConfigParser && mConfigParser->segmentizeFeatureInfoWktGeometry();
    const QSet<QString>& excludedAttributes = layer->excludeAttributesWms();

    QgsFeatureRequest fReq;
    bool hasGeometry = addWktGeometry || featureBBox;
    fReq.setFlags((( hasGeometry ) ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) | QgsFeatureRequest::ExactIntersect );

    if ( ! searchRect.isEmpty() )
    {
      fReq.setFilterRect( searchRect );
    }
    else
    {
      fReq.setFlags( fReq.flags() & ~ QgsFeatureRequest::ExactIntersect );
    }

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    mAccessControl->filterFeatures( layer, fReq );

    QStringList attributes;
    QgsField field;
    Q_FOREACH ( field, layer->pendingFields().toList() )
    {
      attributes.append( field.name() );
    }
    attributes = mAccessControl->layerAttributes( layer, attributes );
    fReq.setSubsetOfAttributes( attributes, layer->pendingFields() );
#endif

    QgsFeatureIterator fit = layer->getFeatures( fReq );
    QgsFeatureRenderer* r2 = layer->renderer();
    if ( r2 )
    {
      r2->startRender( renderContext, layer->pendingFields() );
    }

    bool featureBBoxInitialized = false;
    while ( fit.nextFeature( feature ) )
    {
      if ( layer->wkbType() == QgsWkbTypes::NoGeometry && ! searchRect.isEmpty() )
      {
        break;
      }

      ++featureCounter;
      if ( featureCounter > nFeatures )
      {
        break;
      }

      if ( layer->wkbType() != QgsWkbTypes::NoGeometry && ! searchRect.isEmpty() )
      {
        if ( !r2 )
        {
          continue;
        }

        renderContext.expressionContext().setFeature( feature );

        //check if feature is rendered at all
        bool render = r2->willRenderFeature( feature, renderContext );
        if ( !render )
        {
          continue;
        }
      }

      QgsRectangle box;
      if ( layer->wkbType() != QgsWkbTypes::NoGeometry && hasGeometry )
      {
        box = mapSettings.layerExtentToOutputExtent( layer, feature.geometry().boundingBox() );
        if ( featureBBox ) //extend feature info bounding box if requested
        {
          if ( !featureBBoxInitialized && featureBBox->isEmpty() )
          {
            *featureBBox = box;
            featureBBoxInitialized = true;
          }
          else
          {
            featureBBox->combineExtentWith( box );
          }
        }
      }

      QgsCoordinateReferenceSystem outputCrs = layer->crs();
      if ( layer->crs() != mapSettings.destinationCrs() && mapSettings.hasCrsTransformEnabled() )
      {
        outputCrs = mapSettings.destinationCrs();
      }

      if ( infoFormat == QLatin1String( "application/vnd.ogc.gml" ) )
      {
        bool withGeom = layer->wkbType() != QgsWkbTypes::NoGeometry && addWktGeometry;
        int version = infoFormat.startsWith( QLatin1String( "application/vnd.ogc.gml/3" ) ) ? 3 : 2;
        QString typeName =  layer->name();
        if ( mConfigParser && mConfigParser->useLayerIds() )
          typeName = layer->id();
        else if ( !layer->shortName().isEmpty() )
          typeName = layer->shortName();
        QDomElement elem = createFeatureGML(
                             &feature, layer, infoDocument, outputCrs, mapSettings, typeName, withGeom, version
#ifdef HAVE_SERVER_PYTHON_PLUGINS
                             , &attributes
#endif
                           );
        QDomElement featureMemberElem = infoDocument.createElement( QStringLiteral( "gml:featureMember" )/*wfs:FeatureMember*/ );
        featureMemberElem.appendChild( elem );
        layerElement.appendChild( featureMemberElem );
        continue;
      }
      else
      {
        QDomElement featureElement = infoDocument.createElement( QStringLiteral( "Feature" ) );
        featureElement.setAttribute( QStringLiteral( "id" ), FID_TO_STRING( feature.id() ) );
        layerElement.appendChild( featureElement );

        //read all attribute values from the feature
        featureAttributes = feature.attributes();
        for ( int i = 0; i < featureAttributes.count(); ++i )
        {
          //skip attribute if it is explicitly excluded from WMS publication
          if ( excludedAttributes.contains( fields.at( i ).name() ) )
          {
            continue;
          }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
          //skip attribute if it is excluded by access control
          if ( !attributes.contains( fields.at( i ).name() ) )
          {
            continue;
          }
#endif

          //replace attribute name if there is an attribute alias?
          QString attributeName = layer->attributeDisplayName( i );

          QDomElement attributeElement = infoDocument.createElement( QStringLiteral( "Attribute" ) );
          attributeElement.setAttribute( QStringLiteral( "name" ), attributeName );
          attributeElement.setAttribute( QStringLiteral( "value" ),
                                         replaceValueMapAndRelation(
                                           layer, i,
                                           featureAttributes[i].isNull() ?  QString::null : QgsExpression::replaceExpressionText( featureAttributes[i].toString(), &renderContext.expressionContext() )
                                         )
                                       );
          featureElement.appendChild( attributeElement );
        }

        //add maptip attribute based on html/expression (in case there is no maptip attribute)
        QString mapTip = layer->mapTipTemplate();
        if ( !mapTip.isEmpty() )
        {
          QDomElement maptipElem = infoDocument.createElement( QStringLiteral( "Attribute" ) );
          maptipElem.setAttribute( QStringLiteral( "name" ), QStringLiteral( "maptip" ) );
          maptipElem.setAttribute( QStringLiteral( "value" ),  QgsExpression::replaceExpressionText( mapTip, &renderContext.expressionContext() ) );
          featureElement.appendChild( maptipElem );
        }

        //append feature bounding box to feature info xml
        if ( layer->wkbType() != QgsWkbTypes::NoGeometry && hasGeometry && mConfigParser )
        {
          QDomElement bBoxElem = infoDocument.createElement( QStringLiteral( "BoundingBox" ) );
          bBoxElem.setAttribute( version == QLatin1String( "1.1.1" ) ? "SRS" : "CRS", outputCrs.authid() );
          bBoxElem.setAttribute( QStringLiteral( "minx" ), qgsDoubleToString( box.xMinimum(), getWMSPrecision( 8 ) ) );
          bBoxElem.setAttribute( QStringLiteral( "maxx" ), qgsDoubleToString( box.xMaximum(), getWMSPrecision( 8 ) ) );
          bBoxElem.setAttribute( QStringLiteral( "miny" ), qgsDoubleToString( box.yMinimum(), getWMSPrecision( 8 ) ) );
          bBoxElem.setAttribute( QStringLiteral( "maxy" ), qgsDoubleToString( box.yMaximum(), getWMSPrecision( 8 ) ) );
          featureElement.appendChild( bBoxElem );
        }

        //also append the wkt geometry as an attribute
        if ( layer->wkbType() != QgsWkbTypes::NoGeometry && addWktGeometry && hasGeometry )
        {
          QgsGeometry geom = feature.geometry();
          if ( !geom.isEmpty() )
          {
            if ( layer->crs() != outputCrs )
            {
              QgsCoordinateTransform transform = mapSettings.layerTransform( layer );
              if ( transform.isValid() )
                geom.transform( transform );
            }

            if ( segmentizeWktGeometry )
            {
              QgsAbstractGeometry* abstractGeom = geom.geometry();
              if ( abstractGeom )
              {
                if ( QgsWkbTypes::isCurvedType( abstractGeom->wkbType() ) )
                {
                  QgsAbstractGeometry* segmentizedGeom = abstractGeom-> segmentize();
                  geom.setGeometry( segmentizedGeom );
                }
              }
            }
            QDomElement geometryElement = infoDocument.createElement( QStringLiteral( "Attribute" ) );
            geometryElement.setAttribute( QStringLiteral( "name" ), QStringLiteral( "geometry" ) );
            geometryElement.setAttribute( QStringLiteral( "value" ), geom.exportToWkt( getWMSPrecision( 8 ) ) );
            geometryElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "derived" ) );
            featureElement.appendChild( geometryElement );
          }
        }
      }
    }
    if ( r2 )
    {
      r2->stopRender( renderContext );
    }

    return true;
  }

  bool QgsWmsServer::featureInfoFromRasterLayer( QgsRasterLayer* layer,
      const QgsMapSettings& mapSettings,
      const QgsPoint* infoPoint,
      QDomDocument& infoDocument,
      QDomElement& layerElement,
      const QString& version,
      const QString& infoFormat ) const
  {
    Q_UNUSED( version );

    if ( !infoPoint || !layer || !layer->dataProvider() )
    {
      return false;
    }

    QgsMessageLog::logMessage( QStringLiteral( "infoPoint: %1 %2" ).arg( infoPoint->x() ).arg( infoPoint->y() ) );

    if ( !( layer->dataProvider()->capabilities() & QgsRasterDataProvider::IdentifyValue ) )
    {
      return false;
    }
    QMap<int, QVariant> attributes;
    // use context extent, width height (comes with request) to use WCS cache
    // We can only use context if raster is not reprojected, otherwise it is difficult
    // to guess correct source resolution
    if ( mapSettings.hasCrsTransformEnabled() && layer->dataProvider()->crs() != mapSettings.destinationCrs() )
    {
      attributes = layer->dataProvider()->identify( *infoPoint, QgsRaster::IdentifyFormatValue ).results();
    }
    else
    {
      attributes = layer->dataProvider()->identify( *infoPoint, QgsRaster::IdentifyFormatValue, mapSettings.extent(), mapSettings.outputSize().width(), mapSettings.outputSize().height() ).results();
    }

    if ( infoFormat == QLatin1String( "application/vnd.ogc.gml" ) )
    {
      QgsFeature feature;
      QgsFields fields;
      feature.initAttributes( attributes.count() );
      int index = 0;
      for ( QMap<int, QVariant>::const_iterator it = attributes.constBegin(); it != attributes.constEnd(); ++it )
      {
        fields.append( QgsField( layer->bandName( it.key() ), QVariant::Double ) );
        feature.setAttribute( index++, QString::number( it.value().toDouble() ) );
      }
      feature.setFields( fields );

      QgsCoordinateReferenceSystem layerCrs = layer->crs();
      int version = infoFormat.startsWith( QLatin1String( "application/vnd.ogc.gml/3" ) ) ? 3 : 2;
      QString typeName =  layer->name();
      if ( mConfigParser && mConfigParser->useLayerIds() )
        typeName = layer->id();
      else if ( !layer->shortName().isEmpty() )
        typeName = layer->shortName();
      QDomElement elem = createFeatureGML(
                           &feature, nullptr, infoDocument, layerCrs, mapSettings, typeName, false, version, nullptr );
      layerElement.appendChild( elem );
    }
    else
    {
      for ( QMap<int, QVariant>::const_iterator it = attributes.constBegin(); it != attributes.constEnd(); ++it )
      {
        QDomElement attributeElement = infoDocument.createElement( QStringLiteral( "Attribute" ) );
        attributeElement.setAttribute( QStringLiteral( "name" ), layer->bandName( it.key() ) );
        attributeElement.setAttribute( QStringLiteral( "value" ), QString::number( it.value().toDouble() ) );
        layerElement.appendChild( attributeElement );
      }
    }
    return true;
  }

  QStringList QgsWmsServer::layerSet( const QStringList &layersList,
                                      const QStringList &stylesList,
                                      const QgsCoordinateReferenceSystem &destCRS, double scaleDenominator ) const
  {
    Q_UNUSED( destCRS );
    QStringList layerKeys;
    QStringList::const_iterator llstIt;
    QStringList::const_iterator slstIt;
    QgsMapLayer* theMapLayer = nullptr;
    QgsMessageLog::logMessage( QStringLiteral( "Calculating layerset using %1 layers, %2 styles and CRS %3" ).arg( layersList.count() ).arg( stylesList.count() ).arg( destCRS.description() ) );
    for ( llstIt = layersList.begin(), slstIt = stylesList.begin(); llstIt != layersList.end(); ++llstIt )
    {
      QString styleName;
      if ( slstIt != stylesList.end() )
      {
        styleName = *slstIt;
      }
      QgsMessageLog::logMessage( "Trying to get layer " + *llstIt + "//" + styleName );

      //does the layer name appear several times in the layer list?
      //if yes, layer caching must be disabled because several named layers could have
      //several user styles
      bool allowCaching = true;
      if ( layersList.count( *llstIt ) > 1 )
      {
        allowCaching = false;
      }

      QList<QgsMapLayer*> layerList = mConfigParser->mapLayerFromStyle( *llstIt, styleName, allowCaching );
      int listIndex;

      for ( listIndex = layerList.size() - 1; listIndex >= 0; listIndex-- )
      {
        theMapLayer = layerList.at( listIndex );
        if ( theMapLayer )
        {
          QString lName =  theMapLayer->name();
          if ( mConfigParser && mConfigParser->useLayerIds() )
            lName = theMapLayer->id();
          else if ( !theMapLayer->shortName().isEmpty() )
            lName = theMapLayer->shortName();
          QgsMessageLog::logMessage( QStringLiteral( "Checking layer: %1" ).arg( lName ) );
          //test if layer is visible in requested scale
          bool useScaleConstraint = ( scaleDenominator > 0 && theMapLayer->hasScaleBasedVisibility() );
          if ( !useScaleConstraint ||
               ( theMapLayer->minimumScale() <= scaleDenominator && theMapLayer->maximumScale() >= scaleDenominator ) )
          {
            layerKeys.push_front( theMapLayer->id() );
            QgsProject::instance()->addMapLayers(
              QList<QgsMapLayer *>() << theMapLayer, false, false );
          }
        }
        else
        {
          QgsMessageLog::logMessage( QStringLiteral( "Layer or style not defined, aborting" ) );
          throw QgsMapServiceException( QStringLiteral( "LayerNotDefined" ), "Layer '" + *llstIt + "' and/or style '" + styleName + "' not defined" );
        }
      }

      if ( slstIt != stylesList.end() )
      {
        ++slstIt;
      }
    }
    return layerKeys;
  }


  void QgsWmsServer::applyRequestedLayerFilters( const QStringList& layerList , QgsMapSettings& mapSettings, QHash<QgsMapLayer*, QString>& originalFilters ) const
  {
    if ( layerList.isEmpty() )
    {
      return;
    }

    QString filterParameter = mParameters.value( QStringLiteral( "FILTER" ) );
    if ( !filterParameter.isEmpty() )
    {
      QStringList layerSplit = filterParameter.split( QStringLiteral( ";" ) );
      QStringList::const_iterator layerIt = layerSplit.constBegin();
      for ( ; layerIt != layerSplit.constEnd(); ++layerIt )
      {
        QStringList eqSplit = layerIt->split( QStringLiteral( ":" ) );
        if ( eqSplit.size() < 2 )
        {
          continue;
        }

        //filter string could be unsafe (danger of sql injection)
        if ( !testFilterStringSafety( eqSplit.at( 1 ) ) )
        {
          throw QgsMapServiceException( QStringLiteral( "Filter string rejected" ), "The filter string " + eqSplit.at( 1 ) +
                                        " has been rejected because of security reasons. Note: Text strings have to be enclosed in single or double quotes. " +
                                        "A space between each word / special character is mandatory. Allowed Keywords and special characters are " +
                                        "AND,OR,IN,<,>=,>,>=,!=,',',(,),DMETAPHONE,SOUNDEX. Not allowed are semicolons in the filter expression." );
        }

        //we need to find the maplayer objects matching the layer name
        QList<QgsMapLayer*> layersToFilter;

        Q_FOREACH ( QgsMapLayer *layer, QgsProject::instance()->mapLayers() )
        {
          if ( layer )
          {
            QString lName =  layer->name();
            if ( mConfigParser && mConfigParser->useLayerIds() )
              lName = layer->id();
            else if ( !layer->shortName().isEmpty() )
              lName = layer->shortName();
            if ( lName == eqSplit.at( 0 ) )
              layersToFilter.push_back( layer );
          }
        }

        Q_FOREACH ( QgsMapLayer *filter, layersToFilter )
        {
          QgsVectorLayer* filteredLayer = qobject_cast<QgsVectorLayer*>( filter );
          if ( filteredLayer )
          {
            originalFilters.insert( filteredLayer, filteredLayer->subsetString() );
            QString newSubsetString = eqSplit.at( 1 );
            if ( !filteredLayer->subsetString().isEmpty() )
            {
              newSubsetString.prepend( " AND " );
              newSubsetString.prepend( filteredLayer->subsetString() );
            }
            filteredLayer->setSubsetString( newSubsetString );
          }
        }
      }

      //No BBOX parameter in request. We use the union of the filtered layer
      //to provide the functionality of zooming to selected records via (enhanced) WMS.
      if ( mapSettings.extent().isEmpty() )
      {
        QgsRectangle filterExtent;
        QHash<QgsMapLayer*, QString>::const_iterator filterIt = originalFilters.constBegin();
        for ( ; filterIt != originalFilters.constEnd(); ++filterIt )
        {
          QgsMapLayer* mapLayer = filterIt.key();
          if ( !mapLayer )
          {
            continue;
          }

          QgsRectangle layerExtent = mapSettings.layerToMapCoordinates( mapLayer, mapLayer->extent() );
          if ( filterExtent.isEmpty() )
          {
            filterExtent = layerExtent;
          }
          else
          {
            filterExtent.combineExtentWith( layerExtent );
          }
        }
        mapSettings.setExtent( filterExtent );
      }
    }
  }

  void QgsWmsServer::applyAccessControlLayersFilters( const QStringList& layerList, QHash<QgsMapLayer*, QString>& originalLayerFilters ) const
  {
    Q_FOREACH ( const QString& layerName, layerList )
    {
      QList<QgsMapLayer*> mapLayers = QgsProject::instance()->mapLayersByName( layerName );
      Q_FOREACH ( QgsMapLayer* mapLayer, mapLayers )
      {
        QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( mAccessControl, mapLayer, originalLayerFilters );
      }
    }
  }

  bool QgsWmsServer::testFilterStringSafety( const QString& filter ) const
  {
    //; too dangerous for sql injections
    if ( filter.contains( QLatin1String( ";" ) ) )
    {
      return false;
    }

    QStringList tokens = filter.split( QStringLiteral( " " ), QString::SkipEmptyParts );
    groupStringList( tokens, QStringLiteral( "'" ) );
    groupStringList( tokens, QStringLiteral( "\"" ) );

    QStringList::const_iterator tokenIt = tokens.constBegin();
    for ( ; tokenIt != tokens.constEnd(); ++tokenIt )
    {
      //whitelist of allowed characters and keywords
      if ( tokenIt->compare( QLatin1String( "," ) ) == 0
           || tokenIt->compare( QLatin1String( "(" ) ) == 0
           || tokenIt->compare( QLatin1String( ")" ) ) == 0
           || tokenIt->compare( QLatin1String( "=" ) ) == 0
           || tokenIt->compare( QLatin1String( "!=" ) ) == 0
           || tokenIt->compare( QLatin1String( "<" ) ) == 0
           || tokenIt->compare( QLatin1String( "<=" ) ) == 0
           || tokenIt->compare( QLatin1String( ">" ) ) == 0
           || tokenIt->compare( QLatin1String( ">=" ) ) == 0
           || tokenIt->compare( QLatin1String( "%" ) ) == 0
           || tokenIt->compare( QLatin1String( "AND" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "OR" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "IN" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "LIKE" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "ILIKE" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "DMETAPHONE" ), Qt::CaseInsensitive ) == 0
           || tokenIt->compare( QLatin1String( "SOUNDEX" ), Qt::CaseInsensitive ) == 0 )
      {
        continue;
      }

      //numbers are ok
      bool isNumeric;
      tokenIt->toDouble( &isNumeric );
      if ( isNumeric )
      {
        continue;
      }

      //numeric strings need to be quoted once either with single or with double quotes

      //empty strings are ok
      if ( *tokenIt == QLatin1String( "''" ) )
      {
        continue;
      }

      //single quote
      if ( tokenIt->size() > 2
           && ( *tokenIt )[0] == QChar( '\'' )
           && ( *tokenIt )[tokenIt->size() - 1] == QChar( '\'' )
           && ( *tokenIt )[1] != QChar( '\'' )
           && ( *tokenIt )[tokenIt->size() - 2] != QChar( '\'' ) )
      {
        continue;
      }

      //double quote
      if ( tokenIt->size() > 2
           && ( *tokenIt )[0] == QChar( '"' )
           && ( *tokenIt )[tokenIt->size() - 1] == QChar( '"' )
           && ( *tokenIt )[1] != QChar( '"' )
           && ( *tokenIt )[tokenIt->size() - 2] != QChar( '"' ) )
      {
        continue;
      }

      return false;
    }

    return true;
  }

  void QgsWmsServer::groupStringList( QStringList& list, const QString& groupString )
  {
    //group contents within single quotes together
    bool groupActive = false;
    int startGroup = -1;
    QString concatString;

    for ( int i = 0; i < list.size(); ++i )
    {
      QString& str = list[i];
      if ( str.startsWith( groupString ) )
      {
        startGroup = i;
        groupActive = true;
        concatString.clear();
      }

      if ( groupActive )
      {
        if ( i != startGroup )
        {
          concatString.append( " " );
        }
        concatString.append( str );
      }

      if ( str.endsWith( groupString ) )
      {
        int endGroup = i;
        groupActive = false;

        if ( startGroup != -1 )
        {
          list[startGroup] = concatString;
          for ( int j = startGroup + 1; j <= endGroup; ++j )
          {
            list.removeAt( startGroup + 1 );
            --i;
          }
        }

        concatString.clear();
        startGroup = -1;
      }
    }
  }

  QStringList QgsWmsServer::applyFeatureSelections( const QStringList& layerList ) const
  {
    QStringList layersWithSelections;
    if ( layerList.isEmpty() )
    {
      return layersWithSelections;
    }

    QString selectionString = mParameters.value( QStringLiteral( "SELECTION" ) );
    if ( selectionString.isEmpty() )
    {
      return layersWithSelections;
    }

    Q_FOREACH ( const QString& selectionLayer, selectionString.split( ";" ) )
    {
      //separate layer name from id list
      QStringList layerIdSplit = selectionLayer.split( QStringLiteral( ":" ) );
      if ( layerIdSplit.size() < 2 )
      {
        continue;
      }

      //find layerId for layer name
      QString layerName = layerIdSplit.at( 0 );
      QgsVectorLayer* vLayer = nullptr;

      Q_FOREACH ( QgsMapLayer *layer, QgsProject::instance()->mapLayers() )
      {
        if ( layer )
        {
          QString lName =  layer->name();
          if ( mConfigParser && mConfigParser->useLayerIds() )
            lName = layer->id();
          else if ( !layer->shortName().isEmpty() )
            lName = layer->shortName();
          if ( lName == layerName )
          {
            vLayer = qobject_cast<QgsVectorLayer*>( layer );
            layersWithSelections.push_back( vLayer->id() );
            break;
          }
        }
      }

      if ( !vLayer )
      {
        continue;
      }

      QStringList idList = layerIdSplit.at( 1 ).split( QStringLiteral( "," ) );
      QgsFeatureIds selectedIds;

      Q_FOREACH ( const QString& id, idList )
      {
        selectedIds.insert( STRING_TO_FID( id ) );
      }

      vLayer->selectByIds( selectedIds );
    }


    return layersWithSelections;
  }

  void QgsWmsServer::clearFeatureSelections( const QStringList& layerIds ) const
  {
    const QMap<QString, QgsMapLayer*>& layerMap = QgsProject::instance()->mapLayers();

    Q_FOREACH ( const QString& id, layerIds )
    {
      QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( layerMap.value( id, nullptr ) );
      if ( !layer )
        continue;

      layer->selectByIds( QgsFeatureIds() );
    }

    return;
  }

  void QgsWmsServer::applyOpacities( const QStringList& layerList, QList< QPair< QgsVectorLayer*, QgsFeatureRenderer*> >& vectorRenderers,
                                     QList< QPair< QgsRasterLayer*, QgsRasterRenderer* > >& rasterRenderers,
                                     QList< QPair< QgsVectorLayer*, double > >& labelTransparencies,
                                     QList< QPair< QgsVectorLayer*, double > >& labelBufferTransparencies )
  {
    //get opacity list
    QMap<QString, QString>::const_iterator opIt = mParameters.constFind( QStringLiteral( "OPACITIES" ) );
    if ( opIt == mParameters.constEnd() )
    {
      return;
    }
    QStringList opacityList = opIt.value().split( QStringLiteral( "," ) );

    //collect leaf layers and their opacity
    QVector< QPair< QgsMapLayer*, int > > layerOpacityList;
    QStringList::const_iterator oIt = opacityList.constBegin();
    QStringList::const_iterator lIt = layerList.constBegin();
    for ( ; oIt != opacityList.constEnd() && lIt != layerList.constEnd(); ++oIt, ++lIt )
    {
      //get layer list for
      int opacity = oIt->toInt();
      if ( opacity < 0 || opacity > 255 )
      {
        continue;
      }
      QList<QgsMapLayer*> llist = mConfigParser->mapLayerFromStyle( *lIt, QLatin1String( "" ) );
      QList<QgsMapLayer*>::const_iterator lListIt = llist.constBegin();
      for ( ; lListIt != llist.constEnd(); ++lListIt )
      {
        layerOpacityList.push_back( qMakePair( *lListIt, opacity ) );
      }
    }

    QVector< QPair< QgsMapLayer*, int > >::const_iterator lOpIt = layerOpacityList.constBegin();
    for ( ; lOpIt != layerOpacityList.constEnd(); ++lOpIt )
    {
      //vector or raster?
      QgsMapLayer* ml = lOpIt->first;
      int opacity = lOpIt->second;
      double opacityRatio = opacity / 255.0; //opacity value between 0 and 1

      if ( !ml || opacity == 255 )
      {
        continue;
      }

      if ( ml->type() == QgsMapLayer::VectorLayer )
      {
        QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( ml );

        QgsFeatureRenderer* renderer = vl->renderer();
        //backup old renderer
        vectorRenderers.push_back( qMakePair( vl, renderer->clone() ) );
        //modify symbols of current renderer
        QgsRenderContext context;
        context.expressionContext().appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( vl ) );

        QgsSymbolList symbolList = renderer->symbols( context );
        QgsSymbolList::iterator symbolIt = symbolList.begin();
        for ( ; symbolIt != symbolList.end(); ++symbolIt )
        {
          ( *symbolIt )->setAlpha(( *symbolIt )->alpha() * opacityRatio );
        }

        //labeling
        if ( vl->customProperty( QStringLiteral( "labeling/enabled" ) ).toString() == QLatin1String( "true" ) )
        {
          double labelTransparency = vl->customProperty( QStringLiteral( "labeling/textTransp" ) ).toDouble();
          labelTransparencies.push_back( qMakePair( vl, labelTransparency ) );
          vl->setCustomProperty( QStringLiteral( "labeling/textTransp" ), labelTransparency + ( 100 - labelTransparency ) * ( 1.0 - opacityRatio ) );
          double bufferTransparency = vl->customProperty( QStringLiteral( "labeling/bufferTransp" ) ).toDouble();
          labelBufferTransparencies.push_back( qMakePair( vl, bufferTransparency ) );
          vl->setCustomProperty( QStringLiteral( "labeling/bufferTransp" ), bufferTransparency + ( 100 - bufferTransparency )* ( 1.0 - opacityRatio ) );
        }
      }
      else if ( ml->type() == QgsMapLayer::RasterLayer )
      {
        QgsRasterLayer* rl = qobject_cast<QgsRasterLayer*>( ml );
        if ( rl )
        {
          QgsRasterRenderer* rasterRenderer = rl->renderer();
          if ( rasterRenderer )
          {
            rasterRenderers.push_back( qMakePair( rl, rasterRenderer->clone() ) );
            rasterRenderer->setOpacity( rasterRenderer->opacity() * opacityRatio );
          }
        }
      }
    }
  }

  void QgsWmsServer::restoreOpacities( QList< QPair< QgsVectorLayer*, QgsFeatureRenderer*> >& vectorRenderers,
                                       QList < QPair< QgsRasterLayer*, QgsRasterRenderer* > >& rasterRenderers,
                                       QList< QPair< QgsVectorLayer*, double > >& labelOpacities,
                                       QList< QPair< QgsVectorLayer*, double > >& labelBufferOpacities )
  {
    if ( vectorRenderers.isEmpty() && rasterRenderers.isEmpty() )
    {
      return;
    }

    QList< QPair< QgsVectorLayer*, QgsFeatureRenderer*> >::iterator vIt = vectorRenderers.begin();
    for ( ; vIt != vectorRenderers.end(); ++vIt )
    {
      ( *vIt ).first->setRenderer(( *vIt ).second );
    }

    QList< QPair< QgsRasterLayer*, QgsRasterRenderer* > >::iterator rIt = rasterRenderers.begin();
    for ( ; rIt != rasterRenderers.end(); ++rIt )
    {
      ( *rIt ).first->setRenderer(( *rIt ).second );
    }

    QList< QPair< QgsVectorLayer*, double > >::iterator loIt = labelOpacities.begin();
    for ( ; loIt != labelOpacities.end(); ++loIt )
    {
      ( *loIt ).first->setCustomProperty( QStringLiteral( "labeling/textTransp" ), ( *loIt ).second );
    }

    QList< QPair< QgsVectorLayer*, double > >::iterator lboIt = labelBufferOpacities.begin();
    for ( ; lboIt != labelBufferOpacities.end(); ++lboIt )
    {
      ( *lboIt ).first->setCustomProperty( QStringLiteral( "labeling/bufferTransp" ), ( *lboIt ).second );
    }
  }

  bool QgsWmsServer::checkMaximumWidthHeight() const
  {
    //test if maxWidth / maxHeight set and WIDTH / HEIGHT parameter is in the range
    if ( mConfigParser->maxWidth() != -1 )
    {
      QMap<QString, QString>::const_iterator widthIt = mParameters.find( QStringLiteral( "WIDTH" ) );
      if ( widthIt != mParameters.constEnd() )
      {
        if ( widthIt->toInt() > mConfigParser->maxWidth() )
        {
          return false;
        }
      }
    }
    if ( mConfigParser->maxHeight() != -1 )
    {
      QMap<QString, QString>::const_iterator heightIt = mParameters.find( QStringLiteral( "HEIGHT" ) );
      if ( heightIt != mParameters.constEnd() )
      {
        if ( heightIt->toInt() > mConfigParser->maxHeight() )
        {
          return false;
        }
      }
    }
    return true;
  }

  QString QgsWmsServer::serviceUrl() const
  {
    QString requestUri = getenv( "REQUEST_URI" );
    if ( requestUri.isEmpty() )
    {
      // in some cases (e.g. when running through python's CGIHTTPServer) the REQUEST_URI is not defined
      requestUri = QString( getenv( "SCRIPT_NAME" ) ) + "?" + QString( getenv( "QUERY_STRING" ) );
    }

    QUrl mapUrl( requestUri );
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
      else if ( queryIt->first.compare( QLatin1String( "LAYERS" ), Qt::CaseInsensitive ) == 0 )
      {
        mapUrl.removeQueryItem( queryIt->first );
      }
      else if ( queryIt->first.compare( QLatin1String( "SLD_VERSION" ), Qt::CaseInsensitive ) == 0 )
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

  void QgsWmsServer::addXmlDeclaration( QDomDocument& doc ) const
  {
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction( QStringLiteral( "xml" ), QStringLiteral( "version=\"1.0\" encoding=\"utf-8\"" ) );
    doc.appendChild( xmlDeclaration );
  }

  void QgsWmsServer::convertFeatureInfoToSIA2045( QDomDocument& doc )
  {
    QDomDocument SIAInfoDoc;
    QDomElement infoDocElement = doc.documentElement();
    QDomElement SIAInfoDocElement = SIAInfoDoc.importNode( infoDocElement, false ).toElement();
    SIAInfoDoc.appendChild( SIAInfoDocElement );

    QString currentAttributeName;
    QString currentAttributeValue;
    QDomElement currentAttributeElem;
    QString currentLayerName;
    QDomElement currentLayerElem;
    QDomNodeList layerNodeList = infoDocElement.elementsByTagName( QStringLiteral( "Layer" ) );
    for ( int i = 0; i < layerNodeList.size(); ++i )
    {
      currentLayerElem = layerNodeList.at( i ).toElement();
      currentLayerName = currentLayerElem.attribute( QStringLiteral( "name" ) );

      QDomElement currentFeatureElem;

      QDomNodeList featureList = currentLayerElem.elementsByTagName( QStringLiteral( "Feature" ) );
      if ( featureList.size() < 1 )
      {
        //raster?
        QDomNodeList attributeList = currentLayerElem.elementsByTagName( QStringLiteral( "Attribute" ) );
        QDomElement rasterLayerElem;
        if ( !attributeList.isEmpty() )
        {
          rasterLayerElem = SIAInfoDoc.createElement( currentLayerName );
        }
        for ( int j = 0; j < attributeList.size(); ++j )
        {
          currentAttributeElem = attributeList.at( j ).toElement();
          currentAttributeName = currentAttributeElem.attribute( QStringLiteral( "name" ) );
          currentAttributeValue = currentAttributeElem.attribute( QStringLiteral( "value" ) );
          QDomElement outAttributeElem = SIAInfoDoc.createElement( currentAttributeName );
          QDomText outAttributeText = SIAInfoDoc.createTextNode( currentAttributeValue );
          outAttributeElem.appendChild( outAttributeText );
          rasterLayerElem.appendChild( outAttributeElem );
        }
        if ( !attributeList.isEmpty() )
        {
          SIAInfoDocElement.appendChild( rasterLayerElem );
        }
      }
      else //vector
      {
        //property attributes
        QSet<QString> layerPropertyAttributes;
        QString currentLayerId = currentLayerElem.attribute( QStringLiteral( "id" ) );
        if ( !currentLayerId.isEmpty() )
        {
          QgsMapLayer* currentLayer = QgsProject::instance()->mapLayer( currentLayerId );
          if ( currentLayer )
          {
            QString WMSPropertyAttributesString = currentLayer->customProperty( QStringLiteral( "WMSPropertyAttributes" ) ).toString();
            if ( !WMSPropertyAttributesString.isEmpty() )
            {
              QStringList propertyList = WMSPropertyAttributesString.split( QStringLiteral( "//" ) );
              QStringList::const_iterator propertyIt = propertyList.constBegin();
              for ( ; propertyIt != propertyList.constEnd(); ++propertyIt )
              {
                layerPropertyAttributes.insert( *propertyIt );
              }
            }
          }
        }

        QDomElement propertyRefChild; //child to insert the next property after (or
        for ( int j = 0; j < featureList.size(); ++j )
        {
          QDomElement SIAFeatureElem = SIAInfoDoc.createElement( currentLayerName );
          currentFeatureElem = featureList.at( j ).toElement();
          QDomNodeList attributeList = currentFeatureElem.elementsByTagName( QStringLiteral( "Attribute" ) );

          for ( int k = 0; k < attributeList.size(); ++k )
          {
            currentAttributeElem = attributeList.at( k ).toElement();
            currentAttributeName = currentAttributeElem.attribute( QStringLiteral( "name" ) );
            currentAttributeValue = currentAttributeElem.attribute( QStringLiteral( "value" ) );
            if ( layerPropertyAttributes.contains( currentAttributeName ) )
            {
              QDomElement propertyElem = SIAInfoDoc.createElement( QStringLiteral( "property" ) );
              QDomElement identifierElem = SIAInfoDoc.createElement( QStringLiteral( "identifier" ) );
              QDomText identifierText = SIAInfoDoc.createTextNode( currentAttributeName );
              identifierElem.appendChild( identifierText );
              QDomElement valueElem = SIAInfoDoc.createElement( QStringLiteral( "value" ) );
              QDomText valueText = SIAInfoDoc.createTextNode( currentAttributeValue );
              valueElem.appendChild( valueText );
              propertyElem.appendChild( identifierElem );
              propertyElem.appendChild( valueElem );
              if ( propertyRefChild.isNull() )
              {
                SIAFeatureElem.insertBefore( propertyElem, QDomNode() );
                propertyRefChild = propertyElem;
              }
              else
              {
                SIAFeatureElem.insertAfter( propertyElem, propertyRefChild );
              }
            }
            else
            {
              QDomElement SIAAttributeElem = SIAInfoDoc.createElement( currentAttributeName );
              QDomText SIAAttributeText = SIAInfoDoc.createTextNode( currentAttributeValue );
              SIAAttributeElem.appendChild( SIAAttributeText );
              SIAFeatureElem.appendChild( SIAAttributeElem );
            }
          }
          SIAInfoDocElement.appendChild( SIAFeatureElem );
        }
      }
    }
    doc = SIAInfoDoc;
  }

  QDomElement QgsWmsServer::createFeatureGML(
    QgsFeature* feat,
    QgsVectorLayer* layer,
    QDomDocument& doc,
    QgsCoordinateReferenceSystem& crs,
    const QgsMapSettings& mapSettings,
    const QString& typeName,
    bool withGeom,
    int version,
    QStringList* attributes ) const
  {
    //qgs:%TYPENAME%
    QDomElement typeNameElement = doc.createElement( "qgs:" + typeName /*qgs:%TYPENAME%*/ );
    typeNameElement.setAttribute( QStringLiteral( "fid" ), typeName + "." + QString::number( feat->id() ) );

    QgsCoordinateTransform transform;
    if ( layer && layer->crs() != crs )
    {
      transform = mapSettings.layerTransform( layer );
    }

    QgsGeometry geom = feat->geometry();

    QgsExpressionContext expressionContext;
    expressionContext << QgsExpressionContextUtils::globalScope()
    << QgsExpressionContextUtils::projectScope( QgsProject::instance() );
    if ( layer )
      expressionContext << QgsExpressionContextUtils::layerScope( layer );
    expressionContext.setFeature( *feat );

    // always add bounding box info if feature contains geometry
    if ( !geom.isEmpty() && geom.type() != QgsWkbTypes::UnknownGeometry && geom.type() != QgsWkbTypes::NullGeometry )
    {
      QgsRectangle box = feat->geometry().boundingBox();
      if ( transform.isValid() )
      {
        try
        {
          QgsRectangle transformedBox = transform.transformBoundingBox( box );
          box = transformedBox;
        }
        catch ( QgsCsException &e )
        {
          QgsMessageLog::logMessage( QStringLiteral( "Transform error caught: %1" ).arg( e.what() ) );
        }
      }

      QDomElement bbElem = doc.createElement( QStringLiteral( "gml:boundedBy" ) );
      QDomElement boxElem;
      if ( version < 3 )
      {
        boxElem = QgsOgcUtils::rectangleToGMLBox( &box, doc, 8 );
      }
      else
      {
        boxElem = QgsOgcUtils::rectangleToGMLEnvelope( &box, doc, 8 );
      }

      if ( crs.isValid() )
      {
        boxElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
      }
      bbElem.appendChild( boxElem );
      typeNameElement.appendChild( bbElem );
    }

    if ( withGeom && !geom.isEmpty() )
    {
      //add geometry column (as gml)

      if ( transform.isValid() )
      {
        geom.transform( transform );
      }

      QDomElement geomElem = doc.createElement( QStringLiteral( "qgs:geometry" ) );
      QDomElement gmlElem;
      if ( version < 3 )
      {
        gmlElem = QgsOgcUtils::geometryToGML( &geom, doc, 8 );
      }
      else
      {
        gmlElem = QgsOgcUtils::geometryToGML( &geom, doc, QStringLiteral( "GML3" ), 8 );
      }

      if ( !gmlElem.isNull() )
      {
        if ( crs.isValid() )
        {
          gmlElem.setAttribute( QStringLiteral( "srsName" ), crs.authid() );
        }
        geomElem.appendChild( gmlElem );
        typeNameElement.appendChild( geomElem );
      }
    }

    //read all allowed attribute values from the feature
    QgsAttributes featureAttributes = feat->attributes();
    QgsFields fields = feat->fields();
    for ( int i = 0; i < fields.count(); ++i )
    {
      QString attributeName = fields.at( i ).name();
      //skip attribute if it is explicitly excluded from WMS publication
      if ( layer && layer->excludeAttributesWms().contains( attributeName ) )
      {
        continue;
      }
      //skip attribute if it is excluded by access control
      if ( attributes && !attributes->contains( attributeName ) )
      {
        continue;
      }

      QDomElement fieldElem = doc.createElement( "qgs:" + attributeName.replace( QStringLiteral( " " ), QStringLiteral( "_" ) ) );
      QString fieldTextString = featureAttributes.at( i ).toString();
      if ( layer )
      {
        fieldTextString = replaceValueMapAndRelation( layer, i, QgsExpression::replaceExpressionText( fieldTextString, &expressionContext ) );
      }
      QDomText fieldText = doc.createTextNode( fieldTextString );
      fieldElem.appendChild( fieldText );
      typeNameElement.appendChild( fieldElem );
    }

    //add maptip attribute based on html/expression (in case there is no maptip attribute)
    if ( layer )
    {
      QString mapTip = layer->mapTipTemplate();

      if ( !mapTip.isEmpty() )
      {
        QString fieldTextString = QgsExpression::replaceExpressionText( mapTip, &expressionContext );
        QDomElement fieldElem = doc.createElement( QStringLiteral( "qgs:maptip" ) );
        QDomText maptipText = doc.createTextNode( fieldTextString );
        fieldElem.appendChild( maptipText );
        typeNameElement.appendChild( fieldElem );
      }
    }

    return typeNameElement;
  }

  QString QgsWmsServer::replaceValueMapAndRelation( QgsVectorLayer* vl, int idx, const QString& attributeVal )
  {
    const QgsEditorWidgetSetup setup = QgsEditorWidgetRegistry::instance()->findBest( vl, vl->fields().field( idx ).name() );
    QgsFieldFormatter* fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
    QString value( fieldFormatter->representValue( vl, idx, setup.config(), QVariant(), attributeVal ) );

    if ( setup.config().value( QStringLiteral( "AllowMulti" ) ).toBool() && value.startsWith( QLatin1String( "{" ) ) && value.endsWith( QLatin1String( "}" ) ) )
    {
      value = value.mid( 1, value.size() - 2 );
    }
    return value;
  }

  int QgsWmsServer::getImageQuality() const
  {

    // First taken from QGIS project
    int imageQuality = mConfigParser->imageQuality();

    // Then checks if a parameter is given, if so use it instead
    if ( mParameters.contains( QStringLiteral( "IMAGE_QUALITY" ) ) )
    {
      bool conversionSuccess;
      int imageQualityParameter;
      imageQualityParameter = mParameters[ QStringLiteral( "IMAGE_QUALITY" )].toInt( &conversionSuccess );
      if ( conversionSuccess )
      {
        imageQuality = imageQualityParameter;
      }
    }
    return imageQuality;
  }

  int QgsWmsServer::getWMSPrecision( int defaultValue = 8 ) const
  {
    // First taken from QGIS project
    int WMSPrecision = mConfigParser->wmsPrecision();

    // Then checks if a parameter is given, if so use it instead
    if ( mParameters.contains( QStringLiteral( "WMS_PRECISION" ) ) )
    {
      bool conversionSuccess;
      int WMSPrecisionParameter;
      WMSPrecisionParameter = mParameters[ QStringLiteral( "WMS_PRECISION" )].toInt( &conversionSuccess );
      if ( conversionSuccess )
      {
        WMSPrecision = WMSPrecisionParameter;
      }
    }
    if ( WMSPrecision == -1 )
    {
      WMSPrecision = defaultValue;
    }
    return WMSPrecision;
  }

  QgsRectangle QgsWmsServer::featureInfoSearchRect( QgsVectorLayer* ml, const QgsMapSettings& mapSettings, const QgsRenderContext& rct, const QgsPoint& infoPoint ) const
  {
    if ( !ml )
    {
      return QgsRectangle();
    }

    double mapUnitTolerance = 0.0;
    if ( ml->geometryType() == QgsWkbTypes::PolygonGeometry )
    {
      QMap<QString, QString>::const_iterator tolIt = mParameters.find( QStringLiteral( "FI_POLYGON_TOLERANCE" ) );
      if ( tolIt != mParameters.constEnd() )
      {
        mapUnitTolerance = tolIt.value().toInt() * rct.mapToPixel().mapUnitsPerPixel();
      }
      else
      {
        mapUnitTolerance = mapSettings.extent().width() / 400.0;
      }
    }
    else if ( ml->geometryType() == QgsWkbTypes::LineGeometry )
    {
      QMap<QString, QString>::const_iterator tolIt = mParameters.find( QStringLiteral( "FI_LINE_TOLERANCE" ) );
      if ( tolIt != mParameters.constEnd() )
      {
        mapUnitTolerance = tolIt.value().toInt() * rct.mapToPixel().mapUnitsPerPixel();
      }
      else
      {
        mapUnitTolerance = mapSettings.extent().width() / 200.0;
      }
    }
    else //points
    {
      QMap<QString, QString>::const_iterator tolIt = mParameters.find( QStringLiteral( "FI_POINT_TOLERANCE" ) );
      if ( tolIt != mParameters.constEnd() )
      {
        mapUnitTolerance = tolIt.value().toInt() * rct.mapToPixel().mapUnitsPerPixel();
      }
      else
      {
        mapUnitTolerance = mapSettings.extent().width() / 100.0;
      }
    }

    QgsRectangle mapRectangle( infoPoint.x() - mapUnitTolerance, infoPoint.y() - mapUnitTolerance,
                               infoPoint.x() + mapUnitTolerance, infoPoint.y() + mapUnitTolerance );
    return( mapSettings.mapToLayerCoordinates( ml, mapRectangle ) );
  }


} // namespace QgsWms

