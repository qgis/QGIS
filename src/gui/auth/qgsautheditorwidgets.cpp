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
#include "qgsapplication.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsauthmethodmetadata.h"


QgsAuthMethodPlugins::QgsAuthMethodPlugins( QWidget *parent )
  : QDialog( parent )

{
  if ( QgsApplication::authManager()->isDisabled() )
  {
    mAuthNotifyLayout = new QVBoxLayout;
    this->setLayout( mAuthNotifyLayout );
    mAuthNotify = new QLabel( QgsApplication::authManager()->disabledMessage(), this );
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
  tblAuthPlugins->setRowCount( QgsApplication::authManager()->authMethodsKeys().size() );
  tblAuthPlugins->verticalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );
  tblAuthPlugins->setSortingEnabled( true );
  tblAuthPlugins->setSelectionBehavior( QAbstractItemView::SelectRows );
}

void QgsAuthMethodPlugins::populateTable()
{
  const QStringList authMethodKeys = QgsApplication::authManager()->authMethodsKeys();

  int i = 0;
  const auto constAuthMethodKeys = authMethodKeys;
  for ( const QString &authMethodKey : constAuthMethodKeys )
  {
    const QgsAuthMethodMetadata *meta = QgsApplication::authManager()->authMethodMetadata( authMethodKey );
    const QgsAuthMethod *method = QgsApplication::authManager()->authMethod( authMethodKey );
    if ( !meta || !method )
    {
      QgsDebugMsg( QStringLiteral( "Load auth method instance FAILED for auth method key (%1)" ).arg( authMethodKey ) );
      continue;
    }

    QTableWidgetItem *twi = new QTableWidgetItem( meta->key() );
    twi->setFlags( twi->flags() & ~Qt::ItemIsEditable );
    tblAuthPlugins->setItem( i, 0, twi );

    twi = new QTableWidgetItem( meta->description() );
    twi->setFlags( twi->flags() & ~Qt::ItemIsEditable );
    tblAuthPlugins->setItem( i, 1, twi );

    twi = new QTableWidgetItem( method->supportedDataProviders().join( QLatin1String( ", " ) ) );
    twi->setFlags( twi->flags() & ~Qt::ItemIsEditable );
    tblAuthPlugins->setItem( i, 2, twi );

    i++;
  }
  tblAuthPlugins->sortItems( 0 );
}



QgsAuthEditorWidgets::QgsAuthEditorWidgets( QWidget *parent )
  : QWidget( parent )

{
  setupUi( this );
  connect( btnCertManager, &QPushButton::clicked, this, &QgsAuthEditorWidgets::btnCertManager_clicked );
  connect( btnAuthPlugins, &QPushButton::clicked, this, &QgsAuthEditorWidgets::btnAuthPlugins_clicked );
  if ( !QgsApplication::authManager()->isDisabled() )
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

void QgsAuthEditorWidgets::btnCertManager_clicked()
{
  QgsAuthCertManager *dlg = new QgsAuthCertManager( this );
  dlg->setWindowModality( Qt::ApplicationModal );
  dlg->resize( 750, 500 );
  dlg->exec();
  dlg->deleteLater();
}

void QgsAuthEditorWidgets::btnAuthPlugins_clicked()
{
  QgsAuthMethodPlugins *dlg = new QgsAuthMethodPlugins( this );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 675, 500 );
  dlg->exec();
  dlg->deleteLater();
}

