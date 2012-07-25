/***************************************************************************
    qgstilescalewidget.cpp  - slider to choose wms-c resolutions
                             -------------------
    begin    : 28 Mar 2010
    copyright: (C) 2010 Juergen E. Fischer < jef at norbit dot de >

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILESCALEWIDGET_H
#define QGSTILESCALEWIDGET_H

#include "ui_qgstilescalewidgetbase.h"

class QgsMapCanvas;
class QgsMapLayer;
class QMainWindow;

class QgsTileScaleWidget : public QWidget, private Ui::QgsTileScaleWidget
{
    Q_OBJECT

  public:
    static void showTileScale( QMainWindow *mainWindow );

  public slots:
    void layerChanged( QgsMapLayer *layer );
    void scaleChanged( double );
    void on_mSlider_valueChanged( int );
    void scaleEnabled( bool );

  private:
    QgsTileScaleWidget( QgsMapCanvas *mapCanvas, QWidget *parent = 0, Qt::WindowFlags f = 0 );
    ~QgsTileScaleWidget();

    QgsMapCanvas *mMapCanvas;
    QList<double> mResolutions;
};

#endif // QGSTILESCALEWIDGET
