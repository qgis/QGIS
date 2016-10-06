/***************************************************************************
    qgsauthsslimportdialog.cpp
    ---------------------
    begin                : May 17, 2015
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

/****************************************************************************
**
** Portions of this code were derived from the following...
**
** qt-everywhere-opensource-src-4.8.6/examples/network/
**   securesocketclient/certificateinfo.h (and .cpp)
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "qgsauthcertificateinfo.h"
#include "qgsauthsslimportdialog.h"
#include "qgsauthsslconfigwidget.h"
#include "ui_qgsauthsslimporterrors.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QPushButton>
#include <QScrollBar>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QSslCipher>

#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"


QgsAuthSslImportDialog::QgsAuthSslImportDialog( QWidget *parent )
    : QDialog( parent )
    , mSocket( nullptr )
    , mExecErrorsDialog( false )
    , mTimer( nullptr )
    , mSslErrors( QList<QSslError>() )
    , mTrustedCAs( QList<QSslCertificate>() )
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
    QStyle *style = QApplication::style();
    lblWarningIcon->setPixmap( style->standardIcon( QStyle::SP_MessageBoxWarning ).pixmap( 48, 48 ) );
    lblWarningIcon->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    closeButton()->setDefault( false );
    saveButton()->setEnabled( false );

    leServer->setSelection( 0, leServer->text().size() );
    pteSessionStatus->setReadOnly( true );
    spinbxTimeout->setValue( 15 );

    grpbxServer->setCollapsed( false );
    radioServerImport->setChecked( true );
    frameServerImport->setEnabled( true );
    radioFileImport->setChecked( false );
    frameFileImport->setEnabled( false );

    connect( radioServerImport, SIGNAL( toggled( bool ) ),
             this, SLOT( radioServerImportToggled( bool ) ) );
    connect( radioFileImport, SIGNAL( toggled( bool ) ),
             this, SLOT( radioFileImportToggled( bool ) ) );

    connect( leServer, SIGNAL( textChanged( QString ) ),
             this, SLOT( updateEnabledState() ) );
    connect( btnConnect, SIGNAL( clicked() ),
             this, SLOT( secureConnect() ) );
    connect( leServer, SIGNAL( returnPressed() ),
             btnConnect, SLOT( click() ) );

    connect( buttonBox, SIGNAL( accepted() ),
             this, SLOT( accept() ) );
    connect( buttonBox, SIGNAL( rejected() ),
             this, SLOT( reject() ) );

    connect( wdgtSslConfig, SIGNAL( readyToSaveChanged( bool ) ),
             this, SLOT( widgetReadyToSaveChanged( bool ) ) );
    wdgtSslConfig->setEnabled( false );

    mTrustedCAs = QgsAuthManager::instance()->getTrustedCaCertsCache();
  }
}

QgsAuthSslImportDialog::~QgsAuthSslImportDialog()
{
}

void QgsAuthSslImportDialog::accept()
{
  wdgtSslConfig->saveSslCertConfig();
  QDialog::accept();
}

void QgsAuthSslImportDialog::updateEnabledState()
{
  leServer->setStyleSheet( "" );

  bool unconnected = !mSocket || mSocket->state() == QAbstractSocket::UnconnectedState;

  leServer->setReadOnly( !unconnected );
  spinbxPort->setReadOnly( !unconnected );
  spinbxTimeout->setReadOnly( !unconnected );

  leServer->setFocusPolicy( unconnected ? Qt::StrongFocus : Qt::NoFocus );
  btnConnect->setEnabled( unconnected && !leServer->text().isEmpty() );

  bool connected = mSocket && mSocket->state() == QAbstractSocket::ConnectedState;
  if ( connected && !mSocket->peerName().isEmpty() )
  {
    appendString( tr( "Connected to %1:%2" ).arg( mSocket->peerName() ).arg( mSocket->peerPort() ) );
  }
}

void QgsAuthSslImportDialog::secureConnect()
{
  if ( leServer->text().isEmpty() )
  {
    return;
  }

  leServer->setStyleSheet( "" );
  clearStatusCertificateConfig();

  if ( !mSocket )
  {
    mSocket = new QSslSocket( this );
    connect( mSocket, SIGNAL( stateChanged( QAbstractSocket::SocketState ) ),
             this, SLOT( socketStateChanged( QAbstractSocket::SocketState ) ) );
    connect( mSocket, SIGNAL( connected() ),
             this, SLOT( socketConnected() ) );
    connect( mSocket, SIGNAL( disconnected() ),
             this, SLOT( socketDisconnected() ) );
    connect( mSocket, SIGNAL( encrypted() ),
             this, SLOT( socketEncrypted() ) );
    connect( mSocket, SIGNAL( error( QAbstractSocket::SocketError ) ),
             this, SLOT( socketError( QAbstractSocket::SocketError ) ) );
    connect( mSocket, SIGNAL( sslErrors( QList<QSslError> ) ),
             this, SLOT( sslErrors( QList<QSslError> ) ) );
    connect( mSocket, SIGNAL( readyRead() ),
             this, SLOT( socketReadyRead() ) );
  }

  mSocket->setCaCertificates( mTrustedCAs );

  if ( !mTimer )
  {
    mTimer = new QTimer( this );
    connect( mTimer, SIGNAL( timeout() ), this, SLOT( destroySocket() ) );
  }
  mTimer->start( spinbxTimeout->value() * 1000 );

  mSocket->connectToHost( leServer->text(), spinbxPort->value() );
  updateEnabledState();
}

void QgsAuthSslImportDialog::socketStateChanged( QAbstractSocket::SocketState state )
{
  if ( mExecErrorsDialog )
  {
    return;
  }

  updateEnabledState();
  if ( state == QAbstractSocket::UnconnectedState )
  {
    leServer->setFocus();
    destroySocket();
  }
}

void QgsAuthSslImportDialog::socketConnected()
{
  appendString( tr( "Socket CONNECTED" ) );
  mSocket->startClientEncryption();
}

void QgsAuthSslImportDialog::socketDisconnected()
{
  appendString( tr( "Socket DISCONNECTED" ) );
}

void QgsAuthSslImportDialog::socketEncrypted()
{
  QgsDebugMsg( "socketEncrypted entered" );
  if ( !mSocket )
    return;  // might have disconnected already

  appendString( tr( "Socket ENCRYPTED" ) );

  appendString( QString( "%1: %2" ).arg( tr( "Protocol" ),
                                         QgsAuthCertUtils::getSslProtocolName( mSocket->protocol() ) ) );

  QSslCipher ciph = mSocket->sessionCipher();
  QString cipher = QString( "%1: %2, %3 (%4/%5)" )
                   .arg( tr( "Session cipher" ), ciph.authenticationMethod(), ciph.name() )
                   .arg( ciph.usedBits() ).arg( ciph.supportedBits() );
  appendString( cipher );



  wdgtSslConfig->setEnabled( true );
  QString hostport( QString( "%1:%2" ).arg( mSocket->peerName() ).arg( mSocket->peerPort() ) );
  wdgtSslConfig->setSslCertificate( mSocket->peerCertificate(), hostport.trimmed() );
  if ( !mSslErrors.isEmpty() )
  {
    wdgtSslConfig->appendSslIgnoreErrors( mSslErrors );
    mSslErrors.clear();
  }

//  checkCanSave();

  // must come after last state change, or gets reverted
  leServer->setStyleSheet( QgsAuthGuiUtils::greenTextStyleSheet() );

  destroySocket();
}

void QgsAuthSslImportDialog::socketError( QAbstractSocket::SocketError err )
{
  Q_UNUSED( err );
  if ( mSocket )
  {
    appendString( QString( "%1: %2" ).arg( tr( "Socket ERROR" ), mSocket->errorString() ) );
  }
}

void QgsAuthSslImportDialog::socketReadyRead()
{
  appendString( QString::fromUtf8( mSocket->readAll() ) );
}

void QgsAuthSslImportDialog::destroySocket()
{
  if ( !mSocket )
  {
    return;
  }
  if ( !mSocket->isEncrypted() )
  {
    appendString( tr( "Socket unavailable or not encrypted" ) );
  }
  mSocket->disconnectFromHost();
  mSocket->deleteLater();
  mSocket = nullptr;
}

void QgsAuthSslImportDialog::sslErrors( const QList<QSslError> &errors )
{
  if ( !mTimer->isActive() )
  {
    return;
  }
  mTimer->stop();

  QDialog errorDialog( this );
  Ui_SslErrors ui;
  ui.setupUi( &errorDialog );
  connect( ui.certificateChainButton, SIGNAL( clicked() ),
           this, SLOT( showCertificateInfo() ) );

  Q_FOREACH ( const QSslError &error, errors )
  {
    ui.sslErrorList->addItem( error.errorString() );
  }

  mExecErrorsDialog = true;
  if ( errorDialog.exec() == QDialog::Accepted )
  {
    mSocket->ignoreSslErrors();
    mSslErrors = errors;
  }
  mExecErrorsDialog = false;

  mTimer->start();

  // did the socket state change?
  if ( mSocket->state() != QAbstractSocket::ConnectedState )
    socketStateChanged( mSocket->state() );
}

void QgsAuthSslImportDialog::showCertificateInfo()
{
  QList<QSslCertificate> peerchain( mSocket->peerCertificateChain() );

  if ( !peerchain.isEmpty() )
  {
    QSslCertificate cert = peerchain.takeFirst();
    if ( !cert.isNull() )
    {
      QgsAuthCertInfoDialog *info = new QgsAuthCertInfoDialog( cert, false, this, peerchain );
      info->exec();
      info->deleteLater();
    }
  }
}

void QgsAuthSslImportDialog::widgetReadyToSaveChanged( bool cansave )
{
  saveButton()->setEnabled( cansave );
}

void QgsAuthSslImportDialog::checkCanSave()
{
  saveButton()->setEnabled( wdgtSslConfig->readyToSave() );
  saveButton()->setDefault( false );
  closeButton()->setDefault( false );
}

void QgsAuthSslImportDialog::radioServerImportToggled( bool checked )
{
  frameServerImport->setEnabled( checked );
  clearStatusCertificateConfig();
}

void QgsAuthSslImportDialog::radioFileImportToggled( bool checked )
{
  frameFileImport->setEnabled( checked );
  clearStatusCertificateConfig();
}

void QgsAuthSslImportDialog::on_btnCertPath_clicked()
{
  const QString& fn = getOpenFileName( tr( "Open Server Certificate File" ),  tr( "PEM (*.pem);;DER (*.der)" ) );
  if ( !fn.isEmpty() )
  {
    leCertPath->setText( fn );
    loadCertFromFile();
  }
}

void QgsAuthSslImportDialog::clearCertificateConfig()
{
  wdgtSslConfig->resetSslCertConfig();
  wdgtSslConfig->setEnabled( false );
}

void QgsAuthSslImportDialog::clearStatusCertificateConfig()
{
  mSslErrors.clear();
  pteSessionStatus->clear();
  saveButton()->setEnabled( false );
  clearCertificateConfig();
}

void QgsAuthSslImportDialog::loadCertFromFile()
{
  clearStatusCertificateConfig();
  QList<QSslCertificate> certs( QgsAuthCertUtils::certsFromFile( leCertPath->text() ) );

  if ( certs.isEmpty() )
  {
    appendString( tr( "Could not load any certs from file" ) );
    return;
  }

  QSslCertificate cert( certs.first() );
  if ( cert.isNull() )
  {
    appendString( tr( "Could not load server cert from file" ) );
    return;
  }

  if ( !QgsAuthCertUtils::certificateIsSslServer( cert ) )
  {
    appendString( tr( "Certificate does not appear for be for an SSL server. "
                      "You can still add a configuration, if you know it is the correct certificate." ) );
  }

  wdgtSslConfig->setEnabled( true );
  wdgtSslConfig->setSslHost( "" );
  wdgtSslConfig->setSslCertificate( cert );
  if ( !mSslErrors.isEmpty() )
  {
    wdgtSslConfig->appendSslIgnoreErrors( mSslErrors );
    mSslErrors.clear();
  }
//  checkCanSave();
}

void QgsAuthSslImportDialog::appendString( const QString &line )
{
  QTextCursor cursor( pteSessionStatus->textCursor() );
  cursor.movePosition( QTextCursor::End );
  cursor.insertText( line + '\n' );
//  pteSessionStatus->verticalScrollBar()->setValue( pteSessionStatus->verticalScrollBar()->maximum() );
}

QPushButton *QgsAuthSslImportDialog::saveButton()
{
  return buttonBox->button( QDialogButtonBox::Save );
}

QPushButton *QgsAuthSslImportDialog::closeButton()
{
  return buttonBox->button( QDialogButtonBox::Close );
}

QString QgsAuthSslImportDialog::getOpenFileName( const QString &title, const QString &extfilter )
{
  QSettings settings;
  QString recentdir = settings.value( "UI/lastAuthImportSslOpenFileDir", QDir::homePath() ).toString();
  QString f = QFileDialog::getOpenFileName( this, title, recentdir, extfilter );

  // return dialog focus on Mac
  this->raise();
  this->activateWindow();

  if ( !f.isEmpty() )
  {
    settings.setValue( "UI/lastAuthImportSslOpenFileDir", QFileInfo( f ).absoluteDir().path() );
  }
  return f;
}
