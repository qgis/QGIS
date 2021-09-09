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

#include "qgssettings.h"
#include "qgsauthcertutils.h"
#include "qgsauthconfig.h"
#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgsapplication.h"


QgsAuthImportIdentityDialog::QgsAuthImportIdentityDialog( QgsAuthImportIdentityDialog::IdentityType identitytype,
    QWidget *parent )
  : QDialog( parent )
  , mIdentityType( CertIdentity )
  , mPkiBundle( QgsPkiBundle() )
  , mDisabled( false )

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
    connect( lePkiPathsKeyPass, &QLineEdit::textChanged, this, &QgsAuthImportIdentityDialog::lePkiPathsKeyPass_textChanged );
    connect( chkPkiPathsPassShow, &QCheckBox::stateChanged, this, &QgsAuthImportIdentityDialog::chkPkiPathsPassShow_stateChanged );
    connect( btnPkiPathsCert, &QToolButton::clicked, this, &QgsAuthImportIdentityDialog::btnPkiPathsCert_clicked );
    connect( btnPkiPathsKey, &QToolButton::clicked, this, &QgsAuthImportIdentityDialog::btnPkiPathsKey_clicked );
    connect( lePkiPkcs12KeyPass, &QLineEdit::textChanged, this, &QgsAuthImportIdentityDialog::lePkiPkcs12KeyPass_textChanged );
    connect( chkPkiPkcs12PassShow, &QCheckBox::stateChanged, this, &QgsAuthImportIdentityDialog::chkPkiPkcs12PassShow_stateChanged );
    connect( btnPkiPkcs12Bundle, &QToolButton::clicked, this, &QgsAuthImportIdentityDialog::btnPkiPkcs12Bundle_clicked );
    connect( buttonBox, &QDialogButtonBox::rejected, this, &QWidget::close );
    connect( buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );

    mIdentityType = identitytype;

    populateIdentityType();
  }
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

    connect( cmbIdentityTypes, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
             stkwBundleType, &QStackedWidget::setCurrentIndex );
    connect( stkwBundleType, &QStackedWidget::currentChanged,
             cmbIdentityTypes, &QComboBox::setCurrentIndex );

    connect( cmbIdentityTypes, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
             this, [ = ] { validateIdentity(); } );
    connect( stkwBundleType, &QStackedWidget::currentChanged,
             this, &QgsAuthImportIdentityDialog::validateIdentity );

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
  const QSslCertificate emptycert;
  const QSslKey emptykey;
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
  teValidation->setStyleSheet( QString() );
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
      ss = QgsAuthGuiUtils::greenTextStyleSheet( QStringLiteral( "QTextEdit" ) );
      txt = tr( "Valid: %1" ).arg( msg );
      break;
    case Invalid:
      ss = QgsAuthGuiUtils::redTextStyleSheet( QStringLiteral( "QTextEdit" ) );
      txt = tr( "Invalid: %1" ).arg( msg );
      break;
    case Unknown:
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

void QgsAuthImportIdentityDialog::lePkiPathsKeyPass_textChanged( const QString &pass )
{
  Q_UNUSED( pass )
  validateIdentity();
}

