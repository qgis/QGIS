/***************************************************************************
                              qgswcsserver.cpp
                              -------------------
  begin                : December 9, 2013
  copyright            : (C) 2013 by René-Luc D'Hont
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
#include "qgswcsserver.h"
#include "qgswcsprojectparser.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsrasterpipe.h"
#include "qgsrasterprojector.h"
#include "qgsrasterfilewriter.h"
#include "qgslogger.h"
#include "qgsmapserviceexception.h"
#include "qgsaccesscontrol.h"

#include <QTemporaryFile>
#include <QUrl>

#ifndef Q_OS_WIN
#include <netinet/in.h>
#else
#include <winsock.h>
#endif

static const QString WCS_NAMESPACE = QStringLiteral( "http://www.opengis.net/wcs" );
static const QString GML_NAMESPACE = QStringLiteral( "http://www.opengis.net/gml" );
static const QString OGC_NAMESPACE = QStringLiteral( "http://www.opengis.net/ogc" );

QgsWCSServer::QgsWCSServer(
  const QString& configFilePath
  , QMap<QString, QString> &parameters
  , QgsWCSProjectParser* pp
  , QgsRequestHandler* rh
  , const QgsProject* project
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  , const QgsAccessControl* accessControl
#endif
)
    : QgsOWSServer(
      configFilePath
      , parameters
      , rh
      , project
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      , accessControl
#endif
    )
    , mConfigParser(
      pp
    )
{
}

void QgsWCSServer::executeRequest()
{
  //request type
  QString request = mParameters.value( QStringLiteral( "REQUEST" ) );
  if ( request.isEmpty() )
  {
    //do some error handling
    QgsDebugMsg( "unable to find 'REQUEST' parameter, exiting..." );
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
    QgsDebugMsg( "seting GetCapabilities response" );
    mRequestHandler->setGetCapabilitiesResponse( capabilitiesDocument );
    return;
  }
  else if ( request.compare( QLatin1String( "DescribeCoverage" ), Qt::CaseInsensitive ) == 0 )
  {
    QDomDocument describeDocument;
    try
    {
      describeDocument = describeCoverage();
    }
    catch ( QgsMapServiceException& ex )
    {
      mRequestHandler->setServiceException( ex );
      return;
    }
    QgsDebugMsg( "seting GetCapabilities response" );
    mRequestHandler->setGetCapabilitiesResponse( describeDocument );
    return;
  }
  else if ( request.compare( QLatin1String( "GetCoverage" ), Qt::CaseInsensitive ) == 0 )
  {
    QByteArray* coverageOutput;
    try
    {
      coverageOutput = getCoverage();
    }
    catch ( QgsMapServiceException& ex )
    {
      mRequestHandler->setServiceException( ex );
      return;
    }
    if ( coverageOutput )
    {
      mRequestHandler->setGetCoverageResponse( coverageOutput );
    }
    return;
  }
}

QDomDocument QgsWCSServer::getCapabilities()
{
  QgsDebugMsg( "Entering." );
  QDomDocument doc;

  //wcs:WCS_Capabilities element
  QDomElement wcsCapabilitiesElement = doc.createElement( QStringLiteral( "WCS_Capabilities" )/*wcs:WCS_Capabilities*/ );
  wcsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns" ), WCS_NAMESPACE );
  wcsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
  wcsCapabilitiesElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), WCS_NAMESPACE + " http://schemas.opengis.net/wcs/1.0.0/wcsCapabilities.xsd" );
  wcsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:gml" ), GML_NAMESPACE );
  wcsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
  wcsCapabilitiesElement.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0.0" ) );
  wcsCapabilitiesElement.setAttribute( QStringLiteral( "updateSequence" ), QStringLiteral( "0" ) );
  doc.appendChild( wcsCapabilitiesElement );

  if ( mConfigParser )
  {
    mConfigParser->serviceCapabilities( wcsCapabilitiesElement, doc );
  }

  //INSERT Service

  //wcs:Capability element
  QDomElement capabilityElement = doc.createElement( QStringLiteral( "Capability" )/*wcs:Capability*/ );
  wcsCapabilitiesElement.appendChild( capabilityElement );

  //wcs:Request element
  QDomElement requestElement = doc.createElement( QStringLiteral( "Request" )/*wcs:Request*/ );
  capabilityElement.appendChild( requestElement );

  //wcs:GetCapabilities
  QDomElement getCapabilitiesElement = doc.createElement( QStringLiteral( "GetCapabilities" )/*wcs:GetCapabilities*/ );
  requestElement.appendChild( getCapabilitiesElement );

  QDomElement dcpTypeElement = doc.createElement( QStringLiteral( "DCPType" )/*wcs:DCPType*/ );
  getCapabilitiesElement.appendChild( dcpTypeElement );
  QDomElement httpElement = doc.createElement( QStringLiteral( "HTTP" )/*wcs:HTTP*/ );
  dcpTypeElement.appendChild( httpElement );

  //Prepare url
  QString hrefString = QgsServerProjectUtils::wcsServiceUrl( *mProject );
  if ( hrefString.isEmpty() )
  {
    hrefString = QgsServerProjectUtils::wmsServiceUrl( *mProject );
  }
  if ( hrefString.isEmpty() )
  {
    hrefString = serviceUrl();
  }

  QDomElement getElement = doc.createElement( QStringLiteral( "Get" )/*wcs:Get*/ );
  httpElement.appendChild( getElement );
  QDomElement onlineResourceElement = doc.createElement( QStringLiteral( "OnlineResource" )/*wcs:OnlineResource*/ );
  onlineResourceElement.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
  onlineResourceElement.setAttribute( QStringLiteral( "xlink:href" ), hrefString );
  getElement.appendChild( onlineResourceElement );

  QDomElement getCapabilitiesDhcTypePostElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  getCapabilitiesDhcTypePostElement.firstChild().firstChild().toElement().setTagName( QStringLiteral( "Post" ) );
  getCapabilitiesElement.appendChild( getCapabilitiesDhcTypePostElement );

  QDomElement describeCoverageElement = getCapabilitiesElement.cloneNode().toElement();//this is the same as 'GetCapabilities'
  describeCoverageElement.setTagName( QStringLiteral( "DescribeCoverage" ) );
  requestElement.appendChild( describeCoverageElement );

  QDomElement getCoverageElement = getCapabilitiesElement.cloneNode().toElement();//this is the same as 'GetCapabilities'
  getCoverageElement.setTagName( QStringLiteral( "GetCoverage" ) );
  requestElement.appendChild( getCoverageElement );

  /*
   * Adding layer list in ContentMetadata
   */
  QDomElement contentMetadataElement = doc.createElement( QStringLiteral( "ContentMetadata" )/*wcs:ContentMetadata*/ );
  wcsCapabilitiesElement.appendChild( contentMetadataElement );
  /*
   * Adding layer list in contentMetadataElement
   */
  if ( mConfigParser )
  {
    mConfigParser->wcsContentMetadata( contentMetadataElement, doc );
  }

  return doc;
}

