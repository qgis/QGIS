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

QgsStacObject::Type QgsStacCatalog::type() const
{
  return QgsStacObject::Type::Catalog;
}

QString QgsStacCatalog::toHtml() const
{
  QString html = QStringLiteral( "<html><head></head>\n<body>\n" );

  html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QLatin1String( "Catalog") );
  html += QLatin1String( "<table class=\"list-view\">\n" );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "id" ), id() );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "stac_version" ), stacVersion() );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "title" ), title() );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "description" ), description() );
  html += QLatin1String( "</table>\n" );

  if ( !mStacExtensions.isEmpty() )
  {
    html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QLatin1String( "Extensions") );
    html += QLatin1String( "<ul>\n" );
    for ( const QString &extension : mStacExtensions )
    {
      html += QStringLiteral( "<li>%1</li>\n" ).arg( extension );
    }
    html += QLatin1String( "</ul>\n" );
  }

  if ( ! mConformanceClasses.isEmpty() )
  {
    html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QLatin1String( "Conformance Classes") );
    html += QLatin1String( "<ul>\n" );
    for ( const QString &cc : mConformanceClasses )
    {
      html += QStringLiteral( "<li>%1</li>\n" ).arg( cc );
    }
    html += QLatin1String( "</ul>\n" );
  }

  html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QLatin1String( "Links") );
  for ( const QgsStacLink &link : mLinks )
  {
    html += QLatin1String( "<table class=\"list-view\">\n" );
    html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "relation" ), link.relation() );
    html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "title" ), link.title() );
    html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td><a href=\"%2\">%2</a></td></tr>\n" ).arg( QStringLiteral( "url" ), link.href() );
    html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "type" ), link.mediaType() );
    html += QLatin1String( "</table><br/>\n" );
  }

  html += QLatin1String( "\n</body>\n</html>\n" );
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

bool QgsStacCatalog::supportsStacApi() const
{
  return conformsTo( QStringLiteral( "http://www.opengis.net/spec/ogcapi-features-1/1.0/conf/oas30" ) );
}
