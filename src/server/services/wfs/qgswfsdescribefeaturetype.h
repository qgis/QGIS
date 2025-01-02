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

#include "qgsserverinterface.h"
#include "qgswfsparameters.h"


namespace QgsWfs
{

  /**
   * Helper for returning the field type and type name
   */
  void getFieldAttributes( const QgsField &field, QString &fieldName, QString &fieldType );

  /**
   * Helper for returning typename list from the request
   */
  QStringList getRequestTypeNames( const QgsServerRequest &request, const QgsWfsParameters &wfsParams );


  /**
   * Output WFS  GetCapabilities response
   */
  void writeDescribeFeatureType( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request, QgsServerResponse &response );

} // namespace QgsWfs

#endif