QDomDocument QgsWCSServer::describeCoverage()
{
  QgsDebugMsg( "Entering." );
  QDomDocument doc;

  //wcs:WCS_Capabilities element
  QDomElement coveDescElement = doc.createElement( QStringLiteral( "CoverageDescription" )/*wcs:CoverageDescription*/ );
  coveDescElement.setAttribute( QStringLiteral( "xmlns" ), WCS_NAMESPACE );
  coveDescElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
  coveDescElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), WCS_NAMESPACE + " http://schemas.opengis.net/wcs/1.0.0/describeCoverage.xsd" );
  coveDescElement.setAttribute( QStringLiteral( "xmlns:gml" ), GML_NAMESPACE );
  coveDescElement.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
  coveDescElement.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0.0" ) );
  coveDescElement.setAttribute( QStringLiteral( "updateSequence" ), QStringLiteral( "0" ) );
  doc.appendChild( coveDescElement );

  //defining coverage name
  QString coveName = QLatin1String( "" );
  //read COVERAGE
  QMap<QString, QString>::const_iterator cove_name_it = mParameters.constFind( QStringLiteral( "COVERAGE" ) );
  if ( cove_name_it != mParameters.constEnd() )
  {
    coveName = cove_name_it.value();
  }
  if ( coveName == QLatin1String( "" ) )
  {
    QMap<QString, QString>::const_iterator cove_name_it = mParameters.constFind( QStringLiteral( "IDENTIFIER" ) );
    if ( cove_name_it != mParameters.constEnd() )
    {
      coveName = cove_name_it.value();
    }
  }
  mConfigParser->describeCoverage( coveName, coveDescElement, doc );
  return doc;
}

