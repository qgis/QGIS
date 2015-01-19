/***************************************************************************
    qgsauthenticationconfig.cpp
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

#include "qgsauthenticationconfig.h"

#include "qgsauthenticationmanager.h"
#include "qgsauthenticationprovider.h"

#include <QFile>
#include <QObject>


const QHash<QgsAuthType::ProviderType, QString> QgsAuthType::typeNameHash()
{
  QHash<QgsAuthType::ProviderType, QString> typeNames;
  typeNames.insert( QgsAuthType::None, QObject::tr( "None" ) );
  typeNames.insert( QgsAuthType::Basic, QObject::tr( "Basic" ) );
#ifndef QT_NO_OPENSSL
  typeNames.insert( QgsAuthType::PkiPaths, QObject::tr( "PKI-Paths" ) );
  typeNames.insert( QgsAuthType::PkiPkcs12, QObject::tr( "PKI-PKCS#12" ) );
#endif
  typeNames.insert( QgsAuthType::Unknown, QObject::tr( "Unknown" ) );
  return typeNames;
}

QgsAuthType::ProviderType QgsAuthType::providerTypeFromInt( int itype )
{
  QgsAuthType::ProviderType ptype = Unknown;
  switch ( itype )
  {
    case 0:
      ptype = None;
      break;
    case 1:
      ptype = Basic;
      break;
#ifndef QT_NO_OPENSSL
    case 2:
      ptype = PkiPaths;
      break;
    case 3:
      ptype = PkiPkcs12;
      break;
#endif
    case 20:
      // Unknown
      break;
    default:
      break;
  }

  return ptype;

}

const QString QgsAuthType::typeToString( QgsAuthType::ProviderType providertype )
{
  return QgsAuthType::typeNameHash().value( providertype, QObject::tr( "Unknown" ) );
}

QgsAuthType::ProviderType QgsAuthType::stringToType( const QString& name )
{
  return QgsAuthType::typeNameHash().key( name, QgsAuthType::Unknown );
}

const QString QgsAuthType::typeDescription( QgsAuthType::ProviderType providertype )
{
  QString s = QObject::tr( "No authentication set" );
  switch ( providertype )
  {
    case None:
      break;
    case Basic:
      s = QObject::tr( "Basic authentication" );
      break;
#ifndef QT_NO_OPENSSL
    case PkiPaths:
      s = QObject::tr( "PKI paths authentication" );
      break;
    case PkiPkcs12:
      s = QObject::tr( "PKI PKCS#12 authentication" );
      break;
#endif
    case Unknown:
      s = QObject::tr( "Unsupported authentication" );
      break;
    default:
      break;
  }
  return s;
}


//////////////////////////////////////////////
// QgsAuthConfigBase
//////////////////////////////////////////////

const QString QgsAuthConfigBase::mConfSep = "|||";

// get uniqueConfigId only on save
QgsAuthConfigBase::QgsAuthConfigBase( QgsAuthType::ProviderType type, int version )
    : mId( QString() )
    , mName( QString() )
    , mUri( QString() )
    , mType( type )
    , mVersion( version )
{
}

QgsAuthConfigBase::QgsAuthConfigBase( const QgsAuthConfigBase &config )
    : mId( config.id() )
    , mName( config.name() )
    , mUri( config.uri() )
    , mType( config.type() )
    , mVersion( config.version() )
{
}

const QString QgsAuthConfigBase::typeToString() const
{
  return QgsAuthType::typeToString( mType );
}

bool QgsAuthConfigBase::isValid( bool validateid ) const
{
  bool idvalid = validateid ? !mId.isEmpty() : true;

  return (
           idvalid
           && !mName.isEmpty()
           && mType != QgsAuthType::Unknown
         );
}

const QgsAuthConfigBase QgsAuthConfigBase::toBaseConfig()
{
  return QgsAuthConfigBase( *this );
}


//////////////////////////////////////////////
// QgsAuthConfigBasic
//////////////////////////////////////////////

QgsAuthConfigBasic::QgsAuthConfigBasic()
    : QgsAuthConfigBase( QgsAuthType::Basic, 1 )
    , mRealm( QString() )
    , mUsername( QString() )
    , mPassword( QString() )
{
}

bool QgsAuthConfigBasic::isValid( bool validateid ) const
{
  // password can be empty
  return (
           QgsAuthConfigBase::isValid( validateid )
           && mVersion != 0
           && !mUsername.isEmpty()
         );
}

const QString QgsAuthConfigBasic::configString() const
{
  QStringList configlist = QStringList() << mRealm << mUsername << mPassword;
  return configlist.join( mConfSep );
}

void QgsAuthConfigBasic::loadConfigString( const QString& config )
{
  if ( config.isEmpty() )
  {
    return;
  }
  QStringList configlist = config.split( mConfSep );
  mRealm = configlist.at( 0 );
  mUsername = configlist.at( 1 );
  mPassword = configlist.at( 2 );
}

//////////////////////////////////////////////
// QgsAuthConfigPkiPaths
//////////////////////////////////////////////

QgsAuthConfigPkiPaths::QgsAuthConfigPkiPaths()
    : QgsAuthConfigBase( QgsAuthType::PkiPaths, 1 )
    , mCertId( QString() )
    , mKeyId( QString() )
    , mKeyPass( QString() )
    , mIssuerId( QString() )
    , mIssuerSelf( false )
{
}

const QString QgsAuthConfigPkiPaths::certAsPem() const
{
  if ( !isValid() )
    return QString();

  return QString( QgsAuthProviderPkiPaths::certAsPem( certId() ) );
}

const QStringList QgsAuthConfigPkiPaths::keyAsPem( bool reencrypt ) const
{
  if ( !isValid() )
    return QStringList() << QString() << QString();

  QString algtype;
  QByteArray keydata( QgsAuthProviderPkiPaths::keyAsPem( keyId(), keyPassphrase(), &algtype, reencrypt ) );
  return QStringList() << QString( keydata ) << algtype;
}

const QString QgsAuthConfigPkiPaths::issuerAsPem() const
{
  if ( !isValid() )
    return QString();

  return QString( QgsAuthProviderPkiPaths::issuerAsPem( issuerId() ) );
}

bool QgsAuthConfigPkiPaths::isValid( bool validateid ) const
{
  return (
           QgsAuthConfigBase::isValid( validateid )
           && mVersion != 0
           && !mCertId.isEmpty()
           && !mKeyId.isEmpty()
         );
}

const QString QgsAuthConfigPkiPaths::configString() const
{
  QStringList configlist = QStringList();
  configlist << mCertId << mKeyId << mKeyPass << mIssuerId << QString::number( mIssuerSelf );
  return configlist.join( mConfSep );
}

void QgsAuthConfigPkiPaths::loadConfigString( const QString& config )
{
  if ( config.isEmpty() )
  {
    return;
  }
  QStringList configlist = config.split( mConfSep );
  mCertId = configlist.at( 0 );
  mKeyId = configlist.at( 1 );
  mKeyPass = configlist.at( 2 );
  mIssuerId = configlist.at( 3 );
  mIssuerSelf = ( bool ) configlist.at( 4 ).toInt();
}

//////////////////////////////////////////////
// QgsAuthConfigPkiPkcs12
//////////////////////////////////////////////

QgsAuthConfigPkiPkcs12::QgsAuthConfigPkiPkcs12()
    : QgsAuthConfigBase( QgsAuthType::PkiPkcs12, 1 )
    , mBundlePath( QString() )
    , mBundlePass( QString() )
    , mIssuerPath( QString() )
    , mIssuerSelf( false )
{
}

const QString QgsAuthConfigPkiPkcs12::certAsPem() const
{
  if ( !isValid() )
    return QString();

  return QgsAuthProviderPkiPkcs12::certAsPem( bundlePath(), bundlePassphrase() );
}

const QStringList QgsAuthConfigPkiPkcs12::keyAsPem( bool reencrypt ) const
{
  if ( !isValid() )
    return QStringList();

  QStringList keylist;
  keylist << QgsAuthProviderPkiPkcs12::keyAsPem( bundlePath(), bundlePassphrase(), reencrypt );
  keylist << QString( "rsa" );
  return keylist;
}

const QString QgsAuthConfigPkiPkcs12::issuerAsPem() const
{
  if ( !isValid() )
    return QString();

  return QgsAuthProviderPkiPkcs12::issuerAsPem( bundlePath(), bundlePassphrase(), issuerPath() );
}

bool QgsAuthConfigPkiPkcs12::isValid( bool validateid ) const
{
  // TODO: add more robust validation via QCA (primary cert, key and issuer chain)?
  return (
           QgsAuthConfigBase::isValid( validateid )
           && version() != 0
           && !bundlePath().isEmpty()
           && QFile::exists( bundlePath() )
         );
}

const QString QgsAuthConfigPkiPkcs12::configString() const
{
  QStringList configlist = QStringList();
  configlist << bundlePath() << bundlePassphrase() << issuerPath() << QString::number( issuerSelfSigned() );
  return configlist.join( mConfSep );
}

void QgsAuthConfigPkiPkcs12::loadConfigString( const QString &config )
{
  if ( config.isEmpty() )
    return;

  QStringList configlist = config.split( mConfSep );
  setBundlePath( configlist.at( 0 ) );
  setBundlePassphrase( configlist.at( 1 ) );
  setIssuerPath( configlist.at( 2 ) );
  setIssuerSelfSigned(( bool ) configlist.at( 3 ).toInt() );
}
