/***************************************************************************
    qgsmaptoolemitpoint.h  -  map tool that emits a signal on click
    ---------------------
    begin                : June 2007
    copyright            : (C) 2007 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLEMITPOINT_H
#define QGSMAPTOOLEMITPOINT_H

#include "qgsmaptool.h"
#include "qgis_gui.h"
class QgsMapCanvas;


/**
 * \ingroup gui
 * A map tool that simply emits a point when clicking on the map.
 * Connecting a slot to its canvasClicked() signal will
 * let you implement custom behavior for the passed in point.
 */
class GUI_EXPORT QgsMapToolEmitPoint : public QgsMapTool
{
    Q_OBJECT

  public:
    //! constructor
    QgsMapToolEmitPoint( QgsMapCanvas *canvas );

    Flags flags() const override { return QgsMapTool::AllowZoomRect; }
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;

  signals:
    //! signal emitted on canvas click
    void canvasClicked( const QgsPointXY &point, Qt::MouseButton button );
};

#endif
