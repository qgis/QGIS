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
#include <QTextStream>
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
  b.setGroup( element.attribute( QStringLiteral( "group" ) ) );
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
  bookmarkElem.setAttribute( QStringLiteral( "group" ), mGroup );
  bookmarkElem.setAttribute( QStringLiteral( "extent" ), mExtent.asWktPolygon() );
  mExtent.crs().writeXml( bookmarkElem, doc );
  return bookmarkElem;
}

bool QgsBookmark::operator==( const QgsBookmark &other ) const
{
  return mId == other.mId && mName == other.mName && mExtent == other.mExtent && mGroup == other.mGroup;
}

bool QgsBookmark::operator!=( const QgsBookmark &other ) const
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

QString QgsBookmark::group() const
{
  return mGroup;
}

void QgsBookmark::setGroup( const QString &group )
{
  mGroup = group;
}

QgsReferencedRectangle QgsBookmark::extent() const
{
  return mExtent;
}

void QgsBookmark::setExtent( const QgsReferencedRectangle &extent )
{
  mExtent = extent;
}


//
// QgsBookmarkManager
//

QgsBookmarkManager *QgsBookmarkManager::createProjectBasedManager( QgsProject *project )
{
  QgsBookmarkManager *res = new QgsBookmarkManager( project );
  res->mProject = project;
  return res;
}

QgsBookmarkManager::QgsBookmarkManager( QObject *parent )
  : QObject( parent )
{
  // we defer actually loading bookmarks until initialize() is called..
}

QgsBookmarkManager::~QgsBookmarkManager()
{
  store();
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
    for ( const QgsBookmark &b : std::as_const( mBookmarks ) )
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
  if ( !mGroups.contains( bookmark.group() ) )
    mGroups << bookmark.group();
  emit bookmarkAdded( bookmark.id() );
  if ( mProject )
  {
    mProject->setDirty( true );
  }

  return bookmark.id();
}

bool QgsBookmarkManager::removeBookmark( const QString &id )
{
  if ( id.isEmpty() )
    return false;

  QString group;
  int pos = -1;
  int i = 0;
  for ( const QgsBookmark &b : std::as_const( mBookmarks ) )
  {
    if ( b.id() == id )
    {
      group = b.group();
      pos = i;
      break;
    }
    i++;
  }

  if ( pos < 0 )
    return false;

  emit bookmarkAboutToBeRemoved( id );
  mBookmarks.removeAt( pos );
  if ( bookmarksByGroup( group ).isEmpty() )
    mGroups.removeOne( group );
  emit bookmarkRemoved( id );
  if ( mProject )
  {
    mProject->setDirty( true );
  }

  return true;
}

