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
#include "qgsmaplayer.h"
#include "qgsmessagelog.h"
#include "qgsproject.h"

#include <QDateTime>
#include <QDir>
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
  for ( QgsAiModelRouter::Provider provider : { QgsAiModelRouter::Provider::Plan, QgsAiModelRouter::Provider::OpenAi, QgsAiModelRouter::Provider::Claude } )
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
    return u"%1 authentication failed. Check the API key in Provider Settings."_s.arg( providerName );
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
    int shown = 0;
    for ( auto it = layers.constBegin(); it != layers.constEnd() && shown < 10; ++it, ++shown )
    {
      QgsMapLayer *layer = it.value();
      if ( !layer )
        continue;
      prompt += u"  - %1 (id=%2, crs=%3)\n"_s.arg( layer->name(), layer->id(), layer->crs().authid() );
    }
    if ( layers.size() > 10 )
      prompt += u"  …%1 more (use list_project_layers for the full list).\n"_s.arg( layers.size() - 10 );
  }

  // Tool list is injected so the model has discoverable names alongside the JSON schema.
  if ( mToolRegistry )
  {
    const QStringList toolNames = mToolRegistry->toolNames();
    if ( !toolNames.isEmpty() )
    {
      prompt += "\n== Available tools ==\n"_L1;
      prompt += toolNames.join( ", "_L1 );
      prompt += '\n';
    }
  }

  prompt += "\n== How to act ==\n"_L1;
  prompt += "- Use tools instead of writing code in chat for the user to copy.\n"_L1;
  prompt += "- To inspect files: read_file, search_files, list_files. To inspect project state: list_project_layers, get_active_canvas_extent.\n"_L1;
  prompt += "- To modify files: ALWAYS go through propose_edit / propose_create_file / propose_delete_file (when available). The user will review and accept your diff.\n"_L1;
  prompt += "- Never call propose_edit blind: read the file first to capture the exact original text.\n"_L1;
  prompt += "- Keep proposals small and reviewable. One concept per proposal.\n"_L1;
  prompt += "- Do not invent file paths; resolve them via search_files or list_files.\n"_L1;
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
