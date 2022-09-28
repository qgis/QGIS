/***************************************************************************
   qgshanaconnectionstringbuilder.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANACONNECTIONSTRINGBUILDER_H
#define QGSHANACONNECTIONSTRINGBUILDER_H

#include "qgsdatasourceuri.h"

class QgsHanaConnectionStringBuilder
{
  public:
    QgsHanaConnectionStringBuilder() = default;
    explicit QgsHanaConnectionStringBuilder( const QgsDataSourceUri &uri );

    QString dsn() const { return mDsn; }
    void setDsn( const QString &dsn ) { mDsn = dsn; }

    QString driver() const { return mDriver; }
    void setDriver( const QString &driver ) { mDriver = driver; }

    QString host() const { return mHost; }
    void setHost( const QString &host ) { mHost = host; }

    QString port() const { return mPort; }
    void setPort( const QString &port ) { mPort = port; }

    QString database() const { return mDatabase; }
    void setDatabase( const QString &database ) { mDatabase = database; }

    QString schema() const { return mSchema; }
    void setSchema( const QString &schema ) { mSchema = schema; }

    QString userName() const { return mUserName; }
    void setUserName( const QString &userName ) { mUserName = userName; }

    QString password() const { return mPassword; }
    void setPassword( const QString &password ) { mPassword = password; }

    bool enableSsl() const { return mSslEnabled; }
    void setEnableSsl( bool value ) { mSslEnabled = value; }

    QString cryptoProvider() const { return mSslCryptoProvider; }
    void setCryptoProvider( const QString &value ) { mSslCryptoProvider = value; }

    QString sslKeyStore() const { return mSslKeyStore; }
    void setSslKeyStore( const QString &value ) { mSslKeyStore = value; }

    QString sslTrustStore() const { return mSslTrustStore; }
    void setSslTrustStore( const QString &value ) { mSslTrustStore = value; }

    bool sslValidateCertificate() const { return mSslValidateCertificate; }
    void setSslValidateCertificate( bool value ) { mSslValidateCertificate = value; }

    QString sslHostNameInCertificate() const { return mSslHostNameInCertificate; }
    void setSslHostNameInCertificate( const QString &value ) { mSslHostNameInCertificate = value; }

    QString toString() const;

  private:
    QString mDsn;
    QString mDriver;
    QString mHost;
    QString mPort;
    QString mDatabase;
    QString mSchema;
    QString mUserName;
    QString mPassword;
    // SSL parameters
    bool mSslEnabled = false;
    QString mSslCryptoProvider;
    QString mSslKeyStore;
    QString mSslTrustStore;
    bool mSslValidateCertificate = false;
    QString mSslHostNameInCertificate;
    // Proxy parameters
    bool mProxyEnabled = false;
    bool mProxyHttp = false;
    QString mProxyHost;
    uint mProxyPort;
    QString mProxyUsername;
    QString mProxyPassword;
};

#endif // QGSHANACONNECTIONSTRINGBUILDER_H
