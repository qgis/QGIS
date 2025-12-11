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

#include "ui_offline_editing_plugin_guibase.h"

#include "qgslayertreemodel.h"
#include "qgsofflineediting.h"

#include <QDialog>

class QgsSelectLayerTreeModel : public QgsLayerTreeModel
{
    Q_OBJECT
  public:
    QgsSelectLayerTreeModel( QgsLayerTree *rootNode, QObject *parent = nullptr );
    [[nodiscard]] int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    [[nodiscard]] QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
};

class QgsOfflineEditingPluginGui : public QDialog, private Ui::QgsOfflineEditingPluginGuiBase
{
    Q_OBJECT

  public:
    QgsOfflineEditingPluginGui( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::WindowFlags() );
    ~QgsOfflineEditingPluginGui() override;

    [[nodiscard]] QString offlineDataPath() const;
    [[nodiscard]] QString offlineDbFile() const;
    [[nodiscard]] QStringList selectedLayerIds() const;
    [[nodiscard]] bool onlySelected() const;
    [[nodiscard]] QgsOfflineEditing::ContainerType dbContainerType() const;

  public slots:
    //! Change the selection of layers in the list
    void selectAll();
    void deSelectAll();
    void datatypeChanged( int index );

  private:
    void saveState();
    void restoreState();

    QString mOfflineDataPath;
    QString mOfflineDbFile;
    QStringList mSelectedLayerIds;

  private slots:
    void mBrowseButton_clicked();
    void buttonBox_accepted();
    void buttonBox_rejected();
    void showHelp();
};

#endif // QGS_OFFLINE_EDITING_PLUGIN_GUI_H
