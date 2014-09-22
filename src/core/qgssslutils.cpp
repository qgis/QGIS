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


QgsSslCertSettings::QgsSslCertSettings()
    : mCertReady( false )
    , mStoreType( QgsSslCertSettings::QGISStore )
    , mCertId( "" )
    , mKeyId( "" )
    , mHasKeyPass( false )
    , mKeyPass( "" )
    , mIssuerCertId( "" )
    , mIssuerSelf( false )
    , mAccessUrl( "" )
{
}

bool QgsSslCertSettings::certIsReady() const
{
  bool ready = true;

  if ( mCertId.isEmpty() )
  {
    ready = ready && false;
    QgsDebugMsg( "SSL cert not ready: client cert is empty" );
  }

  if ( mKeyId.isEmpty() )
  {
    ready = ready && false;
    QgsDebugMsg( "SSL cert not ready: client key is empty" );
  }

  if ( !clientCert().isValid() )
  {
    ready = ready && false;
    QgsDebugMsg( "SSL cert not ready: client cert is invalid" );
  }

  return ready;
}

QSslCertificate QgsSslCertSettings::clientCert() const
{
  if ( mCertId.isEmpty() )
  {
    return QSslCertificate();
  }
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    QString path = QgsSslUtils::qgisCertPath( mCertId );
    if ( !path.isEmpty() )
    {
      return QgsSslUtils::certFromPath( path );
    }
  }
  return QSslCertificate();
}

QSslKey QgsSslCertSettings::clientCertKey() const
{
  if ( mKeyId.isEmpty() )
  {
    return QSslKey();
  }
  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    QString path = QgsSslUtils::qgisKeyPath( mKeyId );
    if ( path.isEmpty() )
    {
      return QSslKey();
    }

    if ( !mKeyPass.isEmpty() || mHasKeyPass )
    {
      QString passphrase = ( !mKeyPass.isEmpty() ? mKeyPass : "" );
      return QgsSslUtils::keyFromPath( path, QSsl::Pem, QSsl::PrivateKey, mHasKeyPass, passphrase, mAccessUrl );
    }
    else
    {
      return QgsSslUtils::keyFromPath( path );
    }

  }
  return QSslKey();
}

QSslCertificate QgsSslCertSettings::issuerCert() const
{
  if ( mIssuerCertId.isEmpty() )
  {
    return QSslCertificate();
  }

  if ( mStoreType == QgsSslCertSettings::QGISStore )
  {
    QString path = QgsSslUtils::qgisIssuerPath( mIssuerCertId );
    if ( !path.isEmpty() )
    {
      return QgsSslUtils::certFromPath( path );
    }
  }
  return QSslCertificate();
}

//////////////////////////////////////////////////////
// QgsSslUtils
//////////////////////////////////////////////////////

static QDir::Filters dirFilters = QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable;
static QFile::Permissions dirPerms = QFile::ReadUser | QFile::WriteUser | QFile::ExeUser;

const QString QgsSslUtils::qgisCertStoreDirPath()
{
  QDir certsDir( QgsApplication::qgisSettingsDirPath() + "cert_store" );
  return certsDir.absolutePath();
}

const QString QgsSslUtils::qgisCertsDirPath() { return QgsSslUtils::qgisCertStoreDirPath() + QDir::separator() + "certs"; }
const QString QgsSslUtils::qgisKeysDirPath() { return QgsSslUtils::qgisCertStoreDirPath() + QDir::separator() + "private"; }
const QString QgsSslUtils::qgisIssuersDirPath() { return QgsSslUtils::qgisCertStoreDirPath() + QDir::separator() + "issuers"; }

