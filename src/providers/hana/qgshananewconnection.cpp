/***************************************************************************
   qgshananewconnection.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsauthmanager.h"
#include "qgsgui.h"
#include "qgshanadriver.h"
#include "qgshanaconnection.h"
#include "qgshananewconnection.h"
#include "qgshanasettings.h"
#include "qgssettings.h"

#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

using namespace std;

namespace
{
  bool isStringEmpty( const QString &input )
  {
    return ( input.isEmpty() || QString( input ).replace( ' ', QString() ).isEmpty() );
  }
}

QgsHanaNewConnection::QgsHanaNewConnection(
  QWidget *parent,
  const QString &connName,
  Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mOriginalConnName( connName )
{
  setupUi( this );

  QgsGui::instance()->enableAutoGeometryRestore( this );

  cmbConnectionType_changed( cmbConnectionType->currentIndex() );
  cmbIdentifierType_changed( cmbIdentifierType->currentIndex() );

  connect( cmbConnectionType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsHanaNewConnection::cmbConnectionType_changed );
  connect( cmbIdentifierType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsHanaNewConnection::cmbIdentifierType_changed );
  connect( rbtnSingleContainer, &QRadioButton::clicked, this, &QgsHanaNewConnection::rbtnSingleContainer_clicked );
  connect( rbtnMultipleContainers, &QRadioButton::clicked, this, &QgsHanaNewConnection::rbtnMultipleContainers_clicked );
  connect( rbtnTenantDatabase, &QRadioButton::clicked, this, &QgsHanaNewConnection::rbtnTenantDatabase_clicked );
  connect( rbtnSystemDatabase, &QRadioButton::clicked, this, &QgsHanaNewConnection::rbtnSystemDatabase_clicked );
  connect( chkEnableSSL, &QCheckBox::clicked, this, &QgsHanaNewConnection::chkEnableSSL_clicked );
  connect( chkEnableProxy, &QCheckBox::clicked, this, &QgsHanaNewConnection::chkEnableProxy_clicked );
  connect( chkValidateCertificate, &QCheckBox::clicked, this, &QgsHanaNewConnection::chkValidateCertificate_clicked );
  connect( btnConnect, &QPushButton::clicked, this, &QgsHanaNewConnection::btnConnect_clicked );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsHanaNewConnection::showHelp );

  txtDriver->setText( QgsHanaDriver::instance()->driver() );
#ifdef Q_OS_WIN
  txtDriver->setToolTip( tr( "The name of the SAP HANA ODBC driver.\n\n"
                             "The SAP HANA ODBC driver is a part of the SAP HANA Client,\n"
                             "which can be found at https://tools.hana.ondemand.com/#hanatools." ) );
#else
  txtDriver->setToolTip( tr( "The name or path to the SAP HANA ODBC driver.\n\n"
                             "If the driver is registered in odbcinst.ini, enter the driver's name.\n"
                             "Otherwise, enter the path to the driver (libodbcHDB.so).\n\n"
                             "The SAP HANA ODBC driver is a part of the SAP HANA Client,\n"
                             "which can be found at https://tools.hana.ondemand.com/#hanatools." ) );
#endif

  cbxCryptoProvider->addItem( QStringLiteral( "openssl" ), QStringLiteral( "openssl" ) );
  cbxCryptoProvider->addItem( QStringLiteral( "commoncrypto" ), QStringLiteral( "commoncrypto" ) );
  cbxCryptoProvider->addItem( QStringLiteral( "sapcrypto" ), QStringLiteral( "sapcrypto" ) );
  cbxCryptoProvider->addItem( QStringLiteral( "mscrypto" ), QStringLiteral( "mscrypto" ) );

  cmbDsn->addItems( QgsHanaDriver::instance()->dataSources() );

  mAuthSettings->setDataprovider( QStringLiteral( "hana" ) );
  mAuthSettings->showStoreCheckboxes( true );

  if ( connName.isEmpty() )
  {
    rbtnSingleContainer->setChecked( true );
    frmMultitenantSettings->setEnabled( false );
  }
  else
  {
    const QgsHanaSettings settings( connName, true );
    updateControlsFromSettings( settings );
  }

  txtName->setValidator( new QRegularExpressionValidator( QRegularExpression( QStringLiteral( "[^\\/]*" ) ), txtName ) );

  chkEnableSSL_clicked();
  chkEnableProxy_clicked();
}

void QgsHanaNewConnection::accept()
{
  if ( isStringEmpty( txtName->text() ) )
  {
    QMessageBox::warning( this,
                          tr( "Save Connection" ), tr( "Connection name cannot be empty." ), QMessageBox::Ok );
    return;
  }

  switch ( getCurrentConnectionType() )
  {
    case QgsHanaConnectionType::HostPort:
      if ( isStringEmpty( txtDriver->text() ) )
      {
        QMessageBox::warning( this,
                              tr( "Save Connection" ), tr( "Driver field cannot be empty." ), QMessageBox::Ok );
        return;
      }

      if ( isStringEmpty( txtHost->text() ) )
      {
        QMessageBox::warning( this,
                              tr( "Save Connection" ), tr( "Host field cannot be empty." ), QMessageBox::Ok );
        return;
      }

      if ( rbtnMultipleContainers->isChecked() && rbtnTenantDatabase->isChecked() )
      {
        if ( isStringEmpty( txtTenantDatabaseName->text() ) )
        {
          QMessageBox::warning( this,
                                tr( "Save Connection" ), tr( "Tenant database name cannot be empty." ), QMessageBox::Ok );
          return;
        }
      }
      break;
    case QgsHanaConnectionType::Dsn:
      if ( cmbDsn->count() == 0 )
      {
        QMessageBox::warning( this,
                              tr( "Save Connection" ), tr( "DSN field cannot be empty." ), QMessageBox::Ok );
        return;
      }

      break;
  }

  const QString connName = txtName->text();
  QgsHanaSettings::setSelectedConnection( connName );
  const bool hasAuthConfigID = !mAuthSettings->configId().isEmpty();

  if ( !hasAuthConfigID && mAuthSettings->storePasswordIsChecked() &&
       QMessageBox::question( this,
                              tr( "Saving Passwords" ),
                              tr( "WARNING: You have opted to save your password. It will be stored in unsecured "
                                  "plain text in your project files and in your home directory (Unix-like OS) or user profile (Windows). "
                                  "If you want to avoid this, press Cancel and either:\n\na) Don't save a password in the connection "
                                  "settings — it will be requested interactively when needed;\nb) Use the Configuration tab to add your "
                                  "credentials in an HTTP Basic Authentication method and store them in an encrypted database." ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  QgsHanaSettings settings( connName, true );
  // warn if entry was renamed to an existing connection
  if ( ( !mOriginalConnName.isNull() && mOriginalConnName.compare( connName, Qt::CaseInsensitive ) != 0 ) &&
       QMessageBox::question( this, tr( "Save Connection" ), tr( "Should the existing connection %1 be overwritten?" ).arg( connName ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  // on rename delete the original entry first
  if ( !mOriginalConnName.isNull() && mOriginalConnName != connName )
    QgsHanaSettings::removeConnection( mOriginalConnName );

  readSettingsFromControls( settings );
  if ( !mAuthSettings->storeUsernameIsChecked() )
    settings.setUserName( QString( ) );
  if ( !( mAuthSettings->storePasswordIsChecked() && !hasAuthConfigID ) )
    settings.setPassword( QString( ) );
  settings.setSaveUserName( mAuthSettings->storeUsernameIsChecked() );
  settings.setSavePassword( mAuthSettings->storePasswordIsChecked() && !hasAuthConfigID );

  settings.save();

  QDialog::accept();
}

void QgsHanaNewConnection::btnConnect_clicked()
{
  testConnection();
}

void QgsHanaNewConnection::cmbConnectionType_changed( int index )
{
  swConnectionControls->setCurrentIndex( index );
  resizeWidgets();
}

void QgsHanaNewConnection::cmbIdentifierType_changed( int index )
{
  if ( QgsHanaIdentifierType::fromInt( static_cast<uint>( index ) ) == QgsHanaIdentifierType::InstanceNumber )
  {
    txtIdentifier->setMaxLength( 2 );
    txtIdentifier->setValidator( new QIntValidator( 0, 99, this ) );
    txtIdentifier->setText( QStringLiteral( "00" ) );
  }
  else
  {
    txtIdentifier->setMaxLength( 5 );
    txtIdentifier->setValidator( new QIntValidator( 1, 65535, this ) );
    txtIdentifier->setText( QStringLiteral( "00000" ) );
  }
}

void QgsHanaNewConnection::rbtnSingleContainer_clicked()
{
  frmMultitenantSettings->setEnabled( false );
}

void QgsHanaNewConnection::rbtnMultipleContainers_clicked()
{
  frmMultitenantSettings->setEnabled( true );
}

void QgsHanaNewConnection::rbtnTenantDatabase_clicked()
{
  lblTenantDatabaseName->setEnabled( true );
  txtTenantDatabaseName->setEnabled( true );
}

void QgsHanaNewConnection::rbtnSystemDatabase_clicked()
{
  lblTenantDatabaseName->setEnabled( false );
  txtTenantDatabaseName->setEnabled( false );
}

void QgsHanaNewConnection::chkEnableSSL_clicked()
{
  const bool enabled = chkEnableSSL->isChecked();
  lblCryptoProvider->setEnabled( enabled );
  cbxCryptoProvider->setEnabled( enabled );
  chkValidateCertificate->setEnabled( enabled );
  lblOverrideHostName->setEnabled( enabled );
  txtOverrideHostName->setEnabled( enabled && chkValidateCertificate->isChecked() );
  lblKeyStore->setEnabled( enabled );
  txtKeyStore->setEnabled( enabled );
  lblTrustStore->setEnabled( enabled );
  txtTrustStore->setEnabled( enabled );
}

void QgsHanaNewConnection::chkEnableProxy_clicked()
{
  const bool enabled = chkEnableProxy->isChecked();
  cmbProxyType->setEnabled( enabled );
  txtProxyHost->setEnabled( enabled );
  txtProxyPort->setEnabled( enabled );
  txtProxyUsername->setEnabled( enabled );
  txtProxyPassword->setEnabled( enabled );
}

void QgsHanaNewConnection::chkValidateCertificate_clicked()
{
  txtOverrideHostName->setEnabled( chkValidateCertificate->isChecked() );
}

void QgsHanaNewConnection::readSettingsFromControls( QgsHanaSettings &settings )
{
  QgsHanaConnectionType connType = getCurrentConnectionType();
  settings.setConnectionType( connType );
  switch ( connType )
  {
    case QgsHanaConnectionType::Dsn:
      settings.setDsn( cmbDsn->count() > 0 ? cmbDsn->currentText() : "" );
      break;
    case QgsHanaConnectionType::HostPort:
      settings.setDriver( txtDriver->text() );
      settings.setHost( txtHost->text() );
      settings.setIdentifierType( static_cast<uint>( cmbIdentifierType->currentIndex() ) );
      settings.setIdentifier( txtIdentifier->text() );
      settings.setDatabase( getDatabaseName() );
      settings.setMultitenant( rbtnMultipleContainers->isChecked() );
      break;
  }
  settings.setSchema( txtSchema->text() );
  settings.setAuthCfg( mAuthSettings->configId() );
  settings.setUserName( mAuthSettings->username() );
  settings.setPassword( mAuthSettings->password() );
  settings.setSaveUserName( mAuthSettings->storeUsernameIsChecked() );
  settings.setSavePassword( mAuthSettings->storePasswordIsChecked() );
  settings.setUserTablesOnly( chkUserTablesOnly->isChecked() );
  settings.setAllowGeometrylessTables( chkAllowGeometrylessTables->isChecked() );
  settings.setEnableSsl( chkEnableSSL->isChecked() );
  settings.setSslCryptoProvider( cbxCryptoProvider->currentData().toString() );
  settings.setSslKeyStore( txtKeyStore->text() );
  settings.setSslTrustStore( txtTrustStore->text() );
  settings.setSslValidateCertificate( chkValidateCertificate->isChecked() );
  settings.setSslHostNameInCertificate( txtOverrideHostName->text() );
  settings.setEnableProxy( chkEnableProxy->isChecked() );
  settings.setEnableProxyHttp( cmbProxyType->currentIndex() == 0 );
  settings.setProxyHost( txtProxyHost->text() );
  settings.setProxyPort( QVariant( txtProxyPort->text() ).toUInt() );
  settings.setProxyUsername( txtProxyUsername->text() );
  settings.setProxyPassword( txtProxyPassword->text() );
}

void QgsHanaNewConnection::updateControlsFromSettings( const QgsHanaSettings &settings )
{
  txtName->setText( settings.name() );

  switch ( settings.connectionType() )
  {
    case QgsHanaConnectionType::Dsn:
    {
      cmbConnectionType->setCurrentIndex( 1 );
      int index = cmbDsn->findText( settings.dsn() );
      if ( index >= 0 )
        cmbDsn->setCurrentIndex( index );
      break;
    }
    case QgsHanaConnectionType::HostPort:
      cmbConnectionType->setCurrentIndex( 0 );
      txtDriver->setText( settings.driver() );
      txtHost->setText( settings.host() );
      cmbIdentifierType->setCurrentIndex( QgsHanaIdentifierType::InstanceNumber );
      cmbIdentifierType->setCurrentIndex( static_cast<int>( settings.identifierType() ) );
      txtIdentifier->setText( settings.identifier() );
      if ( !settings.multitenant() )
      {
        rbtnSingleContainer->setChecked( true );
        frmMultitenantSettings->setEnabled( false );
      }
      else
      {
        rbtnMultipleContainers->setChecked( true );
        if ( settings.database() == QLatin1String( "SYSTEMDB" ) )
          rbtnSystemDatabase->setChecked( true );
        else
          txtTenantDatabaseName->setText( settings.database() );
      }
      break;
  }
  txtSchema->setText( settings.schema() );
  chkUserTablesOnly->setChecked( settings.userTablesOnly() );
  chkAllowGeometrylessTables->setChecked( settings.allowGeometrylessTables() );

  if ( settings.saveUserName() )
  {
    mAuthSettings->setUsername( settings.userName() );
    mAuthSettings->setStoreUsernameChecked( true );
  }

  if ( settings.savePassword() )
  {
    mAuthSettings->setPassword( settings.password() );
    mAuthSettings->setStorePasswordChecked( true );
  }

  mAuthSettings->setConfigId( settings.authCfg() );

  // SSL parameters
  chkEnableSSL->setChecked( settings.enableSsl() );
  const int idx = cbxCryptoProvider->findData( settings.sslCryptoProvider() );
  if ( idx >= 0 )
    cbxCryptoProvider->setCurrentIndex( idx );

  chkValidateCertificate->setChecked( settings.sslValidateCertificate() );
  txtOverrideHostName->setText( settings.sslHostNameInCertificate() );
  txtKeyStore->setText( settings.sslKeyStore() );
  txtTrustStore->setText( settings.sslTrustStore() );

  // Proxy parameters
  chkEnableProxy->setChecked( settings.enableProxy() );
  cmbProxyType->setCurrentIndex( settings.enableProxyHttp() ? 0 : 1 );
  txtProxyHost->setText( settings.proxyHost() );
  txtProxyPort->setText( QString::number( settings.proxyPort() ) );
  txtProxyUsername->setText( settings.proxyUsername() );
  txtProxyPassword->setText( settings.proxyPassword() );
}

void QgsHanaNewConnection::resizeEvent( QResizeEvent * )
{
  resizeWidgets();
}

void QgsHanaNewConnection::resizeWidgets()
{
  auto resizeLayout = []( QLayout * layout )
  {
    QWidget *widget = layout->parentWidget();
    widget->adjustSize();
    widget->resize( widget->parentWidget()->width(), widget->height() );
  };

  // We need to manually resize layouts located inside the StackedWidget.
  resizeLayout( grdConnectionSettings );
  resizeLayout( frmDsn );

  pageHostPort->adjustSize();
  auto size = pageHostPort->size();
  swConnectionControls->setMinimumHeight( size.height() );
  swConnectionControls->setMaximumHeight( size.height() );
}

void QgsHanaNewConnection::testConnection()
{
  QString warningMsg;

  switch ( getCurrentConnectionType() )
  {
    case QgsHanaConnectionType::Dsn:
      if ( cmbDsn->count() == 0 )
        warningMsg = tr( "DSN has not been specified." );
      break;
    case QgsHanaConnectionType::HostPort:
      if ( txtHost->text().isEmpty() )
        warningMsg = tr( "Host name has not been specified." );
      else if ( rbtnMultipleContainers->isChecked() && rbtnTenantDatabase->isChecked() &&
                txtTenantDatabaseName->text().isEmpty() )
        warningMsg = tr( "Database has not been specified." );
      else if ( txtIdentifier->text().isEmpty() )
        warningMsg = tr( "Identifier has not been specified." );
      else
      {
        const QString driver = txtDriver->text();
        if ( driver.isEmpty() )
          warningMsg = tr( "Driver name/path has not been specified." );
        else
        {
          if ( !QgsHanaDriver::isInstalled( driver ) )
          {
#if defined(Q_OS_WIN)
            warningMsg = tr( "Driver with name '%1' is not installed." ).arg( driver );
#else
            if ( !QgsHanaDriver::isValidPath( driver ) )
            {
              if ( QFileInfo::exists( driver ) )
                warningMsg = tr( "Specified driver '%1' cannot be used to connect to SAP HANA." ).arg( driver );
              else
                warningMsg = tr( "Driver with name/path '%1' was not found." ).arg( driver );
            }
#endif
          }
        }
      }
      break;
  }

  if ( mAuthSettings->username().isEmpty() )
    warningMsg = tr( "User name has not been specified." );
  else if ( mAuthSettings->password().isEmpty() )
    warningMsg = tr( "Password has not been specified." );

  if ( !warningMsg.isEmpty() )
  {
    bar->clearWidgets();
    bar->pushWarning( tr( "Connection failed" ), warningMsg );
    return;
  }

  const QgsTemporaryCursorOverride cursorOverride( Qt::WaitCursor );

  QgsHanaSettings settings( txtName->text() );
  readSettingsFromControls( settings );

  QString errorMsg;
  const unique_ptr<QgsHanaConnection> conn( QgsHanaConnection::createConnection( settings.toDataSourceUri(), nullptr, &errorMsg ) );

  if ( conn )
    bar->pushMessage( tr( "Connection to the server was successful." ), Qgis::MessageLevel::Info );
  else
    bar->pushMessage( tr( "Connection failed: %1." ).arg( errorMsg ), Qgis::MessageLevel::Warning );
}

QgsHanaConnectionType QgsHanaNewConnection::getCurrentConnectionType() const
{
  return static_cast<QgsHanaConnectionType>( swConnectionControls->currentIndex() );
}

QString QgsHanaNewConnection::getDatabaseName() const
{
  if ( rbtnMultipleContainers->isChecked() )
  {
    if ( rbtnTenantDatabase->isChecked() )
      return QString( txtTenantDatabaseName->text() );
    else
      return QStringLiteral( "SYSTEMDB" );
  }
  else
    return QString( );
}

void QgsHanaNewConnection::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#creating-a-stored-connection" ) );
}
