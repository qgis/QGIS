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

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>
#include <QFile>
#include <QTextStream>
#include <QByteArray>

#include <sqlite3.h>

#define STYLE_CURRENT_VERSION  "1"

QgsStyle *QgsStyle::mDefaultStyle = nullptr;


QgsStyle::QgsStyle() : QObject()
{
  mCurrentDB = nullptr;
}

QgsStyle::~QgsStyle()
{
  clear();
}

QgsStyle* QgsStyle::defaultStyle() // static
{
  if ( !mDefaultStyle )
  {
    QString styleFilename = QgsApplication::userStylePath();

    // copy default style if user style doesn't exist
    if ( !QFile::exists( styleFilename ) )
    {
      QFile::copy( QgsApplication::defaultStylePath(), styleFilename );
    }

    mDefaultStyle = new QgsStyle;
    mDefaultStyle->load( styleFilename );
  }
  return mDefaultStyle;
}


void QgsStyle::clear()
{
  qDeleteAll( mSymbols );
  qDeleteAll( mColorRamps );

  mSymbols.clear();
  mColorRamps.clear();
  if ( mCurrentDB )
    sqlite3_close( mCurrentDB );
}

bool QgsStyle::addSymbol( const QString& name, QgsSymbol* symbol, bool update )
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
      saveSymbol( name, symbol, 0, QStringList() );
  }

  return true;
}

bool QgsStyle::saveSymbol( const QString& name, QgsSymbol* symbol, int groupid, const QStringList& tags )
{
  // TODO add support for groups
  QDomDocument doc( "dummy" );
  QDomElement symEl = QgsSymbolLayerUtils::saveSymbol( name, symbol, doc );
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
                                 name.toUtf8().constData(), xmlArray.constData(), groupid );

  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( "Couldn't insert symbol into the database!" );
    return false;
  }

  tagSymbol( SymbolEntity, name, tags );

  emit symbolSaved( name, symbol );

  return true;
}

bool QgsStyle::removeSymbol( const QString& name )
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

QgsSymbol* QgsStyle::symbol( const QString& name )
{
  const QgsSymbol *symbol = symbolRef( name );
  return symbol ? symbol->clone() : nullptr;
}

const QgsSymbol *QgsStyle::symbolRef( const QString& name ) const
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


bool QgsStyle::addColorRamp( const QString& name, QgsColorRamp* colorRamp, bool update )
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
      saveColorRamp( name, colorRamp, 0, QStringList() );
  }

  return true;
}

bool QgsStyle::saveColorRamp( const QString& name, QgsColorRamp* ramp, int groupid, const QStringList& tags )
{
  // insert it into the database
  QDomDocument doc( "dummy" );
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
                                 name.toUtf8().constData(), xmlArray.constData(), groupid );

  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( "Couldn't insert colorramp into the database!" );
    return false;
  }

  tagSymbol( ColorrampEntity, name, tags );

  return true;
}

bool QgsStyle::removeColorRamp( const QString& name )
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

QgsColorRamp* QgsStyle::colorRamp( const QString& name ) const
{
  const QgsColorRamp *ramp = colorRampRef( name );
  return ramp ? ramp->clone() : nullptr;
}

const QgsColorRamp* QgsStyle::colorRampRef( const QString& name ) const
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

bool QgsStyle::openDB( const QString& filename )
{
  int rc = sqlite3_open( filename.toUtf8(), &mCurrentDB );
  if ( rc )
  {
    mErrorString = "Couldn't open the style database: " + QString( sqlite3_errmsg( mCurrentDB ) );
    sqlite3_close( mCurrentDB );
    return false;
  }

  return true;
}