bool QgsSslUtils::createQgisCertStoreDir()
{
  QStringList paths;
  paths << QgsSslUtils::qgisCertStoreDirPath()
  << QgsSslUtils::qgisCertsDirPath()
  << QgsSslUtils::qgisKeysDirPath()
  << QgsSslUtils::qgisIssuersDirPath();

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

const QString QgsSslUtils::qgisCertPath( const QString& file )
{
  return existingFilePath_( QgsSslUtils::qgisCertsDirPath(), file );
}

const QString QgsSslUtils::qgisKeyPath( const QString& file )
{
  return existingFilePath_( QgsSslUtils::qgisKeysDirPath(), file );
}

const QString QgsSslUtils::qgisIssuerPath( const QString& file )
{
  return existingFilePath_( QgsSslUtils::qgisIssuersDirPath(), file );
}

static QStringList dirListing_( const QString& dirPath, const QStringList& nameFilter )
{
  QDir dir( dirPath );
  return dir.entryList( nameFilter, dirFilters );
}

const QStringList QgsSslUtils::storeCerts( QgsSslCertSettings::SslStoreType store )
{
  QStringList certs;
  if ( store == QgsSslCertSettings::QGISStore )
  {
    certs = dirListing_( QgsSslUtils::qgisCertsDirPath(), QStringList() << "*.pem" );
  }
  return certs;
}

const QStringList QgsSslUtils::storeKeys( QgsSslCertSettings::SslStoreType store )
{
  QStringList keys;
  if ( store == QgsSslCertSettings::QGISStore )
  {
    keys = dirListing_( QgsSslUtils::qgisKeysDirPath(), QStringList() << "*.pem" << "*.key" );
  }
  return keys;
}

const QStringList QgsSslUtils::storeIssuers( QgsSslCertSettings::SslStoreType store )
{
  QStringList iss;
  if ( store == QgsSslCertSettings::QGISStore )
  {
    iss = dirListing_( QgsSslUtils::qgisIssuersDirPath(), QStringList() << "*.pem" );
  }
  return iss;
}

QSslCertificate QgsSslUtils::certFromPath( const QString &path, QSsl::EncodingFormat format )
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

QSsl::KeyAlgorithm QgsSslUtils::keyAlgorithm( const QByteArray& keydata )
{
  QString keytxt( keydata );
  return (( keytxt.contains( "BEGIN DSA P" ) ) ? QSsl::Dsa : QSsl::Rsa );
}

QSslKey QgsSslUtils::keyFromData( const QByteArray& keydata,
                                  QSsl::EncodingFormat format,
                                  QSsl::KeyType type,
                                  bool hasKeyPhrase,
                                  const QString& passedinphrase,
                                  const QString& accessurl )
{
  QSsl::KeyAlgorithm algorithm = QgsSslUtils::keyAlgorithm( keydata );
  QString keyhash = QgsSslUtils::keyHashFromData( keydata );
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

QSslKey QgsSslUtils::keyFromPath( const QString& path,
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
      certkey = QgsSslUtils::keyFromData( file.readAll(), format, type, hasKeyPhrase, passphrase, accessurl );
    }
    file.close();
  }
  return certkey;
}

const QString QgsSslUtils::keyHashFromData( const QByteArray & data )
{
  if ( data.isEmpty() )
  {
    return QString();
  }
  // MD5 is fine (and faster), since it is not securing anything,
  // it's just used to represent the key data as a short string
  return QString( QCryptographicHash::hash( data, QCryptographicHash::Md5 ).toHex() );
}

const QString QgsSslUtils::keyHashFromPath( const QString& path )
{
  QString hash;
  QFile file( path );
  if ( file.exists() )
  {
    bool ret = file.open( QIODevice::ReadOnly | QIODevice::Text );
    if ( ret )
    {
      hash = QgsSslUtils::keyHashFromData( file.readAll() );
    }
    file.close();
  }
  return hash;
}

void QgsSslUtils::updateRequestSslConfiguration( QNetworkRequest &request, const QgsSslCertSettings& pki )
{
  if ( !pki.certIsReady() )
  {
    return;
  }

  QSslCertificate clientcert = pki.clientCert();
  QSslKey certkey = pki.clientCertKey();
  if ( clientcert.isNull() || certkey.isNull() )
  {
    return;
  }

  QSslConfiguration sslConfig = request.sslConfiguration();
  //QSslConfiguration sslConfig( QSslConfiguration::defaultConfiguration() );

  sslConfig.setProtocol( QSsl::TlsV1SslV3 );

  QSslCertificate issuercert = pki.issuerCert();
  if ( !issuercert.isNull() )
  {
    QList<QSslCertificate> sslCAs( sslConfig.caCertificates() );
    sslCAs << issuercert;
    sslConfig.setCaCertificates( sslCAs );
  }

  sslConfig.setLocalCertificate( clientcert );
  sslConfig.setPrivateKey( certkey );

  request.setSslConfiguration( sslConfig );
}

void QgsSslUtils::updateReplyExpectedSslErrors( QNetworkReply *reply, const QgsSslCertSettings& pki )
{
  if ( !pki.certIsReady() )
  {
    return;
  }

  if ( pki.issuerSelfSigned() )
  {
    QList<QSslError> expectedSslErrors;
    QSslError error = QSslError();
    QString issuer = "";
    QSslCertificate issuercert = pki.issuerCert();

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
}
