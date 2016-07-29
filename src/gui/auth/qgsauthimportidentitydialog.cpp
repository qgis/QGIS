/***************************************************************************
    qgsauthimportidentitydialog.cpp
    ---------------------
    begin                : May 9, 2015
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

#include "qgsauthimportidentitydialog.h"
#include "ui_qgsauthimportidentitydialog.h"

#include <QFile>
#include <QFileDialog>
#include <QPushButton>
#include <QSettings>

#include "qgsauthcertutils.h"
#include "qgsauthconfig.h"
#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"


static QByteArray fileData_( const QString& path, bool astext = false )
{
  QByteArray data;
  QFile file( path );
  if ( file.exists() )
  {
    QFile::OpenMode openflags( QIODevice::ReadOnly );
    if ( astext )
      openflags |= QIODevice::Text;
    bool ret = file.open( openflags );
    if ( ret )
    {
      data = file.readAll();
    }
    file.close();
  }
  return data;
}


QgsAuthImportIdentityDialog::QgsAuthImportIdentityDialog( QgsAuthImportIdentityDialog::IdentityType identitytype,
    QWidget *parent )
    : QDialog( parent )
    , mIdentityType( CertIdentity )
    , mPkiBundle( QgsPkiBundle() )
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
    connect( buttonBox, SIGNAL( rejected() ), this, SLOT( close() ) );
    connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );

    mIdentityType = identitytype;

    populateIdentityType();
  }
}

QgsAuthImportIdentityDialog::~QgsAuthImportIdentityDialog()
{
}

QgsAuthImportIdentityDialog::IdentityType QgsAuthImportIdentityDialog::identityType()
{
  if ( mDisabled )
  {
    return QgsAuthImportIdentityDialog::CertIdentity;
  }
  return mIdentityType;
}

const QPair<QSslCertificate, QSslKey> QgsAuthImportIdentityDialog::certBundleToImport()
{
  if ( mDisabled )
  {
    return qMakePair( QSslCertificate(), QSslKey() );
  }
  return mCertBundle;
}

void QgsAuthImportIdentityDialog::populateIdentityType()
{
  if ( mIdentityType == CertIdentity )
  {
    stkwBundleType->setVisible( true );

    cmbIdentityTypes->addItem( tr( "PKI PEM/DER Certificate Paths" ),
                               QVariant( QgsAuthImportIdentityDialog::PkiPaths ) );
    cmbIdentityTypes->addItem( tr( "PKI PKCS#12 Certificate Bundle" ),
                               QVariant( QgsAuthImportIdentityDialog::PkiPkcs12 ) );

    connect( cmbIdentityTypes, SIGNAL( currentIndexChanged( int ) ),
             stkwBundleType, SLOT( setCurrentIndex( int ) ) );
    connect( stkwBundleType, SIGNAL( currentChanged( int ) ),
             cmbIdentityTypes, SLOT( setCurrentIndex( int ) ) );

    connect( cmbIdentityTypes, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( validateIdentity() ) );
    connect( stkwBundleType, SIGNAL( currentChanged( int ) ),
             this, SLOT( validateIdentity() ) );

    cmbIdentityTypes->setCurrentIndex( 0 );
    stkwBundleType->setCurrentIndex( 0 );
  }
  // else switch stacked widget, and populate/connect according to that type and widget
}

void QgsAuthImportIdentityDialog::validateIdentity()
{
  bool ok = false;
  if ( mIdentityType == CertIdentity )
  {
    ok = validateBundle();
  }
  okButton()->setEnabled( ok );
}

bool QgsAuthImportIdentityDialog::validateBundle()
{

  // clear out any previously set bundle
  QSslCertificate emptycert;
  QSslKey emptykey;
  mCertBundle = qMakePair( emptycert, emptykey );
  mPkiBundle = QgsPkiBundle();

  QWidget *curpage = stkwBundleType->currentWidget();
  if ( curpage == pagePkiPaths )
  {
    return validatePkiPaths();
  }
  else if ( curpage == pagePkiPkcs12 )
  {
    return validatePkiPkcs12();
  }

  return false;
}

void QgsAuthImportIdentityDialog::clearValidation()
{
  teValidation->clear();
  teValidation->setStyleSheet( "" );
}

void QgsAuthImportIdentityDialog::writeValidation( const QString &msg,
    QgsAuthImportIdentityDialog::Validity valid,
    bool append )
{
  QString ss;
  QString txt( msg );
  switch ( valid )
  {
    case Valid:
      ss = QgsAuthGuiUtils::greenTextStyleSheet( "QTextEdit" );
      txt = tr( "Valid: %1" ).arg( msg );
      break;
    case Invalid:
      ss = QgsAuthGuiUtils::redTextStyleSheet( "QTextEdit" );
      txt = tr( "Invalid: %1" ).arg( msg );
      break;
    case Unknown:
    default:
      ss = "";
      break;
  }
  teValidation->setStyleSheet( ss );
  if ( append )
  {
    teValidation->append( txt );
  }
  else
  {
    teValidation->setText( txt );
  }
  teValidation->moveCursor( QTextCursor::Start );
}

void QgsAuthImportIdentityDialog::on_lePkiPathsKeyPass_textChanged( const QString &pass )
{
  Q_UNUSED( pass );
  validateIdentity();
}

void QgsAuthImportIdentityDialog::on_chkPkiPathsPassShow_stateChanged( int state )
{
  lePkiPathsKeyPass->setEchoMode(( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}

void QgsAuthImportIdentityDialog::on_btnPkiPathsCert_clicked()
{
  const QString& fn = getOpenFileName( tr( "Open Client Certificate File" ),  tr( "PEM (*.pem);;DER (*.der)" ) );
  if ( !fn.isEmpty() )
  {
    lePkiPathsCert->setText( fn );
    validateIdentity();
  }
}

void QgsAuthImportIdentityDialog::on_btnPkiPathsKey_clicked()
{
  const QString& fn = getOpenFileName( tr( "Open Private Key File" ),  tr( "PEM (*.pem);;DER (*.der)" ) );
  if ( !fn.isEmpty() )
  {
    lePkiPathsKey->setText( fn );
    validateIdentity();
  }
}

void QgsAuthImportIdentityDialog::on_lePkiPkcs12KeyPass_textChanged( const QString &pass )
{
  Q_UNUSED( pass );
  validateIdentity();
}

void QgsAuthImportIdentityDialog::on_chkPkiPkcs12PassShow_stateChanged( int state )
{
  lePkiPkcs12KeyPass->setEchoMode(( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}

void QgsAuthImportIdentityDialog::on_btnPkiPkcs12Bundle_clicked()
{
  const QString& fn = getOpenFileName( tr( "Open PKCS#12 Certificate Bundle" ),  tr( "PKCS#12 (*.p12 *.pfx)" ) );
  if ( !fn.isEmpty() )
  {
    lePkiPkcs12Bundle->setText( fn );
    validateIdentity();
  }
}

bool QgsAuthImportIdentityDialog::validatePkiPaths()
{
  bool isvalid = false;

  // required components
  QString certpath( lePkiPathsCert->text() );
  QString keypath( lePkiPathsKey->text() );

  bool certfound = QFile::exists( certpath );
  bool keyfound = QFile::exists( keypath );

  fileFound( certpath.isEmpty() || certfound, lePkiPathsCert );
  fileFound( keypath.isEmpty() || keyfound, lePkiPathsKey );

  if ( !certfound || !keyfound )
  {
    writeValidation( tr( "Missing components" ), Invalid );
    return false;
  }

  // check for issue date validity
  QSslCertificate clientcert;
  QList<QSslCertificate> certs( QgsAuthCertUtils::certsFromFile( certpath ) );
  QList<QSslCertificate> ca_certs;
  if ( !certs.isEmpty() )
  {
    clientcert = certs.takeFirst();
  }
  else
  {
    writeValidation( tr( "Failed to read client certificate from file" ), Invalid );
    return false;
  }

  if ( clientcert.isNull() )
  {
    writeValidation( tr( "Failed to load client certificate from file" ), Invalid );
    return false;
  }

  if ( !certs.isEmpty() ) // Multiple certificates in file
  {
    teValidation->append( tr( "Extra certificates found with identity" ) );
    ca_certs = certs;
  }

  isvalid = clientcert.isValid();
  QDateTime startdate( clientcert.effectiveDate() );
  QDateTime enddate( clientcert.expiryDate() );

  writeValidation( tr( "%1 thru %2" ).arg( startdate.toString(), enddate.toString() ),
                   ( isvalid ? Valid : Invalid ) );
  //TODO: set enabled on cert info button, relative to cert validity

  // check for valid private key and that any supplied password works
  bool keypem = keypath.endsWith( ".pem", Qt::CaseInsensitive );
  QByteArray keydata( fileData_( keypath, keypem ) );

  QSslKey clientkey;
  QString keypass = lePkiPathsKeyPass->text();
  clientkey = QSslKey( keydata,
                       QSsl::Rsa,
                       keypem ? QSsl::Pem : QSsl::Der,
                       QSsl::PrivateKey,
                       !keypass.isEmpty() ? keypass.toUtf8() : QByteArray() );
  if ( clientkey.isNull() )
  {
    // try DSA algorithm, since Qt can't seem to determine it otherwise
    clientkey = QSslKey( keydata,
                         QSsl::Dsa,
                         keypem ? QSsl::Pem : QSsl::Der,
                         QSsl::PrivateKey,
                         !keypass.isEmpty() ? keypass.toUtf8() : QByteArray() );
  }

  if ( clientkey.isNull() )
  {
    writeValidation( tr( "Failed to load client private key from file" ), Invalid, true );
    if ( !keypass.isEmpty() )
    {
      writeValidation( tr( "Private key password may not match" ), Invalid, true );
    }
    return false;
  }
  else
  {
    isvalid = isvalid && true;
  }

  if ( isvalid )
  {
    mCertBundle = qMakePair( clientcert, clientkey );
    mPkiBundle = QgsPkiBundle( clientcert,
                               clientkey,
                               ca_certs );
  }

  return isvalid;
}

bool QgsAuthImportIdentityDialog::validatePkiPkcs12()
{
  // required components
  QString bundlepath( lePkiPkcs12Bundle->text() );
  bool bundlefound = QFile::exists( bundlepath );
  fileFound( bundlepath.isEmpty() || bundlefound, lePkiPkcs12Bundle );

  if ( !bundlefound )
  {
    writeValidation( tr( "Missing components" ), Invalid );
    return false;
  }

  if ( !QCA::isSupported( "pkcs12" ) )
  {
    writeValidation( tr( "QCA library has no PKCS#12 support" ), Invalid );
    return false;
  }

  // load the bundle
  QCA::SecureArray passarray;
  QString keypass = QString::null;
  if ( !lePkiPkcs12KeyPass->text().isEmpty() )
  {
    passarray = QCA::SecureArray( lePkiPkcs12KeyPass->text().toUtf8() );
    keypass = lePkiPkcs12KeyPass->text();
  }

  QCA::ConvertResult res;
  QCA::KeyBundle bundle( QCA::KeyBundle::fromFile( bundlepath, passarray, &res, QString( "qca-ossl" ) ) );

  if ( res == QCA::ErrorFile )
  {
    writeValidation( tr( "Failed to read bundle file" ), Invalid );
    return false;
  }
  else if ( res == QCA::ErrorPassphrase )
  {
    writeValidation( tr( "Incorrect bundle password" ), Invalid );
    lePkiPkcs12KeyPass->setPlaceholderText( QString( "Required passphrase" ) );
    return false;
  }
  else if ( res == QCA::ErrorDecode )
  {
    writeValidation( tr( "Failed to decode (try entering password)" ), Invalid );
    return false;
  }

  if ( bundle.isNull() )
  {
    writeValidation( tr( "Bundle empty or can not be loaded" ), Invalid );
    return false;
  }

  // check for primary cert and that it is valid
  QCA::Certificate cert( bundle.certificateChain().primary() );
  if ( cert.isNull() )
  {
    writeValidation( tr( "Bundle client cert can not be loaded" ), Invalid );
    return false;
  }

  // TODO: add more robust validation, including cert chain resolution
  QDateTime startdate( cert.notValidBefore() );
  QDateTime enddate( cert.notValidAfter() );
  QDateTime now( QDateTime::currentDateTime() );
  bool bundlevalid = ( now >= startdate && now <= enddate );

  writeValidation( tr( "%1 thru %2" ).arg( startdate.toString(), enddate.toString() ),
                   ( bundlevalid ? Valid : Invalid ) );

  if ( bundlevalid )
  {
    QSslCertificate clientcert;
    QList<QSslCertificate> certs( QgsAuthCertUtils::certsFromString( cert.toPEM() ) );
    if ( !certs.isEmpty() )
    {
      clientcert = certs.first();
    }
    if ( clientcert.isNull() )
    {
      writeValidation( tr( "Qt cert could not be created from QCA cert" ), Invalid, true );
      return false;
    }
    QSslKey clientkey;
    clientkey = QSslKey( bundle.privateKey().toRSA().toPEM().toAscii(), QSsl::Rsa );
    if ( clientkey.isNull() )
    {
      writeValidation( tr( "Qt private key could not be created from QCA key" ), Invalid, true );
      return false;
    }

    QCA::CertificateChain cert_chain( bundle.certificateChain() );
    QList<QSslCertificate> ca_certs;
    if ( cert_chain.size() > 1 )
    {
      Q_FOREACH ( const QCA::Certificate& ca_cert, cert_chain )
      {
        if ( ca_cert != cert_chain.primary() )
        {
          ca_certs << QSslCertificate( ca_cert.toPEM().toAscii() );
        }
      }
    }

    mCertBundle = qMakePair( clientcert, clientkey );
    mPkiBundle = QgsPkiBundle( clientcert, clientkey, ca_certs );
  }

  return bundlevalid;
}

void QgsAuthImportIdentityDialog::fileFound( bool found, QWidget *widget )
{
  if ( !found )
  {
    widget->setStyleSheet( QgsAuthGuiUtils::redTextStyleSheet( "QLineEdit" ) );
    widget->setToolTip( tr( "File not found" ) );
  }
  else
  {
    widget->setStyleSheet( "" );
    widget->setToolTip( "" );
  }
}

QString QgsAuthImportIdentityDialog::getOpenFileName( const QString &title, const QString &extfilter )
{
  QSettings settings;
  QString recentdir = settings.value( "UI/lastAuthImportBundleOpenFileDir", QDir::homePath() ).toString();
  QString f = QFileDialog::getOpenFileName( this, title, recentdir, extfilter );

  // return dialog focus on Mac
  this->raise();
  this->activateWindow();

  if ( !f.isEmpty() )
  {
    settings.setValue( "UI/lastAuthImportBundleOpenFileDir", QFileInfo( f ).absoluteDir().path() );
  }
  return f;
}

QPushButton *QgsAuthImportIdentityDialog::okButton()
{
  return buttonBox->button( QDialogButtonBox::Ok );
}