bool QgsStyle::load( const QString& filename )
{
  mErrorString.clear();

  // Open the sqlite database
  if ( !openDB( filename ) )
  {
    mErrorString = "Unable to open database file specified";
    QgsDebugMsg( mErrorString );
    return false;
  }

  // Make sure there are no Null fields in parenting symbols ang groups
  char *query = sqlite3_mprintf( "UPDATE symbol SET groupid=0 WHERE groupid IS NULL;"
                                 "UPDATE colorramp SET groupid=0 WHERE groupid IS NULL;"
                                 "UPDATE symgroup SET parent=0 WHERE parent IS NULL;" );
  runEmptyQuery( query );

  // First create all the main symbols
  query = sqlite3_mprintf( "SELECT * FROM symbol" );

  sqlite3_stmt *ppStmt;
  int nError = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );
  while ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QDomDocument doc;
    QString symbol_name = QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt, SymbolName ) ) );
    QString xmlstring = QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt, SymbolXML ) ) );
    if ( !doc.setContent( xmlstring ) )
    {
      QgsDebugMsg( "Cannot open symbol " + symbol_name );
      continue;
    }

    QDomElement symElement = doc.documentElement();
    QgsSymbol *symbol = QgsSymbolLayerUtils::loadSymbol( symElement );
    if ( symbol )
      mSymbols.insert( symbol_name, symbol );
  }

  sqlite3_finalize( ppStmt );

  query = sqlite3_mprintf( "SELECT * FROM colorramp" );
  nError = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );
  while ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QDomDocument doc;
    QString ramp_name = QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt, ColorrampName ) ) );
    QString xmlstring = QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt, ColorrampXML ) ) );
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
  for ( QMap<QString, QgsColorRamp*>::iterator itr = mColorRamps.begin(); itr != mColorRamps.end(); ++itr )
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

bool QgsStyle::renameSymbol( const QString& oldName, const QString& newName )
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

bool QgsStyle::renameColorRamp( const QString& oldName, const QString& newName )
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
  sqlite3_stmt *ppStmt;
  char *query = sqlite3_mprintf( "SELECT id FROM colorramp WHERE name='%q'", oldName.toUtf8().constData() );
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );
  if ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    rampid = sqlite3_column_int( ppStmt, 0 );
  }
  sqlite3_finalize( ppStmt );
  rename( ColorrampEntity, rampid, newName );

  return true;
}

QStringList QgsStyle::groupNames()
{
  QStringList groupNames;
  sqlite3_stmt *ppStmt;
  const char *query = "SELECT * FROM symgroup";
  int nError = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );
  while ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    groupNames << QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt, SymgroupName ) ) );
  }
  sqlite3_finalize( ppStmt );
  return groupNames;
}

QList<int> QgsStyle::groupIds() const
{
  QList<int> groupIds;
  sqlite3_stmt *ppStmt;
  const char *query = "SELECT * FROM symgroup";
  int nError = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );
  while ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    groupIds << QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt, SymgroupId ) ) ).toInt();
  }
  sqlite3_finalize( ppStmt );
  return groupIds;
}

QgsSymbolGroupMap QgsStyle::childGroupNames( const QString& parent )
{
  // get the name list from the sqlite database and return as a QStringList
  if ( !mCurrentDB )
  {
    QgsDebugMsg( "Cannot open database for listing groups" );
    return QgsSymbolGroupMap();
  }

  char *query = nullptr;
  int nError;
  sqlite3_stmt *ppStmt;

  // decide the query to be run based on parent group
  if ( parent == "" || parent == QString() )
  {
    query = sqlite3_mprintf( "SELECT * FROM symgroup WHERE parent=0" );
  }
  else
  {
    char *subquery = sqlite3_mprintf( "SELECT * FROM symgroup WHERE name='%q'", parent.toUtf8().constData() );
    nError = sqlite3_prepare_v2( mCurrentDB, subquery, -1, &ppStmt, nullptr );
    if ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
    {
      query = sqlite3_mprintf( "SELECT * FROM symgroup WHERE parent=%d", sqlite3_column_int( ppStmt, SymgroupId ) );
    }
    sqlite3_finalize( ppStmt );
  }

  if ( !query )
    return QgsSymbolGroupMap();

  QgsSymbolGroupMap groupNames;

  // Now run the query and retrieve the group names
  nError = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );
  while ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QString group = QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt, SymgroupName ) ) );
    groupNames.insert( sqlite3_column_int( ppStmt, SymgroupId ), group );
  }

  sqlite3_finalize( ppStmt );

  return groupNames;
}

