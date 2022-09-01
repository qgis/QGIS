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

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"

#ifdef Q_OS_MAC
#include <string.h>
#include "libtasn1.h"
#endif


QString QgsAuthCertUtils::getSslProtocolName( QSsl::SslProtocol protocol )
{
  switch ( protocol )
  {
    case QSsl::SecureProtocols:
      return QObject::tr( "SecureProtocols" );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    case QSsl::TlsV1SslV3:
      return QObject::tr( "TlsV1SslV3" );
#endif
    case QSsl::TlsV1_0:
      return QObject::tr( "TlsV1" );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    // not supported by Qt 5.15+
    case QSsl::SslV3:
      return QObject::tr( "SslV3" );
    case QSsl::SslV2:
      return QObject::tr( "SslV2" );
#endif
    default:
      return QString();
  }
}

QMap<QString, QSslCertificate> QgsAuthCertUtils::mapDigestToCerts( const QList<QSslCertificate> &certs )
{
  QMap<QString, QSslCertificate> digestmap;
  for ( const auto &cert : certs )
  {
    digestmap.insert( shaHexForCert( cert ), cert );
  }
  return digestmap;
}

QMap<QString, QList<QSslCertificate> > QgsAuthCertUtils::certsGroupedByOrg( const QList<QSslCertificate> &certs )
{
  QMap< QString, QList<QSslCertificate> > orgcerts;
  for ( const auto &cert : certs )
  {
    QString org( SSL_SUBJECT_INFO( cert, QSslCertificate::Organization ) );
    if ( org.isEmpty() )
      org = QStringLiteral( "(Organization not defined)" );
    QList<QSslCertificate> valist = orgcerts.contains( org ) ? orgcerts.value( org ) : QList<QSslCertificate>();
    orgcerts.insert( org, valist << cert );
  }
  return orgcerts;
}

QMap<QString, QgsAuthConfigSslServer> QgsAuthCertUtils::mapDigestToSslConfigs( const QList<QgsAuthConfigSslServer> &configs )
{
  QMap<QString, QgsAuthConfigSslServer> digestmap;
  for ( const auto &config : configs )
  {
    digestmap.insert( shaHexForCert( config.sslCertificate() ), config );
  }
  return digestmap;
}

QMap<QString, QList<QgsAuthConfigSslServer> > QgsAuthCertUtils::sslConfigsGroupedByOrg( const QList<QgsAuthConfigSslServer> &configs )
{
  QMap< QString, QList<QgsAuthConfigSslServer> > orgconfigs;
  for ( const auto &config : configs )
  {
    QString org( SSL_SUBJECT_INFO( config.sslCertificate(), QSslCertificate::Organization ) );

    if ( org.isEmpty() )
      org = QObject::tr( "(Organization not defined)" );
    QList<QgsAuthConfigSslServer> valist = orgconfigs.contains( org ) ? orgconfigs.value( org ) : QList<QgsAuthConfigSslServer>();
    orgconfigs.insert( org, valist << config );
  }
  return orgconfigs;
}

QByteArray QgsAuthCertUtils::fileData( const QString &path )
{
  QByteArray data;
  QFile file( path );
  if ( !file.exists() )
  {
    QgsDebugMsg( QStringLiteral( "Read file error, file not found: %1" ).arg( path ) );
    return data;
  }
  // TODO: add checks for locked file, etc., to ensure it can be read
  const QFile::OpenMode openflags( QIODevice::ReadOnly );
  const bool ret = file.open( openflags );
  if ( ret )
  {
    data = file.readAll();
  }
  file.close();

  return data;
}

QList<QSslCertificate> QgsAuthCertUtils::certsFromFile( const QString &certspath )
{
  QList<QSslCertificate> certs;
  const QByteArray payload( QgsAuthCertUtils::fileData( certspath ) );
  certs = QSslCertificate::fromData( payload, sniffEncoding( payload ) );
  if ( certs.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Parsed cert(s) EMPTY for path: %1" ).arg( certspath ) );
  }
  return certs;
}

QList<QSslCertificate> QgsAuthCertUtils::casFromFile( const QString &certspath )
{
  QList<QSslCertificate> cas;
  const QList<QSslCertificate> certs( certsFromFile( certspath ) );
  for ( const auto &cert : certs )
  {
    if ( certificateIsAuthority( cert ) )
    {
      cas.append( cert );
    }
  }
  return cas;
}

