/***************************************************************************
                         qgsdxfexportdialog.h
                         --------------------
    begin                : September 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDXFEXPORTDIALOG_H
#define QGSDXFEXPORTDIALOG_H

#include "ui_qgsdxfexportdialogbase.h"
#include "qgsdxfexport.h"
#include "qgslayertreemodel.h"

#include <QList>
#include <QPair>

class QgsLayerTreeGroup;
class QgsLayerTreeNode;

#if 0
#include <QItemDelegate>
class FieldSelectorDelegate : public QItemDelegate
{
    Q_OBJECT
  public:
    FieldSelectorDelegate( QObject *parent = 0 );

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;
};
#endif

class QgsVectorLayerAndAttributeModel : public QgsLayerTreeModel
{
    Q_OBJECT
  public:
    QgsVectorLayerAndAttributeModel( QgsLayerTreeGroup* rootNode, QObject *parent = 0 );
    ~QgsVectorLayerAndAttributeModel();

    QModelIndex index( int row, int column, const QModelIndex &parent ) const;
    QModelIndex parent( const QModelIndex &child ) const;
    int rowCount( const QModelIndex &index ) const;
    Qt::ItemFlags flags( const QModelIndex &index ) const;
    QVariant data( const QModelIndex& index, int role ) const;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

    QList< QPair<QgsVectorLayer *, int> > layers( const QModelIndexList &selectedIndexes ) const;

  private:
    QHash<QgsVectorLayer *, int> mAttributeIdx;

#if 0
    friend FieldSelectorDelegate;
#endif
};


class QgsDxfExportDialog : public QDialog, private Ui::QgsDxfExportDialogBase
{
    Q_OBJECT
  public:
    QgsDxfExportDialog( QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsDxfExportDialog();

    QList< QPair<QgsVectorLayer *, int> > layers() const;

    double symbologyScale() const;
    QgsDxfExport::SymbologyExport symbologyMode() const;
    QString saveFile() const;
    bool exportMapExtent() const;

  public slots:
    /** change the selection of layers in the list */
    void selectAll();
    void unSelectAll();

    void on_mTreeView_clicked( const QModelIndex & current );
    void on_mLayerAttributeComboBox_fieldChanged( QString );

  private slots:
    void on_mFileSelectionButton_clicked();
    void setOkEnabled();
    void saveSettings();

  private:
    void cleanGroup( QgsLayerTreeNode *node );
    QgsLayerTreeGroup *mLayerTreeGroup;
#if 0
    FieldSelectorDelegate *mFieldSelectorDelegate;
#endif
};

#endif // QGSDXFEXPORTDIALOG_H