QStringList QgsStyle::symbolsOfGroup( StyleEntity type, int groupid )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( QString( "Cannot Open database for getting group symbols of groupid: %1" ).arg( groupid ) );
    return QStringList();
  }

  char *query;
  if ( type == SymbolEntity )
  {
    query = sqlite3_mprintf( "SELECT name FROM symbol WHERE groupid=%d", groupid );
  }
  else if ( type == ColorrampEntity )
  {
    query = sqlite3_mprintf( "SELECT name FROM colorramp WHERE groupid=%d", groupid );
  }
  else
  {
    QgsDebugMsg( "No such style entity" );
    return QStringList();
  }

  sqlite3_stmt *ppStmt;
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );

  QStringList symbols;
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    symbols << QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt, 0 ) ) );
  }

  sqlite3_finalize( ppStmt );

  return symbols;
}

QStringList QgsStyle::symbolsWithTag( StyleEntity type, int tagid )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( QString( "Cannot open database to get symbols of tagid %1" ).arg( tagid ) );
    return QStringList();
  }

  char *subquery;
  if ( type == SymbolEntity )
  {
    subquery = sqlite3_mprintf( "SELECT symbol_id FROM tagmap WHERE tag_id=%d", tagid );
  }
  else if ( type == ColorrampEntity )
  {
    subquery = sqlite3_mprintf( "SELECT symbol_id FROM ctagmap WHERE tag_id=%d", tagid );
  }
  else
  {
    QgsDebugMsg( "Unknown Entity" );
    return QStringList();
  }

  sqlite3_stmt *ppStmt;
  int nErr = sqlite3_prepare_v2( mCurrentDB, subquery, -1, &ppStmt, nullptr );

  // get the symbol <-> tag connection from table 'tagmap'
  QStringList symbols;
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    int symbolId = sqlite3_column_int( ppStmt, 0 );

    char *query = type == SymbolEntity
                  ? sqlite3_mprintf( "SELECT name FROM symbol WHERE id=%d", symbolId )
                  : sqlite3_mprintf( "SELECT name FROM colorramp WHERE id=%d", symbolId );

    sqlite3_stmt *ppStmt2;
    int sErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt2, nullptr );
    while ( sErr == SQLITE_OK && sqlite3_step( ppStmt2 ) == SQLITE_ROW )
    {
      symbols << QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt2, 0 ) ) );
    }
    sqlite3_finalize( ppStmt2 );
  }
  sqlite3_finalize( ppStmt );

  return symbols;
}

int QgsStyle::addGroup( const QString& groupName, int parentid )
{
  if ( !mCurrentDB )
    return 0;

  char *query = sqlite3_mprintf( "INSERT INTO symgroup VALUES (NULL, '%q', %d)", groupName.toUtf8().constData(), parentid );

  sqlite3_stmt *ppStmt;
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );
  if ( nErr == SQLITE_OK )
    ( void )sqlite3_step( ppStmt );

  sqlite3_finalize( ppStmt );

  return static_cast< int >( sqlite3_last_insert_rowid( mCurrentDB ) );
}

int QgsStyle::addTag( const QString& tagname )
{
  if ( !mCurrentDB )
    return 0;
  sqlite3_stmt *ppStmt;

  char *query = sqlite3_mprintf( "INSERT INTO tag VALUES (NULL, '%q')", tagname.toUtf8().constData() );
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );
  if ( nErr == SQLITE_OK )
    ( void )sqlite3_step( ppStmt );
  sqlite3_finalize( ppStmt );

  return static_cast< int >( sqlite3_last_insert_rowid( mCurrentDB ) );
}

