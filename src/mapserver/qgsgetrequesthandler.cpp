#include "qgsgetrequesthandler.h"
#include "qgsftptransaction.h"
#include "qgslogger.h"
#include "qgsmapserviceexception.h"
#include "qgsremotedatasourcebuilder.h"
#include "qgshttptransaction.h"
#include <QBuffer>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>
#include <QImage>
#include <QStringList>
#include <QUrl>

QgsGetRequestHandler::QgsGetRequestHandler(): QgsHttpRequestHandler()
{
}

std::map<QString, QString> QgsGetRequestHandler::parseInput()
{
  std::map<QString, QString> parameters;
  QString queryString;
  const char* qs = getenv( "QUERY_STRING" );
  if ( qs )
  {
    queryString = QString( qs );
    QgsDebugMsg( "query string is: " + queryString );
  }
  else
  {
    QgsDebugMsg( "error, no query string found" );
    return parameters; //no query string? something must be wrong...
  }

  //parameters are separated by &
  QStringList elements = queryString.split( "&" );

  QString element, key, value;

  //insert key and value into the map
  for ( QStringList::const_iterator it = elements.begin(); it != elements.end(); ++it )
  {
    element = *it;
    int sepidx = element.indexOf( "=", 0, Qt::CaseSensitive );
    if ( sepidx == -1 )
    {
      continue;
    }

    key = element.left( sepidx );
    value = element.mid( sepidx + 1 );
    value = QUrl::fromPercentEncoding( value.toLocal8Bit() ); //replace encoded special caracters and utf-8 encodings


    if ( key.compare( "SLD_BODY", Qt::CaseInsensitive ) == 0 )
    {
      key = "SLD";
    }
    else if ( key.compare( "SLD", Qt::CaseInsensitive ) == 0 )
    {
      QByteArray fileContents;
      if ( value.startsWith( "http", Qt::CaseInsensitive ) )
      {
        QgsHttpTransaction http( value );
        if ( !http.getSynchronously( fileContents ) )
        {
          continue;
        }
      }
      else if ( value.startsWith( "ftp", Qt::CaseInsensitive ) )
      {
        QgsFtpTransaction ftp;
        if ( !ftp.get( value, fileContents ) )
        {
          continue;
        }
        value = QUrl::fromPercentEncoding( fileContents );
      }
      else
      {
        continue; //only http and ftp supported at the moment
      }
      value = QUrl::fromPercentEncoding( fileContents );

    }
    parameters.insert( std::make_pair( key.toUpper(), value ) );
    QgsDebugMsg( "inserting pair " + key.toUpper() + " // " + value + " into the parameter map" );
  }

  //feature info format?
  std::map<QString, QString>::const_iterator info_format_it = parameters.find( "INFO_FORMAT" );
  if ( info_format_it != parameters.end() )
  {
    mFormat = info_format_it->second;
  }
  else //capabilities format or GetMap format
  {
    std::map<QString, QString>::const_iterator formatIt = parameters.find( "FORMAT" );
    if ( formatIt != parameters.end() )
    {
      QString formatString = formatIt->second;

      QgsDebugMsg( QString( "formatString is: %1" ).arg( formatString ) );

      //remove the image/ in front of the format
      if ( formatString.compare( "image/png", Qt::CaseInsensitive ) == 0 || formatString.compare( "png", Qt::CaseInsensitive ) == 0 )
      {
        formatString = "PNG";
      }
      else if ( formatString.compare( "image/jpeg", Qt::CaseInsensitive ) == 0 || formatString.compare( "image/jpg", Qt::CaseInsensitive ) == 0 \
                || formatString.compare( "jpg", Qt::CaseInsensitive ) == 0 )
      {
        formatString = "JPG";
      }
      else if ( formatString.compare( "svg", Qt::CaseInsensitive ) == 0 )
      {
        formatString = "SVG";
      }
      else if ( formatString.compare( "pdf", Qt::CaseInsensitive ) == 0 )
      {
        formatString = "PDF";
      }

      mFormat = formatString;
    }
  }


  return parameters;
}

void QgsGetRequestHandler::sendGetMapResponse( const QString& service, QImage* img ) const
{
  Q_UNUSED( service );
  if ( img )
  {
    if ( mFormat != "PNG" && mFormat != "JPG" )
    {
      sendServiceException( QgsMapServiceException( "InvalidFormat", "Output format '" + mFormat + "' is not supported in the GetMap request" ) );
      return;
    }

    //store the image in a QByteArray and send it directly
    QByteArray ba;
    QBuffer buffer( &ba );
    buffer.open( QIODevice::WriteOnly );
    img->save( &buffer, mFormat.toLocal8Bit().data(), -1 );

    sendHttpResponse( &ba, formatToMimeType( mFormat ) );
  }
}

void QgsGetRequestHandler::sendGetCapabilitiesResponse( const QDomDocument& doc ) const
{
  QByteArray ba = doc.toByteArray();
  sendHttpResponse( &ba, "text/xml" );
}

void QgsGetRequestHandler::sendGetStyleResponse( const QDomDocument& doc ) const
{
  QByteArray ba = doc.toByteArray();
  sendHttpResponse( &ba, "text/xml" );
}

