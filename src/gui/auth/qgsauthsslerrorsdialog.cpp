/***************************************************************************
    qgsauthsslerrorsdialog.cpp
    ---------------------
    begin                : May 22, 2015
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

#include "qgsauthcertificateinfo.h"
#include "qgsauthsslerrorsdialog.h"
#include "qgsauthsslconfigwidget.h"

#include <QDialogButtonBox>
#include <QFont>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>

#include "qgsauthmanager.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsauthcertutils.h"
#include "qgsauthtrustedcasdialog.h"
#include "qgscollapsiblegroupbox.h"
#include "qgslogger.h"
#include "qgsapplication.h"


QgsAuthSslErrorsDialog::QgsAuthSslErrorsDialog( QNetworkReply *reply,
    const QList<QSslError> &sslErrors,
    QWidget *parent,
    const QString &digest,
    const QString &hostport )
  : QDialog( parent )
  , mSslConfiguration( reply->sslConfiguration() )
  , mSslErrors( sslErrors )
  , mDigest( digest )
  , mHostPort( hostport )
{
  if ( mDigest.isEmpty() )
  {
    mDigest = QgsAuthCertUtils::shaHexForCert( mSslConfiguration.peerCertificate() );
  }
  if ( mHostPort.isEmpty() )
  {
    mHostPort = QStringLiteral( "%1:%2" )
                .arg( reply->url().host() )
                .arg( reply->url().port() != -1 ? reply->url().port() : 443 )
                .trimmed();
  }

  setupUi( this );
  connect( buttonBox, &QDialogButtonBox::clicked, this, &QgsAuthSslErrorsDialog::buttonBox_clicked );
  connect( btnChainInfo, &QToolButton::clicked, this, &QgsAuthSslErrorsDialog::btnChainInfo_clicked );
  connect( btnChainCAs, &QToolButton::clicked, this, &QgsAuthSslErrorsDialog::btnChainCAs_clicked );
  connect( grpbxSslErrors, &QgsCollapsibleGroupBoxBasic::collapsedStateChanged, this, &QgsAuthSslErrorsDialog::grpbxSslErrors_collapsedStateChanged );
  QStyle *style = QApplication::style();
  lblWarningIcon->setPixmap( style->standardIcon( QStyle::SP_MessageBoxWarning ).pixmap( 48, 48 ) );
  lblWarningIcon->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

  lblErrorsText->setStyleSheet( QStringLiteral( "QLabel{ font-weight: bold; }" ) );
  leUrl->setText( reply->request().url().toString() );

  ignoreButton()->setDefault( false );
  abortButton()->setDefault( true );

  if ( !QgsApplication::authManager()->isDisabled() )
  {
    saveButton()->setEnabled( false );

    saveButton()->setText( QStringLiteral( "%1 && %2" ).arg( saveButton()->text(),
                           ignoreButton()->text() ) );

    grpbxSslConfig->setChecked( false );
    grpbxSslConfig->setCollapsed( true );
    connect( grpbxSslConfig, &QGroupBox::toggled,
             this, &QgsAuthSslErrorsDialog::loadUnloadCertificate );

    connect( wdgtSslConfig, &QgsAuthSslConfigWidget::readyToSaveChanged,
             this, &QgsAuthSslErrorsDialog::widgetReadyToSaveChanged );
    wdgtSslConfig->setConfigCheckable( false );
    wdgtSslConfig->certificateGroupBox()->setFlat( true );
  }
  else
  {
    btnChainInfo->setVisible( false );
    btnChainCAs->setVisible( false );
    grpbxSslConfig->setVisible( false );
    saveButton()->setVisible( false );
  }

  populateErrorsList();
}

void QgsAuthSslErrorsDialog::loadUnloadCertificate( bool load )
{
  grpbxSslErrors->setCollapsed( load );
  if ( !load )
  {
    QgsDebugMsg( QStringLiteral( "Unloading certificate and host:port" ) );
    clearCertificateConfig();
    return;
  }
  wdgtSslConfig->setEnabled( true );
  QgsDebugMsg( QStringLiteral( "Loading certificate for host:port = %1" ).arg( mHostPort ) );
  wdgtSslConfig->setSslCertificate( mSslConfiguration.peerCertificate(), mHostPort );
  if ( !mSslErrors.isEmpty() )
  {
    wdgtSslConfig->appendSslIgnoreErrors( mSslErrors );
  }
}

void QgsAuthSslErrorsDialog::showCertificateChainInfo()
{
  QList<QSslCertificate> peerchain( mSslConfiguration.peerCertificateChain() );

  if ( !peerchain.isEmpty() )
  {
    const QSslCertificate cert = peerchain.takeFirst();
    if ( !cert.isNull() )
    {
      QgsAuthCertInfoDialog *dlg = new QgsAuthCertInfoDialog( cert, false, this, peerchain );
      dlg->setWindowModality( Qt::WindowModal );
      dlg->resize( 675, 500 );
      dlg->exec();
      dlg->deleteLater();
    }
  }
}

void QgsAuthSslErrorsDialog::showCertificateChainCAsInfo()
{
  const QList< QSslCertificate > certificates = mSslConfiguration.caCertificates();
  for ( const auto &cert : certificates )
  {
    qDebug() << cert.subjectInfo( QSslCertificate::SubjectInfo::CommonName );
  }

  QgsAuthTrustedCAsDialog *dlg = new QgsAuthTrustedCAsDialog( this, mSslConfiguration.caCertificates() );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 675, 500 );
  dlg->exec();
  dlg->deleteLater();
}

void QgsAuthSslErrorsDialog::widgetReadyToSaveChanged( bool cansave )
{
  ignoreButton()->setDefault( false );
  abortButton()->setDefault( !cansave );
  saveButton()->setEnabled( cansave );
  saveButton()->setDefault( cansave );
}

void QgsAuthSslErrorsDialog::checkCanSave()
{
  widgetReadyToSaveChanged( wdgtSslConfig->readyToSave() );
}

void QgsAuthSslErrorsDialog::clearCertificateConfig()
{
  wdgtSslConfig->resetSslCertConfig();
  wdgtSslConfig->setEnabled( false );
  checkCanSave();
}

void QgsAuthSslErrorsDialog::buttonBox_clicked( QAbstractButton *button )
{
  const QDialogButtonBox::StandardButton btnenum( buttonBox->standardButton( button ) );
  switch ( btnenum )
  {
    case QDialogButtonBox::Ignore:
      QgsApplication::authManager()->updateIgnoredSslErrorsCache(
        QStringLiteral( "%1:%2" ).arg( mDigest, mHostPort ),
        mSslErrors );
      accept();
      break;
    case QDialogButtonBox::Save:
      // save config and ignore errors
      wdgtSslConfig->saveSslCertConfig();
      accept();
      break;
    case QDialogButtonBox::Abort:
    default:
      reject();
      break;
  }
  // Clear access cache if the user choose abort and the
  // setting allows it
  if ( btnenum == QDialogButtonBox::Abort &&
       QgsSettings().value( QStringLiteral( "clear_auth_cache_on_errors" ),
                            true,
                            QgsSettings::Section::Auth ).toBool( ) )
  {
    QgsNetworkAccessManager::instance()->clearAccessCache();
  }
}

void QgsAuthSslErrorsDialog::populateErrorsList()
{
  QStringList errs;
  errs.reserve( mSslErrors.size() );
  const auto constMSslErrors = mSslErrors;
  for ( const QSslError &err : constMSslErrors )
  {
    errs <<  QStringLiteral( "* %1: %2" )
         .arg( QgsAuthCertUtils::sslErrorEnumString( err.error() ),
               err.errorString() );
  }
  teSslErrors->setPlainText( errs.join( QLatin1Char( '\n' ) ) );
}

QPushButton *QgsAuthSslErrorsDialog::ignoreButton()
{
  return buttonBox->button( QDialogButtonBox::Ignore );
}

QPushButton *QgsAuthSslErrorsDialog::saveButton()
{
  return buttonBox->button( QDialogButtonBox::Save );
}

QPushButton *QgsAuthSslErrorsDialog::abortButton()
{
  return buttonBox->button( QDialogButtonBox::Abort );
}

void QgsAuthSslErrorsDialog::btnChainInfo_clicked()
{
  showCertificateChainInfo();
}

void QgsAuthSslErrorsDialog::btnChainCAs_clicked()
{
  showCertificateChainCAsInfo();
}

void QgsAuthSslErrorsDialog::grpbxSslErrors_collapsedStateChanged( bool collapsed )
{
  if ( !collapsed && QgsApplication::authManager()->isDisabled() )
  {
    btnChainInfo->setVisible( false );
    btnChainCAs->setVisible( false );
  }
}
