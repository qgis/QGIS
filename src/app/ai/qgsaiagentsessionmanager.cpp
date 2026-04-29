#include "qgsaiagentsessionmanager.h"

#include "qgsaifilecontextprovider.h"
#include "qgsaireviewpatchengine.h"
#include "qgsaitool.h"
#include "qgsaitoolregistry.h"
#include "qgsmaplayer.h"
#include "qgsmessagelog.h"
#include "qgsproject.h"

#include <QDateTime>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QUuid>
#include <QVariantList>

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
        emit requestStateChanged( QStringLiteral( "completed" ), QStringLiteral( "%1 (%2 ms)" ).arg( providerName ).arg( latencyMs ) );
        mActiveRequestId.clear();
        emit requestRunningChanged( false );
        return;
      }

      if ( !mPendingProviders.isEmpty() )
      {
        const QgsAiModelRouter::Provider fallbackProvider = mPendingProviders.takeFirst();
        emit requestStateChanged( QStringLiteral( "retrying" ), QStringLiteral( "%1 failed, retrying with %2…" ).arg( providerName, mRouter->providerDisplayName( fallbackProvider ) ) );
        startProviderAttempt( fallbackProvider );
        return;
      }

      const QString finalError = actionableError( providerName, errorMessage, httpStatus );
      const QgsAiChatMessage assistant = buildAssistantMessage( finalError );
      mHistory.push_back( assistant );
      emit messageAdded( assistant );
      emit requestStateChanged( QStringLiteral( "failed" ), finalError );
      mActiveRequestId.clear();
      emit requestRunningChanged( false );
    } );
  }
}

QStringList QgsAiAgentSessionManager::availableAgents() const
{
  return QStringList() << QStringLiteral( "planner" ) << QStringLiteral( "reviewer" ) << QStringLiteral( "editor" );
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
  emit requestStateChanged( QStringLiteral( "cancelled" ), QStringLiteral( "Request cancelled by user." ) );
  emit requestRunningChanged( false );
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
    QStringLiteral( "Provider attempt: agent=%1 provider=%2 requestId=%3 historyMessages=%4 toolIteration=%5 pendingFallbacks=%6" )
      .arg( mActiveAgent, mRouter->providerDisplayName( provider ), mActiveRequestId )
      .arg( messages.size() ).arg( mToolIterations ).arg( mPendingProviders.size() ),
    QStringLiteral( "AI" ), Qgis::MessageLevel::Info, false );
  emit requestStateChanged( QStringLiteral( "sending" ), QStringLiteral( "Contacting %1…" ).arg( mRouter->providerDisplayName( provider ) ) );
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
    return QStringLiteral( "No file context attached." );

  QStringList summaries;
  int index = 1;
  for ( const QgsAiChatContextFile &contextFile : contextFiles )
  {
    const QgsAiFileContext context = mContextProvider->buildContext( contextFile.filePath, contextFile.selectedText, 16384, contextFile.allowExternal );
    if ( !context.isValid() )
    {
      contextBlocked = true;
      return QStringLiteral( "File context blocked: path is outside allowed workspace or unreadable." );
    }

    const QString root = mContextProvider->workspaceRoot();
    const QString relativePath = QDir( root ).relativeFilePath( context.filePath );
    const bool inWorkspace = !root.isEmpty()
                             && ( relativePath == QLatin1String( "." )
                                  || ( !relativePath.startsWith( QStringLiteral( "../" ) )
                                       && relativePath != QLatin1String( ".." )
                                       && !QDir::isAbsolutePath( relativePath ) ) );
    const QString displayPath = inWorkspace ? relativePath : context.filePath;

    QString summary;
    summary += QStringLiteral( "Context file %1: %2\n" ).arg( index++ ).arg( displayPath );
    summary += QStringLiteral( "Size: %1 bytes%2\n" ).arg( context.fileSize ).arg( context.truncated ? QStringLiteral( " (snippet truncated)" ) : QString() );
    if ( !context.selectedText.isEmpty() )
      summary += QStringLiteral( "Selected text:\n%1\n" ).arg( context.selectedText.left( 8192 ) );
    if ( context.binary )
    {
      summary += QStringLiteral( "Content omitted because the file appears to be binary." );
    }
    else if ( !context.fileSnippet.isEmpty() )
    {
      summary += QStringLiteral( "File content snippet:\n%1" ).arg( context.fileSnippet );
    }
    else
    {
      summary += QStringLiteral( "No text content could be read from this file." );
    }
    summaries << summary.trimmed();
  }

  return summaries.join( QStringLiteral( "\n\n" ) );
}

