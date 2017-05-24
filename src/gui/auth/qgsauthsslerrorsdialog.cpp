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

#include <Qt>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QFont>
#include <QColor>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>
#include <QTimer>

#include "qgsauthmanager.h"
#include "qgsauthcertutils.h"
#include "qgsauthguiutils.h"
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
    , mUrl( reply->url() )
{
  if ( mDigest.isEmpty() )
  {
    mDigest = QgsAuthCertUtils::shaHexForCert( mSslConfiguration.peerCertificate() );
  }
  if ( mHostPort.isEmpty() )
  {
    mHostPort = QString( "%1:%2" )
                .arg( mUrl.host() )
                .arg( mUrl.port() != -1 ? mUrl.port() : 443 )
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

#ifdef Q_OS_WIN
    // in case of a specific set of errors
    connect( this, SIGNAL( sslErrorsCAtoImport( QList<QSslError>& ) ),
             this, SLOT( widgetAllowAddCAsToKeystore( QList<QSslError>& ) ) );

    btnAddCainCAs->setVisible( true );
    QList<QSslError> allowedCaErrors;
    Q_FOREACH ( const QSslError &err, mSslErrors )
    {
      switch ( err.error() )
      {
        case QSslError::UnableToGetLocalIssuerCertificate:
        case QSslError::CertificateUntrusted:
          allowedCaErrors.append( err );
          break;
        default:
          break;
      }
    }
    emit sslErrorsCAtoImport( allowedCaErrors );
#else
    btnAddCainCAs->setVisible( false );
    teSslErrorsMessages->setVisible( false );
#endif // Q_OS_WIN
  }
  else
  {
    btnAddCainCAs->setVisible( false );
    teSslErrorsMessages->setVisible( false );
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

#ifdef Q_OS_WIN
void QgsAuthSslErrorsDialog::widgetAllowAddCAsToKeystore( QList<QSslError> &errors )
{
  if ( errors.isEmpty() )
  {
    btnAddCainCAs->setEnabled( false );
    btnAddCainCAs->setVisible( false );
    teSslErrorsMessages->setVisible( false );
  }
  else
  {
    btnAddCainCAs->setEnabled( true );
    btnAddCainCAs->setVisible( true );

    // set message related to the error
    teSslErrorsMessages->setVisible( true );
    QString msg = QObject::tr( "<html>"
                               "<p align='center'><strong>Certificate Authority unrecognised</strong></p>"
                               "<br>To import CAs into Windows keystore, click <strong>Open</strong> button below or browse:"
                               "<br><strong>%1</strong><br>"
                               "in a Kyestore API capable browser (no Firefox)"
                               "<br>The <strong>Open</strong> button will open the link with the default browser"
                               "<br><br>After loaded the web page, <strong>!!! restart QGIS !!!</strong>"
                               "</html>" ).arg( QString( mUrl.toString() ) );
    teSslErrorsMessages->setHtml( msg );

    QTimer::singleShot( 100, this, SLOT( blinkImportCAsButton() ) );
  }
}

void QgsAuthSslErrorsDialog::blinkImportCAsButton()
{
  static bool low;
  QString lowStyleSheet = QString( "{color: %1;}" ).arg( QColor( 0, 0, 0 ).name() );
  QString highStyleSheet = QgsAuthGuiUtils::redTextStyleSheet();

  QString styleSheet;
  if ( low )
    styleSheet = lowStyleSheet;
  else
    styleSheet = highStyleSheet;
  btnAddCainCAs->setStyleSheet( styleSheet );

  low = !low;
  QTimer::singleShot( 500, this, SLOT( blinkImportCAsButton() ) );
}

void QgsAuthSslErrorsDialog::on_btnAddCainCAs_clicked()
{
  if ( !QDesktopServices::openUrl( mUrl ) )
  {
    QgsDebugMsg( QObject::tr( "Cannot open default browser to the linK: %1" ).arg( QString( mUrl.toString() ) ) );
  }
  else
  {
    // trigger CA certs rebuild whaiting a time to allow external browser web page load
    // added two trigger just to be have cache reloaded in case browser lod
    // web page faster. A second timer in case browser takes time start.
    // BTW cache reload is affected by this issue: https://hub.qgis.org/issues/15687
    // so generally speaking rebuildCache could not be useful. I leave reload because
    // ssl CA issue can be solved later (seems an openssl problem) leaving restart unecessary
    QTimer::singleShot( 3000, QgsAuthManager::instance(), SLOT( rebuildSslCaches() ) );
    QTimer::singleShot( 7000, QgsAuthManager::instance(), SLOT( rebuildSslCaches() ) );

    // NOTE: a reply.close()/.abort() could be necessary to garantee that new SSL handshake
    // could accept new CA list. Btw I preferred to avoid reply manipulation in GUI leaving
    // the responsible to who's created the request. These are the reasons:
    // 1) avoid side effects when a different workflow is necessary when receiving sslError
    // 2) GUI wouldn't manage sessions but only get user instruction how to manipulate the session

    // close current GUI
    reject();
  }
}
#endif // Q_OS_WIN

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
