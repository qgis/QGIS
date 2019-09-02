/***************************************************************************
    qgsbookmarkmanager.cpp
    --------------------
    Date                 : September 2019
    Copyright            : (C) 2019 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsbookmarkmanager.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgssqliteutils.h"
#include "qgsapplication.h"
#include <QUuid>
#include <sqlite3.h>

//
// QgsBookMark
//

QString QgsBookmark::id() const
{
  return mId;
}

void QgsBookmark::setId( const QString &id )
{
  mId = id;
}

QgsBookmark QgsBookmark::fromXml( const QDomElement &element, const QDomDocument & )
{
  QgsBookmark b;
  b.setId( element.attribute( QStringLiteral( "id" ) ) );
  b.setName( element.attribute( QStringLiteral( "name" ) ) );
  const QgsRectangle e = QgsRectangle::fromWkt( element.attribute( QStringLiteral( "extent" ) ) );
  QgsCoordinateReferenceSystem crs;
  crs.readXml( element );
  b.setExtent( QgsReferencedRectangle( e, crs ) );
  return b;
}

QDomElement QgsBookmark::writeXml( QDomDocument &doc ) const
{
  QDomElement bookmarkElem = doc.createElement( QStringLiteral( "Bookmark" ) );
  bookmarkElem.setAttribute( QStringLiteral( "id" ), mId );
  bookmarkElem.setAttribute( QStringLiteral( "name" ), mName );
  bookmarkElem.setAttribute( QStringLiteral( "extent" ), mExtent.asWktPolygon() );
  mExtent.crs().writeXml( bookmarkElem, doc );
  return bookmarkElem;
}

bool QgsBookmark::operator==( const QgsBookmark &other )
{
  return mId == other.mId && mName == other.mName && mExtent == other.mExtent;
}

bool QgsBookmark::operator!=( const QgsBookmark &other )
{
  return !( *this == other );
}

QString QgsBookmark::name() const
{
  return mName;
}

void QgsBookmark::setName( const QString &name )
{
  mName = name;
}

QgsReferencedRectangle QgsBookmark::extent() const
{
  return mExtent;
}

void QgsBookmark::setExtent( const QgsReferencedRectangle &extent )
{
  mExtent = extent;
}


QgsBookmarkManager::QgsBookmarkManager( QgsProject *project )
  : QObject( project )
  , mProject( project )
{

}


QgsBookmarkManager::QgsBookmarkManager( const QString &settingKey, QObject *parent )
  : QObject( parent )
  , mSettingKey( settingKey )
{
  // restore state
  QgsSettings settings;
  if ( !settings.value( settingKey, QVariant(), QgsSettings::Core ).isValid() )
  {
    mNeedToConvertOldBookmarks = true;
    // we can't do the conversion yet -- we need to wait till the application is initialized first...
  }
  else
  {
    const QString textXml = settings.value( settingKey, QString(), QgsSettings::Core ).toString();
    if ( !textXml.isEmpty() )
    {
      QDomDocument doc;
      doc.setContent( textXml );
      QDomElement elem = doc.documentElement();
      readXml( elem, doc );
    }
  }
}

QString QgsBookmarkManager::addBookmark( const QgsBookmark &b, bool *ok )
{
  if ( ok )
    *ok = false;

  QgsBookmark bookmark = b;
  if ( bookmark.id().isEmpty() )
    bookmark.setId( QUuid::createUuid().toString() );
  else
  {
    // check for duplicate ID
    for ( const QgsBookmark &b : qgis::as_const( mBookmarks ) )
    {
      if ( b.id() == bookmark.id() )
      {
        return QString();
      }
    }
  }

  if ( ok )
    *ok = true;

  emit bookmarkAboutToBeAdded( bookmark.id() );
  mBookmarks << bookmark;
  emit bookmarkAdded( bookmark.id() );
  store();
  return bookmark.id();
}

bool QgsBookmarkManager::removeBookmark( const QString &id )
{
  if ( id.isEmpty() )
    return false;

  int pos = -1;
  int i = 0;
  for ( const QgsBookmark &b : qgis::as_const( mBookmarks ) )
  {
    if ( b.id() == id )
    {
      pos = i;
      break;
    }
    i++;
  }

  if ( pos < 0 )
    return false;

  emit bookmarkAboutToBeRemoved( id );
  mBookmarks.removeAt( pos );
  emit bookmarkRemoved( id );
  store();
  return true;
}

bool QgsBookmarkManager::updateBookmark( const QgsBookmark &bookmark )
{
  // check for duplicate ID
  int i = 0;
  for ( const QgsBookmark &b : qgis::as_const( mBookmarks ) )
  {
    if ( b.id() == bookmark.id() )
    {
      mBookmarks[i] = bookmark;
      emit bookmarkChanged( bookmark.id() );
      store();
      return true;
    }
    i++;
  }
  return false;
}

void QgsBookmarkManager::clear()
{
  const QList< QgsBookmark > bookmarks = mBookmarks;
  for ( const QgsBookmark &b : bookmarks )
  {
    removeBookmark( b.id() );
  }
}

QList<QgsBookmark> QgsBookmarkManager::bookmarks() const
{
  return mBookmarks;
}

QgsBookmark QgsBookmarkManager::bookmarkById( const QString &id ) const
{
  for ( const QgsBookmark &b : mBookmarks )
  {
    if ( b.id() == id )
      return b;
  }
  return QgsBookmark();
}

bool QgsBookmarkManager::readXml( const QDomElement &element, const QDomDocument &doc )
{
  clear();

  QDomElement bookmarksElem = element;
  if ( element.tagName() != QStringLiteral( "Bookmarks" ) )
  {
    bookmarksElem = element.firstChildElement( QStringLiteral( "Bookmarks" ) );
  }
  bool result = true;
  if ( mProject && bookmarksElem.isNull() )
  {
    // handle legacy projects
    const int count = mProject->readNumEntry( QStringLiteral( "Bookmarks" ), QStringLiteral( "/count" ) );
    for ( int i = 0; i < count; ++i )
    {
      const double minX = mProject->readDoubleEntry( QStringLiteral( "Bookmarks" ), QStringLiteral( "/Row-%1/MinX" ).arg( i ) );
      const double minY = mProject->readDoubleEntry( QStringLiteral( "Bookmarks" ), QStringLiteral( "/Row-%1/MinY" ).arg( i ) );
      const double maxX = mProject->readDoubleEntry( QStringLiteral( "Bookmarks" ), QStringLiteral( "/Row-%1/MaxX" ).arg( i ) );
      const double maxY = mProject->readDoubleEntry( QStringLiteral( "Bookmarks" ), QStringLiteral( "/Row-%1/MaxY" ).arg( i ) );
      const long srid = mProject->readNumEntry( QStringLiteral( "Bookmarks" ), QStringLiteral( "/Row-%1/SRID" ).arg( i ) );
      QgsBookmark b;
      b.setId( QStringLiteral( "bookmark_%1" ).arg( i ) );
      b.setName( mProject->readEntry( QStringLiteral( "Bookmarks" ), QStringLiteral( "/Row-%1/Name" ).arg( i ) ) );
      b.setExtent( QgsReferencedRectangle( QgsRectangle( minX, minY, maxX, maxY ), QgsCoordinateReferenceSystem::fromSrsId( srid ) ) );

      bool added = false;
      addBookmark( b, &added );
      result = added && result;
    }
    return result;
  }

  //restore each
  QDomNodeList bookmarkNodes = element.elementsByTagName( QStringLiteral( "Bookmark" ) );
  for ( int i = 0; i < bookmarkNodes.size(); ++i )
  {
    QgsBookmark b = QgsBookmark::fromXml( bookmarkNodes.at( i ).toElement(), doc );
    bool added = false;
    addBookmark( b, &added );
    result = added && result;
  }

  return result;
}

QDomElement QgsBookmarkManager::writeXml( QDomDocument &doc ) const
{
  QDomElement bookmarksElem = doc.createElement( QStringLiteral( "Bookmarks" ) );

  for ( const QgsBookmark &b : mBookmarks )
  {
    QDomElement bookmarkElem = b.writeXml( doc );
    bookmarksElem.appendChild( bookmarkElem );
  }
  return bookmarksElem;
}

void QgsBookmarkManager::store()
{
  if ( mProject )
  {
    mProject->setDirty( true );
  }
  else if ( !mSettingKey.isEmpty() )
  {
    // yeah, fine, whatever... storing xml in settings IS kinda ugly. But the alternative
    // is a second marshalling implementation designed just for that, and I'd rather avoid the
    // duplicate code. Plus, this is all internal anyway... just use the public stable API and don't
    // concern your pretty little mind about all these technical details.
    QDomDocument textDoc;
    QDomElement textElem = writeXml( textDoc );
    textDoc.appendChild( textElem );
    QgsSettings().setValue( mSettingKey, textDoc.toString(), QgsSettings::Core );
  }
}

void QgsBookmarkManager::convertOldBookmarks()
{
  if ( !mNeedToConvertOldBookmarks )
    return;

  //convert old bookmarks from db
  sqlite3_database_unique_ptr database;
  int result = database.open( QgsApplication::qgisUserDatabaseFilePath() );
  if ( result != SQLITE_OK )
  {
    return;
  }

  sqlite3_statement_unique_ptr preparedStatement = database.prepare( QStringLiteral( "SELECT name,project_name,xmin,ymin,xmax,ymax,projection_srid FROM tbl_bookmarks" ), result );
  if ( result == SQLITE_OK )
  {
    while ( preparedStatement.step() == SQLITE_ROW )
    {
      const QString name = preparedStatement.columnAsText( 0 );
      const QString group = preparedStatement.columnAsText( 1 );
      const double xMin = preparedStatement.columnAsDouble( 2 );
      const double yMin = preparedStatement.columnAsDouble( 3 );
      const double xMax = preparedStatement.columnAsDouble( 4 );
      const double yMax = preparedStatement.columnAsDouble( 5 );
      const long long srid = preparedStatement.columnAsInt64( 6 );

      QgsBookmark b;
      b.setName( name );
      const QgsRectangle extent( xMin, yMin, xMax, yMax );
      b.setExtent( QgsReferencedRectangle( extent, QgsCoordinateReferenceSystem::fromSrsId( srid ) ) );
      addBookmark( b );
    }
  }
}
