/***************************************************************************
  QgsAttributeTableDialog.h - dialog for attribute table
  -------------------
         date                 : Feb 2009
         copyright            : Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTETABLEDIALOG_H_
#define QGSATTRIBUTETABLEDIALOG_H_

#include <QDialog>
#include <QModelIndex>
#include <QItemSelectionModel>
#include <QMutex>

#include "ui_qgsattributetabledialog.h"

class QgsMapLayer;
class QgsVectorLayer;

#include "qgsvectorlayer.h" //QgsFeatureIds

class QDialogButtonBox;
class QPushButton;
class QLineEdit;
class QComboBox;
class QMenu;
class QDockWidget;

class QgsAttributeTableModel;
class QgsAttributeTableFilterModel;
class QgsAttributeTableView;

class QgsAttributeTableDialog : public QDialog, private Ui::QgsAttributeTableDialog
{
    Q_OBJECT

  public:
    QgsAttributeTableDialog( QgsVectorLayer *theLayer, QWidget *parent = 0, Qt::WindowFlags flags = Qt::Window );
    ~QgsAttributeTableDialog();

  public slots:
    void editingToggled();

  private slots:
    void submit();
    void revert();
    void search();
    void on_mAdvancedSearchButton_clicked();
    void updateSelection();
    void updateSelectionFromLayer();
    void updateRowSelection( int index );
    void updateRowSelection( int first, int last, bool startNewSelection );

    void on_cbxShowSelectedOnly_toggled( bool theFlag );
    void on_mCopySelectedRowsButton_clicked();

    void on_mToggleEditingButton_toggled();
    void on_mInvertSelectionButton_clicked();
    void on_mRemoveSelectionButton_clicked();
    void on_mZoomMapToSelectedRowsButton_clicked();
    void on_mSelectedToTopButton_clicked();
    void showAdvanced();
    void startEditing();

  signals:
    void editingToggled( QgsMapLayer * );

  protected:
    void closeEvent( QCloseEvent* event );

  private:
    void columnBoxInit();
    int columnBoxColumnId();
    void doSearch( QString searchString );

    QIcon getThemeIcon( const QString theName );

    QLineEdit *mQuery;
    QComboBox *mColumnBox;
    QComboBox *mShowBox;

    QMenu* mMenuActions;
    QAction* mActionToggleEditing;

    QgsAttributeTableModel *mModel;
    QgsAttributeTableFilterModel *mFilterModel;
    QgsVectorLayer *mLayer;
    QgsFeatureIds mSelectedFeatures;

    QItemSelectionModel* mSelectionModel;
    int mLastClickedHeaderIndex;

    QDockWidget *mDock;
};

#endif
