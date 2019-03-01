/***************************************************************************
    qgsmapcanvastracer.h
    ---------------------
    begin                : January 2016
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
#ifndef QGSMAPCANVASTRACER_H
#define QGSMAPCANVASTRACER_H

#include "qgstracer.h"
#include "qgis_gui.h"

class QAction;
class QgsMapCanvas;
class QgsMessageBar;
class QgsMessageBarItem;

/**
 * \ingroup gui
 * Extension of QgsTracer that provides extra functionality:
 *  - automatic updates of own configuration based on canvas settings
 *  - reporting of issues to the user via message bar
 *  - determines whether tracing is currently enabled by the user
 *
 * A simple registry of tracer instances associated to map canvas instances
 * is kept for convenience. (Map tools do not need to create their local
 * tracer instances and map canvas API is not "polluted" by this optional
 * functionality).
 *
 * \since QGIS 2.14
 */
class GUI_EXPORT QgsMapCanvasTracer : public QgsTracer
{
    Q_OBJECT

  public:
    //! Create tracer associated with a particular map canvas, optionally message bar for reporting
    explicit QgsMapCanvasTracer( QgsMapCanvas *canvas, QgsMessageBar *messageBar = nullptr );
    ~QgsMapCanvasTracer() override;

    //! Access to action that user may use to toggle tracing on/off. May be null if no action was associated
    QAction *actionEnableTracing() const { return mActionEnableTracing; }

    /**
     * Assign "enable tracing" checkable action to the tracer.
     * The action is used to determine whether tracing is currently enabled by the user
     */
    void setActionEnableTracing( QAction *action ) { mActionEnableTracing = action; }

    /**
     * Access to action that user may use to toggle snapping on/off. May be NULLPTR if no action was associated.
     * \since QGIS 3.0
     */
    QAction *actionEnableSnapping() const { return mActionEnableSnapping; }

    /**
     * Assign "enable snapping" checkable action to the tracer.
     * The action is used to determine whether snapping is currently enabled by the user.
     * \since QGIS 3.0
     */
    void setActionEnableSnapping( QAction *action ) { mActionEnableSnapping = action; }

    /**
     * Retrieve instance of this class associated with given canvas (if any).
     * The class keeps a simple registry of tracers associated with map canvas
     * instances for easier access to the common tracer by various map tools
     */
    static QgsMapCanvasTracer *tracerForCanvas( QgsMapCanvas *canvas );

    //! Report a path finding error to the user
    void reportError( PathError err, bool addingVertex );

  protected:
    //! Sets configuration from current snapping settings and canvas settings
    void configure() override;

  private slots:
    void onCurrentLayerChanged();

  private:
    QgsMapCanvas *mCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
    QgsMessageBarItem *mLastMessage = nullptr;

    QAction *mActionEnableTracing = nullptr;
    QAction *mActionEnableSnapping = nullptr;

    static QHash<QgsMapCanvas *, QgsMapCanvasTracer *> sTracers;
};

#endif // QGSMAPCANVASTRACER_H
