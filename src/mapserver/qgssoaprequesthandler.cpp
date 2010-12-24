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
#include "qgsmapserverlogger.h"
#include "qgsmapserviceexception.h"
#include <QBuffer>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QImage>
#include <QTextStream>
#include <time.h>
#include <fcgi_stdio.h>

QgsSOAPRequestHandler::QgsSOAPRequestHandler()
{

}

QgsSOAPRequestHandler::~QgsSOAPRequestHandler()
{

}

std::map<QString, QString> QgsSOAPRequestHandler::parseInput()
{
  std::map<QString, QString> result;

  char* lengthString = NULL;
  int length = 0;
  char* input = NULL;
  QString inputString;
  QString lengthQString;

  lengthString = getenv( "CONTENT_LENGTH" );
  if ( lengthString != NULL )
  {
    bool conversionSuccess = false;
    lengthQString = QString( lengthString );
    length = lengthQString.toInt( &conversionSuccess );
    QgsMSDebugMsg( "length is: " + lengthQString )
    if ( conversionSuccess )
    {
      input = ( char* )malloc( length + 1 );
      memset( input, 0, length + 1 );
      for ( int i = 0; i < length; ++i )
      {
        input[i] = getchar();
      }
      //fgets(input, length+1, stdin);
      if ( input != NULL )
      {
        inputString = QString::fromLocal8Bit( input );
#ifdef WIN32 //cut off any strange charactes at the end of the file
        int lastClosedBracketPos = inputString.lastIndexOf( ">" );
        if ( lastClosedBracketPos != -1 )
        {
          inputString.truncate( lastClosedBracketPos + 1 );
        }
#endif //WIN32
      }
      else
      {
        QgsMSDebugMsg( "input is NULL " )
      }
      free( input );
    }
    else
    {
      QgsMSDebugMsg( "could not convert CONTENT_LENGTH to int" )
    }
  }

  //QgsMSDebugMsg("input string is: " + inputString)

  //inputString to QDomDocument
  QDomDocument inputXML;
  QString errorMsg;
  if ( !inputXML.setContent( inputString, true, &errorMsg ) )
  {
    QgsMSDebugMsg( "soap request parse error" )
    QgsMSDebugMsg( "error message: " + errorMsg )
    QgsMSDebugMsg( "the xml string was:" )
    QgsMSDebugMsg( inputString )
    throw QgsMapServiceException( "InvalidXML", "XML error: " + errorMsg );
    return result;
  }

  // if xml reading was successfull, save the inputXML in a file
  time_t t;
  struct tm *currentTime;
  time( &t );
  currentTime = localtime( &t );

  QFile soapFile;
  QTextStream soapStream;

  //go through soap envelope->soap body, search for either GetCapabilities or GetMap
  QDomNodeList envelopeNodeList = inputXML.elementsByTagNameNS( "http://schemas.xmlsoap.org/soap/envelope/", "Envelope" );
  if ( envelopeNodeList.size() < 1 )
  {
    QgsMSDebugMsg( "Envelope element not found" )
    throw QgsMapServiceException( "SOAPError", "Element <Envelope> not found" );
    return result;
  }

  QDomNodeList bodyNodeList = envelopeNodeList.item( 0 ).toElement().elementsByTagNameNS( "http://schemas.xmlsoap.org/soap/envelope/", "Body" );
  if ( bodyNodeList.size() < 1 )
  {
    QgsMSDebugMsg( "body node not found" )
    throw QgsMapServiceException( "SOAPError", "Element <Body> not found" );
    return result;
  }
  QDomElement bodyElement = bodyNodeList.item( 0 ).toElement();
  QDomElement firstChildElement = bodyElement.firstChild().toElement();

  QString serviceString = firstChildElement.attribute( "service" );
  if ( serviceString == "MS" )
  {
    QgsMSDebugMsg( "service = MS " )
    result.insert( std::make_pair( "SERVICE", "MS" ) );
    mService = "MS";
  }
  else if ( serviceString == "WMS" )
  {
    result.insert( std::make_pair( "SERVICE", "WMS" ) );
    mService = "WMS";
  }
  else if ( serviceString == "MDS" )
  {
    result.insert( std::make_pair( "SERVICE", "MDS" ) );
    mService = "MDS";
  }
  else if ( serviceString == "MAS" )
  {
    result.insert( std::make_pair( "SERVICE", "MAS" ) );
    mService = "MAS";
  }
  else
  {
    result.insert( std::make_pair( "SERVICE", "DISCOVERY" ) );
    mService = "DISCOVERY";
  }


  //GetCapabilities request
  //if(firstChildElement.localName().compare("getCapabilities", Qt::CaseInsensitive) == 0)
  if ( firstChildElement.localName() == "GetCapabilities" || firstChildElement.localName() == "getCapabilities" )
  {
    result.insert( std::make_pair( "REQUEST", "GetCapabilities" ) );
  }
  //GetMap request
  //else if(firstChildElement.tagName().compare("getMap",Qt::CaseInsensitive) == 0)
  else if ( firstChildElement.localName() == "GetMap" || firstChildElement.localName() == "getMap" )
  {
    result.insert( std::make_pair( "REQUEST", "GetMap" ) );
    parseGetMapElement( result, firstChildElement );
  }
  //GetDiagram request
  //else if(firstChildElement.tagName().compare("getDiagram", Qt::CaseInsensitive) == 0)
  else if ( firstChildElement.localName() == "GetDiagram" )
  {
    result.insert( std::make_pair( "REQUEST", "GetDiagram" ) );
    parseGetMapElement( result, firstChildElement ); //reuse the method for GetMap
  }
  //GetFeatureInfo request
  else if ( firstChildElement.localName() == "GetFeatureInfo" )
  {
    result.insert( std::make_pair( "REQUEST", "GetFeatureInfo" ) );
    parseGetFeatureInfoElement( result, firstChildElement );
  }

  //set mFormat
  std::map<QString, QString>::const_iterator formatIt = result.find( "FORMAT" );
  if ( formatIt != result.end() )
  {
    QString formatString = formatIt->second;
    //remove the image/ in front of the format
    if ( formatString == "image/jpeg" || formatString == "JPG" || formatString == "jpg" )
    {
      formatString = "JPG";
    }
    else if ( formatString == "image/png" || formatString == "PNG" || formatString == "PNG" )
    {
      formatString = "PNG";
    }
    else if ( formatString == "image/gif" || formatString == "GIF" || formatString == "GIF" )
    {
      formatString = "GIF";
    }
    else
    {
      throw QgsMapServiceException( "InvalidFormat", "Invalid format, only jpg and png are supported" );
    }
    mFormat = formatString;
  }

  return result;
}

