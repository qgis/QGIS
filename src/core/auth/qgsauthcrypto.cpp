/***************************************************************************
    qgsauthcrypto.cpp
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

#include "qgsauthcrypto.h"

#include <QObject>
#include <QtCrypto>

// defines culled from MeePasswords (GPL2)
// https://github.com/ruedigergad/meepasswords/blob/master/entrystorage.h
#define CIPHER_SIGNATURE "aes256-cbc-pkcs7"
#define CIPHER_TYPE "aes256"
#define CIPHER_MODE QCA::Cipher::CBC
#define CIPHER_PADDING QCA::Cipher::PKCS7
#define CIPHER_IV_LENGTH 32
#define CIPHER_PROVIDER "qca-ossl"
#define PASSWORD_HASH_ALGORITHM "sha256"
#define KEY_GEN_ITERATIONS 10000
#define KEY_GEN_LENGTH 16
#define KEY_GEN_IV_LENGTH 16

bool QgsAuthCrypto::isDisabled()
{
  if ( !QCA::isSupported( CIPHER_SIGNATURE, CIPHER_PROVIDER ) )
  {
    qDebug( "Authentication system DISABLED: QCA's qca-ossl (OpenSSL) plugin is missing" );
    return true;
  }
  return false;
}

const QString QgsAuthCrypto::encrypt( const QString& pass, const QString& cipheriv, const QString& text )
{
  if ( QgsAuthCrypto::isDisabled() )
    return QString();

  return encryptdecrypt( pass, cipheriv, text, true );
}

const QString QgsAuthCrypto::decrypt( const QString& pass, const QString& cipheriv, const QString& text )
{
  if ( QgsAuthCrypto::isDisabled() )
    return QString();

  return encryptdecrypt( pass, cipheriv, text, false );
}

static QCA::SymmetricKey passwordKey_( const QString& pass, const QCA::InitializationVector& salt )
{
  QCA::SecureArray passarray( QByteArray( pass.toUtf8().constData() ) );
  QCA::SecureArray passhash( QCA::Hash( PASSWORD_HASH_ALGORITHM ).hash( passarray ) );
  return QCA::PBKDF2().makeKey( passhash, salt, KEY_GEN_LENGTH, KEY_GEN_ITERATIONS );
}

void QgsAuthCrypto::passwordKeyHash( const QString& pass, QString *salt, QString *hash, QString *cipheriv )
{
  if ( QgsAuthCrypto::isDisabled() )
    return;

  QCA::InitializationVector saltiv = QCA::InitializationVector( KEY_GEN_IV_LENGTH );
  QCA::SymmetricKey key = passwordKey_( pass, saltiv );

  if ( !key.isEmpty() )
  {
    *salt = QCA::arrayToHex( saltiv.toByteArray() );
    qDebug( "salt hex: %s", qPrintable( *salt ) );

    *hash = QCA::arrayToHex( key.toByteArray() );
    qDebug( "hash hex: %s", qPrintable( *hash ) );

    if ( cipheriv )
    {
      *cipheriv = QCA::arrayToHex( QCA::InitializationVector( CIPHER_IV_LENGTH ).toByteArray() );
      qDebug( "cipheriv hex: %s", qPrintable( *cipheriv ) );
    }
  }
}

bool QgsAuthCrypto::verifyPasswordKeyHash( const QString& pass,
    const QString& salt,
    const QString& hash,
    QString *hashderived )
{
  if ( QgsAuthCrypto::isDisabled() )
    return false;

  QCA::InitializationVector saltiv( QCA::hexToArray( salt ) );
  QString derived( QCA::arrayToHex( passwordKey_( pass, saltiv ).toByteArray() ) );

  if ( hashderived )
  {
    *hashderived = derived;
  }

  return hash == derived;
}

QString QgsAuthCrypto::encryptdecrypt( const QString& passstr,
                                       const QString& cipheriv,
                                       const QString& textstr,
                                       bool encrypt )
{
  QString outtxt = QString();
  if ( QgsAuthCrypto::isDisabled() )
    return outtxt;

  QCA::InitializationVector iv( QCA::hexToArray( cipheriv ) );

  QCA::SymmetricKey key( QCA::SecureArray( QByteArray( passstr.toUtf8().constData() ) ) );

  if ( encrypt )
  {
    QCA::Cipher cipher = QCA::Cipher( CIPHER_TYPE, CIPHER_MODE, CIPHER_PADDING,
                                      QCA::Encode, key, iv,
                                      CIPHER_PROVIDER );

    QCA::SecureArray securedata( textstr.toUtf8() );
    QCA::SecureArray encrypteddata( cipher.process( securedata ) );
    if ( !cipher.ok() )
    {
      qDebug( "Encryption failed!" );
      return outtxt;
    }
    outtxt = QCA::arrayToHex( encrypteddata.toByteArray() );
    // qDebug( "Encrypted hex: %s", qPrintable( outtxt ) );
  }
  else
  {
    QCA::Cipher cipher = QCA::Cipher( CIPHER_TYPE, CIPHER_MODE, CIPHER_PADDING,
                                      QCA::Decode, key, iv,
                                      CIPHER_PROVIDER );

    QCA::SecureArray ciphertext( QCA::hexToArray( textstr ) );
    QCA::SecureArray decrypteddata( cipher.process( ciphertext ) );
    if ( !cipher.ok() )
    {
      qDebug( "Decryption failed!" );
      return outtxt;
    }

    outtxt = QString( decrypteddata.toByteArray() );
    // qDebug( "Decrypted text %s", qPrintable( outtxt ) ); // DO NOT LEAVE THIS LINE UNCOMMENTED
  }

  return outtxt;
}
