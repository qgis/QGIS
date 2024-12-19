/***************************************************************************
                          qgsaccesscontrolfilter.h
                          ------------------------
 Access control interface for QGIS Server plugins

  begin                : 2015-05-19
  copyright            : (C) 2015 by Stéphane Brunner
  email                : stephane dot brunner at camptocamp dot org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSACCESSCONTROLPLUGIN_H
#define QGSACCESSCONTROLPLUGIN_H

#include <QMultiMap>
#include <QString>
#include "qgis_server.h"
#include "qgis_sip.h"

SIP_IF_MODULE( HAVE_SERVER_PYTHON_PLUGINS )

class QgsServerInterface;
class QgsMapLayer;
class QgsVectorLayer;
class QgsFeature;


/**
 * \ingroup server
 * \class QgsAccessControlFilter
 * \brief Class defining access control interface for QGIS Server plugins.
 *
 * Security can define any (or none) of the following method:
 *
 * - layerFilterExpression() - To set an additional QGIS expression filter (WMS/GetMap, WMS/GetFeatureInfo, WFS/GetFeature)
 * - layerFilterSubsetString() - To set an additional SQL subset string filter (WMS/GetMap, WMS/GetFeatureInfo, WFS/GetFeature) for layer that support SQL
 * - layerPermissions() - To set the general layer permissins (read / update / insert / delete)
 * - authorizedLayerAttributes() - To filter the attributes (WMS/GetFeatureInfo, WFS/GetFeature)
 * - allowToEdit() - (all WFS-T requests)
 * - cacheKey()
 */
class SERVER_EXPORT QgsAccessControlFilter
{
  public:
    /**
     * Constructor
     * QgsServerInterface passed to plugins constructors
     * and must be passed to QgsAccessControlFilter instances.
     */
    QgsAccessControlFilter( const QgsServerInterface *serverInterface );

    virtual ~QgsAccessControlFilter() = default;

    //! Describe the layer permission
    struct LayerPermissions
    {
        bool canRead;
        bool canUpdate;
        bool canInsert;
        bool canDelete;
    };

    //! Returns the QgsServerInterface instance
    const QgsServerInterface *serverInterface() const { return mServerInterface; }

    /**
     * Returns an additional expression filter
     * \param layer the layer to control
     * \returns the filter expression
     */
    virtual QString layerFilterExpression( const QgsVectorLayer *layer ) const;

    /**
     * Returns an additional subset string (typically SQL) filter
     * \param layer the layer to control
     * \returns the subset string
     */
    virtual QString layerFilterSubsetString( const QgsVectorLayer *layer ) const;

    /**
     * Returns the layer permissions
     * \param layer the layer to control
     * \returns the permission to use on the layer
     */
    virtual LayerPermissions layerPermissions( const QgsMapLayer *layer ) const;

    /**
     * Returns the authorized layer attributes
     * \param layer the layer to control
     * \param attributes the current list of visible attribute
     * \returns the new list of visible attributes
     */
    virtual QStringList authorizedLayerAttributes( const QgsVectorLayer *layer, const QStringList &attributes ) const;

    /**
     * Are we authorized to modify the following geometry
     * \param layer the layer to control
     * \param feature the concerned feature
     * \returns TRUE if we are allowed to edit
     */
    virtual bool allowToEdit( const QgsVectorLayer *layer, const QgsFeature &feature ) const;

    /**
     * Cache key to used to create the capabilities cache
     * \returns the cache key, "" for no cache
     */
    virtual QString cacheKey() const;

  private:
    //! The server interface
    const QgsServerInterface *mServerInterface = nullptr;
};

//! The registry definition
typedef QMultiMap<int, QgsAccessControlFilter *> QgsAccessControlFilterMap;


#endif // QGSSERVERSECURITY_H