void QgsSOAPRequestHandler::sendGetMapResponse( const QString& service, QImage* img ) const
{
  QgsMapServiceException ex( "Send error", "Error, could not send Image" );
  if ( service == "WMS" )
  {
    if ( sendUrlToFile( img ) != 0 )
    {
      sendServiceException( ex );
    }
  }
  else if ( service == "MAS" || service == "MS" || service == "MDS" )
  {

    if ( sendSOAPWithAttachments( img ) != 0 )
    {
      sendServiceException( ex );
    }
  }
  else
  {
    sendServiceException( ex );
  }
}

void QgsSOAPRequestHandler::sendGetCapabilitiesResponse( const QDomDocument& doc ) const
{
  //Parse the QDomDocument Document and create a SOAP response
  QDomElement DocCapabilitiesElement = doc.firstChildElement();
  if ( !DocCapabilitiesElement.isNull() )
  {
    QDomNodeList capabilitiesNodes =  DocCapabilitiesElement.elementsByTagName( "Capability" );
    if ( capabilitiesNodes.size() > 0 )
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
          QgsMSDebugMsg( "external orchestra common capabilities not found" )
        }
        else
        {
          QDomDocument externCapDoc;
          QString parseError;
          int errorLineNo;
          if ( !externCapDoc.setContent( &common, false, &parseError, &errorLineNo ) )
          {
            QgsMSDebugMsg( "parse error at setting content of external orchestra common capabilities: "
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
          if ( requestNodes.size() > 0 )
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
          QgsMSDebugMsg( "external wms service capabilities not found" )
        }
        else
        {
          QDomDocument externServiceDoc;
          QString parseError;
          int errorLineNo;
          if ( !externServiceDoc.setContent( &wmsService, false, &parseError, &errorLineNo ) )
          {
            QgsMSDebugMsg( "parse error at setting content of external wms service capabilities: "
                           + parseError + " at line " + QString::number( errorLineNo ) )
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
          QgsMSDebugMsg( "external orchestra common capabilities not found" )
        }
        else
        {
          QDomDocument externCapDoc;
          QString parseError;
          int errorLineNo;
          if ( !externCapDoc.setContent( &common, false, &parseError, &errorLineNo ) )
          {
            QgsMSDebugMsg( "parse error at setting content of external orchestra common capabilities: "
                           + parseError + " at line " + QString::number( errorLineNo ) )
            common.close();
          }
          common.close();

          // write common capabilities
          QDomElement orchestraCommon = externCapDoc.firstChildElement();
          msCapabilitiesElement.appendChild( orchestraCommon );
        }
      }

      QByteArray ba = soapResponseDoc.toByteArray();
      sendHttpResponse( &ba, "text/xml" );
    }
  }
}

