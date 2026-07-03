/***************************************************************************
    qgsaiplanclient.h
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

#ifndef QGSAIPLANCLIENT_H
#define QGSAIPLANCLIENT_H

#include "qgis_app.h"
#include "qgsaiagentpolicy.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

/**
 * Small Strata Cloud Plan API client used by the desktop settings and model picker.
 *
 * The chat endpoint stays `/ai/messages`; this client derives the API origin from
 * that configured endpoint and calls auth, account, and model catalog routes.
 */
class APP_EXPORT QgsAiPlanClient : public QObject
{
    Q_OBJECT

  public:
    struct AccountInfo
    {
        QString id;
        QString email;
        QString tier;
    };

    struct ModelInfo
    {
        QString id;
        QString label;
        QString provider;
        int contextWindow = 0;
        int inputCredits = 0;
        int outputCredits = 0;
        QStringList capabilities;
        QStringList tierAvailability;

        QString displayLabel() const;
        QString tooltip() const;
    };

    struct BalanceInfo
    {
        int available = 0;
        int held = 0;
        QString tier;
        //! Monthly credit quota for the current tier; 0 means unlimited (e.g. Enterprise).
        int monthlyCredits = 0;

        bool isUnlimited() const { return monthlyCredits <= 0; }
        //! Percentage of the monthly quota already consumed, clamped to [0, 100]. 0 when unlimited.
        int usedPercent() const;
    };

    struct ModelPreferenceInfo
    {
        QString modelId;
        bool enabled = true;
    };

    explicit QgsAiPlanClient( QObject *parent = nullptr );

    static QString apiBaseForChatEndpoint( const QString &chatEndpoint );
    static QList<ModelInfo> parseModelsJson( const QByteArray &body );
    static QList<QgsAiManagedAgentPreset> parseAgentsJson( const QByteArray &body );
    static QgsAiManagedAgentPolicy parseAgentPolicyJson( const QByteArray &body );
    static AccountInfo parseMeJson( const QByteArray &body );
    static BalanceInfo parseBalanceJson( const QByteArray &body );
    static QList<ModelPreferenceInfo> parseModelPreferencesJson( const QByteArray &body );
    static QString cacheFilePath();
    static QString agentsCacheFilePath();
    static QString agentPolicyCacheFilePath();
    static QString modelPreferencesCacheFilePath();
    static QList<ModelInfo> cachedModels();
    static QList<QgsAiManagedAgentPreset> cachedAgents();
    static QgsAiManagedAgentPolicy cachedAgentPolicy();
    static QList<ModelPreferenceInfo> cachedModelPreferences();
    static void writeCachedModels( const QList<ModelInfo> &models );
    static void writeCachedAgents( const QList<QgsAiManagedAgentPreset> &agents );
    static void writeCachedAgentPolicy( const QgsAiManagedAgentPolicy &policy );
    static void writeCachedModelPreferences( const QList<ModelPreferenceInfo> &preferences );
    //! Convenience lookup used by the chat dock to filter out models the user disabled.
    static bool isModelDisabled( const QString &modelId );

    void login( const QString &chatEndpoint, const QString &email, const QString &password );
    void registerAccount( const QString &chatEndpoint, const QString &email, const QString &password );
    void fetchMe( const QString &chatEndpoint, const QString &sessionToken );
    void fetchBalance( const QString &chatEndpoint, const QString &sessionToken );
    void refreshModels( const QString &chatEndpoint );
    void refreshAgents( const QString &chatEndpoint, const QString &sessionToken );
    void refreshAgentPolicy( const QString &chatEndpoint, const QString &sessionToken );
    void fetchModelPreferences( const QString &chatEndpoint, const QString &sessionToken );
    void setModelPreference( const QString &chatEndpoint, const QString &sessionToken, const QString &modelId, bool enabled );

  signals:
    void desktopTokenReady( const QString &token );
    void accountReady( const QgsAiPlanClient::AccountInfo &account );
    void balanceReady( const QgsAiPlanClient::BalanceInfo &balance );
    void modelsReady( const QList<QgsAiPlanClient::ModelInfo> &models, bool fromCache );
    void agentsReady( const QList<QgsAiManagedAgentPreset> &agents, bool fromCache );
    void agentPolicyReady( const QgsAiManagedAgentPolicy &policy, bool fromCache );
    void modelPreferencesReady( const QList<QgsAiPlanClient::ModelPreferenceInfo> &preferences, bool fromCache );
    void modelPreferenceUpdated( const QString &modelId, bool enabled );
    //! Failure of a single-preference update; carries the requested state so the UI can revert exactly that toggle.
    void modelPreferenceUpdateFailed( const QString &modelId, bool requestedEnabled, const QString &message );
    void requestFailed( const QString &message );

  private:
    void authenticate( const QString &chatEndpoint, const QString &email, const QString &password, bool createAccount );
    void requestDesktopToken( const QString &apiBase, const QString &accessToken );
    void refreshAuthenticatedJson( const QString &chatEndpoint, const QString &sessionToken, const QString &path );
};

Q_DECLARE_METATYPE( QgsAiPlanClient::AccountInfo )
Q_DECLARE_METATYPE( QgsAiPlanClient::ModelInfo )
Q_DECLARE_METATYPE( QList<QgsAiPlanClient::ModelInfo> )
Q_DECLARE_METATYPE( QgsAiPlanClient::BalanceInfo )
Q_DECLARE_METATYPE( QgsAiPlanClient::ModelPreferenceInfo )
Q_DECLARE_METATYPE( QList<QgsAiPlanClient::ModelPreferenceInfo> )

#endif // QGSAIPLANCLIENT_H
