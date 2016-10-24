/***************************************************************************
                              qgssoaprequesthandler.cpp
                            Handles the SOAP communication
                            ------------------------------
  begin                :  2006
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
#include "qgssoaprequesthandler.h"
#include "qgslogger.h"
#include "qgsmapserviceexception.h"
#include <QBuffer>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QImage>
#include <QTextStream>
#include <time.h>
#include <fcgi_stdio.h>

QgsSOAPRequestHandler::QgsSOAPRequestHandler( const bool captureOutput )
    : QgsHttpRequestHandler( captureOutput )
{
}

QgsSOAPRequestHandler::~QgsSOAPRequestHandler()
{
}

void QgsSOAPRequestHandler::parseInput()
{
  QString inputString = readPostBody();

  //QgsDebugMsg("input string is: " + inputString)

  //inputString to QDomDocument
  QDomDocument inputXML;
  QString errorMsg;
  if ( !inputXML.setContent( inputString, true, &errorMsg ) )
  {
    QgsDebugMsg( "soap request parse error" );
    QgsDebugMsg( "error message: " + errorMsg );
    QgsDebugMsg( "the xml string was:" );
    QgsDebugMsg( inputString );
    throw QgsMapServiceException( QStringLiteral( "InvalidXML" ), "XML error: " + errorMsg );
  }

  // if xml reading was successfull, save the inputXML in a file
  QFile soapFile;
  QTextStream soapStream;

  //go through soap envelope->soap body, search for either GetCapabilities or GetMap
  QDomNodeList envelopeNodeList = inputXML.elementsByTagNameNS( QStringLiteral( "http://schemas.xmlsoap.org/soap/envelope/" ), QStringLiteral( "Envelope" ) );
  if ( envelopeNodeList.size() < 1 )
  {
    QgsDebugMsg( "Envelope element not found" );
    throw QgsMapServiceException( QStringLiteral( "SOAPError" ), QStringLiteral( "Element <Envelope> not found" ) );
  }

  QDomNodeList bodyNodeList = envelopeNodeList.item( 0 ).toElement().elementsByTagNameNS( QStringLiteral( "http://schemas.xmlsoap.org/soap/envelope/" ), QStringLiteral( "Body" ) );
  if ( bodyNodeList.size() < 1 )
  {
    QgsDebugMsg( "body node not found" );
    throw QgsMapServiceException( QStringLiteral( "SOAPError" ), QStringLiteral( "Element <Body> not found" ) );
  }
  QDomElement bodyElement = bodyNodeList.item( 0 ).toElement();
  QDomElement firstChildElement = bodyElement.firstChild().toElement();

  QString serviceString = firstChildElement.attribute( QStringLiteral( "service" ) );
  if ( serviceString == QLatin1String( "MS" ) )
  {
    QgsDebugMsg( "service = MS " );
    mParameterMap.insert( QStringLiteral( "SERVICE" ), QStringLiteral( "MS" ) );
    mService = QStringLiteral( "MS" );
  }
  else if ( serviceString == QLatin1String( "WMS" ) )
  {
    mParameterMap.insert( QStringLiteral( "SERVICE" ), QStringLiteral( "WMS" ) );
    mService = QStringLiteral( "WMS" );
  }
  else if ( serviceString == QLatin1String( "MDS" ) )
  {
    mParameterMap.insert( QStringLiteral( "SERVICE" ), QStringLiteral( "MDS" ) );
    mService = QStringLiteral( "MDS" );
  }
  else if ( serviceString == QLatin1String( "MAS" ) )
  {
    mParameterMap.insert( QStringLiteral( "SERVICE" ), QStringLiteral( "MAS" ) );
    mService = QStringLiteral( "MAS" );
  }
  else
  {
    mParameterMap.insert( QStringLiteral( "SERVICE" ), QStringLiteral( "DISCOVERY" ) );
    mService = QStringLiteral( "DISCOVERY" );
  }


  //GetCapabilities request
  //if(firstChildElement.localName().compare("getCapabilities", Qt::CaseInsensitive) == 0)
  if ( firstChildElement.localName() == QLatin1String( "GetCapabilities" ) || firstChildElement.localName() == QLatin1String( "getCapabilities" ) )
  {
    mParameterMap.insert( QStringLiteral( "REQUEST" ), QStringLiteral( "GetCapabilities" ) );
  }
  //GetMap request
  //else if(firstChildElement.tagName().compare("getMap",Qt::CaseInsensitive) == 0)
  else if ( firstChildElement.localName() == QLatin1String( "GetMap" ) || firstChildElement.localName() == QLatin1String( "getMap" ) )
  {
    mParameterMap.insert( QStringLiteral( "REQUEST" ), QStringLiteral( "GetMap" ) );
    parseGetMapElement( mParameterMap, firstChildElement );
  }
  //GetDiagram request
  //else if(firstChildElement.tagName().compare("getDiagram", Qt::CaseInsensitive) == 0)
  else if ( firstChildElement.localName() == QLatin1String( "GetDiagram" ) )
  {
    mParameterMap.insert( QStringLiteral( "REQUEST" ), QStringLiteral( "GetDiagram" ) );
    parseGetMapElement( mParameterMap, firstChildElement ); //reuse the method for GetMap
  }
  //GetFeatureInfo request
  else if ( firstChildElement.localName() == QLatin1String( "GetFeatureInfo" ) )
  {
    mParameterMap.insert( QStringLiteral( "REQUEST" ), QStringLiteral( "GetFeatureInfo" ) );
    parseGetFeatureInfoElement( mParameterMap, firstChildElement );
  }

  //set mFormat
  QString formatString = mParameterMap.value( QStringLiteral( "FORMAT" ) );
  if ( !formatString.isEmpty() )
  {
    //remove the image/ in front of the format
    if ( formatString == QLatin1String( "image/jpeg" ) || formatString == QLatin1String( "JPG" ) || formatString == QLatin1String( "jpg" ) )
    {
      formatString = QStringLiteral( "JPG" );
    }
    else if ( formatString == QLatin1String( "image/png" ) || formatString == QLatin1String( "PNG" ) || formatString == QLatin1String( "png" ) )
    {
      formatString = QStringLiteral( "PNG" );
    }
    else if ( formatString == QLatin1String( "image/gif" ) || formatString == QLatin1String( "GIF" ) || formatString == QLatin1String( "gif" ) )
    {
      formatString = QStringLiteral( "GIF" );
    }
    else
    {
      throw QgsMapServiceException( QStringLiteral( "InvalidFormat" ), "Invalid format " + formatString + ", only jpg and png are supported" );
    }

    mFormat = formatString;
  }
}

void QgsSOAPRequestHandler::setGetMapResponse( const QString& service, QImage* img )
{
  QgsMapServiceException ex( QStringLiteral( "set error" ), QStringLiteral( "Error, could not set Image" ) );
  if ( service == QLatin1String( "WMS" ) )
  {
    if ( setUrlToFile( img ) != 0 )
    {
      setServiceException( ex );
    }
  }
  else if ( service == QLatin1String( "MAS" ) || service == QLatin1String( "MS" ) || service == QLatin1String( "MDS" ) )
  {

    if ( setSOAPWithAttachments( img ) != 0 )
    {
      setServiceException( ex );
    }
  }
  else
  {
    setServiceException( ex );
  }
}

void QgsSOAPRequestHandler::setGetCapabilitiesResponse( const QDomDocument& doc )
{
  //Parse the QDomDocument Document and create a SOAP response
  QDomElement DocCapabilitiesElement = doc.firstChildElement();
  if ( !DocCapabilitiesElement.isNull() )
  {
    QDomNodeList capabilitiesNodes =  DocCapabilitiesElement.elementsByTagName( QStringLiteral( "Capability" ) );
    if ( !capabilitiesNodes.isEmpty() )
    {

      //create response document
      QDomDocument soapResponseDoc;
      //Envelope element
      QDomElement soapEnvelopeElement = soapResponseDoc.createElement( QStringLiteral( "soap:Envelope" ) );
      soapEnvelopeElement.setAttribute( QStringLiteral( "xmlns:soap" ), QStringLiteral( "http://schemas.xmlsoap.org/soap/envelope/" ) );
      soapEnvelopeElement.setAttribute( QStringLiteral( "soap:encoding" ), QStringLiteral( "http://schemas.xmlsoap.org/soap/encoding/" ) );
      soapResponseDoc.appendChild( soapEnvelopeElement );

      //Body element
      QDomElement soapBodyElement = soapResponseDoc.createElementNS( QStringLiteral( "http://schemas.xmlsoap.org/soap/envelope/" ), QStringLiteral( "soap:Body" ) );
      soapEnvelopeElement.appendChild( soapBodyElement );

      // check if WMS or MS SOAP request

      if ( mService == QLatin1String( "MS" ) || mService == QLatin1String( "MDS" ) || mService == QLatin1String( "MAS" ) )
      {
        //OA_MI_MS_Capabilities element
        QDomElement msCapabilitiesElement = soapResponseDoc.createElement( QStringLiteral( "ms:OA_MI_Service_Capabilities" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "service" ), QStringLiteral( "MS" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.1" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:ms" ), QStringLiteral( "http://www.eu-orchestra.org/services/ms" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "xmlns" ), QStringLiteral( "http://www.eu-orchestra.org/services/oas/oa_basic" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:gml" ), QStringLiteral( "http://www.opengis.net/gml" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xsd" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema" ) );
        soapBodyElement.appendChild( msCapabilitiesElement );

        // import the orchestra common capabilities
        QFile common( QStringLiteral( "common.xml" ) );
        if ( !common.open( QIODevice::ReadOnly ) )
        {
          //throw an exception...
          QgsDebugMsg( "external orchestra common capabilities not found" );
        }
        else
        {
          QDomDocument externCapDoc;
          QString parseError;
          int errorLineNo;
          if ( !externCapDoc.setContent( &common, false, &parseError, &errorLineNo ) )
          {
            QgsDebugMsg( "parse error at setting content of external orchestra common capabilities: "
                         + parseError + " at line " + QString::number( errorLineNo ) );
            common.close();
          }
          common.close();

          // write common capabilities
          QDomElement orchestraCommon = externCapDoc.firstChildElement();
          msCapabilitiesElement.appendChild( orchestraCommon );

          // write specific capabilities
          QDomElement msSpecificCapabilitiesElement = soapResponseDoc.createElement( QStringLiteral( "serviceSpecificCapabilities" ) );
          soapBodyElement.appendChild( msSpecificCapabilitiesElement );

          QDomElement capabilitiesElement = capabilitiesNodes.item( 0 ).toElement();
          msSpecificCapabilitiesElement.appendChild( capabilitiesElement );

          // to do supportedOperations
          QDomNodeList requestNodes = capabilitiesElement.elementsByTagName( QStringLiteral( "Request" ) );
          if ( !requestNodes.isEmpty() )
          {
            QDomElement requestElement = requestNodes.item( 0 ).toElement();
            QDomNodeList requestChildNodes = requestElement.childNodes();

            //append an array element for 'supportedOperations' to the soap document
            QDomElement supportedOperationsElement = soapResponseDoc.createElement( QStringLiteral( "supportedOperations" ) );
            supportedOperationsElement.setAttribute( QStringLiteral( "xsi:type" ), QStringLiteral( "soapenc:Array" ) );
            supportedOperationsElement.setAttribute( QStringLiteral( "soap:arrayType" ), "xsd:string[" + QString::number( requestChildNodes.size() ) + "]" );

            for ( int i = 0; i < requestChildNodes.size(); ++i )
            {
              QDomElement itemElement = soapResponseDoc.createElement( QStringLiteral( "item" ) );
              QDomText itemText = soapResponseDoc.createTextNode( requestChildNodes.item( i ).toElement().tagName() );
              itemElement.appendChild( itemText );
              supportedOperationsElement.appendChild( itemElement );
            }
          }
          //operationTypeInfo
          //predefinedLayers
          //predefinedDimensions
          //layerTypeInfo
          //predefinedStyles
          //supportedLayerDataInputFormat
          //supportedExceptionFormats
          //supportedDCP
        }


      }
      else if ( mService == QLatin1String( "WMS" ) )
      {
        //WMS_Capabilities element
        QDomElement msCapabilitiesElement = soapResponseDoc.createElement( QStringLiteral( "wms:Capabilities" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "service" ), QStringLiteral( "WMS" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.3.0" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:wms" ), QStringLiteral( "http://www.opengis.net/wms" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:gml" ), QStringLiteral( "http://www.opengis.net/gml" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xsd" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema" ) );
        soapBodyElement.appendChild( msCapabilitiesElement );

        QFile wmsService( QStringLiteral( "wms_metadata.xml" ) );
        if ( !wmsService.open( QIODevice::ReadOnly ) )
        {
          //throw an exception...
          QgsDebugMsg( "external wms service capabilities not found" );
        }
        else
        {
          QDomDocument externServiceDoc;
          QString parseError;
          int errorLineNo;
          if ( !externServiceDoc.setContent( &wmsService, false, &parseError, &errorLineNo ) )
          {
            QgsDebugMsg( "parse error at setting content of external wms service capabilities: "
                         + parseError + " at line " + QString::number( errorLineNo ) );
            wmsService.close();
          }
          wmsService.close();

          // write WMS Service capabilities
          QDomElement service = externServiceDoc.firstChildElement();
          msCapabilitiesElement.appendChild( service );
        }

        QDomElement msServiceElement = soapResponseDoc.createElement( QStringLiteral( "Service" ) );
        msCapabilitiesElement.appendChild( msServiceElement );

        QDomElement capabilitiesElement = capabilitiesNodes.item( 0 ).toElement();
        msCapabilitiesElement.appendChild( capabilitiesElement );



      }
      else
      {
        //OA_MI_MS_Capabilities element
        QDomElement msCapabilitiesElement = soapResponseDoc.createElement( QStringLiteral( "ms:OA_MI_Service_Capabilities" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "service" ), QStringLiteral( "MS" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.1" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:ms" ), QStringLiteral( "http://www.eu-orchestra.org/services/ms" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "xmlns" ), QStringLiteral( "http://www.eu-orchestra.org/services/oas/oa_basic" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:gml" ), QStringLiteral( "http://www.opengis.net/gml" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
        msCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xsd" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema" ) );
        soapBodyElement.appendChild( msCapabilitiesElement );

        // import the orchestra common capabilities
        QFile common( QStringLiteral( "common.xml" ) );
        if ( !common.open( QIODevice::ReadOnly ) )
        {
          //throw an exception...
          QgsDebugMsg( "external orchestra common capabilities not found" );
        }
        else
        {
          QDomDocument externCapDoc;
          QString parseError;
          int errorLineNo;
          if ( !externCapDoc.setContent( &common, false, &parseError, &errorLineNo ) )
          {
            QgsDebugMsg( "parse error at setting content of external orchestra common capabilities: "
                         + parseError + " at line " + QString::number( errorLineNo ) );
            common.close();
          }
          common.close();

          // write common capabilities
          QDomElement orchestraCommon = externCapDoc.firstChildElement();
          msCapabilitiesElement.appendChild( orchestraCommon );
        }
      }

      QByteArray ba = soapResponseDoc.toByteArray();
      setHttpResponse( &ba, QStringLiteral( "text/xml" ) );
    }
  }
}

void QgsSOAPRequestHandler::setGetFeatureInfoResponse( const QDomDocument& infoDoc, const QString& infoFormat )
{
  Q_UNUSED( infoFormat );
  QDomDocument featureInfoResponseDoc;

  //Envelope
  QDomElement soapEnvelopeElement = featureInfoResponseDoc.createElement( QStringLiteral( "soap:Envelope" ) );
  soapEnvelopeElement.setAttribute( QStringLiteral( "xmlns:soap" ), QStringLiteral( "http://schemas.xmlsoap.org/soap/envelope/" ) );
  soapEnvelopeElement.setAttribute( QStringLiteral( "soap:encoding" ), QStringLiteral( "http://schemas.xmlsoap.org/soap/encoding/" ) );
  featureInfoResponseDoc.appendChild( soapEnvelopeElement );

  //Body
  QDomElement soapBodyElement = featureInfoResponseDoc.createElementNS( QStringLiteral( "http://schemas.xmlsoap.org/soap/envelope/" ), QStringLiteral( "soap:Body" ) );
  soapEnvelopeElement.appendChild( soapBodyElement );

  soapBodyElement.appendChild( infoDoc.documentElement() );

  //now set message
  QByteArray ba = featureInfoResponseDoc.toByteArray();
  setHttpResponse( &ba, QStringLiteral( "text/xml" ) );
}

void QgsSOAPRequestHandler::setXmlResponse( const QDomDocument& infoDoc )
{
  QDomDocument featureInfoResponseDoc;

  //Envelope
  QDomElement soapEnvelopeElement = featureInfoResponseDoc.createElement( QStringLiteral( "soap:Envelope" ) );
  soapEnvelopeElement.setAttribute( QStringLiteral( "xmlns:soap" ), QStringLiteral( "http://schemas.xmlsoap.org/soap/envelope/" ) );
  soapEnvelopeElement.setAttribute( QStringLiteral( "soap:encoding" ), QStringLiteral( "http://schemas.xmlsoap.org/soap/encoding/" ) );
  featureInfoResponseDoc.appendChild( soapEnvelopeElement );

  //Body
  QDomElement soapBodyElement = featureInfoResponseDoc.createElementNS( QStringLiteral( "http://schemas.xmlsoap.org/soap/envelope/" ), QStringLiteral( "soap:Body" ) );
  soapEnvelopeElement.appendChild( soapBodyElement );

  soapBodyElement.appendChild( infoDoc.documentElement() );

  //now set message
  QByteArray ba = featureInfoResponseDoc.toByteArray();
  setHttpResponse( &ba, QStringLiteral( "text/xml" ) );
}

void QgsSOAPRequestHandler::setXmlResponse( const QDomDocument& infoDoc, const QString& mimeType )
{
  Q_UNUSED( mimeType );
  setXmlResponse( infoDoc );
}

void QgsSOAPRequestHandler::setGetPrintResponse( QByteArray* ba )
{
  Q_UNUSED( ba );
  //soon...
}

void QgsSOAPRequestHandler::setServiceException( const QgsMapServiceException& ex )
{
  //create response document
  QDomDocument soapResponseDoc;
  //Envelope element
  QDomElement soapEnvelopeElement = soapResponseDoc.createElement( QStringLiteral( "soap:Envelope" ) );
  soapEnvelopeElement.setAttribute( QStringLiteral( "xmlns:soap" ), QStringLiteral( "http://schemas.xmlsoap.org/soap/envelope/" ) );
  soapEnvelopeElement.setAttribute( QStringLiteral( "soap:encoding" ), QStringLiteral( "http://schemas.xmlsoap.org/soap/encoding/" ) );
  soapResponseDoc.appendChild( soapEnvelopeElement );

  //Body element
  QDomElement soapBodyElement = soapResponseDoc.createElementNS( QStringLiteral( "http://schemas.xmlsoap.org/soap/envelope/" ), QStringLiteral( "soap:Body" ) );
  soapEnvelopeElement.appendChild( soapBodyElement );

  QDomElement msExceptionsElement = soapResponseDoc.createElement( QStringLiteral( "Exception" ) );
  msExceptionsElement.setAttribute( QStringLiteral( "format" ), QStringLiteral( "text/xml" ) );
  soapBodyElement.appendChild( msExceptionsElement );

  QDomText msExceptionMessage = soapResponseDoc.createTextNode( QString( ex.message() ) );
  msExceptionsElement.appendChild( msExceptionMessage );

  QByteArray ba = soapResponseDoc.toByteArray();
  setHttpResponse( &ba, QStringLiteral( "text/xml" ) );
}

int QgsSOAPRequestHandler::parseGetMapElement( QMap<QString, QString>& parameterMap, const QDomElement& getMapElement ) const
{
  QDomNodeList boundingBoxList = getMapElement.elementsByTagName( QStringLiteral( "BoundingBox" ) );
  if ( !boundingBoxList.isEmpty() )
  {
    parseBoundingBoxElement( parameterMap, boundingBoxList.item( 0 ).toElement() );
  }
  QDomNodeList CRSList = getMapElement.elementsByTagName( QStringLiteral( "coordinateReferenceSystem" ) );
  if ( !CRSList.isEmpty() )
  {
    QString crsText = CRSList.item( 0 ).toElement().text();
    QString epsgNumber;
    if ( !crsText.startsWith( QLatin1String( "EPSG_" ) ) ) //use underscore in SOAP because ':' is reserved for namespaces
    {
      //error
    }
    else
    {
      epsgNumber = crsText.replace( 4, 1, QStringLiteral( ":" ) );//replace the underscore with a ':' to make it WMS compatible
    }
    parameterMap.insert( QStringLiteral( "CRS" ), epsgNumber );
  }
  QDomNodeList GMLList = getMapElement.elementsByTagNameNS( QStringLiteral( "http://www.eu-orchestra.org/services/ms" ), QStringLiteral( "GML" ) );
  if ( !GMLList.isEmpty() )
  {
    QString gmlText;
    QTextStream gmlStream( &gmlText );
    GMLList.at( 0 ).save( gmlStream, 2 );
    parameterMap.insert( QStringLiteral( "GML" ), gmlText );
  }

  //outputAttributes
  QDomNodeList imageDocumentAttributesList = getMapElement.elementsByTagName( QStringLiteral( "Output" ) );
  if ( !imageDocumentAttributesList.isEmpty() )
  {
    parseOutputAttributesElement( parameterMap, imageDocumentAttributesList.item( 0 ).toElement() );
  }

  //SLD
  QDomNodeList sldList = getMapElement.elementsByTagName( QStringLiteral( "StyledLayerDescriptor" ) );
  if ( !sldList.isEmpty() )
  {
    QString sldString;
    QTextStream sldStream( &sldString );
    sldList.item( 0 ).save( sldStream, 0 );
    //Replace some special characters
    sldString.replace( QLatin1String( "&lt;" ), QLatin1String( "<" ) );
    sldString.replace( QLatin1String( "&gt;" ), QLatin1String( ">" ) );
    parameterMap.insert( QStringLiteral( "SLD" ), sldString );
  }

  return 0;
}

int QgsSOAPRequestHandler::parseGetFeatureInfoElement( QMap<QString, QString>& parameterMap, const QDomElement& getFeatureInfoElement ) const
{
  QDomNodeList queryList = getFeatureInfoElement.elementsByTagName( QStringLiteral( "Query" ) );
  if ( queryList.size() < 1 )
  {
    return 1;
  }
  QDomElement queryElem = queryList.at( 0 ).toElement();

  //find <QueryLayer>
  QDomNodeList queryLayerList = queryElem.elementsByTagName( QStringLiteral( "QueryLayer" ) );
  if ( queryLayerList.size() < 1 )
  {
    return 0; //no error, but nothing to do
  }
  QString queryLayerString = queryLayerList.at( 0 ).toElement().text();
  parameterMap.insert( QStringLiteral( "QUERY_LAYERS" ), queryLayerString );

  //find <XImagePoint>
  QDomNodeList xImageList = queryElem.elementsByTagName( QStringLiteral( "XImagePoint" ) );
  if ( xImageList.size() < 1 )
  {
    return 2; //mandatory
  }
  bool conversionSuccess;
  int xPoint = xImageList.at( 0 ).toElement().text().toInt( &conversionSuccess );
  if ( !conversionSuccess )
  {
    return 4;
  }
  parameterMap.insert( QStringLiteral( "I" ), QString::number( xPoint ) );

  //find <YImagePoint>
  QDomNodeList yImageList = queryElem.elementsByTagName( QStringLiteral( "YImagePoint" ) );
  if ( yImageList.size() < 1 )
  {
    return 5; //mandatory
  }
  int yPoint = xImageList.at( 0 ).toElement().text().toInt( &conversionSuccess );
  if ( !conversionSuccess )
  {
    return 6;
  }
  parameterMap.insert( QStringLiteral( "J" ), QString::number( yPoint ) );

  //find <FeatureCount>
  QDomNodeList featureCountList = queryElem.elementsByTagName( QStringLiteral( "FeatureCount" ) );
  if ( !featureCountList.isEmpty() ) //optional
  {
    int featureCount = featureCountList.at( 0 ).toElement().text().toInt( &conversionSuccess );
    if ( conversionSuccess )
    {
      parameterMap.insert( QStringLiteral( "FEATURE_COUNT" ), QString::number( featureCount ) );
    }
  }

  //RequestCopy
  QDomNodeList requestCopyList = getFeatureInfoElement.elementsByTagName( QStringLiteral( "RequestCopy" ) );
  if ( requestCopyList.size() < 1 )
  {
    return 7;
  }

  if ( parseGetMapElement( parameterMap, requestCopyList.at( 0 ).toElement() ) != 0 )
  {
    return 8;
  }

  return 0;
}

int QgsSOAPRequestHandler::parseBoundingBoxElement( QMap<QString, QString>& parameterMap, const QDomElement& boundingBoxElement ) const
{
  QString minx, miny, maxx, maxy;

  //leftBound
  QDomNodeList leftBoundList = boundingBoxElement.elementsByTagName( QStringLiteral( "leftBound" ) );
  if ( !leftBoundList.isEmpty() )
  {
    minx = leftBoundList.item( 0 ).toElement().text();
  }

  //rightBound
  QDomNodeList rightBoundList = boundingBoxElement.elementsByTagName( QStringLiteral( "rightBound" ) );
  if ( !rightBoundList.isEmpty() )
  {
    maxx = rightBoundList.item( 0 ).toElement().text();
  }

  //lowerBound
  QDomNodeList lowerBoundList = boundingBoxElement.elementsByTagName( QStringLiteral( "lowerBound" ) );
  if ( !lowerBoundList.isEmpty() )
  {
    miny = lowerBoundList.item( 0 ).toElement().text();
  }

  //upperBound
  QDomNodeList upperBoundList = boundingBoxElement.elementsByTagName( QStringLiteral( "upperBound" ) );
  if ( !upperBoundList.isEmpty() )
  {
    maxy = upperBoundList.item( 0 ).toElement().text();
  }
  parameterMap.insert( QStringLiteral( "BBOX" ), minx + "," + miny + "," + maxx + "," + maxy );
  return 0;
}

int QgsSOAPRequestHandler::parseOutputAttributesElement( QMap<QString, QString>& parameterMap, const QDomElement& outputAttributesElement ) const
{
  //height
  QDomNodeList heightList = outputAttributesElement.elementsByTagName( QStringLiteral( "Height" ) );
  if ( !heightList.isEmpty() )
  {
    QString heightString = heightList.item( 0 ).toElement().text();
    parameterMap.insert( QStringLiteral( "HEIGHT" ), heightString );
  }

  //width
  QDomNodeList widthList = outputAttributesElement.elementsByTagName( QStringLiteral( "Width" ) );
  if ( !widthList.isEmpty() )
  {
    QString widthString = widthList.item( 0 ).toElement().text();
    parameterMap.insert( QStringLiteral( "WIDTH" ), widthString );
  }

  //format
  QDomNodeList formatList = outputAttributesElement.elementsByTagName( QStringLiteral( "Format" ) );
  if ( !formatList.isEmpty() )
  {
    QString formatString = formatList.item( 0 ).toElement().text();
    parameterMap.insert( QStringLiteral( "FORMAT" ), formatString );
  }

  //background transparendy
  QDomNodeList bgTransparencyList = outputAttributesElement.elementsByTagName/*NS*/( /*"http://www.eu-orchestra.org/services/ms",*/ QStringLiteral( "Transparent" ) );
  if ( !bgTransparencyList.isEmpty() )
  {
    QString bgTransparencyString = bgTransparencyList.item( 0 ).toElement().text();
    if ( bgTransparencyString.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0
         || bgTransparencyString == QLatin1String( "1" ) )
    {
      parameterMap.insert( QStringLiteral( "TRANSPARENT" ), QStringLiteral( "TRUE" ) );
    }
    else
    {
      parameterMap.insert( QStringLiteral( "TRANSPARENT" ), QStringLiteral( "FALSE" ) );
    }
  }
  return 0;
}