QStringList QgsStyle::tags() const
{
  if ( !mCurrentDB )
    return QStringList();

  sqlite3_stmt *ppStmt;

  char *query = sqlite3_mprintf( "SELECT name FROM tag" );
  int nError = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );

  QStringList tagList;
  while ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    tagList << QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt, 0 ) ) );
  }

  sqlite3_finalize( ppStmt );

  return tagList;
}

void QgsStyle::rename( StyleEntity type, int id, const QString& newName )
{
  char *query;
  switch ( type )
  {
    case SymbolEntity:
      query = sqlite3_mprintf( "UPDATE symbol SET name='%q' WHERE id=%d", newName.toUtf8().constData(), id );
      break;
    case GroupEntity:
      query = sqlite3_mprintf( "UPDATE symgroup SET name='%q' WHERE id=%d", newName.toUtf8().constData(), id );
      break;
    case TagEntity:
      query = sqlite3_mprintf( "UPDATE tag SET name='%q' WHERE id=%d", newName.toUtf8().constData(), id );
      break;
    case ColorrampEntity:
      query = sqlite3_mprintf( "UPDATE colorramp SET name='%q' WHERE id=%d", newName.toUtf8().constData(), id );
      break;
    case SmartgroupEntity:
      query = sqlite3_mprintf( "UPDATE smartgroup SET name='%q' WHERE id=%d", newName.toUtf8().constData(), id );
      break;
    default:
      QgsDebugMsg( "Invalid Style Entity indicated" );
      return;
  }
  if ( !runEmptyQuery( query ) )
    mErrorString = "Could not rename!";
}

char* QgsStyle::getGroupRemoveQuery( int id )
{
  char *query = sqlite3_mprintf( "SELECT parent FROM symgroup WHERE id=%d", id );

  sqlite3_stmt *ppStmt;
  int err = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );

  int parentid = 0;
  if ( err == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
    parentid = sqlite3_column_int( ppStmt, 0 );

  sqlite3_finalize( ppStmt );

  return sqlite3_mprintf( "UPDATE symbol SET groupid=%d WHERE groupid=%d;"
                          "UPDATE symgroup SET parent=%d WHERE parent=%d;"
                          "DELETE FROM symgroup WHERE id=%d", parentid, id, parentid, id, id );
}

void QgsStyle::remove( StyleEntity type, int id )
{
  char *query;
  switch ( type )
  {
    case SymbolEntity:
      query = sqlite3_mprintf( "DELETE FROM symbol WHERE id=%d; DELETE FROM tagmap WHERE symbol_id=%d", id, id );
      break;
    case GroupEntity:
      query = getGroupRemoveQuery( id );
      break;
    case TagEntity:
      query = sqlite3_mprintf( "DELETE FROM tag WHERE id=%d; DELETE FROM tagmap WHERE tag_id=%d", id, id );
      break;
    case ColorrampEntity:
      query = sqlite3_mprintf( "DELETE FROM colorramp WHERE id=%d", id );
      break;
    case SmartgroupEntity:
      query = sqlite3_mprintf( "DELETE FROM smartgroup WHERE id=%d", id );
      break;
    default:
      QgsDebugMsg( "Invalid Style Entity indicated" );
      return;
  }

  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( "Could not delete entity!" );
  }
}

bool QgsStyle::runEmptyQuery( char *query, bool freeQuery )
{
  if ( !mCurrentDB )
    return false;

  char *zErr = nullptr;
  int nErr = sqlite3_exec( mCurrentDB, query, nullptr, nullptr, &zErr );

  if ( freeQuery )
  {
    sqlite3_free( query );
  }

  if ( nErr != SQLITE_OK )
  {
    QgsDebugMsg( zErr );
  }

  return zErr == SQLITE_OK;
}

bool QgsStyle::group( StyleEntity type, const QString& name, int groupid )
{
  char *query;

  switch ( type )
  {
    case SymbolEntity:
      query = sqlite3_mprintf( "UPDATE symbol SET groupid=%d WHERE name='%q'", groupid, name.toUtf8().constData() );
      break;
    case ColorrampEntity:
      query = sqlite3_mprintf( "UPDATE colorramp SET groupid=%d WHERE name='%q'", groupid, name.toUtf8().constData() );
      break;

    default:
      QgsDebugMsg( "Wrong entity value. cannot apply group" );
      return false;
  }

  return runEmptyQuery( query );
}

