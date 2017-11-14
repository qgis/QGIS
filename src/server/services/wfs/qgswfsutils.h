/***************************************************************************
                              qgswfsutils.h

  Define WFS service utility functions
  ------------------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  ( parts fron qgswfshandler)
                         (C) 2012 by René-Luc D'Hont    ( parts from qgswmshandler)
                         (C) 2014 by Alessandro Pasotti ( parts from qgswfshandler)
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
#ifndef QGSWFSUTILS_H
#define QGSWFSUTILS_H

#include "qgsmodule.h"
#include "qgsfeaturerequest.h"
#include "qgswfsserviceexception.h"

/**
 * \ingroup server
 * WMS implementation
 */

//! WMS implementation
namespace QgsWfs
{

  /**
   * Return the highest version supported by this implementation
   */
  QString implementationVersion();

  /**
   * Service URL string
   */
  QString serviceUrl( const QgsServerRequest &request, const QgsProject *project );

  /**
   * Transform a Filter element to a feature request
   */
  QgsFeatureRequest parseFilterElement( const QString &typeName, QDomElement &filterElem );

  // Define namespaces used in WFS documents
  const QString WFS_NAMESPACE = QStringLiteral( "http://www.opengis.net/wfs" );
  const QString GML_NAMESPACE = QStringLiteral( "http://www.opengis.net/gml" );
  const QString OGC_NAMESPACE = QStringLiteral( "http://www.opengis.net/ogc" );
  const QString QGS_NAMESPACE = QStringLiteral( "http://www.qgis.org/gml" );

} // namespace QgsWfs

#endif


