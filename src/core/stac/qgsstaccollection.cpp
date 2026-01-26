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

Qgis::StacObjectType QgsStacCollection::type() const
{
  return Qgis::StacObjectType::Collection;
}

QString QgsStacCollection::toHtml() const
{
  QString html = u"<html><head></head>\n<body>\n"_s;

  html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Collection"_L1 );
  html += "<table class=\"list-view\">\n"_L1;
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"id"_s, id() );
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"stac_version"_s, stacVersion() );
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"title"_s, title() );
  QTextDocument descr;
  descr.setMarkdown( description() );
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"description"_s, descr.toHtml() );
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"license"_s, license() );
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"keywords"_s, keywords().join( ',' ) );
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

  if ( ! mProviders.isEmpty() )
  {
    html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Providers"_L1 );
    QTextDocument descr;
    for ( const QgsStacProvider &p : mProviders )
    {
      html += "<table class=\"list-view\">\n"_L1;
      html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"name"_s, p.name() );
      descr.setMarkdown( p.description() );
      html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"description"_s, descr.toHtml() );
      html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"roles"_s, p.roles().join( ',' ) );
      html += u"<tr><td class=\"highlight\">%1</td><td><a href=\"%2\">%2</a></td></tr>\n"_s.arg( u"url"_s, p.url() );
      html += "</table><br/>\n"_L1;
    }
  }

  html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Spatial Extent"_L1 );
  html += "<table class=\"list-view\">\n"_L1;
  QString extentString;
  if ( mExtent.spatialExtent().is3D() )
    extentString = mExtent.spatialExtent().toString();
  else
    extentString =  mExtent.spatialExtent().toRectangle().toString();
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"overall"_s, extentString );


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
      html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"detailed"_s, extentString );
    }
  }
  html += "</table><br/>\n"_L1;


  html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Temporal Extent"_L1 );
  html += "<table class=\"list-view\">\n"_L1;
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"start"_s, mExtent.temporalExtent().begin().toString() );
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"end"_s, mExtent.temporalExtent().end().toString() );
  html += "</table><br/>\n"_L1;

  if ( mExtent.hasDetailedTemporalExtents() )
  {
    html += "Detailed extents:\n"_L1;
    const QVector< QgsDateTimeRange > extents = mExtent.detailedTemporalExtents();
    for ( const QgsDateTimeRange &extent : extents )
    {
      html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Temporal Extent"_L1 );
      html += "<table class=\"list-view\">\n"_L1;
      html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"start"_s, extent.begin().toString() );
      html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"end"_s, extent.end().toString() );
      html += "</table><br/>\n"_L1;
    }
  }

  if ( ! mSummaries.isEmpty() )
  {
    html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Summaries"_L1 );
    html += "<table class=\"list-view\">\n"_L1;
    for ( auto it = mSummaries.constBegin(); it != mSummaries.constEnd(); ++it )
    {
      html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( it.key(), it.value().toString() );
    }
    html += "</table><br/>\n"_L1;
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

  if ( ! mAssets.isEmpty() )
  {
    QTextDocument descr;
    html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Assets"_L1 );
    for ( auto it = mAssets.constBegin(); it != mAssets.constEnd(); ++it )
    {
      html += "<table class=\"list-view\">\n"_L1;
      html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"id"_s, it.key() );
      html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"title"_s, it->title() );
      descr.setMarkdown( it->description() );
      html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"description"_s, descr.toHtml() );
      html += u"<tr><td class=\"highlight\">%1</td><td><a href=\"%2\">%2</a></td></tr>\n"_s.arg( u"url"_s, it->href() );
      html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"type"_s, it->mediaType() );
      html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"roles"_s, it->roles().join( ',' ) );
      html += "</table><br/>\n"_L1;
    }
  }

  html += "\n</body>\n</html>\n"_L1;
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
