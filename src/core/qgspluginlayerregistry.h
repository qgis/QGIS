/***************************************************************************
                    qgspluginlayerregistry.cpp - class for
                    registering plugin layer creators
                             -------------------
    begin                : Mon Nov 30 2009
    copyright            : (C) 2009 by Mathias Walker, Sourcepole
    email                : mwa at sourcepole.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPLUGINLAYERREGSITRY_H
#define QGSPLUGINLAYERREGSITRY_H

#include <QMap>
#include "qgis_sip.h"
#include <QDomNode>

#include "qgis_core.h"

class QgsPluginLayer;

/**
 * \ingroup core
    class for creating plugin specific layers
*/
class CORE_EXPORT QgsPluginLayerType
{
  public:

    QgsPluginLayerType( const QString &name );
    virtual ~QgsPluginLayerType() = default;

    QString name();

    //! Returns new layer of this type. Return NULLPTR on error
    virtual QgsPluginLayer *createLayer() SIP_FACTORY;

    /**
     * Returns new layer of this type, using layer URI (specific to this plugin layer type). Return NULLPTR on error.
     * \since QGIS 2.10
     */
    virtual QgsPluginLayer *createLayer( const QString &uri ) SIP_FACTORY;

    //! Show plugin layer properties dialog. Return FALSE if the dialog cannot be shown.
    virtual bool showLayerProperties( QgsPluginLayer *layer );

  protected:
    QString mName;
};

//=============================================================================

/**
 * \ingroup core
 * A registry of plugin layers types.
 *
 * QgsPluginLayerRegistry is not usually directly created, but rather accessed through
 * QgsApplication::pluginLayerRegistry().
*/
class CORE_EXPORT QgsPluginLayerRegistry
{
  public:

    /**
     * Constructor for QgsPluginLayerRegistry.
     */
    QgsPluginLayerRegistry() = default;
    ~QgsPluginLayerRegistry();

    //! QgsPluginLayerRegistry cannot be copied.
    QgsPluginLayerRegistry( const QgsPluginLayerRegistry &rh ) = delete;
    //! QgsPluginLayerRegistry cannot be copied.
    QgsPluginLayerRegistry &operator=( const QgsPluginLayerRegistry &rh ) = delete;

    /**
     * List all known layer types
     */
    QStringList pluginLayerTypes();

    //! Add plugin layer type (take ownership) and return TRUE on success
    bool addPluginLayerType( QgsPluginLayerType *pluginLayerType SIP_TRANSFER );

    //! Remove plugin layer type and return TRUE on success
    bool removePluginLayerType( const QString &typeName );

    //! Returns plugin layer type metadata or NULLPTR if doesn't exist
    QgsPluginLayerType *pluginLayerType( const QString &typeName );

    /**
     * Returns new layer if corresponding plugin has been found else returns NULLPTR.
     * \note parameter uri has been added in QGIS 2.10
     */
    QgsPluginLayer *createLayer( const QString &typeName, const QString &uri = QString() ) SIP_FACTORY;

  private:
#ifdef SIP_RUN
    QgsPluginLayerRegistry( const QgsPluginLayerRegistry &rh );
#endif

    typedef QMap<QString, QgsPluginLayerType *> PluginLayerTypes;

    PluginLayerTypes mPluginLayerTypes;
};

#endif // QGSPLUGINLAYERREGSITRY_H