QList<QSslCertificate> QgsAuthCertUtils::casMerge( const QList<QSslCertificate> &bundle1, const QList<QSslCertificate> &bundle2 )
{
  QStringList shas;
  QList<QSslCertificate> result( bundle1 );
  const QList<QSslCertificate> c_bundle1( bundle1 );
  for ( const auto &cert : c_bundle1 )
  {
    shas.append( shaHexForCert( cert ) );
  }
  const QList<QSslCertificate> c_bundle2( bundle2 );
  for ( const auto &cert : c_bundle2 )
  {
    if ( ! shas.contains( shaHexForCert( cert ) ) )
    {
      result.append( cert );
    }
  }
  return result;
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
    QgsDebugMsg( QStringLiteral( "Parsed cert is NULL for path: %1" ).arg( certpath ) );
  }
  return cert;
}

QSslKey QgsAuthCertUtils::keyFromFile( const QString &keypath,
                                       const QString &keypass,
                                       QString *algtype )
{
  // The approach here is to try all possible encodings and algorithms
  const QByteArray keydata( QgsAuthCertUtils::fileData( keypath ) );
  QSslKey clientkey;

  const QSsl::EncodingFormat keyEncoding( sniffEncoding( keydata ) );

  const std::vector<QSsl::KeyAlgorithm> algs
  {
    QSsl::KeyAlgorithm::Rsa,
    QSsl::KeyAlgorithm::Dsa,
    QSsl::KeyAlgorithm::Ec,
    QSsl::KeyAlgorithm::Opaque
  };

  for ( const auto &alg : algs )
  {
    clientkey = QSslKey( keydata,
                         alg,
                         keyEncoding,
                         QSsl::PrivateKey,
                         !keypass.isEmpty() ? keypass.toUtf8() : QByteArray() );
    if ( ! clientkey.isNull() )
    {
      if ( algtype )
      {
        switch ( alg )
        {
          case QSsl::KeyAlgorithm::Rsa:
            *algtype = QStringLiteral( "rsa" );
            break;
          case QSsl::KeyAlgorithm::Dsa:
            *algtype = QStringLiteral( "dsa" );
            break;
          case QSsl::KeyAlgorithm::Ec:
            *algtype = QStringLiteral( "ec" );
            break;
          case QSsl::KeyAlgorithm::Opaque:
            *algtype = QStringLiteral( "opaque" );
            break;
          case QSsl::KeyAlgorithm::Dh:
            *algtype = QStringLiteral( "dh" );
            break;
        }
      }
      return clientkey;
    }
  }
  return QSslKey();
}

QList<QSslCertificate> QgsAuthCertUtils::certsFromString( const QString &pemtext )
{
  QList<QSslCertificate> certs;
  certs = QSslCertificate::fromData( pemtext.toLatin1(), QSsl::Pem );
  if ( certs.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Parsed cert(s) EMPTY" ) );
  }
  return certs;
}

QList<QSslCertificate> QgsAuthCertUtils::casRemoveSelfSigned( const QList<QSslCertificate> &caList )
{
  QList<QSslCertificate> certs;
  for ( const auto &cert : caList )
  {
    if ( ! cert.isSelfSigned( ) )
    {
      certs.append( cert );
    }
  }
  return certs;
}

QStringList QgsAuthCertUtils::certKeyBundleToPem( const QString &certpath,
    const QString &keypath,
    const QString &keypass,
    bool reencrypt )
{
  QString certpem;
  const QSslCertificate clientcert = QgsAuthCertUtils::certFromFile( certpath );
  if ( !clientcert.isNull() )
  {
    certpem = QString( clientcert.toPem() );
  }

  QString keypem;
  QString algtype;
  const QSslKey clientkey = QgsAuthCertUtils::keyFromFile( keypath, keypass, &algtype );

  // reapply passphrase if protection is requested and passphrase exists
  if ( !clientkey.isNull() )
  {
    keypem = QString( clientkey.toPem( ( reencrypt && !keypass.isEmpty() ) ? keypass.toUtf8() : QByteArray() ) );
  }

  return QStringList() << certpem << keypem << algtype;
}

bool QgsAuthCertUtils::pemIsPkcs8( const QString &keyPemTxt )
{
  const QString pkcs8Header = QStringLiteral( "-----BEGIN PRIVATE KEY-----" );
  const QString pkcs8Footer = QStringLiteral( "-----END PRIVATE KEY-----" );
  return keyPemTxt.contains( pkcs8Header ) && keyPemTxt.contains( pkcs8Footer );
}

