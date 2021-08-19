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
#include "qgswmtsparameters.h"
#include "qgswmtsserviceexception.h"
#include "qgsserversettings.h"

#include <QDomDocument>

/**
 * \ingroup server
 * \brief WMTS implementation
 * \since QGIS 3.4
 */

//! WMTS implementation
namespace QgsWmts
{

  struct tileMatrixInfo
  {
    QString ref;

    QgsRectangle extent;

    QgsUnitTypes::DistanceUnit unit = QgsUnitTypes::DistanceMeters;

    bool hasAxisInverted = false;

    double resolution = 0.0;

    double scaleDenominator = 0.0;

    int lastLevel = -1;
  };

  struct tileMatrixDef
  {
    double resolution = 0.0;

    double scaleDenominator = 0.0;

    int col = 0;

    int row = 0;

    double left = 0.0;

    double top = 0.0;
  };

  struct tileMatrixSetDef
  {
    QString ref;

    QgsRectangle extent;

    QgsUnitTypes::DistanceUnit unit;

    bool hasAxisInverted = false;

    QList< tileMatrixDef > tileMatrixList;
  };

  struct tileMatrixLimitDef
  {
    int minCol;

    int maxCol;

    int minRow;

    int maxRow;
  };

  struct tileMatrixSetLinkDef
  {
    QString ref;

    QMap< int, tileMatrixLimitDef > tileMatrixLimits;
  };

  struct layerDef
  {
    QString id;

    QString title;

    QString abstract;

    QgsRectangle wgs84BoundingRect;

    QStringList formats;

    bool queryable = false;

    double maxScale = 0.0;

    double minScale = 0.0;
  };

  /**
   * Returns the highest version supported by this implementation
   */
  QString implementationVersion();

  /**
   * Service URL string
   */
  QString serviceUrl( const QgsServerRequest &request, const QgsProject *project, const QgsServerSettings &settings );

  // Define namespaces used in WMTS documents
  const QString WMTS_NAMESPACE = QStringLiteral( "http://www.opengis.net/wmts/1.0" );
  const QString GML_NAMESPACE = QStringLiteral( "http://www.opengis.net/gml" );
  const QString OWS_NAMESPACE = QStringLiteral( "http://www.opengis.net/ows/1.1" );

  tileMatrixInfo calculateTileMatrixInfo( const QString &crsStr, const QgsProject *project );
  tileMatrixSetDef calculateTileMatrixSet( tileMatrixInfo tmi, double minScale );
  double getProjectMinScale( const QgsProject *project );
  QList< tileMatrixSetDef > getTileMatrixSetList( const QgsProject *project, const QString &tms_ref = QString() );

  QList< layerDef > getWmtsLayerList( QgsServerInterface *serverIface, const QgsProject *project );
  tileMatrixSetLinkDef getLayerTileMatrixSetLink( const layerDef layer, const tileMatrixSetDef tms, const QgsProject *project );

  /**
   * Translate WMTS parameters to WMS query item
   */
  QUrlQuery translateWmtsParamToWmsQueryItem( const QString &request, const QgsWmtsParameters &params,
      const QgsProject *project, QgsServerInterface *serverIface );

} // namespace QgsWmts

#endif


