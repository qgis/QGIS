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
#include <QSettings>
#include <QSqlTableModel>

#include "qgsauthmanager.h"
#include "qgsauthconfigedit.h"
#include "qgsauthguiutils.h"

QgsAuthConfigEditor::QgsAuthConfigEditor( QWidget *parent, bool showUtilities, bool relayMessages )
    : QWidget( parent )
    , mRelayMessages( relayMessages )
    , mConfigModel( nullptr )
    , mAuthUtilitiesMenu( nullptr )
    , mActionSetMasterPassword( nullptr )
    , mActionClearCachedMasterPassword( nullptr )
    , mActionResetMasterPassword( nullptr )
    , mActionClearCachedAuthConfigs( nullptr )
    , mActionRemoveAuthConfigs( nullptr )
    , mActionEraseAuthDatabase( nullptr )
    , mDisabled( false )
    , mAuthNotifyLayout( nullptr )
    , mAuthNotify( nullptr )
{
  if ( QgsAuthManager::instance()->isDisabled() )
  {
    mDisabled = true;
    mAuthNotifyLayout = new QVBoxLayout;
    this->setLayout( mAuthNotifyLayout );
    mAuthNotify = new QLabel( QgsAuthManager::instance()->disabledMessage(), this );
    mAuthNotifyLayout->addWidget( mAuthNotify );
  }
  else
  {
    setupUi( this );

    setShowUtilitiesButton( showUtilities );

    mConfigModel = new QSqlTableModel( this, QgsAuthManager::instance()->authDbConnection() );
    mConfigModel->setTable( QgsAuthManager::instance()->authDbConfigTable() );
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

    connect( tableViewConfigs->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             this, SLOT( selectionChanged( const QItemSelection&, const QItemSelection& ) ) );

    connect( tableViewConfigs, SIGNAL( doubleClicked( QModelIndex ) ),
             this, SLOT( on_btnEditConfig_clicked() ) );

    if ( mRelayMessages )
    {
      connect( QgsAuthManager::instance(), SIGNAL( messageOut( const QString&, const QString&, QgsAuthManager::MessageLevel ) ),
               this, SLOT( authMessageOut( const QString&, const QString&, QgsAuthManager::MessageLevel ) ) );
    }

    connect( QgsAuthManager::instance(), SIGNAL( authDatabaseChanged() ),
             this, SLOT( refreshTableView() ) );

    checkSelection();

    // set up utility actions menu
    mActionSetMasterPassword = new QAction( "Input master password", this );
    mActionClearCachedMasterPassword = new QAction( "Clear cached master password", this );
    mActionResetMasterPassword = new QAction( "Reset master password", this );
    mActionClearCachedAuthConfigs = new QAction( "Clear cached authentication configurations", this );
    mActionRemoveAuthConfigs = new QAction( "Remove all authentication configurations", this );
    mActionEraseAuthDatabase = new QAction( "Erase authentication database", this );

    connect( mActionSetMasterPassword, SIGNAL( triggered() ), this, SLOT( setMasterPassword() ) );
    connect( mActionClearCachedMasterPassword, SIGNAL( triggered() ), this, SLOT( clearCachedMasterPassword() ) );
    connect( mActionResetMasterPassword, SIGNAL( triggered() ), this, SLOT( resetMasterPassword() ) );
    connect( mActionClearCachedAuthConfigs, SIGNAL( triggered() ), this, SLOT( clearCachedAuthenticationConfigs() ) );
    connect( mActionRemoveAuthConfigs, SIGNAL( triggered() ), this, SLOT( removeAuthenticationConfigs() ) );
    connect( mActionEraseAuthDatabase, SIGNAL( triggered() ), this, SLOT( eraseAuthenticationDatabase() ) );

    mAuthUtilitiesMenu = new QMenu( this );
    mAuthUtilitiesMenu->addAction( mActionSetMasterPassword );
    mAuthUtilitiesMenu->addAction( mActionClearCachedMasterPassword );
    mAuthUtilitiesMenu->addAction( mActionResetMasterPassword );
    mAuthUtilitiesMenu->addSeparator();
    mAuthUtilitiesMenu->addAction( mActionClearCachedAuthConfigs );
    mAuthUtilitiesMenu->addAction( mActionRemoveAuthConfigs );
    mAuthUtilitiesMenu->addSeparator();
    mAuthUtilitiesMenu->addAction( mActionEraseAuthDatabase );

    btnAuthUtilities->setMenu( mAuthUtilitiesMenu );
    lblAuthConfigDb->setVisible( false );
  }
}

QgsAuthConfigEditor::~QgsAuthConfigEditor()
{
}

void QgsAuthConfigEditor::setMasterPassword()
{
  QgsAuthGuiUtils::setMasterPassword( messageBar(), messageTimeout() );
}

