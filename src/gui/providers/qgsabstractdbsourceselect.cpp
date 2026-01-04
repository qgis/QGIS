/***************************************************************************
   qgsabstractdbsourceselect.h
    --------------------------------------
   Date                 : 08.11.2021
   Copyright            : (C) 2021 Denis Rouzaud
   Email                : denis@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsabstractdbsourceselect.h"

#include "qgsabstractdbtablemodel.h"
#include "qgssettings.h"
#include "qgssettingstree.h"

#include <QActionGroup>
#include <QItemDelegate>
#include <QMenu>
#include <QSortFilterProxyModel>

#include "moc_qgsabstractdbsourceselect.cpp"

const QgsSettingsEntryBool *QgsAbstractDbSourceSelect::settingSearchColumnAll = new QgsSettingsEntryBool( u"%1/searchColumnAll"_s, QgsSettingsTree::sTreeWindowState );
const QgsSettingsEntryInteger *QgsAbstractDbSourceSelect::settingSearchColumn = new QgsSettingsEntryInteger( u"%1/searchColumn"_s, QgsSettingsTree::sTreeWindowState, -1 );
const QgsSettingsEntryBool *QgsAbstractDbSourceSelect::settingSearchRegex = new QgsSettingsEntryBool( u"%1/searchRegex"_s, QgsSettingsTree::sTreeWindowState );
const QgsSettingsEntryBool *QgsAbstractDbSourceSelect::settingHoldDialogOpen = new QgsSettingsEntryBool( u"%1/holdDialogOpen"_s, QgsSettingsTree::sTreeWindowState );
const QgsSettingsEntryInteger *QgsAbstractDbSourceSelect::settingColumnWidths = new QgsSettingsEntryInteger( u"%1/columnWidths/%2"_s, QgsSettingsTree::sTreeWindowState );

QgsAbstractDbSourceSelect::QgsAbstractDbSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );

  mProxyModel = new QSortFilterProxyModel( this );
  mProxyModel->setParent( this );
  mProxyModel->setFilterKeyColumn( -1 );
  mProxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );
  mProxyModel->setRecursiveFilteringEnabled( true );

  // Do not do dynamic sorting - otherwise whenever user selects geometry type / srid / pk columns,
  // that item suddenly jumps to the end of the list (because the item gets changed) which is very annoying.
  // The list gets sorted in finishList() method when the listing of tables and views has finished.
  mProxyModel->setDynamicSortFilter( false );

  mTablesTreeView->setSortingEnabled( true );
  mTablesTreeView->setUniformRowHeights( true );
  mTablesTreeView->setEditTriggers( QAbstractItemView::CurrentChanged );

  mBuildQueryButton = new QPushButton( tr( "&Set Filter" ) );
  mBuildQueryButton->setToolTip( tr( "Set Filter" ) );
  mBuildQueryButton->setDisabled( true );
  buttonBox->addButton( mBuildQueryButton, QDialogButtonBox::ActionRole );

  connect( mTablesTreeView, &QTreeView::clicked, this, [this]( const QModelIndex &index ) {
    treeviewClicked( mProxyModel->mapToSource( index ) );
  } );
  connect( mTablesTreeView, &QTreeView::doubleClicked, this, [this]( const QModelIndex &index ) {
    treeviewDoubleClicked( mProxyModel->mapToSource( index ) );
  } );

  connect( mBuildQueryButton, &QAbstractButton::clicked, this, [this]() { setSql( mProxyModel->mapToSource( mTablesTreeView->currentIndex() ) ); } );
}

void QgsAbstractDbSourceSelect::init( QgsAbstractDbTableModel *model, QItemDelegate *delegate )
{
  mProxyModel->setSourceModel( model );
  mTablesTreeView->setModel( mProxyModel );
  if ( delegate )
    mTablesTreeView->setItemDelegate( delegate );

  // setting the search coluns in search settings menu using the model header data
  if ( mSearchSettingsMenu )
    mSearchSettingsMenu->deleteLater();
  mSearchColumnActions.clear();
  mSearchSettingsMenu = new QMenu( this );
  // columns
  QActionGroup *columnActionGroup = new QActionGroup( this );
  mSearchColumnAllAction = new QAction( tr( "All" ), mSearchSettingsMenu );
  mSearchColumnAllAction->setCheckable( true );
  mSearchSettingsMenu->addAction( mSearchColumnAllAction );
  columnActionGroup->addAction( mSearchColumnAllAction );
  bool searchColumnAll = settingSearchColumnAll->value( { settingPath() } );
  int storedSearchColumn = -1;
  //Avoid having a usable presetColumn when all is selected
  if ( !searchColumnAll )
  {
    storedSearchColumn = settingSearchColumn->value( { settingPath() } );
    if ( storedSearchColumn < 0 )
    {
      storedSearchColumn = model->defaultSearchColumn();
    }
  }
  bool hasPresetColumn = false;
  const QStringList columns = model->columns();
  for ( int i = 0; i < columns.count(); i++ )
  {
    if ( !model->searchableColumn( i ) )
      continue;
    QAction *action = new QAction( columns.at( i ), mSearchSettingsMenu );
    action->setCheckable( true );
    if ( storedSearchColumn == i )
    {
      action->setChecked( true );
      hasPresetColumn = true;
    }
    mSearchSettingsMenu->addAction( action );
    columnActionGroup->addAction( action );
    mSearchColumnActions << action;
  }
  mSearchColumnAllAction->setChecked( !hasPresetColumn );
  mSearchSettingsMenu->addSeparator();
  QActionGroup *modeActionGroup = new QActionGroup( this );
  // mode: wildcard
  QAction *wildcardAction = new QAction( tr( "Wildcard" ), mSearchSettingsMenu );
  wildcardAction->setCheckable( true );
  //wildcard is active when regex is disabled
  bool regexActionSetting = settingSearchRegex->value( { settingPath() } );
  wildcardAction->setChecked( !regexActionSetting );
  mSearchSettingsMenu->addAction( wildcardAction );
  modeActionGroup->addAction( wildcardAction );
  // mode: regexp
  mSearchModeRegexAction = new QAction( tr( "Regular Expression" ), mSearchSettingsMenu );
  mSearchModeRegexAction->setCheckable( true );
  mSearchModeRegexAction->setChecked( regexActionSetting );
  mSearchSettingsMenu->addAction( mSearchModeRegexAction );
  modeActionGroup->addAction( mSearchModeRegexAction );

  mSearchSettingsButton->setMenu( mSearchSettingsMenu );

  connect( mSearchSettingsMenu, &QMenu::triggered, this, [this]() { filterResults(); } );
  connect( mSearchTableEdit, &QLineEdit::textChanged, this, [this]() { filterResults(); } );
}

void QgsAbstractDbSourceSelect::storeSettings()
{
  if ( !mSearchColumnAllAction->isChecked() )
  {
    for ( int i = 0; i < mSearchColumnActions.count(); i++ )
    {
      if ( mSearchColumnActions.at( i )->isChecked() )
      {
        settingSearchColumn->setValue( i, { settingPath() } );
        break;
      }
    }
  }
  settingSearchColumnAll->setValue( mSearchColumnAllAction->isChecked(), { settingPath() } );
  settingSearchRegex->setValue( mSearchModeRegexAction->isChecked(), { settingPath() } );
}

void QgsAbstractDbSourceSelect::treeviewClicked( const QModelIndex &index )
{
  mBuildQueryButton->setEnabled( index.parent().isValid() );
}

void QgsAbstractDbSourceSelect::treeviewDoubleClicked( const QModelIndex &index )
{
  Q_UNUSED( index )
  addButtonClicked();
}

void QgsAbstractDbSourceSelect::filterResults()
{
  QString searchText = mSearchTableEdit->text();
  bool regex = mSearchModeRegexAction->isChecked();

  if ( mSearchColumnAllAction->isChecked() )
  {
    mProxyModel->setFilterKeyColumn( -1 );
  }
  else
  {
    for ( int i = 0; i < mSearchColumnActions.count(); i++ )
    {
      if ( mSearchColumnActions.at( i )->isChecked() )
      {
        mProxyModel->setFilterKeyColumn( i );
        break;
      }
    }
  }

  if ( regex )
  {
    mProxyModel->setFilterRegularExpression( searchText );
  }
  else
  {
    mProxyModel->setFilterWildcard( searchText );
  }
}
