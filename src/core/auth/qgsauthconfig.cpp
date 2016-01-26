/***************************************************************************
    qgsauthconfig.cpp
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

#include "qgsauthconfig.h"

#include <QtCrypto>

#include <QFile>
#include <QObject>
#include <QUrl>

#include "qgsauthcertutils.h"


//////////////////////////////////////////////
// QgsAuthMethodConfig
//////////////////////////////////////////////

const QString QgsAuthMethodConfig::mConfigSep = "|||";
const QString QgsAuthMethodConfig::mConfigKeySep = ":::";
const QString QgsAuthMethodConfig::mConfigListSep = "```";

const int QgsAuthMethodConfig::mConfigVersion = 1;

// get uniqueConfigId only on save
QgsAuthMethodConfig::QgsAuthMethodConfig( const QString& method, int version )
    : mId( QString() )
    , mName( QString() )
    , mUri( QString() )
    , mMethod( method )
    , mVersion( version )
    , mConfigMap( QgsStringMap() )
{
}

bool QgsAuthMethodConfig::operator==( const QgsAuthMethodConfig &other ) const
{
  return ( other.id() == id()
           && other.name() == name()
           && other.uri() == uri()
           && other.method() == method()
           && other.version() == version()
           && other.configMap() == configMap() );
}

bool QgsAuthMethodConfig::operator!=( const QgsAuthMethodConfig &other ) const
{
  return  !( *this == other );
}

bool QgsAuthMethodConfig::isValid( bool validateid ) const
{
  bool idvalid = validateid ? !mId.isEmpty() : true;

  return (
           idvalid
           && !mName.isEmpty()
           && !mMethod.isEmpty()
         );
}

const QString QgsAuthMethodConfig::configString() const
{
  QStringList confstrs;
  QgsStringMap::const_iterator i = mConfigMap.constBegin();
  while ( i != mConfigMap.constEnd() )
  {
    confstrs << i.key() + mConfigKeySep + i.value();
    ++i;
  }
  return confstrs.join( mConfigSep );
}

void QgsAuthMethodConfig::loadConfigString( const QString &configstr )
{
  clearConfigMap();
  if ( configstr.isEmpty() )
  {
    return;
  }

  QStringList confs( configstr.split( mConfigSep ) );

  Q_FOREACH ( const QString& conf, confs )
  {
    if ( conf.contains( mConfigKeySep ) )
    {
      QStringList keyval( conf.split( mConfigKeySep ) );
      setConfig( keyval.at( 0 ), keyval.at( 1 ) );
    }
  }

  if ( configMap().empty() )
  {
    setConfig( "oldconfigstyle", configstr );
  }
}

void QgsAuthMethodConfig::setConfig( const QString &key, const QString &value )
{
  mConfigMap.insert( key, value );
}

void QgsAuthMethodConfig::setConfigList( const QString &key, const QStringList &value )
{
  setConfig( key, value.join( mConfigListSep ) );
}

int QgsAuthMethodConfig::removeConfig( const QString &key )
{
  return mConfigMap.remove( key );
}

QString QgsAuthMethodConfig::config( const QString &key, const QString& defaultvalue ) const
{
  return mConfigMap.value( key, defaultvalue );
}

QStringList QgsAuthMethodConfig::configList( const QString &key ) const
{
  return config( key ).split( mConfigListSep );
}

bool QgsAuthMethodConfig::hasConfig( const QString &key ) const
{
  return mConfigMap.contains( key );
}

bool QgsAuthMethodConfig::uriToResource( const QString &accessurl, QString *resource, bool withpath )
{
  QString res = QString();
  if ( !accessurl.isEmpty() )
  {
    QUrl url( accessurl );
    if ( url.isValid() )
    {
      res = QString( "%1://%2:%3%4" ).arg( url.scheme(), url.host() )
            .arg( url.port() ).arg( withpath ? url.path() : "" );
    }
  }
  *resource = res;
  return ( !res.isEmpty() );
}


#ifndef QT_NO_OPENSSL

//////////////////////////////////////////////////////
// QgsPkiBundle
//////////////////////////////////////////////////////

QgsPkiBundle::QgsPkiBundle( const QSslCertificate &clientCert,
                            const QSslKey &clientKey,
                            const QList<QSslCertificate> &caChain )
    : mCert( QSslCertificate() )
    , mCertKey( QSslKey() )
    , mCaChain( caChain )
{
  setClientCert( clientCert );
  setClientKey( clientKey );
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

const QgsPkiBundle QgsPkiBundle::fromPemPaths( const QString &certPath,
    const QString &keyPath,
    const QString &keyPass,
    const QList<QSslCertificate> &caChain )
{
  QgsPkiBundle pkibundle;
  if ( !certPath.isEmpty() && !keyPath.isEmpty()
       && ( certPath.endsWith( ".pem", Qt::CaseInsensitive )
            || certPath.endsWith( ".der", Qt::CaseInsensitive ) )
       && ( keyPath.endsWith( ".pem", Qt::CaseInsensitive )
            || keyPath.endsWith( ".der", Qt::CaseInsensitive ) )
       && QFile::exists( certPath ) && QFile::exists( keyPath )
     )
  {
    // client cert
    bool pem = certPath.endsWith( ".pem", Qt::CaseInsensitive );
    QSslCertificate clientcert( fileData_( certPath, pem ), pem ? QSsl::Pem : QSsl::Der );
    pkibundle.setClientCert( clientcert );

    // client key
    bool pem_key = keyPath.endsWith( ".pem", Qt::CaseInsensitive );
    QByteArray keydata( fileData_( keyPath, pem_key ) );

    QSslKey clientkey;
    clientkey = QSslKey( keydata,
                         QSsl::Rsa,
                         pem_key ? QSsl::Pem : QSsl::Der,
                         QSsl::PrivateKey,
                         !keyPass.isNull() ? keyPass.toUtf8() : QByteArray() );
    if ( clientkey.isNull() )
    {
      // try DSA algorithm, since Qt can't seem to determine it otherwise
      clientkey = QSslKey( keydata,
                           QSsl::Dsa,
                           pem_key ? QSsl::Pem : QSsl::Der,
                           QSsl::PrivateKey,
                           !keyPass.isNull() ? keyPass.toUtf8() : QByteArray() );
    }
    pkibundle.setClientKey( clientkey );
    if ( !caChain.isEmpty() )
    {
      pkibundle.setCaChain( caChain );
    }
  }
  return pkibundle;
}

const QgsPkiBundle QgsPkiBundle::fromPkcs12Paths( const QString &bundlepath,
    const QString &bundlepass )
{
  QgsPkiBundle pkibundle;
  if ( QCA::isSupported( "pkcs12" )
       && !bundlepath.isEmpty()
       && ( bundlepath.endsWith( ".p12", Qt::CaseInsensitive )
            || bundlepath.endsWith( ".pfx", Qt::CaseInsensitive ) )
       && QFile::exists( bundlepath ) )
  {
    QCA::SecureArray passarray;
    if ( !bundlepass.isNull() )
      passarray = QCA::SecureArray( bundlepass.toUtf8() );
    QCA::ConvertResult res;
    QCA::KeyBundle bundle( QCA::KeyBundle::fromFile( bundlepath, passarray, &res, QString( "qca-ossl" ) ) );
    if ( res == QCA::ConvertGood && !bundle.isNull() )
    {
      QCA::CertificateChain cert_chain( bundle.certificateChain() );
      QSslCertificate cert( cert_chain.primary().toPEM().toAscii() );
      if ( !cert.isNull() )
      {
        pkibundle.setClientCert( cert );
      }
      QSslKey cert_key( bundle.privateKey().toPEM().toAscii(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, QByteArray() );
      if ( !cert_key.isNull() )
      {
        pkibundle.setClientKey( cert_key );
      }

      if ( cert_chain.size() > 1 )
      {
        QList<QSslCertificate> ca_chain;
        Q_FOREACH ( const QCA::Certificate& ca_cert, cert_chain )
        {
          if ( ca_cert != cert_chain.primary() )
          {
            ca_chain << QSslCertificate( ca_cert.toPEM().toAscii() );
          }
        }
        pkibundle.setCaChain( ca_chain );
      }

    }
  }
  return pkibundle;
}

bool QgsPkiBundle::isNull() const
{
  return ( mCert.isNull() || mCertKey.isNull() );
}

bool QgsPkiBundle::isValid() const
{
  return ( !isNull() && mCert.isValid() );
}

const QString QgsPkiBundle::certId() const
{
  if ( mCert.isNull() )
  {
    return QString::null;
  }
  return QgsAuthCertUtils::shaHexForCert( mCert );
}

void QgsPkiBundle::setClientCert( const QSslCertificate &cert )
{
  mCert.clear();
  if ( !cert.isNull() )
  {
    mCert = cert;
  }
}

void QgsPkiBundle::setClientKey( const QSslKey &certkey )
{
  mCertKey.clear();
  if ( !certkey.isNull() && certkey.type() == QSsl::PrivateKey )
  {
    mCertKey = certkey;
  }
}


//////////////////////////////////////////////////////
// QgsPkiConfigBundle
//////////////////////////////////////////////////////

QgsPkiConfigBundle::QgsPkiConfigBundle( const QgsAuthMethodConfig& config,
                                        const QSslCertificate& cert,
                                        const QSslKey& certkey )
    : mConfig( config )
    , mCert( cert )
    , mCertKey( certkey )
{
}

bool QgsPkiConfigBundle::isValid()
{
  return ( !mCert.isNull() && !mCertKey.isNull() );
}


//////////////////////////////////////////////
// QgsAuthConfigSslServer
//////////////////////////////////////////////

const QString QgsAuthConfigSslServer::mConfSep = "|||";

QgsAuthConfigSslServer::QgsAuthConfigSslServer()
    : mSslHostPort( QString() )
    , mSslCert( QSslCertificate() )
    , mSslIgnoredErrors( QList<QSslError::SslError>() )
    , mSslPeerVerifyMode( QSslSocket::VerifyPeer )
    , mSslPeerVerifyDepth( 0 )
    , mVersion( 1 )
{
  // TODO: figure out if Qt 5 has changed yet again, e.g. TLS-only
#if QT_VERSION >= 0x040800
  mQtVersion = 480;
  // Qt 4.8 defaults to SecureProtocols, i.e. TlsV1SslV3
  // http://qt-project.org/doc/qt-4.8/qssl.html#SslProtocol-enum
  mSslProtocol = QSsl::SecureProtocols;
#else
  mQtVersion = 470;
  // older Qt 4.7 defaults to now-vulnerable SSLv3
  // http://qt-project.org/doc/qt-4.7/qssl.html
  // Default this to TlsV1 instead
  mSslProtocol = QSsl::TlsV1;
#endif
}

const QList<QSslError> QgsAuthConfigSslServer::sslIgnoredErrors() const
{
  QList<QSslError> errors;
  Q_FOREACH ( QSslError::SslError errenum, sslIgnoredErrorEnums() )
  {
    errors << QSslError( errenum );
  }
  return errors;
}

const QString QgsAuthConfigSslServer::configString() const
{
  QStringList configlist;
  configlist << QString::number( mVersion ) << QString::number( mQtVersion );

  configlist << QString::number( static_cast< int >( mSslProtocol ) );

  QStringList errs;
  Q_FOREACH ( const QSslError::SslError& err, mSslIgnoredErrors )
  {
    errs << QString::number( static_cast< int >( err ) );
  }
  configlist << errs.join( "~~" );

  configlist << QString( "%1~~%2" ).arg( static_cast< int >( mSslPeerVerifyMode ) ).arg( mSslPeerVerifyDepth );

  return configlist.join( mConfSep );
}

void QgsAuthConfigSslServer::loadConfigString( const QString &config )
{
  if ( config.isEmpty() )
  {
    return;
  }
  QStringList configlist( config.split( mConfSep ) );

  mVersion = configlist.at( 0 ).toInt();
  mQtVersion = configlist.at( 1 ).toInt();

  // TODO: Conversion between 4.7 -> 4.8 protocol enum differences (and reverse?).
  //       This is necessary for users upgrading from 4.7 to 4.8
  mSslProtocol = static_cast< QSsl::SslProtocol >( configlist.at( 2 ).toInt() );

  mSslIgnoredErrors.clear();
  QStringList errs( configlist.at( 3 ).split( "~~" ) );
  Q_FOREACH ( const QString& err, errs )
  {
    mSslIgnoredErrors.append( static_cast< QSslError::SslError >( err.toInt() ) );
  }

  QStringList peerverify( configlist.at( 4 ).split( "~~" ) );
  mSslPeerVerifyMode = static_cast< QSslSocket::PeerVerifyMode >( peerverify.at( 0 ).toInt() );
  mSslPeerVerifyDepth = peerverify.at( 1 ).toInt();
}

bool QgsAuthConfigSslServer::isNull() const
{
  return mSslCert.isNull() && mSslHostPort.isEmpty();
}

#endif
