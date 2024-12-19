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

#include <QtCrypto>

#include "qgssettings.h"
#include "qgsauthcertutils.h"
#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"
#include "qgsapplication.h"


QgsAuthImportCertDialog::QgsAuthImportCertDialog( QWidget *parent,
    QgsAuthImportCertDialog::CertFilter filter,
    QgsAuthImportCertDialog::CertInput input )
  : QDialog( parent )
  , mFilter( filter )
  , mInput( input )
{
  if ( QgsApplication::authManager()->isDisabled() )
  {
    mDisabled = true;
    mAuthNotifyLayout = new QVBoxLayout;
    this->setLayout( mAuthNotifyLayout );
    mAuthNotify = new QLabel( QgsApplication::authManager()->disabledMessage(), this );
    mAuthNotifyLayout->addWidget( mAuthNotify );
  }
  else
  {
    setupUi( this );
    connect( btnImportFile, &QToolButton::clicked, this, &QgsAuthImportCertDialog::btnImportFile_clicked );
    connect( chkAllowInvalid, &QCheckBox::toggled, this, &QgsAuthImportCertDialog::chkAllowInvalid_toggled );

    connect( buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
    connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );

    connect( teCertText, &QPlainTextEdit::textChanged, this, &QgsAuthImportCertDialog::validateCertificates );

    connect( radioImportFile, &QAbstractButton::toggled, this, &QgsAuthImportCertDialog::updateGui );
    connect( radioImportText, &QAbstractButton::toggled, this, &QgsAuthImportCertDialog::updateGui );

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
  teValidation->setStyleSheet( QString() );

  bool valid = false;
  QList<QSslCertificate> certs;
  QList<QSslCertificate> nixcerts;
  int validcerts = 0;
  const bool allowinvalid = chkAllowInvalid->isChecked();
  const bool filterCAs = ( mFilter == CaFilter );
  int cas = 0;

  if ( radioImportFile->isChecked() && !leImportFile->text().isEmpty() )
  {
    certs = QgsAuthCertUtils::certsFromFile( leImportFile->text() );
  }
  else if ( radioImportText->isChecked() && !teCertText->toPlainText().trimmed().isEmpty() )
  {
    certs = QgsAuthCertUtils::certsFromString( teCertText->toPlainText().trimmed() );
  }

  const int certssize = certs.size();

  const auto constCerts = certs;
  for ( const QSslCertificate &cert : constCerts )
  {
    if ( QgsAuthCertUtils::certIsViable( cert ) )
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
    const auto constNixcerts = nixcerts;
    for ( const QSslCertificate &nixcert : constNixcerts )
    {
      certs.removeOne( nixcert );
    }
  }

  if ( valid )
    mCerts = certs;

  if ( certssize > 0 )
  {
    teValidation->setStyleSheet(
      valid ? QgsAuthGuiUtils::greenTextStyleSheet( QStringLiteral( "QTextEdit" ) )
      : QgsAuthGuiUtils::redTextStyleSheet( QStringLiteral( "QTextEdit" ) ) );
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

void QgsAuthImportCertDialog::btnImportFile_clicked()
{
  const QString &fn = getOpenFileName( tr( "Open Certificate File" ),  tr( "All files (*.*);;PEM (*.pem);;DER (*.der)" ) );
  if ( !fn.isEmpty() )
  {
    leImportFile->setText( fn );
  }
  validateCertificates();
}

void QgsAuthImportCertDialog::chkAllowInvalid_toggled( bool checked )
{
  Q_UNUSED( checked )
  validateCertificates();
}

QString QgsAuthImportCertDialog::getOpenFileName( const QString &title, const QString &extfilter )
{
  QgsSettings settings;
  const QString recentdir = settings.value( QStringLiteral( "UI/lastAuthImportCertOpenFileDir" ), QDir::homePath() ).toString();
  QString f = QFileDialog::getOpenFileName( this, title, recentdir, extfilter );

  // return dialog focus on Mac
  this->raise();
  this->activateWindow();

  if ( !f.isEmpty() )
  {
    settings.setValue( QStringLiteral( "UI/lastAuthImportCertOpenFileDir" ), QFileInfo( f ).absoluteDir().path() );
  }
  return f;
}

QPushButton *QgsAuthImportCertDialog::okButton()
{
  return buttonBox->button( QDialogButtonBox::Ok );
}
