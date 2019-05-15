/***************************************************************************
                          qgsserverapiutils.h

  Class defining utilities for QGIS server APIs.
  -------------------
  begin                : 2019-04-16
  copyright            : (C) 2019 by Alessandro Pasotti
  email                : elpaso at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSSERVERAPIUTILS_H
#define QGSSERVERAPIUTILS_H

#include "qgis_server.h"
#include <QString>
#include "qgsproject.h"

class QgsRectangle;
class QgsCoordinateReferenceSystem;
class QgsVectorLayer;

#ifndef SIP_RUN
#include "nlohmann/json_fwd.hpp"
using json = nlohmann::json;
#endif


class SERVER_EXPORT QgsServerApiUtils
{

  public:

    /**
     * Parses the comma separated \a bbox into a (possibily empty) QgsRectangle, if
     * \note Z values (i.e. a 6 elements bbox) are silently discarded
     */
    static QgsRectangle parseBbox( const QString &bbox );

    /**
     * layerExtent returns json array with [xMin,yMin,xMax,yMax] CRS84 extent for the given \a layer
     * FIXME: the OpenAPI swagger docs say that it is inverted axis order: West, north, east, south edges of the spatial extent.
     *        but current example implementations and GDAL assume it's not.
     */
    static json layerExtent( const QgsVectorLayer *layer ) SIP_SKIP;

    /**
     * Pasrses the CRS URI \a bboxCrs (example: "http://www.opengis.net/def/crs/OGC/1.3/CRS84") into a QGIS CRS object
     */
    static QgsCoordinateReferenceSystem parseCrs( const QString &bboxCrs );


};
#endif // QGSSERVERAPIUTILS_H
