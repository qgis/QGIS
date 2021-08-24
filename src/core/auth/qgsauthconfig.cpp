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
#include "qgsauthcertutils.h"
#include "qgsxmlutils.h"

#include <QtCrypto>

#include <QFile>
#include <QObject>
#include <QCryptographicHash>
#include <QUrl>


//////////////////////////////////////////////
// QgsAuthMethodConfig
//////////////////////////////////////////////

const QString QgsAuthMethodConfig::CONFIG_SEP = QStringLiteral( "|||" );
const QString QgsAuthMethodConfig::CONFIG_KEY_SEP = QStringLiteral( ":::" );
const QString QgsAuthMethodConfig::CONFIG_LIST_SEP = QStringLiteral( "```" );

const int QgsAuthMethodConfig::CONFIG_VERSION = 1;

// get uniqueConfigId only on save
QgsAuthMethodConfig::QgsAuthMethodConfig( const QString &method, int version )
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
  const bool idvalid = validateid ? !mId.isEmpty() : true;

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
    confstrs << i.key() + CONFIG_KEY_SEP + i.value();
    ++i;
  }
  return confstrs.join( CONFIG_SEP );
}

void QgsAuthMethodConfig::loadConfigString( const QString &configstr )
{
  clearConfigMap();
  if ( configstr.isEmpty() )
  {
    return;
  }

  const QStringList confs( configstr.split( CONFIG_SEP ) );

  for ( const auto &conf : confs )
  {
    if ( conf.contains( CONFIG_KEY_SEP ) )
    {
      const QStringList keyval( conf.split( CONFIG_KEY_SEP ) );
      setConfig( keyval.at( 0 ), keyval.at( 1 ) );
    }
  }

  if ( configMap().empty() )
  {
    setConfig( QStringLiteral( "oldconfigstyle" ), configstr );
  }
}

void QgsAuthMethodConfig::setConfig( const QString &key, const QString &value )
{
  mConfigMap.insert( key, value );
}

void QgsAuthMethodConfig::setConfigList( const QString &key, const QStringList &value )
{
  setConfig( key, value.join( CONFIG_LIST_SEP ) );
}

int QgsAuthMethodConfig::removeConfig( const QString &key )
{
  return mConfigMap.remove( key );
}

QString QgsAuthMethodConfig::config( const QString &key, const QString &defaultvalue ) const
{
  return mConfigMap.value( key, defaultvalue );
}

QStringList QgsAuthMethodConfig::configList( const QString &key ) const
{
  return config( key ).split( CONFIG_LIST_SEP );
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
    const QUrl url( accessurl );
    if ( url.isValid() )
    {
      res = QStringLiteral( "%1://%2:%3%4" ).arg( url.scheme(), url.host() )
            .arg( url.port() ).arg( withpath ? url.path() : QString() );
    }
  }
  *resource = res;
  return ( !res.isEmpty() );
}


bool QgsAuthMethodConfig::writeXml( QDomElement &parentElement, QDomDocument &document )
{
  QDomElement element = document.createElement( QStringLiteral( "AuthMethodConfig" ) );
  element.setAttribute( QStringLiteral( "method" ), mMethod );
  element.setAttribute( QStringLiteral( "id" ), mId );
  element.setAttribute( QStringLiteral( "name" ), mName );
  element.setAttribute( QStringLiteral( "version" ), QString::number( mVersion ) );
  element.setAttribute( QStringLiteral( "uri" ), mUri );

  QDomElement configElements = document.createElement( QStringLiteral( "Config" ) );
  QgsStringMap::const_iterator i = mConfigMap.constBegin();
  while ( i != mConfigMap.constEnd() )
  {
    configElements.setAttribute( i.key(), i.value() );
    ++i;
  }
  element.appendChild( configElements );

  parentElement.appendChild( element );
  return true;
}

bool QgsAuthMethodConfig::readXml( const QDomElement &element )
{
  if ( element.nodeName() != QLatin1String( "AuthMethodConfig" ) )
    return false;

  mMethod = element.attribute( QStringLiteral( "method" ) );
  mId = element.attribute( QStringLiteral( "id" ) );
  mName = element.attribute( QStringLiteral( "name" ) );
  mVersion = element.attribute( QStringLiteral( "version" ) ).toInt();
  mUri = element.attribute( QStringLiteral( "uri" ) );

  clearConfigMap();
  const QDomNamedNodeMap configAttributes = element.firstChildElement().attributes();
  for ( int i = 0; i < configAttributes.length(); i++ )
  {
    const QDomAttr configAttribute = configAttributes.item( i ).toAttr();
    setConfig( configAttribute.name(), configAttribute.value() );
  }

  return true;
}

#ifndef QT_NO_SSL

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

