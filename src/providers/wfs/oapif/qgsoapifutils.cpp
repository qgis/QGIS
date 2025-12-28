/***************************************************************************
    qgsoapifutils.cpp
    ---------------------
    begin                : October 2019
    copyright            : (C) 2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoapifutils.h"

#include <limits>

#include <QRegularExpression>

using namespace Qt::StringLiterals;

const QString OAPIF_PROVIDER_DEFAULT_CRS = u"http://www.opengis.net/def/crs/OGC/1.3/CRS84"_s;

std::vector<QgsOAPIFJson::Link> QgsOAPIFJson::parseLinks( const json &jParent )
{
  std::vector<Link> links;
  if ( jParent.is_object() && jParent.contains( "links" ) )
  {
    const auto jLinks = jParent["links"];
    if ( jLinks.is_array() )
    {
      for ( const auto &jLink : jLinks )
      {
        if ( jLink.is_object() && jLink.contains( "href" ) && jLink.contains( "rel" ) )
        {
          const auto href = jLink["href"];
          const auto rel = jLink["rel"];
          if ( href.is_string() && rel.is_string() )
          {
            Link link;
            link.href = QString::fromStdString( href.get<std::string>() );
            link.rel = QString::fromStdString( rel.get<std::string>() );
            if ( jLink.contains( "type" ) )
            {
              const auto type = jLink["type"];
              if ( type.is_string() )
              {
                link.type = QString::fromStdString( type.get<std::string>() );
              }
            }
            if ( jLink.contains( "title" ) )
            {
              const auto title = jLink["title"];
              if ( title.is_string() )
              {
                link.title = QString::fromStdString( title.get<std::string>() );
              }
            }
            if ( jLink.contains( "length" ) )
            {
              const auto length = jLink["length"];
              if ( length.is_number_integer() )
              {
                link.length = length.get<qint64>();
              }
            }
            links.push_back( link );
          }
        }
      }
    }
  }
  return links;
}

QString QgsOAPIFJson::findLink( const std::vector<QgsOAPIFJson::Link> &links, const QString &rel, const QStringList &preferableTypes )
{
  QString resultHref;
  int resultPriority = std::numeric_limits<int>::max();
  for ( const auto &link : links )
  {
    if ( link.rel == rel )
    {
      int priority = -1;
      if ( !link.type.isEmpty() && !preferableTypes.isEmpty() )
      {
        priority = preferableTypes.indexOf( link.type );
      }
      if ( priority < 0 )
      {
        priority = static_cast<int>( preferableTypes.size() );
      }
      if ( priority < resultPriority )
      {
        resultHref = link.href;
        resultPriority = priority;
      }
    }
  }
  return resultHref;
}

QString QgsOAPIFGetNextLinkFromResponseHeader( const QList<QNetworkReply::RawHeaderPair> &responseHeaders, const QString &formatType )
{
  QString nextUrl;
  for ( const auto &headerKeyValue : responseHeaders )
  {
    if ( headerKeyValue.first.compare( QByteArray( "Link" ), Qt::CaseSensitivity::CaseInsensitive ) == 0 )
    {
      // Parse stuff like:
      //  <https://ogc-api.nrw.de/lika/v1/collections/flurstueck/items?f=html>; rel="alternate"; title="This document as HTML"; type="text/html", <https://ogc-api.nrw.de/lika/v1/collections/flurstueck/items?f=fgb&offset=10>; rel="next"; title="Next page"; type="application/flatgeobuf"

      // Split on commas, except when they are in double quotes or between <...>, and skip padding space before/after separator
      const thread_local QRegularExpression splitOnComma( R"(\s*,\s*(?=(?:[^"<]|"[^"]*"|<[^>]*>)*$))" );
      const QStringList links = QString::fromUtf8( headerKeyValue.second ).split( splitOnComma );
      QString nextUrlCandidate;
      for ( const QString &link : std::as_const( links ) )
      {
        if ( link.isEmpty() || link[0] != QLatin1Char( '<' ) )
          continue;
        const int idxClosingBracket = static_cast<int>( link.indexOf( QLatin1Char( '>' ) ) );
        if ( idxClosingBracket < 0 )
          continue;
        const QString href = link.mid( 1, idxClosingBracket - 1 );
        const int idxSemiColon = static_cast<int>( link.indexOf( QLatin1Char( ';' ), idxClosingBracket ) );
        if ( idxSemiColon < 0 )
          continue;
        // Split on semi-colon, except when they are in double quotes, and skip padding space before/after separator
        const thread_local QRegularExpression splitOnSemiColon( R"(\s*;\s*(?=(?:[^"]*"[^"]*")*[^"]*$))" );
        const QStringList linkParts = link.mid( idxSemiColon + 1 ).split( splitOnSemiColon );
        QString rel, type;
        for ( const QString &linkPart : std::as_const( linkParts ) )
        {
          // Split on equal, except when they are in double quotes, and skip padding space before/after separator
          const thread_local QRegularExpression splitOnEqual( R"(\s*\=\s*(?=(?:[^"]*"[^"]*")*[^"]*$))" );
          const QStringList keyValue = linkPart.split( splitOnEqual );
          if ( keyValue.size() == 2 )
          {
            const QString key = keyValue[0].trimmed();
            QString value = keyValue[1].trimmed();
            if ( !value.isEmpty() && value[0] == QLatin1Char( '"' ) && value.back() == QLatin1Char( '"' ) )
            {
              value = value.mid( 1, value.size() - 2 );
            }
            if ( key == "rel"_L1 )
            {
              rel = value;
            }
            else if ( key == "type"_L1 )
            {
              type = value;
            }
          }
        }
        if ( rel == "next"_L1 )
        {
          if ( type == formatType )
          {
            nextUrl = href;
            break;
          }
          else if ( nextUrlCandidate.isEmpty() && !href.contains( "f="_L1 ) )
          {
            // Some servers return a "next" link but advertizing only application/geojson
            // whereas they actually support paging for other types
            // So use that URL if it doesn't include a f= parameter
            nextUrlCandidate = href;
          }
        }
      }

      if ( nextUrl.isEmpty() && !nextUrlCandidate.isEmpty() )
        nextUrl = nextUrlCandidate;
      break;
    }
  }

  return nextUrl;
}
