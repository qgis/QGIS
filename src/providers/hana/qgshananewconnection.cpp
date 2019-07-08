/***************************************************************************
   qgshananewconnection.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
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
#include "qgshanaconnection.h"
#include "qgshananewconnection.h"
#include "qgshanasettings.h"
#include "qgssettings.h"

#include <QInputDialog>
#include <QMessageBox>

using namespace std;

namespace {
bool isStringEmpty(const QString& input)
{
  return (input.isEmpty() || QString(input).replace(" ", "").isEmpty());
}
}

QgsHanaNewConnection::QgsHanaNewConnection(
  QWidget *parent,
  const QString &connName,
  Qt::WindowFlags fl)
  : QDialog(parent, fl)
  , mOriginalConnName(connName)
{
  setupUi(this);

  cmbIdentifierType_changed(cmbIdentifierType->currentIndex());

  connect(cmbIdentifierType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QgsHanaNewConnection::cmbIdentifierType_changed);
  connect(rbtnSingleContainer, &QRadioButton::clicked, this, &QgsHanaNewConnection::rbtnSingleContainer_clicked);
  connect(rbtnMultipleContainers, &QRadioButton::clicked, this, &QgsHanaNewConnection::rbtnMultipleContainers_clicked);
  connect(rbtnTenantDatabase, &QRadioButton::clicked, this, &QgsHanaNewConnection::rbtnTenantDatabase_clicked);
  connect(rbtnSystemDatabase, &QRadioButton::clicked, this, &QgsHanaNewConnection::rbtnSystemDatabase_clicked);
  connect(chkEnableSSL, &QCheckBox::clicked, this, &QgsHanaNewConnection::chkEnableSSL_clicked);
  connect(chkValidateCertificate, &QCheckBox::clicked, this, &QgsHanaNewConnection::chkValidateCertificate_clicked);
  connect(btnConnect, &QPushButton::clicked, this, &QgsHanaNewConnection::btnConnect_clicked);
  connect(buttonBox, &QDialogButtonBox::helpRequested, this, &QgsHanaNewConnection::showHelp);

#ifdef Q_OS_WIN
  txtDriver->setText(QLatin1String("HDBODBC"));
#else
  txtDriver->setText(QLatin1String("/usr/sap/hdbclient/libodbcHDB.so"));
#endif
  cbxCryptoProvider->addItem(tr("openssl"), QLatin1String("openssl"));
  cbxCryptoProvider->addItem(tr("commoncrypto"), QLatin1String("commoncrypto"));
  cbxCryptoProvider->addItem(tr("sapcrypto"), QLatin1String("sapcrypto"));
  cbxCryptoProvider->addItem(tr("mscrypto"), QLatin1String("mscrypto"));

  mAuthSettings->setDataprovider(QStringLiteral("hana"));
  mAuthSettings->showStoreCheckboxes(true);

  if (connName.isEmpty())
  {
    rbtnSingleContainer->setChecked(true);
    frmMultitenantSettings->setEnabled(false);
  }
  else
  {
    QgsHanaSettings settings(connName, true);
    updateControlsFromSettings(settings);
  }
  txtName->setValidator(new QRegExpValidator(QRegExp("[^\\/]*"), txtName));

  chkEnableSSL_clicked();
}

void QgsHanaNewConnection::accept()
{
  if (isStringEmpty(txtName->text()))
  {
    QMessageBox::warning(this,
      tr("Save Connection"), tr("WARNING: Connection name cannot be empty."), QMessageBox::Ok);
    return;
  }

  if (isStringEmpty(txtDriver->text()))
  {
    QMessageBox::warning(this,
      tr("Save Connection"), tr("WARNING: Driver field cannot be empty."), QMessageBox::Ok);
    return;
  }

  if (isStringEmpty(txtHost->text()))
  {
    QMessageBox::warning(this,
      tr("Save Connection"), tr("WARNING: Host field cannot be empty."), QMessageBox::Ok);
    return;
  }

  if (rbtnMultipleContainers->isChecked() && rbtnTenantDatabase->isChecked())
  {
    if (isStringEmpty(txtTenantDatabaseName->text()))
    {
      QMessageBox::warning(this,
        tr("Save Connection"), tr("WARNING: Tenant database name cannot be empty."), QMessageBox::Ok);
      return;
    }
  }

  QString connName = txtName->text();
  QgsHanaSettings::setSelectedConnection(connName);
  bool hasAuthConfigID = !mAuthSettings->configId().isEmpty();

  if (!hasAuthConfigID && mAuthSettings->storePasswordIsChecked() &&
    QMessageBox::question(this,
      tr("Saving Passwords"),
      tr("WARNING: You have opted to save your password. It will be stored in unsecured "
        "plain text in your project files and in your home directory (Unix-like OS) or user profile (Windows). "
        "If you want to avoid this, press Cancel and either:\n\na) Don't save a password in the connection "
        "settings  — it will be requested interactively when needed;\nb) Use the Configuration tab to add your "
        "credentials in an HTTP Basic Authentication method and store them in an encrypted database."),
      QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Cancel)
  {
    return;
  }

  QgsHanaSettings settings(connName, true);
  // warn if entry was renamed to an existing connection
  if ((!mOriginalConnName.isNull() && mOriginalConnName.compare(connName, Qt::CaseInsensitive) != 0) &&
    QMessageBox::question(this, tr("Save Connection"), tr("Should the existing connection %1 be overwritten?").arg(connName),
      QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Cancel)
  {
    return;
  }

  // on rename delete the original entry first
  if (!mOriginalConnName.isNull() && mOriginalConnName != connName)
    QgsHanaSettings::removeConnection(mOriginalConnName);

  readSettingsFromControls(settings);
  if (!mAuthSettings->storeUsernameIsChecked())
    settings.setUserName(QLatin1String(""));
  if (!(mAuthSettings->storePasswordIsChecked() && !hasAuthConfigID))
    settings.setPassword(QLatin1String(""));
  settings.setSaveUserName(mAuthSettings->storeUsernameIsChecked());
  settings.setSavePassword(mAuthSettings->storePasswordIsChecked() && !hasAuthConfigID);

  settings.save();

  QDialog::accept();
}

void QgsHanaNewConnection::btnConnect_clicked()
{
  testConnection();
}

void QgsHanaNewConnection::cmbIdentifierType_changed(int index)
{
  if (QgsHanaIdentifierType::fromInt(index) == QgsHanaIdentifierType::INSTANCE_NUMBER)
  {
    txtIdentifier->setMaxLength(2);
    txtIdentifier->setValidator(new QIntValidator(0, 99, this));
    txtIdentifier->setText(QStringLiteral("00"));
  }
  else
  {
    txtIdentifier->setMaxLength(5);
    txtIdentifier->setValidator(new QIntValidator(10000, 99999, this));
    txtIdentifier->setText(QStringLiteral("00000"));
  }
}

void QgsHanaNewConnection::rbtnSingleContainer_clicked()
{
  frmMultitenantSettings->setEnabled(false);
}

void QgsHanaNewConnection::rbtnMultipleContainers_clicked()
{
  frmMultitenantSettings->setEnabled(true);
}

void QgsHanaNewConnection::rbtnTenantDatabase_clicked()
{
  lblTenantDatabaseName->setEnabled(true);
  txtTenantDatabaseName->setEnabled(true);
}

void QgsHanaNewConnection::rbtnSystemDatabase_clicked()
{
  lblTenantDatabaseName->setEnabled(false);
  txtTenantDatabaseName->setEnabled(false);
}

void QgsHanaNewConnection::chkEnableSSL_clicked()
{
  bool enabled = chkEnableSSL->isChecked();
  lblCryptoProvider->setEnabled(enabled);
  cbxCryptoProvider->setEnabled(enabled);
  chkValidateCertificate->setEnabled(enabled);
  lblOverrideHostName->setEnabled(enabled);
  txtOverrideHostName->setEnabled(enabled && chkValidateCertificate->isChecked());
  lblKeyStore->setEnabled(enabled);
  txtKeyStore->setEnabled(enabled);
  lblTrustStore->setEnabled(enabled);
  txtTrustStore->setEnabled(enabled);
}

void QgsHanaNewConnection::chkValidateCertificate_clicked()
{
  txtOverrideHostName->setEnabled(chkValidateCertificate->isChecked());
}

void QgsHanaNewConnection::readSettingsFromControls(QgsHanaSettings& settings)
{
  settings.setDriver(txtDriver->text());
  settings.setHost(txtHost->text());
  settings.setIdentifierType(cmbIdentifierType->currentIndex());
  settings.setIdentifier(txtIdentifier->text());
  settings.setDatabase(getDatabaseName());
  settings.setMultitenant(rbtnMultipleContainers->isChecked());
  settings.setSchema(txtSchema->text());
  settings.setAuthCfg(mAuthSettings->configId());
  settings.setUserName(mAuthSettings->username());
  settings.setPassword(mAuthSettings->password());
  settings.setSaveUserName(mAuthSettings->storeUsernameIsChecked());
  settings.setSavePassword(mAuthSettings->storePasswordIsChecked());
  settings.setUserTablesOnly(chkUserTablesOnly->isChecked());
  settings.setAllowGeometrylessTables(chkAllowGeometrylessTables->isChecked());
  settings.setEnableSsl(chkEnableSSL->isChecked());
  settings.setSslCryptoProvider(cbxCryptoProvider->currentData().toString());
  settings.setSslKeyStore(txtKeyStore->text());
  settings.setSslTrustStore(txtTrustStore->text());
  settings.setSslValidateCertificate(chkValidateCertificate->isChecked());
  settings.setSslHostNameInCertificate(txtOverrideHostName->text());
}

void QgsHanaNewConnection::updateControlsFromSettings(const QgsHanaSettings& settings)
{
  txtDriver->setText(settings.getDriver());
  txtHost->setText(settings.getHost());
  cmbIdentifierType->setCurrentIndex(QgsHanaIdentifierType::INSTANCE_NUMBER);
  cmbIdentifierType->setCurrentIndex(settings.getIdentifierType());
  txtIdentifier->setText(settings.getIdentifier());
  if (!settings.getMultitenant())
  {
    rbtnSingleContainer->setChecked(true);
    frmMultitenantSettings->setEnabled(false);
  }
  else
  {
    rbtnMultipleContainers->setChecked(true);
    if (settings.getDatabase() == QStringLiteral("SYSTEMDB"))
      rbtnSystemDatabase->setChecked(true);
    else
      txtTenantDatabaseName->setText(settings.getDatabase());
  }
  txtSchema->setText(settings.getSchema());
  chkUserTablesOnly->setChecked(settings.getUserTablesOnly());
  chkAllowGeometrylessTables->setChecked(settings.getAllowGeometrylessTables());

  chkEnableSSL->setChecked(settings.getEnableSsl());
  int idx = cbxCryptoProvider->findData(settings.getSslCryptoProvider());
  if (idx >= 0)
    cbxCryptoProvider->setCurrentIndex(idx);

  chkValidateCertificate->setChecked(settings.getSslValidateCertificate());
  txtOverrideHostName->setText(settings.getSslHostNameInCertificate());
  txtKeyStore->setText(settings.getSslKeyStore());
  txtTrustStore->setText(settings.getSslTrustStore());

  if (settings.getSaveUserName())
  {
    mAuthSettings->setUsername(settings.getUserName());
    mAuthSettings->setStoreUsernameChecked(true);
  }

  if (settings.getSavePassword())
  {
    mAuthSettings->setPassword(settings.getPassword());
    mAuthSettings->setStorePasswordChecked(true);
  }

  mAuthSettings->setConfigId(settings.getAuthCfg());

  txtName->setText(settings.getName());
}

void QgsHanaNewConnection::testConnection()
{
  QString warningMsg;
  if (txtDriver->text().isEmpty())
    warningMsg = QStringLiteral("Driver name has not been specified.");
  else if (txtHost->text().isEmpty())
    warningMsg = QStringLiteral("Host name has not been specified.");
  else if (rbtnMultipleContainers->isChecked() && rbtnTenantDatabase->isChecked() &&
    txtTenantDatabaseName->text().isEmpty())
    warningMsg = QStringLiteral("Database has not been specified.");
  else if (mAuthSettings->username().isEmpty())
    warningMsg = QStringLiteral("User name has not been specified.");
  else if (mAuthSettings->password().isEmpty())
    warningMsg = QStringLiteral("Password has not been specified.");
  else if (txtIdentifier->text().isEmpty())
    warningMsg = QStringLiteral("Identifier has not been specified.");
  else
  {
    auto id = QgsHanaIdentifierType::fromInt(cmbIdentifierType->currentIndex());
    int len = txtIdentifier->text().length();
    if ((id == QgsHanaIdentifierType::INSTANCE_NUMBER && len != 2) ||
        (id == QgsHanaIdentifierType::PORT_NUMBER && len != 5))
      warningMsg = QStringLiteral("Identifier has incorrect format.");
  }

  if (!warningMsg.isEmpty())
  {
    bar->clearWidgets();
    bar->pushWarning(tr("Connection Failed"), warningMsg);
    return;
  }

  QgsTemporaryCursorOverride cursorOverride(Qt::WaitCursor);

  QgsHanaSettings settings(txtName->text());
  readSettingsFromControls(settings);
  QgsHanaConnectionRef connRef(settings.toDataSourceUri());

  if (!connRef.isNull())
    bar->pushMessage(tr("Connection to the server was successful."), Qgis::Info);
  else
    bar->pushMessage(tr("Connection failed - consult message log for details."), Qgis::Warning);
}

QString QgsHanaNewConnection::getDatabaseName() const
{
  if (rbtnMultipleContainers->isChecked())
  {
    if (rbtnTenantDatabase->isChecked())
      return QString(txtTenantDatabaseName->text());
    else
      return QStringLiteral("SYSTEMDB");
  }
  else
    return QStringLiteral("");
}

void QgsHanaNewConnection::showHelp()
{
  QgsHelp::openHelp(QStringLiteral("managing_data_source/opening_data.html#creating-a-stored-connection"));
}
