/***************************************************************************
    qgsauthcertutils.cpp
    ---------------------
    begin                : May 1, 2015
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

#include "qgsauthcertutils.h"

#include <QColor>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QSslCertificate>
#include <QUuid>

#include "qgsauthmanager.h"
#include "qgslogger.h"

QString QgsAuthCertUtils::getSslProtocolName( QSsl::SslProtocol protocol )
{
  switch ( protocol )
  {
#if QT_VERSION >= 0x040800
    case QSsl::SecureProtocols:
      return QObject::tr( "SecureProtocols" );
    case QSsl::TlsV1SslV3:
      return QObject::tr( "TlsV1SslV3" );
#endif
    case QSsl::TlsV1:
      return QObject::tr( "TlsV1" );
    case QSsl::SslV3:
      return QObject::tr( "SslV3" );
    case QSsl::SslV2:
      return QObject::tr( "SslV2" );
    default:
      return QString();
  }
}

QMap<QString, QSslCertificate> QgsAuthCertUtils::mapDigestToCerts( const QList<QSslCertificate>& certs )
{
  QMap<QString, QSslCertificate> digestmap;
  Q_FOREACH ( const QSslCertificate& cert, certs )
  {
    digestmap.insert( shaHexForCert( cert ), cert );
  }
  return digestmap;
}

QMap<QString, QList<QSslCertificate> > QgsAuthCertUtils::certsGroupedByOrg( const QList<QSslCertificate>& certs )
{
  QMap< QString, QList<QSslCertificate> > orgcerts;
  Q_FOREACH ( const QSslCertificate& cert, certs )
  {
    QString org( SSL_SUBJECT_INFO( cert, QSslCertificate::Organization ) );
    if ( org.isEmpty() )
      org = "(Organization not defined)";
    QList<QSslCertificate> valist = orgcerts.contains( org ) ? orgcerts.value( org ) : QList<QSslCertificate>();
    orgcerts.insert( org, valist << cert );
  }
  return orgcerts;
}

QMap<QString, QgsAuthConfigSslServer> QgsAuthCertUtils::mapDigestToSslConfigs( const QList<QgsAuthConfigSslServer>& configs )
{
  QMap<QString, QgsAuthConfigSslServer> digestmap;
  Q_FOREACH ( const QgsAuthConfigSslServer& config, configs )
  {
    digestmap.insert( shaHexForCert( config.sslCertificate() ), config );
  }
  return digestmap;
}

QMap<QString, QList<QgsAuthConfigSslServer> > QgsAuthCertUtils::sslConfigsGroupedByOrg( const QList<QgsAuthConfigSslServer>& configs )
{
  QMap< QString, QList<QgsAuthConfigSslServer> > orgconfigs;
  Q_FOREACH ( const QgsAuthConfigSslServer& config, configs )
  {
    QString org( SSL_SUBJECT_INFO( config.sslCertificate(), QSslCertificate::Organization ) );

    if ( org.isEmpty() )
      org = QObject::tr( "(Organization not defined)" );
    QList<QgsAuthConfigSslServer> valist = orgconfigs.contains( org ) ? orgconfigs.value( org ) : QList<QgsAuthConfigSslServer>();
    orgconfigs.insert( org, valist << config );
  }
  return orgconfigs;
}

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

QList<QSslCertificate> QgsAuthCertUtils::certsFromFile( const QString &certspath )
{
  QList<QSslCertificate> certs;
  bool pem = certspath.endsWith( ".pem", Qt::CaseInsensitive );
  certs = QSslCertificate::fromData( fileData_( certspath, pem ), pem ? QSsl::Pem : QSsl::Der );
  if ( certs.isEmpty() )
  {
    QgsDebugMsg( QString( "Parsed cert(s) EMPTY for path: %1" ).arg( certspath ) );
  }
  return certs;
}

QSslCertificate QgsAuthCertUtils::certFromFile( const QString &certpath )
{
  QSslCertificate cert;
  QList<QSslCertificate> certs( QgsAuthCertUtils::certsFromFile( certpath ) );
  if ( !certs.isEmpty() )
  {
    cert = certs.first();
  }
  if ( cert.isNull() )
  {
    QgsDebugMsg( QString( "Parsed cert is NULL for path: %1" ).arg( certpath ) );
  }
  return cert;
}

QSslKey QgsAuthCertUtils::keyFromFile( const QString &keypath,
                                       const QString &keypass,
                                       QString *algtype )
{
  bool pem = keypath.endsWith( ".pem", Qt::CaseInsensitive );
  QByteArray keydata( fileData_( keypath, pem ) );

  QSslKey clientkey;
  clientkey = QSslKey( keydata,
                       QSsl::Rsa,
                       pem ? QSsl::Pem : QSsl::Der,
                       QSsl::PrivateKey,
                       !keypass.isEmpty() ? keypass.toUtf8() : QByteArray() );
  if ( clientkey.isNull() )
  {
    // try DSA algorithm, since Qt can't seem to determine it otherwise
    clientkey = QSslKey( keydata,
                         QSsl::Dsa,
                         pem ? QSsl::Pem : QSsl::Der,
                         QSsl::PrivateKey,
                         !keypass.isEmpty() ? keypass.toUtf8() : QByteArray() );
    if ( clientkey.isNull() )
    {
      return QSslKey();
    }
    if ( algtype )
      *algtype = "dsa";
  }
  else
  {
    if ( algtype )
      *algtype = "rsa";
  }

  return clientkey;
}

QList<QSslCertificate> QgsAuthCertUtils::certsFromString( const QString &pemtext )
{
  QList<QSslCertificate> certs;
  certs = QSslCertificate::fromData( pemtext.toAscii(), QSsl::Pem );
  if ( certs.isEmpty() )
  {
    QgsDebugMsg( "Parsed cert(s) EMPTY" );
  }
  return certs;
}

QStringList QgsAuthCertUtils::certKeyBundleToPem( const QString &certpath,
    const QString &keypath,
    const QString &keypass,
    bool reencrypt )
{
  QString certpem;
  QSslCertificate clientcert = QgsAuthCertUtils::certFromFile( certpath );
  if ( !clientcert.isNull() )
  {
    certpem = QString( clientcert.toPem() );
  }

  QString keypem;
  QString algtype;
  QSslKey clientkey = QgsAuthCertUtils::keyFromFile( keypath, keypass, &algtype );

  // reapply passphrase if protection is requested and passphrase exists
  if ( !clientkey.isNull() )
  {
    keypem = QString( clientkey.toPem(( reencrypt && !keypass.isEmpty() ) ? keypass.toUtf8() : QByteArray() ) );
  }

  return QStringList() << certpem << keypem << algtype;
}

QStringList QgsAuthCertUtils::pkcs12BundleToPem( const QString &bundlepath,
    const QString &bundlepass,
    bool reencrypt )
{
  QStringList empty;
  if ( !QCA::isSupported( "pkcs12" ) )
    return empty;

  QCA::KeyBundle bundle( QgsAuthCertUtils::qcaKeyBundle( bundlepath, bundlepass ) );
  if ( bundle.isNull() )
    return empty;

  QCA::SecureArray passarray;
  if ( reencrypt && !bundlepass.isEmpty() )
    passarray = QCA::SecureArray( bundlepass.toUtf8() );

  QString algtype;
  if ( bundle.privateKey().isRSA() )
  {
    algtype = "rsa";
  }
  else if ( bundle.privateKey().isDSA() )
  {
    algtype = "dsa";
  }
  else if ( bundle.privateKey().isDH() )
  {
    algtype = "dh";
  }

  return QStringList() << bundle.certificateChain().primary().toPEM() << bundle.privateKey().toPEM( passarray ) << algtype;
}

QString QgsAuthCertUtils::pemTextToTempFile( const QString &name, const QByteArray &pemtext )
{
  QFile pemFile( QDir::tempPath() + QDir::separator() + name );
  QString pemFilePath( pemFile.fileName() );

  if ( pemFile.open( QIODevice::WriteOnly ) )
  {
    qint64 bytesWritten = pemFile.write( pemtext );
    if ( bytesWritten == -1 )
    {
      QgsDebugMsg( QString( "FAILED to write to temp PEM file: %1" ).arg( pemFilePath ) );
      pemFilePath.clear();
    }
  }
  else
  {
    QgsDebugMsg( QString( "FAILED to open writing for temp PEM file: %1" ).arg( pemFilePath ) );
    pemFilePath.clear();
  }

  if ( !pemFile.setPermissions( QFile::ReadUser ) )
  {
    QgsDebugMsg( QString( "FAILED to set permissions on temp PEM file: %1" ).arg( pemFilePath ) );
    pemFilePath.clear();
  }

  return pemFilePath;
}

QString QgsAuthCertUtils::getCaSourceName( QgsAuthCertUtils::CaCertSource source, bool single )
{
  switch ( source )
  {
    case SystemRoot:
      return single ? QObject::tr( "System Root CA" ) : QObject::tr( "System Root Authorities" );
    case FromFile:
      return single ? QObject::tr( "File CA" ) : QObject::tr( "Authorities from File" );
    case InDatabase:
      return single ? QObject::tr( "Database CA" ) : QObject::tr( "Authorities in Database" );
    case Connection:
      return single ? QObject::tr( "Connection CA" ) : QObject::tr( "Authorities from connection" );
    default:
      return QString();
  }
}

QString QgsAuthCertUtils::resolvedCertName( const QSslCertificate &cert, bool issuer )
{
  QString name( issuer ? SSL_ISSUER_INFO( cert, QSslCertificate::CommonName )
                : SSL_SUBJECT_INFO( cert, QSslCertificate::CommonName ) );

  if ( name.isEmpty() )
    name = issuer ? SSL_ISSUER_INFO( cert, QSslCertificate::OrganizationalUnitName )
           : SSL_SUBJECT_INFO( cert, QSslCertificate::OrganizationalUnitName );

  if ( name.isEmpty() )
    name = issuer ? SSL_ISSUER_INFO( cert, QSslCertificate::Organization )
           : SSL_SUBJECT_INFO( cert, QSslCertificate::Organization );

  if ( name.isEmpty() )
    name = issuer ? SSL_ISSUER_INFO( cert, QSslCertificate::LocalityName )
           : SSL_SUBJECT_INFO( cert, QSslCertificate::LocalityName );

  if ( name.isEmpty() )
    name = issuer ? SSL_ISSUER_INFO( cert, QSslCertificate::StateOrProvinceName )
           : SSL_SUBJECT_INFO( cert, QSslCertificate::StateOrProvinceName );

  if ( name.isEmpty() )
    name = issuer ? SSL_ISSUER_INFO( cert, QSslCertificate::CountryName )
           : SSL_SUBJECT_INFO( cert, QSslCertificate::CountryName );

  return name;
}

// private
void QgsAuthCertUtils::appendDirSegment_( QStringList &dirname,
    const QString& segment, QString value )
{
  if ( !value.isEmpty() )
  {
    dirname.append( segment + '=' + value.replace( ',', "\\," ) );
  }
}

QString QgsAuthCertUtils::getCertDistinguishedName( const QSslCertificate &qcert ,
    const QCA::Certificate &acert ,
    bool issuer )
{
  if ( QgsAuthManager::instance()->isDisabled() )
    return QString();

  if ( acert.isNull() )
  {
    QCA::ConvertResult res;
    QCA::Certificate acert( QCA::Certificate::fromPEM( qcert.toPem(), &res, QString( "qca-ossl" ) ) );
    if ( res != QCA::ConvertGood || acert.isNull() )
    {
      QgsDebugMsg( "Certificate could not be converted to QCA cert" );
      return QString();
    }
  }
  //  E=testcert@boundlessgeo.com,
  //  CN=Boundless Test Root CA,
  //  OU=Certificate Authority,
  //  O=Boundless Test CA,
  //  L=District of Columbia,
  //  ST=Washington\, DC,
  //  C=US
  QStringList dirname;
  QgsAuthCertUtils::appendDirSegment_(
    dirname, "E", issuer ? acert.issuerInfo().value( QCA::Email )
    : acert.subjectInfo().value( QCA::Email ) );
  QgsAuthCertUtils::appendDirSegment_(
    dirname, "CN", issuer ? SSL_ISSUER_INFO( qcert, QSslCertificate::CommonName )
    : SSL_SUBJECT_INFO( qcert, QSslCertificate::CommonName ) );
  QgsAuthCertUtils::appendDirSegment_(
    dirname, "OU", issuer ? SSL_ISSUER_INFO( qcert, QSslCertificate::OrganizationalUnitName )
    : SSL_SUBJECT_INFO( qcert, QSslCertificate::OrganizationalUnitName ) );
  QgsAuthCertUtils::appendDirSegment_(
    dirname, "O", issuer ? SSL_ISSUER_INFO( qcert, QSslCertificate::Organization )
    : SSL_SUBJECT_INFO( qcert, QSslCertificate::Organization ) );
  QgsAuthCertUtils::appendDirSegment_(
    dirname, "L", issuer ? SSL_ISSUER_INFO( qcert, QSslCertificate::LocalityName )
    : SSL_SUBJECT_INFO( qcert, QSslCertificate::LocalityName ) );
  QgsAuthCertUtils::appendDirSegment_(
    dirname, "ST", issuer ? SSL_ISSUER_INFO( qcert, QSslCertificate::StateOrProvinceName )
    : SSL_SUBJECT_INFO( qcert, QSslCertificate::StateOrProvinceName ) );
  QgsAuthCertUtils::appendDirSegment_(
    dirname, "C", issuer ? SSL_ISSUER_INFO( qcert, QSslCertificate::CountryName )
    : SSL_SUBJECT_INFO( qcert, QSslCertificate::CountryName ) );

  return dirname.join( "," );
}

QString QgsAuthCertUtils::getCertTrustName( QgsAuthCertUtils::CertTrustPolicy trust )
{
  switch ( trust )
  {
    case DefaultTrust:
      return QObject::tr( "Default" );
    case Trusted:
      return QObject::tr( "Trusted" );
    case Untrusted:
      return QObject::tr( "Untrusted" );
    default:
      return QString();
  }
}

QString QgsAuthCertUtils::getColonDelimited( const QString &txt )
{
  // 64321c05b0ebab8e2b67ec0d7d9e2b6d4bc3c303
  //   -> 64:32:1c:05:b0:eb:ab:8e:2b:67:ec:0d:7d:9e:2b:6d:4b:c3:c3:03
  QStringList sl;
  sl.reserve( txt.size() );
  for ( int i = 0; i < txt.size(); i += 2 )
  {
    sl << txt.mid( i, ( i + 2 > txt.size() ) ? -1 : 2 );
  }
  return sl.join( ":" );
}

QString QgsAuthCertUtils::shaHexForCert( const QSslCertificate& cert, bool formatted )
{
  QString sha( cert.digest( QCryptographicHash::Sha1 ).toHex() );
  if ( formatted )
  {
    return QgsAuthCertUtils::getColonDelimited( sha );
  }
  return sha;
}

QCA::Certificate QgsAuthCertUtils::qtCertToQcaCert( const QSslCertificate &cert )
{
  if ( QgsAuthManager::instance()->isDisabled() )
    return QCA::Certificate();

  QCA::ConvertResult res;
  QCA::Certificate qcacert( QCA::Certificate::fromPEM( cert.toPem(), &res, QString( "qca-ossl" ) ) );
  if ( res != QCA::ConvertGood || qcacert.isNull() )
  {
    QgsDebugMsg( "Certificate could not be converted to QCA cert" );
    qcacert = QCA::Certificate();
  }
  return qcacert;
}

QCA::CertificateCollection QgsAuthCertUtils::qtCertsToQcaCollection( const QList<QSslCertificate> &certs )
{
  QCA::CertificateCollection qcacoll;
  if ( QgsAuthManager::instance()->isDisabled() )
    return qcacoll;

  Q_FOREACH ( const QSslCertificate& cert, certs )
  {
    QCA::Certificate qcacert( qtCertToQcaCert( cert ) );
    if ( !qcacert.isNull() )
    {
      qcacoll.addCertificate( qcacert );
    }
  }
  return qcacoll;
}

QCA::KeyBundle QgsAuthCertUtils::qcaKeyBundle( const QString &path, const QString &pass )
{
  QCA::SecureArray passarray;
  if ( !pass.isEmpty() )
    passarray = QCA::SecureArray( pass.toUtf8() );

  QCA::ConvertResult res;
  QCA::KeyBundle bundle( QCA::KeyBundle::fromFile( path, passarray, &res, QString( "qca-ossl" ) ) );

  return ( res == QCA::ConvertGood ? bundle : QCA::KeyBundle() );
}

QString QgsAuthCertUtils::qcaValidityMessage( QCA::Validity validity )
{
  switch ( validity )
  {
    case QCA::ValidityGood:
      return QObject::tr( "Certificate is valid." );
    case QCA::ErrorRejected:
      return QObject::tr( "Root CA rejected the certificate purpose." );
    case QCA::ErrorUntrusted:
      return QObject::tr( "Certificate is not trusted." );
    case QCA::ErrorSignatureFailed:
      return QObject::tr( "Signature does not match." );
    case QCA::ErrorInvalidCA:
      return QObject::tr( "Certificate Authority is invalid or not found." );
    case QCA::ErrorInvalidPurpose:
      return QObject::tr( "Purpose does not match the intended usage." );
    case QCA::ErrorSelfSigned:
      return QObject::tr( "Certificate is self-signed, and is not found in the list of trusted certificates." );
    case QCA::ErrorRevoked:
      return QObject::tr( "Certificate has been revoked." );
    case QCA::ErrorPathLengthExceeded:
      return QObject::tr( "Path length from the root CA to this certificate is too long." );
    case QCA::ErrorExpired:
      return QObject::tr( "Certificate has expired or is not yet valid." );
    case QCA::ErrorExpiredCA:
      return QObject::tr( "Certificate Authority has expired." );
    case QCA::ErrorValidityUnknown:
      return QObject::tr( "Validity is unknown." );
    default:
      return QString();
  }
}

QString QgsAuthCertUtils::qcaSignatureAlgorithm( QCA::SignatureAlgorithm algorithm )
{
  switch ( algorithm )
  {
    case QCA::EMSA1_SHA1:
      return QObject::tr( "SHA1, with EMSA1" );
    case QCA::EMSA3_SHA1:
      return QObject::tr( "SHA1, with EMSA3" );
    case QCA::EMSA3_MD5:
      return QObject::tr( "MD5, with EMSA3" );
    case QCA::EMSA3_MD2:
      return QObject::tr( "MD2, with EMSA3" );
    case QCA::EMSA3_RIPEMD160:
      return QObject::tr( "RIPEMD160, with EMSA3" );
    case QCA::EMSA3_Raw:
      return QObject::tr( "EMSA3, without digest" );
#if QCA_VERSION >= 0x020100
    case QCA::EMSA3_SHA224:
      return QObject::tr( "SHA224, with EMSA3" );
    case QCA::EMSA3_SHA256:
      return QObject::tr( "SHA256, with EMSA3" );
    case QCA::EMSA3_SHA384:
      return QObject::tr( "SHA384, with EMSA3" );
    case QCA::EMSA3_SHA512:
      return QObject::tr( "SHA512, with EMSA3" );
#endif
    default:
      return QObject::tr( "Unknown (possibly Elliptic Curve)" );
  }
}

QString QgsAuthCertUtils::qcaKnownConstraint( QCA::ConstraintTypeKnown constraint )
{
  switch ( constraint )
  {
    case QCA::DigitalSignature:
      return QObject::tr( "Digital Signature" );
    case QCA::NonRepudiation:
      return QObject::tr( "Non-repudiation" );
    case QCA::KeyEncipherment:
      return QObject::tr( "Key Encipherment" );
    case QCA::DataEncipherment:
      return QObject::tr( "Data Encipherment" );
    case QCA::KeyAgreement:
      return QObject::tr( "Key Agreement" );
    case QCA::KeyCertificateSign:
      return QObject::tr( "Key Certificate Sign" );
    case QCA::CRLSign:
      return QObject::tr( "CRL Sign" );
    case QCA::EncipherOnly:
      return QObject::tr( "Encipher Only" );
    case QCA::DecipherOnly:
      return QObject::tr( "Decipher Only" );
    case QCA::ServerAuth:
      return QObject::tr( "Server Authentication" );
    case QCA::ClientAuth:
      return QObject::tr( "Client Authentication" );
    case QCA::CodeSigning:
      return QObject::tr( "Code Signing" );
    case QCA::EmailProtection:
      return QObject::tr( "Email Protection" );
    case QCA::IPSecEndSystem:
      return QObject::tr( "IPSec Endpoint" );
    case QCA::IPSecTunnel:
      return QObject::tr( "IPSec Tunnel" );
    case QCA::IPSecUser:
      return QObject::tr( "IPSec User" );
    case QCA::TimeStamping:
      return QObject::tr( "Time Stamping" );
    case QCA::OCSPSigning:
      return QObject::tr( "OCSP Signing" );
    default:
      return QString();
  }
}

QString QgsAuthCertUtils::certificateUsageTypeString( QgsAuthCertUtils::CertUsageType usagetype )
{
  switch ( usagetype )
  {
    case QgsAuthCertUtils::AnyOrUnspecifiedUsage:
      return QObject::tr( "Any or unspecified" );
    case QgsAuthCertUtils::CertAuthorityUsage:
      return QObject::tr( "Certificate Authority" );
    case QgsAuthCertUtils::CertIssuerUsage:
      return QObject::tr( "Certificate Issuer" );
    case QgsAuthCertUtils::TlsServerUsage:
      return QObject::tr( "TLS/SSL Server" );
    case QgsAuthCertUtils::TlsServerEvUsage:
      return QObject::tr( "TLS/SSL Server EV" );
    case QgsAuthCertUtils::TlsClientUsage:
      return QObject::tr( "TLS/SSL Client" );
    case QgsAuthCertUtils::CodeSigningUsage:
      return QObject::tr( "Code Signing" );
    case QgsAuthCertUtils::EmailProtectionUsage:
      return QObject::tr( "Email Protection" );
    case QgsAuthCertUtils::TimeStampingUsage:
      return QObject::tr( "Time Stamping" );
    case QgsAuthCertUtils::CRLSigningUsage:
      return QObject::tr( "CRL Signing" );
    case QgsAuthCertUtils::UndeterminedUsage:
    default:
      return QObject::tr( "Undetermined usage" );
  }
}

QList<QgsAuthCertUtils::CertUsageType> QgsAuthCertUtils::certificateUsageTypes( const QSslCertificate &cert )
{
  QList<QgsAuthCertUtils::CertUsageType> usages;

  if ( QgsAuthManager::instance()->isDisabled() )
    return usages;

  QCA::ConvertResult res;
  QCA::Certificate qcacert( QCA::Certificate::fromPEM( cert.toPem(), &res, QString( "qca-ossl" ) ) );
  if ( res != QCA::ConvertGood || qcacert.isNull() )
  {
    QgsDebugMsg( "Certificate could not be converted to QCA cert" );
    return usages;
  }

  if ( qcacert.isCA() )
  {
    QgsDebugMsg( "Certificate has 'CA:TRUE' basic constraint" );
    usages << QgsAuthCertUtils::CertAuthorityUsage;
  }

  QList<QCA::ConstraintType> certconsts = qcacert.constraints();
  Q_FOREACH ( const QCA::ConstraintType& certconst, certconsts )
  {
    if ( certconst.known() == QCA::KeyCertificateSign )
    {
      QgsDebugMsg( "Certificate has 'Certificate Sign' key usage" );
      usages << QgsAuthCertUtils::CertIssuerUsage;
    }
    else if ( certconst.known() == QCA::ServerAuth )
    {
      QgsDebugMsg( "Certificate has 'server authentication' extended key usage" );
      usages << QgsAuthCertUtils::TlsServerUsage;
    }
  }

  // ask QCA what it thinks about potential usages
  QCA::CertificateCollection trustedCAs(
    qtCertsToQcaCollection( QgsAuthManager::instance()->getTrustedCaCertsCache() ) );
  QCA::CertificateCollection untrustedCAs(
    qtCertsToQcaCollection( QgsAuthManager::instance()->getUntrustedCaCerts() ) );

  QCA::Validity v_any;
  v_any = qcacert.validate( trustedCAs, untrustedCAs, QCA::UsageAny, QCA::ValidateAll );
  if ( v_any == QCA::ValidityGood )
  {
    usages << QgsAuthCertUtils::AnyOrUnspecifiedUsage;
  }

  QCA::Validity v_tlsserver;
  v_tlsserver = qcacert.validate( trustedCAs, untrustedCAs, QCA::UsageTLSServer, QCA::ValidateAll );
  if ( v_tlsserver == QCA::ValidityGood )
  {
    if ( !usages.contains( QgsAuthCertUtils::TlsServerUsage ) )
    {
      usages << QgsAuthCertUtils::TlsServerUsage;
    }
  }

  // TODO: why doesn't this tag client certs?
  //       always seems to return QCA::ErrorInvalidPurpose (enum #5)
  QCA::Validity v_tlsclient;
  v_tlsclient = qcacert.validate( trustedCAs, untrustedCAs, QCA::UsageTLSClient, QCA::ValidateAll );
  //QgsDebugMsg( QString( "QCA::UsageTLSClient validity: %1" ).arg( ( int )v_tlsclient ) );
  if ( v_tlsclient == QCA::ValidityGood )
  {
    usages << QgsAuthCertUtils::TlsClientUsage;
  }

  // TODO: add TlsServerEvUsage, CodeSigningUsage, EmailProtectionUsage, TimeStampingUsage, CRLSigningUsage
  //       as they become necessary, since we do not want the overhead of checking just yet.

  return usages;
}

bool QgsAuthCertUtils::certificateIsAuthority( const QSslCertificate &cert )
{
  return QgsAuthCertUtils::certificateUsageTypes( cert ).contains( QgsAuthCertUtils::CertAuthorityUsage );
}

bool QgsAuthCertUtils::certificateIsIssuer( const QSslCertificate &cert )
{
  return QgsAuthCertUtils::certificateUsageTypes( cert ).contains( QgsAuthCertUtils::CertIssuerUsage );
}

bool QgsAuthCertUtils::certificateIsAuthorityOrIssuer( const QSslCertificate &cert )
{
  return ( QgsAuthCertUtils::certificateIsAuthority( cert )
           || QgsAuthCertUtils::certificateIsIssuer( cert ) );
}

bool QgsAuthCertUtils::certificateIsSslServer( const QSslCertificate &cert )
{
  return ( QgsAuthCertUtils::certificateUsageTypes( cert ).contains( QgsAuthCertUtils::TlsServerUsage )
           || QgsAuthCertUtils::certificateUsageTypes( cert ).contains( QgsAuthCertUtils::TlsServerEvUsage ) );
}

#if 0
bool QgsAuthCertUtils::certificateIsSslServer( const QSslCertificate &cert )
{
  // TODO: There is no difinitive method for strictly enforcing what determines an SSL server cert;
  //       only what it should not be able to do (cert sign, etc.). The logic here may need refined
  // see: http://security.stackexchange.com/a/26650

  if ( QgsAuthManager::instance()->isDisabled() )
    return false;

  QCA::ConvertResult res;
  QCA::Certificate qcacert( QCA::Certificate::fromPEM( cert.toPem(), &res, QString( "qca-ossl" ) ) );
  if ( res != QCA::ConvertGood || qcacert.isNull() )
  {
    QgsDebugMsg( "Certificate could not be converted to QCA cert" );
    return false;
  }

  if ( qcacert.isCA() )
  {
    QgsDebugMsg( "SSL server certificate has 'CA:TRUE' basic constraint (and should not)" );
    return false;
  }

  QList<QCA::ConstraintType> certconsts = qcacert.constraints();
  Q_FOREACH ( QCA::ConstraintType certconst, certconsts )
  {
    if ( certconst.known() == QCA::KeyCertificateSign )
    {
      QgsDebugMsg( "SSL server certificate has 'Certificate Sign' key usage (and should not)" );
      return false;
    }
  }

  // check for common key usage and extended key usage constraints
  // see: https://www.ietf.org/rfc/rfc3280.txt  4.2.1.3(Key Usage) and  4.2.1.13(Extended Key Usage)
  bool serverauth = false;
  bool dsignature = false;
  bool keyencrypt = false;
  Q_FOREACH ( QCA::ConstraintType certconst, certconsts )
  {
    if ( certconst.known() == QCA::DigitalSignature )
    {
      QgsDebugMsg( "SSL server certificate has 'digital signature' key usage" );
      dsignature = true;
    }
    else if ( certconst.known() == QCA::KeyEncipherment )
    {
      QgsDebugMsg( "SSL server certificate has 'key encipherment' key usage" );
      keyencrypt = true;
    }
    else if ( certconst.known() == QCA::KeyAgreement )
    {
      QgsDebugMsg( "SSL server certificate has 'key agreement' key usage" );
      keyencrypt = true;
    }
    else if ( certconst.known() == QCA::ServerAuth )
    {
      QgsDebugMsg( "SSL server certificate has 'server authentication' extended key usage" );
      serverauth = true;
    }
  }
  // From 4.2.1.13(Extended Key Usage):
  //   "If a certificate contains both a key usage extension and an extended
  //   key usage extension, then both extensions MUST be processed
  //   independently and the certificate MUST only be used for a purpose
  //   consistent with both extensions.  If there is no purpose consistent
  //   with both extensions, then the certificate MUST NOT be used for any
  //   purpose."

  if ( serverauth && dsignature && keyencrypt )
  {
    return true;
  }
  if ( dsignature && keyencrypt )
  {
    return true;
  }

  // lastly, check for DH key and key agreement
  bool keyagree = false;
  bool encipheronly = false;
  bool decipheronly = false;

  QCA::PublicKey pubkey( qcacert.subjectPublicKey() );
  // key size may be 0 for eliptical curve-based keys, in which case isDH() crashes QCA
  if ( pubkey.bitSize() > 0 && pubkey.isDH() )
  {
    keyagree = pubkey.canKeyAgree();
    if ( !keyagree )
    {
      return false;
    }
    Q_FOREACH ( QCA::ConstraintType certconst, certconsts )
    {
      if ( certconst.known() == QCA::EncipherOnly )
      {
        QgsDebugMsg( "SSL server public key has 'encipher only' key usage" );
        encipheronly = true;
      }
      else if ( certconst.known() == QCA::DecipherOnly )
      {
        QgsDebugMsg( "SSL server public key has 'decipher only' key usage" );
        decipheronly = true;
      }
    }
    if ( !encipheronly && !decipheronly )
    {
      return true;
    }
  }
  return false;
}
#endif

bool QgsAuthCertUtils::certificateIsSslClient( const QSslCertificate &cert )
{
  return QgsAuthCertUtils::certificateUsageTypes( cert ).contains( QgsAuthCertUtils::TlsClientUsage );
}

QString QgsAuthCertUtils::sslErrorEnumString( QSslError::SslError errenum )
{
  switch ( errenum )
  {
    case QSslError::UnableToGetIssuerCertificate:
      return QObject::tr( "Unable To Get Issuer Certificate" );
    case QSslError::UnableToDecryptCertificateSignature:
      return QObject::tr( "Unable To Decrypt Certificate Signature" );
    case QSslError::UnableToDecodeIssuerPublicKey:
      return QObject::tr( "Unable To Decode Issuer Public Key" );
    case QSslError::CertificateSignatureFailed:
      return QObject::tr( "Certificate Signature Failed" );
    case QSslError::CertificateNotYetValid:
      return QObject::tr( "Certificate Not Yet Valid" );
    case QSslError::CertificateExpired:
      return QObject::tr( "Certificate Expired" );
    case QSslError::InvalidNotBeforeField:
      return QObject::tr( "Invalid Not Before Field" );
    case QSslError::InvalidNotAfterField:
      return QObject::tr( "Invalid Not After Field" );
    case QSslError::SelfSignedCertificate:
      return QObject::tr( "Self-signed Certificate" );
    case QSslError::SelfSignedCertificateInChain:
      return QObject::tr( "Self-signed Certificate In Chain" );
    case QSslError::UnableToGetLocalIssuerCertificate:
      return QObject::tr( "Unable To Get Local Issuer Certificate" );
    case QSslError::UnableToVerifyFirstCertificate:
      return QObject::tr( "Unable To Verify First Certificate" );
    case QSslError::CertificateRevoked:
      return QObject::tr( "Certificate Revoked" );
    case QSslError::InvalidCaCertificate:
      return QObject::tr( "Invalid CA Certificate" );
    case QSslError::PathLengthExceeded:
      return QObject::tr( "Path Length Exceeded" );
    case QSslError::InvalidPurpose:
      return QObject::tr( "Invalid Purpose" );
    case QSslError::CertificateUntrusted:
      return QObject::tr( "Certificate Untrusted" );
    case QSslError::CertificateRejected:
      return QObject::tr( "Certificate Rejected" );
    case QSslError::SubjectIssuerMismatch:
      return QObject::tr( "Subject Issuer Mismatch" );
    case QSslError::AuthorityIssuerSerialNumberMismatch:
      return QObject::tr( "Authority Issuer Serial Number Mismatch" );
    case QSslError::NoPeerCertificate:
      return QObject::tr( "No Peer Certificate" );
    case QSslError::HostNameMismatch:
      return QObject::tr( "Host Name Mismatch" );
    case QSslError::UnspecifiedError:
      return QObject::tr( "Unspecified Error" );
    case QSslError::CertificateBlacklisted:
      return QObject::tr( "Certificate Blacklisted" );
    case QSslError::NoError:
      return QObject::tr( "No Error" );
    case QSslError::NoSslSupport:
      return QObject::tr( "No SSL Support" );
    default:
      return QString();
  }
}

QList<QPair<QSslError::SslError, QString> > QgsAuthCertUtils::sslErrorEnumStrings()
{
  QList<QPair<QSslError::SslError, QString> > errenums;
  errenums << qMakePair( QSslError::UnableToGetIssuerCertificate,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::UnableToGetIssuerCertificate ) );
  errenums << qMakePair( QSslError::UnableToDecryptCertificateSignature,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::UnableToDecryptCertificateSignature ) );
  errenums << qMakePair( QSslError::UnableToDecodeIssuerPublicKey,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::UnableToDecodeIssuerPublicKey ) );
  errenums << qMakePair( QSslError::CertificateSignatureFailed,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::CertificateSignatureFailed ) );
  errenums << qMakePair( QSslError::CertificateNotYetValid,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::CertificateNotYetValid ) );
  errenums << qMakePair( QSslError::CertificateExpired,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::CertificateExpired ) );
  errenums << qMakePair( QSslError::InvalidNotBeforeField,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::InvalidNotBeforeField ) );
  errenums << qMakePair( QSslError::InvalidNotAfterField,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::InvalidNotAfterField ) );
  errenums << qMakePair( QSslError::SelfSignedCertificate,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::SelfSignedCertificate ) );
  errenums << qMakePair( QSslError::SelfSignedCertificateInChain,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::SelfSignedCertificateInChain ) );
  errenums << qMakePair( QSslError::UnableToGetLocalIssuerCertificate,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::UnableToGetLocalIssuerCertificate ) );
  errenums << qMakePair( QSslError::UnableToVerifyFirstCertificate,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::UnableToVerifyFirstCertificate ) );
  errenums << qMakePair( QSslError::CertificateRevoked,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::CertificateRevoked ) );
  errenums << qMakePair( QSslError::InvalidCaCertificate,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::InvalidCaCertificate ) );
  errenums << qMakePair( QSslError::PathLengthExceeded,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::PathLengthExceeded ) );
  errenums << qMakePair( QSslError::InvalidPurpose,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::InvalidPurpose ) );
  errenums << qMakePair( QSslError::CertificateUntrusted,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::CertificateUntrusted ) );
  errenums << qMakePair( QSslError::CertificateRejected,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::CertificateRejected ) );
  errenums << qMakePair( QSslError::SubjectIssuerMismatch,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::SubjectIssuerMismatch ) );
  errenums << qMakePair( QSslError::AuthorityIssuerSerialNumberMismatch,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::AuthorityIssuerSerialNumberMismatch ) );
  errenums << qMakePair( QSslError::NoPeerCertificate,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::NoPeerCertificate ) );
  errenums << qMakePair( QSslError::HostNameMismatch,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::HostNameMismatch ) );
  errenums << qMakePair( QSslError::UnspecifiedError,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::UnspecifiedError ) );
  errenums << qMakePair( QSslError::CertificateBlacklisted,
                         QgsAuthCertUtils::sslErrorEnumString( QSslError::CertificateBlacklisted ) );
  return errenums;
}
