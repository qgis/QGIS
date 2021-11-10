/***************************************************************************
   qgsdbsourceselectbase.h
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

#include "qgsabstractdbtablemodel.h"
#include "qgsdbsourceselectbase.h"

#include <QMenu>
#include <QSortFilterProxyModel>

QgsDbSourceSelectBase::QgsDbSourceSelectBase( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
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

}

void QgsDbSourceSelectBase::setSourceModel( QgsAbstractDbTableModel *model )
{
  mProxyModel->setSourceModel( model );

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
  bool hasDefaultSearchColumn = false;
  const QStringList columns = model->columns();
  for ( int i = 0; i < columns.count(); i++ )
  {
    if ( !model->searchableColumn( i ) )
      continue;
    QAction *action = new QAction( columns.at( i ), mSearchSettingsMenu );
    action->setCheckable( true );
    if ( model->defaultSearchColumn() == i )
    {
      action->setChecked( true );
      hasDefaultSearchColumn = true;
    }
    mSearchSettingsMenu->addAction( action );
    columnActionGroup->addAction( action );
    mSearchColumnActions << action;
  }
  mSearchColumnAllAction->setChecked( !hasDefaultSearchColumn );
  mSearchSettingsMenu->addSeparator();
  QActionGroup *modeActionGroup = new QActionGroup( this );
  // mode: wildcard
  QAction *wildcardAction = new QAction( tr( "Wildcard" ), mSearchSettingsMenu );
  wildcardAction->setCheckable( true );
  wildcardAction->setChecked( true );
  mSearchSettingsMenu->addAction( wildcardAction );
  modeActionGroup->addAction( wildcardAction );
  // mode: regexp
  mSearchModeRegexAction = new QAction( tr( "Regular expression" ), mSearchSettingsMenu );
  mSearchModeRegexAction->setCheckable( true );
  mSearchModeRegexAction->setChecked( false );
  mSearchSettingsMenu->addAction( mSearchModeRegexAction );
  modeActionGroup->addAction( mSearchModeRegexAction );

  mSearchSettingsButton->setMenu( mSearchSettingsMenu );

  connect( mSearchSettingsMenu, &QMenu::triggered, this, [ = ]() {filterResults();} );
  connect( mSearchTableEdit, &QLineEdit::textChanged, this, [ = ]() {filterResults();} );
}


void QgsDbSourceSelectBase::filterResults()
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
      }
    }
  }

  if ( regex )
  {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    mProxyModel->setFilterRegExp( searchText );
#else
    mProxyModel->setFilterRegularExpression( searchText );
#endif
  }
  else
  {
    mProxyModel->setFilterWildcard( searchText );
  }
}