bool QgsBookmarkManager::updateBookmark( const QgsBookmark &bookmark )
{
  // check for duplicate ID
  int i = 0;
  for ( const QgsBookmark &b : std::as_const( mBookmarks ) )
  {
    if ( b.id() == bookmark.id() )
    {
      if ( mBookmarks[i].group() != bookmark.group() )
      {
        if ( bookmarksByGroup( mBookmarks[i].group() ).count() == 1 )
          mGroups.removeOne( mBookmarks[i].group() );
        if ( !mGroups.contains( bookmark.group() ) )
          mGroups << bookmark.group();
      }
      mBookmarks[i] = bookmark;
      emit bookmarkChanged( bookmark.id() );
      if ( mProject )
      {
        mProject->setDirty( true );
      }

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

QStringList QgsBookmarkManager::groups() const
{
  return mGroups;
}

void QgsBookmarkManager::renameGroup( const QString &oldName, const QString &newName )
{
  for ( int i = 0; i < mBookmarks.count(); ++i )
  {
    if ( mBookmarks.at( i ).group() == oldName )
    {
      mBookmarks[ i ].setGroup( newName );
      emit bookmarkChanged( mBookmarks.at( i ).id() );
    }
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

QList<QgsBookmark> QgsBookmarkManager::bookmarksByGroup( const QString &group )
{
  QList<QgsBookmark> bookmarks;
  for ( const QgsBookmark &b : mBookmarks )
  {
    if ( b.group() == group )
      bookmarks << b;
  }
  return bookmarks;
}

bool QgsBookmarkManager::readXml( const QDomElement &element, const QDomDocument &doc )
{
  clear();

  QDomElement bookmarksElem = element;
  if ( element.tagName() != QLatin1String( "Bookmarks" ) )
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

bool QgsBookmarkManager::moveBookmark( const QString &id, QgsBookmarkManager *destination )
{
  QgsBookmark b = bookmarkById( id );
  if ( b.id().isEmpty() )
    return false;

  removeBookmark( id );
  bool ok = false;
  destination->addBookmark( b, &ok );
  return ok;
}

bool QgsBookmarkManager::exportToFile( const QString &path, const QList<const QgsBookmarkManager *> &managers, const QString &group )
{
  // note - we don't use the other writeXml implementation, to maintain older format compatibility
  QDomDocument doc( QStringLiteral( "qgis_bookmarks" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgis_bookmarks" ) );
  doc.appendChild( root );

  QList<QString> headerList;
  headerList
      << QStringLiteral( "project" )
      << QStringLiteral( "xmin" )
      << QStringLiteral( "ymin" )
      << QStringLiteral( "xmax" )
      << QStringLiteral( "ymax" )
      << QStringLiteral( "sr_id" );

  for ( const QgsBookmarkManager *manager : managers )
  {
    const QList< QgsBookmark > bookmarks = manager->bookmarks();
    for ( const QgsBookmark &b : bookmarks )
    {
      if ( !group.isEmpty() && b.group() != group )
        continue;

      QDomElement bookmark = doc.createElement( QStringLiteral( "bookmark" ) );
      root.appendChild( bookmark );

      QDomElement id = doc.createElement( QStringLiteral( "id" ) );
      id.appendChild( doc.createTextNode( b.id() ) );
      bookmark.appendChild( id );

      QDomElement name = doc.createElement( QStringLiteral( "name" ) );
      name.appendChild( doc.createTextNode( b.name() ) );
      bookmark.appendChild( name );

      QDomElement group = doc.createElement( QStringLiteral( "project" ) );
      group.appendChild( doc.createTextNode( b.group() ) );
      bookmark.appendChild( group );

      QDomElement xMin = doc.createElement( QStringLiteral( "xmin" ) );
      xMin.appendChild( doc.createTextNode( qgsDoubleToString( b.extent().xMinimum() ) ) );
      bookmark.appendChild( xMin );
      QDomElement yMin = doc.createElement( QStringLiteral( "ymin" ) );
      yMin.appendChild( doc.createTextNode( qgsDoubleToString( b.extent().yMinimum() ) ) );
      bookmark.appendChild( yMin );
      QDomElement xMax = doc.createElement( QStringLiteral( "xmax" ) );
      xMax.appendChild( doc.createTextNode( qgsDoubleToString( b.extent().xMaximum() ) ) );
      bookmark.appendChild( xMax );
      QDomElement yMax = doc.createElement( QStringLiteral( "ymax" ) );
      yMax.appendChild( doc.createTextNode( qgsDoubleToString( b.extent().yMaximum() ) ) );
      bookmark.appendChild( yMax );

      QDomElement crs = doc.createElement( QStringLiteral( "sr_id" ) );
      crs.appendChild( doc.createTextNode( QString::number( b.extent().crs().srsid() ) ) );
      bookmark.appendChild( crs );
    }
  }

  QFile f( path );
  if ( !f.open( QFile::WriteOnly | QIODevice::Truncate ) )
  {
    f.close();
    return false;
  }

  QTextStream out( &f );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  out.setCodec( "UTF-8" );
#endif
  doc.save( out, 2 );
  f.close();

  return true;
}

bool QgsBookmarkManager::importFromFile( const QString &path )
{
  if ( path.isEmpty() )
  {
    return false;
  }

  QFile f( path );
  if ( !f.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    return false;
  }

  QDomDocument doc;
  if ( !doc.setContent( &f ) )
  {
    return false;
  }
  f.close();

  QDomElement docElem = doc.documentElement();
  QDomNodeList nodeList = docElem.elementsByTagName( QStringLiteral( "bookmark" ) );

  bool res = true;
  for ( int i = 0; i < nodeList.count(); i++ )
  {
    QDomNode bookmark = nodeList.at( i );
    QDomElement name = bookmark.firstChildElement( QStringLiteral( "name" ) );
    QDomElement prjname = bookmark.firstChildElement( QStringLiteral( "project" ) );
    QDomElement xmin = bookmark.firstChildElement( QStringLiteral( "xmin" ) );
    QDomElement ymin = bookmark.firstChildElement( QStringLiteral( "ymin" ) );
    QDomElement xmax = bookmark.firstChildElement( QStringLiteral( "xmax" ) );
    QDomElement ymax = bookmark.firstChildElement( QStringLiteral( "ymax" ) );
    QDomElement srid = bookmark.firstChildElement( QStringLiteral( "sr_id" ) );

    bool ok = false;
    QgsBookmark b;
    b.setName( name.text() );
    b.setGroup( prjname.text() );
    QgsCoordinateReferenceSystem crs;
    crs.createFromSrsId( srid.text().toLongLong() );
    b.setExtent( QgsReferencedRectangle( QgsRectangle( xmin.text().toDouble(),
                                         ymin.text().toDouble(),
                                         xmax.text().toDouble(),
                                         ymax.text().toDouble() ), crs ) );
    addBookmark( b, &ok );
    res = res && ok;
  }

  return res;
}

void QgsBookmarkManager::store()
{
  if ( !mFilePath.isEmpty() )
  {
    QFile f( mFilePath );
    if ( !f.open( QFile::WriteOnly | QIODevice::Truncate ) )
    {
      f.close();
      return;
    }

    QDomDocument doc;
    QDomElement elem = writeXml( doc );
    doc.appendChild( elem );

    QTextStream out( &f );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec( "UTF-8" );
#endif
    doc.save( out, 2 );
    f.close();
  }
}

void QgsBookmarkManager::initialize( const QString &filePath )
{
  if ( mInitialized )
    return;

  mFilePath = filePath;

  mInitialized = true;

  // restore state
  if ( !QFileInfo::exists( mFilePath ) )
  {
    //convert old bookmarks from db
    sqlite3_database_unique_ptr database;
    int result = database.open( QgsApplication::qgisUserDatabaseFilePath() );
    if ( result != SQLITE_OK )
    {
      return;
    }

    sqlite3_statement_unique_ptr preparedStatement = database.prepare( QStringLiteral( "SELECT name,xmin,ymin,xmax,ymax,projection_srid FROM tbl_bookmarks" ), result );
    if ( result == SQLITE_OK )
    {
      while ( preparedStatement.step() == SQLITE_ROW )
      {
        const QString name = preparedStatement.columnAsText( 0 );
        const double xMin = preparedStatement.columnAsDouble( 1 );
        const double yMin = preparedStatement.columnAsDouble( 2 );
        const double xMax = preparedStatement.columnAsDouble( 3 );
        const double yMax = preparedStatement.columnAsDouble( 4 );
        const long long srid = preparedStatement.columnAsInt64( 5 );

        QgsBookmark b;
        b.setName( name );
        const QgsRectangle extent( xMin, yMin, xMax, yMax );
        b.setExtent( QgsReferencedRectangle( extent, QgsCoordinateReferenceSystem::fromSrsId( srid ) ) );
        addBookmark( b );
      }
    }
    store();
  }
  else
  {
    QFile f( mFilePath );
    if ( !f.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      return;
    }

    QDomDocument doc;
    if ( !doc.setContent( &f ) )
    {
      return;
    }
    f.close();

    QDomElement elem = doc.documentElement();
    readXml( elem, doc );
  }
}
