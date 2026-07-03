/***************************************************************************
    qgsaiaccountwidget.h
    ---------------------
    begin                : July 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAIACCOUNTWIDGET_H
#define QGSAIACCOUNTWIDGET_H

#include "qgis_app.h"
#include "qgsaiplanclient.h"

#include <QPointer>
#include <QWidget>

class QgsAiAgentSessionManager;
class QgsAiModelRouter;
class QgsCollapsibleGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QProgressBar;
class QPushButton;
class QStackedWidget;

/**
 * Auth-first Plan Account page: a Log in / Sign up toggle with just email and
 * password, an account card once signed in, and the technical fields (backend
 * endpoint, authcfg ID, manual session token) tucked into a collapsed
 * Advanced group. Login, registration and logout apply immediately; the
 * Advanced fields are persisted by the host dialog on accept.
 */
class APP_EXPORT QgsAiAccountWidget : public QWidget
{
    Q_OBJECT

  public:
    QgsAiAccountWidget( QgsAiModelRouter *modelRouter, QgsAiAgentSessionManager *sessionManager, QWidget *parent = nullptr );

    //! Current Plan chat endpoint text from the Advanced group (live, not yet persisted).
    QString planEndpoint() const;
    //! Current QGIS auth config ID text from the Advanced group.
    QString planAuthConfigId() const;
    //! Manually pasted session token from the Advanced group, applied by the host dialog on accept.
    QString manualSessionToken() const;
    //! Email of the signed-in account, empty while logged out or until /me answers.
    QString accountEmail() const;
    //! Plan tier of the signed-in account, empty until /me answers.
    QString accountTier() const;
    bool isSignedIn() const;

  signals:
    //! Login/logout or managed catalog changes the chat dock should react to immediately.
    void authStateChanged();
    //! Account card data changed (a sidebar header can mirror it).
    void accountInfoChanged();
    //! A model enable/disable preference was toggled; the chat dock model menu should rebuild.
    void modelPreferencesChanged();

  private slots:
    void startLogin();
    void startRegister();
    void logout();
    void refreshManagedModels();
    void onDesktopTokenReady( const QString &token );
    void onRequestFailed( const QString &message );
    void onBalanceReady( const QgsAiPlanClient::BalanceInfo &balance );
    void onModelPreferencesReady( const QList<QgsAiPlanClient::ModelPreferenceInfo> &preferences, bool fromCache );
    void onModelPreferenceUpdated( const QString &modelId, bool enabled );
    void onModelListItemChanged( QListWidgetItem *item );

  private:
    enum class Mode
    {
      Login,
      Signup
    };

    QWidget *buildLoggedOutPane();
    QWidget *buildLoggedInPane();
    void setMode( Mode mode );
    void setBusy( bool busy );
    void setStatus( const QString &text, bool error = false );
    void updateFormState();
    void updateAccountCard();
    void updateUsageCard();
    void populateModelList();
    QString currentEndpoint() const;
    bool endpointUsable() const;
    QString friendlyErrorMessage( const QString &message );

    QPointer<QgsAiModelRouter> mModelRouter;
    QPointer<QgsAiAgentSessionManager> mSessionManager;
    QgsAiPlanClient *mPlanClient = nullptr;

    QStackedWidget *mStateStack = nullptr;
    QPushButton *mModeLoginButton = nullptr;
    QPushButton *mModeSignupButton = nullptr;
    QLabel *mEndpointWarning = nullptr;
    QLineEdit *mEmail = nullptr;
    QLineEdit *mPassword = nullptr;
    QLabel *mPasswordHint = nullptr;
    QPushButton *mLoginButton = nullptr;
    QPushButton *mRegisterButton = nullptr;
    QLabel *mStatusLabel = nullptr;

    QLabel *mAvatarLabel = nullptr;
    QLabel *mEmailLabel = nullptr;
    QLabel *mTierLabel = nullptr;
    QPushButton *mLogoutButton = nullptr;
    QPushButton *mRefreshModelsButton = nullptr;

    QLabel *mUsageLabel = nullptr;
    QProgressBar *mUsageBar = nullptr;

    QListWidget *mModelListWidget = nullptr;
    bool mUpdatingModelList = false;
    //! Item awaiting a setModelPreference() response, so a failure can revert its checkbox.
    //! Cleared whenever the list is repopulated to avoid dangling access.
    QListWidgetItem *mPendingToggleItem = nullptr;
    bool mPendingToggleEnabled = false;

    QgsCollapsibleGroupBox *mAdvancedGroup = nullptr;
    QLineEdit *mEndpointEdit = nullptr;
    QLineEdit *mAuthCfgEdit = nullptr;
    QLineEdit *mTokenEdit = nullptr;

    Mode mMode = Mode::Login;
    bool mBusy = false;
    QString mAccountEmail;
    QString mAccountTier;
    QgsAiPlanClient::BalanceInfo mBalance;
    QList<QgsAiPlanClient::ModelPreferenceInfo> mModelPreferences;
};

#endif // QGSAIACCOUNTWIDGET_H
