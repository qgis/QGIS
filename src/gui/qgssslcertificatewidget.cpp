/***************************************************************************
    qgssslcertificatewidget.cpp
                             -------------------
    begin                : 2014-09-12
    copyright            : (C) 2014 by Boundless Spatial, Inc.
    web                  : http://boundlessgeo.com
    author               : Larry Shaffer
    email                : larrys at dakotacarto dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssslcertificatewidget.h"
#include "qgssslutils.h"

#include <QAction>
#include <QDateTime>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMenu>
#include <QUrl>

static QString validGreen_( const QString& selector = "*" )
{
  return QString( "%1{color: rgb(0, 170, 0);}" ).arg( selector );
}
static QString validRed_( const QString& selector = "*" )
{
  return QString( "%1{color: rgb(200, 0, 0);}" ).arg( selector );
}
//static QString validGray_( const QString& selector = "*" )
//{
//  return QString( "%1{color: rgb(175, 175, 175);}" ).arg( selector );
//}

QgsSslCertificateWidget::QgsSslCertificateWidget( QWidget *parent, const QString& accessUrl )
    : QWidget( parent )
    , mStoreType( QgsSslCertSettings::QGISStore )
    , mCertValid( false )
    , mAccessUrl( accessUrl )
    , mQgisCertsMenu( 0 )
    , mQgisKeysMenu( 0 )
    , mQgisIssuersMenu( 0 )
{
  setupUi( this );

  clearCert();

  // TODO: reactivate this button and add more validation and information on errors and cert chain
  btnValidateCert->hide();

  connect( cmbStoreType, SIGNAL( currentIndexChanged( int ) ), stckwCertStore, SLOT( setCurrentIndex( int ) ) );

  connect( btnClearCert, SIGNAL( clicked() ), this, SLOT( clearCert() ) );

  // QGIS user-local store (default store)
  connect( btnOpenStore, SIGNAL( clicked() ), this, SLOT( openQgisStoreDir() ) );
  mQgisCertsMenu = new QMenu( this );
  btnQgisCert->setMenu( mQgisCertsMenu );
  connect( mQgisCertsMenu, SIGNAL( aboutToShow() ), this, SLOT( populateQgisCertsMenu() ) );
  connect( mQgisCertsMenu, SIGNAL( triggered( QAction* ) ), this, SLOT( qgisCertsMenuTriggered( QAction* ) ) );
  mQgisKeysMenu = new QMenu( this );
  btnQgisKey->setMenu( mQgisKeysMenu );
  connect( mQgisKeysMenu, SIGNAL( aboutToShow() ), this, SLOT( populateQgisKeysMenu() ) );
  connect( mQgisKeysMenu, SIGNAL( triggered( QAction* ) ), this, SLOT( qgisKeysMenuTriggered( QAction* ) ) );
  mQgisIssuersMenu = new QMenu( this );
  btnQgisIssuer->setMenu( mQgisIssuersMenu );
  connect( mQgisIssuersMenu, SIGNAL( aboutToShow() ), this, SLOT( populateQgisIssuersMenu() ) );
  connect( mQgisIssuersMenu, SIGNAL( triggered( QAction* ) ), this, SLOT( qgisIssuersMenuTriggered( QAction* ) ) );
}

QgsSslCertificateWidget::~QgsSslCertificateWidget()
{
}

void QgsSslCertificateWidget::loadSettings( QgsSslCertSettings::SslStoreType storeType, const QString &certId, const QString &keyId,
    bool keyPassphrase, const QString &issuerId, bool issuerSelf )
{
  setSslStoreType( storeType );
  setCertId( certId );
  setKeyId( keyId );
  setKeyHasPassPhrase( keyPassphrase );
  setIssuerId( issuerId );
  setIssuerSelfSigned( issuerSelf );

  validateCert();
}


void QgsSslCertificateWidget::loadSslCertSettings( const QgsSslCertSettings& pki )
{
  setSslStoreType( pki.storeType() );
  setCertId( pki.certId() );
  setKeyId( pki.keyId() );
  setKeyHasPassPhrase( pki.hasKeyPassphrase() );
  setIssuerId( pki.issuerId() );
  setIssuerSelfSigned( pki.issuerSelfSigned() );

  setAccessUrl( pki.accessUrl() );

  validateCert();
}

QgsSslCertSettings QgsSslCertificateWidget::toSslCertSettings()
{
  QgsSslCertSettings pki;
  pki.setCertReady( certIsValid() );
  pki.setStoreType( ssLStoreType() );
  pki.setCertId( certId() );
  pki.setKeyId( keyId() );
  pki.setHasKeyPassphrase( keyHasPassPhrase() );
  pki.setIssuerId( issuerId() );
  pki.setIssuerSelfSigned( issuerSelfSigned() );
  pki.setAccessUrl( mAccessUrl );

  return pki;
}

bool QgsSslCertificateWidget::certIsValid()
{
  return mCertValid;
}

void QgsSslCertificateWidget::fileFound( bool found, QWidget * widget )
{
  if ( !found )
  {
    widget->setStyleSheet( validRed_( "QLineEdit" ) );
    widget->setToolTip( tr( "File not found" ) );
  }
  else
  {
    widget->setStyleSheet( "" );
    widget->setToolTip( "" );
  }
}

void QgsSslCertificateWidget::validateCert()
{
  mCertValid = false;
  bool certFound = false;
  bool keyFound = false;
  bool issuerFound = false;

  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    QString certPath = QgsSslUtils::qgisCertPath( certId() );
    QString keyPath = QgsSslUtils::qgisKeyPath( keyId() );
    QString issuerPath = QgsSslUtils::qgisIssuerPath( issuerId() );
    certFound = !certPath.isNull();
    keyFound = !keyPath.isNull();
    issuerFound = !issuerPath.isNull();

    fileFound( certId().isEmpty() || certFound, leQgisCert );
    fileFound( keyId().isEmpty() || keyFound, leQgisKey );
    fileFound( issuerId().isEmpty() || issuerFound, leQgisIssuer );

    if ( !certFound || !keyFound )
    {
      // invalid, due to missing components
      clearMessage();
      mCertValid = false;
      return;
    }
    else
    {
      // check for validity, then why it is not
      QSslCertificate cert( QgsSslUtils::certFromPath( certPath ) );
      QDateTime startDate( cert.effectiveDate() );
      QDateTime endDate( cert.expiryDate() );

      mCertValid = cert.isValid();

      writeMessage( tr( "%1 thru %2" ).arg( startDate.toString() ).arg( endDate.toString() ),
                    ( mCertValid ? Valid : Invalid ) );
    }
  }
}

void QgsSslCertificateWidget::clearMessage()
{
  leMsg->clear();
  leMsg->setStyleSheet( "" );
  leMsg->hide();
}

void QgsSslCertificateWidget::writeMessage( const QString& msg, Validity valid )
{
  QString ss;
  QString txt( msg );
  switch ( valid )
  {
    case Valid:
      ss = validGreen_( "QLineEdit" );
      txt = tr( "Valid: %1" ).arg( msg );
      break;
    case Invalid:
      ss = validRed_( "QLineEdit" );
      txt = tr( "Invalid: %1" ).arg( msg );
      break;
    case Unknown:
      ss = "";
      break;
    default:
      ss = "";
  }
  leMsg->setStyleSheet( ss );
  leMsg->setText( txt );
  leMsg->setCursorPosition( 0 );
  leMsg->show();
}

QgsSslCertSettings::SslStoreType QgsSslCertificateWidget::ssLStoreType()
{
  return mStoreType;
}

QString QgsSslCertificateWidget::certId() const
{
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    return leQgisCert->text();
  }
  return "";
}

QString QgsSslCertificateWidget::keyId() const
{
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    return leQgisKey->text();
  }
  return "";
}

bool QgsSslCertificateWidget::keyHasPassPhrase()
{
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    return chkQgisKeyPhrase->isChecked();
  }
  return false;
}

QString QgsSslCertificateWidget::issuerId() const
{
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    return leQgisIssuer->text();
  }
  return "";
}

bool QgsSslCertificateWidget::issuerSelfSigned()
{
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    return chkQgisIssuerSelf->isChecked();
  }
  return false;
}

void QgsSslCertificateWidget::setSslStoreType( QgsSslCertSettings::SslStoreType storeType )
{
  mStoreType = storeType;
  cmbStoreType->setCurrentIndex( mStoreType );
  stckwCertStore->setCurrentIndex( mStoreType );
}

void QgsSslCertificateWidget::setCertId( const QString& id )
{
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    leQgisCert->setText( id );
    leQgisCert->setStyleSheet( "" );
  }
}

void QgsSslCertificateWidget::setKeyId( const QString& id )
{
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    leQgisKey->setText( id );
    leQgisKey->setStyleSheet( "" );
  }
}

void QgsSslCertificateWidget::setKeyHasPassPhrase( bool hasPass )
{
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    chkQgisKeyPhrase->setChecked( hasPass );
  }
}

void QgsSslCertificateWidget::setIssuerId( const QString& id )
{
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    leQgisIssuer->setText( id );
    leQgisIssuer->setStyleSheet( "" );
  }
}

void QgsSslCertificateWidget::setIssuerSelfSigned( bool selfSigned )
{
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    chkQgisIssuerSelf->setChecked( selfSigned );
  }
}

void QgsSslCertificateWidget::clearCert()
{
  clearCertId();
  clearKeyId();
  clearKeyHasPassPhrase();
  clearIssuerId();
  clearIssuerSelfSigned();

  clearMessage();

//  btnValidateCert->setStyleSheet( "" );
//  btnValidateCert->setEnabled( false );
}

void QgsSslCertificateWidget::clearCertId()
{
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    leQgisCert->clear();
    leQgisCert->setStyleSheet( "" );
  }
}

void QgsSslCertificateWidget::clearKeyId()
{
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    leQgisKey->clear();
    leQgisKey->setStyleSheet( "" );
  }
  validateCert();
}

void QgsSslCertificateWidget::clearKeyHasPassPhrase()
{
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    chkQgisKeyPhrase->setChecked( false );
  }
  validateCert();
}

void QgsSslCertificateWidget::clearIssuerId()
{
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    leQgisIssuer->clear();
    leQgisIssuer->setStyleSheet( "" );
  }
  validateCert();
}

void QgsSslCertificateWidget::clearIssuerSelfSigned()
{
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    chkQgisIssuerSelf->setChecked( false );
  }
  validateCert();
}

QAction * QgsSslCertificateWidget::clearAction()
{
  QAction * clearAct = new QAction( tr( "Clear" ), this );
  clearAct->setData( QVariant( "clear" ) );
  QFont f( clearAct->font() );
  f.setItalic( true );
  clearAct->setFont( f );
  return clearAct;
}

QAction * QgsSslCertificateWidget::selectAction()
{
  QAction * selectAct = new QAction( tr( "Select..." ), this );
  selectAct->setData( QVariant( "select" ) );
  QFont f( selectAct->font() );
  f.setItalic( true );
  selectAct->setFont( f );
  return selectAct;
}

QString QgsSslCertificateWidget::getOpenFileName( const QString& path )
{
  return QFileDialog::getOpenFileName( this, tr( "Open PEM File" ),
                                       path, tr( "PEM (*.pem)" ) );
}

void QgsSslCertificateWidget::openQgisStoreDir()
{
  QString path = QDir::toNativeSeparators( QgsSslUtils::qgisCertStoreDirPath() );
  QDesktopServices::openUrl( QUrl( QString( "file:///" ) + path ) );
}

void QgsSslCertificateWidget::populateQgisCertsMenu()
{
  mQgisCertsMenu->clear();
  mQgisCertsMenu->addAction( clearAction() );
  mQgisCertsMenu->addSeparator();

  QStringList certs( QgsSslUtils::storeCerts( QgsSslCertSettings::QGISStore ) );
  foreach ( QString cert, certs )
  {
    QAction * act = new QAction( cert, this );
    mQgisCertsMenu->addAction( act );
  }
//  mQgisCertsMenu->addAction( selectAction() );
}

void QgsSslCertificateWidget::populateQgisKeysMenu()
{
  mQgisKeysMenu->clear();
  mQgisKeysMenu->addAction( clearAction() );
  mQgisKeysMenu->addSeparator();

  QStringList keys( QgsSslUtils::storeKeys( QgsSslCertSettings::QGISStore ) );
  foreach ( QString key, keys )
  {
    QAction * act = new QAction( key, this );
    mQgisKeysMenu->addAction( act );
  }
//  mQgisKeysMenu->addAction( selectAction() );
}

void QgsSslCertificateWidget::populateQgisIssuersMenu()
{
  mQgisIssuersMenu->clear();
  mQgisIssuersMenu->addAction( clearAction() );
  mQgisIssuersMenu->addSeparator();

  QStringList isss( QgsSslUtils::storeIssuers( QgsSslCertSettings::QGISStore ) );
  foreach ( QString iss, isss )
  {
    QAction * act = new QAction( iss, this );
    mQgisIssuersMenu->addAction( act );
  }
//  mQgisIssuersMenu->addAction( selectAction() );
}

void QgsSslCertificateWidget::qgisCertsMenuTriggered( QAction * act )
{
  if ( act->data() == QVariant( "clear" ) )
  {
    clearCertId();
  }
  else if ( act->data() == QVariant( "select" ) )
  {
    const QString& fn = getOpenFileName( QgsSslUtils::qgisCertsDirPath() );
    if ( !fn.isEmpty() )
    {
      setCertId( fn );
    }
  }
  else
  {
    setCertId( act->text() );
  }
  validateCert();
}

void QgsSslCertificateWidget::qgisKeysMenuTriggered( QAction * act )
{
  if ( act->data() == QVariant( "clear" ) )
  {
    clearKeyId();
  }
  else if ( act->data() == QVariant( "select" ) )
  {
    const QString& fn = getOpenFileName( QgsSslUtils::qgisKeysDirPath() );
    if ( !fn.isEmpty() )
    {
      setKeyId( fn );
    }
  }
  else
  {
    setKeyId( act->text() );
  }
  validateCert();
}

void QgsSslCertificateWidget::qgisIssuersMenuTriggered( QAction * act )
{
  if ( act->data() == QVariant( "clear" ) )
  {
    clearIssuerId();
  }
  else if ( act->data() == QVariant( "select" ) )
  {
    const QString& fn = getOpenFileName( QgsSslUtils::qgisIssuersDirPath() );
    if ( !fn.isEmpty() )
    {
      setIssuerId( fn );
    }
  }
  else
  {
    setIssuerId( act->text() );
  }
  validateCert();
}

