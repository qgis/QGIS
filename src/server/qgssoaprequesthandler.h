/***************************************************************************
                              qgssoaprequesthandler.h
                              -------------------
  begin                : May 16, 2006
  copyright            : (C) 2006 by Marco Hugentobler
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

#ifndef QGSSOAPREQUESTHANDLER_H
#define QGSSOAPREQUESTHANDLER_H

#include "qgshttprequesthandler.h"

class QDomElement;

/**A handler to parse requests via SOAP/HTTP POST*/
class QgsSOAPRequestHandler: public QgsHttpRequestHandler
{
  public:
    QgsSOAPRequestHandler();
    ~QgsSOAPRequestHandler();
    void parseInput() override;
    void setGetMapResponse( const QString& service, QImage* img );
    void setGetCapabilitiesResponse( const QDomDocument& doc ) override;
    void setGetFeatureInfoResponse( const QDomDocument& infoDoc, const QString& infoFormat ) override;
    void setServiceException( const QgsMapServiceException& ex );
    void setXmlResponse( const QDomDocument& doc ) override;
    void setXmlResponse( const QDomDocument& doc, const QString& mimeType ) override;
    void setGetPrintResponse( QByteArray* ba ) override;
  private:
    /**Parses the xml of a getMap request and fills the parameters into the map. Returns 0 in case of success*/
    int parseGetMapElement( QMap<QString, QString>& parameterMap, const QDomElement& getMapElement ) const;
    /**Parses the xml of a feature info request and fills the parameters into the map. Returns 0 in case of success*/
    int parseGetFeatureInfoElement( QMap<QString, QString>& parameterMap, const QDomElement& getMapElement ) const;
    int parseBoundingBoxElement( QMap<QString, QString>& parameterMap, const QDomElement& boundingBoxElement ) const;
    int parseOutputAttributesElement( QMap<QString, QString>& parameterMap, const QDomElement& outputAttributesElement ) const;
    int setSOAPWithAttachments( QImage* img );
    int setUrlToFile( QImage* img );
    /**Reads the file wms_metadata.xml and extract the OnlineResource href. Returns 0 in case of success.*/
    int findOutHostAddress( QString& address ) const;
};

#endif
