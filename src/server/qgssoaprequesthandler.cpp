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
    throw QgsMapServiceException( "InvalidXML", "XML error: " + errorMsg );
  }

  // if xml reading was successfull, save the inputXML in a file
  QFile soapFile;
  QTextStream soapStream;

  //go through soap envelope->soap body, search for either GetCapabilities or GetMap
  QDomNodeList envelopeNodeList = inputXML.elementsByTagNameNS( "http://schemas.xmlsoap.org/soap/envelope/", "Envelope" );
  if ( envelopeNodeList.size() < 1 )
  {
    QgsDebugMsg( "Envelope element not found" );
    throw QgsMapServiceException( "SOAPError", "Element <Envelope> not found" );
  }

  QDomNodeList bodyNodeList = envelopeNodeList.item( 0 ).toElement().elementsByTagNameNS( "http://schemas.xmlsoap.org/soap/envelope/", "Body" );
  if ( bodyNodeList.size() < 1 )
  {
    QgsDebugMsg( "body node not found" );
    throw QgsMapServiceException( "SOAPError", "Element <Body> not found" );
  }
  QDomElement bodyElement = bodyNodeList.item( 0 ).toElement();
  QDomElement firstChildElement = bodyElement.firstChild().toElement();

  QString serviceString = firstChildElement.attribute( "service" );
  if ( serviceString == "MS" )
  {
    QgsDebugMsg( "service = MS " );
    mParameterMap.insert( "SERVICE", "MS" );
    mService = "MS";
  }
  else if ( serviceString == "WMS" )
  {
    mParameterMap.insert( "SERVICE", "WMS" );
    mService = "WMS";
  }
  else if ( serviceString == "MDS" )
  {
    mParameterMap.insert( "SERVICE", "MDS" );
    mService = "MDS";
  }
  else if ( serviceString == "MAS" )
  {
    mParameterMap.insert( "SERVICE", "MAS" );
    mService = "MAS";
  }
  else
  {
    mParameterMap.insert( "SERVICE", "DISCOVERY" );
    mService = "DISCOVERY";
  }


  //GetCapabilities request
  //if(firstChildElement.localName().compare("getCapabilities", Qt::CaseInsensitive) == 0)
  if ( firstChildElement.localName() == "GetCapabilities" || firstChildElement.localName() == "getCapabilities" )
  {
    mParameterMap.insert( "REQUEST", "GetCapabilities" );
  }
  //GetMap request
  //else if(firstChildElement.tagName().compare("getMap",Qt::CaseInsensitive) == 0)
  else if ( firstChildElement.localName() == "GetMap" || firstChildElement.localName() == "getMap" )
  {
    mParameterMap.insert( "REQUEST", "GetMap" );
    parseGetMapElement( mParameterMap, firstChildElement );
  }
  //GetDiagram request
  //else if(firstChildElement.tagName().compare("getDiagram", Qt::CaseInsensitive) == 0)
  else if ( firstChildElement.localName() == "GetDiagram" )
  {
    mParameterMap.insert( "REQUEST", "GetDiagram" );
    parseGetMapElement( mParameterMap, firstChildElement ); //reuse the method for GetMap
  }
  //GetFeatureInfo request
  else if ( firstChildElement.localName() == "GetFeatureInfo" )
  {
    mParameterMap.insert( "REQUEST", "GetFeatureInfo" );
    parseGetFeatureInfoElement( mParameterMap, firstChildElement );
  }

  //set mFormat
  QString formatString = mParameterMap.value( "FORMAT" );
  if ( !formatString.isEmpty() )
  {
    //remove the image/ in front of the format
    if ( formatString == "image/jpeg" || formatString == "JPG" || formatString == "jpg" )
    {
      formatString = "JPG";
    }
    else if ( formatString == "image/png" || formatString == "PNG" || formatString == "png" )
    {
      formatString = "PNG";
    }
    else if ( formatString == "image/gif" || formatString == "GIF" || formatString == "gif" )
    {
      formatString = "GIF";
    }
    else
    {
      throw QgsMapServiceException( "InvalidFormat", "Invalid format " + formatString + ", only jpg and png are supported" );
    }

    mFormat = formatString;
  }
}

