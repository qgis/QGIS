/***************************************************************************
                              qgswfsutils.h

  Define WFS service utility functions
  ------------------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  ( parts from qgswfshandler)
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
#include "qgsserversettings.h"

/**
 * \ingroup server
 * \brief WMS implementation
 */

//! WMS implementation
namespace QgsWfs
{

  /**
   * Returns the highest version supported by this implementation
   */
  QString implementationVersion();

  /**
   * Service URL string
   */
  QString serviceUrl( const QgsServerRequest &request, const QgsProject *project, const QgsServerSettings &settings );

  /**
   * Returns typename from vector layer
   */
  QString layerTypeName( const QgsMapLayer *layer );

  /**
   * Retrieve a layer by typename
   */
  QgsVectorLayer *layerByTypeName( const QgsProject *project, const QString &typeName );

  /**
   * Transform a Filter element to a feature request
   */
  QgsFeatureRequest parseFilterElement( const QString &typeName, QDomElement &filterElem, QgsProject *project = nullptr );

  /**
   * Transform a Filter element to a feature request and update server feature ids
   */
  QgsFeatureRequest parseFilterElement( const QString &typeName, QDomElement &filterElem, QStringList &serverFids, const QgsProject *project = nullptr, const QgsMapLayer *layer = nullptr );

  // Define namespaces used in WFS documents
  const QString WFS_NAMESPACE = QStringLiteral( "http://www.opengis.net/wfs" );
  const QString GML_NAMESPACE = QStringLiteral( "http://www.opengis.net/gml" );
  const QString OGC_NAMESPACE = QStringLiteral( "http://www.opengis.net/ogc" );
  const QString QGS_NAMESPACE = QStringLiteral( "http://www.qgis.org/gml" );

  // Define clean tagName regExp
  const QRegExp cleanTagNameRegExp( "(?![\\w\\d\\.-])." );

} // namespace QgsWfs

#endif


