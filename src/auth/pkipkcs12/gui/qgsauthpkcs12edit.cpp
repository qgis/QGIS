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
{
  setupUi( this );
  connect( lePkcs12KeyPass, &QLineEdit::textChanged, this, &QgsAuthPkcs12Edit::lePkcs12KeyPass_textChanged );
  connect( chkPkcs12PassShow, &QCheckBox::stateChanged, this, &QgsAuthPkcs12Edit::chkPkcs12PassShow_stateChanged );
  connect( btnPkcs12Bundle, &QToolButton::clicked, this, &QgsAuthPkcs12Edit::btnPkcs12Bundle_clicked );
  connect( cbAddCas, &QCheckBox::stateChanged, this, [ = ]( int state ) {  cbAddRootCa->setEnabled( state == Qt::Checked ); } );
  lblCas->hide();
  twCas->hide();
  cbAddCas->hide();
  cbAddRootCa->hide();
}

bool QgsAuthPkcs12Edit::validateConfig()
{
  // required components
  const QString bundlepath( lePkcs12Bundle->text() );

  const bool bundlefound = QFile::exists( bundlepath );

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
  const QCA::KeyBundle bundle( QCA::KeyBundle::fromFile( bundlepath, passarray, &res, QStringLiteral( "qca-ossl" ) ) );

  if ( res == QCA::ErrorFile )
  {
    writePkiMessage( lePkcs12Msg, tr( "Failed to read bundle file" ), Invalid );
    return validityChange( false );
  }
  else if ( res == QCA::ErrorPassphrase )
  {
    writePkiMessage( lePkcs12Msg, tr( "Incorrect bundle password" ), Invalid );
    lePkcs12KeyPass->setPlaceholderText( QStringLiteral( "Required passphrase" ) );
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
  const QCA::Certificate cert( bundle.certificateChain().primary() );
  if ( cert.isNull() )
  {
    writePkiMessage( lePkcs12Msg, tr( "Bundle client cert can not be loaded" ), Invalid );
    return validityChange( false );
  }

  // TODO: add more robust validation, including cert chain resolution
  const QDateTime startdate( cert.notValidBefore() );
  const QDateTime enddate( cert.notValidAfter() );
  const QDateTime now( QDateTime::currentDateTime() );
  const bool bundlevalid = ( now >= startdate && now <= enddate );

  writePkiMessage( lePkcs12Msg,
                   tr( "%1 thru %2" ).arg( startdate.toString(), enddate.toString() ),
                   ( bundlevalid ? Valid : Invalid ) );

  const bool showCas( bundlevalid && populateCas() );
  lblCas->setVisible( showCas );
  twCas->setVisible( showCas );
  cbAddCas->setVisible( showCas );
  cbAddRootCa->setVisible( showCas );

  return validityChange( bundlevalid );
}



QgsStringMap QgsAuthPkcs12Edit::configMap() const
{
  QgsStringMap config;
  config.insert( QStringLiteral( "bundlepath" ), lePkcs12Bundle->text() );
  config.insert( QStringLiteral( "bundlepass" ), lePkcs12KeyPass->text() );
  config.insert( QStringLiteral( "addcas" ), cbAddCas->isChecked() ? QStringLiteral( "true" ) :  QStringLiteral( "false" ) );
  config.insert( QStringLiteral( "addrootca" ), cbAddRootCa->isChecked() ? QStringLiteral( "true" ) :  QStringLiteral( "false" ) );

  return config;
}

void QgsAuthPkcs12Edit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;
  lePkcs12Bundle->setText( configmap.value( QStringLiteral( "bundlepath" ) ) );
  lePkcs12KeyPass->setText( configmap.value( QStringLiteral( "bundlepass" ) ) );
  cbAddCas->setChecked( configmap.value( QStringLiteral( "addcas" ), QStringLiteral( "false " ) ) == QLatin1String( "true" ) );
  cbAddRootCa->setChecked( configmap.value( QStringLiteral( "addrootca" ), QStringLiteral( "false " ) ) == QLatin1String( "true" ) );

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
  lineedit->setStyleSheet( QString() );
}

void QgsAuthPkcs12Edit::writePkiMessage( QLineEdit *lineedit, const QString &msg, Validity valid )
{
  QString ss;
  QString txt( msg );
  switch ( valid )
  {
    case Valid:
      ss = QgsAuthGuiUtils::greenTextStyleSheet( QStringLiteral( "QLineEdit" ) );
      txt = tr( "Valid: %1" ).arg( msg );
      break;
    case Invalid:
      ss = QgsAuthGuiUtils::redTextStyleSheet( QStringLiteral( "QLineEdit" ) );
      txt = tr( "Invalid: %1" ).arg( msg );
      break;
    case Unknown:
      break;
  }
  lineedit->setStyleSheet( ss );
  lineedit->setText( txt );
  lineedit->setCursorPosition( 0 );
}

void QgsAuthPkcs12Edit::clearPkcs12BundlePath()
{
  lePkcs12Bundle->clear();
  lePkcs12Bundle->setStyleSheet( QString() );
}

void QgsAuthPkcs12Edit::clearPkcs12BundlePass()
{
  lePkcs12KeyPass->clear();
  lePkcs12KeyPass->setStyleSheet( QString() );
  lePkcs12KeyPass->setPlaceholderText( QStringLiteral( "Optional passphrase" ) );
  chkPkcs12PassShow->setChecked( false );
}

void QgsAuthPkcs12Edit::lePkcs12KeyPass_textChanged( const QString &pass )
{
  Q_UNUSED( pass )
  validateConfig();
}

void QgsAuthPkcs12Edit::chkPkcs12PassShow_stateChanged( int state )
{
  lePkcs12KeyPass->setEchoMode( ( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}

void QgsAuthPkcs12Edit::btnPkcs12Bundle_clicked()
{
  const QString &fn = QgsAuthGuiUtils::getOpenFileName( this, tr( "Open PKCS#12 Certificate Bundle" ),
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

bool QgsAuthPkcs12Edit::populateCas()
{
  twCas->clear();
  const QList<QSslCertificate> cas( QgsAuthCertUtils::pkcs12BundleCas( lePkcs12Bundle->text(), lePkcs12KeyPass->text() ) );
  if ( cas.isEmpty() )
  {
    return false;
  }

  QTreeWidgetItem *prevItem( nullptr );
  QList<QSslCertificate>::const_iterator it( cas.constEnd() );
  while ( it != cas.constBegin() )
  {
    --it;
    const QSslCertificate cert = static_cast<QSslCertificate>( *it );
    QTreeWidgetItem *item;

    if ( prevItem && cert.issuerInfo( QSslCertificate::SubjectInfo::CommonName ).contains( prevItem->text( 0 ) ) )
    {
      item = new QTreeWidgetItem( QStringList( cert.subjectInfo( QSslCertificate::SubjectInfo::CommonName ) ) );
      prevItem->addChild( item );
    }
    else
    {
      item = new QTreeWidgetItem( twCas, QStringList( cert.subjectInfo( QSslCertificate::SubjectInfo::CommonName ) ) );
    }
    item->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/mIconCertificate.svg" ) ) );
    item->setToolTip( 0, tr( "<ul><li>Serial #: %1</li><li>Expiry date: %2</li></ul>" ).arg( cert.serialNumber( ), cert.expiryDate().toString( Qt::TextDate ) ) );
    prevItem = item;
  }
  twCas->expandAll();

  return true;
}
