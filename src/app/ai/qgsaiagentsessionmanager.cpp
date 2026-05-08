/***************************************************************************
    qgsaiagentsessionmanager.cpp
    ---------------------
    begin                : April 2026
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

#include "qgsaiagentsessionmanager.h"

#include "qgsaifilecontextprovider.h"
#include "qgsaireviewpatchengine.h"
#include "qgsaitool.h"
#include "qgsaitoolregistry.h"
#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgsmaplayer.h"
#include "qgsmessagelog.h"
#include "qgspointxy.h"
#include "qgsproject.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgswkbtypes.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QString>
#include <QUuid>
#include <QVariantList>

#include "moc_qgsaiagentsessionmanager.cpp"

using namespace Qt::StringLiterals;

QgsAiAgentSessionManager::QgsAiAgentSessionManager( QgsAiModelRouter *router, QgsAiFileContextProvider *contextProvider, QgsAiReviewPatchEngine *reviewEngine, QObject *parent )
  : QObject( parent )
  , mRouter( router )
  , mContextProvider( contextProvider )
  , mReviewEngine( reviewEngine )
{
  loadPersistedBehaviorSettings();
  if ( mRouter )
    mRouter->setToolUseEnabled( mBehaviorSettings.allowCustomActions );

  if ( mRouter )
  {
    connect( mRouter, &QgsAiModelRouter::toolCallsRequested, this, &QgsAiAgentSessionManager::onToolCallsRequested );

    connect( mRouter, &QgsAiModelRouter::requestProgress, this, [this]( const QString &requestId, const QString &chunk ) {
      if ( requestId != mActiveRequestId )
        return;
      mStreamedText += chunk;
      emit responseChunkReceived( chunk );
    } );

    connect( mRouter, &QgsAiModelRouter::requestFinished, this, [this]( const QString &requestId, bool success, const QString &providerName, const QString &responseText, const QString &errorMessage, int httpStatus, int retryCount, bool retriable, qint64 latencyMs ) {
      if ( requestId != mActiveRequestId )
        return;

      Q_UNUSED( retryCount )
      Q_UNUSED( retriable )

      if ( success )
      {
        const QString finalText = !responseText.isEmpty() ? responseText : mStreamedText;
        const QgsAiChatMessage assistant = buildAssistantMessage( finalText );
        mHistory.push_back( assistant );
        emit messageAdded( assistant );
        emit requestStateChanged( u"completed"_s, u"%1 (%2 ms)"_s.arg( providerName ).arg( latencyMs ) );
        mActiveRequestId.clear();
        emit requestRunningChanged( false );
        return;
      }

      if ( !mPendingProviders.isEmpty() )
      {
        const QgsAiModelRouter::Provider fallbackProvider = mPendingProviders.takeFirst();
        emit requestStateChanged( u"retrying"_s, u"%1 failed, retrying with %2…"_s.arg( providerName, mRouter->providerDisplayName( fallbackProvider ) ) );
        startProviderAttempt( fallbackProvider );
        return;
      }

      const QString finalError = actionableError( providerName, errorMessage, httpStatus );
      const QgsAiChatMessage assistant = buildAssistantMessage( finalError );
      mHistory.push_back( assistant );
      emit messageAdded( assistant );
      emit requestStateChanged( u"failed"_s, finalError );
      mActiveRequestId.clear();
      emit requestRunningChanged( false );
    } );
  }
}

QStringList QgsAiAgentSessionManager::availableAgents() const
{
  return QStringList() << u"planner"_s << u"reviewer"_s << u"editor"_s;
}

void QgsAiAgentSessionManager::setActiveAgent( const QString &agentName )
{
  if ( availableAgents().contains( agentName ) )
    mActiveAgent = agentName;
}

void QgsAiAgentSessionManager::clearHistory()
{
  mHistory.clear();
}

void QgsAiAgentSessionManager::cancelActiveRequest()
{
  if ( mActiveRequestId.isEmpty() || !mRouter )
    return;

  mRouter->cancelRequest( mActiveRequestId );
  mActiveRequestId.clear();
  mPendingProviders.clear();
  emit requestStateChanged( u"cancelled"_s, u"Request cancelled by user."_s ); //#spellok
  emit requestRunningChanged( false );                                         //#spellok
}

QStringList QgsAiAgentSessionManager::projectFileCandidates( const QString &query, int maxResults ) const
{
  return mContextProvider ? mContextProvider->workspaceFileCandidates( query, maxResults ) : QStringList();
}

QString QgsAiAgentSessionManager::resolveProjectFile( const QString &filePath ) const
{
  return mContextProvider ? mContextProvider->resolveWorkspaceFile( filePath ) : QString();
}

QString QgsAiAgentSessionManager::workspaceRoot() const
{
  return mContextProvider ? mContextProvider->workspaceRoot() : QString();
}

QList<QgsAiModelRouter::Provider> QgsAiAgentSessionManager::providerFallbackOrder() const
{
  QList<QgsAiModelRouter::Provider> order;
  const QgsAiModelRouter::Provider preferred = mRouter ? mRouter->resolveProvider() : QgsAiModelRouter::Provider::OpenAi;
  order << preferred;
  for ( QgsAiModelRouter::Provider provider : { QgsAiModelRouter::Provider::Plan, QgsAiModelRouter::Provider::Codex, QgsAiModelRouter::Provider::OpenAi, QgsAiModelRouter::Provider::Claude } )
  {
    if ( !order.contains( provider ) )
      order << provider;
  }
  return order;
}

void QgsAiAgentSessionManager::startProviderAttempt( QgsAiModelRouter::Provider provider )
{
  if ( !mRouter )
    return;

  mActiveProvider = provider;
  const QList<QgsAiChatMessage> messages = buildOutgoingMessages();

  mStreamedText.clear();
  mActiveRequestId = mRouter->startChatRequest( provider, messages, true );
  QgsMessageLog::logMessage(
    u"Provider attempt: agent=%1 provider=%2 requestId=%3 historyMessages=%4 toolIteration=%5 pendingFallbacks=%6"_s.arg( mActiveAgent, mRouter->providerDisplayName( provider ), mActiveRequestId )
      .arg( messages.size() )
      .arg( mToolIterations )
      .arg( mPendingProviders.size() ),
    u"AI"_s,
    Qgis::MessageLevel::Info,
    false
  );
  emit requestStateChanged( u"sending"_s, u"Contacting %1…"_s.arg( mRouter->providerDisplayName( provider ) ) );
}

QgsAiChatMessage QgsAiAgentSessionManager::buildAssistantMessage( const QString &text ) const
{
  QgsAiChatMessage reply;
  reply.id = QUuid::createUuid().toString( QUuid::WithoutBraces );
  reply.role = QgsAiChatRole::Assistant;
  reply.content = text;
  reply.timestamp = QDateTime::currentDateTimeUtc();
  return reply;
}

QString QgsAiAgentSessionManager::buildContextSummary( const QList<QgsAiChatContextFile> &contextFiles, bool &contextBlocked ) const
{
  contextBlocked = false;
  if ( !mContextProvider )
    return QString();

  if ( contextFiles.isEmpty() )
    return u"No file context attached."_s;

  QStringList summaries;
  int index = 1;
  for ( const QgsAiChatContextFile &contextFile : contextFiles )
  {
    const QgsAiFileContext context = mContextProvider->buildContext( contextFile.filePath, contextFile.selectedText, 16384, contextFile.allowExternal );
    if ( !context.isValid() )
    {
      contextBlocked = true;
      return u"File context blocked: path is outside allowed workspace or unreadable."_s;
    }

    const QString root = mContextProvider->workspaceRoot();
    const QString relativePath = QDir( root ).relativeFilePath( context.filePath );
    const bool inWorkspace = !root.isEmpty() && ( relativePath == "."_L1 || ( !relativePath.startsWith( "../"_L1 ) && relativePath != ".."_L1 && !QDir::isAbsolutePath( relativePath ) ) );
    const QString displayPath = inWorkspace ? relativePath : context.filePath;

    QString summary;
    summary += u"Context file %1: %2\n"_s.arg( index++ ).arg( displayPath );
    summary += u"Size: %1 bytes%2\n"_s.arg( context.fileSize ).arg( context.truncated ? u" (snippet truncated)"_s : QString() );
    if ( !context.selectedText.isEmpty() )
      summary += u"Selected text:\n%1\n"_s.arg( context.selectedText.left( 8192 ) );
    if ( context.binary )
    {
      summary += "Content omitted because the file appears to be binary."_L1;
    }
    else if ( !context.fileSnippet.isEmpty() )
    {
      summary += u"File content snippet:\n%1"_s.arg( context.fileSnippet );
    }
    else
    {
      summary += "No text content could be read from this file."_L1;
    }
    summaries << summary.trimmed();
  }

  return summaries.join( "\n\n"_L1 );
}

QString QgsAiAgentSessionManager::actionableError( const QString &providerName, const QString &errorMessage, int httpStatus ) const
{
  const QString sanitized = mRouter ? mRouter->sanitizeErrorText( errorMessage ) : errorMessage;
  if ( httpStatus == 401 || httpStatus == 403 )
  {
    if ( providerName == "Plan Account"_L1 )
      return u"%1 authentication failed. Check session token or authcfg in Provider Settings."_s.arg( providerName );
    return u"%1 authentication failed. Check the API key or OAuth login in Provider Settings."_s.arg( providerName );
  }
  if ( httpStatus == 404 )
    return u"%1 endpoint not found. Verify provider endpoint in settings."_s.arg( providerName );
  if ( httpStatus == 429 )
    return u"%1 rate-limited the request. Retry later or switch provider."_s.arg( providerName );
  if ( httpStatus >= 500 )
    return u"%1 is temporarily unavailable (HTTP %2). Please retry shortly."_s.arg( providerName ).arg( httpStatus );
  if ( sanitized.isEmpty() )
    return u"%1 request failed. Check provider settings and connectivity."_s.arg( providerName );
  return u"%1 request failed: %2"_s.arg( providerName, sanitized );
}

bool QgsAiAgentSessionManager::tryBuildPatchProposal( const QString &text, QgsAiPatchProposal &proposal ) const
{
  static const QRegularExpression commandRe( u"^/patch\\s+path=(.+)$"_s );
  static const QRegularExpression bodyRe( u"<<<<\\n([\\s\\S]*)\\n====\\n([\\s\\S]*)\\n>>>>$"_s );

  const QStringList lines = text.split( '\n' );
  if ( lines.isEmpty() )
    return false;

  const QRegularExpressionMatch commandMatch = commandRe.match( lines.at( 0 ).trimmed() );
  if ( !commandMatch.hasMatch() )
    return false;

  const QString payload = lines.mid( 1 ).join( '\n' );
  const QRegularExpressionMatch bodyMatch = bodyRe.match( payload );
  if ( !bodyMatch.hasMatch() )
    return false;

  QgsAiPatchHunk hunk;
  hunk.filePath = commandMatch.captured( 1 ).trimmed();
  hunk.originalText = bodyMatch.captured( 1 );
  hunk.replacementText = bodyMatch.captured( 2 );

  proposal.id = QUuid::createUuid().toString( QUuid::WithoutBraces );
  proposal.title = u"Patch generated by %1 agent"_s.arg( mActiveAgent );
  proposal.hunks = QList<QgsAiPatchHunk>() << hunk;
  proposal.createdAt = QDateTime::currentDateTimeUtc();
  return true;
}

void QgsAiAgentSessionManager::sendUserMessage( const QString &text, const QString &filePath, const QString &selectedText )
{
  QList<QgsAiChatContextFile> contextFiles;
  if ( !filePath.isEmpty() || !selectedText.isEmpty() )
  {
    QgsAiChatContextFile contextFile;
    contextFile.filePath = filePath;
    contextFile.selectedText = selectedText;
    contextFiles << contextFile;
  }
  sendUserMessage( text, contextFiles );
}

void QgsAiAgentSessionManager::sendUserMessage( const QString &text, const QList<QgsAiChatContextFile> &contextFiles )
{
  if ( hasActiveRequest() )
  {
    const QgsAiChatMessage assistant = buildAssistantMessage( u"A request is already running. Please wait or cancel it first."_s );
    mHistory.push_back( assistant );
    emit messageAdded( assistant );
    return;
  }

  QgsAiChatMessage message;
  message.id = QUuid::createUuid().toString( QUuid::WithoutBraces );
  message.role = QgsAiChatRole::User;
  message.content = text;
  message.timestamp = QDateTime::currentDateTimeUtc();
  mHistory.push_back( message );
  emit messageAdded( message );

  QgsAiPatchProposal proposal;
  if ( mReviewEngine && tryBuildPatchProposal( text, proposal ) )
  {
    const QString proposalId = mReviewEngine->registerProposal( proposal );
    emit proposalCreated( proposalId );
    const QgsAiChatMessage assistant = buildAssistantMessage( u"Review proposal created (%1). Use Accept/Reject in the review panel."_s.arg( proposalId ) );
    mHistory.push_back( assistant );
    emit messageAdded( assistant );
    return;
  }

  bool contextBlocked = false;
  const QString contextSummary = buildContextSummary( contextFiles, contextBlocked );
  if ( contextBlocked )
  {
    const QgsAiChatMessage assistant = buildAssistantMessage( contextSummary );
    mHistory.push_back( assistant );
    emit messageAdded( assistant );
    emit requestStateChanged( u"error"_s, contextSummary );
    return;
  }

  // Append the file context (if any) to the user message so it travels through the history
  // and not just into the very first request.
  if ( !contextSummary.isEmpty() && contextSummary != "No file context attached."_L1 )
  {
    QgsAiChatMessage &lastUser = mHistory.last();
    lastUser.content = lastUser.content + u"\n\nContext:\n"_s + contextSummary;
  }
  mCurrentContextFiles = contextFiles;
  mToolIterations = 0;

  mPendingProviders = providerFallbackOrder();
  if ( mPendingProviders.isEmpty() || !mRouter )
  {
    const QgsAiChatMessage assistant = buildAssistantMessage( u"No provider is configured."_s );
    mHistory.push_back( assistant );
    emit messageAdded( assistant );
    return;
  }

  const QgsAiModelRouter::Provider firstProvider = mPendingProviders.takeFirst();
  emit requestRunningChanged( true );
  startProviderAttempt( firstProvider );
}

void QgsAiAgentSessionManager::setToolRegistry( QgsAiToolRegistry *registry )
{
  mToolRegistry = registry;
  if ( mRouter )
    mRouter->setToolRegistry( registry );
}

void QgsAiAgentSessionManager::setAgentBehaviorSettings( const QgsAiAgentBehaviorSettings &settings )
{
  mBehaviorSettings = settings;
  // Normalize the relative paths so saving an empty value falls back to the default folders.
  if ( mBehaviorSettings.rulesPath.trimmed().isEmpty() )
    mBehaviorSettings.rulesPath = u".qgis_ai/rules"_s;
  if ( mBehaviorSettings.skillsPath.trimmed().isEmpty() )
    mBehaviorSettings.skillsPath = u".qgis_ai/skills"_s;

  persistBehaviorSettings();
  if ( mRouter )
    mRouter->setToolUseEnabled( mBehaviorSettings.allowCustomActions );
}

void QgsAiAgentSessionManager::loadPersistedBehaviorSettings()
{
  QgsSettings settings;
  mBehaviorSettings.allowCustomActions = settings.value( u"qgis_ai/agent/allow_custom_actions"_s, false ).toBool();
  mBehaviorSettings.rulesText = settings.value( u"qgis_ai/agent/rules_text"_s, QString() ).toString();
  mBehaviorSettings.skillsText = settings.value( u"qgis_ai/agent/skills_text"_s, QString() ).toString();
  mBehaviorSettings.loadWorkspaceRules = settings.value( u"qgis_ai/agent/load_workspace_rules"_s, true ).toBool();
  mBehaviorSettings.loadWorkspaceSkills = settings.value( u"qgis_ai/agent/load_workspace_skills"_s, true ).toBool();
  mBehaviorSettings.rulesPath = settings.value( u"qgis_ai/agent/rules_path"_s, u".qgis_ai/rules"_s ).toString();
  mBehaviorSettings.skillsPath = settings.value( u"qgis_ai/agent/skills_path"_s, u".qgis_ai/skills"_s ).toString();
}

void QgsAiAgentSessionManager::persistBehaviorSettings() const
{
  QgsSettings settings;
  settings.setValue( u"qgis_ai/agent/allow_custom_actions"_s, mBehaviorSettings.allowCustomActions );
  settings.setValue( u"qgis_ai/agent/rules_text"_s, mBehaviorSettings.rulesText );
  settings.setValue( u"qgis_ai/agent/skills_text"_s, mBehaviorSettings.skillsText );
  settings.setValue( u"qgis_ai/agent/load_workspace_rules"_s, mBehaviorSettings.loadWorkspaceRules );
  settings.setValue( u"qgis_ai/agent/load_workspace_skills"_s, mBehaviorSettings.loadWorkspaceSkills );
  settings.setValue( u"qgis_ai/agent/rules_path"_s, mBehaviorSettings.rulesPath );
  settings.setValue( u"qgis_ai/agent/skills_path"_s, mBehaviorSettings.skillsPath );
}

QString QgsAiAgentSessionManager::readWorkspaceTextFiles( const QString &relativeDir ) const
{
  if ( !mContextProvider || relativeDir.trimmed().isEmpty() )
    return QString();

  const QString root = mContextProvider->workspaceRoot();
  if ( root.isEmpty() )
    return QString();

  QDir baseDir( root );
  const QString absolutePath = QDir::cleanPath( baseDir.filePath( relativeDir ) );
  // Reject paths that escape the workspace root (e.g. "../../etc").
  if ( !mContextProvider->isInWorkspace( absolutePath ) )
    return QString();

  const QFileInfo info( absolutePath );
  if ( !info.exists() || !info.isDir() )
    return QString();

  QDir dir( absolutePath );
  const QStringList filters = { u"*.md"_s, u"*.markdown"_s, u"*.txt"_s };
  const QFileInfoList entries = dir.entryInfoList( filters, QDir::Files | QDir::Readable, QDir::Name );

  QStringList sections;
  // Cap the per-file budget so a runaway rule file cannot drown the prompt.
  static constexpr qint64 MAX_BYTES_PER_FILE = 16384;
  for ( const QFileInfo &entry : entries )
  {
    QFile file( entry.absoluteFilePath() );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
      continue;
    const QByteArray raw = file.read( MAX_BYTES_PER_FILE );
    QString content = QString::fromUtf8( raw ).trimmed();
    if ( content.isEmpty() )
      continue;
    if ( file.size() > MAX_BYTES_PER_FILE )
      content += u"\n…[truncated]"_s;
    const QString relativeFile = QDir( root ).relativeFilePath( entry.absoluteFilePath() );
    sections << u"# %1\n%2"_s.arg( relativeFile, content );
  }
  return sections.join( "\n\n"_L1 );
}

QString QgsAiAgentSessionManager::collectRulesContent() const
{
  QStringList parts;
  const QString inline_ = mBehaviorSettings.rulesText.trimmed();
  if ( !inline_.isEmpty() )
    parts << inline_;
  if ( mBehaviorSettings.loadWorkspaceRules )
  {
    const QString workspace = readWorkspaceTextFiles( mBehaviorSettings.rulesPath );
    if ( !workspace.isEmpty() )
      parts << workspace;
  }
  return parts.join( "\n\n"_L1 );
}

QString QgsAiAgentSessionManager::collectSkillsContent() const
{
  QStringList parts;
  const QString inline_ = mBehaviorSettings.skillsText.trimmed();
  if ( !inline_.isEmpty() )
    parts << inline_;
  if ( mBehaviorSettings.loadWorkspaceSkills )
  {
    const QString workspace = readWorkspaceTextFiles( mBehaviorSettings.skillsPath );
    if ( !workspace.isEmpty() )
      parts << workspace;
  }
  return parts.join( "\n\n"_L1 );
}

QString QgsAiAgentSessionManager::buildSystemPrompt() const
{
  // Cursor-like file-acting agent for QGIS. Tells the model it has tools and must use them
  // instead of telling the user to copy code. Workspace-aware fields are filled when known.
  QString prompt;
  prompt += u"You are the %1 agent for QGIS_AI, a fork of QGIS with native AI assistance.\n"_s.arg( mActiveAgent );
  prompt += "Your job: help the user inspect, modify and run code/data in their workspace.\n\n"_L1;

  prompt += "== Workspace ==\n"_L1;
  if ( mContextProvider && !mContextProvider->workspaceRoot().isEmpty() )
    prompt += u"Root: %1\n"_s.arg( mContextProvider->workspaceRoot() );
  else
    prompt += "Root: (not set)\n"_L1;

  // Inject a snapshot of the active QgsProject so the model knows which layers exist
  // without having to call list_project_layers every turn.
  QgsProject *project = QgsProject::instance();
  if ( project )
  {
    const QString projectFile = project->fileName();
    prompt += u"Active project: %1\n"_s.arg( projectFile.isEmpty() ? u"(unsaved)"_s : projectFile );

    const QMap<QString, QgsMapLayer *> layers = project->mapLayers();
    prompt += u"Loaded layers: %1\n"_s.arg( layers.size() );
    for ( auto it = layers.constBegin(); it != layers.constEnd(); ++it )
    {
      QgsMapLayer *layer = it.value();
      if ( !layer )
        continue;
      prompt += u"  - %1 (id=%2, crs=%3)\n"_s.arg( layer->name(), layer->id(), layer->crs().authid() );
    }
    prompt += '\n';
    prompt += buildLayerDataDump();
  }

  // Tool list is injected so the model has discoverable names alongside the JSON schema,
  // but only when the user actually allows custom actions. Otherwise we hide the catalogue
  // entirely so the model does not even attempt tool use.
  if ( mToolRegistry && mBehaviorSettings.allowCustomActions )
  {
    const QStringList toolNames = mToolRegistry->toolNames();
    if ( !toolNames.isEmpty() )
    {
      prompt += "\n== Available tools ==\n"_L1;
      prompt += toolNames.join( ", "_L1 );
      prompt += '\n';
    }
  }

  // User-provided rules and skills (inline + workspace files) live on top of the built-in
  // guardrails so the user can layer their own policies and helpers on top.
  const QString rulesContent = collectRulesContent();
  if ( !rulesContent.isEmpty() )
  {
    prompt += "\n== User rules ==\n"_L1;
    prompt += rulesContent;
    prompt += '\n';
  }

  const QString skillsContent = collectSkillsContent();
  if ( !skillsContent.isEmpty() )
  {
    prompt += "\n== User skills ==\n"_L1;
    prompt += skillsContent;
    prompt += '\n';
  }

  prompt += "\n== How to act ==\n"_L1;
  if ( !mBehaviorSettings.allowCustomActions )
  {
    prompt += "- Custom agent actions are DISABLED by the user. Do not attempt to call any tool. "
              "Answer in plain text only and tell the user to enable 'Allow custom agent actions' in AI settings if a tool is needed.\n"_L1;
    return prompt;
  }
  prompt += "- Use tools instead of writing code in chat for the user to copy.\n"_L1;
  prompt += "- To inspect files: read_file, search_files, list_files. To inspect project state: list_project_layers, get_active_canvas_extent.\n"_L1;
  prompt += "- To modify files: ALWAYS go through propose_edit / propose_create_file / propose_delete_file (when available). The user will review and accept your diff.\n"_L1;
  prompt += "- Never call propose_edit blind: read the file first to capture the exact original text.\n"_L1;
  prompt += "- Keep proposals small and reviewable. One concept per proposal.\n"_L1;
  prompt += "- Do not invent file paths; resolve them via search_files or list_files.\n"_L1;
  prompt += "- External libraries and remote data are AVAILABLE. Do NOT refuse with phrases like 'I cannot run external libraries in this environment.' You can.\n"_L1;
  prompt += "  - To fetch remote files (GeoJSON, Shapefile, Overpass/Nominatim/GADM responses): PREFER download_file(url, dest_path). One approval, stays inside the workspace, no extra packages.\n"_L1;
  prompt += u"  - To use a Python library not bundled with QGIS (geopy, osmnx, requests, shapely, pandas, …):\n"_s;
  prompt += "      1) Briefly state the plan in chat.\n"_L1;
  prompt += "      2) Call install_python_package with exact pinned specs (the user approves).\n"_L1;
  prompt += "      3) Then call run_python to use them.\n"_L1;
  prompt += u"  - Concrete example — 'boundary of Pomponesco, Italy': prefer download_file with an Overpass API query (admin_level=8 boundary as GeoJSON), save in workspace, then add it as a layer via add_layer_from_file or run_python. Use osmnx only when a true graph/network API is needed.\n"_s;
  prompt += QStringLiteral(
              "- Reusable automation: when the user wants a workflow they can repeat or share with the team, do NOT just run it via run_python — also save it as a Processing script. "
              "The Processing scripts folder for this profile is: %1 . "
              "Use propose_create_file to write a script there following the standard QgsProcessingAlgorithm template "
              "(class extending QgsProcessingAlgorithm with name(), displayName(), createInstance(), initAlgorithm(), processAlgorithm()). "
              "After acceptance the script appears in the Processing Toolbox under 'Scripts' and is callable like any built-in algorithm.\n"
  )
              .arg( processingScriptsFolder() );
  return prompt;
}

QString QgsAiAgentSessionManager::buildLayerDataDump() const
{
  QgsProject *project = QgsProject::instance();
  if ( !project )
    return QString();

  QString out;
  out += "== Layer data dump ==\n"_L1;
  out += "Full snapshot of every layer (attributes + WKT geometries for vectors, "
         "extended metadata + pixel samples for rasters). No truncation.\n\n"_L1;

  const QMap<QString, QgsMapLayer *> layers = project->mapLayers();
  for ( auto it = layers.constBegin(); it != layers.constEnd(); ++it )
  {
    QgsMapLayer *layer = it.value();
    if ( !layer )
      continue;

    out += u"--- Layer: %1 (id=%2) ---\n"_s.arg( layer->name(), layer->id() );

    if ( QgsVectorLayer *v = qobject_cast<QgsVectorLayer *>( layer ) )
      out += dumpVectorLayer( v );
    else if ( QgsRasterLayer *r = qobject_cast<QgsRasterLayer *>( layer ) )
      out += dumpRasterLayer( r );
    else
      out += "(unsupported layer type, only metadata above)\n"_L1;

    out += '\n';
  }
  return out;
}

QString QgsAiAgentSessionManager::dumpVectorLayer( QgsVectorLayer *layer ) const
{
  QString out;
  out += u"type: vector, geometry: %1, crs: %2, features: %3\n"_s
           .arg( QgsWkbTypes::geometryDisplayString( layer->geometryType() ), layer->crs().authid() )
           .arg( static_cast<qint64>( layer->featureCount() ) );

  const QgsFields fields = layer->fields();
  QStringList fieldNames;
  for ( const QgsField &f : fields )
    fieldNames << u"%1:%2"_s.arg( f.name(), f.typeName() );
  out += u"fields: %1\n"_s.arg( fieldNames.join( ", "_L1 ) );
  out += "data (one feature per line: id | attr1=val1; attr2=val2; ... | WKT):\n"_L1;

  QgsFeatureIterator it = layer->getFeatures();
  QgsFeature feature;
  while ( it.nextFeature( feature ) )
  {
    QStringList kv;
    for ( const QgsField &f : fields )
    {
      const QVariant val = feature.attribute( f.name() );
      kv << u"%1=%2"_s.arg( f.name(), val.toString() );
    }
    const QgsGeometry geom = feature.geometry();
    const QString wkt = geom.isNull() ? u"NULL"_s : geom.asWkt();
    out += u"  %1 | %2 | %3\n"_s.arg( feature.id() ).arg( kv.join( "; "_L1 ), wkt );
  }
  return out;
}

QString QgsAiAgentSessionManager::dumpRasterLayer( QgsRasterLayer *layer ) const
{
  QString out;
  out += u"type: raster, crs: %1, size: %2x%3, bands: %4\n"_s
           .arg( layer->crs().authid() )
           .arg( layer->width() )
           .arg( layer->height() )
           .arg( layer->bandCount() );

  const QgsRectangle ext = layer->extent();
  out += u"extent: xmin=%1 ymin=%2 xmax=%3 ymax=%4\n"_s
           .arg( ext.xMinimum() )
           .arg( ext.yMinimum() )
           .arg( ext.xMaximum() )
           .arg( ext.yMaximum() );

  QgsRasterDataProvider *dp = layer->dataProvider();
  if ( !dp )
    return out + "(no data provider)\n"_L1;

  for ( int b = 1; b <= layer->bandCount(); ++b )
  {
    const QgsRasterBandStats stats = dp->bandStatistics( b );
    out += u"band %1: min=%2 max=%3 mean=%4 stddev=%5\n"_s
             .arg( b )
             .arg( stats.minimumValue )
             .arg( stats.maximumValue )
             .arg( stats.mean )
             .arg( stats.stdDev );

    out += u"  samples (16x16 grid over extent):\n"_s;
    constexpr int N = 16;
    for ( int iy = 0; iy < N; ++iy )
    {
      QStringList row;
      for ( int ix = 0; ix < N; ++ix )
      {
        const double x = ext.xMinimum() + ( ix + 0.5 ) * ext.width() / N;
        const double y = ext.yMaximum() - ( iy + 0.5 ) * ext.height() / N;
        bool ok = false;
        const double v = dp->sample( QgsPointXY( x, y ), b, &ok );
        row << ( ok ? QString::number( v ) : u"NaN"_s );
      }
      out += u"    %1\n"_s.arg( row.join( ", "_L1 ) );
    }
  }
  return out;
}

QString QgsAiAgentSessionManager::processingScriptsFolder()
{
  return QgsApplication::qgisSettingsDirPath() + u"processing/scripts"_s;
}

QList<QgsAiChatMessage> QgsAiAgentSessionManager::trimHistoryByTokenBudget( int budgetTokens ) const
{
  if ( mHistory.isEmpty() )
    return mHistory;

  // Identify "atomic groups" so we don't split a tool_use round from its tool_results when trimming.
  struct GroupRange
  {
      int start;
      int end;
  };
  QList<GroupRange> groups;
  int i = 0;
  while ( i < mHistory.size() )
  {
    int start = i;
    int end = i;
    const bool isAssistantWithTools = mHistory.at( i ).role == QgsAiChatRole::Assistant && mHistory.at( i ).metadata.contains( u"tool_calls"_s );
    if ( isAssistantWithTools )
    {
      while ( end + 1 < mHistory.size() && mHistory.at( end + 1 ).role == QgsAiChatRole::Tool )
        ++end;
    }
    groups.append( { start, end } );
    i = end + 1;
  }

  // Walk groups newest-first; keep what fits, but always keep at least the most recent group.
  auto groupTokens = [this]( const GroupRange &g ) {
    int total = 0;
    for ( int j = g.start; j <= g.end; ++j )
      total += mHistory.at( j ).content.size() / 4;
    return total;
  };

  QList<int> keepGroups;
  int totalTokens = 0;
  for ( int g = groups.size() - 1; g >= 0; --g )
  {
    const int t = groupTokens( groups.at( g ) );
    if ( !keepGroups.isEmpty() && totalTokens + t > budgetTokens )
      break;
    keepGroups.prepend( g );
    totalTokens += t;
  }

  QList<QgsAiChatMessage> result;
  for ( int g : keepGroups )
  {
    const GroupRange &range = groups.at( g );
    for ( int j = range.start; j <= range.end; ++j )
      result.append( mHistory.at( j ) );
  }
  return result;
}

QList<QgsAiChatMessage> QgsAiAgentSessionManager::buildOutgoingMessages() const
{
  QList<QgsAiChatMessage> result;

  QgsAiChatMessage systemMessage;
  systemMessage.id = QUuid::createUuid().toString( QUuid::WithoutBraces );
  systemMessage.role = QgsAiChatRole::System;
  systemMessage.content = buildSystemPrompt();
  systemMessage.timestamp = QDateTime::currentDateTimeUtc();
  result.append( systemMessage );

  result.append( trimHistoryByTokenBudget( HISTORY_TOKEN_BUDGET ) );
  return result;
}

QgsAiChatMessage QgsAiAgentSessionManager::buildAssistantToolUseMessage( const QString &text, const QList<QgsAiToolCall> &calls ) const
{
  QgsAiChatMessage assistant;
  assistant.id = QUuid::createUuid().toString( QUuid::WithoutBraces );
  assistant.role = QgsAiChatRole::Assistant;
  assistant.content = text;
  assistant.timestamp = QDateTime::currentDateTimeUtc();

  QVariantList toolCallsVariant;
  for ( const QgsAiToolCall &call : calls )
  {
    QVariantMap entry;
    entry.insert( u"id"_s, call.id );
    entry.insert( u"name"_s, call.name );
    entry.insert( u"args"_s, call.args.toVariantMap() );
    toolCallsVariant.append( entry );
  }
  assistant.metadata.insert( u"tool_calls"_s, toolCallsVariant );
  return assistant;
}

QgsAiChatMessage QgsAiAgentSessionManager::buildToolResultMessage( const QgsAiToolCall &call, const QgsAiToolResult &result ) const
{
  QgsAiChatMessage toolMessage;
  toolMessage.id = QUuid::createUuid().toString( QUuid::WithoutBraces );
  toolMessage.role = QgsAiChatRole::Tool;
  toolMessage.timestamp = QDateTime::currentDateTimeUtc();

  QString serialized;
  if ( result.success )
  {
    if ( result.output.isString() )
      serialized = result.output.toString();
    else
      serialized = QString::fromUtf8( QJsonDocument( result.output.toObject() ).toJson( QJsonDocument::Compact ) );
    if ( serialized.isEmpty() )
      serialized = u"{}"_s;
  }
  else
  {
    QJsonObject errObj;
    errObj.insert( u"error"_s, result.errorMessage );
    serialized = QString::fromUtf8( QJsonDocument( errObj ).toJson( QJsonDocument::Compact ) );
  }
  toolMessage.content = serialized;
  toolMessage.metadata.insert( u"tool_call_id"_s, call.id );
  toolMessage.metadata.insert( u"tool_name"_s, call.name );
  if ( !result.success )
    toolMessage.metadata.insert( u"is_error"_s, true );
  return toolMessage;
}

void QgsAiAgentSessionManager::onToolCallsRequested( const QString &requestId, const QString &providerName, const QString &assistantText, const QList<QgsAiToolCall> &calls )
{
  if ( requestId != mActiveRequestId )
    return;

  // Append the assistant turn that requested the tools to history so the next round carries it.
  const QgsAiChatMessage assistantMessage = buildAssistantToolUseMessage( assistantText, calls );
  mHistory.append( assistantMessage );
  emit messageAdded( assistantMessage );

  // Surface a short status to the UI: which tools the model wants to use.
  QStringList summary;
  summary.reserve( calls.size() );
  for ( const QgsAiToolCall &call : calls )
    summary << call.name;
  emit requestStateChanged( u"tool_use"_s, u"%1 wants to call: %2"_s.arg( providerName, summary.join( ", "_L1 ) ) );

  if ( !mToolRegistry )
  {
    const QgsAiChatMessage error = buildAssistantMessage( u"The model requested tool use but no tool registry is configured. Aborting turn."_s );
    mHistory.append( error );
    emit messageAdded( error );
    mActiveRequestId.clear();
    emit requestRunningChanged( false );
    return;
  }

  ++mToolIterations;
  if ( mToolIterations > MAX_TOOL_ITERATIONS_PER_TURN )
  {
    const QgsAiChatMessage error = buildAssistantMessage( u"Stopping: the model exceeded the maximum number of tool calls (%1) for a single turn."_s.arg( MAX_TOOL_ITERATIONS_PER_TURN ) );
    mHistory.append( error );
    emit messageAdded( error );
    mActiveRequestId.clear();
    emit requestRunningChanged( false );
    return;
  }

  // Execute every requested tool synchronously and add its result to history.
  // Approval-gated tools (file edits, run_python) are wired in later sprints; for B1 we
  // execute everything inline so the loop itself can be exercised end-to-end.
  for ( const QgsAiToolCall &call : calls )
  {
    QgsMessageLog::
      logMessage( u"Tool call: name=%1 id=%2 argsBytes=%3"_s.arg( call.name, call.id ).arg( QJsonDocument( call.args ).toJson( QJsonDocument::Compact ).size() ), u"AI"_s, Qgis::MessageLevel::Info, false );

    const QgsAiToolResult result = mToolRegistry->execute( call.name, call.args );
    const QgsAiChatMessage resultMessage = buildToolResultMessage( call, result );
    mHistory.append( resultMessage );
    emit messageAdded( resultMessage );
  }

  // Continue the conversation with the same provider (no fallback rotation mid-loop).
  mActiveRequestId.clear();
  startProviderAttempt( mActiveProvider );
}
