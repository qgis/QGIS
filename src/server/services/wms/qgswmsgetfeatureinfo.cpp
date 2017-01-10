/***************************************************************************
                              qgswmsgetfeatureinfo.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
                         (C) 2014 by Alessandro Pasotti (original code)
                         (C) 2016 by David Marteau
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
#include "qgswmsutils.h"
#include "qgswmsgetfeatureinfo.h"
#include "qgswmsservertransitional.h"

namespace QgsWms
{

  void writeInfoResponse( QDomDocument& infoDoc, QgsServerResponse& response, const QString& infoFormat )
  {
    QByteArray ba;
    QgsMessageLog::logMessage( "Info format is:" + infoFormat );

    if ( infoFormat == QLatin1String( "text/xml" ) || infoFormat.startsWith( QLatin1String( "application/vnd.ogc.gml" ) ) )
    {
      ba = infoDoc.toByteArray();
    }
    else if ( infoFormat == QLatin1String( "text/plain" ) || infoFormat == QLatin1String( "text/html" ) )
    {
      //create string
      QString featureInfoString;

      if ( infoFormat == QLatin1String( "text/plain" ) )
      {
        featureInfoString.append( "GetFeatureInfo results\n" );
        featureInfoString.append( "\n" );
      }
      else if ( infoFormat == QLatin1String( "text/html" ) )
      {
        featureInfoString.append( "<HEAD>\n" );
        featureInfoString.append( "<TITLE> GetFeatureInfo results </TITLE>\n" );
        featureInfoString.append( "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n" );
        featureInfoString.append( "</HEAD>\n" );
        featureInfoString.append( "<BODY>\n" );
      }

      QDomNodeList layerList = infoDoc.elementsByTagName( QStringLiteral( "Layer" ) );

      //layer loop
      for ( int i = 0; i < layerList.size(); ++i )
      {
        QDomElement layerElem = layerList.at( i ).toElement();
        if ( infoFormat == QLatin1String( "text/plain" ) )
        {
          featureInfoString.append( "Layer '" + layerElem.attribute( QStringLiteral( "name" ) ) + "'\n" );
        }
        else if ( infoFormat == QLatin1String( "text/html" ) )
        {
          featureInfoString.append( "<TABLE border=1 width=100%>\n" );
          featureInfoString.append( "<TR><TH width=25%>Layer</TH><TD>" + layerElem.attribute( QStringLiteral( "name" ) ) + "</TD></TR>\n" );
          featureInfoString.append( "</BR>" );
        }

        //feature loop (for vector layers)
        QDomNodeList featureNodeList = layerElem.elementsByTagName( QStringLiteral( "Feature" ) );
        QDomElement currentFeatureElement;

        if ( featureNodeList.size() < 1 ) //raster layer?
        {
          QDomNodeList attributeNodeList = layerElem.elementsByTagName( QStringLiteral( "Attribute" ) );
          for ( int j = 0; j < attributeNodeList.size(); ++j )
          {
            QDomElement attributeElement = attributeNodeList.at( j ).toElement();
            if ( infoFormat == QLatin1String( "text/plain" ) )
            {
              featureInfoString.append( attributeElement.attribute( QStringLiteral( "name" ) ) + " = '" +
                                        attributeElement.attribute( QStringLiteral( "value" ) ) + "'\n" );
            }
            else if ( infoFormat == QLatin1String( "text/html" ) )
            {
              featureInfoString.append( "<TR><TH>" + attributeElement.attribute( QStringLiteral( "name" ) ) + "</TH><TD>" +
                                        attributeElement.attribute( QStringLiteral( "value" ) ) + "</TD></TR>\n" );
            }
          }
        }
        else //vector layer
        {
          for ( int j = 0; j < featureNodeList.size(); ++j )
          {
            QDomElement featureElement = featureNodeList.at( j ).toElement();
            if ( infoFormat == QLatin1String( "text/plain" ) )
            {
              featureInfoString.append( "Feature " + featureElement.attribute( QStringLiteral( "id" ) ) + "\n" );
            }
            else if ( infoFormat == QLatin1String( "text/html" ) )
            {
              featureInfoString.append( "<TABLE border=1 width=100%>\n" );
              featureInfoString.append( "<TR><TH>Feature</TH><TD>" + featureElement.attribute( QStringLiteral( "id" ) ) + "</TD></TR>\n" );
            }
            //attribute loop
            QDomNodeList attributeNodeList = featureElement.elementsByTagName( QStringLiteral( "Attribute" ) );
            for ( int k = 0; k < attributeNodeList.size(); ++k )
            {
              QDomElement attributeElement = attributeNodeList.at( k ).toElement();
              if ( infoFormat == QLatin1String( "text/plain" ) )
              {
                featureInfoString.append( attributeElement.attribute( QStringLiteral( "name" ) ) + " = '" +
                                          attributeElement.attribute( QStringLiteral( "value" ) ) + "'\n" );
              }
              else if ( infoFormat == QLatin1String( "text/html" ) )
              {
                featureInfoString.append( "<TR><TH>" + attributeElement.attribute( QStringLiteral( "name" ) ) + "</TH><TD>" + attributeElement.attribute( QStringLiteral( "value" ) ) + "</TD></TR>\n" );
              }
            }

            if ( infoFormat == QLatin1String( "text/html" ) )
            {
              featureInfoString.append( "</TABLE>\n</BR>\n" );
            }
          }
        }
        if ( infoFormat == QLatin1String( "text/plain" ) )
        {
          featureInfoString.append( "\n" );
        }
        else if ( infoFormat == QLatin1String( "text/html" ) )
        {
          featureInfoString.append( "</TABLE>\n<BR></BR>\n" );

        }
      }
      if ( infoFormat == QLatin1String( "text/html" ) )
      {
        featureInfoString.append( "</BODY>\n" );
      }
      ba = featureInfoString.toUtf8();
    }
    else //unsupported format, set exception
    {
      writeError( response,  QStringLiteral( "InvalidFormat" ),
                  QString( "Feature info format '%1' is not supported. Possibilities are 'text/plain', 'text/html' or 'text/xml'." ).arg( infoFormat ) );
      return;
    }

    response.setHeader( QStringLiteral( "Content-Type" ), infoFormat + QStringLiteral( "; charset=utf-8" ) );
    response.write( ba );
  }


  void writeGetFeatureInfo( QgsServerInterface* serverIface, const QString& version,
                            const QgsServerRequest& request, QgsServerResponse& response )
  {
    Q_UNUSED( version );
    QgsServerRequest::Parameters params = request.parameters();
    QgsWmsServer server( serverIface->configFilePath(),
                         *serverIface->serverSettings(), params,
                         getConfigParser( serverIface ),
                         serverIface->accessControls() );
    try
    {
      QDomDocument doc = server.getFeatureInfo( version );
      QString outputFormat = params.value( QStringLiteral( "INFO_FORMAT" ), QStringLiteral( "text/plain" ) );
      writeInfoResponse( doc,  response, outputFormat );
    }
    catch ( QgsMapServiceException& ex )
    {
      writeError( response, ex.code(), ex.message() );
    }

  }


} // samespace QgsWms




