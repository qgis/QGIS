/***************************************************************************
    qgsauthenticationconfig.h
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

#ifndef QGSAUTHENTICATIONCONFIG_H
#define QGSAUTHENTICATIONCONFIG_H

#include <QHash>
#include <QString>


class CORE_EXPORT QgsAuthType
{
  public:
    enum ProviderType
    {
      None = 0,
      Basic = 1,
#ifndef QT_NO_OPENSSL
      PkiPaths = 2,
      PkiPkcs12 = 3,
#endif
      Unknown = 20 // padding for more standard auth types
    };

    static const QHash<QgsAuthType::ProviderType, QString> typeNameHash();

    static QgsAuthType::ProviderType providerTypeFromInt( int itype );

    static const QString typeToString( QgsAuthType::ProviderType providertype = None );

    static QgsAuthType::ProviderType stringToType( const QString& name );

    static const QString typeDescription( QgsAuthType::ProviderType providertype = None );
};

/**
 * @brief Base class for configs
 */
class CORE_EXPORT QgsAuthConfigBase
{
  public:

    QgsAuthConfigBase( QgsAuthType::ProviderType type = QgsAuthType::None, int version = 0 );

    QgsAuthConfigBase( const QgsAuthConfigBase& config );

    virtual ~QgsAuthConfigBase() {}

    const QString id() const { return mId; }
    void setId( const QString& id ) { mId = id; }

    const QString name() const { return mName; }
    void setName( const QString& name ) { mName = name; }

    const QString uri() const { return mUri; }
    void setUri( const QString& uri ) { mUri = uri; }

    QgsAuthType::ProviderType type() const { return mType; }
    void setType( QgsAuthType::ProviderType ptype ) { mType = ptype; }

    int version() const { return mVersion; }
    void setVersion( int version ) { mVersion = version; }

    const QString typeToString() const;

    virtual bool isValid( bool validateid = false ) const;

    virtual const QString configString() const { return QString(); }
    virtual void loadConfigString( const QString& config ) { Q_UNUSED( config ); }

    const QgsAuthConfigBase toBaseConfig();

  protected:
    QString mId;
    QString mName;
    QString mUri;
    QgsAuthType::ProviderType mType;
    int mVersion;

    static const QString mConfSep;
};


class CORE_EXPORT QgsAuthConfigBasic: public QgsAuthConfigBase
{
  public:
    QgsAuthConfigBasic();

    QgsAuthConfigBasic( const QgsAuthConfigBase& config )
        : QgsAuthConfigBase( config ) {}

    ~QgsAuthConfigBasic() {}

    const QString realm() const { return mRealm; }
    void setRealm( const QString& realm ) { mRealm = realm; }

    const QString username() const { return mUsername; }
    void setUsername( const QString& name ) { mUsername = name; }

    const QString password() const { return mPassword; }
    void setPassword( const QString& pass ) { mPassword = pass; }

    bool isValid( bool validateid = false ) const;

    const QString configString() const;
    void loadConfigString( const QString& config = QString() );

  private:
    QString mRealm;
    QString mUsername;
    QString mPassword;
};

class CORE_EXPORT QgsAuthConfigPkiPaths: public QgsAuthConfigBase
{
  public:
    QgsAuthConfigPkiPaths();

    QgsAuthConfigPkiPaths( const QgsAuthConfigBase& config )
        : QgsAuthConfigBase( config ) {}

    ~QgsAuthConfigPkiPaths() {}

    const QString certId() const { return mCertId; }
    void setCertId( const QString& id ) { mCertId = id; }

    const QString keyId() const { return mKeyId; }
    void setKeyId( const QString& id ) { mKeyId = id; }

    const QString keyPassphrase() const { return mKeyPass; }
    void setKeyPassphrase( const QString& passphrase ) { mKeyPass = passphrase; }

    const QString issuerId() const { return mIssuerId; }
    void setIssuerId( const QString& id ) { mIssuerId = id; }

    bool issuerSelfSigned() const { return mIssuerSelf; }
    void setIssuerSelfSigned( bool selfsigned ) { mIssuerSelf = selfsigned; }

    const QString certAsPem() const;

    const QStringList keyAsPem( bool reencrypt = true ) const;

    const QString issuerAsPem() const;

    bool isValid( bool validateid = false ) const;

    const QString configString() const;
    void loadConfigString( const QString& config = QString() );

  private:
    QString mCertId;
    QString mKeyId;
    QString mKeyPass;
    QString mIssuerId;
    bool mIssuerSelf;
};

class CORE_EXPORT QgsAuthConfigPkiPkcs12: public QgsAuthConfigBase
{
  public:
    QgsAuthConfigPkiPkcs12();

    QgsAuthConfigPkiPkcs12( const QgsAuthConfigBase& config )
        : QgsAuthConfigBase( config ) {}

    ~QgsAuthConfigPkiPkcs12() {}

    const QString bundlePath() const { return mBundlePath; }
    void setBundlePath( const QString& path ) { mBundlePath = path; }

    const QString bundlePassphrase() const { return mBundlePass; }
    void setBundlePassphrase( const QString& passphrase ) { mBundlePass = passphrase; }

    const QString issuerPath() const { return mIssuerPath; }
    void setIssuerPath( const QString& id ) { mIssuerPath = id; }

    bool issuerSelfSigned() const { return mIssuerSelf; }
    void setIssuerSelfSigned( bool selfsigned ) { mIssuerSelf = selfsigned; }

    const QString certAsPem() const;

    const QStringList keyAsPem( bool reencrypt = true ) const;

    const QString issuerAsPem() const;

    bool isValid( bool validateid = false ) const;

    const QString configString() const;
    void loadConfigString( const QString& config = QString() );

  private:
    QString mBundlePath;
    QString mBundlePass;
    QString mIssuerPath;
    bool mIssuerSelf;
};

#endif // QGSAUTHENTICATIONCONFIG_H
