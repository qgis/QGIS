/***************************************************************************
                           qgsmapoverviewcanvas.h
                      Map canvas subclassed for overview
                              -------------------
    begin                : 09/14/2005
    copyright            : (C) 2005 by Martin Dobias
    email                : won.der at centrum.sk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPOVERVIEWCANVAS_H
#define QGSMAPOVERVIEWCANVAS_H


#include <QMouseEvent>
#include <QWheelEvent>
#include <QWidget>
#include <QStringList>
#include <QPixmap>

class QgsMapCanvas;
class QgsPanningWidget; // defined in .cpp
class QgsRectangle;

class QgsMapRendererQImageJob;
#include "qgsmapsettings.h"

/** \ingroup gui
 * A widget that displays an overview map.
 */
class GUI_EXPORT QgsMapOverviewCanvas : public QWidget
{
    Q_OBJECT

  public:
    QgsMapOverviewCanvas( QWidget * parent = 0, QgsMapCanvas* mapCanvas = NULL );

    ~QgsMapOverviewCanvas();

    //! renders overview and updates panning widget
    void refresh();

    //! changes background color
    void setBackgroundColor( const QColor& color );

    //! updates layer set for overview
    void setLayerSet( const QStringList& layerSet );

    QStringList layerSet() const;

    void enableAntiAliasing( bool flag ) { mSettings.setFlag( QgsMapSettings::Antialiasing, flag ); }

    void updateFullExtent();

  public slots:

    // ### QGIS 3: make protected
    //! used for overview canvas to reflect changed extent in main map canvas
    void drawExtentRect();

    // ### QGIS 3: rename so it does not look like getter, make protected
    void hasCrsTransformEnabled( bool flag );

    // ### QGIS 3: rename Srs to Crs, make protected
    void destinationSrsChanged();

  protected slots:
    void mapRenderingFinished();
    void layerRepaintRequested();

  protected:

    //! Overridden paint event
    void paintEvent( QPaintEvent * pe );

    //! Overridden resize event
    void resizeEvent( QResizeEvent * e );

    //! Overridden mouse move event
    void mouseMoveEvent( QMouseEvent * e );

    //! Overridden mouse press event
    void mousePressEvent( QMouseEvent * e );

    //! Overridden mouse release event
    void mouseReleaseEvent( QMouseEvent * e );

    //! called when panning to reflect mouse movement
    void updatePanningWidget( const QPoint& pos );

    //! widget for panning map in overview
    QgsPanningWidget* mPanningWidget;

    //! position of cursor inside panning widget
    QPoint mPanningCursorOffset;

    //! main map canvas - used to get/set extent
    QgsMapCanvas* mMapCanvas;

    //! pixmap where the map is stored
    QPixmap mPixmap;

    //! map settings used for rendering of the overview map
    QgsMapSettings mSettings;

    //! for rendering overview
    QgsMapRendererQImageJob* mJob;
};

#endif
