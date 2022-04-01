/***************************************************************************
                          qgsplotcanvas.h
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPLOTCANVAS_H
#define QGSPLOTCANVAS_H

#include "qgsconfig.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

#include <QGraphicsView>

class QgsPlotMouseEvent;
class QgsPlotTool;
class QgsCoordinateReferenceSystem;
class QgsPoint;
class QgsPointXY;

class QMenu;


#ifdef SIP_RUN
% ModuleHeaderCode
#include "qgsplotcanvas.h"
#include "qgselevationprofilecanvas.h"
% End
#endif

/**
 * \ingroup gui
 * \brief Plot canvas is a class for displaying interactive 2d charts and plots.
 *
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsPlotCanvas : public QGraphicsView
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsElevationProfileCanvas *>( sipCpp ) != nullptr )
      sipType = sipType_QgsElevationProfileCanvas;
    else if ( qobject_cast<QgsPlotCanvas *>( sipCpp ) != nullptr )
      sipType = sipType_QgsPlotCanvas;
    else
      sipType = nullptr;
    SIP_END
#endif

    Q_OBJECT

  public:

    /**
     * Constructor for QgsPlotCanvas, with the specified \a parent widget.
     */
    QgsPlotCanvas( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsPlotCanvas() override;

    /**
     * Cancel any rendering job, in a blocking way. Used for application closing.
     * \note not available in Python bindings
     */
    virtual void cancelJobs() SIP_SKIP;

    /**
     * Sets the interactive tool currently being used on the canvas.
     */
    void setTool( QgsPlotTool *tool, bool clean = false );

    /**
     * Unset the current \a tool.
     *
     * This is called from destructor of plot tools to make sure
     * that this map tool won't be used any more.
     *
     * You don't have to call it manually, QgsPlotTool takes care of it.
     */
    void unsetTool( QgsPlotTool *tool );

    /**
     * Returns the currently active tool.
     */
    QgsPlotTool *tool();

    /**
     * Returns the coordinate reference system (CRS) for map coordinates used by the canvas.
     *
     * May return an invalid CRS if no CRS is associated with the canvas.
     */
    virtual QgsCoordinateReferenceSystem crs() const;

    /**
     * Converts a \a point on the canvas to the associated map coordinate.
     *
     * May return an empty point if the canvas point cannot be converted to a map point.
     */
    virtual QgsPoint toMapCoordinates( const QgsPointXY &point ) const;

    /**
     * Converts a \a point in map coordinates to the associated canvas point.
     *
     * May return an empty point if the map point cannot be converted to a canvas point.
     */
    virtual QgsPointXY toCanvasCoordinates( const QgsPoint &point ) const;

  public slots:

    /**
     * Updates and redraws the plot.
     */
    virtual void refresh();

  signals:

    /**
     * Emitted when the plot tool is changed.
     */
    void toolChanged( QgsPlotTool *newTool, QgsPlotTool *oldTool );

    /**
     * Emitted before the canvas context menu will be shown.
     * Can be used to extend the context menu.
     */
    void contextMenuAboutToShow( QMenu *menu, QgsPlotMouseEvent *event );

    //! Emitted when key press \a event occurs.
    void keyPressed( QKeyEvent *event );

    //! Emitted when a key release \a event occurs.
    void keyReleased( QKeyEvent *event );

  protected:

    bool event( QEvent *e ) override;
    void keyPressEvent( QKeyEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;
    void mouseDoubleClickEvent( QMouseEvent *e ) override;
    void mouseMoveEvent( QMouseEvent *e ) override;
    void mousePressEvent( QMouseEvent *e ) override;
    void mouseReleaseEvent( QMouseEvent *e ) override;
    void wheelEvent( QWheelEvent *e ) override;
    void resizeEvent( QResizeEvent *e ) override;
    bool viewportEvent( QEvent *event ) override;

  private slots:
    //! Called when current tool is destroyed
    void toolDestroyed();

  private:

    //! graphics scene manages plot items
    QGraphicsScene *mScene = nullptr;

    //! pointer to current plot tool
    QgsPlotTool *mTool = nullptr;

    bool mMouseButtonDown = false;

    void showContextMenu( QgsPlotMouseEvent *event );

};

#endif // QGSPLOTCANVAS_H