QStringList QgsStyle::findSymbols( StyleEntity type, const QString& qword )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( "Sorry! Cannot open database to search" );
    return QStringList();
  }

  // first find symbols with matching name
  QString item = ( type == SymbolEntity ) ? "symbol" : "colorramp";
  char *query = sqlite3_mprintf( "SELECT name FROM %q WHERE name LIKE '%%%q%%'",
                                 item.toUtf8().constData(), qword.toUtf8().constData() );

  sqlite3_stmt *ppStmt;
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );

  QSet< QString > symbols;
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    symbols << QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt, 0 ) ) );
  }

  sqlite3_finalize( ppStmt );

  // next add symbols with matching tags
  query = sqlite3_mprintf( "SELECT id FROM tag WHERE name LIKE '%%%q%%'", qword.toUtf8().constData() );
  nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );

  QStringList tagids;
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    tagids << QString::fromUtf8(( const char * ) sqlite3_column_text( ppStmt, 0 ) );
  }

  sqlite3_finalize( ppStmt );


  QString dummy = tagids.join( ", " );

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
  nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );

  QStringList symbolids;
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    symbolids << QString::fromUtf8(( const char * ) sqlite3_column_text( ppStmt, 0 ) );
  }

  sqlite3_finalize( ppStmt );


  dummy = symbolids.join( ", " );
  query = sqlite3_mprintf( "SELECT name FROM %q  WHERE id IN (%q)",
                           item.toUtf8().constData(), dummy.toUtf8().constData() );
  nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    symbols << QString::fromUtf8(( const char * ) sqlite3_column_text( ppStmt, 0 ) );
  }

  sqlite3_finalize( ppStmt );

  return symbols.toList();
}

bool QgsStyle::tagSymbol( StyleEntity type, const QString& symbol, const QStringList& tags )
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


  Q_FOREACH ( const QString &tag, tags )
  {
    // sql: gets the id of the tag if present or insert the tag and get the id of the tag
    char *query = sqlite3_mprintf( "SELECT id FROM tag WHERE name='%q'", tag.toUtf8().constData() );

    sqlite3_stmt *ppStmt;
    int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );

    int tagid;
    if ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
    {
      tagid = sqlite3_column_int( ppStmt, 0 );
    }
    else
    {
      tagid = addTag( tag );
    }

    sqlite3_finalize( ppStmt );

    // Now map the tag to the symbol
    query = type == SymbolEntity
            ? sqlite3_mprintf( "INSERT INTO tagmap VALUES (%d,%d)", tagid, symbolid )
            : sqlite3_mprintf( "INSERT INTO ctagmap VALUES (%d,%d)", tagid, symbolid );

    char *zErr = nullptr;
    nErr = sqlite3_exec( mCurrentDB, query, nullptr, nullptr, &zErr );
    if ( nErr )
    {
      QgsDebugMsg( zErr );
    }
  }

  return true;
}

bool QgsStyle::detagSymbol( StyleEntity type, const QString& symbol, const QStringList& tags )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( "Sorry! Cannot open database for detgging." );
    return false;
  }

  char *query = type == SymbolEntity
                ? sqlite3_mprintf( "SELECT id FROM symbol WHERE name='%q'", symbol.toUtf8().constData() )
                : sqlite3_mprintf( "SELECT id FROM colorramp WHERE name='%q'", symbol.toUtf8().constData() );
  sqlite3_stmt *ppStmt;
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );

  int symbolid = 0;
  if ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    symbolid = sqlite3_column_int( ppStmt, 0 );
  }
  else
  {
    sqlite3_finalize( ppStmt );
    return false;
  }

  sqlite3_finalize( ppStmt );

  Q_FOREACH ( const QString &tag, tags )
  {
    query = sqlite3_mprintf( "SELECT id FROM tag WHERE name='%q'", tag.toUtf8().constData() );

    sqlite3_stmt *ppStmt2;
    nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt2, nullptr );

    int tagid = 0;
    if ( nErr == SQLITE_OK && sqlite3_step( ppStmt2 ) == SQLITE_ROW )
    {
      tagid = sqlite3_column_int( ppStmt2, 0 );
    }

    sqlite3_finalize( ppStmt2 );

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