void QgsGetRequestHandler::sendGetFeatureInfoResponse( const QDomDocument& infoDoc, const QString& infoFormat ) const
{
  QByteArray ba;
  QgsDebugMsg( "Info format is:" + infoFormat );

  if ( infoFormat == "text/xml" )
  {
    ba = infoDoc.toByteArray();
  }
  else if ( infoFormat == "text/plain" || infoFormat == "text/html" )
  {
    //create string
    QString featureInfoString;

    if ( infoFormat == "text/plain" )
    {
      featureInfoString.append( "GetFeatureInfo results\n" );
      featureInfoString.append( "\n" );
    }
    else if ( infoFormat == "text/html" )
    {
      featureInfoString.append( "<HEAD>\n" );
      featureInfoString.append( "<TITLE> GetFeatureInfo results </TITLE>\n" );
      featureInfoString.append( "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n" );
      featureInfoString.append( "</HEAD>\n" );
      featureInfoString.append( "<BODY>\n" );
    }

    QDomNodeList layerList = infoDoc.elementsByTagName( "Layer" );

    //layer loop
    for ( int i = 0; i < layerList.size(); ++i )
    {
      QDomElement layerElem = layerList.at( i ).toElement();
      if ( infoFormat == "text/plain" )
      {
        featureInfoString.append( "Layer '" + layerElem.attribute( "name" ) + "'\n" );
      }
      else if ( infoFormat == "text/html" )
      {
        featureInfoString.append( "<TABLE border=1 width=100%>\n" );
        featureInfoString.append( "<TR><TH width=25%>Layer</TH><TD>" + layerElem.attribute( "name" ) + "</TD></TR>\n" );
        featureInfoString.append( "</BR>" );
      }

      //feature loop (for vector layers)
      QDomNodeList featureNodeList = layerElem.elementsByTagName( "Feature" );
      QDomElement currentFeatureElement;

      if ( featureNodeList.size() < 1 ) //raster layer?
      {
        QDomNodeList attributeNodeList = layerElem.elementsByTagName( "Attribute" );
        for ( int j = 0; j < attributeNodeList.size(); ++j )
        {
          QDomElement attributeElement = attributeNodeList.at( j ).toElement();
          if ( infoFormat == "text/plain" )
          {
            featureInfoString.append( attributeElement.attribute( "name" ) + " = '" +
                                      attributeElement.attribute( "value" ) + "'\n" );
          }
          else if ( infoFormat == "text/html" )
          {
            featureInfoString.append( "<TR><TH>" + attributeElement.attribute( "name" ) + "</TH><TD>" +
                                      attributeElement.attribute( "value" ) + "</TD></TR>\n" );
          }
        }
      }
      else //vector layer
      {
        for ( int j = 0; j < featureNodeList.size(); ++j )
        {
          QDomElement featureElement = featureNodeList.at( j ).toElement();
          if ( infoFormat == "text/plain" )
          {
            featureInfoString.append( "Feature " + featureElement.attribute( "id" ) + "\n" );
          }
          else if ( infoFormat == "text/html" )
          {
            featureInfoString.append( "<TABLE border=1 width=100%>\n" );
            featureInfoString.append( "<TR><TH>Feature</TH><TD>" + featureElement.attribute( "id" ) + "</TD></TR>\n" );
          }
          //attribute loop
          QDomNodeList attributeNodeList = featureElement.elementsByTagName( "Attribute" );
          for ( int k = 0; k < attributeNodeList.size(); ++k )
          {
            QDomElement attributeElement = attributeNodeList.at( k ).toElement();
            if ( infoFormat == "text/plain" )
            {
              featureInfoString.append( attributeElement.attribute( "name" ) + " = '" +
                                        attributeElement.attribute( "value" ) + "'\n" );
            }
            else if ( infoFormat == "text/html" )
            {
              featureInfoString.append( "<TR><TH>" + attributeElement.attribute( "name" ) + "</TH><TD>" + attributeElement.attribute( "value" ) + "</TD></TR>\n" );
            }
          }

          if ( infoFormat == "text/html" )
          {
            featureInfoString.append( "</TABLE>\n</BR>\n" );
          }
        }
      }
      if ( infoFormat == "text/plain" )
      {
        featureInfoString.append( "\n" );
      }
      else if ( infoFormat == "text/html" )
      {
        featureInfoString.append( "</TABLE>\n<BR></BR>\n" );

      }
    }
    if ( infoFormat == "text/html" )
    {
      featureInfoString.append( "</BODY>\n" );
    }
    ba = featureInfoString.toUtf8();
  }
  else //unsupported format, send exception
  {
    //todo: send service exception
  }

  sendHttpResponse( &ba, infoFormat );
}

void QgsGetRequestHandler::sendServiceException( const QgsMapServiceException& ex ) const
{
  //create Exception DOM document
  QDomDocument exceptionDoc;
  QDomElement serviceExceptionReportElem = exceptionDoc.createElement( "ServiceExceptionReport" );
  serviceExceptionReportElem.setAttribute( "version", "1.3.0" );
  serviceExceptionReportElem.setAttribute( "xmlns", "http://www.opengis.net/ogc" );
  exceptionDoc.appendChild( serviceExceptionReportElem );
  QDomElement serviceExceptionElem = exceptionDoc.createElement( "ServiceException" );
  serviceExceptionElem.setAttribute( "code", ex.code() );
  QDomText messageText = exceptionDoc.createTextNode( ex.message() );
  serviceExceptionElem.appendChild( messageText );
  serviceExceptionReportElem.appendChild( serviceExceptionElem );

  QByteArray ba = exceptionDoc.toByteArray();
  sendHttpResponse( &ba, "text/xml" );
}

void QgsGetRequestHandler::sendGetPrintResponse( QByteArray* ba ) const
{
  sendHttpResponse( ba, formatToMimeType( mFormat ) );
}
