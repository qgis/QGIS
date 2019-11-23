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

/**
 * The QgsAuthOAuth2Config class stores the configuration for OAuth2 authentication plugin
 * \ingroup auth_plugins
 * \since QGIS 3.4
 */
class QgsAuthOAuth2Config : public QObject
{
    Q_OBJECT
    Q_ENUMS( ConfigType )
    Q_ENUMS( GrantFlow )
    Q_ENUMS( ConfigFormat )
    Q_ENUMS( AccessMethod )
    Q_PROPERTY( QString id READ id WRITE setId NOTIFY idChanged )
    Q_PROPERTY( int version READ version WRITE setVersion NOTIFY versionChanged )
    Q_PROPERTY( ConfigType configType READ configType WRITE setConfigType NOTIFY configTypeChanged )
    Q_PROPERTY( GrantFlow grantFlow READ grantFlow WRITE setGrantFlow NOTIFY grantFlowChanged )
    Q_PROPERTY( QString name READ name WRITE setName NOTIFY nameChanged )
    Q_PROPERTY( QString description READ description WRITE setDescription NOTIFY descriptionChanged )
    Q_PROPERTY( QString requestUrl READ requestUrl WRITE setRequestUrl NOTIFY requestUrlChanged )
    Q_PROPERTY( QString tokenUrl READ tokenUrl WRITE setTokenUrl NOTIFY tokenUrlChanged )
    Q_PROPERTY( QString refreshTokenUrl READ refreshTokenUrl WRITE setRefreshTokenUrl NOTIFY refreshTokenUrlChanged )
    Q_PROPERTY( QString redirectUrl READ redirectUrl WRITE setRedirectUrl NOTIFY redirectUrlChanged )
    Q_PROPERTY( int redirectPort READ redirectPort WRITE setRedirectPort NOTIFY redirectPortChanged )
    Q_PROPERTY( QString clientId READ clientId WRITE setClientId NOTIFY clientIdChanged )
    Q_PROPERTY( QString clientSecret READ clientSecret WRITE setClientSecret NOTIFY clientSecretChanged )
    Q_PROPERTY( QString username READ username WRITE setUsername NOTIFY usernameChanged )
    Q_PROPERTY( QString password READ password WRITE setPassword NOTIFY passwordChanged )
    Q_PROPERTY( QString scope READ scope WRITE setScope NOTIFY scopeChanged )
    Q_PROPERTY( QString apiKey READ apiKey WRITE setApiKey NOTIFY apiKeyChanged )
    Q_PROPERTY( bool persistToken READ persistToken WRITE setPersistToken NOTIFY persistTokenChanged )
    Q_PROPERTY( AccessMethod accessMethod READ accessMethod WRITE setAccessMethod NOTIFY accessMethodChanged )
    Q_PROPERTY( int requestTimeout READ requestTimeout WRITE setRequestTimeout NOTIFY requestTimeoutChanged )
    Q_PROPERTY( QVariantMap queryPairs READ queryPairs WRITE setQueryPairs NOTIFY queryPairsChanged )

  public:

    //! Configuration type
    enum ConfigType
    {
      Predefined,
      Custom,
    };

    //! OAuth2 grant flow
    enum GrantFlow
    {
      AuthCode,      //!< @see http://tools.ietf.org/html/rfc6749#section-4.1
      Implicit,      //!< @see http://tools.ietf.org/html/rfc6749#section-4.2
      ResourceOwner, //!< @see http://tools.ietf.org/html/rfc6749#section-4.3
    };

    //! Configuration format for serialize/unserialize operations
    enum ConfigFormat
    {
      JSON,
    };

    //! Access method
    enum AccessMethod
    {
      Header,
      Form,
      Query,
    };

    //! Construct a QgsAuthOAuth2Config instance
    explicit QgsAuthOAuth2Config( QObject *parent = nullptr );

    //! Unique ID
    QString id() const { return mId; }

    //! Increment this if method is significantly updated, allow updater code to be written
    int version() const { return mVersion; }

    //! Configuration type
    ConfigType configType() const { return mConfigType; }

    //! Authorization flow
    GrantFlow grantFlow()  const { return mGrantFlow; }

    //! Configuration name
    QString name() const { return mName; }

    //! Configuration description
    QString description() const { return mDescription; }

