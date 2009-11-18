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
#ifndef QGSSCALEBARPLUGINGUI_H
#define QGSSCALEBARPLUGINGUI_H

#include <ui_pluginguibase.h>
#include <QDialog>
#include "qgscontexthelp.h"

/**
@author Peter Brewer
*/
class QgsScaleBarPluginGui : public QDialog, private Ui::QgsScaleBarPluginGuiBase
{
    Q_OBJECT

  public:
    QgsScaleBarPluginGui( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~QgsScaleBarPluginGui();
    void setPlacementLabels( QStringList& );
    void setPlacement( int );
    void setPreferredSize( int );
    void setSnapping( bool );
    void setEnabled( bool );
    void setStyleLabels( QStringList& );
    void setStyle( int );
    void setColour( QColor );

    //accessor for getting a pointer to the size spin widget
    QSpinBox * getSpinSize();

  private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
    void on_pbnChangeColour_clicked();

  signals:
    void drawRasterLayer( QString );
    void drawVectorrLayer( QString, QString, QString );
    void changePlacement( int );
    void changePreferredSize( int );
    void changeSnapping( bool );
    void changeEnabled( bool );
    void changeStyle( int );
    void changeColour( QColor );
    void refreshCanvas();
};

#endif
