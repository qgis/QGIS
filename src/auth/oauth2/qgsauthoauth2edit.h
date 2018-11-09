/***************************************************************************
    begin                : July 13, 2016
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

#ifndef QGSAUTHOAUTH2EDIT_H
#define QGSAUTHOAUTH2EDIT_H

#include <QWidget>
#include <QNetworkReply>
#include "qgsauthmethodedit.h"
#include "ui_qgsauthoauth2edit.h"

#include "qgsauthconfig.h"
#include "qgsauthoauth2config.h"


/**
 * The QgsAuthOAuth2Edit class allows editing of an OAuth2 authentication configuration
 * \ingroup auth_plugins
 * \since QGIS 3.4
 */
class QgsAuthOAuth2Edit : public QgsAuthMethodEdit, private Ui::QgsAuthOAuth2Edit
{
    Q_OBJECT

  public:

    //! Construct a QgsAuthOAuth2Edit instance
    explicit QgsAuthOAuth2Edit( QWidget *parent = nullptr );

    /**
     * Validate current configuration
     * \return true if current configuration is valid
     */
    bool validateConfig() override;

    /**
     * Current configuration
     * \return current configuration map
     */
    QgsStringMap configMap() const override;


  public slots:

    //! Load the configuration from \a configMap
    void loadConfig( const QgsStringMap &configmap ) override;

    //! Reset configuration to defaults
    void resetConfig() override;

    //! Clear configuration
    void clearConfig() override;

  private slots:
    void setupConnections();

    void configValidityChanged();

    void removeTokenCacheFile();

    void populateGrantFlows();

    void updateGrantFlow( int indx );

    void exportOAuthConfig();

    void importOAuthConfig();

    void descriptionChanged();

    void populateAccessMethods();

    void updateConfigAccessMethod( int indx );

    void addQueryPair();

    void removeQueryPair();

    void clearQueryPairs();

    void populateQueryPairs( const QVariantMap &querypairs, bool append = false );

    void queryTableSelectionChanged();

    void updateConfigQueryPairs();

    void updateDefinedConfigsCache();

    void loadDefinedConfigs();

    void setCurrentDefinedConfig( const QString &id );

    void currentDefinedItemChanged( QListWidgetItem *cur, QListWidgetItem *prev );

    void selectCurrentDefinedConfig();

    void getSoftStatementDir();

    void updateTokenCacheFile( bool curpersist ) const;

    void tabIndexChanged( int indx );

    void definedCustomDirChanged( const QString &path );

    void getDefinedCustomDir();

    void loadFromOAuthConfig( const QgsAuthOAuth2Config *config );

    void softwareStatementJwtPathChanged( const QString &path );

    void configReplyFinished();

    void registerReplyFinished();

    void networkError( QNetworkReply::NetworkError error );

  private:

    void initGui();
    void parseSoftwareStatement( const QString &path );

    QWidget *parentWidget() const;
    QLineEdit *parentNameField() const;
    QString parentConfigId() const;

    void initConfigObjs();

    bool hasTokenCacheFile();

    void addQueryPairRow( const QString &key, const QString &val );
    QVariantMap queryPairs() const;

    int customTab() const { return 0; }
    int definedTab() const { return 1; }
    int statementTab() const { return 2; }
    bool onCustomTab() const;
    bool onDefinedTab() const;
    bool onStatementTab() const;
    void getSoftwareStatementConfig();

    QString currentDefinedConfig() const { return mDefinedId; }

    void updatePredefinedLocationsTooltip();

    std::unique_ptr<QgsAuthOAuth2Config> mOAuthConfigCustom;
    QgsStringMap mDefinedConfigsCache;
    QString mDefinedId;
    QLineEdit *mParentName = nullptr;
    QgsStringMap mConfigMap;
    bool mValid = false;
    int mCurTab = 0;
    bool mPrevPersistToken = false;
    QToolButton *btnTokenClear = nullptr;
    QString mRegistrationEndpoint;
    QMap<QString, QVariant> mSoftwareStatement;
    void registerSoftStatement( const QString &registrationUrl );
    bool mDownloading = false;
    friend class TestQgsAuthOAuth2Method;
};

#endif // QGSAUTHOAUTH2EDIT_H
