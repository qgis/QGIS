/***************************************************************************
                              qgswfsdescribefeaturetypejson.h
                              -------------------------------
  begin                : December 09 , 2022
  copyright            : (C) 2022 by David Marteau
  email                : david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWFSDESCRIBEFEATURETYPEGEOJSON_H
#define QGSWFSDESCRIBEFEATURETYPEGEOJSON_H

#include "qgsserverinterface.h"
#include "qgswfsparameters.h"

#include <QJsonObject>

/**
 * \ingroup server
 * \class QgsWfsDescribeFeatureTypeJson
 * \brief Json output formatter for DescribeFeatureType
 *
 * \since QGIS 3.30
 */
class QgsWfsDescribeFeatureTypeJson
{
  private:
    /**
      * Returns the GML geometry type.
      */
    void getGeometryType( const QgsVectorLayer *layer, QString &geomType, QString &geomLocalType ) const;

    QJsonObject schemaLayerToJson( const QgsVectorLayer *layer ) const;

    /**
     * Create get capabilities document
     */
    QJsonObject createDescribeFeatureTypeDocument( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request ) const;

    const QgsWfs::QgsWfsParameters wfsParameters;

  public:
    /**
     * Constructor
     *
     * \param wfsParams WFS parameters
     */
    QgsWfsDescribeFeatureTypeJson( const QgsWfs::QgsWfsParameters wfsParams );

    /**
     * Output GeoJson response
     *
     * \param serverIface Server interface
     * \param project Qgis project
     * \param version The WFS version
     * \param request Input request handler
     * \param response Output response handler
     */
    void writeDescribeFeatureType( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request, QgsServerResponse &response ) const;
};


#endif
