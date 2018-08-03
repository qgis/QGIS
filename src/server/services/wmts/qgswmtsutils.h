/***************************************************************************
                              qgswmtsutils.h

  Define WMTS service utility functions
  ------------------------------------
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
#ifndef QGSWMTSUTILS_H
#define QGSWMTSUTILS_H

#include "qgsmodule.h"
#include "qgswmtsserviceexception.h"

#include <QDomDocument>

/**
 * \ingroup server
 * WMTS implementation
 */

//! WMTS implementation
namespace QgsWmts
{

  struct tileMatrixInfo
  {
    QString ref;

    QgsRectangle extent;

    double scaleDenominator = 0.0;

    QgsUnitTypes::DistanceUnit unit;
  };

  struct tileMatrix
  {
    double resolution = 0.0;

    double scaleDenominator = 0.0;

    int col = 0;

    int row = 0;

    double left = 0.0;

    double top = 0.0;
  };

  struct tileMatrixSet
  {
    QString ref;

    QgsRectangle extent;

    QgsUnitTypes::DistanceUnit unit;

    QList< tileMatrix > tileMatrixList;
  };

  struct layerDef
  {
    QString id;

    QString title;

    QString abstract;

    QgsRectangle wgs84BoundingRect;

    QStringList formats;

    bool queryable = false;
  };

  /**
   * Returns the highest version supported by this implementation
   */
  QString implementationVersion();

  /**
   * Service URL string
   */
  QString serviceUrl( const QgsServerRequest &request, const QgsProject *project );

  /**
   * Parse bounding box
   */
  //XXX At some point, should be moved to common library
  QgsRectangle parseBbox( const QString &bboxStr );

  // Define namespaces used in WMTS documents
  const QString WMTS_NAMESPACE = QStringLiteral( "http://www.opengis.net/wmts/1.0" );
  const QString GML_NAMESPACE = QStringLiteral( "http://www.opengis.net/gml" );
  const QString OWS_NAMESPACE = QStringLiteral( "http://www.opengis.net/ows/1.1" );

  tileMatrixInfo getTileMatrixInfo( const QString &crsStr, const QgsProject *project );
  tileMatrixSet getTileMatrixSet( tileMatrixInfo tmi, double minScale );
  double getProjectMinScale( const QgsProject *project );
  QList< tileMatrixSet > getTileMatrixSetList( const QgsProject *project );

  /**
   * Translate WMTS parameters to WMS query item
   */
  QUrlQuery translateWmtsParamToWmsQueryItem( const QString &request, const QgsServerRequest::Parameters &params,
      const QgsProject *project, QgsServerInterface *serverIface );

} // namespace QgsWmts

#endif


