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

    void enableAntiAliasing( bool flag ) { mAntiAliasing = flag; }

    //! @deprecated in 2.1 - does nothing. Kept for API compatibility
    void render() {}

    void setBackgroundColor( const QColor& color ) { mBgColor = color; }

    void setPanningOffset( const QPoint& point );

    //! @deprecated in 2.1
    QPaintDevice& paintDevice();

    void paint( QPainter* p, const QStyleOptionGraphicsItem*, QWidget* );

    QRectF boundingRect() const;

    //! @deprecated in 2.1 - does nothing. Kept for API compatibility
    void updateContents() {}

    const QgsMapSettings& settings() const;

public slots:
    void finish();
    void onMapUpdateTimeout();

  private:

    //! indicates whether antialiasing will be used for rendering
    bool mAntiAliasing;

    QImage mImage;
    QImage mLastImage;

    //QgsMapRenderer* mRender;
    QgsMapCanvas* mCanvas;

    QColor mBgColor;

    QPoint mOffset;

    bool mDirty; //!< whether a new rendering job should be started upon next paint() call

    QgsMapRendererCustomPainterJob* mJob;

    QPainter* mPainter;

    QTimer mTimer;
};

#endif
