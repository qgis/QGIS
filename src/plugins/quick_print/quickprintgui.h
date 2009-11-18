/***************************************************************************
  Quick Print is a plugin to quickly print a map with minimal effort.
  -------------------
         begin                : Jan 2008
         copyright            : (c) Tim Sutton, 2008
         email                : tim@linfiniti.com

 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef QuickPrintGUI_H
#define QuickPrintGUI_H

#include <QDialog>
#include <ui_quickprintguibase.h>
#include "qgsmapcanvas.h"
#include "qgscontexthelp.h"

/**
@author Tim Sutton
*/
class QuickPrintGui : public QDialog, private Ui::QuickPrintGuiBase
{
    Q_OBJECT

  public:
    QuickPrintGui( QgsMapCanvas * thepMapCanvas, QWidget* parent = 0,  Qt::WFlags fl = 0 );
    ~QuickPrintGui();
  private:
    void readSettings();
    void writeSettings();
    QgsMapCanvas * mpMapCanvas;
  private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

};

#endif
