/***************************************************************************
                              qgswfsdescribefeaturetype.h
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
#ifndef QGSWFSDESCRIBEFEATURETYPE_H
#define QGSWFSDESCRIBEFEATURETYPE_H


#include <QDomDocument>
#include <QDomElement>

#include "qgsserverinterface.h"
#include "qgswfsparameters.h"

namespace QgsWfs
{

  /**
  * Returns the GML geometry type.
  */
  QString getGmlGeometryType( const QgsVectorLayer *layer, QgsWfsParameters::Format format );

  void setSchemaLayer( QDomElement &parentElement, QDomDocument &doc, const QgsVectorLayer *layer, QgsWfsParameters::Format format );

  /**
   * Create get capabilities document
   */
  QDomDocument createDescribeFeatureTypeDocument( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
      const QgsServerRequest &request );

  /**
   * Output WFS  GetCapabilities response
   */
  void writeDescribeFeatureType( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
                                 const QgsServerRequest &request, QgsServerResponse &response );

} // namespace QgsWfs

#endif
