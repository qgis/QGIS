/***************************************************************************
    qgsautheditorwidgets.cpp
    ---------------------
    begin                : April 26, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
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

#include "qgsautheditorwidgets.h"
#include "ui_qgsauthmethodplugins.h"

#include <QAction>
#include <QMenu>
#include <QWidget>
#include <QTableWidget>

#include "qgssettings.h"
#include "qgsauthcertificatemanager.h"
#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"


QgsAuthMethodPlugins::QgsAuthMethodPlugins( QWidget *parent )
  : QDialog( parent )
  , mAuthNotifyLayout( nullptr )
  , mAuthNotify( nullptr )
{
  if ( QgsAuthManager::instance()->isDisabled() )
  {
    mAuthNotifyLayout = new QVBoxLayout;
    this->setLayout( mAuthNotifyLayout );
    mAuthNotify = new QLabel( QgsAuthManager::instance()->disabledMessage(), this );
    mAuthNotifyLayout->addWidget( mAuthNotify );
  }
  else
  {
    setupUi( this );
    connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );

    setupTable();
    populateTable();
  }
}

void QgsAuthMethodPlugins::setupTable()
{
  tblAuthPlugins->setColumnCount( 3 );
  tblAuthPlugins->verticalHeader()->hide();
  tblAuthPlugins->horizontalHeader()->setVisible( true );
  tblAuthPlugins->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "Method" ) ) );
  tblAuthPlugins->setHorizontalHeaderItem( 1, new QTableWidgetItem( tr( "Description" ) ) );
  tblAuthPlugins->setHorizontalHeaderItem( 2, new QTableWidgetItem( tr( "Works with" ) ) );
  tblAuthPlugins->horizontalHeader()->setStretchLastSection( true );
  tblAuthPlugins->setAlternatingRowColors( true );
  tblAuthPlugins->setColumnWidth( 0, 150 );
  tblAuthPlugins->setColumnWidth( 1, 300 );
  tblAuthPlugins->setRowCount( QgsAuthManager::instance()->authMethodsKeys().size() );
  tblAuthPlugins->verticalHeader()->setResizeMode( QHeaderView::ResizeToContents );
  tblAuthPlugins->setSortingEnabled( true );
  tblAuthPlugins->setSelectionBehavior( QAbstractItemView::SelectRows );
}

void QgsAuthMethodPlugins::populateTable()
{
  QgsAuthMethodsMap authmethods( QgsAuthManager::instance()->authMethodsMap() );

  int i = 0;
  for ( QgsAuthMethodsMap::const_iterator it = authmethods.constBegin(); it != authmethods.constEnd(); ++it, i++ )
  {
    QgsAuthMethod *authmethod( it.value() );
    if ( !authmethod )
    {
      continue;
    }

    QTableWidgetItem *twi = new QTableWidgetItem( authmethod->key() );
    twi->setFlags( twi->flags() & ~Qt::ItemIsEditable );
    tblAuthPlugins->setItem( i, 0, twi );

    twi = new QTableWidgetItem( authmethod->displayDescription() );
    twi->setFlags( twi->flags() & ~Qt::ItemIsEditable );
    tblAuthPlugins->setItem( i, 1, twi );

    twi = new QTableWidgetItem( authmethod->supportedDataProviders().join( QStringLiteral( ", " ) ) );
    twi->setFlags( twi->flags() & ~Qt::ItemIsEditable );
    tblAuthPlugins->setItem( i, 2, twi );
  }
  tblAuthPlugins->sortItems( 0 );
}



QgsAuthEditorWidgets::QgsAuthEditorWidgets( QWidget *parent )
  : QWidget( parent )
  , mAuthUtilitiesMenu( nullptr )
  , mActionSetMasterPassword( nullptr )
  , mActionClearCachedMasterPassword( nullptr )
  , mActionResetMasterPassword( nullptr )
  , mActionClearCachedAuthConfigs( nullptr )
  , mActionRemoveAuthConfigs( nullptr )
  , mActionEraseAuthDatabase( nullptr )
{
  setupUi( this );
  if ( !QgsAuthManager::instance()->isDisabled() )
  {
    wdgtConfigEditor->setRelayMessages( false );
    wdgtConfigEditor->setShowUtilitiesButton( false );
    setupUtilitiesMenu();
  }
  else
  {
    grpbxManagers->setEnabled( false );
  }
}

void QgsAuthEditorWidgets::on_btnCertManager_clicked()
{
  QgsAuthCertManager *dlg = new QgsAuthCertManager( this );
  dlg->setWindowModality( Qt::ApplicationModal );
  dlg->resize( 750, 500 );
  dlg->exec();
  dlg->deleteLater();
}

void QgsAuthEditorWidgets::on_btnAuthPlugins_clicked()
{
  QgsAuthMethodPlugins *dlg = new QgsAuthMethodPlugins( this );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 675, 500 );
  dlg->exec();
  dlg->deleteLater();
}

void QgsAuthEditorWidgets::setupUtilitiesMenu()
{
  connect( QgsAuthManager::instance(), &QgsAuthManager::messageOut,
           this, &QgsAuthEditorWidgets::authMessageOut );

  // set up utility actions menu
  mActionSetMasterPassword = new QAction( QStringLiteral( "Input master password" ), this );
  mActionClearCachedMasterPassword = new QAction( QStringLiteral( "Clear cached master password" ), this );
  mActionResetMasterPassword = new QAction( QStringLiteral( "Reset master password" ), this );
  mActionClearCachedAuthConfigs = new QAction( QStringLiteral( "Clear cached authentication configurations" ), this );
  mActionRemoveAuthConfigs = new QAction( QStringLiteral( "Remove all authentication configurations" ), this );
  mActionEraseAuthDatabase = new QAction( QStringLiteral( "Erase authentication database" ), this );

  mActionPasswordHelperSync = new QAction( tr( "Store/update the master password in your %1" )
      .arg( QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME ), this );
  mActionPasswordHelperDelete = new QAction( tr( "Clear the master password from your %1" )
      .arg( QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME ), this );
  mActionPasswordHelperEnable = new QAction( tr( "Integrate master password with your %1" )
      .arg( QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME ), this );
  mActionPasswordHelperLoggingEnable = new QAction( tr( "Enable password helper debug log" ), this );

  mActionPasswordHelperEnable->setCheckable( true );
  mActionPasswordHelperEnable->setChecked( QgsAuthManager::instance()->passwordHelperEnabled() );

  mActionPasswordHelperLoggingEnable->setCheckable( true );
  mActionPasswordHelperLoggingEnable->setChecked( QgsAuthManager::instance()->passwordHelperLoggingEnabled() );

  connect( mActionSetMasterPassword, &QAction::triggered, this, &QgsAuthEditorWidgets::setMasterPassword );
  connect( mActionClearCachedMasterPassword, &QAction::triggered, this, &QgsAuthEditorWidgets::clearCachedMasterPassword );
  connect( mActionResetMasterPassword, &QAction::triggered, this, &QgsAuthEditorWidgets::resetMasterPassword );
  connect( mActionClearCachedAuthConfigs, &QAction::triggered, this, &QgsAuthEditorWidgets::clearCachedAuthenticationConfigs );
  connect( mActionRemoveAuthConfigs, &QAction::triggered, this, &QgsAuthEditorWidgets::removeAuthenticationConfigs );
  connect( mActionEraseAuthDatabase, &QAction::triggered, this, &QgsAuthEditorWidgets::eraseAuthenticationDatabase );

  connect( mActionPasswordHelperSync, &QAction::triggered, this, &QgsAuthEditorWidgets::passwordHelperSync );
  connect( mActionPasswordHelperDelete, &QAction::triggered, this, &QgsAuthEditorWidgets::passwordHelperDelete );
  connect( mActionPasswordHelperEnable, &QAction::triggered, this, &QgsAuthEditorWidgets::passwordHelperEnableTriggered );
  connect( mActionPasswordHelperLoggingEnable, &QAction::triggered, this, &QgsAuthEditorWidgets::passwordHelperLoggingEnableTriggered );

  mAuthUtilitiesMenu = new QMenu( this );
  mAuthUtilitiesMenu->addAction( mActionSetMasterPassword );
  mAuthUtilitiesMenu->addAction( mActionClearCachedMasterPassword );
  mAuthUtilitiesMenu->addAction( mActionResetMasterPassword );
  mAuthUtilitiesMenu->addSeparator();
  mAuthUtilitiesMenu->addAction( mActionPasswordHelperEnable );
  mAuthUtilitiesMenu->addAction( mActionPasswordHelperSync );
  mAuthUtilitiesMenu->addAction( mActionPasswordHelperDelete );
  mAuthUtilitiesMenu->addAction( mActionPasswordHelperLoggingEnable );
  mAuthUtilitiesMenu->addSeparator();
  mAuthUtilitiesMenu->addAction( mActionClearCachedAuthConfigs );
  mAuthUtilitiesMenu->addAction( mActionRemoveAuthConfigs );
  mAuthUtilitiesMenu->addSeparator();
  mAuthUtilitiesMenu->addAction( mActionEraseAuthDatabase );

  btnAuthUtilities->setMenu( mAuthUtilitiesMenu );
}

void QgsAuthEditorWidgets::setMasterPassword()
{
  QgsAuthGuiUtils::setMasterPassword( messageBar(), messageTimeout() );
}

void QgsAuthEditorWidgets::clearCachedMasterPassword()
{
  QgsAuthGuiUtils::clearCachedMasterPassword( messageBar(), messageTimeout() );
}

void QgsAuthEditorWidgets::resetMasterPassword()
{
  QgsAuthGuiUtils::resetMasterPassword( messageBar(), messageTimeout(), this );
}

void QgsAuthEditorWidgets::clearCachedAuthenticationConfigs()
{
  QgsAuthGuiUtils::clearCachedAuthenticationConfigs( messageBar(), messageTimeout() );
}

void QgsAuthEditorWidgets::removeAuthenticationConfigs()
{
  QgsAuthGuiUtils::removeAuthenticationConfigs( messageBar(), messageTimeout(), this );
}

void QgsAuthEditorWidgets::eraseAuthenticationDatabase()
{
  QgsAuthGuiUtils::eraseAuthenticationDatabase( messageBar(), messageTimeout(), this );
}

void QgsAuthEditorWidgets::authMessageOut( const QString &message, const QString &authtag, QgsAuthManager::MessageLevel level )
{
  int levelint = ( int )level;
  messageBar()->pushMessage( authtag, message, ( QgsMessageBar::MessageLevel )levelint, 7 );
}

void QgsAuthEditorWidgets::passwordHelperDelete()
{
  QgsAuthGuiUtils::passwordHelperDelete( messageBar(), messageTimeout(), this );
}

void QgsAuthEditorWidgets::passwordHelperSync()
{
  QgsAuthGuiUtils::passwordHelperSync( messageBar(), messageTimeout() );
}

void QgsAuthEditorWidgets::passwordHelperEnableTriggered()
{
  // Only fire on real changes
  QgsAuthGuiUtils::passwordHelperEnable( mActionPasswordHelperEnable->isChecked(), messageBar(), messageTimeout() );
}

void QgsAuthEditorWidgets::passwordHelperLoggingEnableTriggered()
{
  QgsAuthGuiUtils::passwordHelperLoggingEnable( mActionPasswordHelperLoggingEnable->isChecked(), messageBar(), messageTimeout() );
}

QgsMessageBar *QgsAuthEditorWidgets::messageBar()
{
  return mMsgBar;
}

int QgsAuthEditorWidgets::messageTimeout()
{
  QgsSettings settings;
  return settings.value( QStringLiteral( "qgis/messageTimeout" ), 5 ).toInt();
}
