/***************************************************************************
    begin                : August 1, 2016
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

#ifndef QGSO2_H
#define QGSO2_H

#include "o2.h"

class QgsAuthOAuth2Config;

/**
 * QGIS-specific subclass of O2 lib's base OAuth 2.0 authenticator.
 * Adds support for QGIS authentication system.
 * \ingroup auth_plugins
 * \since QGIS 3.4
 */
class QgsO2 : public O2
{
    Q_OBJECT

  public:
    /**
     * Construct QgsO2
     * \param authcfg authentication configuration id
     * \param oauth2config OAuth2 configuration. Will be reparented to this object.
     * \param parent
     * \param manager QGIS network access manager instance
     */
    explicit QgsO2( const QString &authcfg, QgsAuthOAuth2Config *oauth2config = nullptr, QObject *parent = nullptr, QNetworkAccessManager *manager = nullptr );

    ~QgsO2() override;

    //! Authentication configuration id
    QString authcfg() const { return mAuthcfg; }

    //! OAuth2 configuration
    QgsAuthOAuth2Config *oauth2config() { return mOAuth2Config; }

    Q_PROPERTY( QString state READ state WRITE setState NOTIFY stateChanged )

    //! Retrieve oauth2 state
    QString state() const { return state_; }

    //! Store oauth2 state to a random value when called
    void setState( const QString &value );

    //! Refresh token in a synchronous way
    void refreshSynchronous();

    /**
     * Compute expiration delay from current timestamp and expires()
     * Should only be called just after a refresh / link event.
    */
    void computeExpirationDelay();

    /**
     * Returns expiration delay.
     * May be 0 if it is unknown
     */
    int expirationDelay() const { return mExpirationDelay; }

  public slots:

    //! Clear all properties
    void clearProperties();

    //! Triggered when auth code was set
    void onSetAuthCode( const QString &code );

  protected slots:

    //! Handle verification response.
    void onVerificationReceived( QMap<QString, QString> response ) override;

  protected:
    QNetworkAccessManager *getManager() override;

  signals:

    //! Emitted when the state has changed
    void stateChanged();

    //! Emitted when auth code needs to be retrieved (when not localhost)
    void getAuthCode();

  private:
    // block from calling externally -- this may be dangerous, we want to prevent
    // anyone from calling this from a different thread
    // Use instead QgsOAuth2Factory::requestLink
    void link() override;

    void initOAuthConfig();

    void setSettingsStore( bool persist = false );

    void setVerificationResponseContent();

    bool isLocalHost( const QUrl redirectUrl ) const;

    QString mTokenCacheFile;
    QString mAuthcfg;
    // Follow O2 style for this variable only:
    QString state_;
    QgsAuthOAuth2Config *mOAuth2Config;
    bool mIsLocalHost = false;
    int mExpirationDelay = 0;

    static QString O2_OAUTH2_STATE;
    friend class QgsOAuth2Factory;
};

#endif // QGSO2_H
