/***************************************************************************
                          qgsaccesscontrolfilter.h
                          ------------------------
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
#include <QList>
#include <QString>

class QgsServerInterface;
class QgsMapLayer;
class QgsVectorLayer;
class QgsExpression;
class QgsFeature;


/**
 * \ingroup server
 * \class QgsAccessControlFilter
 * \brief Class defining access control interface for QGIS Server plugins.
 *
 * Security can define any (or none) of the following method:
 *  * layerFilterExpression() - To get an additional expression filter (WMS/GetMap, WMS/GetFeatureInfo, WFS/GetFeature)
 *  * layerFilterSQL() - To get an additional SQL filter (WMS/GetMap, WMS/GetFeatureInfo, WFS/GetFeature) for layer that support SQL
 *  * layerPermissions() - To give the general layer permissins (read / update / insert / delete)
 *  * authorizedLayerAttributes() - Tho filter the attributes (WMS/GetFeatureInfo, WFS/GetFeature)
 *  * allowToEdit() - (all WFS-T requests)
 */
class SERVER_EXPORT QgsAccessControlFilter
{

  public:

    /** Constructor
     * QgsServerInterface passed to plugins constructors
     * and must be passed to QgsAccessControlFilter instances.
     */
    QgsAccessControlFilter( const QgsServerInterface* serverInterface );
    /** Destructor */
    virtual ~QgsAccessControlFilter();

    /** Describe the layer permission */
    struct LayerPermissions
    {
      bool canRead;
      bool canUpdate;
      bool canInsert;
      bool canDelete;
    };

    /** Return the QgsServerInterface instance */
    const QgsServerInterface* serverInterface() const { return mServerInterface; }

    /** Return an additional expression filter
     * @param layer the layer to control
     * @return the filter expression
     */
    virtual QString layerFilterExpression( const QgsVectorLayer* layer ) const;

    /** Return an additional subset string (typically SQL) filter
     * @param layer the layer to control
     * @return the subset string
     */
    virtual QString layerFilterSubsetString( const QgsVectorLayer* layer ) const;

    /** Return the layer permissions
     * @param layer the layer to control
     * @return the permission to use on the layer
     */
    virtual LayerPermissions layerPermissions( const QgsMapLayer* layer ) const;

    /** Return the authorized layer attributes
     * @param layer the layer to control
     * @param attributes the current list of visible attribute
     * @return the new list of visible attributes
     */
    virtual QStringList authorizedLayerAttributes( const QgsVectorLayer* layer, const QStringList& attributes ) const;

    /** Are we authorized to modify the following geometry
     * @param layer the layer to control
     * @param feature the concerned feature
     * @return true if we are allowed to edit
     */
    virtual bool allowToEdit( const QgsVectorLayer* layer, const QgsFeature& feature ) const;

    /** Cache key to used to create the capabilities cache
     * @return the cache key, "" for no cache
     */
    virtual QString cacheKey() const;

  private:

    /** The server interface */
    const QgsServerInterface* mServerInterface;

};

/** The registry definition */
typedef QMultiMap<int, QgsAccessControlFilter*> QgsAccessControlFilterMap;


#endif // QGSSERVERSECURITY_H
