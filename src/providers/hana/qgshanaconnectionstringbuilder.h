/***************************************************************************
   qgshanaconnectionstringbuilder.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
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

    const QString &getDriver() const { return mDriver; }
    void setDriver( const QString &driver ) { mDriver = driver; }

    const QString &getHost() const { return mHost; }
    void setHost( const QString &host ) { mHost = host; }

    const QString &getPort() const { return mPort; }
    void setPort( const QString &port ) { mPort = port; }

    const QString &getDatabase() const { return mDatabase; }
    void setDatabase( const QString &database ) { mDatabase = database; }

    const QString &getSchema() const { return mSchema; }
    void setSchema( const QString &schema ) { mSchema = schema; }

    const QString &getUserName() const { return mUserName; }
    void setUserName( const QString &userName ) { mUserName = userName; }

    const QString &getPassword() const { return mPassword; }
    void setPassword( const QString &password ) { mPassword = password; }

    bool getEnableSsl() const { return mSslEnabled; }
    void setEnableSsl( bool value ) { mSslEnabled = value; }

    const QString &getCryptoProvider() const { return mSslCryptoProvider; }
    void setCryptoProvider( const QString &value ) { mSslCryptoProvider = value; }

    const QString &getSslKeyStore() const { return mSslKeyStore; }
    void setSslKeyStore( const QString &value ) { mSslKeyStore = value; }

    const QString &getSslTrustStore() const { return mSslTrustStore; }
    void setSslTrustStore( const QString &value ) { mSslTrustStore = value; }

    bool getSslValidateCertificate() const { return mSslValidateCertificate; }
    void setSslValidateCertificate( bool value ) { mSslValidateCertificate = value; }

    const QString &getSslHostNameInCertificate() const { return mSslHostNameInCertificate; }
    void setSslHostNameInCertificate( const QString &value ) { mSslHostNameInCertificate = value; }

    QString toString();

  private:
    QString mDriver;
    QString mHost;
    QString mPort;
    QString mDatabase;
    QString mSchema;
    QString mUserName;
    QString mPassword;
    // Ssl parameters
    bool mSslEnabled;
    QString mSslCryptoProvider;
    QString mSslKeyStore;
    QString mSslTrustStore;
    bool mSslValidateCertificate;
    QString mSslHostNameInCertificate;
};

#endif // QGSHANACONNECTIONSTRINGBUILDER_H
