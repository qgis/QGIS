/***************************************************************************
                         qgsattributeselectiondialog.h
                         -----------------------------
    begin                : January 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTESELECTIONDIALOG_H
#define QGSATTRIBUTESELECTIONDIALOG_H

#include <QDialog>
#include <QMap>
#include <QSet>
#include <QItemDelegate>
#include "ui_qgsattributeselectiondialogbase.h"

class QGridLayout;
class QgsVectorLayer;
class QPushButton;
class QgsComposerAttributeTable;
class QgsComposerAttributeTableV2;
class QgsComposerAttributeTableColumnModel;
class QgsComposerAttributeTableColumnModelV2;
class QgsComposerTableSortColumnsProxyModel;
class QgsComposerTableSortColumnsProxyModelV2;
class QgsComposerTableAvailableSortProxyModelV2;

// QgsComposerColumnAlignmentDelegate

/**A delegate for showing column alignment as a combo box*/
class QgsComposerColumnAlignmentDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    QgsComposerColumnAlignmentDelegate( QObject *parent = 0 );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const;

};


// QgsComposerColumnAlignmentDelegate

/**A delegate for showing column attribute source as a QgsFieldExpressionWidget*/
class QgsComposerColumnSourceDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    QgsComposerColumnSourceDelegate( QgsVectorLayer* vlayer, QObject *parent = 0 );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
  public slots:
    void commitAndCloseEditor();
  private:
    QgsVectorLayer* mVectorLayer;
};

// QgsComposerColumnWidthDelegate

/**A delegate for showing column width as a spin box*/
class QgsComposerColumnWidthDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    QgsComposerColumnWidthDelegate( QObject *parent = 0 );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const;

};


// QgsComposerColumnSortOrderDelegate

/**A delegate for showing column sort order as a combo box*/
class QgsComposerColumnSortOrderDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    QgsComposerColumnSortOrderDelegate( QObject *parent = 0 );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const;

};


// QgsAttributeSelectionDialog

/**A dialog to select what attributes to display (in the table item), set the column properties and specify a sort order*/
class QgsAttributeSelectionDialog: public QDialog, private Ui::QgsAttributeSelectionDialogBase
{
    Q_OBJECT
  public:
    QgsAttributeSelectionDialog( QgsComposerAttributeTableV2* table, QgsVectorLayer* vLayer, QWidget * parent = 0, Qt::WindowFlags f = 0 );

    //todo - remove for QGIS 3.0
    QgsAttributeSelectionDialog( QgsComposerAttributeTable* table, QgsVectorLayer* vLayer, QWidget * parent = 0, Qt::WindowFlags f = 0 );


    ~QgsAttributeSelectionDialog();

  private slots:
    void on_mRemoveColumnPushButton_clicked();
    void on_mAddColumnPushButton_clicked();
    void on_mColumnUpPushButton_clicked();
    void on_mColumnDownPushButton_clicked();
    void on_mResetColumnsPushButton_clicked();
    void on_mAddSortColumnPushButton_clicked();
    void on_mRemoveSortColumnPushButton_clicked();
    void on_mSortColumnUpPushButton_clicked();
    void on_mSortColumnDownPushButton_clicked();

  private:
    QgsComposerAttributeTableV2* mComposerTable;
    QgsComposerAttributeTable* mComposerTableV1;

    const QgsVectorLayer* mVectorLayer;

    QgsComposerAttributeTableColumnModelV2* mColumnModel;
    QgsComposerAttributeTableColumnModel* mColumnModelV1;

    QgsComposerTableSortColumnsProxyModelV2* mSortedProxyModel;
    QgsComposerTableSortColumnsProxyModel* mSortedProxyModelV1;

    QgsComposerTableSortColumnsProxyModelV2* mAvailableSortProxyModel;
    QgsComposerTableSortColumnsProxyModel* mAvailableSortProxyModelV1;

    QgsComposerColumnAlignmentDelegate *mColumnAlignmentDelegate;
    QgsComposerColumnSourceDelegate *mColumnSourceDelegate;
    QgsComposerColumnSortOrderDelegate *mColumnSortOrderDelegate;
    QgsComposerColumnWidthDelegate *mColumnWidthDelegate;

};

#endif // QGSATTRIBUTESELECTIONDIALOG_H