void QgsSOAPRequestHandler::setGetMapResponse( const QString& service, QImage* img )
{
  QgsMapServiceException ex( "set error", "Error, could not set Image" );
  if ( service == "WMS" )
  {
    if ( setUrlToFile( img ) != 0 )
    {
      setServiceException( ex );
    }
  }
  else if ( service == "MAS" || service == "MS" || service == "MDS" )
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
    QDomNodeList capabilitiesNodes =  DocCapabilitiesElement.elementsByTagName( "Capability" );
    if ( !capabilitiesNodes.isEmpty() )
    {

      //create response document
      QDomDocument soapResponseDoc;
      //Envelope element
      QDomElement soapEnvelopeElement = soapResponseDoc.createElement( "soap:Envelope" );
      soapEnvelopeElement.setAttribute( "xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/" );
      soapEnvelopeElement.setAttribute( "soap:encoding", "http://schemas.xmlsoap.org/soap/encoding/" );
      soapResponseDoc.appendChild( soapEnvelopeElement );

      //Body element
      QDomElement soapBodyElement = soapResponseDoc.createElementNS( "http://schemas.xmlsoap.org/soap/envelope/", "soap:Body" );
      soapEnvelopeElement.appendChild( soapBodyElement );

      // check if WMS or MS SOAP request

      if ( mService == "MS" || mService == "MDS" || mService == "MAS" )
      {
        //OA_MI_MS_Capabilities element
        QDomElement msCapabilitiesElement = soapResponseDoc.createElement( "ms:OA_MI_Service_Capabilities" );
        msCapabilitiesElement.setAttribute( "service", "MS" );
        msCapabilitiesElement.setAttribute( "version", "1.1" );
        msCapabilitiesElement.setAttribute( "xmlns:ms", "http://www.eu-orchestra.org/services/ms" );
        msCapabilitiesElement.setAttribute( "xmlns", "http://www.eu-orchestra.org/services/oas/oa_basic" );
        msCapabilitiesElement.setAttribute( "xmlns:gml", "http://www.opengis.net/gml" );
        msCapabilitiesElement.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
        msCapabilitiesElement.setAttribute( "xmlns:xsd", "http://www.w3.org/2001/XMLSchema" );
        soapBodyElement.appendChild( msCapabilitiesElement );

        // import the orchestra common capabilities
        QFile common( "common.xml" );
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
          QDomElement msSpecificCapabilitiesElement = soapResponseDoc.createElement( "serviceSpecificCapabilities" );
          soapBodyElement.appendChild( msSpecificCapabilitiesElement );

          QDomElement capabilitiesElement = capabilitiesNodes.item( 0 ).toElement();
          msSpecificCapabilitiesElement.appendChild( capabilitiesElement );

          // to do supportedOperations
          QDomNodeList requestNodes = capabilitiesElement.elementsByTagName( "Request" );
          if ( !requestNodes.isEmpty() )
          {
            QDomElement requestElement = requestNodes.item( 0 ).toElement();
            QDomNodeList requestChildNodes = requestElement.childNodes();

            //append an array element for 'supportedOperations' to the soap document
            QDomElement supportedOperationsElement = soapResponseDoc.createElement( "supportedOperations" );
            supportedOperationsElement.setAttribute( "xsi:type", "soapenc:Array" );
            supportedOperationsElement.setAttribute( "soap:arrayType", "xsd:string[" + QString::number( requestChildNodes.size() ) + "]" );

            for ( int i = 0; i < requestChildNodes.size(); ++i )
            {
              QDomElement itemElement = soapResponseDoc.createElement( "item" );
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
      else if ( mService == "WMS" )
      {
        //WMS_Capabilities element
        QDomElement msCapabilitiesElement = soapResponseDoc.createElement( "wms:Capabilities" );
        msCapabilitiesElement.setAttribute( "service", "WMS" );
        msCapabilitiesElement.setAttribute( "version", "1.3.0" );
        msCapabilitiesElement.setAttribute( "xmlns:wms", "http://www.opengis.net/wms" );
        msCapabilitiesElement.setAttribute( "xmlns:gml", "http://www.opengis.net/gml" );
        msCapabilitiesElement.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
        msCapabilitiesElement.setAttribute( "xmlns:xsd", "http://www.w3.org/2001/XMLSchema" );
        soapBodyElement.appendChild( msCapabilitiesElement );

        QFile wmsService( "wms_metadata.xml" );
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

        QDomElement msServiceElement = soapResponseDoc.createElement( "Service" );
        msCapabilitiesElement.appendChild( msServiceElement );

        QDomElement capabilitiesElement = capabilitiesNodes.item( 0 ).toElement();
        msCapabilitiesElement.appendChild( capabilitiesElement );



      }
      else
      {
        //OA_MI_MS_Capabilities element
        QDomElement msCapabilitiesElement = soapResponseDoc.createElement( "ms:OA_MI_Service_Capabilities" );
        msCapabilitiesElement.setAttribute( "service", "MS" );
        msCapabilitiesElement.setAttribute( "version", "1.1" );
        msCapabilitiesElement.setAttribute( "xmlns:ms", "http://www.eu-orchestra.org/services/ms" );
        msCapabilitiesElement.setAttribute( "xmlns", "http://www.eu-orchestra.org/services/oas/oa_basic" );
        msCapabilitiesElement.setAttribute( "xmlns:gml", "http://www.opengis.net/gml" );
        msCapabilitiesElement.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
        msCapabilitiesElement.setAttribute( "xmlns:xsd", "http://www.w3.org/2001/XMLSchema" );
        soapBodyElement.appendChild( msCapabilitiesElement );

        // import the orchestra common capabilities
        QFile common( "common.xml" );
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
      setHttpResponse( &ba, "text/xml" );
    }
  }
}

void QgsSOAPRequestHandler::setGetFeatureInfoResponse( const QDomDocument& infoDoc, const QString& infoFormat )
{
  Q_UNUSED( infoFormat );
  QDomDocument featureInfoResponseDoc;

  //Envelope
  QDomElement soapEnvelopeElement = featureInfoResponseDoc.createElement( "soap:Envelope" );
  soapEnvelopeElement.setAttribute( "xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/" );
  soapEnvelopeElement.setAttribute( "soap:encoding", "http://schemas.xmlsoap.org/soap/encoding/" );
  featureInfoResponseDoc.appendChild( soapEnvelopeElement );

  //Body
  QDomElement soapBodyElement = featureInfoResponseDoc.createElementNS( "http://schemas.xmlsoap.org/soap/envelope/", "soap:Body" );
  soapEnvelopeElement.appendChild( soapBodyElement );

  soapBodyElement.appendChild( infoDoc.documentElement() );

  //now set message
  QByteArray ba = featureInfoResponseDoc.toByteArray();
  setHttpResponse( &ba, "text/xml" );
}

void QgsSOAPRequestHandler::setXmlResponse( const QDomDocument& infoDoc )
{
  QDomDocument featureInfoResponseDoc;

  //Envelope
  QDomElement soapEnvelopeElement = featureInfoResponseDoc.createElement( "soap:Envelope" );
  soapEnvelopeElement.setAttribute( "xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/" );
  soapEnvelopeElement.setAttribute( "soap:encoding", "http://schemas.xmlsoap.org/soap/encoding/" );
  featureInfoResponseDoc.appendChild( soapEnvelopeElement );

  //Body
  QDomElement soapBodyElement = featureInfoResponseDoc.createElementNS( "http://schemas.xmlsoap.org/soap/envelope/", "soap:Body" );
  soapEnvelopeElement.appendChild( soapBodyElement );

  soapBodyElement.appendChild( infoDoc.documentElement() );

  //now set message
  QByteArray ba = featureInfoResponseDoc.toByteArray();
  setHttpResponse( &ba, "text/xml" );
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
  QDomElement soapEnvelopeElement = soapResponseDoc.createElement( "soap:Envelope" );
  soapEnvelopeElement.setAttribute( "xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/" );
  soapEnvelopeElement.setAttribute( "soap:encoding", "http://schemas.xmlsoap.org/soap/encoding/" );
  soapResponseDoc.appendChild( soapEnvelopeElement );

  //Body element
  QDomElement soapBodyElement = soapResponseDoc.createElementNS( "http://schemas.xmlsoap.org/soap/envelope/", "soap:Body" );
  soapEnvelopeElement.appendChild( soapBodyElement );

  QDomElement msExceptionsElement = soapResponseDoc.createElement( "Exception" );
  msExceptionsElement.setAttribute( "format", "text/xml" );
  soapBodyElement.appendChild( msExceptionsElement );

  QDomText msExceptionMessage = soapResponseDoc.createTextNode( QString( ex.message() ) );
  msExceptionsElement.appendChild( msExceptionMessage );

  QByteArray ba = soapResponseDoc.toByteArray();
  setHttpResponse( &ba, "text/xml" );
}

int QgsSOAPRequestHandler::parseGetMapElement( QMap<QString, QString>& parameterMap, const QDomElement& getMapElement ) const
{
  QDomNodeList boundingBoxList = getMapElement.elementsByTagName( "BoundingBox" );
  if ( !boundingBoxList.isEmpty() )
  {
    parseBoundingBoxElement( parameterMap, boundingBoxList.item( 0 ).toElement() );
  }
  QDomNodeList CRSList = getMapElement.elementsByTagName( "coordinateReferenceSystem" );
  if ( !CRSList.isEmpty() )
  {
    QString crsText = CRSList.item( 0 ).toElement().text();
    QString epsgNumber;
    if ( !crsText.startsWith( "EPSG_" ) ) //use underscore in SOAP because ':' is reserved for namespaces
    {
      //error
    }
    else
    {
      epsgNumber = crsText.replace( 4, 1, ":" );//replace the underscore with a ':' to make it WMS compatible
    }
    parameterMap.insert( "CRS", epsgNumber );
  }
  QDomNodeList GMLList = getMapElement.elementsByTagNameNS( "http://www.eu-orchestra.org/services/ms", "GML" );
  if ( !GMLList.isEmpty() )
  {
    QString gmlText;
    QTextStream gmlStream( &gmlText );
    GMLList.at( 0 ).save( gmlStream, 2 );
    parameterMap.insert( "GML", gmlText );
  }

  //outputAttributes
  QDomNodeList imageDocumentAttributesList = getMapElement.elementsByTagName( "Output" );
  if ( !imageDocumentAttributesList.isEmpty() )
  {
    parseOutputAttributesElement( parameterMap, imageDocumentAttributesList.item( 0 ).toElement() );
  }

  //SLD
  QDomNodeList sldList = getMapElement.elementsByTagName( "StyledLayerDescriptor" );
  if ( !sldList.isEmpty() )
  {
    QString sldString;
    QTextStream sldStream( &sldString );
    sldList.item( 0 ).save( sldStream, 0 );
    //Replace some special characters
    sldString.replace( "&lt;", "<" );
    sldString.replace( "&gt;", ">" );
    parameterMap.insert( "SLD", sldString );
  }

  return 0;
}

int QgsSOAPRequestHandler::parseGetFeatureInfoElement( QMap<QString, QString>& parameterMap, const QDomElement& getFeatureInfoElement ) const
{
  QDomNodeList queryList = getFeatureInfoElement.elementsByTagName( "Query" );
  if ( queryList.size() < 1 )
  {
    return 1;
  }
  QDomElement queryElem = queryList.at( 0 ).toElement();

  //find <QueryLayer>
  QDomNodeList queryLayerList = queryElem.elementsByTagName( "QueryLayer" );
  if ( queryLayerList.size() < 1 )
  {
    return 0; //no error, but nothing to do
  }
  QString queryLayerString = queryLayerList.at( 0 ).toElement().text();
  parameterMap.insert( "QUERY_LAYERS", queryLayerString );

  //find <XImagePoint>
  QDomNodeList xImageList = queryElem.elementsByTagName( "XImagePoint" );
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
  parameterMap.insert( "I", QString::number( xPoint ) );

  //find <YImagePoint>
  QDomNodeList yImageList = queryElem.elementsByTagName( "YImagePoint" );
  if ( yImageList.size() < 1 )
  {
    return 5; //mandatory
  }
  int yPoint = xImageList.at( 0 ).toElement().text().toInt( &conversionSuccess );
  if ( !conversionSuccess )
  {
    return 6;
  }
  parameterMap.insert( "J", QString::number( yPoint ) );

  //find <FeatureCount>
  QDomNodeList featureCountList = queryElem.elementsByTagName( "FeatureCount" );
  if ( !featureCountList.isEmpty() ) //optional
  {
    int featureCount = featureCountList.at( 0 ).toElement().text().toInt( &conversionSuccess );
    if ( conversionSuccess )
    {
      parameterMap.insert( "FEATURE_COUNT", QString::number( featureCount ) );
    }
  }

  //RequestCopy
  QDomNodeList requestCopyList = getFeatureInfoElement.elementsByTagName( "RequestCopy" );
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
  QDomNodeList leftBoundList = boundingBoxElement.elementsByTagName( "leftBound" );
  if ( !leftBoundList.isEmpty() )
  {
    minx = leftBoundList.item( 0 ).toElement().text();
  }

  //rightBound
  QDomNodeList rightBoundList = boundingBoxElement.elementsByTagName( "rightBound" );
  if ( !rightBoundList.isEmpty() )
  {
    maxx = rightBoundList.item( 0 ).toElement().text();
  }

  //lowerBound
  QDomNodeList lowerBoundList = boundingBoxElement.elementsByTagName( "lowerBound" );
  if ( !lowerBoundList.isEmpty() )
  {
    miny = lowerBoundList.item( 0 ).toElement().text();
  }

  //upperBound
  QDomNodeList upperBoundList = boundingBoxElement.elementsByTagName( "upperBound" );
  if ( !upperBoundList.isEmpty() )
  {
    maxy = upperBoundList.item( 0 ).toElement().text();
  }
  parameterMap.insert( "BBOX", minx + "," + miny + "," + maxx + "," + maxy );
  return 0;
}

int QgsSOAPRequestHandler::parseOutputAttributesElement( QMap<QString, QString>& parameterMap, const QDomElement& outputAttributesElement ) const
{
  //height
  QDomNodeList heightList = outputAttributesElement.elementsByTagName( "Height" );
  if ( !heightList.isEmpty() )
  {
    QString heightString = heightList.item( 0 ).toElement().text();
    parameterMap.insert( "HEIGHT", heightString );
  }

  //width
  QDomNodeList widthList = outputAttributesElement.elementsByTagName( "Width" );
  if ( !widthList.isEmpty() )
  {
    QString widthString = widthList.item( 0 ).toElement().text();
    parameterMap.insert( "WIDTH", widthString );
  }

  //format
  QDomNodeList formatList = outputAttributesElement.elementsByTagName( "Format" );
  if ( !formatList.isEmpty() )
  {
    QString formatString = formatList.item( 0 ).toElement().text();
    parameterMap.insert( "FORMAT", formatString );
  }

  //background transparendy
  QDomNodeList bgTransparencyList = outputAttributesElement.elementsByTagName/*NS*/( /*"http://www.eu-orchestra.org/services/ms",*/ "Transparent" );
  if ( !bgTransparencyList.isEmpty() )
  {
    QString bgTransparencyString = bgTransparencyList.item( 0 ).toElement().text();
    if ( bgTransparencyString.compare( "true", Qt::CaseInsensitive ) == 0
         || bgTransparencyString == "1" )
    {
      parameterMap.insert( "TRANSPARENT", "TRUE" );
    }
    else
    {
      parameterMap.insert( "TRANSPARENT", "FALSE" );
    }
  }
  return 0;
}

int QgsSOAPRequestHandler::setSOAPWithAttachments( QImage* img )
{
  QgsDebugMsg( "Entering." );
  //create response xml document
  QDomDocument xmlResponse; //response xml, save this into mimeString
  QDomElement envelopeElement = xmlResponse.createElementNS( "http://schemas.xmlsoap.org/soap/envelope/", "Envelope" );
  xmlResponse.appendChild( envelopeElement );
  QDomElement bodyElement = xmlResponse.createElementNS( "http://schemas.xmlsoap.org/soap/envelope/", "Body" );
  envelopeElement.appendChild( bodyElement );
  QDomElement getMapResponseElement = xmlResponse.createElementNS( "http://www.eu-orchestra.org/services/ms", "getMapResponse" );
  bodyElement.appendChild( getMapResponseElement );
  QDomElement returnElement = xmlResponse.createElementNS( "http://www.eu-orchestra.org/services/ms", "return" );
  returnElement.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
  returnElement.setAttribute( "xsi:type", "OA_ImageDocument" );
  returnElement.setAttribute( "href", "image@mapservice" );
  getMapResponseElement.appendChild( returnElement );

  //create image buffer
  QByteArray ba;
  QBuffer buffer( &ba );
  buffer.open( QIODevice::WriteOnly );
  img->save( &buffer, mFormat.toLocal8Bit().data(), -1 ); // writes image into ba

  QByteArray xmlByteArray = xmlResponse.toString().toLocal8Bit();

  // Set headers
  setHeader( "MIME-Version", "1.0" );
  setHeader( "Content-Type", "Multipart/Related; boundary=\"MIME_boundary\"; type=\"text/xml\"; start=\"<xml@mapservice>\"" );
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
  if ( mFormat == "JPG" )
  {
    appendBody( "Content-Type: image/jpg\n" );
  }
  else if ( mFormat == "PNG" )
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
  tmpDir = QDir( "/tmp" );
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
    if ( !tmpDir.mkdir( "mas_tmp" ) )
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

  if ( mFormat == "JPG" )
  {
    theFile.setFileName( tmpMasDir.absolutePath() + "/" + folderName + "/map.jpg" );
    uri.append( "/mas_tmp/" + folderName + "/map.jpg" );
  }
  else if ( mFormat == "PNG" )
  {
    theFile.setFileName( tmpMasDir.absolutePath() + "/" + folderName + "/map.png" );
    uri.append( "/mas_tmp/" + folderName + "/map.png" );
  }
  if ( !theFile.open( QIODevice::WriteOnly ) )
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
  QDomElement envelopeElement = xmlResponse.createElementNS( "http://schemas.xmlsoap.org/soap/envelope/", "Envelope" );
  xmlResponse.appendChild( envelopeElement );
  QDomElement bodyElement = xmlResponse.createElementNS( "http://schemas.xmlsoap.org/soap/envelope/", "Body" );
  envelopeElement.appendChild( bodyElement );
  QDomElement getMapResponseElement = xmlResponse.createElementNS( "http://www.eu-orchestra.org/services/ms", "getMapResponse" );
  QDomElement ahrefElement = xmlResponse.createElement( "a" );
  ahrefElement.setAttribute( "href", uri );
  //QString href("<a href=\""+uri+"\">"+uri+"</a>");
  QDomText uriNode = xmlResponse.createTextNode( uri );
  ahrefElement.appendChild( uriNode );
  //getMapResponseElement.appendChild(uriNode);
  getMapResponseElement.appendChild( ahrefElement );
  bodyElement.appendChild( getMapResponseElement );

  QByteArray xmlByteArray = xmlResponse.toByteArray();
  setHttpResponse( &xmlByteArray, "text/xml" );
  return 0;
}

int QgsSOAPRequestHandler::findOutHostAddress( QString& address ) const
{
  QDomDocument wmsMetadataDoc;
  QFile wmsMetadataFile( "wms_metadata.xml" );

  if ( !wmsMetadataFile.open( QIODevice::ReadOnly ) )
  {
    return 1;
  }
  if ( !wmsMetadataDoc.setContent( &wmsMetadataFile, true ) )
  {
    return 2;
  }
  QDomNodeList onlineResourceList = wmsMetadataDoc.elementsByTagName( "OnlineResource" );
  if ( onlineResourceList.size() < 1 )
  {
    return 3;
  }
  address = onlineResourceList.at( 0 ).toElement().attribute( "href" );
  QgsDebugMsg( "address found: " + address );
  if ( address.isEmpty() )
  {
    return 4;
  }
  return 0;
}