void QgsAuthEditorWidgets::setupUtilitiesMenu()
{
  connect( QgsApplication::authManager(), &QgsAuthManager::messageOut,
           this, &QgsAuthEditorWidgets::authMessageOut );

  // set up utility actions menu
  mActionImportAuthenticationConfigs = new QAction( tr( "Import authentication configurations from file" ), this );
  mActionExportSelectedAuthenticationConfigs = new QAction( tr( "Export selected authentication configurations to file" ), this );
  mActionSetMasterPassword = new QAction( tr( "Input master password" ), this );
  mActionClearCachedMasterPassword = new QAction( tr( "Clear cached master password" ), this );
  mActionResetMasterPassword = new QAction( tr( "Reset master password" ), this );
  mActionClearCachedAuthConfigs = new QAction( tr( "Clear cached authentication configurations" ), this );
  mActionRemoveAuthConfigs = new QAction( tr( "Remove all authentication configurations" ), this );
  mActionEraseAuthDatabase = new QAction( tr( "Erase authentication database" ), this );

  mActionClearAccessCacheNow = new QAction( tr( "Clear network authentication access cache" ), this );
  mActionAutoClearAccessCache = new QAction( tr( "Automatically clear network authentication access cache on SSL errors" ), this );
  mActionAutoClearAccessCache->setCheckable( true );
  mActionAutoClearAccessCache->setChecked( QgsSettings().value( QStringLiteral( "clear_auth_cache_on_errors" ), true, QgsSettings::Section::Auth ).toBool( ) );

  mActionPasswordHelperSync = new QAction( tr( "Store/update the master password in your %1" )
      .arg( QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME ), this );
  mActionPasswordHelperDelete = new QAction( tr( "Clear the master password from your %1" )
      .arg( QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME ), this );
  mActionPasswordHelperEnable = new QAction( tr( "Integrate master password with your %1" )
      .arg( QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME ), this );
  mActionPasswordHelperLoggingEnable = new QAction( tr( "Enable password helper debug log" ), this );

  mActionPasswordHelperEnable->setCheckable( true );
  mActionPasswordHelperEnable->setChecked( QgsApplication::authManager()->passwordHelperEnabled() );

  mActionPasswordHelperLoggingEnable->setCheckable( true );
  mActionPasswordHelperLoggingEnable->setChecked( QgsApplication::authManager()->passwordHelperLoggingEnabled() );

  connect( mActionImportAuthenticationConfigs, &QAction::triggered, this, &QgsAuthEditorWidgets::importAuthenticationConfigs );
  connect( mActionExportSelectedAuthenticationConfigs, &QAction::triggered, this, &QgsAuthEditorWidgets::exportSelectedAuthenticationConfigs );
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

  connect( mActionClearAccessCacheNow, &QAction::triggered, this, [ = ]
  {
    QgsNetworkAccessManager::instance()->clearAccessCache();
    messageBar()->pushSuccess( tr( "Auth cache cleared" ), tr( "Network authentication cache has been cleared" ) );
  } );
  connect( mActionAutoClearAccessCache, &QAction::triggered, this, [ ]( bool checked )
  {
    QgsSettings().setValue( QStringLiteral( "clear_auth_cache_on_errors" ), checked, QgsSettings::Section::Auth );
  } );

  mAuthUtilitiesMenu = new QMenu( this );
  mAuthUtilitiesMenu->addAction( mActionSetMasterPassword );
  mAuthUtilitiesMenu->addAction( mActionClearCachedMasterPassword );
  mAuthUtilitiesMenu->addAction( mActionResetMasterPassword );
  mAuthUtilitiesMenu->addSeparator();
  mAuthUtilitiesMenu->addAction( mActionClearAccessCacheNow );
  mAuthUtilitiesMenu->addAction( mActionAutoClearAccessCache );
  mAuthUtilitiesMenu->addSeparator();
  mAuthUtilitiesMenu->addAction( mActionPasswordHelperEnable );
  mAuthUtilitiesMenu->addAction( mActionPasswordHelperSync );
  mAuthUtilitiesMenu->addAction( mActionPasswordHelperDelete );
  mAuthUtilitiesMenu->addAction( mActionPasswordHelperLoggingEnable );
  mAuthUtilitiesMenu->addSeparator();
  mAuthUtilitiesMenu->addAction( mActionClearCachedAuthConfigs );
  mAuthUtilitiesMenu->addAction( mActionRemoveAuthConfigs );
  mAuthUtilitiesMenu->addSeparator();
  mAuthUtilitiesMenu->addAction( mActionImportAuthenticationConfigs );
  mAuthUtilitiesMenu->addAction( mActionExportSelectedAuthenticationConfigs );
  mAuthUtilitiesMenu->addSeparator();
  mAuthUtilitiesMenu->addAction( mActionEraseAuthDatabase );

  btnAuthUtilities->setMenu( mAuthUtilitiesMenu );
}

void QgsAuthEditorWidgets::importAuthenticationConfigs()
{
  QgsAuthGuiUtils::importAuthenticationConfigs( messageBar() );
}

void QgsAuthEditorWidgets::exportSelectedAuthenticationConfigs()
{
  if ( !wdgtConfigEditor )
    return;

  QgsAuthGuiUtils::exportSelectedAuthenticationConfigs( wdgtConfigEditor->selectedAuthenticationConfigIds(), messageBar() );
}

void QgsAuthEditorWidgets::setMasterPassword()
{
  QgsAuthGuiUtils::setMasterPassword( messageBar() );
}

void QgsAuthEditorWidgets::clearCachedMasterPassword()
{
  QgsAuthGuiUtils::clearCachedMasterPassword( messageBar() );
}

void QgsAuthEditorWidgets::resetMasterPassword()
{
  QgsAuthGuiUtils::resetMasterPassword( messageBar(), this );
}

void QgsAuthEditorWidgets::clearCachedAuthenticationConfigs()
{
  QgsAuthGuiUtils::clearCachedAuthenticationConfigs( messageBar() );
}

void QgsAuthEditorWidgets::removeAuthenticationConfigs()
{
  QgsAuthGuiUtils::removeAuthenticationConfigs( messageBar(), this );
}

void QgsAuthEditorWidgets::eraseAuthenticationDatabase()
{
  QgsAuthGuiUtils::eraseAuthenticationDatabase( messageBar(), this );
}

void QgsAuthEditorWidgets::authMessageOut( const QString &message, const QString &authtag, QgsAuthManager::MessageLevel level )
{
  const int levelint = static_cast<int>( level );
  messageBar()->pushMessage( authtag, message, ( Qgis::MessageLevel )levelint );
}

void QgsAuthEditorWidgets::passwordHelperDelete()
{
  QgsAuthGuiUtils::passwordHelperDelete( messageBar(), this );
}

void QgsAuthEditorWidgets::passwordHelperSync()
{
  QgsAuthGuiUtils::passwordHelperSync( messageBar() );
}

void QgsAuthEditorWidgets::passwordHelperEnableTriggered()
{
  // Only fire on real changes
  QgsAuthGuiUtils::passwordHelperEnable( mActionPasswordHelperEnable->isChecked(), messageBar() );
}

void QgsAuthEditorWidgets::passwordHelperLoggingEnableTriggered()
{
  QgsAuthGuiUtils::passwordHelperLoggingEnable( mActionPasswordHelperLoggingEnable->isChecked(), messageBar() );
}

QgsMessageBar *QgsAuthEditorWidgets::messageBar()
{
  return mMsgBar;
}

