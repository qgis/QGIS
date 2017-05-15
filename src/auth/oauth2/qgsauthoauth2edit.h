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
#include "qgsauthmethodedit.h"
#include "ui_qgsauthoauth2edit.h"

#include "qgsauthconfig.h"
#include "qgsauthoauth2config.h"


class QgsAuthOAuth2Edit : public QgsAuthMethodEdit, private Ui::QgsAuthOAuth2Edit
{
    Q_OBJECT

  public:
    explicit QgsAuthOAuth2Edit( QWidget *parent = nullptr );
    virtual ~QgsAuthOAuth2Edit();

    bool validateConfig() override;

    QgsStringMap configMap() const override;

  public slots:
    void loadConfig( const QgsStringMap &configmap ) override;

    void resetConfig() override;

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

    void loadFromOAuthConfig( const QgsAuthOAuth2Config *config = nullptr );

    void updateTokenCacheFile( bool curpersist ) const;

    void tabIndexChanged( int indx );

    void definedCustomDirChanged( const QString &path );

    void getDefinedCustomDir();

  private:
    void initGui();

    QWidget *parentWidget() const;
    QLineEdit *parentNameField() const;
    QString parentConfigId() const;

    void initConfigObjs();
    void deleteConfigObjs();

    bool hasTokenCacheFile();

    void addQueryPairRow( const QString &key, const QString &val );
    QVariantMap queryPairs() const;

    int customTab() const { return 0; }
    int definedTab() const { return 1; }
    bool onCustomTab() const;
    bool onDefinedTab() const;

    QString currentDefinedConfig() const { return mDefinedId; }

    QgsAuthOAuth2Config *mOAuthConfigCustom;
    QgsStringMap mDefinedConfigsCache;
    QString mDefinedId;
    QLineEdit *mParentName;
    QgsStringMap mConfigMap;
    bool mValid;
    int mCurTab;
    bool mPrevPersistToken;
    QToolButton *btnTokenClear;
};

#endif // QGSAUTHOAUTH2EDIT_H