    //! Request url
    QString requestUrl() const { return mRequestUrl; }

    //! Token url
    QString tokenUrl() const { return mTokenUrl; }

    //! Refresh token url
    QString refreshTokenUrl() const { return mRefreshTokenUrl; }

    //! Redirect url
    QString redirectUrl() const { return mRedirectURL; }

    //! Redirect port
    int redirectPort() const { return mRedirectPort; }

    //! Client id
    QString clientId() const { return mClientId; }

    //! Client secret
    QString clientSecret() const { return mClientSecret; }

    //! Resource owner username
    QString username() const { return mUsername; }

    //! Resource owner password
    QString password() const { return mPassword; }

    //! Scope of authentication
    QString scope() const { return mScope; }

    //! API key
    QString apiKey() const { return mApiKey; }

    //! Returns TRUE if the token is persistent
    bool persistToken() const { return mPersistToken; }

    //! Access method
    AccessMethod accessMethod() const { return mAccessMethod; }

    //! Request timeout
    int requestTimeout() const { return mRequestTimeout; }

    //! Query pairs
    QVariantMap queryPairs() const { return mQueryPairs; }

    //! Operator used to compare configs' equality
    bool operator==( const QgsAuthOAuth2Config &other ) const;

    //! Operator used to compare configs' inequality
    bool operator!=( const QgsAuthOAuth2Config &other ) const;

    //! Check whether config is valid, then return it
    bool isValid() const;

    //! \see http://tools.ietf.org/html/rfc6749 for required data per flow
    void validateConfigId( bool needsId = false );

    //! Load a string (e.g. JSON) of a config
    bool loadConfigTxt( const QByteArray &configtxt, ConfigFormat format = JSON );

    //! Save a config to a string (e.g. JSON)
    QByteArray saveConfigTxt( ConfigFormat format = JSON, bool pretty = false, bool *ok = nullptr ) const;

    //! Configuration as a QVariant map
    QVariantMap mappedProperties() const;

    /**
     * Serialize the configuration \a variant according to \a format
     * \param variant map where configuration is stored
     * \param format output format
     * \param pretty indentation in output
     * \param ok is set to FALSE in case something goes wrong, TRUE otherwise
     * \return serialized config
     */
    static QByteArray serializeFromVariant( const QVariantMap &variant,
                                            ConfigFormat format = JSON,
                                            bool pretty = false,
                                            bool *ok = nullptr );

    /**
     * Unserialize the configuration in \a serial according to \a format
     * \param serial serialized configuration
     * \param format output format
     * \param ok is set to FALSE in case something goes wrong, TRUE otherwise
     * \return config map
     */
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

    /**
     * Returns an ordered list of locations from which stored configuration files
     * will be loaded. The list is in ascending order of precedence, so configuration
     * files from later items will override those from earlier locations.
     */
    static QStringList configLocations( const QString &extradir = QString() );

    //! Load and parse standard directories of configs (e.g. JSON) to a mapped cache
    static QgsStringMap mappedOAuth2ConfigsCache( QObject *parent, const QString &extradir = QString() );

    //! Path where config is stored
    static QString oauth2ConfigsPkgDataDir();

    //! Path where user settings are stored
    static QString oauth2ConfigsUserSettingsDir();

    //! User readable name of the \a configtype
    static QString configTypeString( ConfigType configtype );

    //! User readable name of the grant \a flow
    static QString grantFlowString( GrantFlow flow );

    //! User readable name of the access \a method
    static QString accessMethodString( AccessMethod method );

    //! Path of the token cache \a temporary directory
    static QString tokenCacheDirectory( bool temporary = false );

    //! Path of the token cache file, with optional \a suffix
    static QString tokenCacheFile( const QString &suffix = QString() );

    //! Path of the token cache file, with optional \a suffix and \a temporary flag
    static QString tokenCachePath( const QString &suffix = QString(), bool temporary = false );

