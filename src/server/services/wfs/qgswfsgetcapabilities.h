/***************************************************************************
                              qgswfsgecapabilities.h
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
#ifndef QGSWFSGETCAPABILITIES_H
#define QGSWFSGETCAPABILITIES_H

#include <QDomDocument>

namespace QgsWfs
{

  /**
   * Create FeatureTypeList element for get capabilities document
   */
  QDomElement getFeatureTypeListElement( QDomDocument &doc, QgsServerInterface *serverIface, const QgsProject *project );

  /**
   * Create a parameter element
   */
  QDomElement getParameterElement( QDomDocument &doc, const QString &name, const QStringList &values );

  /**
   * Create OperationsMetadata element for get capabilities document
   */
  QDomElement getOperationsMetadataElement( QDomDocument &doc, const QgsProject *project, const QgsServerRequest &request, const QgsServerSettings *settings );

  /**
   * Create Service Provider element for get capabilities document
   */
  QDomElement getServiceProviderElement( QDomDocument &doc, const QgsProject *project );

  /**
   * Create Service Identification element for get capabilities document
   */
  QDomElement getServiceIdentificationElement( QDomDocument &doc, const QgsProject *project );

  /**
   * Create get capabilities document
   */
  QDomDocument createGetCapabilitiesDocument( QgsServerInterface *serverIface,
      const QgsProject *project, const QString &version,
      const QgsServerRequest &request );

  /**
   * Output WFS GetCapabilities response
   */
  void writeGetCapabilities( QgsServerInterface *serverIface, const QgsProject *project,
                             const QString &version, const QgsServerRequest &request,
                             QgsServerResponse &response );

} // namespace QgsWfs

#endif

