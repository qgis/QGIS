/***************************************************************************
  qgsmaplayerfactory.h
  --------------------------------------
  Date                 : March 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERFACTORY_H
#define QGSMAPLAYERFACTORY_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgscoordinatetransformcontext.h"

#include <QString>

/**
 * \ingroup core
 * \brief Contains utility functions for creating map layers.
 *
 * \since QGIS 3.18.1
 */
class CORE_EXPORT QgsMapLayerFactory
{
  public:

    /**
     * Returns the map layer type corresponding a \a string value.
     *
     * \param string string to convert to map layer type
     * \param ok will be set to TRUE if \a string was successfully converted to a map layer type
     *
     * \returns converted map layer type
     *
     * \see typeToString()
     */
    static QgsMapLayerType typeFromString( const QString &string, bool &ok SIP_OUT );

    /**
     * Converts a map layer \a type to a string value.
     *
     * \see typeFromString()
     */
    static QString typeToString( QgsMapLayerType type );

    /**
     * Setting options for loading layers.
     *
     * \since QGIS 3.22
     */
    struct LayerOptions
    {

      /**
       * Constructor for LayerOptions with \a transformContext.
       */
      explicit LayerOptions( const QgsCoordinateTransformContext &transformContext )
        : transformContext( transformContext )
      {}

      //! Transform context
      QgsCoordinateTransformContext transformContext;

      //! Set to TRUE if the default layer style should be loaded
      bool loadDefaultStyle = true;

      /**
       * Controls whether the stored styles will be all loaded.
       *
       * If TRUE and the layer's provider supports style stored in the
       * data source all the available styles will be loaded in addition
       * to the default one.
       *
       * If FALSE (the default), the layer's provider will only load
       * the default style.
       *
       * \since QGIS 3.30
       */
      bool loadAllStoredStyles = false;
    };

    /**
     * Creates a map layer, given a \a uri, \a name, layer \a type and \a provider name.
     *
     * Caller takes ownership of the returned layer.
     *
     * \since QGIS 3.22
     */
    static QgsMapLayer *createLayer( const QString &uri, const QString &name, QgsMapLayerType type, const LayerOptions &options,
                                     const QString &provider = QString() ) SIP_FACTORY;

};

#endif // QGSMAPLAYERFACTORY_H
