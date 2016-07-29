/***************************************************************************
                          qgsaccesscontrolfilter.sip
                          --------------------------
 Access control interface for Qgis Server plugins

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

/**
 * \class QgsAccessControlFilter
 * \brief Class defining access control interface for QGIS Server.
 *
 * Security can define any (or none) of the following method:
 *  * layerFilterExpression()
 *  * layerFilterSubsetString()
 *  * layerPermissions()
 *  * authorizedLayerAttributes()
 *  * allowToEdit()
 *  * cacheKey()
 */

class QgsAccessControlFilter
{
%TypeHeaderCode
#include "qgsaccesscontrolfilter.h"
%End

  public:
    /** Constructor
     * QgsServerInterface passed to plugins constructors
     * and must be passed to QgsAccessControlPlugin instances.
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

    /** Return the QgsServerInterface instance*/
    const QgsServerInterface* serverInterface() const;
    /** Return an additional expression filter */
    virtual QString layerFilterExpression( const QgsVectorLayer* layer ) const;
    /** Return an additional the subset string (typically SQL) filter.
        Faster than the layerFilterExpression but not supported on all the type of layer */
    virtual QString layerFilterSubsetString( const QgsVectorLayer* layer ) const;
    /** Return the layer permissions */
    virtual LayerPermissions layerPermissions( const QgsMapLayer* layer ) const;
    /** Return the authorized layer attributes */
    virtual QStringList authorizedLayerAttributes( const QgsVectorLayer* layer, const QStringList& attributes ) const;
    /** Are we authorize to modify the following geometry */
    virtual bool allowToEdit( const QgsVectorLayer* layer, const QgsFeature& feature ) const;
    /** Cache key to used to create the capabilities cache, "" for no cache, shouldn't any contains "-", default to "" */
    virtual QString cacheKey() const;
};

typedef QMultiMap<int, QgsAccessControlFilter*> QgsAccessControlFilterMap;