QStringList QgsStyle::tagsOfSymbol( StyleEntity type, const QString& symbol )
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

  sqlite3_stmt *ppStmt;
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );

  QStringList tagList;
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    char *subquery = sqlite3_mprintf( "SELECT name FROM tag WHERE id=%d", sqlite3_column_int( ppStmt, 0 ) );

    sqlite3_stmt *ppStmt2;
    int pErr = sqlite3_prepare_v2( mCurrentDB, subquery, -1, &ppStmt2, nullptr );
    if ( pErr == SQLITE_OK && sqlite3_step( ppStmt2 ) == SQLITE_ROW )
    {
      tagList << QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt2, 0 ) ) );
    }
    sqlite3_finalize( ppStmt2 );
  }

  sqlite3_finalize( ppStmt );

  return tagList;
}

int QgsStyle::getId( const QString& table, const QString& name )
{
  char *query = sqlite3_mprintf( "SELECT id FROM %q WHERE name='%q'", table.toUtf8().constData(), name.toUtf8().constData() );

  sqlite3_stmt *ppStmt;
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );

  int id = 0;
  if ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    id = sqlite3_column_int( ppStmt, 0 );
  }

  sqlite3_finalize( ppStmt );

  return id;
}

QString QgsStyle::getName( const QString& table, int id ) const
{
  char *query = sqlite3_mprintf( "SELECT name FROM %q WHERE id='%q'", table.toUtf8().constData(), QString::number( id ).toUtf8().constData() );

  sqlite3_stmt *ppStmt;
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );

  QString name;
  if ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    name = QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt, 0 ) ) );
  }

  sqlite3_finalize( ppStmt );

  return name;
}

int QgsStyle::symbolId( const QString& name )
{
  return getId( "symbol", name );
}

int QgsStyle::colorrampId( const QString& name )
{
  return getId( "colorramp", name );
}

int QgsStyle::groupId( const QString& name )
{
  return getId( "symgroup", name );
}

QString QgsStyle::groupName( int groupId ) const
{
  return getName( "symgroup", groupId );
}

int QgsStyle::tagId( const QString& name )
{
  return getId( "tag", name );
}

int QgsStyle::smartgroupId( const QString& name )
{
  return getId( "smartgroup", name );
}

int QgsStyle::addSmartgroup( const QString& name, const QString& op, const QgsSmartConditionMap& conditions )
{
  QDomDocument doc( "dummy" );
  QDomElement smartEl = doc.createElement( "smartgroup" );
  smartEl.setAttribute( "name", name );
  smartEl.setAttribute( "operator", op );

  QStringList constraints;
  constraints << "tag" << "group" << "name" << "!tag" << "!group" << "!name";

  Q_FOREACH ( const QString &constraint, constraints )
  {
    QStringList parameters = conditions.values( constraint );
    Q_FOREACH ( const QString &param, parameters )
    {
      QDomElement condEl = doc.createElement( "condition" );
      condEl.setAttribute( "constraint", constraint );
      condEl.setAttribute( "param", param );
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
    return static_cast< int >( sqlite3_last_insert_rowid( mCurrentDB ) );
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
  sqlite3_stmt *ppStmt;
  int nError = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );

  QgsSymbolGroupMap groupNames;
  while ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QString group = QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt, SmartgroupName ) ) );
    groupNames.insert( sqlite3_column_int( ppStmt, SmartgroupId ), group );
  }

  sqlite3_finalize( ppStmt );

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
  sqlite3_stmt *ppStmt;
  int nError = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );

  QStringList groups;
  while ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    groups << QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt, 0 ) ) );
  }

  sqlite3_finalize( ppStmt );

  return groups;
}