void QgsAuthImportIdentityDialog::chkPkiPathsPassShow_stateChanged( int state )
{
  lePkiPathsKeyPass->setEchoMode( ( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}

void QgsAuthImportIdentityDialog::btnPkiPathsCert_clicked()
{
  const QString &fn = getOpenFileName( tr( "Open Client Certificate File" ),  tr( "All files (*.*);;PEM (*.pem);;DER (*.der)" ) );
  if ( !fn.isEmpty() )
  {
    lePkiPathsCert->setText( fn );
    validateIdentity();
  }
}

void QgsAuthImportIdentityDialog::btnPkiPathsKey_clicked()
{
  const QString &fn = getOpenFileName( tr( "Open Private Key File" ),  tr( "All files (*.*);;PEM (*.pem);;DER (*.der)" ) );
  if ( !fn.isEmpty() )
  {
    lePkiPathsKey->setText( fn );
    validateIdentity();
  }
}

void QgsAuthImportIdentityDialog::lePkiPkcs12KeyPass_textChanged( const QString &pass )
{
  Q_UNUSED( pass )
  validateIdentity();
}

void QgsAuthImportIdentityDialog::chkPkiPkcs12PassShow_stateChanged( int state )
{
  lePkiPkcs12KeyPass->setEchoMode( ( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}

void QgsAuthImportIdentityDialog::btnPkiPkcs12Bundle_clicked()
{
  const QString &fn = getOpenFileName( tr( "Open PKCS#12 Certificate Bundle" ),  tr( "PKCS#12 (*.p12 *.pfx)" ) );
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
  const QString certpath( lePkiPathsCert->text() );
  const QString keypath( lePkiPathsKey->text() );

  const bool certfound = QFile::exists( certpath );
  const bool keyfound = QFile::exists( keypath );

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

  isvalid = QgsAuthCertUtils::certIsViable( clientcert );

  const QDateTime startdate( clientcert.effectiveDate() );
  const QDateTime enddate( clientcert.expiryDate() );

  writeValidation( tr( "%1 thru %2" ).arg( startdate.toString(), enddate.toString() ),
                   ( QgsAuthCertUtils::certIsCurrent( clientcert ) ? Valid : Invalid ) );
  //TODO: set enabled on cert info button, relative to cert validity

  // check for valid private key and that any supplied password works
  const QString keypass( lePkiPathsKeyPass->text() );
  const QSslKey clientkey( QgsAuthCertUtils::keyFromFile( keypath, keypass ) );
  if ( clientkey.isNull() )
  {
    writeValidation( tr( "Failed to load client private key from file" ), Invalid, true );
    if ( !keypass.isEmpty() )
    {
      writeValidation( tr( "Private key password may not match" ), Invalid, true );
    }
    return false;
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
  const QString bundlepath( lePkiPkcs12Bundle->text() );
  const bool bundlefound = QFile::exists( bundlepath );
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
  QString keypass = QString();
  if ( !lePkiPkcs12KeyPass->text().isEmpty() )
  {
    passarray = QCA::SecureArray( lePkiPkcs12KeyPass->text().toUtf8() );
    keypass = lePkiPkcs12KeyPass->text();
  }

  QCA::ConvertResult res;
  const QCA::KeyBundle bundle( QCA::KeyBundle::fromFile( bundlepath, passarray, &res, QStringLiteral( "qca-ossl" ) ) );

  if ( res == QCA::ErrorFile )
  {
    writeValidation( tr( "Failed to read bundle file" ), Invalid );
    return false;
  }
  else if ( res == QCA::ErrorPassphrase )
  {
    writeValidation( tr( "Incorrect bundle password" ), Invalid );
    lePkiPkcs12KeyPass->setPlaceholderText( QStringLiteral( "Required passphrase" ) );
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
  const QCA::Certificate cert( bundle.certificateChain().primary() );
  if ( cert.isNull() )
  {
    writeValidation( tr( "Bundle client cert can not be loaded" ), Invalid );
    return false;
  }

  // TODO: add more robust validation, including cert chain resolution
  const QDateTime startdate( cert.notValidBefore() );
  const QDateTime enddate( cert.notValidAfter() );
  const QDateTime now( QDateTime::currentDateTime() );
  const bool bundlevalid = ( now >= startdate && now <= enddate );

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
    clientkey = QSslKey( bundle.privateKey().toRSA().toPEM().toLatin1(), QSsl::Rsa );
    if ( clientkey.isNull() )
    {
      writeValidation( tr( "Qt private key could not be created from QCA key" ), Invalid, true );
      return false;
    }

    const QCA::CertificateChain cert_chain( bundle.certificateChain() );
    QList<QSslCertificate> ca_certs;
    if ( cert_chain.size() > 1 )
    {
      const auto constCert_chain = cert_chain;
      for ( const QCA::Certificate &ca_cert : constCert_chain )
      {
        if ( ca_cert != cert_chain.primary() )
        {
          ca_certs << QSslCertificate( ca_cert.toPEM().toLatin1() );
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
    widget->setStyleSheet( QgsAuthGuiUtils::redTextStyleSheet( QStringLiteral( "QLineEdit" ) ) );
    widget->setToolTip( tr( "File not found" ) );
  }
  else
  {
    widget->setStyleSheet( QString() );
    widget->setToolTip( QString() );
  }
}

QString QgsAuthImportIdentityDialog::getOpenFileName( const QString &title, const QString &extfilter )
{
  QgsSettings settings;
  const QString recentdir = settings.value( QStringLiteral( "UI/lastAuthImportBundleOpenFileDir" ), QDir::homePath() ).toString();
  QString f = QFileDialog::getOpenFileName( this, title, recentdir, extfilter );

  // return dialog focus on Mac
  this->raise();
  this->activateWindow();

  if ( !f.isEmpty() )
  {
    settings.setValue( QStringLiteral( "UI/lastAuthImportBundleOpenFileDir" ), QFileInfo( f ).absoluteDir().path() );
  }
  return f;
}

QPushButton *QgsAuthImportIdentityDialog::okButton()
{
  return buttonBox->button( QDialogButtonBox::Ok );
}
