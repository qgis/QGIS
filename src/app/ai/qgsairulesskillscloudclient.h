/***************************************************************************
    qgsairulesskillscloudclient.h
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

#ifndef QGSAIRULESSKILLSCLOUDCLIENT_H
#define QGSAIRULESSKILLSCLOUDCLIENT_H

#include "qgis_app.h"
#include "qgsairulesskillsstore.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

/**
 * Optional, opt-in sync of local Cursor-style rules/skills to Strata Cloud
 * (`/v1/workspaces/:id/rules` and `/v1/workspaces/:id/skills`), so a team can
 * share a canonical set across desktop installs. Purely additive: the
 * desktop store (QgsAiRulesSkillsStore) remains the source of truth used to
 * build the system prompt; this client only pushes/pulls the same content to
 * the backend's `AgentRule`/`AgentSkill` tables.
 *
 * All requests are asynchronous; results are reported via signals. Every
 * method requires an already-authenticated Plan Account \a sessionToken.
 */
class APP_EXPORT QgsAiRulesSkillsCloudClient : public QObject
{
    Q_OBJECT

  public:
    struct RemoteRule
    {
        //! Empty for a rule not yet pushed to the cloud.
        QString id;
        QString slug;
        QString name;
        QString description;
        QStringList globs;
        bool alwaysApply = true;
        bool enabled = true;
        QString content;
    };

    struct RemoteSkill
    {
        //! Empty for a skill not yet pushed to the cloud.
        QString id;
        QString slug;
        QString name;
        QString description;
        bool enabled = true;
        QString content;
    };

    explicit QgsAiRulesSkillsCloudClient( QObject *parent = nullptr );

    //! Builds a RemoteRule payload from a local rule's metadata and body, ready for pushRule().
    static RemoteRule toRemoteRule( const QgsAiRuleInfo &info, const QString &content );
    //! Builds a RemoteSkill payload from a local skill's metadata and body, ready for pushSkill().
    static RemoteSkill toRemoteSkill( const QgsAiSkillInfo &info, const QString &content );

    /**
     * Upserts the desktop workspace (identified by \a workspaceRoot's fingerprint,
     * see QgsAiCloudIndexClient::workspaceFingerprint()) and reports its cloud id
     * via workspaceReady(), so callers can then fetch/push rules and skills.
     */
    void ensureWorkspace( const QString &apiBase, const QString &sessionToken, const QString &workspaceRoot, const QString &workspaceName );

    //! Fetches every rule stored in the cloud for \a workspaceId. Emits rulesFetched() or requestFailed().
    void fetchRules( const QString &apiBase, const QString &sessionToken, const QString &workspaceId );
    //! Fetches every skill stored in the cloud for \a workspaceId. Emits skillsFetched() or requestFailed().
    void fetchSkills( const QString &apiBase, const QString &sessionToken, const QString &workspaceId );

    //! Creates (\a rule.id empty) or updates (\a rule.id set) a cloud rule. Emits ruleSynced() or requestFailed().
    void pushRule( const QString &apiBase, const QString &sessionToken, const QString &workspaceId, const RemoteRule &rule );
    //! Creates (\a skill.id empty) or updates (\a skill.id set) a cloud skill. Emits skillSynced() or requestFailed().
    void pushSkill( const QString &apiBase, const QString &sessionToken, const QString &workspaceId, const RemoteSkill &skill );

    //! Deletes a cloud rule. Emits ruleDeleted() or requestFailed().
    void deleteRuleRemote( const QString &apiBase, const QString &sessionToken, const QString &workspaceId, const QString &remoteRuleId );
    //! Deletes a cloud skill. Emits skillDeleted() or requestFailed().
    void deleteSkillRemote( const QString &apiBase, const QString &sessionToken, const QString &workspaceId, const QString &remoteSkillId );

  signals:
    void workspaceReady( const QString &workspaceId );
    void rulesFetched( const QList<QgsAiRulesSkillsCloudClient::RemoteRule> &rules );
    void skillsFetched( const QList<QgsAiRulesSkillsCloudClient::RemoteSkill> &skills );
    void ruleSynced( const QgsAiRulesSkillsCloudClient::RemoteRule &rule );
    void skillSynced( const QgsAiRulesSkillsCloudClient::RemoteSkill &skill );
    void ruleDeleted( const QString &remoteRuleId );
    void skillDeleted( const QString &remoteSkillId );
    void requestFailed( const QString &message );
};

Q_DECLARE_METATYPE( QgsAiRulesSkillsCloudClient::RemoteRule )
Q_DECLARE_METATYPE( QgsAiRulesSkillsCloudClient::RemoteSkill )

#endif // QGSAIRULESSKILLSCLOUDCLIENT_H
