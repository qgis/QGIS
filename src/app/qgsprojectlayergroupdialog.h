/***************************************************************************
    qgsprojectlayergroupdialog.h
    ----------------------------
    begin                : June 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROJECTLAYERGROUPDIALOG_H
#define QGSPROJECTLAYERGROUPDIALOG_H

#include "QDialog"
#include "ui_qgsprojectlayergroupdialogbase.h"

class QDomElement;

/**A dialog to select layers and groups from a qgs project*/
class APP_EXPORT QgsProjectLayerGroupDialog: public QDialog, private Ui::QgsProjectLayerGroupDialogBase
{
    Q_OBJECT
  public:
    /**Constructor. If a project file is given, the groups/layers are displayed directly and the file selection hidden*/
    QgsProjectLayerGroupDialog( QWidget * parent = 0, const QString& projectFile = QString(), Qt::WindowFlags f = 0 );
    ~QgsProjectLayerGroupDialog();

    QStringList selectedGroups() const;
    QStringList selectedLayerIds() const;
    QStringList selectedLayerNames() const;
    QString selectedProjectFile() const;

  private slots:
    void on_mBrowseFileToolButton_clicked();
    void on_mProjectFileLineEdit_editingFinished();
    void on_mTreeWidget_itemSelectionChanged();
    void on_mButtonBox_accepted();

  private:
    void changeProjectFile();
    void addLegendGroupToTreeWidget( const QDomElement& groupElem, QTreeWidgetItem* parent = 0 );
    void addLegendLayerToTreeWidget( const QDomElement& layerElem, QTreeWidgetItem* parent = 0 );
    void unselectChildren( QTreeWidgetItem* item );
    QString mProjectPath;
    bool mShowEmbeddedContent;
};

#endif //QGSPROJECTLAYERGROUPDIALOG_H
