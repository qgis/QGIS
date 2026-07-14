/***************************************************************************
  qgsdropfeedbackoverlay.h
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

#ifndef QGSDROPFEEDBACKOVERLAY_H
#define QGSDROPFEEDBACKOVERLAY_H

#include "qgis.h"
#include "qgis_gui.h"

#include <QWidget>

/**
 * \ingroup gui
 * \brief A translucent overlay widget which renders visual feedback while a drag
 * carrying map layers, a project or an unloadable payload hovers a widget which
 * accepts such drops (e.g. the layer tree view or the map canvas).
 *
 * The overlay is transparent for mouse events and does not accept drops itself, so
 * it never interferes with the drag and drop event delivery of the widget it covers.
 * It is meant to be laid over a target widget (matching its geometry), shown while a
 * drag hovers and hidden on drop or drag leave.
 *
 * Nothing is drawn for Qgis::LayerDropPayloadType::Layers or
 * Qgis::LayerDropPayloadType::CustomHandler payloads, for which the hosting widget
 * provides its own (or no) feedback.
 *
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsDropFeedbackOverlay : public QWidget
{
    Q_OBJECT

  public:
    //! Constructor for QgsDropFeedbackOverlay, with the specified \a parent widget.
    explicit QgsDropFeedbackOverlay( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the \a type of payload to render feedback for, triggering a repaint.
     * \see payloadType()
     */
    void setPayloadType( Qgis::LayerDropPayloadType type );

    /**
     * Returns the type of payload the overlay currently renders feedback for.
     * \see setPayloadType()
     */
    Qgis::LayerDropPayloadType payloadType() const { return mPayloadType; }

    /**
     * Renders the drop feedback for \a payloadType into \a rect using \a painter.
     *
     * Nothing is drawn for Qgis::LayerDropPayloadType::Layers or
     * Qgis::LayerDropPayloadType::CustomHandler payloads.
     *
     * The \a widget supplies the palette, font and device pixel ratio used for
     * themed, hidpi-aware rendering. This is exposed so that widgets which prefer to
     * paint the feedback directly (e.g. within their own paintEvent()) can reuse the
     * exact same rendering without hosting an overlay widget.
     */
    static void paintFeedback( QPainter *painter, const QRect &rect, Qgis::LayerDropPayloadType payloadType, const QWidget *widget );

  protected:
    void paintEvent( QPaintEvent *event ) override;

  private:
    Qgis::LayerDropPayloadType mPayloadType = Qgis::LayerDropPayloadType::Layers;
};

#endif // QGSDROPFEEDBACKOVERLAY_H
