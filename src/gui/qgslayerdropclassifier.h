/***************************************************************************
  qgslayerdropclassifier.h
  --------------------------------------
  Date                 : July 2026
  Copyright            : (C) 2026 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERDROPCLASSIFIER_H
#define QGSLAYERDROPCLASSIFIER_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QPointer>
#include <QVector>

#define SIP_NO_FILE

class QMimeData;
class QgsCustomDropHandler;


/**
 * \ingroup gui
 * \brief Classifies the payload of a drag and drop event dropped onto a widget
 * which accepts map layers, such as the layer tree view or the map canvas.
 *
 * The classification drives the visual feedback shown to the user while a drag
 * hovers such a widget (e.g. an insertion indicator, a "load project" overlay or
 * a "cannot be loaded" hint).
 *
 * The widget itself is responsible for handling the drop event, and may use the classification.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsLayerDropClassifier
{
  public:
    /**
     * Classifies the \a mimeData of a drag or drop event.
     *
     * If no data provider recognizes the payload, the \a customHandlers are consulted:
     * when one of them can handle the mime data, the payload classifies as
     * Qgis::LayerDropPayloadType::CustomHandler, otherwise as Qgis::LayerDropPayloadType::Invalid.
     */
    static Qgis::LayerDropPayloadType classify( const QMimeData *mimeData, const QVector<QPointer<QgsCustomDropHandler>> &customHandlers = QVector<QPointer<QgsCustomDropHandler>>() );

    /**
     * Returns TRUE if the \a mimeData represents a drag of datasets onto a widget which accepts
     * map layers (local files, or uris originating from within QGIS such as the browser panel),
     * as opposed to an internal layer tree reordering. This is the kind of drag which classify()
     * applies to.
     */
    static bool isDatasetDrag( const QMimeData *mimeData );
};

#endif // QGSLAYERDROPCLASSIFIER_H
