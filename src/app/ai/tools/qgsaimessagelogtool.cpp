/***************************************************************************
    qgsaimessagelogtool.cpp
    ---------------------
    begin                : June 2026
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

#include "qgsaimessagelogtool.h"

#include "qgsaimessagelogbuffer.h"
#include "qgsaitoolschemautil.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace Qt::StringLiterals;

namespace
{
  QString levelToString( Qgis::MessageLevel level )
  {
    switch ( level )
    {
      case Qgis::MessageLevel::Info:
        return u"info"_s;
      case Qgis::MessageLevel::Warning:
        return u"warning"_s;
      case Qgis::MessageLevel::Critical:
        return u"critical"_s;
      case Qgis::MessageLevel::Success:
        return u"success"_s;
      case Qgis::MessageLevel::NoLevel:
        return u"none"_s;
    }
    return u"info"_s;
  }

  bool parseLevelToken( const QString &token, Qgis::MessageLevel &levelOut )
  {
    const QString normalized = token.trimmed().toLower();
    if ( normalized == u"info"_s )
    {
      levelOut = Qgis::MessageLevel::Info;
      return true;
    }
    if ( normalized == u"warning"_s || normalized == u"warn"_s )
    {
      levelOut = Qgis::MessageLevel::Warning;
      return true;
    }
    if ( normalized == u"critical"_s || normalized == u"error"_s )
    {
      levelOut = Qgis::MessageLevel::Critical;
      return true;
    }
    if ( normalized == u"success"_s )
    {
      levelOut = Qgis::MessageLevel::Success;
      return true;
    }
    return false;
  }

  QList<Qgis::MessageLevel> parseLevels( const QJsonArray &levelsArray )
  {
    QList<Qgis::MessageLevel> levels;
    for ( const QJsonValue &value : levelsArray )
    {
      Qgis::MessageLevel level;
      if ( parseLevelToken( value.toString(), level ) && !levels.contains( level ) )
        levels.append( level );
    }
    return levels;
  }

  QList<Qgis::MessageLevel> defaultLevels()
  {
    return {
      Qgis::MessageLevel::Warning,
      Qgis::MessageLevel::Critical,
    };
  }

  QJsonObject entryJson( const QgsAiMessageLogBuffer::Entry &entry )
  {
    QJsonObject object;
    object.insert( u"timestamp"_s, entry.timestamp.toString( Qt::ISODateWithMs ) );
    object.insert( u"level"_s, levelToString( entry.level ) );
    object.insert( u"tag"_s, entry.tag.isEmpty() ? u"General"_s : entry.tag );
    object.insert( u"message"_s, entry.message );
    return object;
  }
} // namespace

QgsAiReadMessageLogTool::QgsAiReadMessageLogTool( QgsAiMessageLogBuffer *buffer )
  : mBuffer( buffer )
{}

QString QgsAiReadMessageLogTool::description() const
{
  return QStringLiteral(
    "Returns recent QGIS Message Log entries (errors, warnings, info). Use when diagnosing "
    "layer load failures, Processing errors, plugin warnings, or after operations outside "
    "run_python. Prefer filtering by level and tag to reduce noise."
  );
}

QJsonObject QgsAiReadMessageLogTool::schema() const
{
  QJsonObject properties;
  QJsonObject levelsItem;
  levelsItem.insert( u"type"_s, u"array"_s );
  levelsItem.insert( u"description"_s, u"Optional log levels to include: info, warning, critical, success. Defaults to warning and critical."_s );
  QJsonObject levelToken;
  levelToken.insert( u"type"_s, u"string"_s );
  levelsItem.insert( u"items"_s, levelToken );
  properties.insert( u"levels"_s, levelsItem );

  QJsonObject tagsItem;
  tagsItem.insert( u"type"_s, u"array"_s );
  tagsItem.insert( u"description"_s, u"Optional exact tag filters, e.g. AI/Python or Processing."_s );
  QJsonObject tagToken;
  tagToken.insert( u"type"_s, u"string"_s );
  tagsItem.insert( u"items"_s, tagToken );
  properties.insert( u"tags"_s, tagsItem );

  properties.insert( u"search"_s, prop( u"string"_s, u"Optional case-insensitive substring filter on the message text."_s ) );
  properties.insert( u"limit"_s, prop( u"integer"_s, u"Maximum number of entries to return (default 50, max 200)."_s ) );
  properties.insert( u"since_seconds"_s, prop( u"integer"_s, u"Optional time window: only entries from the last N seconds."_s ) );

  return schemaObject( properties );
}

bool QgsAiReadMessageLogTool::isAvailable() const
{
  return mBuffer != nullptr;
}

QgsAiToolResult QgsAiReadMessageLogTool::execute( const QJsonObject &args )
{
  if ( !mBuffer )
    return QgsAiToolResult::error( u"Message log buffer is not available."_s );

  QgsAiMessageLogBuffer::Query query;
  if ( args.contains( u"levels"_s ) && args.value( u"levels"_s ).isArray() )
  {
    query.levels = parseLevels( args.value( u"levels"_s ).toArray() );
    if ( query.levels.isEmpty() )
      return QgsAiToolResult::error( u"No valid levels were provided."_s );
  }
  else
  {
    query.levels = defaultLevels();
  }

  if ( args.contains( u"tags"_s ) && args.value( u"tags"_s ).isArray() )
  {
    for ( const QJsonValue &value : args.value( u"tags"_s ).toArray() )
    {
      const QString tag = value.toString().trimmed();
      if ( !tag.isEmpty() )
        query.tags.append( tag );
    }
  }

  query.search = args.value( u"search"_s ).toString();
  query.limit = args.value( u"limit"_s ).toInt( 50 );
  if ( args.contains( u"since_seconds"_s ) )
    query.sinceSeconds = args.value( u"since_seconds"_s ).toInt( -1 );

  const QgsAiMessageLogBuffer::QueryResult bufferResult = mBuffer->query( query );

  QJsonArray entries;
  bool byteTruncated = false;
  int payloadBytes = 0;

  for ( const QgsAiMessageLogBuffer::Entry &entry : bufferResult.entries )
  {
    const QJsonObject object = entryJson( entry );
    const int objectBytes = QJsonDocument( object ).toJson( QJsonDocument::Compact ).size();
    if ( payloadBytes + objectBytes > MAX_CAPTURE_BYTES )
    {
      byteTruncated = true;
      break;
    }
    entries.append( object );
    payloadBytes += objectBytes;
  }

  QJsonObject output;
  output.insert( u"entries"_s, entries );
  output.insert( u"returned"_s, entries.size() );
  output.insert( u"total_buffered"_s, bufferResult.totalBuffered );
  output.insert( u"truncated"_s, bufferResult.truncated || byteTruncated );
  return QgsAiToolResult::ok( output );
}
