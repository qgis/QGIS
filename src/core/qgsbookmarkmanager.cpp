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
#include <QUuid>

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

QgsBookmarkManager::~QgsBookmarkManager()
{
  clear();
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
  mProject->setDirty( true );
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
  mProject->setDirty( true );
  return true;
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
  if ( bookmarksElem.isNull() )
  {
    // handle legacy projects
    const int count = QgsProject::instance()->readNumEntry( QStringLiteral( "Bookmarks" ), QStringLiteral( "/count" ) );
    for ( int i = 0; i < count; ++i )
    {
      const double minX = QgsProject::instance()->readDoubleEntry( QStringLiteral( "Bookmarks" ), QStringLiteral( "/Row-%1/MinX" ).arg( i ) );
      const double minY = QgsProject::instance()->readDoubleEntry( QStringLiteral( "Bookmarks" ), QStringLiteral( "/Row-%1/MinY" ).arg( i ) );
      const double maxX = QgsProject::instance()->readDoubleEntry( QStringLiteral( "Bookmarks" ), QStringLiteral( "/Row-%1/MaxX" ).arg( i ) );
      const double maxY = QgsProject::instance()->readDoubleEntry( QStringLiteral( "Bookmarks" ), QStringLiteral( "/Row-%1/MaxY" ).arg( i ) );
      const long srid = QgsProject::instance()->readNumEntry( QStringLiteral( "Bookmarks" ), QStringLiteral( "/Row-%1/SRID" ).arg( i ) );
      QgsBookmark b;
      b.setId( QStringLiteral( "bookmark_%1" ).arg( i ) );
      b.setName( QgsProject::instance()->readEntry( QStringLiteral( "Bookmarks" ), QStringLiteral( "/Row-%1/Name" ).arg( i ) ) );
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