QStringList QgsStyle::symbolsOfSmartgroup( StyleEntity type, int id )
{
  QStringList symbols;

  char *query = sqlite3_mprintf( "SELECT xml FROM smartgroup WHERE id=%d", id );

  sqlite3_stmt *ppStmt;
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );
  if ( !( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW ) )
  {
    sqlite3_finalize( ppStmt );
    return QStringList();
  }
  else
  {
    QDomDocument doc;
    QString xmlstr = QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt, 0 ) ) );
    if ( !doc.setContent( xmlstr ) )
    {
      QgsDebugMsg( QString( "Cannot open smartgroup id: %1" ).arg( id ) );
    }
    QDomElement smartEl = doc.documentElement();
    QString op = smartEl.attribute( "operator" );
    QDomNodeList conditionNodes = smartEl.childNodes();

    bool firstSet = true;
    for ( int i = 0; i < conditionNodes.count(); i++ )
    {
      QDomElement condEl = conditionNodes.at( i ).toElement();
      QString constraint = condEl.attribute( "constraint" );
      QString param = condEl.attribute( "param" );

      QStringList resultNames;
      // perform suitable action for the given constraint
      if ( constraint == "tag" )
      {
        resultNames = symbolsWithTag( type, tagId( param ) );
      }
      else if ( constraint == "group" )
      {
        // XXX Validating group id might be a good idea here
        resultNames = symbolsOfGroup( type, groupId( param ) );

      }
      else if ( constraint == "name" )
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
      else if ( constraint == "!tag" )
      {
        resultNames = type == SymbolEntity ? symbolNames() : colorRampNames();
        QStringList unwanted = symbolsWithTag( type, tagId( param ) );
        Q_FOREACH ( const QString& name, unwanted )
        {
          resultNames.removeAll( name );
        }
      }
      else if ( constraint == "!group" )
      {
        resultNames = type == SymbolEntity ? symbolNames() : colorRampNames();
        QStringList unwanted = symbolsOfGroup( type, groupId( param ) );
        Q_FOREACH ( const QString& name, unwanted )
        {
          resultNames.removeAll( name );
        }
      }
      else if ( constraint == "!name" )
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
        if ( op == "OR" )
        {
          symbols << resultNames;
        }
        else if ( op == "AND" )
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

  sqlite3_finalize( ppStmt );

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

  sqlite3_stmt *ppStmt;
  int nError = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );
  if ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QDomDocument doc;
    QString xmlstr = QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt, 0 ) ) );
    if ( !doc.setContent( xmlstr ) )
    {
      QgsDebugMsg( QString( "Cannot open smartgroup id: %1" ).arg( id ) );
    }

    QDomElement smartEl = doc.documentElement();
    QDomNodeList conditionNodes = smartEl.childNodes();

    for ( int i = 0; i < conditionNodes.count(); i++ )
    {
      QDomElement condEl = conditionNodes.at( i ).toElement();
      QString constraint = condEl.attribute( "constraint" );
      QString param = condEl.attribute( "param" );

      condition.insert( constraint, param );
    }
  }

  sqlite3_finalize( ppStmt );

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

  sqlite3_stmt *ppStmt;
  int nError = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, nullptr );
  if ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QDomDocument doc;
    QString xmlstr = QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( ppStmt, 0 ) ) );
    if ( !doc.setContent( xmlstr ) )
    {
      QgsDebugMsg( QString( "Cannot open smartgroup id: %1" ).arg( id ) );
    }
    QDomElement smartEl = doc.documentElement();
    op = smartEl.attribute( "operator" );
  }

  sqlite3_finalize( ppStmt );

  return op;
}