QByteArray* QgsWCSServer::getCoverage()
{
  QStringList wcsLayersId = mConfigParser->wcsLayers();

  QList<QgsMapLayer*> layerList;

  QStringList mErrors = QStringList();

  //defining coverage name
  QString coveName = QLatin1String( "" );
  //read COVERAGE
  QMap<QString, QString>::const_iterator cove_name_it = mParameters.constFind( QStringLiteral( "COVERAGE" ) );
  if ( cove_name_it != mParameters.constEnd() )
  {
    coveName = cove_name_it.value();
  }
  if ( coveName == QLatin1String( "" ) )
  {
    QMap<QString, QString>::const_iterator cove_name_it = mParameters.constFind( QStringLiteral( "IDENTIFIER" ) );
    if ( cove_name_it != mParameters.constEnd() )
    {
      coveName = cove_name_it.value();
    }
  }

  if ( coveName == QLatin1String( "" ) )
  {
    mErrors << QStringLiteral( "COVERAGE is mandatory" );
  }

  layerList = mConfigParser->mapLayerFromCoverage( coveName );
  if ( layerList.size() < 1 )
  {
    mErrors << QStringLiteral( "The layer for the COVERAGE '%1' is not found" ).arg( coveName );
  }

  bool conversionSuccess;
  // BBOX
  bool bboxOk = false;
  double minx = 0.0, miny = 0.0, maxx = 0.0, maxy = 0.0;
  // WIDTh and HEIGHT
  int width = 0, height = 0;
  // CRS
  QString crs = QLatin1String( "" );

  // read BBOX
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
    bboxOk = true;
    QString bbString = bbIt.value();
    minx = bbString.section( QStringLiteral( "," ), 0, 0 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess ) {bboxOk = false;}
    miny = bbString.section( QStringLiteral( "," ), 1, 1 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess ) {bboxOk = false;}
    maxx = bbString.section( QStringLiteral( "," ), 2, 2 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess ) {bboxOk = false;}
    maxy = bbString.section( QStringLiteral( "," ), 3, 3 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess ) {bboxOk = false;}
  }
  if ( !bboxOk )
  {
    mErrors << QStringLiteral( "The BBOX is mandatory and has to be xx.xxx,yy.yyy,xx.xxx,yy.yyy" );
  }

  // read WIDTH
  width = mParameters.value( QStringLiteral( "WIDTH" ), QStringLiteral( "0" ) ).toInt( &conversionSuccess );
  if ( !conversionSuccess )
    width = 0;
  // read HEIGHT
  height = mParameters.value( QStringLiteral( "HEIGHT" ), QStringLiteral( "0" ) ).toInt( &conversionSuccess );
  if ( !conversionSuccess )
  {
    height = 0;
  }

  if ( width < 0 || height < 0 )
  {
    mErrors << QStringLiteral( "The WIDTH and HEIGHT are mandatory and have to be integer" );
  }

  crs = mParameters.value( QStringLiteral( "CRS" ), QLatin1String( "" ) );
  if ( crs == QLatin1String( "" ) )
  {
    mErrors << QStringLiteral( "The CRS is mandatory" );
  }

  if ( mErrors.count() != 0 )
  {
    throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), mErrors.join( QStringLiteral( ". " ) ) );
  }

  QgsCoordinateReferenceSystem requestCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs );
  if ( !requestCRS.isValid() )
  {
    mErrors << QStringLiteral( "Could not create request CRS" );
    throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), mErrors.join( QStringLiteral( ". " ) ) );
  }

  QgsRectangle rect( minx, miny, maxx, maxy );

  QgsMapLayer* layer = layerList.at( 0 );
  QgsRasterLayer* rLayer = qobject_cast<QgsRasterLayer*>( layer );
  if ( rLayer && wcsLayersId.contains( rLayer->id() ) )
  {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    if ( !mAccessControl->layerReadPermission( rLayer ) )
    {
      throw QgsMapServiceException( QStringLiteral( "Security" ), QStringLiteral( "You are not allowed to access to this coverage" ) );
    }
#endif

    // RESPONSE_CRS
    QgsCoordinateReferenceSystem responseCRS = rLayer->crs();
    crs = mParameters.value( QStringLiteral( "RESPONSE_CRS" ), QLatin1String( "" ) );
    if ( crs != QLatin1String( "" ) )
    {
      responseCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs );
      if ( !responseCRS.isValid() )
      {
        responseCRS = rLayer->crs();
      }
    }

    // transform rect
    if ( requestCRS != rLayer->crs() )
    {
      QgsCoordinateTransform t( requestCRS, rLayer->crs() );
      rect = t.transformBoundingBox( rect );
    }

    QTemporaryFile tempFile;
    tempFile.open();
    QgsRasterFileWriter fileWriter( tempFile.fileName() );

    // clone pipe/provider
    QgsRasterPipe* pipe = new QgsRasterPipe();
    if ( !pipe->set( rLayer->dataProvider()->clone() ) )
    {
      mErrors << QStringLiteral( "Cannot set pipe provider" );
      throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), mErrors.join( QStringLiteral( ". " ) ) );
    }

    // add projector if necessary
    if ( responseCRS != rLayer->crs() )
    {
      QgsRasterProjector * projector = new QgsRasterProjector;
      projector->setCrs( rLayer->crs(), responseCRS );
      if ( !pipe->insert( 2, projector ) )
      {
        mErrors << QStringLiteral( "Cannot set pipe projector" );
        throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), mErrors.join( QStringLiteral( ". " ) ) );
      }
    }

    QgsRasterFileWriter::WriterError err = fileWriter.writeRaster( pipe, width, height, rect, responseCRS );
    if ( err != QgsRasterFileWriter::NoError )
    {
      mErrors << QStringLiteral( "Cannot write raster error code: %1" ).arg( err );
      throw QgsMapServiceException( QStringLiteral( "RequestNotWellFormed" ), mErrors.join( QStringLiteral( ". " ) ) );
    }
    delete pipe;
    QByteArray* ba = nullptr;
    ba = new QByteArray();
    *ba = tempFile.readAll();

    return ba;
  }
  return nullptr;
}

QString QgsWCSServer::serviceUrl() const
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