#ifdef Q_OS_MAC
QByteArray QgsAuthCertUtils::pkcs8PrivateKey( QByteArray &pkcs8Der )
{
  QByteArray pkcs1;

  if ( pkcs8Der.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "ERROR, passed DER is empty" ) );
    return pkcs1;
  }
  // Dump as unarmored PEM format, e.g. missing '-----BEGIN|END...' wrapper
  //QgsDebugMsg ( QStringLiteral( "pkcs8Der: %1" ).arg( QString( pkcs8Der.toBase64() ) ) );

  QFileInfo asnDefsRsrc( QgsApplication::pkgDataPath() + QStringLiteral( "/resources/pkcs8.asn" ) );
  if ( ! asnDefsRsrc.exists() )
  {
    QgsDebugMsg( QStringLiteral( "ERROR, pkcs.asn resource file not found: %1" ).arg( asnDefsRsrc.filePath() ) );
    return pkcs1;
  }
  const char *asnDefsFile = asnDefsRsrc.absoluteFilePath().toLocal8Bit().constData();

  int asn1_result = ASN1_SUCCESS, der_len = 0, oct_len = 0;
  asn1_node definitions = NULL, structure = NULL;
  char errorDescription[ASN1_MAX_ERROR_DESCRIPTION_SIZE], oct_data[1024];
  unsigned char *der = NULL;
  unsigned int flags = 0; //TODO: see if any or all ASN1_DECODE_FLAG_* flags can be set
  unsigned oct_etype;

  // Base PKCS#8 element to decode
  QString typeName( QStringLiteral( "PKCS-8.PrivateKeyInfo" ) );

  asn1_result = asn1_parser2tree( asnDefsFile, &definitions, errorDescription );

  switch ( asn1_result )
  {
    case ASN1_SUCCESS:
      QgsDebugMsgLevel( QStringLiteral( "Parse: done.\n" ), 4 );
      break;
    case ASN1_FILE_NOT_FOUND:
      QgsDebugMsg( QStringLiteral( "ERROR, file not found: %1" ).arg( asnDefsFile ) );
      return pkcs1;
    case ASN1_SYNTAX_ERROR:
    case ASN1_IDENTIFIER_NOT_FOUND:
    case ASN1_NAME_TOO_LONG:
      QgsDebugMsg( QStringLiteral( "ERROR, asn1 parsing: %1" ).arg( errorDescription ) );
      return pkcs1;
    default:
      QgsDebugMsg( QStringLiteral( "ERROR, libtasn1: %1" ).arg( asn1_strerror( asn1_result ) ) );
      return pkcs1;
  }

  // Generate the ASN.1 structure
  asn1_result = asn1_create_element( definitions, typeName.toLatin1().constData(), &structure );

  //asn1_print_structure( stdout, structure, "", ASN1_PRINT_ALL);

  if ( asn1_result != ASN1_SUCCESS )
  {
    QgsDebugMsg( QStringLiteral( "ERROR, structure creation: %1" ).arg( asn1_strerror( asn1_result ) ) );
    goto PKCS1DONE;
  }

  // Populate the ASN.1 structure with decoded DER data
  der = reinterpret_cast<unsigned char *>( pkcs8Der.data() );
  der_len = pkcs8Der.size();

  if ( flags != 0 )
  {
    asn1_result = asn1_der_decoding2( &structure, der, &der_len, flags, errorDescription );
  }
  else
  {
    asn1_result = asn1_der_decoding( &structure, der, der_len, errorDescription );
  }

  if ( asn1_result != ASN1_SUCCESS )
  {
    QgsDebugMsg( QStringLiteral( "ERROR, decoding: %1" ).arg( errorDescription ) );
    goto PKCS1DONE;
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Decoding: %1" ).arg( asn1_strerror( asn1_result ) ), 4 );
  }

  if ( QgsLogger::debugLevel() >= 4 )
  {
    QgsDebugMsg( QStringLiteral( "DECODING RESULT:" ) );
    asn1_print_structure( stdout, structure, "", ASN1_PRINT_NAME_TYPE_VALUE );
  }

  // Validate and extract privateKey value
  QgsDebugMsgLevel( QStringLiteral( "Validating privateKey type..." ), 4 );
  typeName.append( QStringLiteral( ".privateKey" ) );
  QgsDebugMsgLevel( QStringLiteral( "privateKey element name: %1" ).arg( typeName ), 4 );

  asn1_result = asn1_read_value_type( structure, "privateKey", NULL, &oct_len, &oct_etype );

  if ( asn1_result != ASN1_MEM_ERROR ) // not sure why ASN1_MEM_ERROR = success, but it does
  {
    QgsDebugMsg( QStringLiteral( "ERROR, asn1 read privateKey value type: %1" ).arg( asn1_strerror( asn1_result ) ) );
    goto PKCS1DONE;
  }

  if ( oct_etype != ASN1_ETYPE_OCTET_STRING )
  {
    QgsDebugMsg( QStringLiteral( "ERROR, asn1 privateKey value not octet string, but type: %1" ).arg( static_cast<int>( oct_etype ) ) );
    goto PKCS1DONE;
  }

  if ( oct_len == 0 )
  {
    QgsDebugMsg( QStringLiteral( "ERROR, asn1 privateKey octet string empty" ) );
    goto PKCS1DONE;
  }

  QgsDebugMsgLevel( QStringLiteral( "Reading privateKey value..." ), 4 );
  asn1_result = asn1_read_value( structure, "privateKey", oct_data, &oct_len );

  if ( asn1_result != ASN1_SUCCESS )
  {
    QgsDebugMsg( QStringLiteral( "ERROR, asn1 read privateKey value: %1" ).arg( asn1_strerror( asn1_result ) ) );
    goto PKCS1DONE;
  }

  if ( oct_len == 0 )
  {
    QgsDebugMsg( QStringLiteral( "ERROR, asn1 privateKey value octet string empty" ) );
    goto PKCS1DONE;
  }

  pkcs1 = QByteArray( oct_data, oct_len );

  // !!! SENSITIVE DATA - DO NOT LEAVE UNCOMMENTED !!!
  //QgsDebugMsgLevel( QStringLiteral( "privateKey octet data as PEM: %1" ).arg( QString( pkcs1.toBase64() ) ), 4 );

