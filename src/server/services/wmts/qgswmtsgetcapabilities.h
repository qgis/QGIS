/***************************************************************************
                              qgswmtsgecapabilities.h
                              -------------------------
  begin                : July 23 , 2017
  copyright            : (C) 2018 by René-Luc D'Hont
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
#ifndef QGSWMTSGETCAPABILITIES_H
#define QGSWMTSGETCAPABILITIES_H

#include <QDomDocument>

namespace QgsWmts
{

  /**
   * Create Contents element for get capabilities document
   */
  QDomElement getContentsElement( QDomDocument &doc, QgsServerInterface *serverIface, const QgsProject *project );

  /**
   * Create OperationsMetadata element for get capabilities document
   */
  QDomElement getOperationsMetadataElement( QDomDocument &doc, const QgsProject *project, const QgsServerRequest &request, const QgsServerSettings *settings );

  /**
   * Create ServiceProvider element for get capabilities document
   */
  QDomElement getServiceProviderElement( QDomDocument &doc, const QgsProject *project );

  /**
   * Create ServiceIdentification element for get capabilities document
   */
  QDomElement getServiceIdentificationElement( QDomDocument &doc, const QgsProject *project );

  /**
   * Create get capabilities document
   */
  QDomDocument createGetCapabilitiesDocument( QgsServerInterface *serverIface,
      const QgsProject *project, const QString &version,
      const QgsServerRequest &request );

  /**
   * Output WCS  GetCapabilities response
   */
  void writeGetCapabilities( QgsServerInterface *serverIface, const QgsProject *project,
                             const QString &version, const QgsServerRequest &request,
                             QgsServerResponse &response );

} // namespace QgsWcs

#endif

