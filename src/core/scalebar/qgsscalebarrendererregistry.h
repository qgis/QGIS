/***************************************************************************
    QgsScaleBarRendererRegistry.h
    ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSCALEBARRENDERERREGISTRY_H
#define QGSSCALEBARRENDERERREGISTRY_H

#include "qgis_core.h"
#include "qgis.h"

class QgsScaleBarRenderer;

/**
 * \ingroup core
 * \brief The QgsScaleBarRendererRegistry manages registered scalebar renderers.
 *
 * A reference to the QgsScaleBarRendererRegistry can be obtained from
 * QgsApplication::scalebarRendererRegistry().
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsScaleBarRendererRegistry
{

  public:

    /**
     * You should not normally need to create your own scalebar renderer registry.
     *
     * Use the one provided by `QgsApplication::scalebarRendererRegistry()` instead.
     */
    explicit QgsScaleBarRendererRegistry();
    ~QgsScaleBarRendererRegistry();

    /**
     * Returns a list of the renderer ids currently contained in the registry.
     */
    QStringList renderers() const;

    /**
     * Returns a list of the renderer ids currently contained in the registry,
     * sorted in an order respecting the renderer's sort keys and display strings.
     */
    QStringList sortedRendererList() const;

    /**
     * Adds a new \a renderer to the registry.
     *
     * Ownership is transferred to the registry.
     */
    void addRenderer( QgsScaleBarRenderer *renderer SIP_TRANSFER );

    /**
     * Removes the renderer with matching \a id from the registry.
     */
    void removeRenderer( const QString &id );

    /**
     * Creates a new scalebar renderer by \a id. If there is no such \a id registered,
     * NULLPTR will be returned instead.
     *
     * The caller takes ownership of the returned object.
     */
    QgsScaleBarRenderer *renderer( const QString &id ) const SIP_TRANSFERBACK;

    /**
     * Returns the translated, user-visible name for the renderer with matching \a id.
     */
    QString visibleName( const QString &id ) const;

    /**
     * Returns the sorting key for the renderer with matching \a id.
     */
    int sortKey( const QString &id ) const;

  private:
    QHash<QString, QgsScaleBarRenderer *> mRenderers;
};
#endif // QGSSCALEBARRENDERERREGISTRY_H
