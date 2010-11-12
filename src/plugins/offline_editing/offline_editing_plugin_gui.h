/***************************************************************************
    offline_editing_plugin_gui.h

    Offline Editing Plugin
    a QGIS plugin
     --------------------------------------
    Date                 : 08-Jul-2010
    Copyright            : (C) 2010 by Sourcepole
    Email                : info at sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_OFFLINE_EDITING_PLUGIN_GUI_H
#define QGS_OFFLINE_EDITING_PLUGIN_GUI_H

#include <QDialog>
#include <ui_offline_editing_plugin_guibase.h>

class QgsOfflineEditingPluginGui : public QDialog, private Ui::QgsOfflineEditingPluginGuiBase
{
    Q_OBJECT

  public:
    QgsOfflineEditingPluginGui( QWidget* parent = 0, Qt::WFlags fl = 0 );
    virtual ~QgsOfflineEditingPluginGui();

    QString offlineDataPath();
    QString offlineDbFile();
    QStringList& selectedLayerIds();

  private:
    void updateLayerList( bool filterEditableLayers );

    static const int context_id = 0;
    QString mOfflineDataPath;
    QString mOfflineDbFile;
    QStringList mSelectedLayerIds;

  private slots:
    void on_butBrowse_clicked();
    void on_checkboxShowEditableLayers_stateChanged( int state );
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_buttonBox_helpRequested();
};

#endif // QGS_OFFLINE_EDITING_PLUGIN_GUI_H
