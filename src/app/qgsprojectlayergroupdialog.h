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
#include "qgshelp.h"
#include "qgslayertreemodel.h"
#include "qgis_app.h"

class QDomElement;

class QgsLayerTree;

/**
 * Subclass of QgsLayerTreeModel which overrides font styling
 * from base model.
 */
class QgsEmbeddedLayerTreeModel : public QgsLayerTreeModel
{
    Q_OBJECT
  public:

    /**
     * Construct a new tree model with given layer tree (root node must not be NULLPTR).
     * The root node is not transferred by the model.
     */
    explicit QgsEmbeddedLayerTreeModel( QgsLayerTree *rootNode, QObject *parent = nullptr );

    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
};

//! A dialog to select layers and groups from a qgs project
class APP_EXPORT QgsProjectLayerGroupDialog: public QDialog, private Ui::QgsProjectLayerGroupDialogBase
{
    Q_OBJECT
  public:
    //! Constructor. If a project file is given, the groups/layers are displayed directly and the file selection hidden
    QgsProjectLayerGroupDialog( QWidget *parent = nullptr, const QString &projectFile = QString(), Qt::WindowFlags f = Qt::WindowFlags() );
    ~QgsProjectLayerGroupDialog() override;

    QStringList selectedGroups() const;
    QStringList selectedLayerIds() const;
    QStringList selectedLayerNames() const;
    QString selectedProjectFile() const;

    bool isValid() const;

  private slots:
    void onTreeViewSelectionChanged();
    void mButtonBox_accepted();
    void showHelp();

  private:
    void changeProjectFile();
    void removeEmbeddedNodes( QgsLayerTreeGroup *node );
    void deselectChildren( const QModelIndex &index );
    QString mProjectPath;
    bool mShowEmbeddedContent = false;
    bool mPresetProjectMode = false;

    QgsLayerTree *mRootGroup = nullptr;
};

#endif //QGSPROJECTLAYERGROUPDIALOG_H