QString QgsAiAgentSessionManager::actionableError( const QString &providerName, const QString &errorMessage, int httpStatus ) const
{
  const QString sanitized = mRouter ? mRouter->sanitizeErrorText( errorMessage ) : errorMessage;
  if ( httpStatus == 401 || httpStatus == 403 )
  {
    if ( providerName == QStringLiteral( "Plan Account" ) )
      return QStringLiteral( "%1 authentication failed. Check session token or authcfg in Provider Settings." ).arg( providerName );
    return QStringLiteral( "%1 authentication failed. Check the API key in Provider Settings." ).arg( providerName );
  }
  if ( httpStatus == 404 )
    return QStringLiteral( "%1 endpoint not found. Verify provider endpoint in settings." ).arg( providerName );
  if ( httpStatus == 429 )
    return QStringLiteral( "%1 rate-limited the request. Retry later or switch provider." ).arg( providerName );
  if ( httpStatus >= 500 )
    return QStringLiteral( "%1 is temporarily unavailable (HTTP %2). Please retry shortly." ).arg( providerName ).arg( httpStatus );
  if ( sanitized.isEmpty() )
    return QStringLiteral( "%1 request failed. Check provider settings and connectivity." ).arg( providerName );
  return QStringLiteral( "%1 request failed: %2" ).arg( providerName, sanitized );
}

bool QgsAiAgentSessionManager::tryBuildPatchProposal( const QString &text, QgsAiPatchProposal &proposal ) const
{
  static const QRegularExpression commandRe( QStringLiteral( "^/patch\\s+path=(.+)$" ) );
  static const QRegularExpression bodyRe( QStringLiteral( "<<<<\\n([\\s\\S]*)\\n====\\n([\\s\\S]*)\\n>>>>$" ) );

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
  proposal.title = QStringLiteral( "Patch generated by %1 agent" ).arg( mActiveAgent );
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
    const QgsAiChatMessage assistant = buildAssistantMessage( QStringLiteral( "A request is already running. Please wait or cancel it first." ) );
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
    const QgsAiChatMessage assistant = buildAssistantMessage( QStringLiteral( "Review proposal created (%1). Use Accept/Reject in the review panel." ).arg( proposalId ) );
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
    emit requestStateChanged( QStringLiteral( "error" ), contextSummary );
    return;
  }

  // Append the file context (if any) to the user message so it travels through the history
  // and not just into the very first request.
  if ( !contextSummary.isEmpty() && contextSummary != QLatin1String( "No file context attached." ) )
  {
    QgsAiChatMessage &lastUser = mHistory.last();
    lastUser.content = lastUser.content + QStringLiteral( "\n\nContext:\n" ) + contextSummary;
  }
  mCurrentContextFiles = contextFiles;
  mToolIterations = 0;

  mPendingProviders = providerFallbackOrder();
  if ( mPendingProviders.isEmpty() || !mRouter )
  {
    const QgsAiChatMessage assistant = buildAssistantMessage( QStringLiteral( "No provider is configured." ) );
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
  prompt += QStringLiteral( "You are the %1 agent for QGIS_AI, a fork of QGIS with native AI assistance.\n" ).arg( mActiveAgent );
  prompt += QStringLiteral( "Your job: help the user inspect, modify and run code/data in their workspace.\n\n" );

  prompt += QStringLiteral( "== Workspace ==\n" );
  if ( mContextProvider && !mContextProvider->workspaceRoot().isEmpty() )
    prompt += QStringLiteral( "Root: %1\n" ).arg( mContextProvider->workspaceRoot() );
  else
    prompt += QStringLiteral( "Root: (not set)\n" );

  // Inject a snapshot of the active QgsProject so the model knows which layers exist
  // without having to call list_project_layers every turn.
  QgsProject *project = QgsProject::instance();
  if ( project )
  {
    const QString projectFile = project->fileName();
    prompt += QStringLiteral( "Active project: %1\n" ).arg( projectFile.isEmpty() ? QStringLiteral( "(unsaved)" ) : projectFile );

    const QMap<QString, QgsMapLayer *> layers = project->mapLayers();
    prompt += QStringLiteral( "Loaded layers: %1\n" ).arg( layers.size() );
    int shown = 0;
    for ( auto it = layers.constBegin(); it != layers.constEnd() && shown < 10; ++it, ++shown )
    {
      QgsMapLayer *layer = it.value();
      if ( !layer )
        continue;
      prompt += QStringLiteral( "  - %1 (id=%2, crs=%3)\n" ).arg( layer->name(), layer->id(), layer->crs().authid() );
    }
    if ( layers.size() > 10 )
      prompt += QStringLiteral( "  …%1 more (use list_project_layers for the full list).\n" ).arg( layers.size() - 10 );
  }

  // Tool list is injected so the model has discoverable names alongside the JSON schema.
  if ( mToolRegistry )
  {
    const QStringList toolNames = mToolRegistry->toolNames();
    if ( !toolNames.isEmpty() )
    {
      prompt += QStringLiteral( "\n== Available tools ==\n" );
      prompt += toolNames.join( QStringLiteral( ", " ) );
      prompt += '\n';
    }
  }

  prompt += QStringLiteral( "\n== How to act ==\n" );
  prompt += QStringLiteral( "- Use tools instead of writing code in chat for the user to copy.\n" );
  prompt += QStringLiteral( "- To inspect files: read_file, search_files, list_files. To inspect project state: list_project_layers, get_active_canvas_extent.\n" );
  prompt += QStringLiteral( "- To modify files: ALWAYS go through propose_edit / propose_create_file / propose_delete_file (when available). The user will review and accept your diff.\n" );
  prompt += QStringLiteral( "- Never call propose_edit blind: read the file first to capture the exact original text.\n" );
  prompt += QStringLiteral( "- Keep proposals small and reviewable. One concept per proposal.\n" );
  prompt += QStringLiteral( "- Do not invent file paths; resolve them via search_files or list_files.\n" );
  return prompt;
}

