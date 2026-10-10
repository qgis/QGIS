#include "qgsagentplugin.h"

#include "qgsgui.h"
#include "qgshistoryentry.h"
#include "qgshistoryproviderregistry.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>

#include <algorithm>

using namespace Qt::StringLiterals;

namespace
{
  bool isSensitiveKey( const QString &key )
  {
    const QString normalized = key.toLower();
    return normalized.contains( u"password"_s )
           || normalized.contains( u"passwd"_s )
           || normalized.contains( u"secret"_s )
           || normalized.contains( u"token"_s )
           || normalized.contains( u"authorization"_s )
           || normalized == u"auth"_s
           || normalized == u"api_key"_s
           || normalized == u"apikey"_s
           || normalized == u"access_key"_s
           || normalized == u"secret_key"_s;
  }

  QJsonValue redactDiagnosticValue( const QJsonValue &value )
  {
    if ( value.isArray() )
    {
      QJsonArray redacted;
      for ( const QJsonValue &item : value.toArray() )
        redacted.append( redactDiagnosticValue( item ) );
      return redacted;
    }
    if ( value.isObject() )
    {
      QJsonObject redacted;
      const QJsonObject object = value.toObject();
      for ( auto it = object.constBegin(); it != object.constEnd(); ++it )
        redacted.insert( it.key(), isSensitiveKey( it.key() ) ? QJsonValue( u"[redacted]"_s ) : redactDiagnosticValue( it.value() ) );
      return redacted;
    }
    return value;
  }

  QString boundedText( const QVariant &value )
  {
    return value.toString().left( 12000 );
  }

  bool looksLikeFailure( const QString &text )
  {
    return text.contains( u"error"_s, Qt::CaseInsensitive )
           || text.contains( u"fail"_s, Qt::CaseInsensitive )
           || text.contains( u"exception"_s, Qt::CaseInsensitive )
           || text.contains( u"traceback"_s, Qt::CaseInsensitive )
           || text.contains( u"invalid"_s, Qt::CaseInsensitive );
  }
}

void QgsAgentServer::recordToolDiagnostic( const QString &tool, const QJsonObject &arguments, const QJsonObject &result )
{
  if ( tool == u"recent_diagnostics"_s )
    return;

  QJsonObject entry{
    { u"timestamp"_s, QDateTime::currentDateTime().toString( Qt::ISODateWithMs ) },
    { u"tool"_s, tool },
    { u"ok"_s, result.value( u"ok"_s ).toBool( false ) },
    { u"arguments"_s, redactDiagnosticValue( arguments ) },
  };
  for ( const QString &key : { u"error"_s, u"log"_s } )
  {
    if ( result.value( key ).isString() )
      entry.insert( key, result.value( key ).toString().left( 12000 ) );
  }
  if ( result.contains( u"cancelled"_s ) )
    entry.insert( u"cancelled"_s, result.value( u"cancelled"_s ) );
  if ( result.contains( u"algorithm_id"_s ) )
    entry.insert( u"algorithm_id"_s, result.value( u"algorithm_id"_s ) );

  mRecentToolDiagnostics.append( entry );
  while ( mRecentToolDiagnostics.size() > 50 )
    mRecentToolDiagnostics.removeAt( 0 );
}

QJsonObject QgsAgentServer::recentDiagnostics( const QJsonObject &arguments ) const
{
  const int limit = std::clamp( arguments.value( u"limit"_s ).toInt( 15 ), 1, 50 );
  const int sinceMinutes = std::clamp( arguments.value( u"since_minutes"_s ).toInt( 120 ), 1, 10080 );
  const bool errorsOnly = arguments.value( u"errors_only"_s ).toBool( true );
  const bool includeProcessingHistory = arguments.value( u"include_processing_history"_s ).toBool( true );
  const QDateTime cutoff = QDateTime::currentDateTime().addSecs( -sinceMinutes * 60 );

  QJsonArray toolCalls;
  for ( int i = mRecentToolDiagnostics.size() - 1; i >= 0 && toolCalls.size() < limit; --i )
  {
    const QJsonObject entry = mRecentToolDiagnostics.at( i ).toObject();
    if ( errorsOnly && entry.value( u"ok"_s ).toBool() )
      continue;
    toolCalls.append( entry );
  }

  QJsonArray qgisMessages;
  for ( int i = mRecentQgisMessages.size() - 1; i >= 0 && qgisMessages.size() < limit; --i )
  {
    const QJsonObject entry = mRecentQgisMessages.at( i ).toObject();
    const QDateTime timestamp = QDateTime::fromString( entry.value( u"timestamp"_s ).toString(), Qt::ISODateWithMs );
    const QString level = entry.value( u"level"_s ).toString();
    if ( timestamp.isValid() && timestamp < cutoff )
      continue;
    if ( errorsOnly && level != u"warning"_s && level != u"critical"_s )
      continue;
    qgisMessages.append( entry );
  }

  QJsonArray processingHistory;
  if ( includeProcessingHistory && QgsGui::historyProviderRegistry() )
  {
    const QList<QgsHistoryEntry> entries = QgsGui::historyProviderRegistry()->queryEntries(
      cutoff,
      QDateTime::currentDateTime(),
      u"processing"_s
    );
    for ( int i = entries.size() - 1; i >= 0 && processingHistory.size() < limit; --i )
    {
      const QgsHistoryEntry &entry = entries.at( i );
      const QVariantMap details = entry.entry;
      const QString log = boundedText( details.value( u"log"_s ) );
      if ( errorsOnly && !looksLikeFailure( log ) )
        continue;

      QJsonObject item{
        { u"timestamp"_s, entry.timestamp.toString( Qt::ISODateWithMs ) },
        { u"algorithm_id"_s, details.value( u"algorithm_id"_s ).toString() },
        { u"python_command"_s, boundedText( details.value( u"python_command"_s ) ) },
        { u"process_command"_s, boundedText( details.value( u"process_command"_s ) ) },
        { u"log"_s, log },
      };
      if ( details.contains( u"parameters"_s ) )
        item.insert( u"parameters"_s, redactDiagnosticValue( QJsonValue::fromVariant( details.value( u"parameters"_s ) ) ) );
      if ( details.contains( u"results"_s ) )
        item.insert( u"results"_s, redactDiagnosticValue( QJsonValue::fromVariant( details.value( u"results"_s ) ) ) );
      processingHistory.append( item );
    }
  }

  return QJsonObject{
    { u"ok"_s, true },
    { u"since_minutes"_s, sinceMinutes },
    { u"errors_only"_s, errorsOnly },
    { u"project"_s, projectSummary() },
    { u"tool_calls"_s, toolCalls },
    { u"qgis_messages"_s, qgisMessages },
    { u"processing_history"_s, processingHistory },
    { u"note"_s, tr( "QGIS message capture starts when the Agent plugin is loaded. Processing history may include operations run before that time." ) },
  };
}