void QgsAuthConfigEditor::clearCachedMasterPassword()
{
  QgsAuthGuiUtils::clearCachedMasterPassword( messageBar(), messageTimeout() );
}

void QgsAuthConfigEditor::resetMasterPassword()
{
  QgsAuthGuiUtils::resetMasterPassword( messageBar(), messageTimeout(), this );
}

void QgsAuthConfigEditor::clearCachedAuthenticationConfigs()
{
  QgsAuthGuiUtils::clearCachedAuthenticationConfigs( messageBar(), messageTimeout() );
}

void QgsAuthConfigEditor::removeAuthenticationConfigs()
{
  QgsAuthGuiUtils::removeAuthenticationConfigs( messageBar(), messageTimeout(), this );
}

void QgsAuthConfigEditor::eraseAuthenticationDatabase()
{
  QgsAuthGuiUtils::eraseAuthenticationDatabase( messageBar(), messageTimeout(), this );
}

void QgsAuthConfigEditor::authMessageOut( const QString& message, const QString& authtag, QgsAuthManager::MessageLevel level )
{
  int levelint = ( int )level;
  messageBar()->pushMessage( authtag, message, ( QgsMessageBar::MessageLevel )levelint, 7 );
}

void QgsAuthConfigEditor::toggleTitleVisibility( bool visible )
{
  if ( !mDisabled )
  {
    lblAuthConfigDb->setVisible( visible );
  }
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
    disconnect( QgsAuthManager::instance(), SIGNAL( messageOut( const QString&, const QString&, QgsAuthManager::MessageLevel ) ),
                this, SLOT( authMessageOut( const QString&, const QString&, QgsAuthManager::MessageLevel ) ) );
    mRelayMessages = relay;
    return;
  }

  connect( QgsAuthManager::instance(), SIGNAL( messageOut( const QString&, const QString&, QgsAuthManager::MessageLevel ) ),
           this, SLOT( authMessageOut( const QString&, const QString&, QgsAuthManager::MessageLevel ) ) );
  mRelayMessages = relay;
}

void QgsAuthConfigEditor::refreshTableView()
{
  mConfigModel->select();
  tableViewConfigs->reset();
}

void QgsAuthConfigEditor::selectionChanged( const QItemSelection& selected , const QItemSelection& deselected )
{
  Q_UNUSED( selected );
  Q_UNUSED( deselected );
  checkSelection();
}

void QgsAuthConfigEditor::checkSelection()
{
  bool hasselection = tableViewConfigs->selectionModel()->selection().length() > 0;
  btnEditConfig->setEnabled( hasselection );
  btnRemoveConfig->setEnabled( hasselection );
}

void QgsAuthConfigEditor::on_btnAddConfig_clicked()
{
  if ( !QgsAuthManager::instance()->setMasterPassword( true ) )
    return;

  QgsAuthConfigEdit * ace = new QgsAuthConfigEdit( this );
  ace->setWindowModality( Qt::WindowModal );
  if ( ace->exec() )
  {
    mConfigModel->select();
  }
  ace->deleteLater();
}

void QgsAuthConfigEditor::on_btnEditConfig_clicked()
{
  QString authcfg = selectedConfigId();

  if ( authcfg.isEmpty() )
    return;

  if ( !QgsAuthManager::instance()->setMasterPassword( true ) )
    return;

  QgsAuthConfigEdit * ace = new QgsAuthConfigEdit( this, authcfg );
  ace->setWindowModality( Qt::WindowModal );
  if ( ace->exec() )
  {
    mConfigModel->select();
  }
  ace->deleteLater();
}

void QgsAuthConfigEditor::on_btnRemoveConfig_clicked()
{
  QModelIndexList selection = tableViewConfigs->selectionModel()->selectedRows( 0 );

  if ( selection.empty() )
    return;

  QModelIndex indx = selection.at( 0 );
  QString name = indx.sibling( indx.row(), 1 ).data().toString();

  if ( QMessageBox::warning( this, tr( "Remove Configuration" ),
                             tr( "Are you sure you want to remove '%1'?\n\n"
                                 "Operation can NOT be undone!" ).arg( name ),
                             QMessageBox::Ok | QMessageBox::Cancel,
                             QMessageBox::Cancel ) == QMessageBox::Ok )
  {
    mConfigModel->removeRow( indx.row() );
  }
}

QgsMessageBar * QgsAuthConfigEditor::messageBar()
{
  return mMsgBar;
}

int QgsAuthConfigEditor::messageTimeout()
{
  QSettings settings;
  return settings.value( "/qgis/messageTimeout", 5 ).toInt();
}

QString QgsAuthConfigEditor::selectedConfigId()
{
  QModelIndexList selection = tableViewConfigs->selectionModel()->selectedRows( 0 );

  if ( selection.empty() )
    return QString();

  QModelIndex indx = selection.at( 0 );
  return indx.sibling( indx.row(), 0 ).data().toString();
}
