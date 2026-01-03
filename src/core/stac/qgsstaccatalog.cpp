/***************************************************************************
    qgsstaccatalog.cpp
    ---------------------
    begin                : August 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstaccatalog.h"

QgsStacCatalog::QgsStacCatalog( const QString &id,
                                const QString &version,
                                const QString &description,
                                const QVector< QgsStacLink > &links )
  : QgsStacObject( id, version, links )
  , mDescription( description )
{
}

Qgis::StacObjectType QgsStacCatalog::type() const
{
  return Qgis::StacObjectType::Catalog;
}

QString QgsStacCatalog::toHtml() const
{
  QString html = u"<html><head></head>\n<body>\n"_s;

  html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Catalog"_L1 );
  html += "<table class=\"list-view\">\n"_L1;
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"id"_s, id() );
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"stac_version"_s, stacVersion() );
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"title"_s, title() );
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"description"_s, description() );
  html += "</table>\n"_L1;

  if ( !mStacExtensions.isEmpty() )
  {
    html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Extensions"_L1 );
    html += "<ul>\n"_L1;
    for ( const QString &extension : mStacExtensions )
    {
      html += u"<li>%1</li>\n"_s.arg( extension );
    }
    html += "</ul>\n"_L1;
  }

  if ( ! mConformanceClasses.isEmpty() )
  {
    html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Conformance Classes"_L1 );
    html += "<ul>\n"_L1;
    for ( const QString &cc : mConformanceClasses )
    {
      html += u"<li>%1</li>\n"_s.arg( cc );
    }
    html += "</ul>\n"_L1;
  }

  html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Links"_L1 );
  for ( const QgsStacLink &link : mLinks )
  {
    html += "<table class=\"list-view\">\n"_L1;
    html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"relation"_s, link.relation() );
    html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"title"_s, link.title() );
    html += u"<tr><td class=\"highlight\">%1</td><td><a href=\"%2\">%2</a></td></tr>\n"_s.arg( u"url"_s, link.href() );
    html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"type"_s, link.mediaType() );
    html += "</table><br/>\n"_L1;
  }

  html += "\n</body>\n</html>\n"_L1;
  return html;
}

QString QgsStacCatalog::title() const
{
  return mTitle;
}

void QgsStacCatalog::setTitle( const QString &title )
{
  mTitle = title;
}

QString QgsStacCatalog::description() const
{
  return mDescription;
}

void QgsStacCatalog::setDescription( const QString &description )
{
  mDescription = description;
}

bool QgsStacCatalog::conformsTo( const QString &conformanceClass ) const
{
  return mConformanceClasses.contains( conformanceClass );
}

void QgsStacCatalog::setConformanceClasses( const QStringList &conformanceClasses )
{
  mConformanceClasses = QSet< QString >( conformanceClasses.constBegin(), conformanceClasses.constEnd() );
}

void QgsStacCatalog::addConformanceClass( const QString &conformanceClass )
{
  mConformanceClasses.insert( conformanceClass );
}
