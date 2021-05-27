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

#include "qgis_server.h"
#include "qgis_sip.h"

SIP_IF_MODULE( HAVE_SERVER_PYTHON_PLUGINS )


/**
 * \ingroup server
 * \class QgsAccessControl
 * \brief A helper class that centralizes restrictions given by all the access control filter plugins.
 * \since QGIS 2.14
 */
class SERVER_EXPORT QgsAccessControl : public QgsFeatureFilterProvider
{
#ifdef SIP_RUN
#include "qgsaccesscontrolfilter.h"
#endif

  public:
    //! Constructor
    QgsAccessControl()
    {
      mPluginsAccessControls = new QgsAccessControlFilterMap();
      mResolved = false;
    }

    //! Constructor
    QgsAccessControl( const QgsAccessControl &copy )
    {
      mPluginsAccessControls = new QgsAccessControlFilterMap( *copy.mPluginsAccessControls );
      mFilterFeaturesExpressions = copy.mFilterFeaturesExpressions;
      mResolved = copy.mResolved;
    }


    ~QgsAccessControl() override
    {
      delete mPluginsAccessControls;
    }

    //! Assignment operator
    QgsAccessControl &operator= ( const QgsAccessControl &other )
    {
      if ( this != &other )
      {
        delete mPluginsAccessControls;
        mPluginsAccessControls = new QgsAccessControlFilterMap( *other.mPluginsAccessControls );
        mFilterFeaturesExpressions = other.mFilterFeaturesExpressions;
        mResolved = other.mResolved;
      }
      return *this;
    }

    /**
     * Resolve features' filter of layers
     * \param layers to filter
     */
    void resolveFilterFeatures( const QList<QgsMapLayer *> &layers );

    /**
     * Filter the features of the layer
     * \param layer the layer to control
     * \param filterFeatures the request to fill
     */
    void filterFeatures( const QgsVectorLayer *layer, QgsFeatureRequest &filterFeatures ) const override;

    /**
     * Returns a clone of the object
     * \returns A clone
     */
    QgsFeatureFilterProvider *clone() const override SIP_FACTORY;

    /**
     * Returns an additional subset string (typically SQL) filter
     * \param layer the layer to control
     * \returns the subset string to use
     */
    QString extraSubsetString( const QgsVectorLayer *layer ) const;

    /**
     * Returns the layer read right
     * \param layer the layer to control
     * \returns TRUE if it can be read
     */
    bool layerReadPermission( const QgsMapLayer *layer ) const;

    /**
     * Returns the layer insert right
     * \param layer the layer to control
     * \returns TRUE if we can insert on it
     */
    bool layerInsertPermission( const QgsVectorLayer *layer ) const;

    /**
     * Returns the layer update right
     * \param layer the layer to control
     * \returns TRUE if we can do an update
     */
    bool layerUpdatePermission( const QgsVectorLayer *layer ) const;

    /**
     * Returns the layer delete right
     * \param layer the layer to control
     * \returns TRUE if we can do a delete
     */
    bool layerDeletePermission( const QgsVectorLayer *layer ) const;

    /**
     * Returns the authorized layer attributes
     * \param layer the layer to control
     * \param attributes the list of attribute
     * \returns the list of visible attributes
     */
    QStringList layerAttributes( const QgsVectorLayer *layer, const QStringList &attributes ) const override;

    /**
     * Are we authorized to modify the following geometry
     * \param layer the layer to control
     * \param feature the concerned feature
     * \returns TRUE if we are allowed to edit the feature
     */
    bool allowToEdit( const QgsVectorLayer *layer, const QgsFeature &feature ) const;

    /**
     * Fill the capabilities caching key
     * \param cacheKey the list to fill with a cache variant
     */
    bool fillCacheKey( QStringList &cacheKey ) const;

    /**
     * Register an access control filter
     * \param accessControl the access control to add
     * \param priority the priority used to define the order
     */
    void registerAccessControl( QgsAccessControlFilter *accessControl, int priority = 0 );

  private:
    QString resolveFilterFeatures( const QgsVectorLayer *layer ) const;

    //! The AccessControl plugins registry
    QgsAccessControlFilterMap *mPluginsAccessControls = nullptr;

    QMap<QString, QString> mFilterFeaturesExpressions;
    bool mResolved;

};

#endif
