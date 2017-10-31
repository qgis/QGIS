/***************************************************************************
    begin                : July 30, 2016
    copyright            : (C) 2016 by Monsanto Company, USA
    author               : Larry Shaffer, Boundless Spatial
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHOAUTH2CONFIG_H
#define QGSAUTHOAUTH2CONFIG_H

// TODO: add SimpleCrypt or QgsAuthCrypto for (en|de)crypting client secret key

#include <QObject>
#include <QVariantMap>

#include "qgis.h"


class QgsAuthOAuth2Config : public QObject
{
    Q_OBJECT
    Q_ENUMS( ConfigType )
    Q_ENUMS( GrantFlow )
    Q_ENUMS( ConfigFormat )
    Q_ENUMS( AccessMethod )

  public:

    enum ConfigType
    {
      Predefined,
      Custom,
    };

    enum GrantFlow
    {
      AuthCode,      //!< @see http://tools.ietf.org/html/rfc6749#section-4.1
      Implicit,      //!< @see http://tools.ietf.org/html/rfc6749#section-4.2
      ResourceOwner, //!< @see http://tools.ietf.org/html/rfc6749#section-4.3
    };

    enum ConfigFormat
    {
      JSON,
    };

    enum AccessMethod
    {
      Header,
      Form,
      Query,
    };

    explicit QgsAuthOAuth2Config( QObject *parent = nullptr );

    ~QgsAuthOAuth2Config();

    //! Unique ID
    Q_PROPERTY( QString id READ id WRITE setId NOTIFY idChanged )
    QString id() const { return mId; }

    //! Increment this if method is significantly updated, allow updater code to be written
    Q_PROPERTY( int version READ version WRITE setVersion NOTIFY versionChanged )
    int version() const { return mVersion; }

    //! Configuration type
    Q_PROPERTY( ConfigType configType READ configType WRITE setConfigType NOTIFY configTypeChanged )
    ConfigType configType() const { return mConfigType; }

    //! Authorization flow
    Q_PROPERTY( GrantFlow grantFlow READ grantFlow WRITE setGrantFlow NOTIFY grantFlowChanged )
    GrantFlow grantFlow()  const { return mGrantFlow; }

    //! Configuration name
    Q_PROPERTY( QString name READ name WRITE setName NOTIFY nameChanged )
    QString name() const { return mName; }

    //! Configuration description
    Q_PROPERTY( QString description READ description WRITE setDescription NOTIFY descriptionChanged )
    QString description() const { return mDescription; }

    //!
    Q_PROPERTY( QString requestUrl READ requestUrl WRITE setRequestUrl NOTIFY requestUrlChanged )
    QString requestUrl() const { return mRequestUrl; }

    //!
    Q_PROPERTY( QString tokenUrl READ tokenUrl WRITE setTokenUrl NOTIFY tokenUrlChanged )
    QString tokenUrl() const { return mTokenUrl; }

    //!
    Q_PROPERTY( QString refreshTokenUrl READ refreshTokenUrl WRITE setRefreshTokenUrl NOTIFY refreshTokenUrlChanged )
    QString refreshTokenUrl() const { return mRefreshTokenUrl; }

    //!
    Q_PROPERTY( QString redirectUrl READ redirectUrl WRITE setRedirectUrl NOTIFY redirectUrlChanged )
    QString redirectUrl() const { return mRedirectURL; }

    //!
    Q_PROPERTY( int redirectPort READ redirectPort WRITE setRedirectPort NOTIFY redirectPortChanged )
    int redirectPort() const { return mRedirectPort; }

    //!
    Q_PROPERTY( QString clientId READ clientId WRITE setClientId NOTIFY clientIdChanged )
    QString clientId() const { return mClientId; }

    //!
    Q_PROPERTY( QString clientSecret READ clientSecret WRITE setClientSecret NOTIFY clientSecretChanged )
    QString clientSecret() const { return mClientSecret; }

    //! Resource owner username
    Q_PROPERTY( QString username READ username WRITE setUsername NOTIFY usernameChanged )
    QString username() const { return mUsername; }

    //! Resource owner password
    Q_PROPERTY( QString password READ password WRITE setPassword NOTIFY passwordChanged )
    QString password() const { return mPassword; }

    //! Scope of authentication
    Q_PROPERTY( QString scope READ scope WRITE setScope NOTIFY scopeChanged )
    QString scope() const { return mScope; }

    //! State passed with request
    Q_PROPERTY( QString state READ state WRITE setState NOTIFY stateChanged )
    QString state() const { return mState; }

    //!
    Q_PROPERTY( QString apiKey READ apiKey WRITE setApiKey NOTIFY apiKeyChanged )
    QString apiKey() const { return mApiKey; }

    //!
    Q_PROPERTY( bool persistToken READ persistToken WRITE setPersistToken NOTIFY persistTokenChanged )
    bool persistToken() const { return mPersistToken; }

    //!
    Q_PROPERTY( AccessMethod accessMethod READ accessMethod WRITE setAccessMethod NOTIFY accessMethodChanged )
    AccessMethod accessMethod() const { return mAccessMethod; }

    //!
    Q_PROPERTY( int requestTimeout READ requestTimeout WRITE setRequestTimeout NOTIFY requestTimeoutChanged )
    int requestTimeout() const { return mRequestTimeout; }

    //!
    Q_PROPERTY( QVariantMap queryPairs READ queryPairs WRITE setQueryPairs NOTIFY queryPairsChanged )
    QVariantMap queryPairs() const { return mQueryPairs; }

    //! Operator used to compare configs' equality
    bool operator==( const QgsAuthOAuth2Config &other ) const;

    //! Operator used to compare configs' inequality
    bool operator!=( const QgsAuthOAuth2Config &other ) const;

    //! Check whether config is valid, then return it
    bool isValid() const;

    //! @see http://tools.ietf.org/html/rfc6749 for required data per flow
    void validateConfigId( bool needsId = false );

    //! Load a string (e.g. JSON) of a config
    bool loadConfigTxt( const QByteArray &configtxt, ConfigFormat format = JSON );

    //! Save a config to a string (e.g. JSON)
    QByteArray saveConfigTxt( ConfigFormat format = JSON, bool pretty = false, bool *ok = nullptr ) const;

    //!
    QVariantMap mappedProperties() const;

    //!
    static QByteArray serializeFromVariant( const QVariantMap &variant,
                                            ConfigFormat format = JSON,
                                            bool pretty = false,
                                            bool *ok = nullptr );

    //!
    static QVariantMap variantFromSerialized( const QByteArray &serial,
        ConfigFormat format = JSON,
        bool *ok = nullptr );

    //! Write config object out to a formatted file (e.g. JSON)
    static bool writeOAuth2Config( const QString &filepath,
                                   QgsAuthOAuth2Config *config,
                                   ConfigFormat format = JSON,
                                   bool pretty = false );

    //! Load and parse a directory of configs (e.g. JSON) to objects
    static QList<QgsAuthOAuth2Config *> loadOAuth2Configs(
      const QString &configdirectory,
      QObject *parent = nullptr,
      ConfigFormat format = JSON,
      bool *ok = nullptr );

    //! Load and parse a directory of configs (e.g. JSON) to a map
    static QgsStringMap mapOAuth2Configs(
      const QString &configdirectory,
      QObject *parent = nullptr,
      ConfigFormat format = JSON,
      bool *ok = nullptr );

    //! Load and parse standard directories of configs (e.g. JSON) to a mapped cache
    static QgsStringMap mappedOAuth2ConfigsCache( QObject *parent, const QString &extradir = QString::null );

    //!
    static QString oauth2ConfigsPkgDataDir();

    //!
    static QString oauth2ConfigsUserSettingsDir();

    //!
    static QString configTypeString( ConfigType configtype );

    //!
    static QString grantFlowString( GrantFlow flow );

    //!
    static QString accessMethodString( AccessMethod method );

    //!
    static QString tokenCacheDirectory( bool temporary = false );

    //!
    static QString tokenCacheFile( const QString &suffix = QString::null );

    //!
    static QString tokenCachePath( const QString &suffix = QString::null, bool temporary = false );

  public slots:
    void setId( const QString &value );
    void setVersion( int value );
    void setConfigType( ConfigType value );
    void setGrantFlow( GrantFlow value );
    void setName( const QString &value );
    void setDescription( const QString &value );
    void setRequestUrl( const QString &value );
    void setTokenUrl( const QString &value );
    void setRefreshTokenUrl( const QString &value );
    void setRedirectUrl( const QString &value );
    void setRedirectPort( int value );
    void setClientId( const QString &value );
    void setClientSecret( const QString &value );
    void setUsername( const QString &value );
    void setPassword( const QString &value );
    void setScope( const QString &value );
    void setState( const QString &value );
    void setApiKey( const QString &value );
    // advanced
    void setPersistToken( bool persist );
    void setAccessMethod( AccessMethod value );
    void setRequestTimeout( int value );
    void setQueryPairs( const QVariantMap &pairs );

    void setToDefaults();

    void validateConfig();

  signals:
    void configChanged();
    void idChanged( const QString & );
    void versionChanged( int );
    void configTypeChanged( ConfigType );
    void grantFlowChanged( GrantFlow );
    void nameChanged( const QString & );
    void descriptionChanged( const QString & );
    void requestUrlChanged( const QString & );
    void tokenUrlChanged( const QString & );
    void refreshTokenUrlChanged( const QString & );
    void redirectUrlChanged( const QString & );
    void redirectPortChanged( int );
    void clientIdChanged( const QString & );
    void clientSecretChanged( const QString & );
    void usernameChanged( const QString & );
    void passwordChanged( const QString & );
    void scopeChanged( const QString & );
    void stateChanged( const QString & );
    void apiKeyChanged( const QString & );

    // advanced
    void persistTokenChanged( bool );
    void accessMethodChanged( AccessMethod );
    void requestTimeoutChanged( int );
    void queryPairsChanged( const QVariantMap & );

    void validityChanged( bool );

  private:
    QString mId;
    int mVersion;
    ConfigType mConfigType;
    GrantFlow mGrantFlow;
    QString mName;
    QString mDescription;
    QString mRequestUrl;
    QString mTokenUrl;
    QString mRefreshTokenUrl;
    QString mRedirectURL;
    int mRedirectPort;
    QString mClientId;
    QString mClientSecret;
    QString mUsername;
    QString mPassword;
    QString mScope;
    QString mState;
    QString mApiKey;
    bool mPersistToken;
    AccessMethod mAccessMethod;
    int mRequestTimeout; // in seconds
    QVariantMap mQueryPairs;
    bool mValid;
};

#endif // QGSAUTHOAUTH2CONFIG_H
