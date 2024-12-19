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
        if ( jLink.is_object() &&
             jLink.contains( "href" ) &&
             jLink.contains( "rel" ) )
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

QString QgsOAPIFJson::findLink( const std::vector<QgsOAPIFJson::Link> &links,
                                const QString &rel,
                                const QStringList &preferableTypes )
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