int QgsSOAPRequestHandler::setSOAPWithAttachments( QImage* img )
{
  QgsDebugMsg( "Entering." );
  //create response xml document
  QDomDocument xmlResponse; //response xml, save this into mimeString
  QDomElement envelopeElement = xmlResponse.createElementNS( QStringLiteral( "http://schemas.xmlsoap.org/soap/envelope/" ), QStringLiteral( "Envelope" ) );
  xmlResponse.appendChild( envelopeElement );
  QDomElement bodyElement = xmlResponse.createElementNS( QStringLiteral( "http://schemas.xmlsoap.org/soap/envelope/" ), QStringLiteral( "Body" ) );
  envelopeElement.appendChild( bodyElement );
  QDomElement getMapResponseElement = xmlResponse.createElementNS( QStringLiteral( "http://www.eu-orchestra.org/services/ms" ), QStringLiteral( "getMapResponse" ) );
  bodyElement.appendChild( getMapResponseElement );
  QDomElement returnElement = xmlResponse.createElementNS( QStringLiteral( "http://www.eu-orchestra.org/services/ms" ), QStringLiteral( "return" ) );
  returnElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
  returnElement.setAttribute( QStringLiteral( "xsi:type" ), QStringLiteral( "OA_ImageDocument" ) );
  returnElement.setAttribute( QStringLiteral( "href" ), QStringLiteral( "image@mapservice" ) );
  getMapResponseElement.appendChild( returnElement );

  //create image buffer
  QByteArray ba;
  QBuffer buffer( &ba );
  buffer.open( QIODevice::WriteOnly );
  img->save( &buffer, mFormat.toLocal8Bit().data(), -1 ); // writes image into ba

  QByteArray xmlByteArray = xmlResponse.toString().toLocal8Bit();

  // Set headers
  setHeader( QStringLiteral( "MIME-Version" ), QStringLiteral( "1.0" ) );
  setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "Multipart/Related; boundary=\"MIME_boundary\"; type=\"text/xml\"; start=\"<xml@mapservice>\"" ) );
  // Start body
  appendBody( "--MIME_boundary\r\n" );
  appendBody( "Content-Type: text/xml\n" );
  appendBody( "Content-ID: <xml@mapservice>\n" );
  appendBody( "\n" );
  appendBody( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );
  appendBody( xmlByteArray );
  appendBody( "\n" );
  appendBody( "\r\n" );
  appendBody( "--MIME_boundary\r\n" );
  if ( mFormat == QLatin1String( "JPG" ) )
  {
    appendBody( "Content-Type: image/jpg\n" );
  }
  else if ( mFormat == QLatin1String( "PNG" ) )
  {
    appendBody( "Content-Type: image/png\n" );
  }
  appendBody( "Content-Transfer-Encoding: binary\n" );
  appendBody( "Content-ID: <image@mapservice>\n" );
  appendBody( "\n" );
  appendBody( ba );
  appendBody( "\r\n" );
  appendBody( "--MIME_boundary\r\n" );

  return 0;
}