PKCS1DONE:

  asn1_delete_structure( &structure );
  return pkcs1;
}
#endif

QStringList QgsAuthCertUtils::pkcs12BundleToPem( const QString &bundlepath,
    const QString &bundlepass,
    bool reencrypt )
{
  QStringList empty;
  if ( !QCA::isSupported( "pkcs12" ) )
  {
    QgsDebugMsg( QStringLiteral( "QCA does not support PKCS#12" ) );
    return empty;
  }

  const QCA::KeyBundle bundle( QgsAuthCertUtils::qcaKeyBundle( bundlepath, bundlepass ) );
  if ( bundle.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "FAILED to convert PKCS#12 file to QCA key bundle: %1" ).arg( bundlepath ) );
    return empty;
  }

  QCA::SecureArray passarray;
  if ( reencrypt && !bundlepass.isEmpty() )
  {
    passarray = QCA::SecureArray( bundlepass.toUtf8() );
  }

  QString algtype;
  QSsl::KeyAlgorithm keyalg = QSsl::Opaque;
  if ( bundle.privateKey().isRSA() )
  {
    algtype = QStringLiteral( "rsa" );
    keyalg = QSsl::Rsa;
  }
  else if ( bundle.privateKey().isDSA() )
  {
    algtype = QStringLiteral( "dsa" );
    keyalg = QSsl::Dsa;
  }
  else if ( bundle.privateKey().isDH() )
  {
    algtype = QStringLiteral( "dh" );
  }
  // TODO: add support for EC keys, once QCA supports them

  // can currently only support RSA and DSA between QCA and Qt
  if ( keyalg == QSsl::Opaque )
  {
    QgsDebugMsg( QStringLiteral( "FAILED to read PKCS#12 key (only RSA and DSA algorithms supported): %1" ).arg( bundlepath ) );
    return empty;
  }

  QString keyPem;
#ifdef Q_OS_MAC
  if ( keyalg == QSsl::Rsa && QgsAuthCertUtils::pemIsPkcs8( bundle.privateKey().toPEM() ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "Private key is PKCS#8: attempting conversion to PKCS#1..." ), 4 );
    // if RSA, convert from PKCS#8 key to 'traditional' OpenSSL RSA format, which Qt prefers
    // note: QCA uses OpenSSL, regardless of the Qt SSL backend, and 1.0.2+ OpenSSL versions return
    //       RSA private keys as PKCS#8, which choke Qt upon QSslKey creation

    QByteArray pkcs8Der = bundle.privateKey().toDER().toByteArray();
    if ( pkcs8Der.isEmpty() )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to convert PKCS#12 key to DER-encoded format: %1" ).arg( bundlepath ) );
      return empty;
    }

    QByteArray pkcs1Der = QgsAuthCertUtils::pkcs8PrivateKey( pkcs8Der );
    if ( pkcs1Der.isEmpty() )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to convert PKCS#12 key from PKCS#8 to PKCS#1: %1" ).arg( bundlepath ) );
      return empty;
    }

    QSslKey pkcs1Key( pkcs1Der, QSsl::Rsa, QSsl::Der, QSsl::PrivateKey );
    if ( pkcs1Key.isNull() )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to convert PKCS#12 key from PKCS#8 to PKCS#1 QSslKey: %1" ).arg( bundlepath ) );
      return empty;
    }
    keyPem = QString( pkcs1Key.toPem( passarray.toByteArray() ) );
  }
  else
  {
    keyPem = bundle.privateKey().toPEM( passarray );
  }
