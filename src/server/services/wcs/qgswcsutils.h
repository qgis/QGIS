/***************************************************************************
                              qgswcsutils.h

  Define WCS service utility functions
  ------------------------------------
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
#ifndef QGSWCSUTILS_H
#define QGSWCSUTILS_H

#include "qgsmodule.h"
#include "qgswcsserviceexception.h"
#include "qgsserversettings.h"

#include "qgsrasterlayer.h"

#include <QDomDocument>

/**
 * \ingroup server
 * \brief WCS implementation
 */

//! WCS implementation
namespace QgsWcs
{

  /**
   * Returns the highest version supported by this implementation
   */
  QString implementationVersion();

  /**
   * CoverageOffering or CoverageOfferingBrief element
   */
  QDomElement getCoverageOffering( QDomDocument &doc, const QgsRasterLayer *layer, const QgsProject *project, bool brief = false );

  /**
   * Service URL string
   */
  QString serviceUrl( const QgsServerRequest &request, const QgsProject *project, const QgsServerSettings &settings );

  /**
   * Parse bounding box
   */
  //XXX At some point, should be moved to common library
  QgsRectangle parseBbox( const QString &bboxStr );

  // Define namespaces used in WCS documents
  const QString WCS_NAMESPACE = QStringLiteral( "http://www.opengis.net/wcs" );
  const QString GML_NAMESPACE = QStringLiteral( "http://www.opengis.net/gml" );
  const QString OGC_NAMESPACE = QStringLiteral( "http://www.opengis.net/ogc" );

} // namespace QgsWcs

#endif


