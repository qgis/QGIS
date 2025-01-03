/***************************************************************************
                              qgswcsgecapabilities.h
                              -------------------------
  begin                : January 16 , 2017
  copyright            : (C) 2013 by René-Luc D'Hont  ( parts from qgswcsserver )
                         (C) 2017 by David Marteau
  email                : rldhont at 3liz dot com
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
#ifndef QGSWCSGETCAPABILITIES_H
#define QGSWCSGETCAPABILITIES_H

#include <QDomDocument>

namespace QgsWcs
{

  /**
   * Create ContentMetadata element for get capabilities document
   */
  QDomElement getContentMetadataElement( QDomDocument &doc, QgsServerInterface *serverIface, const QgsProject *project );

  /**
   * Create Service element for get capabilities document
   */
  QDomElement getServiceElement( QDomDocument &doc, const QgsProject *project );

  /**
   * Create get capabilities document
   */
  QDomDocument createGetCapabilitiesDocument( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request );

  /**
   * Output WCS  GetCapabilities response
   */
  void writeGetCapabilities( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request, QgsServerResponse &response );

} // namespace QgsWcs

#endif
