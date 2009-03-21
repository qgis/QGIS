/***************************************************************************
  BeataDialog.h 
  BEtter Attribute TAble
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

#ifndef BEATADIALOG_H_
#define BEATADIALOG_H_

#include <QDialog>
#include <QModelIndex>
#include <QItemSelectionModel>
#include <QMutex>

#include "ui_BeataGui.h"

class QgsMapLayer;
class QgsVectorLayer;

#include "qgsvectorlayer.h" //QgsFeatureIds

class QDialogButtonBox;
class QPushButton;
class QLineEdit;
class QComboBox;
class QMenu;
class QDockWidget;

class BeataModel;
class BeataFilterModel;
class BeataView;

class BeataDialog : public QDialog, private Ui::BeataDialogGui
{
Q_OBJECT

public:
  BeataDialog(QgsVectorLayer *theLayer, QWidget *parent = 0, Qt::WindowFlags flags = Qt::Window);
  ~BeataDialog();

public slots:
  void editingToggled();
  
private slots:
  void submit();
  void revert();
  void search();
  void advancedSearch();
  void updateSelection();
  void updateSelectionFromLayer();
  void updateRowSelection(int index);
  void updateRowSelection(int first, int last, bool startNewSelection);

  void clickedShowAll();
  void clickedShowSelected();
  
  void startEditing();
  void invertSelection();
  void removeSelection();
  void copySelectedRowsToClipboard();
  void zoomMapToSelectedRows();
  void selectedToTop();
  void showAdvanced();
  void toggleEditing();

signals:
  void editingToggled( QgsMapLayer * );
  
protected:
  void closeEvent( QCloseEvent* event );
  
private:
  void columnBoxInit();
  int columnBoxColumnId();
  void doSearch(QString searchString);

  QIcon getThemeIcon( const QString theName );
  
  QLineEdit *mQuery;
  QComboBox *mColumnBox; 
  QComboBox *mShowBox; 
  
  QMenu* mMenuActions;
  QAction* mActionToggleEditing;

  BeataModel *mModel;
  BeataFilterModel *mFilterModel;
  QgsVectorLayer *mLayer;
  QgsFeatureIds mSelectedFeatures;

  QItemSelectionModel* mSelectionModel;
  int mLastClickedHeaderIndex;
  
  QDockWidget *mDock;
};

#endif
