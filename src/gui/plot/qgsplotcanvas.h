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
#include <QPointer>

class QgsPlotMouseEvent;
class QgsPlotTool;
class QgsCoordinateReferenceSystem;
class QgsPoint;
class QgsPointXY;
class QgsPlotToolTemporaryKeyPan;
class QgsPlotToolTemporaryMousePan;
class QgsPlotToolTemporaryKeyZoom;

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
    void setTool( QgsPlotTool *tool );

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

    /**
     * Pans the plot contents by \a dx, \a dy in canvas units.
     *
     * The default implementation does nothing.
     */
    virtual void panContentsBy( double dx, double dy );

    /**
     * Centers the plot on the plot point corresponding to \a x, \a y in canvas units.
     *
     * The default implementation does nothing.
     */
    virtual void centerPlotOn( double x, double y );

    /**
     * Scales the plot by a specified \a scale factor.
     *
     * The default implementation does nothing.
     */
    virtual void scalePlot( double factor );

    /**
     * Zooms the plot to the specified \a rect in canvas units.
     *
     * The default implementation does nothing.
     */
    virtual void zoomToRect( const QRectF &rect );

    /**
     * Snap a canvas point to the plot
     *
     * Returns an empty point if snapping was not possible.
     *
     * \param point point in canvas coordinates
     */
    virtual QgsPointXY snapToPlot( QPoint point );

  public slots:

    /**
     * Updates and redraws the plot.
     */
    virtual void refresh();

  signals:

    /**
     * Emitted when the plot tool is changed.
     */
    void toolChanged( QgsPlotTool *newTool );

    /**
     * Emitted before the canvas context menu will be shown.
     * Can be used to extend the context menu.
     */
    void contextMenuAboutToShow( QMenu *menu, QgsPlotMouseEvent *event );

    /**
     * Emitted in the destructor when the canvas is about to be deleted,
     * but is still in a perfectly valid state.
     */
    void willBeDeleted();

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

    /**
     * Zoom plot from a mouse wheel \a event.
     *
     * The default implementation does nothing.
     */
    virtual void wheelZoom( QWheelEvent *event );

  private:

    //! graphics scene manages plot items
    QGraphicsScene *mScene = nullptr;

    //! pointer to current plot tool
    QPointer< QgsPlotTool > mTool;

    QgsPlotToolTemporaryKeyPan *mSpacePanTool = nullptr;
    QgsPlotToolTemporaryMousePan *mMidMouseButtonPanTool = nullptr;
    QgsPlotToolTemporaryKeyZoom *mSpaceZoomTool = nullptr;

    void showContextMenu( QgsPlotMouseEvent *event );

};

#endif // QGSPLOTCANVAS_H
