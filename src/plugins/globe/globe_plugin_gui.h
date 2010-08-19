/***************************************************************************
    globe_plugin_gui.h

    Globe Plugin
    a QGIS plugin
     --------------------------------------
    Date                 : 08-Jul-2010
    Copyright            : (C) 2010 by Sourcepole
    Email                : info at sourcepole.ch
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_GLOBE_PLUGIN_GUI_H
#define QGS_GLOBE_PLUGIN_GUI_H

#include <QDialog>
#include <ui_globe_plugin_guibase.h>

class QgsGlobePluginGui : public QDialog, private Ui::QgsGlobePluginGuiBase
{
  Q_OBJECT

  public:
    QgsGlobePluginGui( QWidget* parent = 0, Qt::WFlags fl = 0 );
    virtual ~QgsGlobePluginGui();

  private:
    static const int context_id = 0;

  private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_buttonBox_helpRequested();
};

#endif // QGS_GLOBE_PLUGIN_GUI_H
