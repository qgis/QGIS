/***************************************************************************
    qgsauthimportcertdialog.cpp
    ---------------------
    begin                : April 30, 2015
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

#include "qgsauthimportcertdialog.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QPushButton>
#include <QSettings>

#include <QtCrypto>

#include "qgsauthcertutils.h"
#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"


QgsAuthImportCertDialog::QgsAuthImportCertDialog( QWidget *parent ,
    QgsAuthImportCertDialog::CertFilter filter,
    QgsAuthImportCertDialog::CertInput input )
    : QDialog( parent )
    , mFilter( filter )
    , mInput( input )
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

    connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

    connect( teCertText, SIGNAL( textChanged() ), this, SLOT( validateCertificates() ) );

    connect( radioImportFile, SIGNAL( toggled( bool ) ), this, SLOT( updateGui() ) );
    connect( radioImportText, SIGNAL( toggled( bool ) ), this, SLOT( updateGui() ) );

    // hide unused widgets
    if ( mInput == FileInput )
    {
      radioImportText->setHidden( true );
      teCertText->setHidden( true );
    }
    else if ( mInput == TextInput )
    {
      radioImportFile->setHidden( true );
      frameImportFile->setHidden( true );
    }

    radioImportFile->setChecked( true );
    updateGui();

    if ( mFilter == CaFilter )
    {
      grpbxImportCert->setTitle( tr( "Import Certificate Authorities" ) );
    }

    okButton()->setText( tr( "Import" ) );
    okButton()->setEnabled( false );
    teValidation->setFocus();
  }
}

QgsAuthImportCertDialog::~QgsAuthImportCertDialog()
{
}

const QList<QSslCertificate> QgsAuthImportCertDialog::certificatesToImport()
{
  if ( mDisabled )
  {
    return QList<QSslCertificate>();
  }
  return mCerts;
}

const QString QgsAuthImportCertDialog::certFileToImport()
{
  if ( mDisabled )
  {
    return QString();
  }
  if ( !radioImportFile->isChecked() )
    return QString();

  return leImportFile->text();
}

const QString QgsAuthImportCertDialog::certTextToImport()
{
  if ( mDisabled )
  {
    return QString();
  }
  if ( !radioImportText->isChecked() )
    return QString();

  return teCertText->toPlainText().trimmed();
}

bool QgsAuthImportCertDialog::allowInvalidCerts()
{
  if ( mDisabled )
  {
    return false;
  }
  return chkAllowInvalid->isChecked();
}

QgsAuthCertUtils::CertTrustPolicy QgsAuthImportCertDialog::certTrustPolicy()
{
  if ( mDisabled )
  {
    return QgsAuthCertUtils::DefaultTrust;
  }
  return cmbbxTrust->trustPolicy();
}

void QgsAuthImportCertDialog::updateGui()
{
  frameImportFile->setEnabled( radioImportFile->isChecked() );
  teCertText->setEnabled( radioImportText->isChecked() );
  validateCertificates();
}

void QgsAuthImportCertDialog::validateCertificates()
{
  mCerts.clear();
  teValidation->clear();
  teValidation->setStyleSheet( "" );

  bool valid = false;
  QList<QSslCertificate> certs;
  QList<QSslCertificate> nixcerts;
  int validcerts = 0;
  bool allowinvalid = chkAllowInvalid->isChecked();
  bool filterCAs = ( mFilter == CaFilter );
  int cas = 0;

  if ( radioImportFile->isChecked() && !leImportFile->text().isEmpty() )
  {
    certs = QgsAuthCertUtils::certsFromFile( leImportFile->text() );
  }
  else if ( radioImportText->isChecked() && !teCertText->toPlainText().trimmed().isEmpty() )
  {
    certs = QgsAuthCertUtils::certsFromString( teCertText->toPlainText().trimmed() );
  }

  int certssize = certs.size();

  Q_FOREACH ( const QSslCertificate &cert, certs )
  {
    if ( cert.isValid() )
      ++validcerts;

    if ( filterCAs )
    {
      if ( QgsAuthCertUtils::certificateIsAuthorityOrIssuer( cert ) )
      {
        ++cas;
      }
      else
      {
        nixcerts << cert;
      }
    }
  }

  valid = ( certssize > 0
            && ( allowinvalid || certssize == validcerts )
            && ( !filterCAs || nixcerts.size() < certssize ) );

  if ( !nixcerts.isEmpty() )
  {
    Q_FOREACH ( const QSslCertificate &nixcert, nixcerts )
    {
      certs.removeOne( nixcert );
    }
  }

  if ( valid )
    mCerts = certs;

  if ( certssize > 0 )
  {
    teValidation->setStyleSheet(
      valid ? QgsAuthGuiUtils::greenTextStyleSheet( "QTextEdit" )
      : QgsAuthGuiUtils::redTextStyleSheet( "QTextEdit" ) );
  }

  QString msg = tr( "Certificates found: %1\n"
                    "Certificates valid: %2" ).arg( certssize ).arg( validcerts );

  if ( filterCAs )
  {
    msg += tr( "\nAuthorities/Issuers: %1%2" ).arg( cas )
           .arg( !nixcerts.isEmpty() && nixcerts.size() < certssize ? " (others not imported)" : "" );
  }

  teValidation->setText( msg );

  okButton()->setEnabled( valid );
}

void QgsAuthImportCertDialog::on_btnImportFile_clicked()
{
  const QString& fn = getOpenFileName( tr( "Open Certificate File" ),  tr( "PEM (*.pem);;DER (*.der)" ) );
  if ( !fn.isEmpty() )
  {
    leImportFile->setText( fn );
  }
  validateCertificates();
}

void QgsAuthImportCertDialog::on_chkAllowInvalid_toggled( bool checked )
{
  Q_UNUSED( checked );
  validateCertificates();
}

QString QgsAuthImportCertDialog::getOpenFileName( const QString &title, const QString &extfilter )
{
  QSettings settings;
  QString recentdir = settings.value( "UI/lastAuthImportCertOpenFileDir", QDir::homePath() ).toString();
  QString f = QFileDialog::getOpenFileName( this, title, recentdir, extfilter );

  // return dialog focus on Mac
  this->raise();
  this->activateWindow();

  if ( !f.isEmpty() )
  {
    settings.setValue( "UI/lastAuthImportCertOpenFileDir", QFileInfo( f ).absoluteDir().path() );
  }
  return f;
}

QPushButton* QgsAuthImportCertDialog::okButton()
{
  return buttonBox->button( QDialogButtonBox::Ok );
}
