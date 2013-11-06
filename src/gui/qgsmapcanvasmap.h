/***************************************************************************
    qgsmapcanvasmap.h  -  draws the map in map canvas
    ----------------------
    begin                : February 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPCANVASMAP_H
#define QGSMAPCANVASMAP_H

#include <QGraphicsRectItem>
#include <QPixmap>
#include <QTimer>

#include <qgis.h>
#include <qgsmaptopixel.h>

#include <qgsmaprendererjob.h>

class QgsMapRenderer;
class QgsMapSettings;
class QgsMapCanvas;

/** \ingroup gui
 * A rectangular graphics item representing the map on the canvas.
 */
class GUI_EXPORT QgsMapCanvasMap : public QObject, public QGraphicsRectItem
{
  Q_OBJECT
  public:

    //! constructor
    QgsMapCanvasMap( QgsMapCanvas* canvas );

    ~QgsMapCanvasMap();

    void refresh();

    //! resize canvas item and pixmap
    void resize( QSize size );

    //! @deprecated in 2.1 - does nothing. Kept for API compatibility
    void enableAntiAliasing( bool flag );

    //! @deprecated in 2.1 - does nothing. Kept for API compatibility
    void render() {}

    //! @deprecated in 2.1 - does nothing. Kept for API compatibility
    void setBackgroundColor( const QColor& color ) {}

    //! @deprecated in 2.1 - not called by QgsMapCanvas anymore
    Q_DECL_DEPRECATED void setPanningOffset( const QPoint& point ) {}

    //! @deprecated in 2.1
    QPaintDevice& paintDevice();

    void paint( QPainter* p, const QStyleOptionGraphicsItem*, QWidget* );

    QRectF boundingRect() const;

    //! @deprecated in 2.1 - does nothing. Kept for API compatibility
    void updateContents() {}

    const QgsMapSettings& settings() const;

    //! called by map canvas to handle panning with mouse (kind of)
    //! @note added in 2.1
    void mapDragged( const QPoint& diff);

public slots:
    void finish();
    void onMapUpdateTimeout();

  private:

    QImage mImage;

    QgsMapCanvas* mCanvas;

    QPoint mOffset;

    bool mDirty; //!< whether a new rendering job should be started upon next paint() call

    QgsMapRendererSequentialJob* mJob;

    QTimer mTimer;
};

#endif
