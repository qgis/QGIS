/***************************************************************************
    qgsstyle.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstyle.h"

#include "qgssymbol.h"
#include "qgscolorramp.h"
#include "qgssymbollayerregistry.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsreadwritecontext.h"
#include "qgssettings.h"

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>
#include <QFile>
#include <QTextStream>
#include <QByteArray>

#include <sqlite3.h>
#include "qgssqliteutils.h"

#define STYLE_CURRENT_VERSION  "1"

QgsStyle *QgsStyle::sDefaultStyle = nullptr;

QgsStyle::~QgsStyle()
{
  clear();
}

QgsStyle *QgsStyle::defaultStyle() // static
{
  if ( !sDefaultStyle )
  {
    QString styleFilename = QgsApplication::userStylePath();

    // copy default style if user style doesn't exist
    if ( !QFile::exists( styleFilename ) )
    {
      sDefaultStyle = new QgsStyle;
      sDefaultStyle->createDatabase( styleFilename );
      if ( QFile::exists( QgsApplication::defaultStylePath() ) )
      {
        sDefaultStyle->importXml( QgsApplication::defaultStylePath() );
      }
    }
    else
    {
      sDefaultStyle = new QgsStyle;
      sDefaultStyle->load( styleFilename );
    }
  }
  return sDefaultStyle;
}


void QgsStyle::clear()
{
  qDeleteAll( mSymbols );
  qDeleteAll( mColorRamps );

  mSymbols.clear();
  mColorRamps.clear();
}

bool QgsStyle::addSymbol( const QString &name, QgsSymbol *symbol, bool update )
{
  if ( !symbol || name.isEmpty() )
    return false;

  // delete previous symbol (if any)
  if ( mSymbols.contains( name ) )
  {
    // TODO remove groups and tags?
    delete mSymbols.value( name );
    mSymbols.insert( name, symbol );
    if ( update )
      updateSymbol( SymbolEntity, name );
  }
  else
  {
    mSymbols.insert( name, symbol );
    if ( update )
      saveSymbol( name, symbol, false, QStringList() );
  }

  return true;
}

bool QgsStyle::saveSymbol( const QString &name, QgsSymbol *symbol, bool favorite, const QStringList &tags )
{
  // TODO add support for groups
  QDomDocument doc( QStringLiteral( "dummy" ) );
  QDomElement symEl = QgsSymbolLayerUtils::saveSymbol( name, symbol, doc, QgsReadWriteContext() );
  if ( symEl.isNull() )
  {
    QgsDebugMsg( "Couldn't convert symbol to valid XML!" );
    return false;
  }

  QByteArray xmlArray;
  QTextStream stream( &xmlArray );
  stream.setCodec( "UTF-8" );
  symEl.save( stream, 4 );
  char *query = sqlite3_mprintf( "INSERT INTO symbol VALUES (NULL, '%q', '%q', %d);",
                                 name.toUtf8().constData(), xmlArray.constData(), ( favorite ? 1 : 0 ) );

  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( "Couldn't insert symbol into the database!" );
    return false;
  }

  tagSymbol( SymbolEntity, name, tags );

  emit symbolSaved( name, symbol );

  return true;
}

bool QgsStyle::removeSymbol( const QString &name )
{
  QgsSymbol *symbol = mSymbols.take( name );
  if ( !symbol )
    return false;

  // remove from map and delete
  delete symbol;

  // TODO
  // Simplify this work here, its STUPID to run two DB queries for the sake of remove()
  if ( !mCurrentDB )
  {
    QgsDebugMsg( "Sorry! Cannot open database to tag." );
    return false;
  }

  int symbolid = symbolId( name );
  if ( !symbolid )
  {
    QgsDebugMsg( "No such symbol for deleting in database: " + name + ". Cheers." );
  }

  remove( SymbolEntity, symbolid );

  return true;
}

QgsSymbol *QgsStyle::symbol( const QString &name )
{
  const QgsSymbol *symbol = symbolRef( name );
  return symbol ? symbol->clone() : nullptr;
}

const QgsSymbol *QgsStyle::symbolRef( const QString &name ) const
{
  return mSymbols.value( name );
}

int QgsStyle::symbolCount()
{
  return mSymbols.count();
}

QStringList QgsStyle::symbolNames()
{
  return mSymbols.keys();
}


bool QgsStyle::addColorRamp( const QString &name, QgsColorRamp *colorRamp, bool update )
{
  if ( !colorRamp || name.isEmpty() )
    return false;

  // delete previous color ramps (if any)
  if ( mColorRamps.contains( name ) )
  {
    // TODO remove groups and tags?
    delete mColorRamps.value( name );
    mColorRamps.insert( name, colorRamp );
    if ( update )
      updateSymbol( ColorrampEntity, name );
  }
  else
  {
    mColorRamps.insert( name, colorRamp );
    if ( update )
      saveColorRamp( name, colorRamp, false, QStringList() );
  }

  return true;
}

bool QgsStyle::saveColorRamp( const QString &name, QgsColorRamp *ramp, bool favorite, const QStringList &tags )
{
  // insert it into the database
  QDomDocument doc( QStringLiteral( "dummy" ) );
  QDomElement rampEl = QgsSymbolLayerUtils::saveColorRamp( name, ramp, doc );

  if ( rampEl.isNull() )
  {
    QgsDebugMsg( "Couldn't convert color ramp to valid XML!" );
    return false;
  }

  QByteArray xmlArray;
  QTextStream stream( &xmlArray );
  stream.setCodec( "UTF-8" );
  rampEl.save( stream, 4 );
  char *query = sqlite3_mprintf( "INSERT INTO colorramp VALUES (NULL, '%q', '%q', %d);",
                                 name.toUtf8().constData(), xmlArray.constData(), ( favorite ? 1 : 0 ) );
  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( "Couldn't insert colorramp into the database!" );
    return false;
  }

  tagSymbol( ColorrampEntity, name, tags );

  return true;
}

bool QgsStyle::removeColorRamp( const QString &name )
{
  QgsColorRamp *ramp = mColorRamps.take( name );
  if ( !ramp )
    return false;

  char *query = sqlite3_mprintf( "DELETE FROM colorramp WHERE name='%q'", name.toUtf8().constData() );
  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( "Couldn't remove color ramp from the database." );
    return false;
  }

  delete ramp;

  return true;
}

QgsColorRamp *QgsStyle::colorRamp( const QString &name ) const
{
  const QgsColorRamp *ramp = colorRampRef( name );
  return ramp ? ramp->clone() : nullptr;
}

const QgsColorRamp *QgsStyle::colorRampRef( const QString &name ) const
{
  return mColorRamps.value( name );
}

int QgsStyle::colorRampCount()
{
  return mColorRamps.count();
}

QStringList QgsStyle::colorRampNames()
{
  return mColorRamps.keys();
}

bool QgsStyle::openDatabase( const QString &filename )
{
  int rc = mCurrentDB.open( filename );
  if ( rc )
  {
    mErrorString = QStringLiteral( "Couldn't open the style database: %1" ).arg( mCurrentDB.errorMessage() );
    return false;
  }

  return true;
}

bool QgsStyle::createDatabase( const QString &filename )
{
  mErrorString.clear();
  if ( !openDatabase( filename ) )
  {
    mErrorString = QStringLiteral( "Unable to create database" );
    QgsDebugMsg( mErrorString );
    return false;
  }

  createTables();

  return true;
}

bool QgsStyle::createMemoryDatabase()
{
  mErrorString.clear();
  if ( !openDatabase( QStringLiteral( ":memory:" ) ) )
  {
    mErrorString = QStringLiteral( "Unable to create temporary memory database" );
    QgsDebugMsg( mErrorString );
    return false;
  }

  createTables();

  return true;
}

void QgsStyle::createTables()
{
  char *query = sqlite3_mprintf( "CREATE TABLE symbol("\
                                 "id INTEGER PRIMARY KEY,"\
                                 "name TEXT UNIQUE,"\
                                 "xml TEXT,"\
                                 "favorite INTEGER);"\
                                 "CREATE TABLE colorramp("\
                                 "id INTEGER PRIMARY KEY,"\
                                 "name TEXT UNIQUE,"\
                                 "xml TEXT,"\
                                 "favorite INTEGER);"\
                                 "CREATE TABLE tag("\
                                 "id INTEGER PRIMARY KEY,"\
                                 "name TEXT);"\
                                 "CREATE TABLE tagmap("\
                                 "tag_id INTEGER NOT NULL,"\
                                 "symbol_id INTEGER);"\
                                 "CREATE TABLE ctagmap("\
                                 "tag_id INTEGER NOT NULL,"\
                                 "colorramp_id INTEGER);"\
                                 "CREATE TABLE smartgroup("\
                                 "id INTEGER PRIMARY KEY,"\
                                 "name TEXT,"\
                                 "xml TEXT);" );
  runEmptyQuery( query );
}

bool QgsStyle::load( const QString &filename )
{
  mErrorString.clear();

  // Open the sqlite database
  if ( !openDatabase( filename ) )
  {
    mErrorString = QStringLiteral( "Unable to open database file specified" );
    QgsDebugMsg( mErrorString );
    return false;
  }

  // Make sure there are no Null fields in parenting symbols and groups
  char *query = sqlite3_mprintf( "UPDATE symbol SET favorite=0 WHERE favorite IS NULL;"
                                 "UPDATE colorramp SET favorite=0 WHERE favorite IS NULL;"
                               );
  runEmptyQuery( query );

  // First create all the main symbols
  query = sqlite3_mprintf( "SELECT * FROM symbol" );

  sqlite3_statement_unique_ptr statement;
  int rc;
  statement = mCurrentDB.prepare( query, rc );

  while ( rc == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    QDomDocument doc;
    QString symbol_name = statement.columnAsText( SymbolName );
    QString xmlstring = statement.columnAsText( SymbolXML );
    if ( !doc.setContent( xmlstring ) )
    {
      QgsDebugMsg( "Cannot open symbol " + symbol_name );
      continue;
    }

    QDomElement symElement = doc.documentElement();
    QgsSymbol *symbol = QgsSymbolLayerUtils::loadSymbol( symElement, QgsReadWriteContext() );
    if ( symbol )
      mSymbols.insert( symbol_name, symbol );
  }

  query = sqlite3_mprintf( "SELECT * FROM colorramp" );
  statement = mCurrentDB.prepare( query, rc );
  while ( rc == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    QDomDocument doc;
    QString ramp_name = statement.columnAsText( ColorrampName );
    QString xmlstring = statement.columnAsText( ColorrampXML );
    if ( !doc.setContent( xmlstring ) )
    {
      QgsDebugMsg( "Cannot open symbol " + ramp_name );
      continue;
    }
    QDomElement rampElement = doc.documentElement();
    QgsColorRamp *ramp = QgsSymbolLayerUtils::loadColorRamp( rampElement );
    if ( ramp )
      mColorRamps.insert( ramp_name, ramp );
  }

  mFileName = filename;
  return true;
}



bool QgsStyle::save( QString filename )
{
  mErrorString.clear();

  if ( filename.isEmpty() )
    filename = mFileName;

  // TODO evaluate the requirement of this function and change implementation accordingly
  // TODO remove QEXPECT_FAIL from TestStyle::testSaveLoad() when done
#if 0
  QDomDocument doc( "qgis_style" );
  QDomElement root = doc.createElement( "qgis_style" );
  root.setAttribute( "version", STYLE_CURRENT_VERSION );
  doc.appendChild( root );

  QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( mSymbols, "symbols", doc );

  QDomElement rampsElem = doc.createElement( "colorramps" );

  // save color ramps
  for ( QMap<QString, QgsColorRamp *>::iterator itr = mColorRamps.begin(); itr != mColorRamps.end(); ++itr )
  {
    QDomElement rampEl = QgsSymbolLayerUtils::saveColorRamp( itr.key(), itr.value(), doc );
    rampsElem.appendChild( rampEl );
  }

  root.appendChild( symbolsElem );
  root.appendChild( rampsElem );

  // save
  QFile f( filename );
  if ( !f.open( QFile::WriteOnly ) )
  {
    mErrorString = "Couldn't open file for writing: " + filename;
    return false;
  }
  QTextStream ts( &f );
  ts.setCodec( "UTF-8" );
  doc.save( ts, 2 );
  f.close();
#endif

  mFileName = filename;
  return true;
}

bool QgsStyle::renameSymbol( const QString &oldName, const QString &newName )
{
  if ( mSymbols.contains( newName ) )
  {
    QgsDebugMsg( "Symbol of new name already exists" );
    return false;
  }

  QgsSymbol *symbol = mSymbols.take( oldName );
  if ( !symbol )
    return false;

  mSymbols.insert( newName, symbol );

  if ( !mCurrentDB )
  {
    QgsDebugMsg( "Sorry! Cannot open database to tag." );
    return false;
  }

  int symbolid = symbolId( oldName );
  if ( !symbolid )
  {
    QgsDebugMsg( "No such symbol for tagging in database: " + oldName );
    return false;
  }

  rename( SymbolEntity, symbolid, newName );

  return true;
}

bool QgsStyle::renameColorRamp( const QString &oldName, const QString &newName )
{
  if ( mColorRamps.contains( newName ) )
  {
    QgsDebugMsg( "Color ramp of new name already exists." );
    return false;
  }

  QgsColorRamp *ramp = mColorRamps.take( oldName );
  if ( !ramp )
    return false;

  mColorRamps.insert( newName, ramp );

  int rampid = 0;
  sqlite3_statement_unique_ptr statement;
  char *query = sqlite3_mprintf( "SELECT id FROM colorramp WHERE name='%q'", oldName.toUtf8().constData() );
  int nErr;
  statement = mCurrentDB.prepare( query, nErr );
  if ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    rampid = sqlite3_column_int( statement.get(), 0 );
  }
  rename( ColorrampEntity, rampid, newName );

  return true;
}

QStringList QgsStyle::symbolsOfFavorite( StyleEntity type ) const
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( QString( "Cannot Open database for getting favorite symbols" ) );
    return QStringList();
  }

  char *query = nullptr;
  if ( type == SymbolEntity )
  {
    query = sqlite3_mprintf( "SELECT name FROM symbol WHERE favorite=1" );
  }
  else if ( type == ColorrampEntity )
  {
    query = sqlite3_mprintf( "SELECT name FROM colorramp WHERE favorite=1" );
  }
  else
  {
    QgsDebugMsg( "No such style entity" );
    return QStringList();
  }

  int nErr;
  sqlite3_statement_unique_ptr statement;
  statement = mCurrentDB.prepare( query, nErr );

  QStringList symbols;
  while ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    symbols << statement.columnAsText( 0 );
  }

  return symbols;
}

QStringList QgsStyle::symbolsWithTag( StyleEntity type, int tagid ) const
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( QString( "Cannot open database to get symbols of tagid %1" ).arg( tagid ) );
    return QStringList();
  }

  char *subquery = nullptr;
  if ( type == SymbolEntity )
  {
    subquery = sqlite3_mprintf( "SELECT symbol_id FROM tagmap WHERE tag_id=%d", tagid );
  }
  else if ( type == ColorrampEntity )
  {
    subquery = sqlite3_mprintf( "SELECT colorramp_id FROM ctagmap WHERE tag_id=%d", tagid );
  }
  else
  {
    QgsDebugMsg( "Unknown Entity" );
    return QStringList();
  }

  int nErr;
  sqlite3_statement_unique_ptr statement;
  statement = mCurrentDB.prepare( subquery, nErr );

  // get the symbol <-> tag connection from table 'tagmap'/'ctagmap'
  QStringList symbols;
  while ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    int id = sqlite3_column_int( statement.get(), 0 );

    char *query = type == SymbolEntity
                  ? sqlite3_mprintf( "SELECT name FROM symbol WHERE id=%d", id )
                  : sqlite3_mprintf( "SELECT name FROM colorramp WHERE id=%d", id );

    int rc;
    sqlite3_statement_unique_ptr statement2;
    statement2 = mCurrentDB.prepare( query, rc );
    while ( rc == SQLITE_OK && sqlite3_step( statement2.get() ) == SQLITE_ROW )
    {
      symbols << QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( statement2.get(), 0 ) ) );
    }
  }

  return symbols;
}

int QgsStyle::addTag( const QString &tagname )
{
  if ( !mCurrentDB )
    return 0;
  sqlite3_statement_unique_ptr statement;

  char *query = sqlite3_mprintf( "INSERT INTO tag VALUES (NULL, '%q')", tagname.toUtf8().constData() );
  int nErr;
  statement = mCurrentDB.prepare( query, nErr );
  if ( nErr == SQLITE_OK )
    ( void )sqlite3_step( statement.get() );

  QgsSettings settings;
  settings.setValue( QStringLiteral( "qgis/symbolsListGroupsIndex" ), 0 );

  emit groupsModified();

  return static_cast< int >( sqlite3_last_insert_rowid( mCurrentDB.get() ) );
}

QStringList QgsStyle::tags() const
{
  if ( !mCurrentDB )
    return QStringList();

  sqlite3_statement_unique_ptr statement;

  char *query = sqlite3_mprintf( "SELECT name FROM tag" );
  int nError;
  statement = mCurrentDB.prepare( query, nError );

  QStringList tagList;
  while ( nError == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    tagList << statement.columnAsText( 0 );
  }

  return tagList;
}

void QgsStyle::rename( StyleEntity type, int id, const QString &newName )
{
  bool groupRenamed = false;
  char *query = nullptr;
  switch ( type )
  {
    case SymbolEntity:
      query = sqlite3_mprintf( "UPDATE symbol SET name='%q' WHERE id=%d", newName.toUtf8().constData(), id );
      break;
    case ColorrampEntity:
      query = sqlite3_mprintf( "UPDATE colorramp SET name='%q' WHERE id=%d", newName.toUtf8().constData(), id );
      break;
    case TagEntity:
      query = sqlite3_mprintf( "UPDATE tag SET name='%q' WHERE id=%d", newName.toUtf8().constData(), id );
      groupRenamed = true;
      break;
    case SmartgroupEntity:
      query = sqlite3_mprintf( "UPDATE smartgroup SET name='%q' WHERE id=%d", newName.toUtf8().constData(), id );
      groupRenamed = true;
      break;
    default:
      QgsDebugMsg( "Invalid Style Entity indicated" );
      return;
  }
  if ( !runEmptyQuery( query ) )
  {
    mErrorString = QStringLiteral( "Could not rename!" );
  }
  else
  {
    if ( groupRenamed )
    {
      emit groupsModified();
    }
  }
}

void QgsStyle::remove( StyleEntity type, int id )
{
  bool groupRemoved = false;
  char *query = nullptr;
  switch ( type )
  {
    case SymbolEntity:
      query = sqlite3_mprintf( "DELETE FROM symbol WHERE id=%d; DELETE FROM tagmap WHERE symbol_id=%d", id, id );
      break;
    case ColorrampEntity:
      query = sqlite3_mprintf( "DELETE FROM colorramp WHERE id=%d", id );
      break;
    case TagEntity:
      query = sqlite3_mprintf( "DELETE FROM tag WHERE id=%d; DELETE FROM tagmap WHERE tag_id=%d", id, id );
      groupRemoved = true;
      break;
    case SmartgroupEntity:
      query = sqlite3_mprintf( "DELETE FROM smartgroup WHERE id=%d", id );
      groupRemoved = true;
      break;
    default:
      QgsDebugMsg( "Invalid Style Entity indicated" );
      return;
  }

  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( "Could not delete entity!" );
  }
  else
  {
    if ( groupRemoved )
    {
      QgsSettings settings;
      settings.setValue( QStringLiteral( "qgis/symbolsListGroupsIndex" ), 0 );

      emit groupsModified();
    }
  }
}

bool QgsStyle::runEmptyQuery( char *query, bool freeQuery )
{
  if ( !mCurrentDB )
    return false;

  char *zErr = nullptr;
  int nErr = sqlite3_exec( mCurrentDB.get(), query, nullptr, nullptr, &zErr );

  if ( freeQuery )
  {
    sqlite3_free( query );
  }

  if ( nErr != SQLITE_OK )
  {
    QgsDebugMsg( zErr );
    sqlite3_free( zErr );
  }

  return zErr == SQLITE_OK;
}

bool QgsStyle::addFavorite( StyleEntity type, const QString &name )
{
  char *query = nullptr;

  switch ( type )
  {
    case SymbolEntity:
      query = sqlite3_mprintf( "UPDATE symbol SET favorite=1 WHERE name='%q'", name.toUtf8().constData() );
      break;
    case ColorrampEntity:
      query = sqlite3_mprintf( "UPDATE colorramp SET favorite=1 WHERE name='%q'", name.toUtf8().constData() );
      break;

    default:
      QgsDebugMsg( "Wrong entity value. cannot apply group" );
      return false;
  }

  return runEmptyQuery( query );
}

bool QgsStyle::removeFavorite( StyleEntity type, const QString &name )
{
  char *query = nullptr;

  switch ( type )
  {
    case SymbolEntity:
      query = sqlite3_mprintf( "UPDATE symbol SET favorite=0 WHERE name='%q'", name.toUtf8().constData() );
      break;
    case ColorrampEntity:
      query = sqlite3_mprintf( "UPDATE colorramp SET favorite=0 WHERE name='%q'", name.toUtf8().constData() );
      break;

    default:
      QgsDebugMsg( "Wrong entity value. cannot apply group" );
      return false;
  }

  return runEmptyQuery( query );
}

QStringList QgsStyle::findSymbols( StyleEntity type, const QString &qword )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( "Sorry! Cannot open database to search" );
    return QStringList();
  }

  // first find symbols with matching name
  QString item = ( type == SymbolEntity ) ? QStringLiteral( "symbol" ) : QStringLiteral( "colorramp" );
  char *query = sqlite3_mprintf( "SELECT name FROM %q WHERE name LIKE '%%%q%%'",
                                 item.toUtf8().constData(), qword.toUtf8().constData() );

  sqlite3_statement_unique_ptr statement;
  int nErr; statement = mCurrentDB.prepare( query, nErr );

  QSet< QString > symbols;
  while ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    symbols << statement.columnAsText( 0 );
  }

  // next add symbols with matching tags
  query = sqlite3_mprintf( "SELECT id FROM tag WHERE name LIKE '%%%q%%'", qword.toUtf8().constData() );
  statement = mCurrentDB.prepare( query, nErr );

  QStringList tagids;
  while ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    tagids << QString::fromUtf8( ( const char * ) sqlite3_column_text( statement.get(), 0 ) );
  }

  QString dummy = tagids.join( QStringLiteral( ", " ) );

  if ( type == SymbolEntity )
  {
    query = sqlite3_mprintf( "SELECT symbol_id FROM tagmap WHERE tag_id IN (%q)",
                             dummy.toUtf8().constData() );
  }
  else
  {
    query = sqlite3_mprintf( "SELECT colorramp_id FROM ctagmap WHERE tag_id IN (%q)",
                             dummy.toUtf8().constData() );
  }
  statement = mCurrentDB.prepare( query, nErr );

  QStringList symbolids;
  while ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    symbolids << QString::fromUtf8( ( const char * ) sqlite3_column_text( statement.get(), 0 ) );
  }

  dummy = symbolids.join( QStringLiteral( ", " ) );
  query = sqlite3_mprintf( "SELECT name FROM %q  WHERE id IN (%q)",
                           item.toUtf8().constData(), dummy.toUtf8().constData() );
  statement = mCurrentDB.prepare( query, nErr );
  while ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    symbols << QString::fromUtf8( ( const char * ) sqlite3_column_text( statement.get(), 0 ) );
  }

  return symbols.toList();
}

bool QgsStyle::tagSymbol( StyleEntity type, const QString &symbol, const QStringList &tags )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( "Sorry! Cannot open database to tag." );
    return false;
  }

  int symbolid = type == SymbolEntity ? symbolId( symbol ) : colorrampId( symbol );
  if ( !symbolid )
  {
    QgsDebugMsg( "No such symbol for tagging in database: " + symbol );
    return false;
  }

  QString tag;
  Q_FOREACH ( const QString &t, tags )
  {
    tag = t.trimmed();
    if ( !tag.isEmpty() )
    {
      // sql: gets the id of the tag if present or insert the tag and get the id of the tag
      char *query = sqlite3_mprintf( "SELECT id FROM tag WHERE LOWER(name)='%q'", tag.toUtf8().toLower().constData() );

      sqlite3_statement_unique_ptr statement;
      int nErr; statement = mCurrentDB.prepare( query, nErr );

      int tagid;
      if ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
      {
        tagid = sqlite3_column_int( statement.get(), 0 );
      }
      else
      {
        tagid = addTag( tag );
      }

      // Now map the tag to the symbol if it's not already tagged
      if ( !symbolHasTag( type, symbol, tag ) )
      {
        query = type == SymbolEntity
                ? sqlite3_mprintf( "INSERT INTO tagmap VALUES (%d,%d)", tagid, symbolid )
                : sqlite3_mprintf( "INSERT INTO ctagmap VALUES (%d,%d)", tagid, symbolid );

        char *zErr = nullptr;
        nErr = sqlite3_exec( mCurrentDB.get(), query, nullptr, nullptr, &zErr );
        if ( nErr )
        {
          QgsDebugMsg( zErr );
        }
      }
    }
  }

  return true;
}

bool QgsStyle::detagSymbol( StyleEntity type, const QString &symbol, const QStringList &tags )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( "Sorry! Cannot open database for detgging." );
    return false;
  }

  char *query = type == SymbolEntity
                ? sqlite3_mprintf( "SELECT id FROM symbol WHERE name='%q'", symbol.toUtf8().constData() )
                : sqlite3_mprintf( "SELECT id FROM colorramp WHERE name='%q'", symbol.toUtf8().constData() );
  sqlite3_statement_unique_ptr statement;
  int nErr; statement = mCurrentDB.prepare( query, nErr );

  int symbolid = 0;
  if ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    symbolid = sqlite3_column_int( statement.get(), 0 );
  }
  else
  {
    return false;
  }

  Q_FOREACH ( const QString &tag, tags )
  {
    query = sqlite3_mprintf( "SELECT id FROM tag WHERE name='%q'", tag.toUtf8().constData() );

    sqlite3_statement_unique_ptr statement2;
    statement2 = mCurrentDB.prepare( query, nErr );

    int tagid = 0;
    if ( nErr == SQLITE_OK && sqlite3_step( statement2.get() ) == SQLITE_ROW )
    {
      tagid = sqlite3_column_int( statement2.get(), 0 );
    }

    if ( tagid )
    {
      // remove from the tagmap
      query = type == SymbolEntity
              ? sqlite3_mprintf( "DELETE FROM tagmap WHERE tag_id=%d AND symbol_id=%d", tagid, symbolid )
              : sqlite3_mprintf( "DELETE FROM ctagmap WHERE tag_id=%d AND colorramp_id=%d", tagid, symbolid );
      runEmptyQuery( query );
    }
  }

  // TODO Perform tag cleanup
  // check the number of entries for a given tag in the tagmap
  // if the count is 0, then remove( TagEntity, tagid )
  return true;
}

bool QgsStyle::detagSymbol( StyleEntity type, const QString &symbol )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( "Sorry! Cannot open database for detgging." );
    return false;
  }

  char *query = type == SymbolEntity
                ? sqlite3_mprintf( "SELECT id FROM symbol WHERE name='%q'", symbol.toUtf8().constData() )
                : sqlite3_mprintf( "SELECT id FROM colorramp WHERE name='%q'", symbol.toUtf8().constData() );
  sqlite3_statement_unique_ptr statement;
  int nErr;
  statement = mCurrentDB.prepare( query, nErr );

  int symbolid = 0;
  if ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    symbolid = sqlite3_column_int( statement.get(), 0 );
  }
  else
  {
    return false;
  }

  // remove all tags
  query = type == SymbolEntity
          ? sqlite3_mprintf( "DELETE FROM tagmap WHERE symbol_id=%d", symbolid )
          : sqlite3_mprintf( "DELETE FROM ctagmap WHERE colorramp_id=%d", symbolid );
  runEmptyQuery( query );

  // TODO Perform tag cleanup
  // check the number of entries for a given tag in the tagmap
  // if the count is 0, then remove( TagEntity, tagid )
  return true;
}

QStringList QgsStyle::tagsOfSymbol( StyleEntity type, const QString &symbol )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( "Sorry! Cannot open database for getting the tags." );
    return QStringList();
  }

  int symbolid = type == SymbolEntity ? symbolId( symbol ) : colorrampId( symbol );
  if ( !symbolid )
    return QStringList();

  // get the ids of tags for the symbol
  char *query = type == SymbolEntity
                ? sqlite3_mprintf( "SELECT tag_id FROM tagmap WHERE symbol_id=%d", symbolid )
                : sqlite3_mprintf( "SELECT tag_id FROM ctagmap WHERE colorramp_id=%d", symbolid );

  sqlite3_statement_unique_ptr statement;
  int nErr; statement = mCurrentDB.prepare( query, nErr );

  QStringList tagList;
  while ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    char *subquery = sqlite3_mprintf( "SELECT name FROM tag WHERE id=%d", sqlite3_column_int( statement.get(), 0 ) );

    sqlite3_statement_unique_ptr statement2;
    int pErr;
    statement2 = mCurrentDB.prepare( subquery, pErr );
    if ( pErr == SQLITE_OK && sqlite3_step( statement2.get() ) == SQLITE_ROW )
    {
      tagList << QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( statement2.get(), 0 ) ) );
    }
  }

  return tagList;
}

bool QgsStyle::symbolHasTag( StyleEntity type, const QString &symbol, const QString &tag )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( "Sorry! Cannot open database for getting the tags." );
    return false;
  }

  int symbolid = type == SymbolEntity ? symbolId( symbol ) : colorrampId( symbol );
  if ( !symbolid )
  {
    return false;
  }
  int tagid = tagId( tag );
  if ( !tagid )
  {
    return false;
  }

  // get the ids of tags for the symbol
  char *query = type == SymbolEntity
                ? sqlite3_mprintf( "SELECT tag_id FROM tagmap WHERE tag_id=%d AND symbol_id=%d", tagid, symbolid )
                : sqlite3_mprintf( "SELECT tag_id FROM ctagmap WHERE tag_id=%d AND colorramp_id=%d", tagid, symbolid );

  sqlite3_statement_unique_ptr statement;
  int nErr; statement = mCurrentDB.prepare( query, nErr );

  return ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW );
}

QString QgsStyle::tag( int id ) const
{
  if ( !mCurrentDB )
    return QString();

  sqlite3_statement_unique_ptr statement;

  char *query = sqlite3_mprintf( "SELECT name FROM tag WHERE id=%d", id );
  int nError;
  statement = mCurrentDB.prepare( query, nError );

  QString tag;
  if ( nError == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    tag = statement.columnAsText( 0 );
  }

  return tag;
}

int QgsStyle::getId( const QString &table, const QString &name )
{
  char *query = sqlite3_mprintf( "SELECT id FROM %q WHERE LOWER(name)='%q'", table.toUtf8().constData(), name.toUtf8().toLower().constData() );

  sqlite3_statement_unique_ptr statement;
  int nErr; statement = mCurrentDB.prepare( query, nErr );

  int id = 0;
  if ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    id = sqlite3_column_int( statement.get(), 0 );
  }

  return id;
}

QString QgsStyle::getName( const QString &table, int id ) const
{
  char *query = sqlite3_mprintf( "SELECT name FROM %q WHERE id='%q'", table.toUtf8().constData(), QString::number( id ).toUtf8().constData() );

  sqlite3_statement_unique_ptr statement;
  int nErr; statement = mCurrentDB.prepare( query, nErr );

  QString name;
  if ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    name = statement.columnAsText( 0 );
  }

  return name;
}

int QgsStyle::symbolId( const QString &name )
{
  return getId( QStringLiteral( "symbol" ), name );
}

int QgsStyle::colorrampId( const QString &name )
{
  return getId( QStringLiteral( "colorramp" ), name );
}

int QgsStyle::tagId( const QString &name )
{
  return getId( QStringLiteral( "tag" ), name );
}

int QgsStyle::smartgroupId( const QString &name )
{
  return getId( QStringLiteral( "smartgroup" ), name );
}

int QgsStyle::addSmartgroup( const QString &name, const QString &op, const QgsSmartConditionMap &conditions )
{
  QDomDocument doc( QStringLiteral( "dummy" ) );
  QDomElement smartEl = doc.createElement( QStringLiteral( "smartgroup" ) );
  smartEl.setAttribute( QStringLiteral( "name" ), name );
  smartEl.setAttribute( QStringLiteral( "operator" ), op );

  QStringList constraints;
  constraints << QStringLiteral( "tag" ) << QStringLiteral( "group" ) << QStringLiteral( "name" ) << QStringLiteral( "!tag" ) << QStringLiteral( "!group" ) << QStringLiteral( "!name" );

  Q_FOREACH ( const QString &constraint, constraints )
  {
    QStringList parameters = conditions.values( constraint );
    Q_FOREACH ( const QString &param, parameters )
    {
      QDomElement condEl = doc.createElement( QStringLiteral( "condition" ) );
      condEl.setAttribute( QStringLiteral( "constraint" ), constraint );
      condEl.setAttribute( QStringLiteral( "param" ), param );
      smartEl.appendChild( condEl );
    }
  }

  QByteArray xmlArray;
  QTextStream stream( &xmlArray );
  stream.setCodec( "UTF-8" );
  smartEl.save( stream, 4 );
  char *query = sqlite3_mprintf( "INSERT INTO smartgroup VALUES (NULL, '%q', '%q')",
                                 name.toUtf8().constData(), xmlArray.constData() );

  if ( runEmptyQuery( query ) )
  {
    QgsSettings settings;
    settings.setValue( QStringLiteral( "qgis/symbolsListGroupsIndex" ), 0 );

    emit groupsModified();
    return static_cast< int >( sqlite3_last_insert_rowid( mCurrentDB.get() ) );
  }
  else
  {
    QgsDebugMsg( "Couldn't insert symbol into the database!" );
    return 0;
  }
}

QgsSymbolGroupMap QgsStyle::smartgroupsListMap()
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( "Cannot open database for listing groups" );
    return QgsSymbolGroupMap();
  }

  char *query = sqlite3_mprintf( "SELECT * FROM smartgroup" );

  // Now run the query and retrieve the group names
  sqlite3_statement_unique_ptr statement;
  int nError;
  statement = mCurrentDB.prepare( query, nError );

  QgsSymbolGroupMap groupNames;
  while ( nError == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    QString group = statement.columnAsText( SmartgroupName );
    groupNames.insert( sqlite3_column_int( statement.get(), SmartgroupId ), group );
  }

  return groupNames;
}

QStringList QgsStyle::smartgroupNames()
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( "Cannot open database for listing groups" );
    return QStringList();
  }

  char *query = sqlite3_mprintf( "SELECT name FROM smartgroup" );

  // Now run the query and retrieve the group names
  sqlite3_statement_unique_ptr statement;
  int nError;
  statement = mCurrentDB.prepare( query, nError );

  QStringList groups;
  while ( nError == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    groups << statement.columnAsText( 0 );
  }

  return groups;
}

QStringList QgsStyle::symbolsOfSmartgroup( StyleEntity type, int id )
{
  QStringList symbols;

  char *query = sqlite3_mprintf( "SELECT xml FROM smartgroup WHERE id=%d", id );

  sqlite3_statement_unique_ptr statement;
  int nErr; statement = mCurrentDB.prepare( query, nErr );
  if ( !( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW ) )
  {
    return QStringList();
  }
  else
  {
    QDomDocument doc;
    QString xmlstr = statement.columnAsText( 0 );
    if ( !doc.setContent( xmlstr ) )
    {
      QgsDebugMsg( QString( "Cannot open smartgroup id: %1" ).arg( id ) );
    }
    QDomElement smartEl = doc.documentElement();
    QString op = smartEl.attribute( QStringLiteral( "operator" ) );
    QDomNodeList conditionNodes = smartEl.childNodes();

    bool firstSet = true;
    for ( int i = 0; i < conditionNodes.count(); i++ )
    {
      QDomElement condEl = conditionNodes.at( i ).toElement();
      QString constraint = condEl.attribute( QStringLiteral( "constraint" ) );
      QString param = condEl.attribute( QStringLiteral( "param" ) );

      QStringList resultNames;
      // perform suitable action for the given constraint
      if ( constraint == QLatin1String( "tag" ) )
      {
        resultNames = symbolsWithTag( type, tagId( param ) );
      }
      else if ( constraint == QLatin1String( "name" ) )
      {
        if ( type == SymbolEntity )
        {
          resultNames = symbolNames().filter( param, Qt::CaseInsensitive );
        }
        else
        {
          resultNames = colorRampNames().filter( param, Qt::CaseInsensitive );
        }
      }
      else if ( constraint == QLatin1String( "!tag" ) )
      {
        resultNames = type == SymbolEntity ? symbolNames() : colorRampNames();
        QStringList unwanted = symbolsWithTag( type, tagId( param ) );
        Q_FOREACH ( const QString &name, unwanted )
        {
          resultNames.removeAll( name );
        }
      }
      else if ( constraint == QLatin1String( "!name" ) )
      {
        QStringList all = type == SymbolEntity ? symbolNames() : colorRampNames();
        Q_FOREACH ( const QString &str, all )
        {
          if ( !str.contains( param, Qt::CaseInsensitive ) )
            resultNames << str;
        }
      }

      // not apply the operator
      if ( firstSet )
      {
        symbols = resultNames;
        firstSet = false;
      }
      else
      {
        if ( op == QLatin1String( "OR" ) )
        {
          symbols << resultNames;
        }
        else if ( op == QLatin1String( "AND" ) )
        {
          QStringList dummy = symbols;
          symbols.clear();
          Q_FOREACH ( const QString &result, resultNames )
          {
            if ( dummy.contains( result ) )
              symbols << result;
          }
        }
      }
    } // DOM loop ends here
  }

  return symbols;
}

QgsSmartConditionMap QgsStyle::smartgroup( int id )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( "Cannot open database for listing groups" );
    return QgsSmartConditionMap();
  }

  QgsSmartConditionMap condition;

  char *query = sqlite3_mprintf( "SELECT xml FROM smartgroup WHERE id=%d", id );

  sqlite3_statement_unique_ptr statement;
  int nError;
  statement = mCurrentDB.prepare( query, nError );
  if ( nError == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    QDomDocument doc;
    QString xmlstr = statement.columnAsText( 0 );
    if ( !doc.setContent( xmlstr ) )
    {
      QgsDebugMsg( QString( "Cannot open smartgroup id: %1" ).arg( id ) );
    }

    QDomElement smartEl = doc.documentElement();
    QDomNodeList conditionNodes = smartEl.childNodes();

    for ( int i = 0; i < conditionNodes.count(); i++ )
    {
      QDomElement condEl = conditionNodes.at( i ).toElement();
      QString constraint = condEl.attribute( QStringLiteral( "constraint" ) );
      QString param = condEl.attribute( QStringLiteral( "param" ) );

      condition.insert( constraint, param );
    }
  }

  return condition;
}

QString QgsStyle::smartgroupOperator( int id )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( "Cannot open database for listing groups" );
    return QString();
  }

  QString op;

  char *query = sqlite3_mprintf( "SELECT xml FROM smartgroup WHERE id=%d", id );

  int nError;
  sqlite3_statement_unique_ptr statement;
  statement = mCurrentDB.prepare( query, nError );
  if ( nError == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    QDomDocument doc;
    QString xmlstr = statement.columnAsText( 0 );
    if ( !doc.setContent( xmlstr ) )
    {
      QgsDebugMsg( QString( "Cannot open smartgroup id: %1" ).arg( id ) );
    }
    QDomElement smartEl = doc.documentElement();
    op = smartEl.attribute( QStringLiteral( "operator" ) );
  }

  return op;
}

bool QgsStyle::exportXml( const QString &filename )
{
  if ( filename.isEmpty() )
  {
    QgsDebugMsg( "Invalid filename for style export." );
    return false;
  }

  QDomDocument doc( QStringLiteral( "qgis_style" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgis_style" ) );
  root.setAttribute( QStringLiteral( "version" ), STYLE_CURRENT_VERSION );
  doc.appendChild( root );

  QStringList favoriteSymbols = symbolsOfFavorite( SymbolEntity );
  QStringList favoriteColorramps = symbolsOfFavorite( ColorrampEntity );

  // save symbols and attach tags
  QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( mSymbols, QStringLiteral( "symbols" ), doc, QgsReadWriteContext() );
  QDomNodeList symbolsList = symbolsElem.elementsByTagName( QStringLiteral( "symbol" ) );
  int nbSymbols = symbolsList.count();
  for ( int i = 0; i < nbSymbols; ++i )
  {
    QDomElement symbol = symbolsList.at( i ).toElement();
    QString name = symbol.attribute( QStringLiteral( "name" ) );
    QStringList tags = tagsOfSymbol( SymbolEntity, name );
    if ( tags.count() > 0 )
    {
      symbol.setAttribute( QStringLiteral( "tags" ), tags.join( ',' ) );
    }
    if ( favoriteSymbols.contains( name ) )
    {
      symbol.setAttribute( QStringLiteral( "favorite" ), QStringLiteral( "1" ) );
    }
  }

  // save color ramps
  QDomElement rampsElem = doc.createElement( QStringLiteral( "colorramps" ) );
  for ( QMap<QString, QgsColorRamp *>::const_iterator itr = mColorRamps.constBegin(); itr != mColorRamps.constEnd(); ++itr )
  {
    QDomElement rampEl = QgsSymbolLayerUtils::saveColorRamp( itr.key(), itr.value(), doc );
    QStringList tags = tagsOfSymbol( ColorrampEntity, itr.key() );
    if ( tags.count() > 0 )
    {
      rampEl.setAttribute( QStringLiteral( "tags" ), tags.join( ',' ) );
    }
    if ( favoriteColorramps.contains( itr.key() ) )
    {
      rampEl.setAttribute( QStringLiteral( "favorite" ), QStringLiteral( "1" ) );
    }
    rampsElem.appendChild( rampEl );
  }

  root.appendChild( symbolsElem );
  root.appendChild( rampsElem );

  // save
  QFile f( filename );
  if ( !f.open( QFile::WriteOnly | QIODevice::Truncate ) )
  {
    mErrorString = "Couldn't open file for writing: " + filename;
    return false;
  }

  QTextStream ts( &f );
  ts.setCodec( "UTF-8" );
  doc.save( ts, 2 );
  f.close();

  mFileName = filename;
  return true;
}

bool QgsStyle::importXml( const QString &filename )
{
  mErrorString = QString();
  QDomDocument doc( QStringLiteral( "style" ) );
  QFile f( filename );
  if ( !f.open( QFile::ReadOnly ) )
  {
    mErrorString = QStringLiteral( "Unable to open the specified file" );
    QgsDebugMsg( "Error opening the style XML file." );
    return false;
  }

  if ( !doc.setContent( &f ) )
  {
    mErrorString = QStringLiteral( "Unable to understand the style file: %1" ).arg( filename );
    QgsDebugMsg( "XML Parsing error" );
    f.close();
    return false;
  }
  f.close();

  QDomElement docEl = doc.documentElement();
  if ( docEl.tagName() != QLatin1String( "qgis_style" ) )
  {
    mErrorString = "Incorrect root tag in style: " + docEl.tagName();
    return false;
  }

  QString version = docEl.attribute( QStringLiteral( "version" ) );
  if ( version != STYLE_CURRENT_VERSION && version != QLatin1String( "0" ) )
  {
    mErrorString = "Unknown style file version: " + version;
    return false;
  }

  QgsSymbolMap symbols;

  QDomElement symbolsElement = docEl.firstChildElement( QStringLiteral( "symbols" ) );
  QDomElement e = symbolsElement.firstChildElement();

  // gain speed by re-grouping the INSERT statements in a transaction
  char *query = nullptr;
  query = sqlite3_mprintf( "BEGIN TRANSACTION;" );
  runEmptyQuery( query );

  if ( version == STYLE_CURRENT_VERSION )
  {
    // For the new style, load symbols individually
    while ( !e.isNull() )
    {
      if ( e.tagName() == QLatin1String( "symbol" ) )
      {
        QString name = e.attribute( QStringLiteral( "name" ) );
        QStringList tags;
        if ( e.hasAttribute( QStringLiteral( "tags" ) ) )
        {
          tags = e.attribute( QStringLiteral( "tags" ) ).split( ',' );
        }
        bool favorite = false;
        if ( e.hasAttribute( QStringLiteral( "favorite" ) ) && e.attribute( QStringLiteral( "favorite" ) ) == QStringLiteral( "1" ) )
        {
          favorite = true;
        }

        QgsSymbol *symbol = QgsSymbolLayerUtils::loadSymbol( e, QgsReadWriteContext() );
        if ( symbol )
        {
          addSymbol( name, symbol );
          if ( mCurrentDB )
          {
            saveSymbol( name, symbol, favorite, tags );
          }
        }
      }
      else
      {
        QgsDebugMsg( "unknown tag: " + e.tagName() );
      }
      e = e.nextSiblingElement();
    }
  }
  else
  {
    // for the old version, use the utility function to solve @symbol@layer subsymbols
    symbols = QgsSymbolLayerUtils::loadSymbols( symbolsElement, QgsReadWriteContext() );

    // save the symbols with proper name
    for ( QMap<QString, QgsSymbol *>::iterator it = symbols.begin(); it != symbols.end(); ++it )
    {
      addSymbol( it.key(), it.value() );
    }
  }

  // load color ramps
  QDomElement rampsElement = docEl.firstChildElement( QStringLiteral( "colorramps" ) );
  e = rampsElement.firstChildElement();
  while ( !e.isNull() )
  {
    if ( e.tagName() == QLatin1String( "colorramp" ) )
    {
      QString name = e.attribute( QStringLiteral( "name" ) );
      QStringList tags;
      if ( e.hasAttribute( QStringLiteral( "tags" ) ) )
      {
        tags = e.attribute( QStringLiteral( "tags" ) ).split( ',' );
      }
      bool favorite = false;
      if ( e.hasAttribute( QStringLiteral( "favorite" ) ) && e.attribute( QStringLiteral( "favorite" ) ) == QStringLiteral( "1" ) )
      {
        favorite = true;
      }

      QgsColorRamp *ramp = QgsSymbolLayerUtils::loadColorRamp( e );
      if ( ramp )
      {
        addColorRamp( name, ramp );
        if ( mCurrentDB )
        {
          saveColorRamp( name, ramp, favorite, tags );
        }
      }
    }
    else
    {
      QgsDebugMsg( "unknown tag: " + e.tagName() );
    }
    e = e.nextSiblingElement();
  }

  query = sqlite3_mprintf( "COMMIT TRANSACTION;" );
  runEmptyQuery( query );

  mFileName = filename;
  return true;
}

bool QgsStyle::updateSymbol( StyleEntity type, const QString &name )
{
  QDomDocument doc( QStringLiteral( "dummy" ) );
  QDomElement symEl;
  QByteArray xmlArray;
  QTextStream stream( &xmlArray );
  stream.setCodec( "UTF-8" );

  char *query = nullptr;

  if ( type == SymbolEntity )
  {
    // check if it is an existing symbol
    if ( !symbolNames().contains( name ) )
    {
      QgsDebugMsg( "Update request received for unavailable symbol" );
      return false;
    }

    symEl = QgsSymbolLayerUtils::saveSymbol( name, symbol( name ), doc, QgsReadWriteContext() );
    if ( symEl.isNull() )
    {
      QgsDebugMsg( "Couldn't convert symbol to valid XML!" );
      return false;
    }
    symEl.save( stream, 4 );
    query = sqlite3_mprintf( "UPDATE symbol SET xml='%q' WHERE name='%q';",
                             xmlArray.constData(), name.toUtf8().constData() );
  }
  else if ( type == ColorrampEntity )
  {
    if ( !colorRampNames().contains( name ) )
    {
      QgsDebugMsg( "Update requested for unavailable color ramp." );
      return false;
    }

    std::unique_ptr< QgsColorRamp > ramp( colorRamp( name ) );
    symEl = QgsSymbolLayerUtils::saveColorRamp( name, ramp.get(), doc );
    if ( symEl.isNull() )
    {
      QgsDebugMsg( "Couldn't convert color ramp to valid XML!" );
      return false;
    }
    symEl.save( stream, 4 );
    query = sqlite3_mprintf( "UPDATE colorramp SET xml='%q' WHERE name='%q';",
                             xmlArray.constData(), name.toUtf8().constData() );
  }
  else
  {
    QgsDebugMsg( "Updating the unsupported StyleEntity" );
    return false;
  }


  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( "Couldn't insert symbol into the database!" );
    return false;
  }
  return true;
}
