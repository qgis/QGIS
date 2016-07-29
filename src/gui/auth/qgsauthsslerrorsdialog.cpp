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
#include "qgsauthcertutils.h"
#include "qgsauthtrustedcasdialog.h"
#include "qgscollapsiblegroupbox.h"
#include "qgslogger.h"


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
    mHostPort = QString( "%1:%2" )
                .arg( reply->url().host() )
                .arg( reply->url().port() != -1 ? reply->url().port() : 443 )
                .trimmed();
  }

  setupUi( this );
  QStyle *style = QApplication::style();
  lblWarningIcon->setPixmap( style->standardIcon( QStyle::SP_MessageBoxWarning ).pixmap( 48, 48 ) );
  lblWarningIcon->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

  lblErrorsText->setStyleSheet( "QLabel{ font-weight: bold; }" );
  leUrl->setText( reply->request().url().toString() );

  ignoreButton()->setDefault( false );
  abortButton()->setDefault( true );

  if ( !QgsAuthManager::instance()->isDisabled() )
  {
    saveButton()->setEnabled( false );

    saveButton()->setText( QString( "%1 && %2" ).arg( saveButton()->text(),
                           ignoreButton()->text() ) );

    grpbxSslConfig->setChecked( false );
    grpbxSslConfig->setCollapsed( true );
    connect( grpbxSslConfig, SIGNAL( toggled( bool ) ),
             this, SLOT( loadUnloadCertificate( bool ) ) );

    connect( wdgtSslConfig, SIGNAL( readyToSaveChanged( bool ) ),
             this, SLOT( widgetReadyToSaveChanged( bool ) ) );
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

QgsAuthSslErrorsDialog::~QgsAuthSslErrorsDialog()
{
}

void QgsAuthSslErrorsDialog::loadUnloadCertificate( bool load )
{
  grpbxSslErrors->setCollapsed( load );
  if ( !load )
  {
    QgsDebugMsg( "Unloading certificate and host:port" );
    clearCertificateConfig();
    return;
  }
  wdgtSslConfig->setEnabled( true );
  QgsDebugMsg( QString( "Loading certificate for host:port = %1" ).arg( mHostPort ) );
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
    QSslCertificate cert = peerchain.takeFirst();
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

void QgsAuthSslErrorsDialog::on_buttonBox_clicked( QAbstractButton *button )
{
  QDialogButtonBox::StandardButton btnenum( buttonBox->standardButton( button ) );
  switch ( btnenum )
  {
    case QDialogButtonBox::Ignore:
      QgsAuthManager::instance()->updateIgnoredSslErrorsCache(
        QString( "%1:%2" ).arg( mDigest, mHostPort ),
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
}

void QgsAuthSslErrorsDialog::populateErrorsList()
{
  QStringList errs;
  errs.reserve( mSslErrors.size() );
  Q_FOREACH ( const QSslError &err, mSslErrors )
  {
    errs <<  QString( "* %1: %2" )
    .arg( QgsAuthCertUtils::sslErrorEnumString( err.error() ),
          err.errorString() );
  }
  teSslErrors->setPlainText( errs.join( "\n" ) );
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

void QgsAuthSslErrorsDialog::on_btnChainInfo_clicked()
{
  showCertificateChainInfo();
}

void QgsAuthSslErrorsDialog::on_btnChainCAs_clicked()
{
  showCertificateChainCAsInfo();
}

void QgsAuthSslErrorsDialog::on_grpbxSslErrors_collapsedStateChanged( bool collapsed )
{
  if ( !collapsed && QgsAuthManager::instance()->isDisabled() )
  {
    btnChainInfo->setVisible( false );
    btnChainCAs->setVisible( false );
  }
}