const QgsPkiBundle QgsPkiBundle::fromPemPaths( const QString &certPath,
    const QString &keyPath,
    const QString &keyPass,
    const QList<QSslCertificate> &caChain )
{
  QgsPkiBundle pkibundle;
  if ( !certPath.isEmpty() && !keyPath.isEmpty()
       && ( certPath.endsWith( QLatin1String( ".pem" ), Qt::CaseInsensitive )
            || certPath.endsWith( QLatin1String( ".der" ), Qt::CaseInsensitive ) )
       && QFile::exists( certPath ) && QFile::exists( keyPath )
     )
  {
    // client cert
    const bool pem = certPath.endsWith( QLatin1String( ".pem" ), Qt::CaseInsensitive );
    const QSslCertificate clientcert( QgsAuthCertUtils::fileData( certPath ), pem ? QSsl::Pem : QSsl::Der );
    pkibundle.setClientCert( clientcert );

    QSslKey clientkey;
    clientkey = QgsAuthCertUtils::keyFromFile( keyPath, keyPass );
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
       && ( bundlepath.endsWith( QLatin1String( ".p12" ), Qt::CaseInsensitive )
            || bundlepath.endsWith( QLatin1String( ".pfx" ), Qt::CaseInsensitive ) )
       && QFile::exists( bundlepath ) )
  {
    QCA::SecureArray passarray;
    if ( !bundlepass.isNull() )
      passarray = QCA::SecureArray( bundlepass.toUtf8() );
    QCA::ConvertResult res;
    const QCA::KeyBundle bundle( QCA::KeyBundle::fromFile( bundlepath, passarray, &res, QStringLiteral( "qca-ossl" ) ) );
    if ( res == QCA::ConvertGood && !bundle.isNull() )
    {
      const QCA::CertificateChain cert_chain( bundle.certificateChain() );
      const QSslCertificate cert( cert_chain.primary().toPEM().toLatin1() );
      if ( !cert.isNull() )
      {
        pkibundle.setClientCert( cert );
      }
      const QSslKey cert_key( bundle.privateKey().toPEM().toLatin1(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, QByteArray() );
      if ( !cert_key.isNull() )
      {
        pkibundle.setClientKey( cert_key );
      }

      if ( cert_chain.size() > 1 )
      {
        QList<QSslCertificate> ca_chain;
        for ( const auto &ca_cert : cert_chain )
        {
          if ( ca_cert != cert_chain.primary() )
          {
            ca_chain << QSslCertificate( ca_cert.toPEM().toLatin1() );
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
  return ( !isNull() && QgsAuthCertUtils::certIsViable( mCert ) );
}

const QString QgsPkiBundle::certId() const
{
  if ( mCert.isNull() )
  {
    return QString();
  }
  return QString( mCert.digest( QCryptographicHash::Sha1 ).toHex() );
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

QgsPkiConfigBundle::QgsPkiConfigBundle( const QgsAuthMethodConfig &config,
                                        const QSslCertificate &cert,
                                        const QSslKey &certkey,
                                        const QList<QSslCertificate> &cachain )
  : mConfig( config )
  , mCert( cert )
  , mCertKey( certkey )
  , mCaChain( cachain )
{
}

bool QgsPkiConfigBundle::isValid()
{
  return ( !mCert.isNull() && !mCertKey.isNull() );
}


//////////////////////////////////////////////
// QgsAuthConfigSslServer
//////////////////////////////////////////////

const QString QgsAuthConfigSslServer::CONF_SEP = QStringLiteral( "|||" );

QgsAuthConfigSslServer::QgsAuthConfigSslServer()
  : mSslHostPort( QString() )
  , mSslCert( QSslCertificate() )
  , mSslIgnoredErrors( QList<QSslError::SslError>() )
{
  // TODO: figure out if Qt 5 has changed yet again, e.g. TLS-only
  mQtVersion = 480;
  // Qt 4.8 defaults to SecureProtocols, i.e. TlsV1SslV3
  // http://qt-project.org/doc/qt-4.8/qssl.html#SslProtocol-enum
  mSslProtocol = QSsl::SecureProtocols;
}

const QList<QSslError> QgsAuthConfigSslServer::sslIgnoredErrors() const
{
  QList<QSslError> errors;
  const QList<QSslError::SslError> ignoredErrors = sslIgnoredErrorEnums();
  for ( const QSslError::SslError errenum : ignoredErrors )
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
  for ( const auto err : mSslIgnoredErrors )
  {
    errs << QString::number( static_cast< int >( err ) );
  }
  configlist << errs.join( QLatin1String( "~~" ) );

  configlist << QStringLiteral( "%1~~%2" ).arg( static_cast< int >( mSslPeerVerifyMode ) ).arg( mSslPeerVerifyDepth );

  return configlist.join( CONF_SEP );
}

void QgsAuthConfigSslServer::loadConfigString( const QString &config )
{
  if ( config.isEmpty() )
  {
    return;
  }
  const QStringList configlist( config.split( CONF_SEP ) );

  mVersion = configlist.at( 0 ).toInt();
  mQtVersion = configlist.at( 1 ).toInt();

  // TODO: Conversion between 4.7 -> 4.8 protocol enum differences (and reverse?).
  //       This is necessary for users upgrading from 4.7 to 4.8
  mSslProtocol = static_cast< QSsl::SslProtocol >( configlist.at( 2 ).toInt() );

  mSslIgnoredErrors.clear();
  const QStringList errs( configlist.at( 3 ).split( QStringLiteral( "~~" ) ) );
  for ( const auto &err : errs )
  {
    mSslIgnoredErrors.append( static_cast< QSslError::SslError >( err.toInt() ) );
  }

  const QStringList peerverify( configlist.at( 4 ).split( QStringLiteral( "~~" ) ) );
  mSslPeerVerifyMode = static_cast< QSslSocket::PeerVerifyMode >( peerverify.at( 0 ).toInt() );
  mSslPeerVerifyDepth = peerverify.at( 1 ).toInt();
}

bool QgsAuthConfigSslServer::isNull() const
{
  return mSslCert.isNull() && mSslHostPort.isEmpty();
}

#endif
