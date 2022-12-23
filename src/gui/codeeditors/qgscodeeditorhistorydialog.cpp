/***************************************************************************
    qgscodeeditorhistorydialog.cpp
    ----------------------
    begin                : October 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscodeeditorhistorydialog.h"
#include "qgscodeeditor.h"
#include <QStandardItemModel>
#include <QShortcut>

QgsCodeEditorHistoryDialog::QgsCodeEditorHistoryDialog( QgsCodeEditor *editor, QWidget *parent )
  : QDialog( parent )
  , mEditor( editor )
{
  setupUi( this );

  if ( mEditor )
  {
    setWindowTitle( tr( "%1 Console - Command History" ).arg( QgsCodeEditor::languageToString( mEditor->language() ) ) );
  }

  listView->setToolTip( tr( "Double-click on item to execute" ) );

  mModel = new CodeHistoryModel( listView );
  listView->setModel( mModel );

  reloadHistory();

  QShortcut *deleteShortcut = new QShortcut( QKeySequence( Qt::Key_Delete ), this );
  connect( deleteShortcut, &QShortcut::activated, this, &QgsCodeEditorHistoryDialog::deleteItem );
  connect( listView, &QListView::doubleClicked, this, &QgsCodeEditorHistoryDialog::runCommand );
  connect( mButtonReloadHistory, &QPushButton::clicked, this, & QgsCodeEditorHistoryDialog::reloadHistory );
  connect( mButtonSaveHistory, &QPushButton::clicked, this, & QgsCodeEditorHistoryDialog::saveHistory );
  connect( mButtonRunHistory, &QPushButton::clicked, this, &QgsCodeEditorHistoryDialog::executeSelectedHistory );
}

void QgsCodeEditorHistoryDialog::executeSelectedHistory()
{
  if ( !mEditor )
    return;

  QModelIndexList selection = listView->selectionModel()->selectedIndexes();
  std::sort( selection.begin(), selection.end() );
  for ( const QModelIndex &index : std::as_const( selection ) )
  {
    mEditor->runCommand( index.data( Qt::DisplayRole ).toString() );
  }
}

void QgsCodeEditorHistoryDialog::runCommand( const QModelIndex &index )
{
  if ( !mEditor )
    return;

  mEditor->runCommand( index.data( Qt::DisplayRole ).toString() );
}

void QgsCodeEditorHistoryDialog::saveHistory()
{
  if ( !mEditor )
    return;

  mEditor->writeHistoryFile();
}

void QgsCodeEditorHistoryDialog::reloadHistory()
{
  if ( mEditor )
  {
    mModel->setStringList( mEditor->history() );
  }

  listView->scrollToBottom();
  listView->setCurrentIndex( mModel->index( mModel->rowCount() - 1, 0 ) );
}

void QgsCodeEditorHistoryDialog::deleteItem()
{
  const QModelIndexList selection = listView->selectionModel()->selectedRows();
  if ( selection.empty() )
    return;

  QList< int > selectedRows;
  selectedRows.reserve( selection.size() );
  for ( const QModelIndex &index : selection )
    selectedRows << index.row();
  std::sort( selectedRows.begin(), selectedRows.end(), std::greater< int >() );

  for ( int row : std::as_const( selectedRows ) )
  {
    if ( mEditor )
      mEditor->removeHistoryCommand( row );

    // Remove row from the command history dialog
    mModel->removeRow( row );
  }
}

///@cond PRIVATE
CodeHistoryModel::CodeHistoryModel( QObject *parent )
  : QStringListModel( parent )
{
  mFont = QgsCodeEditor::getMonospaceFont();
}

QVariant CodeHistoryModel::data( const QModelIndex &index, int role ) const
{
  if ( role == Qt::FontRole )
  {
    return mFont;
  }

  return QStringListModel::data( index, role );
}
///@endcond
