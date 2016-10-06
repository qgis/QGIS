/***************************************************************************
    qgsauthpkcs12edit.cpp
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

#include "qgsauthpkcs12edit.h"
#include "ui_qgsauthpkcs12edit.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QSslCertificate>
#include <QSslKey>

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsauthguiutils.h"
#include "qgslogger.h"


QgsAuthPkcs12Edit::QgsAuthPkcs12Edit( QWidget *parent )
    : QgsAuthMethodEdit( parent )
    , mValid( 0 )
{
  setupUi( this );
}

QgsAuthPkcs12Edit::~QgsAuthPkcs12Edit()
{
}

bool QgsAuthPkcs12Edit::validateConfig()
{
  // required components
  QString bundlepath( lePkcs12Bundle->text() );

  bool bundlefound = QFile::exists( bundlepath );

  QgsAuthGuiUtils::fileFound( bundlepath.isEmpty() || bundlefound, lePkcs12Bundle );

  if ( !bundlefound )
  {
    writePkiMessage( lePkcs12Msg, tr( "Missing components" ), Invalid );
    return validityChange( false );
  }

  if ( !QCA::isSupported( "pkcs12" ) )
  {
    writePkiMessage( lePkcs12Msg, tr( "QCA library has no PKCS#12 support" ), Invalid );
    return validityChange( false );
  }

  // load the bundle
  QCA::SecureArray passarray;
  if ( !lePkcs12KeyPass->text().isEmpty() )
    passarray = QCA::SecureArray( lePkcs12KeyPass->text().toUtf8() );

  QCA::ConvertResult res;
  QCA::KeyBundle bundle( QCA::KeyBundle::fromFile( bundlepath, passarray, &res, QString( "qca-ossl" ) ) );

  if ( res == QCA::ErrorFile )
  {
    writePkiMessage( lePkcs12Msg, tr( "Failed to read bundle file" ), Invalid );
    return validityChange( false );
  }
  else if ( res == QCA::ErrorPassphrase )
  {
    writePkiMessage( lePkcs12Msg, tr( "Incorrect bundle password" ), Invalid );
    lePkcs12KeyPass->setPlaceholderText( QString( "Required passphrase" ) );
    return validityChange( false );
  }
  else if ( res == QCA::ErrorDecode )
  {
    writePkiMessage( lePkcs12Msg, tr( "Failed to decode (try entering password)" ), Invalid );
    return validityChange( false );
  }

  if ( bundle.isNull() )
  {
    writePkiMessage( lePkcs12Msg, tr( "Bundle empty or can not be loaded" ), Invalid );
    return validityChange( false );
  }

  // check for primary cert and that it is valid
  QCA::Certificate cert( bundle.certificateChain().primary() );
  if ( cert.isNull() )
  {
    writePkiMessage( lePkcs12Msg, tr( "Bundle client cert can not be loaded" ), Invalid );
    return validityChange( false );
  }

  // TODO: add more robust validation, including cert chain resolution
  QDateTime startdate( cert.notValidBefore() );
  QDateTime enddate( cert.notValidAfter() );
  QDateTime now( QDateTime::currentDateTime() );
  bool bundlevalid = ( now >= startdate && now <= enddate );

  writePkiMessage( lePkcs12Msg,
                   tr( "%1 thru %2" ).arg( startdate.toString(), enddate.toString() ),
                   ( bundlevalid ? Valid : Invalid ) );

  return validityChange( bundlevalid );
}

QgsStringMap QgsAuthPkcs12Edit::configMap() const
{
  QgsStringMap config;
  config.insert( "bundlepath", lePkcs12Bundle->text() );
  config.insert( "bundlepass", lePkcs12KeyPass->text() );

  return config;
}

void QgsAuthPkcs12Edit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;
  lePkcs12Bundle->setText( configmap.value( "bundlepath" ) );
  lePkcs12KeyPass->setText( configmap.value( "bundlepass" ) );

  validateConfig();
}

void QgsAuthPkcs12Edit::resetConfig()
{
  loadConfig( mConfigMap );
}

void QgsAuthPkcs12Edit::clearConfig()
{
  clearPkcs12BundlePath();
  clearPkcs12BundlePass();

  clearPkiMessage( lePkcs12Msg );
  validateConfig();
}

void QgsAuthPkcs12Edit::clearPkiMessage( QLineEdit *lineedit )
{
  lineedit->clear();
  lineedit->setStyleSheet( "" );
}

void QgsAuthPkcs12Edit::writePkiMessage( QLineEdit *lineedit, const QString &msg, Validity valid )
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

void QgsAuthPkcs12Edit::clearPkcs12BundlePath()
{
  lePkcs12Bundle->clear();
  lePkcs12Bundle->setStyleSheet( "" );
}

void QgsAuthPkcs12Edit::clearPkcs12BundlePass()
{
  lePkcs12KeyPass->clear();
  lePkcs12KeyPass->setStyleSheet( "" );
  lePkcs12KeyPass->setPlaceholderText( QString( "Optional passphrase" ) );
  chkPkcs12PassShow->setChecked( false );
}

void QgsAuthPkcs12Edit::on_lePkcs12KeyPass_textChanged( const QString &pass )
{
  Q_UNUSED( pass );
  validateConfig();
}

void QgsAuthPkcs12Edit::on_chkPkcs12PassShow_stateChanged( int state )
{
  lePkcs12KeyPass->setEchoMode(( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}

void QgsAuthPkcs12Edit::on_btnPkcs12Bundle_clicked()
{
  const QString& fn = QgsAuthGuiUtils::getOpenFileName( this, tr( "Open PKCS#12 Certificate Bundle" ),
                      tr( "PKCS#12 (*.p12 *.pfx)" ) );
  if ( !fn.isEmpty() )
  {
    lePkcs12Bundle->setText( fn );
    validateConfig();
  }
}

bool QgsAuthPkcs12Edit::validityChange( bool curvalid )
{
  if ( mValid != curvalid )
  {
    mValid = curvalid;
    emit validityChanged( curvalid );
  }
  return curvalid;
}
