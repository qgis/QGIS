/***************************************************************************
    qgsauthpkipathsedit.cpp
    ---------------------
    begin                : September 1, 2015
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

#include "qgsauthpkipathsedit.h"
#include "ui_qgsauthpkipathsedit.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QSslCertificate>
#include <QSslKey>

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsauthguiutils.h"
#include "qgslogger.h"


QgsAuthPkiPathsEdit::QgsAuthPkiPathsEdit( QWidget *parent )
    : QgsAuthMethodEdit( parent )
    , mValid( 0 )
{
  setupUi( this );
}

QgsAuthPkiPathsEdit::~QgsAuthPkiPathsEdit()
{
}

bool QgsAuthPkiPathsEdit::validateConfig()
{
  // required components
  QString certpath( lePkiPathsCert->text() );
  QString keypath( lePkiPathsKey->text() );

  bool certfound = QFile::exists( certpath );
  bool keyfound = QFile::exists( keypath );

  QgsAuthGuiUtils::fileFound( certpath.isEmpty() || certfound, lePkiPathsCert );
  QgsAuthGuiUtils::fileFound( keypath.isEmpty() || keyfound, lePkiPathsKey );

  if ( !certfound || !keyfound )
  {
    writePkiMessage( lePkiPathsMsg, tr( "Missing components" ), Invalid );
    return validityChange( false );
  }

  // check for issue date validity, then notify status
  QSslCertificate cert;
  QFile file( certpath );
  QFileInfo fileinfo( file );
  QString ext( fileinfo.fileName().remove( fileinfo.completeBaseName() ).toLower() );
  if ( ext.isEmpty() )
  {
    writePkiMessage( lePkiPathsMsg, tr( "Certificate file has no extension" ), Invalid );
    return validityChange( false );
  }

  QFile::OpenMode openflags( QIODevice::ReadOnly );
  QSsl::EncodingFormat encformat( QSsl::Der );
  if ( ext == ".pem" )
  {
    openflags |= QIODevice::Text;
    encformat = QSsl::Pem;
  }

  if ( file.open( openflags ) )
  {
    cert = QSslCertificate( file.readAll(), encformat );
    file.close();
  }
  else
  {
    writePkiMessage( lePkiPathsMsg, tr( "Failed to read certificate file" ), Invalid );
    return validityChange( false );
  }

  if ( cert.isNull() )
  {
    writePkiMessage( lePkiPathsMsg, tr( "Failed to load certificate from file" ), Invalid );
    return validityChange( false );
  }

  bool certvalid = cert.isValid();
  QDateTime startdate( cert.effectiveDate() );
  QDateTime enddate( cert.expiryDate() );

  writePkiMessage( lePkiPathsMsg,
                   tr( "%1 thru %2" ).arg( startdate.toString(), enddate.toString() ),
                   ( certvalid ? Valid : Invalid ) );

  return validityChange( certvalid );
}

QgsStringMap QgsAuthPkiPathsEdit::configMap() const
{
  QgsStringMap config;
  config.insert( "certpath", lePkiPathsCert->text() );
  config.insert( "keypath", lePkiPathsKey->text() );
  config.insert( "keypass", lePkiPathsKeyPass->text() );

  return config;
}

void QgsAuthPkiPathsEdit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;
  lePkiPathsCert->setText( configmap.value( "certpath" ) );
  lePkiPathsKey->setText( configmap.value( "keypath" ) );
  lePkiPathsKeyPass->setText( configmap.value( "keypass" ) );

  validateConfig();
}

void QgsAuthPkiPathsEdit::resetConfig()
{
  loadConfig( mConfigMap );
}

void QgsAuthPkiPathsEdit::clearConfig()
{
  clearPkiPathsCertPath();
  clearPkiPathsKeyPath();
  clearPkiPathsKeyPass();

  clearPkiMessage( lePkiPathsMsg );
  validateConfig();
}

void QgsAuthPkiPathsEdit::clearPkiMessage( QLineEdit *lineedit )
{
  lineedit->clear();
  lineedit->setStyleSheet( "" );
}

void QgsAuthPkiPathsEdit::writePkiMessage( QLineEdit *lineedit, const QString &msg, Validity valid )
{
  QString ss;
  QString txt( msg );
  switch ( valid )
  {
    case Valid:
      ss = QgsAuthGuiUtils::greenTextStyleSheet( "QLineEdit" );
      txt = tr( "Valid: %1" ).arg( msg );
      break;
    case Invalid:
      ss = QgsAuthGuiUtils::redTextStyleSheet( "QLineEdit" );
      txt = tr( "Invalid: %1" ).arg( msg );
      break;
    case Unknown:
      ss = "";
      break;
    default:
      ss = "";
  }
  lineedit->setStyleSheet( ss );
  lineedit->setText( txt );
  lineedit->setCursorPosition( 0 );
}

void QgsAuthPkiPathsEdit::clearPkiPathsCertPath()
{
  lePkiPathsCert->clear();
  lePkiPathsCert->setStyleSheet( "" );
}

void QgsAuthPkiPathsEdit::clearPkiPathsKeyPath()
{
  lePkiPathsKey->clear();
  lePkiPathsKey->setStyleSheet( "" );
}


void QgsAuthPkiPathsEdit::clearPkiPathsKeyPass()
{
  lePkiPathsKeyPass->clear();
  lePkiPathsKeyPass->setStyleSheet( "" );
  chkPkiPathsPassShow->setChecked( false );
}

void QgsAuthPkiPathsEdit::on_chkPkiPathsPassShow_stateChanged( int state )
{
  lePkiPathsKeyPass->setEchoMode(( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}

void QgsAuthPkiPathsEdit::on_btnPkiPathsCert_clicked()
{
  const QString& fn = QgsAuthGuiUtils::getOpenFileName( this, tr( "Open Client Certificate File" ),
                      tr( "PEM (*.pem);;DER (*.der)" ) );
  if ( !fn.isEmpty() )
  {
    lePkiPathsCert->setText( fn );
    validateConfig();
  }
}

void QgsAuthPkiPathsEdit::on_btnPkiPathsKey_clicked()
{
  const QString& fn = QgsAuthGuiUtils::getOpenFileName( this, tr( "Open Private Key File" ),
                      tr( "PEM (*.pem);;DER (*.der)" ) );
  if ( !fn.isEmpty() )
  {
    lePkiPathsKey->setText( fn );
    validateConfig();
  }
}

bool QgsAuthPkiPathsEdit::validityChange( bool curvalid )
{
  if ( mValid != curvalid )
  {
    mValid = curvalid;
    emit validityChanged( curvalid );
  }
  return curvalid;
}
