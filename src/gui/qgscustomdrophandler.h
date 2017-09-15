/***************************************************************************
    qgscustomdrophandler.h
    ---------------------
    begin                : August 2016
    copyright            : (C) 2016 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCUSTOMDROPHANDLER_H
#define QGSCUSTOMDROPHANDLER_H

#include "qgsmimedatautils.h"
#include "qgis_gui.h"

/** \ingroup gui
 * Abstract base class that may be implemented to handle new types of data to be dropped in QGIS.
 * Implementations will be used when a QgsMimeDataUtils::Uri has layerType equal to "custom",
 * and the providerKey is equal to key() returned by the implementation.
 *
 * Alternatively, implementations can override the handleMimeData() or handleFileDrop()
 * methods to handle QMimeData and file drops directly. Reimplementation of these methods
 * does not rely on key() matching.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsCustomDropHandler
{
  public:
    virtual ~QgsCustomDropHandler() = default;

    //! Type of custom URI recognized by the handler
    virtual QString key() const = 0;

    //! Method called from QGIS after a drop event with custom URI known by the handler
    virtual void handleDrop( const QgsMimeDataUtils::Uri &uri ) const;

    /**
     * Called when the specified mime \a data has been dropped onto QGIS.
     *
     * The base class implementation does nothing.
     *
     * Subclasses should take care when overriding this method. When a drop event
     * occurs, Qt will lock the source application of the drag for the duration
     * of the drop event handling (e.g. dragging files from explorer to QGIS will
     * lock the explorer window until the drop handling has been complete).
     *
     * Accordingly, only implementations must be lightweight and return ASAP.
     * (For instance by copying the relevant parts of \a data and then handling
     * the data after a short timeout).
     */
    virtual void handleMimeData( const QMimeData *data );

    /**
     * Called when the specified \a file has been dropped onto QGIS. If true
     * is returned, then the handler has accepted this file and it should not
     * be further processed (e.g. by other QgsCustomDropHandlers).
     *
     * The base class implementation does nothing.
     *
     * This method is not called directly while drop handling is occurring,
     * so the limitations described in handleMimeData() about returning
     * quickly do not apply.
     */
    virtual bool handleFileDrop( const QString &file );
};

#endif // QGSCUSTOMDROPHANDLER_H
