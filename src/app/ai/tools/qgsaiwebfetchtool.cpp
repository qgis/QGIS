/***************************************************************************
    qgsaiwebfetchtool.cpp
    ---------------------
    begin                : July 2026
    copyright            : (C) 2026 by Valerio Sota
    email                : valeriosota.dev at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaiwebfetchtool.h"

#include "qgsaitoolschemautil.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QUrl>

using namespace Qt::StringLiterals;

namespace
{
  // The backend fetch provider waits up to 15s for JS-rendered pages, so this
  // tool needs more headroom than the 20s used by the metadata-only searches.
  constexpr int WEB_FETCH_TIMEOUT_MS = 45000;
} //namespace

QString QgsAiWebFetchTool::description() const
{
  return u"Fetches a web page through Strata Plan and returns its content as markdown (credits metered). Use it to read a page found via web_search or catalog_search before acting on it."_s;
}

QJsonObject QgsAiWebFetchTool::schema() const
{
  QJsonObject maxTokens = prop( u"integer"_s, u"Maximum content size in tokens, 100-8000. Default 4000."_s );
  maxTokens.insert( u"minimum"_s, 100 );
  maxTokens.insert( u"maximum"_s, 8000 );

  QJsonObject properties;
  properties.insert( u"url"_s, prop( u"string"_s, u"The http(s) URL of the page to fetch."_s ) );
  properties.insert( u"maxTokens"_s, maxTokens );
  return schemaObject( properties, QJsonArray { u"url"_s } );
}

QgsAiToolResult QgsAiWebFetchTool::execute( const QJsonObject &args )
{
  const QString url = args.value( u"url"_s ).toString().trimmed();
  if ( url.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'url' is required."_s );
  const QUrl parsed( url );
  if ( !parsed.isValid() || ( parsed.scheme() != "http"_L1 && parsed.scheme() != "https"_L1 ) )
    return QgsAiToolResult::error( u"Argument 'url' must be a valid http(s) URL."_s );

  QJsonObject body;
  body.insert( u"url"_s, url );
  if ( args.contains( u"maxTokens"_s ) )
    body.insert( u"maxTokens"_s, args.value( u"maxTokens"_s ).toInt() );
  return postSearch( u"/v1/tools/web-fetch"_s, body, WEB_FETCH_TIMEOUT_MS );
}
