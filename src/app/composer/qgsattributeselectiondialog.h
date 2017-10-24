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
#include "qgsexpressioncontextgenerator.h"

class QGridLayout;
class QgsVectorLayer;
class QPushButton;
class QgsComposerAttributeTableV2;
class QgsComposerAttributeTableColumnModelV2;
class QgsComposerTableSortColumnsProxyModelV2;
class QgsComposerTableAvailableSortProxyModel;
class QgsComposerObject;

// QgsComposerColumnAlignmentDelegate

//! A delegate for showing column alignment as a combo box
class QgsComposerColumnAlignmentDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsComposerColumnAlignmentDelegate( QObject *parent = nullptr );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

};


// QgsComposerColumnAlignmentDelegate

//! A delegate for showing column attribute source as a QgsFieldExpressionWidget
class QgsComposerColumnSourceDelegate : public QItemDelegate, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    QgsComposerColumnSourceDelegate( QgsVectorLayer *vlayer, QObject *parent = nullptr, const QgsComposerObject *composerObject = nullptr );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
  public slots:
    void commitAndCloseEditor();
  private:
    QgsVectorLayer *mVectorLayer = nullptr;
    const QgsComposerObject *mComposerObject = nullptr;
    QgsExpressionContext createExpressionContext() const override;
};

// QgsComposerColumnWidthDelegate

//! A delegate for showing column width as a spin box
class QgsComposerColumnWidthDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsComposerColumnWidthDelegate( QObject *parent = nullptr );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

};


// QgsComposerColumnSortOrderDelegate

//! A delegate for showing column sort order as a combo box
class QgsComposerColumnSortOrderDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsComposerColumnSortOrderDelegate( QObject *parent = nullptr );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

};


// QgsAttributeSelectionDialog

//! A dialog to select what attributes to display (in the table item), set the column properties and specify a sort order
class QgsAttributeSelectionDialog: public QDialog, private Ui::QgsAttributeSelectionDialogBase
{
    Q_OBJECT
  public:
    QgsAttributeSelectionDialog( QgsComposerAttributeTableV2 *table, QgsVectorLayer *vLayer, QWidget *parent = nullptr, Qt::WindowFlags f = 0 );

    ~QgsAttributeSelectionDialog();

  private slots:
    void mRemoveColumnPushButton_clicked();
    void mAddColumnPushButton_clicked();
    void mColumnUpPushButton_clicked();
    void mColumnDownPushButton_clicked();
    void mResetColumnsPushButton_clicked();
    void mClearColumnsPushButton_clicked();
    void mAddSortColumnPushButton_clicked();
    void mRemoveSortColumnPushButton_clicked();
    void mSortColumnUpPushButton_clicked();
    void mSortColumnDownPushButton_clicked();

  private:
    QgsComposerAttributeTableV2 *mComposerTable = nullptr;

    const QgsVectorLayer *mVectorLayer = nullptr;

    QgsComposerAttributeTableColumnModelV2 *mColumnModel = nullptr;

    QgsComposerTableSortColumnsProxyModelV2 *mSortedProxyModel = nullptr;

    QgsComposerTableSortColumnsProxyModelV2 *mAvailableSortProxyModel = nullptr;

    QgsComposerColumnAlignmentDelegate *mColumnAlignmentDelegate = nullptr;
    QgsComposerColumnSourceDelegate *mColumnSourceDelegate = nullptr;
    QgsComposerColumnSortOrderDelegate *mColumnSortOrderDelegate = nullptr;
    QgsComposerColumnWidthDelegate *mColumnWidthDelegate = nullptr;

};

#endif // QGSATTRIBUTESELECTIONDIALOG_H
