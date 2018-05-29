/***************************************************************************
  qgsprojectbadlayerhandler.h - QgsProjectBadLayerHandler

 ---------------------
 begin                : 22.10.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROJECTBADLAYERHANDLER_H
#define QGSPROJECTBADLAYERHANDLER_H

#include <QDomNode>

#include "qgis_core.h"

/**
 * \ingroup core
 * Interface for classes that handle missing layer files when reading project file.
 */
class CORE_EXPORT QgsProjectBadLayerHandler
{
  public:

    /**
     * This method will be called whenever the project tries to load layers which
     * cannot be accessed. It should inform the user about this and if possible offer
     * to fix the unavailable layers by setting a valid datasource, e.g. by showing a file
     * dialog.
     *
     * The default implementation will dismiss all bad layers and write information to the
     * log.
     *
     * \since QGIS 3.0
     */
    virtual void handleBadLayers( const QList<QDomNode> &layers );
    virtual ~QgsProjectBadLayerHandler() = default;


  protected:

    //! file data representation
    enum DataType
    {
      IS_VECTOR, //!< Vector data
      IS_RASTER, //!< Raster data
      IS_BOGUS   //!< Bogus data
    };

    //! the flavors for data storage
    enum ProviderType
    {
      IS_FILE,     //!< Saved in a file
      IS_DATABASE, //!< Saved in a database
      IS_URL,      //!< Retrieved from a URL
      IS_Unknown   //!< Unknown type
    };


    /**
     * Returns data type associated with the given QgsProject file Dom node
     *
     * The Dom node should represent the state associated with a specific layer.
     *
     * \since QGIS 3.0
     */
    DataType dataType( const QDomNode &layerNode );

    /**
     * Returns the data source for the given layer
     *
     * The QDomNode is a QgsProject Dom node corresponding to a map layer state.
     *
     * Essentially dumps datasource tag.
     *
     * \since QGIS 3.0
     */
    QString dataSource( const QDomNode &layerNode );

    /**
     * Returns the physical storage type associated with the given layer
     *
     * The QDomNode is a QgsProject Dom node corresponding to a map layer state.
     *
     * If the provider tag is "ogr", then it's a file type.
     *
     * However, if the layer is a raster, then there won't be a
     * provider tag.  It will always have an associated file.
     *
     * If the layer doesn't fall into either of the previous two categories, then
     * it's either a database or URL.  If the datasource tag has "url=", then it's
     * URL based and if it has "dbname=">, then the layer data is in a database.
     *
     * \since QGIS 3.0
     */
    ProviderType providerType( const QDomNode &layerNode );

    /**
     * Set the datasource element to the new value
     *
     * \since QGIS 3.0
     */
    void setDataSource( QDomNode &layerNode, const QString &dataSource );
};

#endif // QGSPROJECTBADLAYERHANDLER_H
