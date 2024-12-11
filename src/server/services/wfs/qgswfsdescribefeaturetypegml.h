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
#ifndef QGSWFSDESCRIBEFEATURETYPEGML_H
#define QGSWFSDESCRIBEFEATURETYPEGML_H

#include <QDomDocument>
#include <QDomElement>

#include "qgsserverinterface.h"
#include "qgswfsparameters.h"

/**
 * \ingroup server
 * \class QgsWfsDescribeFeatureTypeGml
 * \brief GML output formatter for DescribeFeatureType
 *
 * \since QGIS 3.30
 */
class QgsWfsDescribeFeatureTypeGml
{
  private:
    /**
      * Returns the GML geometry type.
      */
    QString getGmlGeometryType( const QgsVectorLayer *layer ) const;

    void setSchemaLayer( QDomElement &parentElement, QDomDocument &doc, const QgsVectorLayer *layer ) const;

    /**
     * Create get capabilities document
     */
    QDomDocument createDescribeFeatureTypeDocument( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request ) const;

    const QgsWfs::QgsWfsParameters wfsParameters;

  public:
    /**
     * Constructor
     *
     * \param wfsParams WFS parameters
     */
    QgsWfsDescribeFeatureTypeGml( const QgsWfs::QgsWfsParameters wfsParams );

    /**
     * Output GML response
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