bool QgsStyle::exportXml( const QString& filename )
{
  if ( filename.isEmpty() )
  {
    QgsDebugMsg( "Invalid filename for style export." );
    return false;
  }

  QDomDocument doc( "qgis_style" );
  QDomElement root = doc.createElement( "qgis_style" );
  root.setAttribute( "version", STYLE_CURRENT_VERSION );
  doc.appendChild( root );

  // TODO work on the groups and tags
  QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( mSymbols, "symbols", doc );
  QDomElement rampsElem = doc.createElement( "colorramps" );

  // save color ramps
  for ( QMap<QString, QgsColorRamp*>::const_iterator itr = mColorRamps.constBegin(); itr != mColorRamps.constEnd(); ++itr )
  {
    QDomElement rampEl = QgsSymbolLayerUtils::saveColorRamp( itr.key(), itr.value(), doc );
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

bool QgsStyle::importXml( const QString& filename )
{
  mErrorString = QString();
  QDomDocument doc( "style" );
  QFile f( filename );
  if ( !f.open( QFile::ReadOnly ) )
  {
    mErrorString = "Unable to open the specified file";
    QgsDebugMsg( "Error opening the style XML file." );
    return false;
  }

  if ( !doc.setContent( &f ) )
  {
    mErrorString = QString( "Unable to understand the style file: %1" ).arg( filename );
    QgsDebugMsg( "XML Parsing error" );
    f.close();
    return false;
  }
  f.close();

  QDomElement docEl = doc.documentElement();
  if ( docEl.tagName() != "qgis_style" )
  {
    mErrorString = "Incorrect root tag in style: " + docEl.tagName();
    return false;
  }

  QString version = docEl.attribute( "version" );
  if ( version != STYLE_CURRENT_VERSION && version != "0" )
  {
    mErrorString = "Unknown style file version: " + version;
    return false;
  }

  QgsSymbolMap symbols;

  QDomElement symbolsElement = docEl.firstChildElement( "symbols" );
  QDomElement e = symbolsElement.firstChildElement();

  if ( version == STYLE_CURRENT_VERSION )
  {
    // For the new style, load symbols individualy
    while ( !e.isNull() )
    {
      if ( e.tagName() == "symbol" )
      {
        QgsSymbol* symbol = QgsSymbolLayerUtils::loadSymbol( e );
        if ( symbol )
        {
          symbols.insert( e.attribute( "name" ), symbol );
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
    symbols = QgsSymbolLayerUtils::loadSymbols( symbolsElement );
  }

  // save the symbols with proper name
  for ( QMap<QString, QgsSymbol*>::iterator it = symbols.begin(); it != symbols.end(); ++it )
  {
    addSymbol( it.key(), it.value() );
  }

  // load color ramps
  QDomElement rampsElement = docEl.firstChildElement( "colorramps" );
  e = rampsElement.firstChildElement();
  while ( !e.isNull() )
  {
    if ( e.tagName() == "colorramp" )
    {
      QgsColorRamp* ramp = QgsSymbolLayerUtils::loadColorRamp( e );
      if ( ramp )
      {
        addColorRamp( e.attribute( "name" ), ramp );
      }
    }
    else
    {
      QgsDebugMsg( "unknown tag: " + e.tagName() );
    }
    e = e.nextSiblingElement();
  }

  mFileName = filename;
  return true;
}

bool QgsStyle::updateSymbol( StyleEntity type, const QString& name )
{
  QDomDocument doc( "dummy" );
  QDomElement symEl;
  QByteArray xmlArray;
  QTextStream stream( &xmlArray );
  stream.setCodec( "UTF-8" );

  char *query;

  if ( type == SymbolEntity )
  {
    // check if it is an existing symbol
    if ( !symbolNames().contains( name ) )
    {
      QgsDebugMsg( "Update request received for unavailable symbol" );
      return false;
    }

    symEl = QgsSymbolLayerUtils::saveSymbol( name, symbol( name ), doc );
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

    QScopedPointer< QgsColorRamp > ramp( colorRamp( name ) );
    symEl = QgsSymbolLayerUtils::saveColorRamp( name, ramp.data(), doc );
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