  public slots:
    //! Set the id to \a value
    void setId( const QString &value );
    //! Set version to \a value
    void setVersion( int value );
    //! Set config type to \a value
    void setConfigType( QgsAuthOAuth2Config::ConfigType value );
    //! Set grant flow to \a value
    void setGrantFlow( QgsAuthOAuth2Config::GrantFlow value );
    //! Set name to \a value
    void setName( const QString &value );
    //! Set description to \a value
    void setDescription( const QString &value );
    //! Set request url to \a value
    void setRequestUrl( const QString &value );
    //! Set token url to \a value
    void setTokenUrl( const QString &value );
    //! Set refresh token url to \a value
    void setRefreshTokenUrl( const QString &value );
    //! Set redirect url to \a value
    void setRedirectUrl( const QString &value );
    //! Set redirect port to \a value
    void setRedirectPort( int value );
    //! Set client id to \a value
    void setClientId( const QString &value );
    //! Set client secret to \a value
    void setClientSecret( const QString &value );
    //! Set username to \a value
    void setUsername( const QString &value );
    //! Set password to \a value
    void setPassword( const QString &value );
    //! Set scope to \a value
    void setScope( const QString &value );
    //! Set api key to \a value
    void setApiKey( const QString &value );
    // advanced
    //! Set persistent token flag to \a persist
    void setPersistToken( bool persist );
    //! Set access method to \a value
    void setAccessMethod( QgsAuthOAuth2Config::AccessMethod value );
    //! Set request timeout to \a value
    void setRequestTimeout( int value );
    //! Set query pairs to \a pairs
    void setQueryPairs( const QVariantMap &pairs );
    //! Reset configuration to defaults
    void setToDefaults();
    //! Validate configuration
    void validateConfig();

  signals:
    //! Emitted when configuration has changed
    void configChanged();
    //! Emitted when configuration id has changed
    void idChanged( const QString & );
    //! Emitted when configuration version has changed
    void versionChanged( int );
    //! Emitted when configuration type has changed
    void configTypeChanged( QgsAuthOAuth2Config::ConfigType );
    //! Emitted when configuration grant flow has changed
    void grantFlowChanged( QgsAuthOAuth2Config::GrantFlow );
    //! Emitted when configuration grant flow has changed
    void nameChanged( const QString & );
    //! Emitted when configuration name has changed
    void descriptionChanged( const QString & );
    //! Emitted when configuration request urlhas changed
    void requestUrlChanged( const QString & );
    //! Emitted when configuration token url has changed
    void tokenUrlChanged( const QString & );
    //! Emitted when configuration refresh token url has changed
    void refreshTokenUrlChanged( const QString & );
    //! Emitted when configuration redirect url has changed
    void redirectUrlChanged( const QString & );
    //! Emitted when configuration redirect port has changed
    void redirectPortChanged( int );
    //! Emitted when configuration client id has changed
    void clientIdChanged( const QString & );
    //! Emitted when configuration client secret has changed
    void clientSecretChanged( const QString & );
    //! Emitted when configuration username has changed
    void usernameChanged( const QString & );
    //! Emitted when configuration password has changed
    void passwordChanged( const QString & );
    //! Emitted when configuration scope has changed
    void scopeChanged( const QString & );
    //! Emitted when configuration API key has changed
    void apiKeyChanged( const QString & );

    // advanced
    //! Emitted when configuration persist token flag has changed
    void persistTokenChanged( bool );
    //! Emitted when configuration access method has changed
    void accessMethodChanged( QgsAuthOAuth2Config::AccessMethod );
    //! Emitted when configuration request timeout has changed
    void requestTimeoutChanged( int );
    //! Emitted when configuration query pair has changed
    void queryPairsChanged( const QVariantMap & );
    //! Emitted when configuration validity has changed
    void validityChanged( bool );

  private:
    QString mId;
    int mVersion = 1;
    ConfigType mConfigType = ConfigType::Custom;
    GrantFlow mGrantFlow = GrantFlow::AuthCode;
    QString mName;
    QString mDescription;
    QString mRequestUrl;
    QString mTokenUrl;
    QString mRefreshTokenUrl;
    QString mRedirectURL;
    int mRedirectPort = 7070;
    QString mClientId;
    QString mClientSecret;
    QString mUsername;
    QString mPassword;
    QString mScope;
    QString mApiKey;
    bool mPersistToken = false;
    AccessMethod mAccessMethod = AccessMethod::Header;
    int mRequestTimeout = 30 ; // in seconds
    QVariantMap mQueryPairs;
    bool mValid = false;
};

#endif // QGSAUTHOAUTH2CONFIG_H
