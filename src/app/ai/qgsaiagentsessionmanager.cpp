#include "qgsaiagentsessionmanager.h"

#include "qgsaifilecontextprovider.h"
#include "qgsaireviewpatchengine.h"

#include <QDateTime>
#include <QDir>
#include <QRegularExpression>
#include <QUuid>

QgsAiAgentSessionManager::QgsAiAgentSessionManager( QgsAiModelRouter *router, QgsAiFileContextProvider *contextProvider, QgsAiReviewPatchEngine *reviewEngine, QObject *parent )
  : QObject( parent )
  , mRouter( router )
  , mContextProvider( contextProvider )
  , mReviewEngine( reviewEngine )
{
  if ( mRouter )
  {
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

  QList<QgsAiChatMessage> messages;
  QgsAiChatMessage systemMessage;
  systemMessage.id = QUuid::createUuid().toString( QUuid::WithoutBraces );
  systemMessage.role = QgsAiChatRole::System;
  systemMessage.content = QStringLiteral( "You are the %1 agent for QGIS. Keep answers concise and implementation-oriented." ).arg( mActiveAgent );
  systemMessage.timestamp = QDateTime::currentDateTimeUtc();
  messages << systemMessage;

  QgsAiChatMessage userMessage;
  userMessage.id = QUuid::createUuid().toString( QUuid::WithoutBraces );
  userMessage.role = QgsAiChatRole::User;
  userMessage.content = mCurrentPrompt;
  userMessage.timestamp = QDateTime::currentDateTimeUtc();
  messages << userMessage;

  mStreamedText.clear();
  mActiveRequestId = mRouter->startChatRequest( provider, messages, true );
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

  mCurrentPrompt = text + QStringLiteral( "\n\nContext:\n" ) + contextSummary;
  mCurrentContextFiles = contextFiles;

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
