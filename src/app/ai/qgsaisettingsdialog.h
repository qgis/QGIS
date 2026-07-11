/***************************************************************************
    qgsaisettingsdialog.h
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

#ifndef QGSAISETTINGSDIALOG_H
#define QGSAISETTINGSDIALOG_H

#include "qgis_app.h"
#include "qgsaicodexoauthclient.h"
#include "qgsairulesskillsstore.h"

#include <QDialog>
#include <QPointer>

class QgsAiAccountWidget;
class QgsAiAgentSessionManager;
class QgsAiLayerIndexCoordinator;
class QgsAiModelRouter;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpinBox;
class QStackedWidget;
class QTextEdit;
class QVBoxLayout;

/**
 * AI settings dialog: a Cursor-style sidebar (account header + section list)
 * with stacked pages. The Account page comes first and applies immediately;
 * everything else is collected and persisted on accept, exactly like the
 * legacy single-form dialog it replaces.
 */
class APP_EXPORT QgsAiSettingsDialog : public QDialog
{
    Q_OBJECT

  public:
    QgsAiSettingsDialog( QgsAiAgentSessionManager *sessionManager, QgsAiModelRouter *modelRouter, QgsAiLayerIndexCoordinator *layerIndexCoordinator, QWidget *parent = nullptr );

    void accept() override;

  public slots:
    //! Selects a sidebar section: account, providers, agent, rules, indexing, workspace, privacy or onboarding.
    void showSection( const QString &key );

  signals:
    //! The embedding provider configuration changed (provider selection or model download).
    void embeddingProviderSettingsChanged();
    //! Plan auth or managed catalog changed while the dialog is open; the dock should rebuild its model menu.
    void planAuthStateChanged();
    //! The demo project was (re)created; the dock should refresh its GIS suggestion card.
    void demoProjectCreated();

  protected:
    bool eventFilter( QObject *watched, QEvent *event ) override;

  private:
    QWidget *createPage( const QString &title, const QString &subtitle, QVBoxLayout *&contentLayout );
    void addSection( const QString &key, const QString &label, QWidget *page );
    QWidget *buildAccountPage();
    QWidget *buildProvidersPage();
    QWidget *buildAgentPage();
    QWidget *buildRulesSkillsPage();
    QWidget *buildIndexingPage();
    QWidget *buildWorkspacePage();
    QWidget *buildPrivacyPage();
    QWidget *buildOnboardingPage();

    void applySettings();
    void refreshSidebarAccountHeader();
    void refreshTrustWorkspace();

    //! Wraps mSessionManager's file context provider; cheap value type, safe to build on demand.
    QgsAiRulesSkillsStore rulesSkillsStore() const;
    bool rulesSkillsWritable() const;
    void refreshRulesSkillsTrustState();

    void refreshRulesList();
    void selectRuleInEditor( const QgsAiRuleInfo &rule );
    void clearRuleEditor();
    void newRule();
    void duplicateSelectedRule();
    void deleteSelectedRule();
    void saveCurrentRule();

    void refreshSkillsList();
    void selectSkillInEditor( const QgsAiSkillInfo &skill );
    void clearSkillEditor();
    void clearSkillPropertyRows();
    void addSkillPropertyRow( const QString &key, const QStringList &values = QStringList(), bool isList = false, bool required = false );
    QgsAiMarkdownDocument skillDocumentFromEditor() const;
    void setSkillDocumentInEditor( const QgsAiMarkdownDocument &document );
    void newSkill();
    void duplicateSelectedSkill();
    void deleteSelectedSkill();
    void saveCurrentSkill();

    //! Pushes every enabled local rule/skill to Strata Cloud (opt-in, one-way local -> cloud).
    void syncRulesSkillsToCloud();
    void refreshEmbeddingStatusLabel();
    void refreshRemoteEmbeddingModelField();
    void refreshIndexStatusLabel();
    void refreshCloudIndexStatusLabel();
    void refreshOnboardingStatus();
    QString onboardingStatusText() const;
    bool ensureEmbeddingProvider();
    bool confirmRebuild( const QString &what );

    QPointer<QgsAiAgentSessionManager> mSessionManager;
    QPointer<QgsAiModelRouter> mModelRouter;
    QPointer<QgsAiLayerIndexCoordinator> mLayerIndexCoordinator;

    QListWidget *mSidebarList = nullptr;
    QStackedWidget *mStack = nullptr;
    QWidget *mSidebarHeader = nullptr;
    QLabel *mSidebarAvatar = nullptr;
    QLabel *mSidebarEmailLabel = nullptr;