void QgsSOAPRequestHandler::sendGetFeatureInfoResponse( const QDomDocument& infoDoc, const QString& infoFormat ) const
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

  //now send message
  QByteArray ba = featureInfoResponseDoc.toByteArray();
  sendHttpResponse( &ba, "text/xml" );
}

void QgsSOAPRequestHandler::sendGetStyleResponse( const QDomDocument& infoDoc ) const
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

  //now send message
  QByteArray ba = featureInfoResponseDoc.toByteArray();
  sendHttpResponse( &ba, "text/xml" );
}

void QgsSOAPRequestHandler::sendGetPrintResponse( QByteArray* ba, const QString& formatString ) const
{
  //soon...
}

void QgsSOAPRequestHandler::sendServiceException( const QgsMapServiceException& ex ) const
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
  sendHttpResponse( &ba, "text/xml" );
}

int QgsSOAPRequestHandler::parseGetMapElement( std::map<QString, QString>& parameterMap, const QDomElement& getMapElement ) const
{
  QDomNodeList boundingBoxList = getMapElement.elementsByTagName( "BoundingBox" );
  if ( boundingBoxList.size() > 0 )
  {
    parseBoundingBoxElement( parameterMap, boundingBoxList.item( 0 ).toElement() );
  }
  QDomNodeList CRSList = getMapElement.elementsByTagName( "coordinateReferenceSystem" );
  if ( CRSList.size() > 0 )
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
    parameterMap.insert( std::make_pair( "CRS", epsgNumber ) );
  }
  QDomNodeList GMLList = getMapElement.elementsByTagNameNS( "http://www.eu-orchestra.org/services/ms", "GML" );
  if ( GMLList.size() > 0 )
  {
    QString gmlText;
    QTextStream gmlStream( &gmlText );
    GMLList.at( 0 ).save( gmlStream, 2 );
    parameterMap.insert( std::make_pair( "GML", gmlText ) );
  }

  //outputAttributes
  QDomNodeList imageDocumentAttributesList = getMapElement.elementsByTagName( "Output" );
  if ( imageDocumentAttributesList.size() > 0 )
  {
    parseOutputAttributesElement( parameterMap, imageDocumentAttributesList.item( 0 ).toElement() );
  }

  //SLD
  QDomNodeList sldList = getMapElement.elementsByTagName( "StyledLayerDescriptor" );
  if ( sldList.size() > 0 )
  {
    QString sldString;
    QTextStream sldStream( &sldString );
    sldList.item( 0 ).save( sldStream, 0 );
    //Replace some special characters
    sldString.replace( "&lt;", "<" );
    sldString.replace( "&gt;", ">" );
    parameterMap.insert( std::make_pair( "SLD", sldString ) );
  }

  return 0;
}

