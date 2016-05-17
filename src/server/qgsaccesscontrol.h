/***************************************************************************
                              qgsaccesscontrol.h
                              ------------------
  begin                : 22-05-2015
  copyright            : (C) 2008 by Stéphane Brunner
  email                : stephane dot brunner at camptocamp dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSACCESSCONTROL_H
#define QGSACCESSCONTROL_H

#include "qgsfeaturefilterprovider.h"
#include "qgsaccesscontrolfilter.h"

#include <QMultiMap>

class QgsAccessControlPlugin;


/**
 * \ingroup server
 * \class QgsAccessControl
 * \brief A helper class that centralise the restrictions given by all the
 *        access control filter plugins.
 **/
class SERVER_EXPORT QgsAccessControl : public QgsFeatureFilterProvider
{
  public:
    /** Constructor */
    QgsAccessControl()
    {
      mPluginsAccessControls = new QgsAccessControlFilterMap();
    }

    /** Constructor */
    QgsAccessControl( const QgsAccessControl& copy )
    {
      mPluginsAccessControls = new QgsAccessControlFilterMap( *copy.mPluginsAccessControls );
    }

    /** Destructor */
    ~QgsAccessControl()
    {
      delete mPluginsAccessControls;
    }

    /** Filter the features of the layer
     * @param layer the layer to control
     * @param filterFeatures the request to fill
     */
    void filterFeatures( const QgsVectorLayer* layer, QgsFeatureRequest& filterFeatures ) const;

    /** Return a clone of the object
     * @return A clone
     */
    QgsFeatureFilterProvider* clone() const;

    /** Return an additional subset string (typically SQL) filter
     * @param layer the layer to control
     * @return the subset string to use
     */
    QString extraSubsetString( const QgsVectorLayer* layer ) const;

    /** Return the layer read right
     * @param layer the layer to control
     * @return true if it can be read
     */
    bool layerReadPermission( const QgsMapLayer* layer ) const;

    /** Return the layer insert right
     * @param layer the layer to control
     * @return true if we can insert on it
     */
    bool layerInsertPermission( const QgsVectorLayer* layer ) const;

    /** Return the layer update right
     * @param layer the layer to control
     * @return true if we can do an update
     */
    bool layerUpdatePermission( const QgsVectorLayer* layer ) const;

    /** Return the layer delete right
     * @param layer the layer to control
     * @return true if we can do a delete
     */
    bool layerDeletePermission( const QgsVectorLayer* layer ) const;

    /** Return the authorized layer attributes
     * @param layer the layer to control
     * @param attributes the list of attribute
     * @return the list of visible attributes
     */
    QStringList layerAttributes( const QgsVectorLayer* layer, const QStringList& attributes ) const;

    /** Are we authorized to modify the following geometry
     * @param layer the layer to control
     * @param feature the concerned feature
     * @return true if we are allowed to edit the feature
     */
    bool allowToEdit( const QgsVectorLayer* layer, const QgsFeature& feature ) const;

    /** Fill the capabilities caching key
     * @param cacheKey the list to fill with a cache variant
     * @return false if we cant create a cache
     */
    bool fillCacheKey( QStringList& cacheKey ) const;

    /** Register an access control filter
     * @param accessControl the access control to add
     * @priority the priority used to define the order
     */
    void registerAccessControl( QgsAccessControlFilter* accessControl, int priority = 0 );

  private:
    /** The AccessControl plugins registry */
    QgsAccessControlFilterMap* mPluginsAccessControls;
};

#endif