    QgsAiAccountWidget *mAccountWidget = nullptr;

    QLineEdit *mOpenAiEndpoint = nullptr;
    QLineEdit *mOpenAiModel = nullptr;
    QLineEdit *mOpenAiKey = nullptr;
    QLineEdit *mOpenRouterEndpoint = nullptr;
    QComboBox *mOpenRouterModel = nullptr;
    QLineEdit *mOpenRouterKey = nullptr;
    QCheckBox *mOpenRouterAutoRouting = nullptr;
    QLineEdit *mCodexEndpoint = nullptr;
    QLineEdit *mCodexModel = nullptr;
    QLabel *mCodexStatus = nullptr;
    QgsAiCodexOAuthClient::DeviceCode mCodexDeviceCode;
    QLineEdit *mClaudeEndpoint = nullptr;
    QLineEdit *mClaudeModel = nullptr;
    QLineEdit *mClaudeKey = nullptr;
    QLineEdit *mClaudeSubscriptionToken = nullptr;
    QCheckBox *mClaudeUseOAuth = nullptr;
    QLabel *mClaudeOAuthStatus = nullptr;

    QCheckBox *mAllowCustomActions = nullptr;
    QSpinBox *mMaxToolIterationsPerTurn = nullptr;
    QCheckBox *mGisSuggestionsEnabled = nullptr;
    QCheckBox *mGisSuggestionsProjectEnabled = nullptr;

    QLabel *mRulesSkillsTrustBanner = nullptr;
    QString mRulesRelativeDirForList;
    QString mSkillsRelativeDirForList;

    QListWidget *mRulesListWidget = nullptr;
    QWidget *mRuleEditorWidget = nullptr;
    QTextEdit *mRuleBodyEdit = nullptr;
    QPushButton *mRuleNewButton = nullptr;
    QPushButton *mRuleDuplicateButton = nullptr;
    QPushButton *mRuleDeleteButton = nullptr;
    QPushButton *mRuleSaveButton = nullptr;
    QString mCurrentRuleSlug;
    QString mCurrentRulePath;

    QListWidget *mSkillsListWidget = nullptr;
    QWidget *mSkillEditorWidget = nullptr;
    QVBoxLayout *mSkillPropertiesLayout = nullptr;
    QPushButton *mSkillAddPropertyButton = nullptr;
    QTextEdit *mSkillBodyEdit = nullptr;
    QPushButton *mSkillNewButton = nullptr;
    QPushButton *mSkillDuplicateButton = nullptr;
    QPushButton *mSkillDeleteButton = nullptr;
    QPushButton *mSkillSaveButton = nullptr;
    QString mCurrentSkillSlug;

    struct SkillPropertyRow
    {
        QWidget *rowWidget = nullptr;
        QLineEdit *keyEdit = nullptr;
        QTextEdit *valueEdit = nullptr;
        QPushButton *removeButton = nullptr;
        bool isList = false;
        bool required = false;
    };
    QList<SkillPropertyRow> mSkillPropertyRows;

    QPushButton *mSyncRulesSkillsCloudButton = nullptr;
    QLabel *mRulesSkillsCloudStatusLabel = nullptr;

    QComboBox *mEmbeddingProvider = nullptr;
    QLineEdit *mRemoteEmbeddingModel = nullptr;
    QWidget *mRemoteEmbeddingModelRow = nullptr;
    QLabel *mEmbeddingStatusLabel = nullptr;
    QPushButton *mDownloadEmbeddingModelButton = nullptr;
    QCheckBox *mAutomaticIndexing = nullptr;
    QCheckBox *mEnableLayerIndexing = nullptr;
    QCheckBox *mCloudContextOptIn = nullptr;
    QLabel *mIndexStatusLabel = nullptr;
    QLabel *mCloudIndexStatusLabel = nullptr;
    QPushButton *mSyncCloudContextButton = nullptr;
    QPushButton *mRebuildWorkspaceIndexButton = nullptr;
    QPushButton *mRebuildLayerIndexButton = nullptr;

    QLineEdit *mWorkspaceRoot = nullptr;
    QCheckBox *mTrustWorkspace = nullptr;
    QString mTrustRootForCheckbox;

    QCheckBox *mPrivacyMetadataOnly = nullptr;
    QCheckBox *mTelemetryOptIn = nullptr;
    QCheckBox *mCrashReportOptIn = nullptr;

    QLabel *mOnboardingStatusLabel = nullptr;
    QLabel *mReleaseDryRunStatus = nullptr;
};

#endif // QGSAISETTINGSDIALOG_H