int QgsSOAPRequestHandler::setUrlToFile( QImage* img )
{
  QString uri;
  QFile theFile;
  QDir tmpDir;

  QgsDebugMsg( "Entering." );

  if ( findOutHostAddress( uri ) != 0 )
  {
    return 1;
  }

#ifdef Q_OS_WIN
  if ( !QFile::exists( QDir::currentPath() + "/tmp" ) )
  {
    QDir::current().mkdir( "tmp" );
  }
  tmpDir = QDir( QDir::currentPath() + "/tmp" );
#else // Q_OS_WIN
  tmpDir = QDir( QStringLiteral( "/tmp" ) );
#endif

  QgsDebugMsg( "Path to tmpDir is: " + tmpDir.absolutePath() );

  //create a random folder under /tmp with map.jpg/png in it
  //and return the link to the client
  srand( time( nullptr ) );
  int randomNumber = rand();
  QString folderName = QString::number( randomNumber );
  if ( !QFile::exists( tmpDir.absolutePath() + "/mas_tmp" ) )
  {
    QgsDebugMsg( "Trying to create mas_tmp folder" );
    if ( !tmpDir.mkdir( QStringLiteral( "mas_tmp" ) ) )
    {
      return 2;
    }
  }
  QDir tmpMasDir( tmpDir.absolutePath() + "/mas_tmp" );
  if ( !tmpMasDir.mkdir( folderName ) )
  {
    QgsDebugMsg( "Trying to create random folder" );
    return 3;
  }

  QgsDebugMsg( "Temp. folder is: " + tmpMasDir.absolutePath() + "/" + folderName );

  if ( mFormat == QLatin1String( "JPG" ) )
  {
    theFile.setFileName( tmpMasDir.absolutePath() + "/" + folderName + "/map.jpg" );
    uri.append( "/mas_tmp/" + folderName + "/map.jpg" );
  }
  else if ( mFormat == QLatin1String( "PNG" ) )
  {
    theFile.setFileName( tmpMasDir.absolutePath() + "/" + folderName + "/map.png" );
    uri.append( "/mas_tmp/" + folderName + "/map.png" );
  }
  if ( !theFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    QgsDebugMsg( "Error, could not open file" );
    return 4;
  }

  if ( !img->save( &theFile, mFormat.toLocal8Bit().data(), -1 ) )
  {
    QgsDebugMsg( "Error, could not save image" );
    return 5;
  }

  QDomDocument xmlResponse;
  QDomElement envelopeElement = xmlResponse.createElementNS( QStringLiteral( "http://schemas.xmlsoap.org/soap/envelope/" ), QStringLiteral( "Envelope" ) );
  xmlResponse.appendChild( envelopeElement );
  QDomElement bodyElement = xmlResponse.createElementNS( QStringLiteral( "http://schemas.xmlsoap.org/soap/envelope/" ), QStringLiteral( "Body" ) );
  envelopeElement.appendChild( bodyElement );
  QDomElement getMapResponseElement = xmlResponse.createElementNS( QStringLiteral( "http://www.eu-orchestra.org/services/ms" ), QStringLiteral( "getMapResponse" ) );
  QDomElement ahrefElement = xmlResponse.createElement( QStringLiteral( "a" ) );
  ahrefElement.setAttribute( QStringLiteral( "href" ), uri );
  //QString href("<a href=\""+uri+"\">"+uri+"</a>");
  QDomText uriNode = xmlResponse.createTextNode( uri );
  ahrefElement.appendChild( uriNode );
  //getMapResponseElement.appendChild(uriNode);
  getMapResponseElement.appendChild( ahrefElement );
  bodyElement.appendChild( getMapResponseElement );

  QByteArray xmlByteArray = xmlResponse.toByteArray();
  setHttpResponse( &xmlByteArray, QStringLiteral( "text/xml" ) );
  return 0;
}

int QgsSOAPRequestHandler::findOutHostAddress( QString& address ) const
{
  QDomDocument wmsMetadataDoc;
  QFile wmsMetadataFile( QStringLiteral( "wms_metadata.xml" ) );

  if ( !wmsMetadataFile.open( QIODevice::ReadOnly ) )
  {
    return 1;
  }
  if ( !wmsMetadataDoc.setContent( &wmsMetadataFile, true ) )
  {
    return 2;
  }
  QDomNodeList onlineResourceList = wmsMetadataDoc.elementsByTagName( QStringLiteral( "OnlineResource" ) );
  if ( onlineResourceList.size() < 1 )
  {
    return 3;
  }
  address = onlineResourceList.at( 0 ).toElement().attribute( QStringLiteral( "href" ) );
  QgsDebugMsg( "address found: " + address );
  if ( address.isEmpty() )
  {
    return 4;
  }
  return 0;
}