#else
  keyPem = bundle.privateKey().toPEM( passarray );
#endif

  QgsDebugMsgLevel( QStringLiteral( "PKCS#12 cert as PEM:\n%1" ).arg( QString( bundle.certificateChain().primary().toPEM() ) ), 4 );
  // !!! SENSITIVE DATA - DO NOT LEAVE UNCOMMENTED !!!
  //QgsDebugMsgLevel( QStringLiteral( "PKCS#12 key as PEM:\n%1" ).arg( QString( keyPem ) ), 4 );

  return QStringList() << bundle.certificateChain().primary().toPEM() << keyPem << algtype;
}

QList<QSslCertificate> QgsAuthCertUtils::pkcs12BundleCas( const QString &bundlepath, const QString &bundlepass )
{
  QList<QSslCertificate> result;
  if ( !QCA::isSupported( "pkcs12" ) )
    return result;

  const QCA::KeyBundle bundle( QgsAuthCertUtils::qcaKeyBundle( bundlepath, bundlepass ) );
  if ( bundle.isNull() )
    return result;

  const QCA::CertificateChain chain( bundle.certificateChain() );
  for ( const auto &cert : chain )
  {
    if ( cert.isCA( ) )
    {
      result.append( QSslCertificate::fromData( cert.toPEM().toLatin1() ) );
    }
  }
  return result;
}

QByteArray QgsAuthCertUtils::certsToPemText( const QList<QSslCertificate> &certs )
{
  QByteArray capem;
  if ( !certs.isEmpty() )
  {
    QStringList certslist;
    for ( const auto &cert : certs )
    {
      certslist << cert.toPem();
    }
    capem = certslist.join( QLatin1Char( '\n' ) ).toLatin1(); //+ "\n";
  }
  return capem;
}

QString QgsAuthCertUtils::pemTextToTempFile( const QString &name, const QByteArray &pemtext )
{
  QFile pemFile( QDir::tempPath() + QDir::separator() + name );
  QString pemFilePath( pemFile.fileName() );

  if ( pemFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    const qint64 bytesWritten = pemFile.write( pemtext );
    if ( bytesWritten == -1 )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to write to temp PEM file: %1" ).arg( pemFilePath ) );
      pemFilePath.clear();
    }
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "FAILED to open writing for temp PEM file: %1" ).arg( pemFilePath ) );
    pemFilePath.clear();
  }

  if ( !pemFile.setPermissions( QFile::ReadUser ) )
  {
    QgsDebugMsg( QStringLiteral( "FAILED to set permissions on temp PEM file: %1" ).arg( pemFilePath ) );
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
    const QString &segment, QString value )
{
  if ( !value.isEmpty() )
  {
    dirname.append( segment + '=' + value.replace( ',', QLatin1String( "\\," ) ) );
  }
}

QSsl::EncodingFormat QgsAuthCertUtils::sniffEncoding( const QByteArray &payload )
{
  return payload.contains( QByteArrayLiteral( "-----BEGIN " ) ) ?
         QSsl::Pem :
         QSsl::Der;
}

