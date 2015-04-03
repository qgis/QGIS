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
#include <QDomNode>

class QgsPluginLayer;

/** \ingroup core
    class for creating plugin specific layers
*/
class CORE_EXPORT QgsPluginLayerType
{
  public:

    QgsPluginLayerType( QString name );
    virtual ~QgsPluginLayerType();

    QString name();

    /** return new layer of this type. Return NULL on error */
    virtual QgsPluginLayer* createLayer();

    /** return new layer of this type, using layer URI (specific to this plugin layer type). Return NULL on error.
     * @note added in 2.10
     */
    virtual QgsPluginLayer* createLayer( const QString& uri );

    /** show plugin layer properties dialog. Return false if the dialog cannot be shown. */
    virtual bool showLayerProperties( QgsPluginLayer* layer );

  protected:
    QString mName;
};

//=============================================================================

/** \ingroup core
    a registry of plugin layers types
*/
class CORE_EXPORT QgsPluginLayerRegistry
{
  public:

    /** means of accessing canonical single instance  */
    static QgsPluginLayerRegistry* instance();

    ~QgsPluginLayerRegistry();

    /** list all known layer types
     *  \note added in v2.1 */
    QStringList pluginLayerTypes();

    /** add plugin layer type (take ownership) and return true on success */
    bool addPluginLayerType( QgsPluginLayerType* pluginLayerType );

    /** remove plugin layer type and return true on success */
    bool removePluginLayerType( QString typeName );

    /** return plugin layer type metadata or NULL if doesn't exist */
    QgsPluginLayerType* pluginLayerType( QString typeName );

    /** return new layer if corresponding plugin has been found, else return NULL.
     * @note optional param uri added in 2.10
     */
    QgsPluginLayer* createLayer( QString typeName, const QString& uri = QString() );

  private:

    typedef QMap<QString, QgsPluginLayerType*> PluginLayerTypes;

    /** private since instance() creates it */
    QgsPluginLayerRegistry();

    /** pointer to canonical Singleton object */
    static QgsPluginLayerRegistry* _instance;

    PluginLayerTypes mPluginLayerTypes;
};

#endif // QGSPLUGINLAYERREGSITRY_H
