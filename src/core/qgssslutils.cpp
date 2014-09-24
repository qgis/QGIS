/***************************************************************************
    qgssslutils.cpp
    ---------------------
    begin                : 2014/09/12
    copyright            : (C) 2014 by Boundless Spatial, Inc.
    web                  : http://boundlessgeo.com
    author               : Larry Shaffer
    email                : larrys at dakotacarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssslutils.h"

#include <QDir>
#include <QFile>
#include <QInputDialog>
#include <QSslConfiguration>
#include <QSslError>

#include "qgsapplication.h"
#include "qgscredentials.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"


QgsSslPkiSettings::QgsSslPkiSettings()
    : mCertReady( false )
    , mStoreType( QgsSslPkiSettings::QGISStore )
    , mCertId( "" )
    , mKeyId( "" )
    , mNeedsKeyId( false )
    , mHasKeyPass( false )
    , mKeyPass( "" )
    , mIssuerCertId( "" )
    , mIssuerSelf( false )
    , mAccessUrl( "" )
{
}

bool QgsSslPkiSettings::certIsReady() const
{
  bool ready = true;

  if ( mCertId.isEmpty() )
  {
    ready = ready && false;
    QgsDebugMsg( "SSL cert not ready: client cert is empty" );
  }

  if ( !mNeedsKeyId && mKeyId.isEmpty() )
  {
    ready = ready && false;
    QgsDebugMsg( "SSL cert not ready: client key is empty" );
  }

//  if ( !clientCert().isValid() )
//  {
//    ready = ready && false;
//    QgsDebugMsg( "SSL cert not ready: client cert is invalid" );
//  }

  return ready;
}

static QByteArray fileData_( const QString& path )
{
  QByteArray data;
  QFile file( path );
  if ( file.exists() )
  {
    bool ret = file.open( QIODevice::ReadOnly | QIODevice::Text );
    if ( ret )
    {
      data = file.readAll();
    }
    file.close();
  }
  return data;
}

QByteArray QgsSslPkiSettings::clientCertData() const
{
  QByteArray data;
  if ( mCertId.isEmpty() )
  {
    return data;
  }
  if ( mStoreType == QgsSslPkiSettings::QGISStore )
  {
    QString path = QgsSslPkiUtility::qgisCertPath( mCertId );
    if ( !path.isEmpty() )
    {
      return data = fileData_( path );
    }
  }
  return data;
}

QByteArray QgsSslPkiSettings::clientKeyData() const
{
  QByteArray data;
  if ( mNeedsKeyId || mKeyId.isEmpty() )
  {
    return data;
  }
  if ( mStoreType == QgsSslPkiSettings::QGISStore )
  {
    QString path = QgsSslPkiUtility::qgisKeyPath( mKeyId );
    if ( !path.isEmpty() )
    {
      data = fileData_( path );
    }
  }
  return data;
}

QByteArray QgsSslPkiSettings::issuerCertData() const
{
  QByteArray data;
  if ( mIssuerCertId.isEmpty() )
  {
    return data;
  }

  if ( mStoreType == QgsSslPkiSettings::QGISStore )
  {
    QString path = QgsSslPkiUtility::qgisIssuerPath( mIssuerCertId );
    if ( !path.isEmpty() )
    {
      data = fileData_( path );
    }
  }
  return data;
}

//////////////////////////////////////////////////////
// QgsSslPkiGroup
//////////////////////////////////////////////////////

QgsSslPkiGroup::QgsSslPkiGroup( const QSslCertificate& cert, const QSslKey& certkey,
                                const QSslCertificate& issuer, bool issuerselfsigned )
    : mCert( cert )
    , mCertKey( certkey )
    , mIssuer( issuer )
    , mIssuerSelfSigned( issuerselfsigned )
{
}

QgsSslPkiGroup::~QgsSslPkiGroup()
{
}

bool QgsSslPkiGroup::isNull()
{
  return ( mCert.isNull() && mCertKey.isNull() );
}

//////////////////////////////////////////////////////
// QgsSslPkiUtility
//////////////////////////////////////////////////////

QgsSslPkiUtility *QgsSslPkiUtility::smInstance = 0;
QMap<QString, QgsSslPkiGroup *> QgsSslPkiUtility::mSslPkiGroupCache = QMap<QString, QgsSslPkiGroup *>();

QgsSslPkiUtility *QgsSslPkiUtility::instance()
{
  if ( !smInstance )
  {
    smInstance = new QgsSslPkiUtility();
  }
  return smInstance;
}

// protected
QgsSslPkiUtility::QgsSslPkiUtility()
{
}

// protected
QgsSslPkiUtility::~QgsSslPkiUtility()
{
  // clear cache
  QMap<QString, QgsSslPkiGroup *>::iterator it = mSslPkiGroupCache.begin();
  for ( ; it != mSslPkiGroupCache.end(); ++it )
  {
    delete( it.value() );
    mSslPkiGroupCache.erase( it );
  }
}

bool QgsSslPkiUtility::urlToResource( const QString& accessurl, QString *resource )
{
  QString res = QString();
  if ( !accessurl.isEmpty() )
  {
    QUrl url( accessurl );
    if ( url.isValid() )
    {
      res = QString( "%1://%2:%3" ).arg( url.scheme() ).arg( url.host() ).arg( url.port() );
    }
  }
  *resource = res;
  return ( !res.isEmpty() );
}

QgsSslPkiGroup * QgsSslPkiUtility::getSslPkiGroup( const QgsSslPkiSettings& pki )
{
  QgsSslPkiGroup * pkigrp = 0;

  QString resource;
  if ( !QgsSslPkiUtility::urlToResource( pki.accessUrl(), &resource ) )
  {
    QgsDebugMsg( QString( "Insert SSL PKI group FAILED: could not convert to resource from url: %1" ).arg( pki.accessUrl() ) );
    return pkigrp;
  }

  // check if it is cached
  if ( mSslPkiGroupCache.contains( resource ) )
  {
    pkigrp = mSslPkiGroupCache.value( resource );
    if ( pkigrp )
    {
      QgsDebugMsg( QString( "Retrieved SSL PKI group for resource: %1" ).arg( resource ) );
      return pkigrp;
    }
  }

  // else build the SSL PKI group

  // init client cert
  // Note: if this is not valid, no sense continuing
  QSslCertificate clientCert = QSslCertificate( pki.clientCertData() );
  if ( !clientCert.isValid() )
  {
    QgsDebugMsg( "Insert SSL PKI group FAILED: client cert is not valid" );
    return pkigrp;
  }

  // init key
  QSslKey clientkey;
  QByteArray keydata = pki.clientKeyData();

  if ( keydata.isNull() && !pki.needsKeyPath() )
  {
    QgsDebugMsg( "Insert SSL PKI group FAILED: no key data read" );
    return pkigrp;
  }

  // add key that is already defined
  if ( !keydata.isNull() && !pki.needsKeyPath() && !pki.needsKeyPassphrase() )
  {
    if ( !pki.keyPassphrase().isEmpty() )
    {
      clientkey = QSslKey( keydata, QgsSslPkiUtility::keyAlgorithm( keydata ),
                           QSsl::Pem, QSsl::PrivateKey, pki.keyPassphrase().toLocal8Bit() );
    }
    else
    {
      clientkey = QSslKey( keydata, QgsSslPkiUtility::keyAlgorithm( keydata ) );
    }
  }

  // get needed key path or passphrase
  QString inputphrase, keypath;
  if ( clientkey.isNull() && ( pki.needsKeyPath() || pki.needsKeyPassphrase() ) )
  {
    QgsCredentials * creds = QgsCredentials::instance();
    creds->lock();
    bool ok = creds->getSslKeyInfo( inputphrase, keypath, pki.needsKeyPath(), resource );
    creds->unlock();
    if ( !ok )
    {
      QgsDebugMsg( "Insert SSL PKI group FAILED: user cancelled key credentials dialog" );
      return pkigrp;
    }

    if ( pki.needsKeyPath() && !keypath.isEmpty() )
    {
      QFile file( keypath );
      bool ret = file.open( QIODevice::ReadOnly | QIODevice::Text );
      if ( ret )
      {
        keydata = file.readAll();
      }
      else
      {
        file.close();
        QgsDebugMsg( "Insert SSL PKI group FAILED: chosen key filepath can not be read" );
        return pkigrp;
      }
      file.close();
    }

    clientkey = QSslKey( keydata, QgsSslPkiUtility::keyAlgorithm( keydata ),
                         QSsl::Pem, QSsl::PrivateKey, inputphrase.toLocal8Bit() );
  }
  // final client key check
  if ( clientkey.isNull() )
  {
    QgsDebugMsg( "Insert SSL PKI group FAILED: cert key could not be created" );
    return pkigrp;
  }

  // init issuer cert
  QSslCertificate issuercert;
  QByteArray issuerdata = pki.issuerCertData();
  if ( !issuerdata.isNull() )
  {
    issuercert = QSslCertificate( pki.issuerCertData() );
    if ( !issuercert.isValid() )
    {
      QgsDebugMsg( "Insert SSL PKI group FAILED: issuer cert is not valid" );
      return pkigrp;
    }
  }

  pkigrp = new QgsSslPkiGroup( clientCert, clientkey, issuercert, pki.issuerSelfSigned() );

  // cache group
  putSslPkiGroup( pki, pkigrp );

  return pkigrp;
}

void QgsSslPkiUtility::putSslPkiGroup( const QgsSslPkiSettings& pki, QgsSslPkiGroup * pkigroup )
{
  QString resource;
  if ( QgsSslPkiUtility::urlToResource( pki.accessUrl(), &resource ) )
  {
    QgsDebugMsg( QString( "Inserting SSL PKI group for resource: %1" ).arg( resource ) );
    mSslPkiGroupCache.insert( resource, pkigroup );
  }
  else
  {
    QgsDebugMsg( QString( "Inserting SSL PKI group FAILED: accessurl=%1" ).arg( pki.accessUrl() ) );
  }
}

void QgsSslPkiUtility::removeSslPkiGroup( const QgsSslPkiSettings &pki )
{
  QString resource;
  if ( !QgsSslPkiUtility::urlToResource( pki.accessUrl(), &resource ) )
  {
    QgsDebugMsg( QString( "Remove SSL PKI group FAILED: could not convert to resource from url: %1" ).arg( pki.accessUrl() ) );
  }
  if ( mSslPkiGroupCache.contains( resource ) )
  {
    QgsSslPkiGroup * pkigrp = mSslPkiGroupCache.take( resource );
    delete pkigrp;
    pkigrp = 0;
    QgsDebugMsg( QString( "Removed SSL PKI group associated with resource: %1" ).arg( resource ) );
  }
}

static QDir::Filters dirFilters = QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable;
static QFile::Permissions dirPerms = QFile::ReadUser | QFile::WriteUser | QFile::ExeUser;

const QString QgsSslPkiUtility::qgisCertStoreDirPath()
{
  QDir certsDir( QgsApplication::qgisSettingsDirPath() + "cert_store" );
  return certsDir.absolutePath();
}

const QString QgsSslPkiUtility::qgisCertsDirPath() { return QgsSslPkiUtility::qgisCertStoreDirPath() + QDir::separator() + "certs"; }
const QString QgsSslPkiUtility::qgisKeysDirPath() { return QgsSslPkiUtility::qgisCertStoreDirPath() + QDir::separator() + "private"; }
const QString QgsSslPkiUtility::qgisIssuersDirPath() { return QgsSslPkiUtility::qgisCertStoreDirPath() + QDir::separator() + "issuers"; }

bool QgsSslPkiUtility::createQgisCertStoreDir()
{
  QStringList paths;
  paths << QgsSslPkiUtility::qgisCertStoreDirPath()
  << QgsSslPkiUtility::qgisCertsDirPath()
  << QgsSslPkiUtility::qgisKeysDirPath()
  << QgsSslPkiUtility::qgisIssuersDirPath();

  QDir cwd;
  foreach ( const QString& path, paths )
  {
    if ( !cwd.mkpath( path ) )
      return false;
    if ( !QFile::setPermissions( path, dirPerms ) )
      return false;
  }
  return true;
}

static QString existingFilePath_( const QString& dir, const QString& file )
{
  QString path = dir + QDir::separator() + file;
  bool exists = false;
  if ( !file.isNull() && !file.isEmpty() )
  {
    exists = QFile::exists( path );
  }
  return ( exists ? path : QString() );
}

const QString QgsSslPkiUtility::qgisCertPath( const QString& file )
{
  if ( QFile::exists( file ) )
  {
    return file;
  }
  return existingFilePath_( QgsSslPkiUtility::qgisCertsDirPath(), file );
}

const QString QgsSslPkiUtility::qgisKeyPath( const QString& file )
{
  if ( QFile::exists( file ) )
  {
    return file;
  }
  return existingFilePath_( QgsSslPkiUtility::qgisKeysDirPath(), file );
}

const QString QgsSslPkiUtility::qgisIssuerPath( const QString& file )
{
  if ( QFile::exists( file ) )
  {
    return file;
  }
  return existingFilePath_( QgsSslPkiUtility::qgisIssuersDirPath(), file );
}

static QStringList dirListing_( const QString& dirPath, const QStringList& nameFilter )
{
  QDir dir( dirPath );
  return dir.entryList( nameFilter, dirFilters );
}

const QStringList QgsSslPkiUtility::storeCerts( QgsSslPkiSettings::SslStoreType store )
{
  QStringList certs;
  if ( store == QgsSslPkiSettings::QGISStore )
  {
    certs = dirListing_( QgsSslPkiUtility::qgisCertsDirPath(), QStringList() << "*.pem" );
  }
  return certs;
}

const QStringList QgsSslPkiUtility::storeKeys( QgsSslPkiSettings::SslStoreType store )
{
  QStringList keys;
  if ( store == QgsSslPkiSettings::QGISStore )
  {
    keys = dirListing_( QgsSslPkiUtility::qgisKeysDirPath(), QStringList() << "*.pem" << "*.key" );
  }
  return keys;
}

const QStringList QgsSslPkiUtility::storeIssuers( QgsSslPkiSettings::SslStoreType store )
{
  QStringList iss;
  if ( store == QgsSslPkiSettings::QGISStore )
  {
    iss = dirListing_( QgsSslPkiUtility::qgisIssuersDirPath(), QStringList() << "*.pem" );
  }
  return iss;
}

QSslCertificate QgsSslPkiUtility::certFromPath( const QString &path, QSsl::EncodingFormat format )
{
  QSslCertificate cert;
  QFile file( path );
  if ( file.exists() )
  {
    bool ret = file.open( QIODevice::ReadOnly | QIODevice::Text );
    if ( ret )
    {
      cert = QSslCertificate( file.readAll(), format );
    }
    file.close();
  }
  return cert;
}

QSsl::KeyAlgorithm QgsSslPkiUtility::keyAlgorithm( const QByteArray& keydata )
{
  QString keytxt( keydata );
  return (( keytxt.contains( "BEGIN DSA P" ) ) ? QSsl::Dsa : QSsl::Rsa );
}

#if 0
QSslKey QgsSslPkiUtility::keyFromData( const QByteArray& keydata,
                                       QSsl::EncodingFormat format,
                                       QSsl::KeyType type,
                                       bool hasKeyPhrase,
                                       const QString& passedinphrase,
                                       const QString& accessurl )
{
  QSsl::KeyAlgorithm algorithm = QgsSslPkiUtility::keyAlgorithm( keydata );
  QString keyhash = QgsSslPkiUtility::keyHashFromData( keydata );
  QgsNetworkAccessManager * naM = QgsNetworkAccessManager::instance();

  if ( !passedinphrase.isEmpty() )
  {
    // cache passed-in password
    if ( !naM->hasKeyPass( keyhash ) )
    {
      naM->setKeyPass( keyhash, passedinphrase );
    }
    QgsDebugMsg( QString( "Creating SSL key using passed-in passphrase" ) );
  }
  else if ( hasKeyPhrase && !naM->hasKeyPass( keyhash ) )
  {
    QString inputphrase;
    QgsCredentials * creds = QgsCredentials::instance();
    creds->lock();
    bool ok = creds->getSslNoCache( inputphrase, accessurl );
    creds->unlock();
    if ( !ok )
    {
      return QSslKey( keydata, algorithm, format, type );
    }

    if ( !inputphrase.isEmpty() )
    {
      if ( !naM->hasKeyPass( keyhash ) )
      {
        naM->setKeyPass( keyhash, inputphrase );
      }
      QgsDebugMsg( QString( "Creating SSL key using input passphrase" ) );
    }
  }

  QString passphrase;
  if ( naM->hasKeyPass( keyhash ) )
  {
    passphrase = naM->keyPass( keyhash );
  }

  // create key
  QSslKey clientKey = QSslKey( keydata, algorithm, format, type, passphrase.toLocal8Bit() );

  if ( hasKeyPhrase && !passphrase.isEmpty() && clientKey.isNull() )
  {
    // assume passphrase didn't work, so make sure it is removed from cache
    // NOTE: could be other reasons for key creation failure?
    if ( naM->hasKeyPass( keyhash ) )
    {
      naM->removeKeyPass( keyhash );
    }
    QgsDebugMsg( QString( "Creating SSL key FAILED using passphrase" ) );
  }

  return clientKey;
}
#endif

#if 0
QSslKey QgsSslPkiUtility::keyFromPath( const QString& path,
                                       QSsl::EncodingFormat format,
                                       QSsl::KeyType type,
                                       bool hasKeyPhrase,
                                       const QString& passphrase,
                                       const QString& accessurl )
{
  QSslKey certkey;
  QFile file( path );
  if ( file.exists() )
  {
    bool ret = file.open( QIODevice::ReadOnly | QIODevice::Text );
    if ( ret )
    {
      certkey = QgsSslPkiUtility::keyFromData( file.readAll(), format, type, hasKeyPhrase, passphrase, accessurl );
    }
    file.close();
  }
  return certkey;
}
#endif

#if 0
const QString QgsSslPkiUtility::keyHashFromData( const QByteArray & data )
{
  if ( data.isEmpty() )
  {
    return QString();
  }
  // MD5 is fine (and faster), since it is not securing anything,
  // it's just used to represent the key data as a short string
  return QString( QCryptographicHash::hash( data, QCryptographicHash::Md5 ).toHex() );
}
#endif

#if 0
const QString QgsSslPkiUtility::keyHashFromPath( const QString& path )
{
  QString hash;
  QFile file( path );
  if ( file.exists() )
  {
    bool ret = file.open( QIODevice::ReadOnly | QIODevice::Text );
    if ( ret )
    {
      hash = QgsSslPkiUtility::keyHashFromData( file.readAll() );
    }
    file.close();
  }
  return hash;
}
#endif

void QgsSslPkiUtility::updateRequestSslConfiguration( QNetworkRequest &request, const QgsSslPkiSettings& pki )
{
  if ( !pki.certIsReady() )
  {
    return;
  }

  QgsSslPkiGroup * pkigrp = getSslPkiGroup( pki );
  if ( !pkigrp || pkigrp->isNull() )
  {
    QgsDebugMsg( QString( "Update request SSL config FAILED: PKI group empty or null" ) );
    return;
  }

  QSslConfiguration sslConfig = request.sslConfiguration();
  //QSslConfiguration sslConfig( QSslConfiguration::defaultConfiguration() );

  sslConfig.setProtocol( QSsl::TlsV1SslV3 );

  QSslCertificate issuercert = pkigrp->issuerCert();
  if ( !issuercert.isNull() )
  {
    QList<QSslCertificate> sslCAs( sslConfig.caCertificates() );
    sslCAs << issuercert;
    sslConfig.setCaCertificates( sslCAs );
  }

  sslConfig.setLocalCertificate( pkigrp->clientCert() );
  sslConfig.setPrivateKey( pkigrp->clientCertKey() );

  request.setSslConfiguration( sslConfig );
}

void QgsSslPkiUtility::updateReplyExpectedSslErrors( QNetworkReply *reply, const QgsSslPkiSettings& pki )
{
  QgsSslPkiGroup * pkigrp = getSslPkiGroup( pki );
  if ( !pkigrp || pkigrp->isNull() )
  {
    QgsDebugMsg( QString( "Update reply SSL errors FAILED: PKI group empty or null" ) );
    return;
  }
  if ( !pkigrp->issuerSelfSigned() )
  {
    // TODO: maybe sniff if it is self-signed, regardless of what user defines
    QgsDebugMsg( QString( "Update reply SSL errors SKIPPED: PKI issuer not set as self-signed" ) );
    return;
  }

  QList<QSslError> expectedSslErrors;
  QSslError error = QSslError();
  QString issuer = "";
  QSslCertificate issuercert = pkigrp->issuerCert();

  if ( !issuercert.isNull() )
  {
    issuer = " for defined issuer";
    error = QSslError( QSslError::SelfSignedCertificate, issuercert );
  }
  else
  {
    // issuer not defined, but may already be in available CAs
    issuer = " for ALL in chain";
    error = QSslError( QSslError::SelfSignedCertificate );
  }
  if ( error.error() != QSslError::NoError )
  {
    QgsDebugMsg( QString( "Adding self-signed cert expected ssl error%1" ).arg( issuer ) );
    expectedSslErrors.append( error );
    reply->ignoreSslErrors( expectedSslErrors );
  }
}