QString QgsAuthCertUtils::getCertDistinguishedName( const QSslCertificate &qcert,
    const QCA::Certificate &acert,
    bool issuer )
{
  if ( QgsApplication::authManager()->isDisabled() )
    return QString();

  if ( acert.isNull() )
  {
    QCA::ConvertResult res;
    const QCA::Certificate acert( QCA::Certificate::fromPEM( qcert.toPem(), &res, QStringLiteral( "qca-ossl" ) ) );
    if ( res != QCA::ConvertGood || acert.isNull() )
    {
      QgsDebugMsg( QStringLiteral( "Certificate could not be converted to QCA cert" ) );
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
    dirname, QStringLiteral( "E" ), issuer ? acert.issuerInfo().value( QCA::Email )
    : acert.subjectInfo().value( QCA::Email ) );
  QgsAuthCertUtils::appendDirSegment_(
    dirname, QStringLiteral( "CN" ), issuer ? SSL_ISSUER_INFO( qcert, QSslCertificate::CommonName )
    : SSL_SUBJECT_INFO( qcert, QSslCertificate::CommonName ) );
  QgsAuthCertUtils::appendDirSegment_(
    dirname, QStringLiteral( "OU" ), issuer ? SSL_ISSUER_INFO( qcert, QSslCertificate::OrganizationalUnitName )
    : SSL_SUBJECT_INFO( qcert, QSslCertificate::OrganizationalUnitName ) );
  QgsAuthCertUtils::appendDirSegment_(
    dirname, QStringLiteral( "O" ), issuer ? SSL_ISSUER_INFO( qcert, QSslCertificate::Organization )
    : SSL_SUBJECT_INFO( qcert, QSslCertificate::Organization ) );
  QgsAuthCertUtils::appendDirSegment_(
    dirname, QStringLiteral( "L" ), issuer ? SSL_ISSUER_INFO( qcert, QSslCertificate::LocalityName )
    : SSL_SUBJECT_INFO( qcert, QSslCertificate::LocalityName ) );
  QgsAuthCertUtils::appendDirSegment_(
    dirname, QStringLiteral( "ST" ), issuer ? SSL_ISSUER_INFO( qcert, QSslCertificate::StateOrProvinceName )
    : SSL_SUBJECT_INFO( qcert, QSslCertificate::StateOrProvinceName ) );
  QgsAuthCertUtils::appendDirSegment_(
    dirname, QStringLiteral( "C" ), issuer ? SSL_ISSUER_INFO( qcert, QSslCertificate::CountryName )
    : SSL_SUBJECT_INFO( qcert, QSslCertificate::CountryName ) );

  return dirname.join( QLatin1Char( ',' ) );
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
  return sl.join( QLatin1Char( ':' ) );
}

QString QgsAuthCertUtils::shaHexForCert( const QSslCertificate &cert, bool formatted )
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
  if ( QgsApplication::authManager()->isDisabled() )
    return QCA::Certificate();

  QCA::ConvertResult res;
  QCA::Certificate qcacert( QCA::Certificate::fromPEM( cert.toPem(), &res, QStringLiteral( "qca-ossl" ) ) );
  if ( res != QCA::ConvertGood || qcacert.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "Certificate could not be converted to QCA cert" ) );
    qcacert = QCA::Certificate();
  }
  return qcacert;
}