int QgsSOAPRequestHandler::parseGetFeatureInfoElement( std::map<QString, QString>& parameterMap, const QDomElement& getFeatureInfoElement ) const
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
  parameterMap.insert( std::make_pair( "QUERY_LAYERS", queryLayerString ) );

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
  parameterMap.insert( std::make_pair( "I", QString::number( xPoint ) ) );

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
  parameterMap.insert( std::make_pair( "J", QString::number( yPoint ) ) );

  //find <FeatureCount>
  QDomNodeList featureCountList = queryElem.elementsByTagName( "FeatureCount" );
  if ( featureCountList.size() > 0 ) //optional
  {
    int featureCount = featureCountList.at( 0 ).toElement().text().toInt( &conversionSuccess );
    if ( conversionSuccess )
    {
      parameterMap.insert( std::make_pair( "FEATURE_COUNT", QString::number( featureCount ) ) );
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

int QgsSOAPRequestHandler::parseBoundingBoxElement( std::map<QString, QString>& parameterMap, const QDomElement& boundingBoxElement ) const
{
  QString minx, miny, maxx, maxy;

  //leftBound
  QDomNodeList leftBoundList = boundingBoxElement.elementsByTagName( "leftBound" );
  if ( leftBoundList.size() > 0 )
  {
    minx = leftBoundList.item( 0 ).toElement().text();
  }

  //rightBound
  QDomNodeList rightBoundList = boundingBoxElement.elementsByTagName( "rightBound" );
  if ( rightBoundList.size() > 0 )
  {
    maxx = rightBoundList.item( 0 ).toElement().text();
  }

  //lowerBound
  QDomNodeList lowerBoundList = boundingBoxElement.elementsByTagName( "lowerBound" );
  if ( lowerBoundList.size() > 0 )
  {
    miny = lowerBoundList.item( 0 ).toElement().text();
  }

  //upperBound
  QDomNodeList upperBoundList = boundingBoxElement.elementsByTagName( "upperBound" );
  if ( upperBoundList.size() > 0 )
  {
    maxy = upperBoundList.item( 0 ).toElement().text();
  }
  parameterMap.insert( std::make_pair( "BBOX", minx + "," + miny + "," + maxx + "," + maxy ) );
  return 0;
}

int QgsSOAPRequestHandler::parseOutputAttributesElement( std::map<QString, QString>& parameterMap, const QDomElement& outputAttributesElement ) const
{
  //height
  QDomNodeList heightList = outputAttributesElement.elementsByTagName( "Height" );
  if ( heightList.size() > 0 )
  {
    QString heightString = heightList.item( 0 ).toElement().text();
    parameterMap.insert( std::make_pair( "HEIGHT", heightString ) );
  }

  //width
  QDomNodeList widthList = outputAttributesElement.elementsByTagName( "Width" );
  if ( widthList.size() > 0 )
  {
    QString widthString = widthList.item( 0 ).toElement().text();
    parameterMap.insert( std::make_pair( "WIDTH", widthString ) );
  }

  //format
  QDomNodeList formatList = outputAttributesElement.elementsByTagName( "Format" );
  if ( formatList.size() > 0 )
  {
    QString formatString = formatList.item( 0 ).toElement().text();
    parameterMap.insert( std::make_pair( "FORMAT", formatString ) );
  }

  //background transparendy
  QDomNodeList bgTransparencyList = outputAttributesElement.elementsByTagName/*NS*/( /*"http://www.eu-orchestra.org/services/ms",*/ "Transparent" );
  if ( bgTransparencyList.size() > 0 )
  {
    QString bgTransparencyString = bgTransparencyList.item( 0 ).toElement().text();
    if ( bgTransparencyString.compare( "true", Qt::CaseInsensitive ) == 0
         || bgTransparencyString == "1" )
    {
      parameterMap.insert( std::make_pair( "TRANSPARENT", "TRUE" ) );
    }
    else
    {
      parameterMap.insert( std::make_pair( "TRANSPARENT", "FALSE" ) );
    }
  }
  return 0;
}

int QgsSOAPRequestHandler::sendSOAPWithAttachments( QImage* img ) const
{
  QgsMSDebugMsg( "Entering." )
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
  printf( "MIME-Version: 1.0\n" );
  printf( "Content-Type: Multipart/Related; boundary=\"MIME_boundary\"; type=\"text/xml\"; start=\"<xml@mapservice>\"\n" );
  printf( "\n" );
  printf( "--MIME_boundary\r\n" );
  printf( "Content-Type: text/xml\n" );
  printf( "Content-ID: <xml@mapservice>\n" );
  printf( "\n" );
  printf( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );
  fwrite( xmlByteArray.data(), xmlByteArray.size(), 1, FCGI_stdout );
  printf( "\n" );
  printf( "\r\n" );
  printf( "--MIME_boundary\r\n" );
  if ( mFormat == "JPG" )
  {
    printf( "Content-Type: image/jpg\n" );
  }
  else if ( mFormat == "PNG" )
  {
    printf( "Content-Type: image/png\n" );
  }
  printf( "Content-Transfer-Encoding: binary\n" );
  printf( "Content-ID: <image@mapservice>\n" );
  printf( "\n" );
  fwrite( ba.data(), ba.size(), 1, FCGI_stdout );
  printf( "\r\n" );
  printf( "--MIME_boundary\r\n" );

  return 0;
}

int QgsSOAPRequestHandler::sendUrlToFile( QImage* img ) const
{
  QString uri;
  QFile theFile;
  QDir tmpDir;

  QgsMSDebugMsg( "Entering." )

  if ( findOutHostAddress( uri ) != 0 )
  {
    return 1;
  }

#ifdef WIN32
  if ( !QFile::exists( QDir::currentPath() + "/tmp" ) )
  {
    QDir::current().mkdir( "tmp" );
  }
  tmpDir = QDir( QDir::currentPath() + "/tmp" );
#else //WIN32
  tmpDir = QDir( "/tmp" );
#endif

  QgsMSDebugMsg( "Path to tmpDir is: " + tmpDir.absolutePath() )

  //create a random folder under /tmp with map.jpg/png in it
  //and return the link to the client
  srand( time( NULL ) );
  int randomNumber = rand();
  QString folderName = QString::number( randomNumber );
  if ( !QFile::exists( tmpDir.absolutePath() + "/mas_tmp" ) )
  {
    QgsMSDebugMsg( "Trying to create mas_tmp folder" )
    if ( !tmpDir.mkdir( "mas_tmp" ) )
    {
      return 2;
    }
  }
  QDir tmpMasDir( tmpDir.absolutePath() + "/mas_tmp" );
  if ( !tmpMasDir.mkdir( folderName ) )
  {
    QgsMSDebugMsg( "Trying to create random folder" )
    return 3;
  }

  QgsMSDebugMsg( "Temp. folder is: " + tmpMasDir.absolutePath() + "/" + folderName )

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
    QgsMSDebugMsg( "Error, could not open file" )
    return 4;
  }

  if ( !img->save( &theFile, mFormat.toLocal8Bit().data(), -1 ) )
  {
    QgsMSDebugMsg( "Error, could not save image" )
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
  sendHttpResponse( &xmlByteArray, "text/xml" );
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
  QgsMSDebugMsg( "address found: " + address )
  if ( address.isEmpty() )
  {
    return 4;
  }
  return 0;
}
