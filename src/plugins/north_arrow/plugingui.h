/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef QGSNORTHARROWPLUGINGUI_H
#define QGSNORTHARROWPLUGINGUI_H

#include "ui_pluginguibase.h"
#include "qgscontexthelp.h"

/**
@author Tim Sutton
*/
class QgsNorthArrowPluginGui : public QDialog, private Ui::QgsNorthArrowPluginGuiBase
{
    Q_OBJECT

  public:
    QgsNorthArrowPluginGui( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~QgsNorthArrowPluginGui();

  private:
    void rotatePixmap( int theRotationInt );
    //    void paintEvent( QPaintEvent * );//overloads qwidget
    void resizeEvent( QResizeEvent * ); //overloads qwidget

  signals:
    //void drawRasterLayer(QString);
    //void drawVectorrLayer(QString,QString,QString);
    void rotationChanged( int );
    void changePlacement( int );
    // enable NorthArrow
    void enableNorthArrow( bool );
    void enableAutomatic( bool );
    void needToRefresh();

  public slots:
    void setRotation( int );
    void setPlacementLabels( QStringList& );
    void setPlacement( int );
    void setEnabled( bool );
    void setAutomatic( bool );
    void setAutomaticDisabled();

  private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
    void on_spinAngle_valueChanged( int theInt );
    void on_sliderRotation_valueChanged( int theInt );
};

#endif