QCA::CertificateCollection QgsAuthCertUtils::qtCertsToQcaCollection( const QList<QSslCertificate> &certs )
{
  QCA::CertificateCollection qcacoll;
  if ( QgsApplication::authManager()->isDisabled() )
    return qcacoll;

  for ( const auto &cert : certs )
  {
    const QCA::Certificate qcacert( qtCertToQcaCert( cert ) );
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
  const QCA::KeyBundle bundle( QCA::KeyBundle::fromFile( path, passarray, &res, QStringLiteral( "qca-ossl" ) ) );

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

  if ( QgsApplication::authManager()->isDisabled() )
    return usages;

  QCA::ConvertResult res;
  const QCA::Certificate qcacert( QCA::Certificate::fromPEM( cert.toPem(), &res, QStringLiteral( "qca-ossl" ) ) );
  if ( res != QCA::ConvertGood || qcacert.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "Certificate could not be converted to QCA cert" ) );
    return usages;
  }

  if ( qcacert.isCA() )
  {
    QgsDebugMsg( QStringLiteral( "Certificate has 'CA:TRUE' basic constraint" ) );
    usages << QgsAuthCertUtils::CertAuthorityUsage;
  }

  const QList<QCA::ConstraintType> certconsts = qcacert.constraints();
  for ( const auto &certconst : certconsts )
  {
    if ( certconst.known() == QCA::KeyCertificateSign )
    {
      QgsDebugMsg( QStringLiteral( "Certificate has 'Certificate Sign' key usage" ) );
      usages << QgsAuthCertUtils::CertIssuerUsage;
    }
    else if ( certconst.known() == QCA::ServerAuth )
    {
      QgsDebugMsg( QStringLiteral( "Certificate has 'server authentication' extended key usage" ) );
      usages << QgsAuthCertUtils::TlsServerUsage;
    }
  }

  // ask QCA what it thinks about potential usages
  const QCA::CertificateCollection trustedCAs(
    qtCertsToQcaCollection( QgsApplication::authManager()->trustedCaCertsCache() ) );
  const QCA::CertificateCollection untrustedCAs(
    qtCertsToQcaCollection( QgsApplication::authManager()->untrustedCaCerts() ) );

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
  //QgsDebugMsg( QStringLiteral( "QCA::UsageTLSClient validity: %1" ).arg( static_cast<int>(v_tlsclient) ) );
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

  if ( QgsApplication::authManager()->isDisabled() )
    return false;

  QCA::ConvertResult res;
  QCA::Certificate qcacert( QCA::Certificate::fromPEM( cert.toPem(), &res, QString( "qca-ossl" ) ) );
  if ( res != QCA::ConvertGood || qcacert.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "Certificate could not be converted to QCA cert" ) );
    return false;
  }

  if ( qcacert.isCA() )
  {
    QgsDebugMsg( QStringLiteral( "SSL server certificate has 'CA:TRUE' basic constraint (and should not)" ) );
    return false;
  }

  const QList<QCA::ConstraintType> certconsts = qcacert.constraints();
  for ( const auto & certconst, certconsts )
  {
    if ( certconst.known() == QCA::KeyCertificateSign )
    {
      QgsDebugMsg( QStringLiteral( "SSL server certificate has 'Certificate Sign' key usage (and should not)" ) );
      return false;
    }
  }

  // check for common key usage and extended key usage constraints
  // see: https://www.ietf.org/rfc/rfc3280.txt  4.2.1.3(Key Usage) and  4.2.1.13(Extended Key Usage)
  bool serverauth = false;
  bool dsignature = false;
  bool keyencrypt = false;
  for ( const auto &certconst : certconsts )
  {
    if ( certconst.known() == QCA::DigitalSignature )
    {
      QgsDebugMsg( QStringLiteral( "SSL server certificate has 'digital signature' key usage" ) );
      dsignature = true;
    }
    else if ( certconst.known() == QCA::KeyEncipherment )
    {
      QgsDebugMsg( QStringLiteral( "SSL server certificate has 'key encipherment' key usage" ) );
      keyencrypt = true;
    }
    else if ( certconst.known() == QCA::KeyAgreement )
    {
      QgsDebugMsg( QStringLiteral( "SSL server certificate has 'key agreement' key usage" ) );
      keyencrypt = true;
    }
    else if ( certconst.known() == QCA::ServerAuth )
    {
      QgsDebugMsg( QStringLiteral( "SSL server certificate has 'server authentication' extended key usage" ) );
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
    for ( const auto &certconst : certconsts )
    {
      if ( certconst.known() == QCA::EncipherOnly )
      {
        QgsDebugMsg( QStringLiteral( "SSL server public key has 'encipher only' key usage" ) );
        encipheronly = true;
      }
      else if ( certconst.known() == QCA::DecipherOnly )
      {
        QgsDebugMsg( QStringLiteral( "SSL server public key has 'decipher only' key usage" ) );
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
      return QObject::tr( "Unable to Get Issuer Certificate" );
    case QSslError::UnableToDecryptCertificateSignature:
      return QObject::tr( "Unable to Decrypt Certificate Signature" );
    case QSslError::UnableToDecodeIssuerPublicKey:
      return QObject::tr( "Unable to Decode Issuer Public Key" );
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
      return QObject::tr( "Unable to Get Local Issuer Certificate" );
    case QSslError::UnableToVerifyFirstCertificate:
      return QObject::tr( "Unable to Verify First Certificate" );
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

bool QgsAuthCertUtils::certIsCurrent( const QSslCertificate &cert )
{
  if ( cert.isNull() )
    return false;
  const QDateTime currentTime = QDateTime::currentDateTime();
  return cert.effectiveDate() <= currentTime && cert.expiryDate() >= currentTime;
}

QList<QSslError> QgsAuthCertUtils::certViabilityErrors( const QSslCertificate &cert )
{
  QList<QSslError> sslErrors;

  if ( cert.isNull() )
    return sslErrors;

  const QDateTime currentTime = QDateTime::currentDateTime();
  if ( cert.expiryDate() <= currentTime )
  {
    sslErrors << QSslError( QSslError::SslError::CertificateExpired, cert );
  }
  if ( cert.effectiveDate() >= QDateTime::currentDateTime() )
  {
    sslErrors << QSslError( QSslError::SslError::CertificateNotYetValid, cert );
  }
  if ( cert.isBlacklisted() )
  {
    sslErrors << QSslError( QSslError::SslError::CertificateBlacklisted, cert );
  }

  return sslErrors;
}

bool QgsAuthCertUtils::certIsViable( const QSslCertificate &cert )
{
  return !cert.isNull() && QgsAuthCertUtils::certViabilityErrors( cert ).isEmpty();
}

QList<QSslError> QgsAuthCertUtils::validateCertChain( const QList<QSslCertificate> &certificateChain,
    const QString &hostName,
    bool trustRootCa )
{
  QList<QSslError> sslErrors;
  QList<QSslCertificate> trustedChain;
  // Filter out all CAs that are not trusted from QgsAuthManager
  for ( const auto &cert : certificateChain )
  {
    bool untrusted = false;
    for ( const auto &untrustedCert : QgsApplication::authManager()->untrustedCaCerts() )
    {
      if ( cert.digest( ) == untrustedCert.digest( ) )
      {
        untrusted = true;
        break;
      }
    }
    if ( ! untrusted )
    {
      trustedChain << cert;
    }
  }

  // Check that no certs in the chain are expired or not yet valid or blocklisted
  const QList<QSslCertificate> constTrustedChain( trustedChain );
  for ( const auto &cert : constTrustedChain )
  {
    sslErrors << QgsAuthCertUtils::certViabilityErrors( cert );
  }

  // Merge in the root CA if present and asked for
  if ( trustRootCa && trustedChain.count() > 1 && trustedChain.last().isSelfSigned() )
  {
    static QMutex sMutex;
    const QMutexLocker lock( &sMutex );
    const QSslConfiguration oldSslConfig( QSslConfiguration::defaultConfiguration() );
    QSslConfiguration sslConfig( oldSslConfig );
    sslConfig.setCaCertificates( casMerge( sslConfig.caCertificates(), QList<QSslCertificate>() << trustedChain.last() ) );
    QSslConfiguration::setDefaultConfiguration( sslConfig );
    sslErrors = QSslCertificate::verify( trustedChain, hostName );
    QSslConfiguration::setDefaultConfiguration( oldSslConfig );
  }
  else
  {
    sslErrors = QSslCertificate::verify( trustedChain, hostName );
  }
  return sslErrors;
}

QStringList QgsAuthCertUtils::validatePKIBundle( QgsPkiBundle &bundle, bool useIntermediates, bool trustRootCa )
{
  QStringList errors;
  if ( bundle.clientCert().isNull() )
    errors << QObject::tr( "Client certificate is NULL." );

  if ( bundle.clientKey().isNull() )
    errors << QObject::tr( "Client certificate key is NULL." );

  // immediately bail out if cert or key is NULL
  if ( !errors.isEmpty() )
    return errors;

  QList<QSslError> sslErrors;
  if ( useIntermediates )
  {
    QList<QSslCertificate> certsList( bundle.caChain() );
    certsList.insert( 0, bundle.clientCert( ) );
    sslErrors = QgsAuthCertUtils::validateCertChain( certsList, QString(), trustRootCa );
  }
  else
  {
    sslErrors = QSslCertificate::verify( QList<QSslCertificate>() << bundle.clientCert() );
  }
  const QList<QSslError> constSslErrors( sslErrors );
  for ( const auto &sslError : constSslErrors )
  {
    if ( sslError.error() != QSslError::NoError )
    {
      errors << sslError.errorString();
    }
  }
  // Now check the key with QCA!
  const QCA::PrivateKey pvtKey( QCA::PrivateKey::fromPEM( bundle.clientKey().toPem() ) );
  const QCA::PublicKey pubKey( QCA::PublicKey::fromPEM( bundle.clientCert().publicKey().toPem( ) ) );
  bool keyValid( ! pvtKey.isNull() );
  if ( keyValid && !( pubKey.toRSA().isNull( ) || pvtKey.toRSA().isNull( ) ) )
  {
    keyValid = pubKey.toRSA().n() == pvtKey.toRSA().n();
  }
  else if ( keyValid && !( pubKey.toDSA().isNull( ) || pvtKey.toDSA().isNull( ) ) )
  {
    keyValid = pubKey == QCA::DSAPublicKey( pvtKey.toDSA() );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Key is not DSA, RSA: validation is not supported by QCA" ) );
  }
  if ( ! keyValid )
  {
    errors << QObject::tr( "Private key does not match client certificate public key." );
  }
  return errors;
}
