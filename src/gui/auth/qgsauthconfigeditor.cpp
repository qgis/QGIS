/***************************************************************************
    qgsauthconfigeditor.cpp
    ---------------------
    begin                : October 5, 2014
    copyright            : (C) 2014 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthconfigeditor.h"
#include "ui_qgsauthconfigeditor.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlTableModel>

#include "qgssettings.h"
#include "qgsauthmanager.h"
#include "qgsauthconfigedit.h"
#include "qgsauthguiutils.h"
#include "qgsapplication.h"

QgsAuthConfigEditor::QgsAuthConfigEditor( QWidget *parent, bool showUtilities, bool relayMessages )
  : QWidget( parent )
  , mRelayMessages( relayMessages )
{
  if ( QgsApplication::authManager()->isDisabled() )
  {
    mDisabled = true;
    mAuthNotifyLayout = new QVBoxLayout;
    this->setLayout( mAuthNotifyLayout );
    mAuthNotify = new QLabel( QgsApplication::authManager()->disabledMessage(), this );
    mAuthNotifyLayout->addWidget( mAuthNotify );
  }
  else
  {
    setupUi( this );
    connect( btnAddConfig, &QToolButton::clicked, this, &QgsAuthConfigEditor::btnAddConfig_clicked );
    connect( btnEditConfig, &QToolButton::clicked, this, &QgsAuthConfigEditor::btnEditConfig_clicked );
    connect( btnRemoveConfig, &QToolButton::clicked, this, &QgsAuthConfigEditor::btnRemoveConfig_clicked );

    setShowUtilitiesButton( showUtilities );

    mConfigModel = new QSqlTableModel( this, QgsApplication::authManager()->authDatabaseConnection() );
    mConfigModel->setTable( QgsApplication::authManager()->authDatabaseConfigTable() );
    mConfigModel->select();

    mConfigModel->setHeaderData( 0, Qt::Horizontal, tr( "ID" ) );
    mConfigModel->setHeaderData( 1, Qt::Horizontal, tr( "Name" ) );
    mConfigModel->setHeaderData( 2, Qt::Horizontal, tr( "URI" ) );
    mConfigModel->setHeaderData( 3, Qt::Horizontal, tr( "Type" ) );
    mConfigModel->setHeaderData( 4, Qt::Horizontal, tr( "Version" ) );
    mConfigModel->setHeaderData( 5, Qt::Horizontal, tr( "Config" ) );

    tableViewConfigs->setModel( mConfigModel );
    tableViewConfigs->resizeColumnsToContents();
//    tableViewConfigs->resizeColumnToContents( 0 );
//    tableViewConfigs->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
//    tableViewConfigs->horizontalHeader()->setResizeMode(2, QHeaderView::Interactive);
//    tableViewConfigs->resizeColumnToContents( 3 );
    tableViewConfigs->hideColumn( 4 );
    tableViewConfigs->hideColumn( 5 );

    // sort by config 'name'
    tableViewConfigs->sortByColumn( 1, Qt::AscendingOrder );
    tableViewConfigs->setSortingEnabled( true );

    connect( tableViewConfigs->selectionModel(), &QItemSelectionModel::selectionChanged,
             this, &QgsAuthConfigEditor::selectionChanged );

    connect( tableViewConfigs, &QAbstractItemView::doubleClicked,
             this, &QgsAuthConfigEditor::btnEditConfig_clicked );

    if ( mRelayMessages )
    {
      connect( QgsApplication::authManager(), &QgsAuthManager::messageOut,
               this, &QgsAuthConfigEditor::authMessageOut );
    }

    connect( QgsApplication::authManager(), &QgsAuthManager::authDatabaseChanged,
             this, &QgsAuthConfigEditor::refreshTableView );

    checkSelection();

    // set up utility actions menu
    mActionImportAuthenticationConfigs = new QAction( tr( "Import authentication configurations from file" ), this );
    mActionExportSelectedAuthenticationConfigs = new QAction( tr( "Export selected authentication configurations to file" ), this );
    mActionSetMasterPassword = new QAction( QStringLiteral( "Input master password" ), this );
    mActionClearCachedMasterPassword = new QAction( QStringLiteral( "Clear cached master password" ), this );
    mActionResetMasterPassword = new QAction( QStringLiteral( "Reset master password" ), this );
    mActionClearCachedAuthConfigs = new QAction( QStringLiteral( "Clear cached authentication configurations" ), this );
    mActionRemoveAuthConfigs = new QAction( QStringLiteral( "Remove all authentication configurations" ), this );
    mActionEraseAuthDatabase = new QAction( QStringLiteral( "Erase authentication database" ), this );

    connect( mActionImportAuthenticationConfigs, &QAction::triggered, this, &QgsAuthConfigEditor::importAuthenticationConfigs );
    connect( mActionExportSelectedAuthenticationConfigs, &QAction::triggered, this, &QgsAuthConfigEditor::exportSelectedAuthenticationConfigs );
    connect( mActionSetMasterPassword, &QAction::triggered, this, &QgsAuthConfigEditor::setMasterPassword );
    connect( mActionClearCachedMasterPassword, &QAction::triggered, this, &QgsAuthConfigEditor::clearCachedMasterPassword );
    connect( mActionResetMasterPassword, &QAction::triggered, this, &QgsAuthConfigEditor::resetMasterPassword );
    connect( mActionClearCachedAuthConfigs, &QAction::triggered, this, &QgsAuthConfigEditor::clearCachedAuthenticationConfigs );
    connect( mActionRemoveAuthConfigs, &QAction::triggered, this, &QgsAuthConfigEditor::removeAuthenticationConfigs );
    connect( mActionEraseAuthDatabase, &QAction::triggered, this, &QgsAuthConfigEditor::eraseAuthenticationDatabase );

    mAuthUtilitiesMenu = new QMenu( this );
    mAuthUtilitiesMenu->addAction( mActionSetMasterPassword );
    mAuthUtilitiesMenu->addAction( mActionClearCachedMasterPassword );
    mAuthUtilitiesMenu->addAction( mActionResetMasterPassword );
    mAuthUtilitiesMenu->addSeparator();
    mAuthUtilitiesMenu->addAction( mActionClearCachedAuthConfigs );
    mAuthUtilitiesMenu->addAction( mActionRemoveAuthConfigs );
    mAuthUtilitiesMenu->addSeparator();
    mAuthUtilitiesMenu->addAction( mActionImportAuthenticationConfigs );
    mAuthUtilitiesMenu->addAction( mActionExportSelectedAuthenticationConfigs );
    mAuthUtilitiesMenu->addSeparator();
    mAuthUtilitiesMenu->addAction( mActionEraseAuthDatabase );

    btnAuthUtilities->setMenu( mAuthUtilitiesMenu );
    lblAuthConfigDb->setVisible( false );
  }
}

void QgsAuthConfigEditor::importAuthenticationConfigs()
{
  QgsAuthGuiUtils::importAuthenticationConfigs( messageBar() );
}

void QgsAuthConfigEditor::exportSelectedAuthenticationConfigs()
{
  QgsAuthGuiUtils::exportSelectedAuthenticationConfigs( selectedAuthenticationConfigIds(), messageBar() );
}

void QgsAuthConfigEditor::setMasterPassword()
{
  QgsAuthGuiUtils::setMasterPassword( messageBar() );
}

void QgsAuthConfigEditor::clearCachedMasterPassword()
{
  QgsAuthGuiUtils::clearCachedMasterPassword( messageBar() );
}

void QgsAuthConfigEditor::resetMasterPassword()
{
  QgsAuthGuiUtils::resetMasterPassword( messageBar(), this );
}

void QgsAuthConfigEditor::clearCachedAuthenticationConfigs()
{
  QgsAuthGuiUtils::clearCachedAuthenticationConfigs( messageBar() );
}

void QgsAuthConfigEditor::removeAuthenticationConfigs()
{
  QgsAuthGuiUtils::removeAuthenticationConfigs( messageBar(), this );
}

void QgsAuthConfigEditor::eraseAuthenticationDatabase()
{
  QgsAuthGuiUtils::eraseAuthenticationDatabase( messageBar(), this );
}

void QgsAuthConfigEditor::authMessageOut( const QString &message, const QString &authtag, QgsAuthManager::MessageLevel level )
{
  const int levelint = static_cast<int>( level );
  messageBar()->pushMessage( authtag, message, ( Qgis::MessageLevel )levelint );
}

void QgsAuthConfigEditor::toggleTitleVisibility( bool visible )
{
  if ( !mDisabled )
  {
    lblAuthConfigDb->setVisible( visible );
  }
}

QStringList QgsAuthConfigEditor::selectedAuthenticationConfigIds() const
{
  QStringList ids;
  const QModelIndexList selection = tableViewConfigs->selectionModel()->selectedRows( 0 );
  for ( const QModelIndex index : selection )
  {
    ids << index.sibling( index.row(), 0 ).data().toString();
  }
  return ids;
}

void QgsAuthConfigEditor::setShowUtilitiesButton( bool show )
{
  if ( !mDisabled )
  {
    btnAuthUtilities->setVisible( show );
  }
}

void QgsAuthConfigEditor::setRelayMessages( bool relay )
{
  if ( mDisabled )
  {
    return;
  }
  if ( relay == mRelayMessages )
  {
    return;
  }

  if ( mRelayMessages )
  {
    disconnect( QgsApplication::authManager(), &QgsAuthManager::messageOut,
                this, &QgsAuthConfigEditor::authMessageOut );
    mRelayMessages = relay;
    return;
  }

  connect( QgsApplication::authManager(), &QgsAuthManager::messageOut,
           this, &QgsAuthConfigEditor::authMessageOut );
  mRelayMessages = relay;
}

void QgsAuthConfigEditor::refreshTableView()
{
  mConfigModel->select();
  tableViewConfigs->reset();
}

void QgsAuthConfigEditor::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( selected )
  Q_UNUSED( deselected )
  checkSelection();
}

void QgsAuthConfigEditor::checkSelection()
{
  const bool hasselection = tableViewConfigs->selectionModel()->selection().length() > 0;
  btnEditConfig->setEnabled( hasselection );
  btnRemoveConfig->setEnabled( hasselection );
}

void QgsAuthConfigEditor::btnAddConfig_clicked()
{
  if ( !QgsApplication::authManager()->setMasterPassword( true ) )
    return;

  QgsAuthConfigEdit *ace = new QgsAuthConfigEdit( this );
  ace->setWindowModality( Qt::WindowModal );
  if ( ace->exec() )
  {
    mConfigModel->select();
  }
  ace->deleteLater();
}

void QgsAuthConfigEditor::btnEditConfig_clicked()
{
  const QString authcfg = selectedConfigId();

  if ( authcfg.isEmpty() )
    return;

  if ( !QgsApplication::authManager()->setMasterPassword( true ) )
    return;

  QgsAuthConfigEdit *ace = new QgsAuthConfigEdit( this, authcfg );
  ace->setWindowModality( Qt::WindowModal );
  if ( ace->exec() )
  {
    mConfigModel->select();
  }
  ace->deleteLater();
}

void QgsAuthConfigEditor::btnRemoveConfig_clicked()
{
  const QModelIndexList selection = tableViewConfigs->selectionModel()->selectedRows( 0 );

  if ( selection.empty() )
    return;

  for ( const QModelIndex index : selection )
  {
    const QString name = index.sibling( index.row(), 1 ).data().toString();

    if ( QMessageBox::warning( this, tr( "Remove Configuration" ),
                               tr( "Are you sure you want to remove '%1'?\n\n"
                                   "Operation can NOT be undone!" ).arg( name ),
                               QMessageBox::Ok | QMessageBox::Cancel,
                               QMessageBox::Cancel ) == QMessageBox::Ok )
    {
      mConfigModel->removeRow( index.row() );
    }
  }
}

QgsMessageBar *QgsAuthConfigEditor::messageBar()
{
  return mMsgBar;
}

QString QgsAuthConfigEditor::selectedConfigId()
{
  const QModelIndexList selection = tableViewConfigs->selectionModel()->selectedRows( 0 );

  if ( selection.empty() )
    return QString();

  const QModelIndex indx = selection.at( 0 );
  return indx.sibling( indx.row(), 0 ).data().toString();
}
