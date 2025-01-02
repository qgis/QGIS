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

class QgsMapCanvas;

/**
 * \ingroup gui
 * \brief Abstract base class that may be implemented to handle new types of data to be dropped in QGIS.
 *
 * Implementations have three approaches they can use to handle drops.
 *
 * - The simplest approach is to implement handleFileDrop() when they need to handle
 *   dropped files (i.e. with mime type "text/uri-list").
 * - Reimplement handleCustomUriDrop() when they want to handle dropped custom
 *   QgsMimeDataUtils::Uri entries, for instance handling dropping custom entries
 *   from the browser tree (with mime type "application/x-vnd.qgis.qgis.uri"). In
 *   this case the implementation's customUriProviderKey() must match the uri
 *   entry's providerKey.
 * - Reimplement handleMimeData() to directly handle dropped QMimeData.
 *   Subclasses should take care when overriding this method. When a drop event
 *   occurs, Qt will lock the source application of the drag for the duration
 *   of the drop event handling via handleMimeData() (e.g. dragging files from
 *   explorer to QGIS will lock the explorer window until the drop handling has
 *   been complete). Accordingly handleMimeData() implementations must return
 *   quickly and defer any intensive or slow processing.
 *
 */
class GUI_EXPORT QgsCustomDropHandler : public QObject
{
    Q_OBJECT

  public:
    /**
     * Type of custom URI recognized by the handler. This must match
     * the URI entry's providerKey in order for handleCustomUriDrop()
     * to be called.
     *
     * \see handleCustomUriDrop()
     */
    virtual QString customUriProviderKey() const;

    /**
     * Called from QGIS after a drop event with custom URI known by the handler.
     *
     * In order for handleCustomUriDrop() to be called, subclasses must
     * also implement customUriProviderKey() to indicate the providerKey
     * value which the handler accepts.
     *
     * \see customUriProviderKey()
     */
    virtual void handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const;

    /**
     * Returns TRUE if the handler is capable of handling the provided mime \a data.
     * The base class implementation returns FALSE regardless of mime data.
     *
     * This method is called when mime data is dragged over the QGIS window, in order
     * to determine whether any handlers are capable of handling the data and to
     * determine whether the drag action should be accepted.
     *
     * \since QGIS 3.10
     */
    virtual bool canHandleMimeData( const QMimeData *data );

    // TODO QGIS 4.0 - return bool

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
     *
     * \deprecated QGIS 3.10. Use handleMimeDataV2() instead.
     */
    Q_DECL_DEPRECATED virtual void handleMimeData( const QMimeData *data ) SIP_DEPRECATED;

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
     *
     * If the function returns TRUE, it means the handler has accepted the drop
     * and it should not be further processed (e.g. by other QgsCustomDropHandlers)
     *
     * \since QGIS 3.10
     */
    virtual bool handleMimeDataV2( const QMimeData *data );

    /**
     * Called when the specified \a file has been dropped onto QGIS. If TRUE
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

    /**
     * Returns TRUE if the handler is capable of handling the provided mime \a uri
     * when dropped onto a map \a canvas.
     *
     * The base class implementation returns FALSE regardless of mime data.
     *
     * This method is called when mime data is dragged over a map canvas, in order
     * to determine whether any handlers are capable of handling the data and to
     * determine whether the drag action should be accepted.
     *
     * \warning Subclasses should be very careful about implementing this. If they
     * incorrectly return TRUE to a \a uri, it will prevent the default application
     * drop handling from occurring and will break the ability to drag and drop layers
     * and files onto QGIS.
     *
     * \since QGIS 3.10
     */
    virtual bool canHandleCustomUriCanvasDrop( const QgsMimeDataUtils::Uri &uri, QgsMapCanvas *canvas );

    /**
     * Called from QGIS after a drop event with custom \a uri known by the handler occurs
     * onto a map \a canvas.
     *
     * In order for handleCustomUriCanvasDrop() to be called, subclasses must
     * also implement customUriProviderKey() to indicate the providerKey
     * value which the handler accepts.
     *
     * If the function returns TRUE, it means the handler has accepted the drop
     * and it should not be further processed (e.g. by other QgsCustomDropHandlers).
     *
     * Subclasses which implement this must also implement corresponding versions of
     * canHandleCustomUriCanvasDrop().
     *
     * \see customUriProviderKey()
     * \see canHandleCustomUriCanvasDrop()
     * \since QGIS 3.10
     */
    virtual bool handleCustomUriCanvasDrop( const QgsMimeDataUtils::Uri &uri, QgsMapCanvas *canvas ) const;
};

#endif // QGSCUSTOMDROPHANDLER_H
