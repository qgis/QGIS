/***************************************************************************
    qgsstaccollection.cpp
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

#include "qgsstaccollection.h"

#include <QTextDocument>

QgsStacCollection::QgsStacCollection( const QString &id,
                                      const QString &version,
                                      const QString &description,
                                      const QVector< QgsStacLink > &links,
                                      const QString &license,
                                      const QgsStacExtent &extent )
  : QgsStacCatalog( id, version, description, links )
  , mLicense( license )
  , mExtent( extent )
{
}

QgsStacObject::Type QgsStacCollection::type() const
{
  return QgsStacObject::Type::Collection;
}

QString QgsStacCollection::toHtml() const
{
  QString html = QStringLiteral( "<html><head></head>\n<body>\n" );

  html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QStringLiteral( "Collection" ) );
  html += QStringLiteral( "<table class=\"list-view\">\n" );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "id" ), id() );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "stac_version" ), stacVersion() );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "title" ), title() );
  QTextDocument descr;
  descr.setMarkdown( description() );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "description" ), descr.toHtml() );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "license" ), license() );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "keywords" ), keywords().join( ',' ) );
  html += QStringLiteral( "</table>\n" );

  if ( !mStacExtensions.isEmpty() )
  {
    html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QStringLiteral( "Extensions" ) );
    html += QStringLiteral( "<ul>\n" );
    for ( const QString &extension : mStacExtensions )
    {
      html += QStringLiteral( "<li>%1</li>\n" ).arg( extension );
    }
    html += QStringLiteral( "</ul>\n" );
  }

  if ( ! mConformanceClasses.isEmpty() )
  {
    html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QStringLiteral( "Conformance Classes" ) );
    html += QStringLiteral( "<ul>\n" );
    for ( const QString &cc : mConformanceClasses )
    {
      html += QStringLiteral( "<li>%1</li>\n" ).arg( cc );
    }
    html += QStringLiteral( "</ul>\n" );
  }

  if ( ! mProviders.isEmpty() )
  {
    html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QStringLiteral( "Providers" ) );
    QTextDocument descr;
    for ( const QgsStacProvider &p : mProviders )
    {
      html += QStringLiteral( "<table class=\"list-view\">\n" );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "name" ), p.name() );
      descr.setMarkdown( p.description() );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "description" ), descr.toHtml() );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "roles" ), p.roles().join( ',' ) );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td><a href=\"%2\">%2</a></td></tr>\n" ).arg( QStringLiteral( "url" ), p.url() );
      html += QStringLiteral( "</table><br/>\n" );
    }
  }

  html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QStringLiteral( "Spatial Extent" ) );
  html += QStringLiteral( "<table class=\"list-view\">\n" );
  QString extentString;
  if ( mExtent.spatialExtent().is3D() )
    extentString = mExtent.spatialExtent().toString();
  else
    extentString =  mExtent.spatialExtent().toRectangle().toString();
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "overall" ), extentString );


  if ( mExtent.hasDetailedSpatialExtents() )
  {
    const QVector< QgsBox3D > extents = mExtent.detailedSpatialExtents();
    for ( const QgsBox3D &extent : extents )
    {
      QString extentString;
      if ( extent.is3D() )
        extentString = extent.toString();
      else
        extentString =  extent.toRectangle().toString();
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "detailed" ), extentString );
    }
  }
  html += QStringLiteral( "</table><br/>\n" );


  html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QStringLiteral( "Temporal Extent" ) );
  html += QStringLiteral( "<table class=\"list-view\">\n" );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "start" ), mExtent.temporalExtent().begin().toString() );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "end" ), mExtent.temporalExtent().end().toString() );
  html += QStringLiteral( "</table><br/>\n" );

  if ( mExtent.hasDetailedTemporalExtents() )
  {
    html += QStringLiteral( "Detailed extents:\n" );
    const QVector< QgsDateTimeRange > extents = mExtent.detailedTemporalExtents();
    for ( const QgsDateTimeRange &extent : extents )
    {
      html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QStringLiteral( "Temporal Extent" ) );
      html += QStringLiteral( "<table class=\"list-view\">\n" );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "start" ), extent.begin().toString() );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "end" ), extent.end().toString() );
      html += QStringLiteral( "</table><br/>\n" );
    }
  }

  if ( ! mSummaries.isEmpty() )
  {
    html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QStringLiteral( "Summaries" ) );
    html += QStringLiteral( "<table class=\"list-view\">\n" );
    for ( auto it = mSummaries.constBegin(); it != mSummaries.constEnd(); ++it )
    {
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( it.key(), it.value().toString() );
    }
    html += QStringLiteral( "</table><br/>\n" );
  }

  html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QStringLiteral( "Links" ) );
  for ( const QgsStacLink &link : mLinks )
  {
    html += QStringLiteral( "<table class=\"list-view\">\n" );
    html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "relation" ), link.relation() );
    html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "title" ), link.title() );
    html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td><a href=\"%2\">%2</a></td></tr>\n" ).arg( QStringLiteral( "url" ), link.href() );
    html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "type" ), link.mediaType() );
    html += QStringLiteral( "</table><br/>\n" );
  }

  if ( ! mAssets.isEmpty() )
  {
    QTextDocument descr;
    html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QStringLiteral( "Assets" ) );
    for ( auto it = mAssets.constBegin(); it != mAssets.constEnd(); ++it )
    {
      html += QStringLiteral( "<table class=\"list-view\">\n" );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "id" ), it.key() );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "title" ), it->title() );
      descr.setMarkdown( it->description() );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "description" ), descr.toHtml() );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td><a href=\"%2\">%2</a></td></tr>\n" ).arg( QStringLiteral( "url" ), it->href() );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "type" ), it->mediaType() );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "roles" ), it->roles().join( ',' ) );
      html += QStringLiteral( "</table><br/>\n" );
    }
  }

  html += QStringLiteral( "\n</body>\n</html>\n" );
  return html;
}

QStringList QgsStacCollection::keywords() const
{
  return mKeywords;
}

void QgsStacCollection::setKeywords( const QStringList &keywords )
{
  mKeywords = keywords;
}

QString QgsStacCollection::license() const
{
  return mLicense;
}

void QgsStacCollection::setLicense( const QString &license )
{
  mLicense = license;
}

QVector<QgsStacProvider> QgsStacCollection::providers() const
{
  return mProviders;
}

void QgsStacCollection::setProviders( const QVector<QgsStacProvider> &providers )
{
  mProviders = providers;
}

QgsStacExtent QgsStacCollection::extent() const
{
  return mExtent;
}

void QgsStacCollection::setExtent( const QgsStacExtent &extent )
{
  mExtent = extent;
}

QVariantMap QgsStacCollection::summaries() const
{
  return mSummaries;
}

void QgsStacCollection::setSummaries( const QVariantMap &summaries )
{
  mSummaries = summaries;
}

QMap<QString, QgsStacAsset> QgsStacCollection::assets() const
{
  return mAssets;
}

void QgsStacCollection::setAssets( const QMap<QString, QgsStacAsset> &assets )
{
  mAssets = assets;
}
