/***************************************************************************
    qgsmaptoolselect.h  -  map tool for selecting features
    ---------------------
    begin                : May 2010
    copyright            : (C) 2010 by Jeremy Palmer
    email                : jpalmer at linz dot govt dot nz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSELECT_H
#define QGSMAPTOOLSELECT_H

#include "qgsmaptool.h"
#include "qgsmaptoolselectionhandler.h"

#define SIP_NO_FILE

// no bindings for now, not stable yet. Previously lived in src/app

class QgsMapCanvas;
class QgsHighlight;

class QMouseEvent;

/**
 * \ingroup gui
 * \brief A map tool for selecting features on a map canvas.
 * \see QgsMapTool
 *
 * \since QGIS 4.2
 */
class GUI_EXPORT QgsMapToolSelect : public QgsMapTool
{
    Q_OBJECT
  public:
    //! Define selection modes used by the different combinations of modifiers.
    enum Mode
    {
      GeometryIntersectsSetSelection,
      GeometryIntersectsAddToSelection,
      GeometryIntersectsSubtractFromSelection,
      GeometryIntersectsIntersectWithSelection,
      GeometryWithinSetSelection,
      GeometryWithinAddToSelection,
      GeometryWithinSubtractFromSelection,
      GeometryWithinIntersectWithSelection,
    };

    QgsMapToolSelect( QgsMapCanvas *canvas );

    //! Sets the current selection mode
    void setSelectionMode( QgsMapToolSelectionHandler::SelectionMode selectionMode );

    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void keyPressEvent( QKeyEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;
    void deactivate() override;
    Flags flags() const override;

    bool populateContextMenuWithEvent( QMenu *menu, QgsMapMouseEvent *event ) override;

  signals:
    //! Emitted when the selection mode changes, usually when qt modifiers are changed
    void modeChanged( QgsMapToolSelect::Mode mode );

  private slots:
    void selectFeatures( Qt::KeyboardModifiers modifiers );

  private:
    std::unique_ptr<QgsMapToolSelectionHandler> mSelectionHandler;

    void modifiersChanged( bool ctrlModifier, bool shiftModifier, bool altModifier );
};

#endif
