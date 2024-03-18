/***************************************************************************
                         qgsrasterrendererregistry.h
                         ---------------------------
    begin                : January 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERRENDERERREGISTRY_H
#define QGSRASTERRENDERERREGISTRY_H

#include "qgis_core.h"
#include "qgis.h"
#include <QHash>
#include <QString>

class QDomElement;
class QgsRasterInterface;
class QgsRasterLayer;
class QgsRasterRenderer;
class QgsRasterRendererWidget;
class QgsRasterDataProvider;
class QgsRectangle;

#ifndef SIP_RUN
typedef QgsRasterRenderer *( *QgsRasterRendererCreateFunc )( const QDomElement &, QgsRasterInterface *input );
typedef QgsRasterRendererWidget *( *QgsRasterRendererWidgetCreateFunc )( QgsRasterLayer *, const QgsRectangle &extent );

/**
 * \ingroup core
  * \brief Registry for raster renderer entries.
  *
  * \note Not available in Python bindings
  */
struct CORE_EXPORT QgsRasterRendererRegistryEntry
{

  /**
   * Constructor for QgsRasterRendererRegistryEntry.
   *
   * Since QGIS 3.38, the \a capabilities argument can be used to specify renderer capabilities.
   */
  QgsRasterRendererRegistryEntry( const QString &name, const QString &visibleName, QgsRasterRendererCreateFunc rendererFunction,
                                  QgsRasterRendererWidgetCreateFunc widgetFunction, Qgis::RasterRendererCapabilities capabilities = Qgis::RasterRendererCapabilities() );

  /**
   * Constructor for QgsRasterRendererRegistryEntry.
   */
  QgsRasterRendererRegistryEntry() = default;
  QString name;
  QString visibleName; //visible (and translatable) name

  /**
   * Renderer capabilities.
   *
   * \since QGIS 3.38
   */
  Qgis::RasterRendererCapabilities capabilities;

  QIcon icon();
  QgsRasterRendererCreateFunc rendererCreateFunction = nullptr ; //pointer to create function
  QgsRasterRendererWidgetCreateFunc widgetCreateFunction = nullptr ; //pointer to create function for renderer widget
};

#endif

/**
 * \ingroup core
  * \brief Registry for raster renderers.
  *
  * QgsRasterRendererRegistry is not usually directly created, but rather accessed through
  * QgsApplication::rasterRendererRegistry().
  *
  * \note Exposed to Python bindings in QGIS 3.38
  */
class CORE_EXPORT QgsRasterRendererRegistry
{
  public:

    /**
     * Constructor for QgsRasterRendererRegistry.
     *
     * QgsRasterRendererRegistry is not usually directly created, but rather accessed through
     * QgsApplication::rasterRendererRegistry().
     *
     * The registry is pre-populated with standard raster renderers.
     */
    QgsRasterRendererRegistry();

    /**
     * Inserts a new \a entry into the registry.
     *
     * \note Not available in Python bindings
     */
    void insert( const QgsRasterRendererRegistryEntry &entry ) SIP_SKIP;

    /**
     * Sets the widget creation function for a renderer.
     *
     * \note Not available in Python bindings
     */
    void insertWidgetFunction( const QString &rendererName, QgsRasterRendererWidgetCreateFunc func ) SIP_SKIP;

    /**
     * Retrieves renderer data from the registry.
     *
     * \note Not available in Python bindings
     */
    bool rendererData( const QString &rendererName, QgsRasterRendererRegistryEntry &data ) const SIP_SKIP;

    /**
     * Returns a list of the names of registered renderers.
     */
    QStringList renderersList() const;

    /**
     * Returns the list of registered renderers.
     *
     * \note Not available in Python bindings
     */
    QList< QgsRasterRendererRegistryEntry > entries() const SIP_SKIP;

    /**
     * Returns the capabilities for the renderer with the specified name.
     *
     * \since QGIS 3.38
     */
    Qgis::RasterRendererCapabilities rendererCapabilities( const QString &rendererName ) const;

    /**
     * Creates a default renderer for a raster drawing style (considering user options such as default contrast enhancement).
     * Caller takes ownership.
    */
    QgsRasterRenderer *defaultRendererForDrawingStyle( Qgis::RasterDrawingStyle drawingStyle, QgsRasterDataProvider *provider ) const SIP_FACTORY;

  private:
    QHash< QString, QgsRasterRendererRegistryEntry > mEntries;
    QStringList mSortedEntries;

    //read min/max values from
    bool minMaxValuesForBand( int band, QgsRasterDataProvider *provider, double &minValue, double &maxValue ) const;
};

#endif // QGSRASTERRENDERERREGISTRY_H
