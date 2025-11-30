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

    [[nodiscard]] QString dsn() const { return mDsn; }
    void setDsn( const QString &dsn ) { mDsn = dsn; }

    [[nodiscard]] QString driver() const { return mDriver; }
    void setDriver( const QString &driver ) { mDriver = driver; }

    [[nodiscard]] QString host() const { return mHost; }
    void setHost( const QString &host ) { mHost = host; }

    [[nodiscard]] QString port() const { return mPort; }
    void setPort( const QString &port ) { mPort = port; }

    [[nodiscard]] QString database() const { return mDatabase; }
    void setDatabase( const QString &database ) { mDatabase = database; }

    [[nodiscard]] QString schema() const { return mSchema; }
    void setSchema( const QString &schema ) { mSchema = schema; }

    [[nodiscard]] QString userName() const { return mUserName; }
    void setUserName( const QString &userName ) { mUserName = userName; }

    [[nodiscard]] QString password() const { return mPassword; }
    void setPassword( const QString &password ) { mPassword = password; }

    [[nodiscard]] bool enableSsl() const { return mSslEnabled; }
    void setEnableSsl( bool value ) { mSslEnabled = value; }

    [[nodiscard]] QString cryptoProvider() const { return mSslCryptoProvider; }
    void setCryptoProvider( const QString &value ) { mSslCryptoProvider = value; }

    [[nodiscard]] QString sslKeyStore() const { return mSslKeyStore; }
    void setSslKeyStore( const QString &value ) { mSslKeyStore = value; }

    [[nodiscard]] QString sslTrustStore() const { return mSslTrustStore; }
    void setSslTrustStore( const QString &value ) { mSslTrustStore = value; }

    [[nodiscard]] bool sslValidateCertificate() const { return mSslValidateCertificate; }
    void setSslValidateCertificate( bool value ) { mSslValidateCertificate = value; }

    [[nodiscard]] QString sslHostNameInCertificate() const { return mSslHostNameInCertificate; }
    void setSslHostNameInCertificate( const QString &value ) { mSslHostNameInCertificate = value; }

    [[nodiscard]] QString toString() const;

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
    uint mProxyPort = 0;
    QString mProxyUsername;
    QString mProxyPassword;
};

#endif // QGSHANACONNECTIONSTRINGBUILDER_H