QList<QgsAiChatMessage> QgsAiAgentSessionManager::trimHistoryByTokenBudget( int budgetTokens ) const
{
  if ( mHistory.isEmpty() )
    return mHistory;

  // Identify "atomic groups" so we don't split a tool_use round from its tool_results when trimming.
  struct GroupRange { int start; int end; };
  QList<GroupRange> groups;
  int i = 0;
  while ( i < mHistory.size() )
  {
    int start = i;
    int end = i;
    const bool isAssistantWithTools = mHistory.at( i ).role == QgsAiChatRole::Assistant
                                      && mHistory.at( i ).metadata.contains( QStringLiteral( "tool_calls" ) );
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
    entry.insert( QStringLiteral( "id" ), call.id );
    entry.insert( QStringLiteral( "name" ), call.name );
    entry.insert( QStringLiteral( "args" ), call.args.toVariantMap() );
    toolCallsVariant.append( entry );
  }
  assistant.metadata.insert( QStringLiteral( "tool_calls" ), toolCallsVariant );
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
      serialized = QStringLiteral( "{}" );
  }
  else
  {
    QJsonObject errObj;
    errObj.insert( QStringLiteral( "error" ), result.errorMessage );
    serialized = QString::fromUtf8( QJsonDocument( errObj ).toJson( QJsonDocument::Compact ) );
  }
  toolMessage.content = serialized;
  toolMessage.metadata.insert( QStringLiteral( "tool_call_id" ), call.id );
  toolMessage.metadata.insert( QStringLiteral( "tool_name" ), call.name );
  if ( !result.success )
    toolMessage.metadata.insert( QStringLiteral( "is_error" ), true );
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
  emit requestStateChanged( QStringLiteral( "tool_use" ),
                            QStringLiteral( "%1 wants to call: %2" ).arg( providerName, summary.join( QStringLiteral( ", " ) ) ) );

  if ( !mToolRegistry )
  {
    const QgsAiChatMessage error = buildAssistantMessage(
      QStringLiteral( "The model requested tool use but no tool registry is configured. Aborting turn." ) );
    mHistory.append( error );
    emit messageAdded( error );
    mActiveRequestId.clear();
    emit requestRunningChanged( false );
    return;
  }

  ++mToolIterations;
  if ( mToolIterations > MAX_TOOL_ITERATIONS_PER_TURN )
  {
    const QgsAiChatMessage error = buildAssistantMessage(
      QStringLiteral( "Stopping: the model exceeded the maximum number of tool calls (%1) for a single turn." ).arg( MAX_TOOL_ITERATIONS_PER_TURN ) );
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
    QgsMessageLog::logMessage(
      QStringLiteral( "Tool call: name=%1 id=%2 argsBytes=%3" )
        .arg( call.name, call.id ).arg( QJsonDocument( call.args ).toJson( QJsonDocument::Compact ).size() ),
      QStringLiteral( "AI" ), Qgis::MessageLevel::Info, false );

    const QgsAiToolResult result = mToolRegistry->execute( call.name, call.args );
    const QgsAiChatMessage resultMessage = buildToolResultMessage( call, result );
    mHistory.append( resultMessage );
    emit messageAdded( resultMessage );
  }

  // Continue the conversation with the same provider (no fallback rotation mid-loop).
  mActiveRequestId.clear();
  startProviderAttempt( mActiveProvider );
}
