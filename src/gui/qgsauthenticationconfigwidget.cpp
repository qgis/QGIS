/***************************************************************************
    qgsauthenticationconfigwidget.cpp
    ---------------------
    begin                : October 5, 2014
    copyright            : (C) 2014 by Boundless Spatial, Inc. USA
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

#include "qgsauthenticationconfigwidget.h"
#include "ui_qgsauthenticationconfigwidget.h"

#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QPushButton>
#include <QSettings>
#ifndef QT_NO_OPENSSL
#include <QSslCertificate>
#include <QSslKey>
#include <QtCrypto>
#endif

#include "qgsauthenticationconfig.h"
#include "qgsauthenticationmanager.h"

static QString validGreen_( const QString& selector = "*" )
{
  return QString( "%1{color: rgb(0, 170, 0);}" ).arg( selector );
}
static QString validRed_( const QString& selector = "*" )
{
  return QString( "%1{color: rgb(200, 0, 0);}" ).arg( selector );
}

QgsAuthConfigWidget::QgsAuthConfigWidget( QWidget *parent , const QString& authid )
    : QDialog( parent )
    , mAuthId( authid )
    , mAuthNotifyLayout( 0 )
    , mAuthNotify( 0 )
{
  if ( QgsAuthManager::instance()->isDisabled() )
  {
    mAuthNotifyLayout = new QVBoxLayout;
    this->setLayout( mAuthNotifyLayout );
    QString msg( QgsAuthManager::instance()->disabledMessage() );
    if ( !authid.isEmpty() )
    {
      msg += "\n\n" + tr( "Authentication config id not loaded: %1" ).arg( authid );
    }
    mAuthNotify = new QLabel( msg, this );
    mAuthNotifyLayout->addWidget( mAuthNotify );

    mAuthId.clear(); // otherwise will contiue to try authenticate (and fail) after save
  }
  else
  {
    setupUi( this );
    connect( buttonBox, SIGNAL( rejected() ), this, SLOT( close() ) );
    connect( buttonBox, SIGNAL( accepted() ), this, SLOT( saveConfig() ) );
    connect( buttonBox->button( QDialogButtonBox::Reset ), SIGNAL( clicked() ), this, SLOT( resetConfig() ) );

    cmbAuthProviderType->addItem( tr( "Username/Password" ), QVariant( QgsAuthType::Basic ) );

#ifdef QT_NO_OPENSSL
    stkwProviderType->removeWidget( pagePkiPaths );
#else
    cmbAuthProviderType->addItem( tr( "PKI PEM/DER Certificate Paths" ), QVariant( QgsAuthType::PkiPaths ) );
    cmbAuthProviderType->addItem( tr( "PKI PKCS#12 Certificate Bundle" ), QVariant( QgsAuthType::PkiPkcs12 ) );
#endif

    connect( cmbAuthProviderType, SIGNAL( currentIndexChanged( int ) ),
             stkwProviderType, SLOT( setCurrentIndex( int ) ) );
    connect( stkwProviderType, SIGNAL( currentChanged( int ) ),
             cmbAuthProviderType, SLOT( setCurrentIndex( int ) ) );

    connect( cmbAuthProviderType, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( validateAuth() ) );
    connect( stkwProviderType, SIGNAL( currentChanged( int ) ),
             this, SLOT( validateAuth() ) );

    cmbAuthProviderType->setCurrentIndex( 0 );
    stkwProviderType->setCurrentIndex( 0 );

    loadConfig();
    validateAuth();

    leName->setFocus();
  }
}

QgsAuthConfigWidget::~QgsAuthConfigWidget()
{
}

void QgsAuthConfigWidget::loadConfig()
{
  if ( mAuthId.isEmpty() )
    return;

  QgsAuthType::ProviderType authtype = QgsAuthManager::instance()->configProviderType( mAuthId );

  qDebug( "Loading auth id: %s", mAuthId.toAscii().constData() );
  qDebug( "Loading auth type: %s", QgsAuthType::typeToString( authtype ).toAscii().constData() );

  if ( authtype == QgsAuthType::None || authtype == QgsAuthType::Unknown )
    return;

  // edit mode requires master password to have been set and verified against auth db
  if ( !QgsAuthManager::instance()->setMasterPassword( true ) )
    return;

  int indx = providerIndexByType( authtype );
  if ( indx == -1 )
    return;

  cmbAuthProviderType->setCurrentIndex( indx );

  if ( authtype == QgsAuthType::Basic )
  {
    QgsAuthConfigBasic configbasic;
    if ( QgsAuthManager::instance()->loadAuthenticationConfig( mAuthId, configbasic, true ) )
    {
      if ( configbasic.isValid() && configbasic.type() != QgsAuthType::Unknown )
      {
        leName->setText( configbasic.name() );
        leResource->setText( configbasic.uri() );
        leAuthId->setText( configbasic.id() );

        leBasicUsername->setText( configbasic.username() );
        leBasicPassword->setText( configbasic.password() );
        leBasicRealm->setText( configbasic.realm() );
      }
    }
  }
#ifndef QT_NO_OPENSSL
  else if ( authtype == QgsAuthType::PkiPaths )
  {
    stkwProviderType->setCurrentIndex( stkwProviderType->indexOf( pagePkiPaths ) );
    QgsAuthConfigPkiPaths configpki;
    if ( QgsAuthManager::instance()->loadAuthenticationConfig( mAuthId, configpki, true ) )
    {
      if ( configpki.isValid() && configpki.type() != QgsAuthType::Unknown )
      {
        leName->setText( configpki.name() );
        leResource->setText( configpki.uri() );
        leAuthId->setText( configpki.id() );

        lePkiPathsCert->setText( configpki.certId() );
        lePkiPathsKey->setText( configpki.keyId() );
        lePkiPathsKeyPass->setText( configpki.keyPassphrase() );
        lePkiPathsIssuer->setText( configpki.issuerId() );
        chkPkiPathsIssuerSelf->setChecked( configpki.issuerSelfSigned() );
      }
      //qDebug( configpki.certAsPem().toAscii().constData() );
      //qDebug( configpki.keyAsPem( false ).first().toAscii().constData() );
      //qDebug( configpki.keyAsPem( true ).first().toAscii().constData() );
      //qDebug( configpki.issuerAsPem().toAscii().constData() );
    }
  }
  else if ( authtype == QgsAuthType::PkiPkcs12 )
  {
    stkwProviderType->setCurrentIndex( stkwProviderType->indexOf( pagePkiPkcs12 ) );
    QgsAuthConfigPkiPkcs12 configpkcs;
    if ( QgsAuthManager::instance()->loadAuthenticationConfig( mAuthId, configpkcs, true ) )
    {
      if ( configpkcs.isValid() && configpkcs.type() != QgsAuthType::Unknown )
      {
        leName->setText( configpkcs.name() );
        leResource->setText( configpkcs.uri() );
        leAuthId->setText( configpkcs.id() );

        lePkiPkcs12Bundle->setText( configpkcs.bundlePath() );
        lePkiPkcs12KeyPass->setText( configpkcs.bundlePassphrase() );
        lePkiPkcs12Issuer->setText( configpkcs.issuerPath() );
        chkPkiPkcs12IssuerSelf->setChecked( configpkcs.issuerSelfSigned() );
      }
      //qDebug( configpkcs.certAsPem().toAscii().constData() );
      //qDebug( configpkcs.keyAsPem( false ).first().toAscii().constData() );
      //qDebug( configpkcs.keyAsPem( true ).first().toAscii().constData() );
      //qDebug( configpkcs.issuerAsPem().toAscii().constData() );
    }
  }
#endif
}

void QgsAuthConfigWidget::resetConfig()
{
  clearAll();
  loadConfig();
  validateAuth();
}

void QgsAuthConfigWidget::saveConfig()
{
  if ( !QgsAuthManager::instance()->setMasterPassword( true ) )
    return;

  QWidget *curpage = stkwProviderType->currentWidget();
  if ( curpage == pageBasic ) // basic
  {
    QgsAuthConfigBasic configbasic;
    configbasic.setName( leName->text() );
    configbasic.setUri( leResource->text() );

    configbasic.setUsername( leBasicUsername->text() );
    configbasic.setPassword( leBasicPassword->text() );
    configbasic.setRealm( leBasicRealm->text() );

    if ( !mAuthId.isEmpty() ) // update
    {
      configbasic.setId( mAuthId );
      if ( QgsAuthManager::instance()->updateAuthenticationConfig( configbasic ) )
      {
        emit authenticationConfigUpdated( mAuthId );
      }
    }
    else // create new
    {
      if ( QgsAuthManager::instance()->storeAuthenticationConfig( configbasic ) )
      {
        mAuthId = configbasic.id();
        emit authenticationConfigStored( mAuthId );
      }
    }
  }
#ifndef QT_NO_OPENSSL
  else if ( curpage == pagePkiPaths ) // pki paths
  {
    QgsAuthConfigPkiPaths configpki;
    configpki.setName( leName->text() );
    configpki.setUri( leResource->text() );

    configpki.setCertId( lePkiPathsCert->text() );
    configpki.setKeyId( lePkiPathsKey->text() );
    configpki.setKeyPassphrase( lePkiPathsKeyPass->text() );
    configpki.setIssuerId( lePkiPathsIssuer->text() );
    configpki.setIssuerSelfSigned( chkPkiPathsIssuerSelf->isChecked() );

    if ( !mAuthId.isEmpty() ) // update
    {
      configpki.setId( mAuthId );
      if ( QgsAuthManager::instance()->updateAuthenticationConfig( configpki ) )
      {
        emit authenticationConfigUpdated( mAuthId );
      }
    }
    else // create new
    {
      if ( QgsAuthManager::instance()->storeAuthenticationConfig( configpki ) )
      {
        mAuthId = configpki.id();
        emit authenticationConfigStored( mAuthId );
      }
    }
  }
  else if ( curpage == pagePkiPkcs12 ) // pki pkcs#12 bundle
  {
    QgsAuthConfigPkiPkcs12 configpkcs;
    configpkcs.setName( leName->text() );
    configpkcs.setUri( leResource->text() );

    configpkcs.setBundlePath( lePkiPkcs12Bundle->text() );
    configpkcs.setBundlePassphrase( lePkiPkcs12KeyPass->text() );
    configpkcs.setIssuerPath( lePkiPkcs12Issuer->text() );
    configpkcs.setIssuerSelfSigned( chkPkiPkcs12IssuerSelf->isChecked() );

    if ( !mAuthId.isEmpty() ) // update
    {
      configpkcs.setId( mAuthId );
      if ( QgsAuthManager::instance()->updateAuthenticationConfig( configpkcs ) )
      {
        emit authenticationConfigUpdated( mAuthId );
      }
    }
    else // create new
    {
      if ( QgsAuthManager::instance()->storeAuthenticationConfig( configpkcs ) )
      {
        mAuthId = configpkcs.id();
        emit authenticationConfigStored( mAuthId );
      }
    }
  }
#endif

  this->accept();
}

void QgsAuthConfigWidget::on_btnClear_clicked()
{
  QWidget *curpage = stkwProviderType->currentWidget();
  if ( curpage == pageBasic )
  {
    clearAuthBasic();
  }
#ifndef QT_NO_OPENSSL
  else if ( curpage == pagePkiPaths )
  {
    clearPkiPathsCert();
  }
  else if ( curpage == pagePkiPkcs12 )
  {
    clearPkiPkcs12Bundle();
  }
#endif
  validateAuth();
}

void QgsAuthConfigWidget::clearAll()
{
  leName->clear();
  leResource->clear();
  leAuthId->clear();

  // basic
  clearAuthBasic();

#ifndef QT_NO_OPENSSL
  // pki paths
  clearPkiPathsCert();
  // pki pkcs#12
  clearPkiPkcs12Bundle();
#endif

  validateAuth();
}

void QgsAuthConfigWidget::validateAuth()
{
  bool authok = !leName->text().isEmpty();

  QWidget *curpage = stkwProviderType->currentWidget();
  if ( curpage == pageBasic )
  {
    authok = authok && validateBasic();
  }
#ifndef QT_NO_OPENSSL
  else if ( curpage == pagePkiPaths )
  {
    authok = authok && validatePkiPaths();
  }
  else if ( curpage == pagePkiPkcs12 )
  {
    authok = authok && validatePkiPkcs12();
  }
#endif
  buttonBox->button( QDialogButtonBox::Save )->setEnabled( authok );
}

void QgsAuthConfigWidget::on_leName_textChanged( const QString& txt )
{
  Q_UNUSED( txt );
  validateAuth();
}

int QgsAuthConfigWidget::providerIndexByType( QgsAuthType::ProviderType ptype )
{
  return cmbAuthProviderType->findData( QVariant( ptype ) );
}

void QgsAuthConfigWidget::fileFound( bool found, QWidget *widget )
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

QString QgsAuthConfigWidget::getOpenFileName( const QString& title, const QString& extfilter )
{
  QSettings settings;
  QString recentdir = settings.value( "UI/lastAuthConfigWidgetOpenFileDir", QDir::homePath() ).toString();
  QString f = QFileDialog::getOpenFileName( this, title, recentdir, extfilter );
  if ( !f.isEmpty() )
  {
    settings.setValue( "UI/lastAuthConfigWidgetOpenFileDir", QFileInfo( f ).absoluteDir().path() );
  }
  return f;
}

//////////////////////////////////////////////////////
// Auth Basic
//////////////////////////////////////////////////////

void QgsAuthConfigWidget::clearAuthBasic()
{
  leBasicUsername->clear();
  leBasicPassword->clear();
  leBasicRealm->clear();
  chkBasicPasswordShow->setChecked( false );
}

void QgsAuthConfigWidget::on_leBasicUsername_textChanged( const QString& txt )
{
  Q_UNUSED( txt );
  validateAuth();
}

void QgsAuthConfigWidget::on_chkBasicPasswordShow_stateChanged( int state )
{
  leBasicPassword->setEchoMode(( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}

bool QgsAuthConfigWidget::validateBasic()
{
  return !leBasicUsername->text().isEmpty();
}


//////// PKI below that requires Qt to be built against OpenSSL ////////////////

#ifndef QT_NO_OPENSSL

// Shared functions

void QgsAuthConfigWidget::clearPkiMessage( QLineEdit *lineedit )
{
  lineedit->clear();
  lineedit->setStyleSheet( "" );
}

void QgsAuthConfigWidget::writePkiMessage( QLineEdit *lineedit, const QString &msg, QgsAuthConfigWidget::Validity valid )
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
  lineedit->setStyleSheet( ss );
  lineedit->setText( txt );
  lineedit->setCursorPosition( 0 );
}

//////////////////////////////////////////////////////
// Auth PkiPaths
//////////////////////////////////////////////////////

void QgsAuthConfigWidget::clearPkiPathsCert()
{
  clearPkiPathsCertId();
  clearPkiPathsKeyId();
  clearPkiPathsKeyPassphrase();
  clearPkiPathsIssuerId();
  clearPkiPathsIssuerSelfSigned();

  clearPkiMessage( lePkiPathsMsg );
  validateAuth();
}

void QgsAuthConfigWidget::clearPkiPathsCertId()
{
  lePkiPathsCert->clear();
  lePkiPathsCert->setStyleSheet( "" );
}

void QgsAuthConfigWidget::clearPkiPathsKeyId()
{
  lePkiPathsKey->clear();
  lePkiPathsKey->setStyleSheet( "" );
}

void QgsAuthConfigWidget::clearPkiPathsKeyPassphrase()
{
  lePkiPathsKeyPass->clear();
  lePkiPathsKeyPass->setStyleSheet( "" );
}

void QgsAuthConfigWidget::clearPkiPathsIssuerId()
{
  lePkiPathsIssuer->clear();
  lePkiPathsIssuer->setStyleSheet( "" );
}

void QgsAuthConfigWidget::clearPkiPathsIssuerSelfSigned()
{
  chkPkiPathsIssuerSelf->setChecked( false );
}

bool QgsAuthConfigWidget::validatePkiPaths()
{
  bool certvalid = false;

  // required components
  QString certpath( lePkiPathsCert->text() );
  QString keypath( lePkiPathsKey->text() );

  bool certfound = QFile::exists( certpath );
  bool keyfound = QFile::exists( keypath );

  // optional, but if set and missing, flag it
  QString issuerpath( lePkiPathsIssuer->text() );
  bool issuerfound = true;
  if ( !issuerpath.isEmpty() )
    issuerfound = QFile::exists( issuerpath );

  fileFound( certpath.isEmpty() || certfound, lePkiPathsCert );
  fileFound( keypath.isEmpty() || keyfound, lePkiPathsKey );
  fileFound( issuerpath.isEmpty() || issuerfound, lePkiPathsIssuer );

  if ( !certfound || !keyfound || !issuerfound )
  {
    writePkiMessage( lePkiPathsMsg, tr( "Missing components" ), Invalid );
    return false;
  }

  // check for issue date validity, then notify status
  QSslCertificate cert;
  QFile file( certpath );
  QFileInfo fileinfo( file );
  QString ext( fileinfo.fileName().replace( fileinfo.completeBaseName(), "" ).toLower() );
  if ( ext.isEmpty() )
  {
    writePkiMessage( lePkiPathsMsg, tr( "Certificate file has no extension" ), Invalid );
    return false;
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
    return false;
  }

  if ( cert.isNull() )
  {
    writePkiMessage( lePkiPathsMsg, tr( "Failed to load certificate from file" ), Invalid );
    return false;
  }

  certvalid = cert.isValid();
  QDateTime startdate( cert.effectiveDate() );
  QDateTime enddate( cert.expiryDate() );

  writePkiMessage( lePkiPathsMsg,
                   tr( "%1 thru %2" ).arg( startdate.toString() ).arg( enddate.toString() ),
                   ( certvalid ? Valid : Invalid ) );

  return certvalid;
}

void QgsAuthConfigWidget::on_chkPkiPathsPassShow_stateChanged( int state )
{
  lePkiPathsKeyPass->setEchoMode(( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}

void QgsAuthConfigWidget::on_btnPkiPathsCert_clicked()
{
  const QString& fn = getOpenFileName( tr( "Open Client Certificate File" ),  tr( "PEM (*.pem);;DER (*.cer *.crt *.der)" ) );
  if ( !fn.isEmpty() )
  {
    lePkiPathsCert->setText( fn );
    validateAuth();
  }
}

void QgsAuthConfigWidget::on_btnPkiPathsKey_clicked()
{
  const QString& fn = getOpenFileName( tr( "Open Private Key File" ),  tr( "PEM (*.pem);;DER (*.der)" ) );
  if ( !fn.isEmpty() )
  {
    lePkiPathsKey->setText( fn );
    validateAuth();
  }
}

void QgsAuthConfigWidget::on_btnPkiPathsIssuer_clicked()
{
  const QString& fn = getOpenFileName( tr( "Open Issuer Certificate File" ),  tr( "PEM (*.pem);;DER (*.cer *.crt *.der)" ) );
  if ( !fn.isEmpty() )
  {
    lePkiPathsIssuer->setText( fn );
    validateAuth();
  }
}

//////////////////////////////////////////////////////
// Auth PkiPkcs#12
//////////////////////////////////////////////////////

void QgsAuthConfigWidget::clearPkiPkcs12Bundle()
{
  clearPkiPkcs12BundlePath();
  clearPkiPkcs12KeyPassphrase();
  clearPkiPkcs12IssuerPath();
  clearPkiPkcs12IssuerSelfSigned();

  clearPkiMessage( lePkiPkcs12Msg );
  validateAuth();
}

void QgsAuthConfigWidget::clearPkiPkcs12BundlePath()
{
  lePkiPkcs12Bundle->clear();
  lePkiPkcs12Bundle->setStyleSheet( "" );
}

void QgsAuthConfigWidget::clearPkiPkcs12KeyPassphrase()
{
  lePkiPkcs12KeyPass->clear();
  lePkiPkcs12KeyPass->setStyleSheet( "" );
  lePkiPkcs12KeyPass->setPlaceholderText( QString( "Optional passphrase" ) );
}

void QgsAuthConfigWidget::clearPkiPkcs12IssuerPath()
{
  lePkiPkcs12Issuer->clear();
  lePkiPkcs12Issuer->setStyleSheet( "" );
}

void QgsAuthConfigWidget::clearPkiPkcs12IssuerSelfSigned()
{
  chkPkiPkcs12IssuerSelf->setChecked( false );
}

bool QgsAuthConfigWidget::validatePkiPkcs12()
{
  // required components
  QString bundlepath( lePkiPkcs12Bundle->text() );

  bool bundlefound = QFile::exists( bundlepath );

  // optional, but if set and missing, flag it
  QString issuerpath( lePkiPkcs12Issuer->text() );
  bool issuerfound = true;
  if ( !issuerpath.isEmpty() )
    issuerfound = QFile::exists( issuerpath );

  fileFound( bundlepath.isEmpty() || bundlefound, lePkiPkcs12Bundle );
  fileFound( issuerpath.isEmpty() || issuerfound, lePkiPkcs12Issuer );

  if ( !bundlefound || !issuerfound )
  {
    writePkiMessage( lePkiPkcs12Msg, tr( "Missing components" ), Invalid );
    return false;
  }

  if ( !QCA::isSupported( "pkcs12" ) )
  {
    writePkiMessage( lePkiPkcs12Msg, tr( "QCA library has no PKCS#12 support" ), Invalid );
    return false;
  }

  // load the bundle
  QCA::SecureArray passarray;
  if ( !lePkiPkcs12KeyPass->text().isEmpty() )
    passarray = QCA::SecureArray( lePkiPkcs12KeyPass->text().toUtf8() );

  QCA::ConvertResult res;
  QCA::KeyBundle bundle( QCA::KeyBundle::fromFile( bundlepath, passarray, &res, QString( "qca-ossl" ) ) );

  if ( res == QCA::ErrorFile )
  {
    writePkiMessage( lePkiPkcs12Msg, tr( "Failed to read bundle file" ), Invalid );
    return false;
  }
  else if ( res == QCA::ErrorPassphrase )
  {
    writePkiMessage( lePkiPkcs12Msg, tr( "Incorrect bundle password" ), Invalid );
    lePkiPkcs12KeyPass->setPlaceholderText( QString( "Required passphrase" ) );
    return false;
  }
  else if ( res == QCA::ErrorDecode )
  {
    writePkiMessage( lePkiPkcs12Msg, tr( "Failed to decode (try entering password)" ), Invalid );
    return false;
  }

  if ( bundle.isNull() )
  {
    writePkiMessage( lePkiPkcs12Msg, tr( "Bundle empty or can not be loaded" ), Invalid );
    return false;
  }

  // check for primary cert and that it is valid
  QCA::Certificate cert( bundle.certificateChain().primary() );
  if ( cert.isNull() )
  {
    writePkiMessage( lePkiPkcs12Msg, tr( "Bundle client cert can not be loaded" ), Invalid );
    return false;
  }

  // TODO: add more robust validation, including cert chain resolution
  QDateTime startdate( cert.notValidBefore() );
  QDateTime enddate( cert.notValidAfter() );
  QDateTime now( QDateTime::currentDateTime() );
  bool bundlevalid = ( now >= startdate && now <= enddate );

  writePkiMessage( lePkiPkcs12Msg,
                   tr( "%1 thru %2" ).arg( startdate.toString() ).arg( enddate.toString() ),
                   ( bundlevalid ? Valid : Invalid ) );

  return bundlevalid;
}

void QgsAuthConfigWidget::on_lePkiPkcs12KeyPass_textChanged( const QString &pass )
{
  Q_UNUSED( pass );
  validateAuth();
}

void QgsAuthConfigWidget::on_chkPkiPkcs12PassShow_stateChanged( int state )
{
  lePkiPkcs12KeyPass->setEchoMode(( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}

void QgsAuthConfigWidget::on_btnPkiPkcs12Bundle_clicked()
{
  const QString& fn = getOpenFileName( tr( "Open PKCS#12 Certificate Bundle" ),  tr( "PKCS#12 (*.p12 *.pfx)" ) );
  if ( !fn.isEmpty() )
  {
    lePkiPkcs12Bundle->setText( fn );
    validateAuth();
  }
}

void QgsAuthConfigWidget::on_btnPkiPkcs12Issuer_clicked()
{
  const QString& fn = getOpenFileName( tr( "Open Issuer Certificate File" ),  tr( "PEM (*.pem);;DER (*.cer *.crt *.der)" ) );
  if ( !fn.isEmpty() )
  {
    lePkiPkcs12Issuer->setText( fn );
    validateAuth();
  }
}


#endif
