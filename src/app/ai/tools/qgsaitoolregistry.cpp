/***************************************************************************
    qgsaitoolregistry.cpp
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

#include "qgsaitoolregistry.h"

#include <QJsonObject>
#include <QSet>
#include <QString>

#include "moc_qgsaitoolregistry.cpp"

using namespace Qt::StringLiterals;

QgsAiToolRegistry::QgsAiToolRegistry( QObject *parent )
  : QObject( parent )
{}

QgsAiToolRegistry::~QgsAiToolRegistry() = default;

bool QgsAiToolRegistry::registerTool( std::unique_ptr<QgsAiTool> tool )
{
  if ( !tool )
    return false;

  const QString toolName = tool->name();
  if ( toolName.isEmpty() || mTools.contains( toolName ) )
    return false;

  mTools.emplace( toolName, std::move( tool ) );
  emit toolRegistered( toolName );
  return true;
}

QgsAiTool *QgsAiToolRegistry::find( const QString &name ) const
{
  const auto it = mTools.find( name );
  return it == mTools.end() ? nullptr : it->second.get();
}

QStringList QgsAiToolRegistry::toolNames() const
{
  QStringList names;
  names.reserve( static_cast<int>( mTools.size() ) );
  for ( const auto &entry : mTools )
    names.append( entry.first );
  return names;
}

QStringList QgsAiToolRegistry::availableToolNames() const
{
  QStringList names;
  names.reserve( static_cast<int>( mTools.size() ) );
  for ( const auto &entry : mTools )
  {
    const QgsAiTool *tool = entry.second.get();
    if ( tool && tool->isAvailable() )
      names.append( entry.first );
  }
  return names;
}

QJsonArray QgsAiToolRegistry::schemasJson( const QStringList &allowedTools ) const
{
  return schemasJsonForFormat( WireFormat::AnthropicTools, allowedTools );
}

QJsonArray QgsAiToolRegistry::schemasJsonForFormat( WireFormat format, const QStringList &allowedTools ) const
{
  const QSet<QString> filter( allowedTools.begin(), allowedTools.end() );
  QJsonArray array;
  for ( auto it = mTools.cbegin(); it != mTools.cend(); ++it )
  {
    if ( !filter.isEmpty() && !filter.contains( it->first ) )
      continue;

    const QgsAiTool *tool = it->second.get();
    if ( !tool || !tool->isAvailable() )
      continue;

    QJsonObject entry;
    switch ( format )
    {
      case WireFormat::AnthropicTools:
        entry.insert( u"name"_s, tool->name() );
        entry.insert( u"description"_s, tool->description() );
        entry.insert( u"input_schema"_s, tool->schema() );
        break;

      case WireFormat::OpenAiResponses:
        entry.insert( u"type"_s, u"function"_s );
        entry.insert( u"name"_s, tool->name() );
        entry.insert( u"description"_s, tool->description() );
        entry.insert( u"parameters"_s, tool->schema() );
        break;

      case WireFormat::OpenAiChatCompletions:
      {
        QJsonObject function;
        function.insert( u"name"_s, tool->name() );
        function.insert( u"description"_s, tool->description() );
        function.insert( u"parameters"_s, tool->schema() );
        entry.insert( u"type"_s, u"function"_s );
        entry.insert( u"function"_s, function );
        break;
      }
    }
    array.append( entry );
  }
  return array;
}

QgsAiToolResult QgsAiToolRegistry::execute( const QString &name, const QJsonObject &args ) const
{
  QgsAiTool *tool = find( name );
  if ( !tool )
    return QgsAiToolResult::error( u"Unknown tool: %1"_s.arg( name ) );
  if ( !tool->isAvailable() )
  {
    const QString reason = tool->availabilityReason();
    return QgsAiToolResult::error( reason.isEmpty() ? u"Tool is not available: %1"_s.arg( name ) : reason );
  }
  return tool->execute( args );
}

void QgsAiToolRegistry::clear()
{
  mTools.clear();
}
