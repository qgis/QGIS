/***************************************************************************
    qgsstylev2.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstylev2.h"

#include "qgssymbolv2.h"
#include "qgsvectorcolorrampv2.h"

#include "qgssymbollayerv2registry.h"

#include "qgsapplication.h"
#include "qgslogger.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QTextStream>
#include <QByteArray>

#include <sqlite3.h>

#define STYLE_CURRENT_VERSION  "0"

QgsStyleV2* QgsStyleV2::mDefaultStyle = NULL;


QgsStyleV2::QgsStyleV2()
{
}

QgsStyleV2::~QgsStyleV2()
{
  clear();
}

QgsStyleV2* QgsStyleV2::defaultStyle() // static
{
  if ( mDefaultStyle == NULL )
  {
    QString styleFilename = QgsApplication::userStyleV2Path();

    // copy default style if user style doesn't exist
    if ( !QFile::exists( styleFilename ) )
    {
      QFile::copy( QgsApplication::defaultStyleV2Path(), styleFilename );
    }

    mDefaultStyle = new QgsStyleV2;
    mDefaultStyle->load( styleFilename );
  }
  return mDefaultStyle;
}


void QgsStyleV2::clear()
{
  for ( QMap<QString, QgsSymbolV2*>::iterator its = mSymbols.begin(); its != mSymbols.end(); ++its )
    delete its.value();
  for ( QMap<QString, QgsVectorColorRampV2*>::iterator itr = mColorRamps.begin(); itr != mColorRamps.end(); ++itr )
    delete itr.value();

  mSymbols.clear();
  mColorRamps.clear();

  sqlite3_close( mCurrentDB );
}

bool QgsStyleV2::addSymbol( QString name, QgsSymbolV2* symbol )
{
  if ( !symbol || name.count() == 0 )
    return false;

  if ( mSymbols.contains( name ) )
    delete mSymbols.value( name );

  mSymbols.insert( name, symbol );
  saveSymbol( name, symbol, 0, QStringList() );

  return true;
}

bool QgsStyleV2::saveSymbol( QString name, QgsSymbolV2* symbol, int groupid, QStringList tags )
{
  // TODO add support for tags and groups
  Q_UNUSED( groupid );
  Q_UNUSED( tags );

  QDomDocument doc( "dummy" );
  QDomElement symEl = QgsSymbolLayerV2Utils::saveSymbol( name, symbol, doc );
  if ( symEl.isNull() )
  {
    QgsDebugMsg( "Couldnot convert symbol to valid XML!" );
    return false;
  }

  QByteArray *xmlArray = new QByteArray();
  QTextStream stream( xmlArray );
  symEl.save( stream, 4 );
  QByteArray nameArray = name.toUtf8();
  char *query = sqlite3_mprintf( "INSERT INTO symbol VALUES (NULL, '%q', '%q', NULL);",
      nameArray.constData(), xmlArray->constData() );

  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( "Couldnot insert symbol into the Database!" );
    return false;
  }
  return true;
}

bool QgsStyleV2::removeSymbol( QString name )
{
  if ( !mSymbols.contains( name ) )
    return false;

  // remove from map and delete
  delete mSymbols.take( name );

  // TODO
  // Simplify this work here, its STUPID to run two DB queries for the sake of remove()
  if ( mCurrentDB == NULL )
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

QgsSymbolV2* QgsStyleV2::symbol( QString name )
{
  if ( !mSymbols.contains( name ) )
    return NULL;
  return mSymbols[name]->clone();
}

const QgsSymbolV2* QgsStyleV2::symbolRef( QString name ) const
{
  if ( !mSymbols.contains( name ) )
    return NULL;
  return mSymbols[name];
}

int QgsStyleV2::symbolCount()
{
  return mSymbols.count();
}

QStringList QgsStyleV2::symbolNames()
{
  return mSymbols.keys();
}


bool QgsStyleV2::addColorRamp( QString name, QgsVectorColorRampV2* colorRamp )
{
  if ( !colorRamp || name.count() == 0 )
    return false;

  // delete previous symbol (if any)
  if ( mColorRamps.contains( name ) )
    delete mColorRamps.value( name );

  // insert it into the database
  QDomDocument doc( "dummy" );
  QDomElement rampEl = QgsSymbolLayerV2Utils::saveColorRamp( name, colorRamp, doc );
  if ( rampEl.isNull() )
  {
    QgsDebugMsg( "Couldnot convert color ramp to valid XML!" );
    return false;
  }

  QByteArray *xmlArray = new QByteArray();
  QTextStream stream( xmlArray );
  rampEl.save( stream, 4 );
  QByteArray nameArray = name.toUtf8();
  char *query = sqlite3_mprintf( "INSERT INTO colorramp VALUES (NULL, '%q', '%q');",
      nameArray.constData(), xmlArray->constData() );

  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( "Couldnot insert colorramp into the Database!" );
    return false;
  }

  mColorRamps.insert( name, colorRamp );
  return true;
}

bool QgsStyleV2::removeColorRamp( QString name )
{
  if ( !mColorRamps.contains( name ) )
    return false;

  QByteArray array = name.toUtf8();
  char *query = sqlite3_mprintf( "DELETE FROM colorramp WHERE name='%q';", array.constData() );
  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( "Couldn't remove color ramp from the database." );
    return false;
  }
  // remove from map and delete
  delete mColorRamps.take( name );
  return true;
}

QgsVectorColorRampV2* QgsStyleV2::colorRamp( QString name )
{
  if ( !mColorRamps.contains( name ) )
    return NULL;
  return mColorRamps[name]->clone();
}

const QgsVectorColorRampV2* QgsStyleV2::colorRampRef( QString name ) const
{
  if ( !mColorRamps.contains( name ) )
    return NULL;
  return mColorRamps[name];
}

int QgsStyleV2::colorRampCount()
{
  return mColorRamps.count();
}

QStringList QgsStyleV2::colorRampNames()
{
  return mColorRamps.keys();
}

bool QgsStyleV2::openDB( QString filename )
{
  int rc;

  QByteArray namearray = filename.toUtf8();
  rc = sqlite3_open( namearray.constData(), &mCurrentDB );
  if ( rc )
  {
    mErrorString = "Couldn't open the style Database: " + QString( sqlite3_errmsg( mCurrentDB ) );
    sqlite3_close( mCurrentDB );
    return false;
  }

  return true;
}

bool QgsStyleV2::load( QString filename )
{
  mErrorString = QString();

  // Open the sqlite database
  if ( !openDB( filename ) )
    return false;
  // First create all the Main symbols
  sqlite3_stmt *ppStmt;
  const char *query = "SELECT * FROM symbol;";
  int nError = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  while ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QDomDocument doc;
    QString symbol_name = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, SymbolName ) ) );
    QString xmlstring = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, SymbolXML ) ) );
    if ( !doc.setContent( xmlstring ) )
    {
      QgsDebugMsg( "Cannot open symbol" + symbol_name );
      continue;
    }
    QDomElement symElement = doc.documentElement();
    QgsSymbolV2 *symbol = QgsSymbolLayerV2Utils::loadSymbol( symElement );
    if ( symbol != NULL )
      mSymbols.insert( symbol_name, symbol );
  }
  sqlite3_finalize( ppStmt );

  const char *rquery = "SELECT * FROM colorramp;";
  nError = sqlite3_prepare_v2( mCurrentDB, rquery, -1, &ppStmt, NULL );
  while ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QDomDocument doc;
    QString ramp_name = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, ColorrampName ) ) );
    QString xmlstring = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, ColorrampXML ) ) );
    if( !doc.setContent( xmlstring ) )
    {
      QgsDebugMsg( "Cannot open symbol" + ramp_name );
      continue;
    }
    QDomElement rampElement = doc.documentElement();
    QgsVectorColorRampV2* ramp = QgsSymbolLayerV2Utils::loadColorRamp( rampElement );
    if ( ramp != NULL )
      mColorRamps.insert( ramp_name, ramp );
  }

  mFileName = filename;
  return true;
}



bool QgsStyleV2::save( QString filename )
{
  mErrorString = QString();
  if ( filename.isEmpty() )
    filename = mFileName;

  // TODO evaluate the requirement of this function and change implementation accordingly

  /*
  QDomDocument doc( "qgis_style" );
  QDomElement root = doc.createElement( "qgis_style" );
  root.setAttribute( "version", STYLE_CURRENT_VERSION );
  doc.appendChild( root );

  QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols( mSymbols, "symbols", doc );

  QDomElement rampsElem = doc.createElement( "colorramps" );

  // save color ramps
  for ( QMap<QString, QgsVectorColorRampV2*>::iterator itr = mColorRamps.begin(); itr != mColorRamps.end(); ++itr )
  {
    QDomElement rampEl = QgsSymbolLayerV2Utils::saveColorRamp( itr.key(), itr.value(), doc );
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
  doc.save( ts, 2 );
  f.close();
  */

  mFileName = filename;
  return true;
}

bool QgsStyleV2::renameSymbol( QString oldName, QString newName )
{
  if ( !mSymbols.contains( oldName ) )
    return NULL;

  mSymbols.insert( newName, mSymbols.take( oldName ) );

  if ( mCurrentDB == NULL )
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

bool QgsStyleV2::renameColorRamp( QString oldName, QString newName )
{
  if ( !mColorRamps.contains( oldName ) )
    return NULL;

  mColorRamps.insert( newName, mColorRamps.take( oldName ) );

  int rampid = 0;
  char *query;
  sqlite3_stmt *ppStmt;
  QByteArray array = oldName.toUtf8();
  query = sqlite3_mprintf( "SELECT id FROM colorramp WHERE name='%q';", array.constData() );
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  if ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    rampid = sqlite3_column_int( ppStmt, 0 );
  }
  sqlite3_finalize( ppStmt );
  rename( ColorrampEntity, rampid, newName );

  return true;
}

QStringList QgsStyleV2::groupNames()
{
  QStringList groupNames;
  sqlite3_stmt *ppStmt;
  const char *query = "SELECT * FROM symgroup;";
  int nError = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  while ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QString group = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, SymgroupName ) ) );
    groupNames.append( group );
  }
  sqlite3_finalize( ppStmt );
  return groupNames;
}

QgsSymbolGroupMap QgsStyleV2::childGroupNames( QString parent )
{
  // get the name list from the sqlite database and return as a QStringList
  if( mCurrentDB == NULL )
  {
    QgsDebugMsg( "Cannot open database for listing groups" );
    return QgsSymbolGroupMap();
  }
  char *query;
  int nError;
  sqlite3_stmt *ppStmt;

  // decide the query to be run based on parent group
  if ( parent == "" || parent == QString() )
  {
    query = sqlite3_mprintf( "SELECT * FROM symgroup WHERE parent IS NULL;" );
  }
  else
  {
    QByteArray parentArray = parent.toUtf8();
    char *subquery = sqlite3_mprintf( "SELECT * FROM symgroup WHERE name='%q';", parentArray.constData() );
    nError = sqlite3_prepare_v2( mCurrentDB, subquery, -1, &ppStmt, NULL );
    if ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
    {
      query = sqlite3_mprintf( "SELECT * FROM symgroup WHERE parent=%d;", sqlite3_column_int( ppStmt, SymgroupId ) );
    }
    sqlite3_finalize( ppStmt );
  }

  QgsSymbolGroupMap groupNames;

  // Now run the query and retrive the group names
  nError = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  while ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QString group = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, SymgroupName ) ) );
    groupNames.insert( sqlite3_column_int( ppStmt, SymgroupId ), group );
  }
  sqlite3_finalize( ppStmt );
  return groupNames;
}

QStringList QgsStyleV2::symbolsOfGroup( int groupid )
{
  if( mCurrentDB == NULL )
  {
    QgsDebugMsg( "Cannot Open database for getting group symbols of groupid: " + groupid );
    return QStringList();
  }

  QStringList symbols;
  sqlite3_stmt *ppStmt;
  char *query;
  query = groupid ? sqlite3_mprintf( "SELECT name FROM symbol WHERE groupid=%d;", groupid ) :
          sqlite3_mprintf( "SELECT name FROM symbol WHERE groupid IS NULL;");
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QString symbol = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, 0 ) ) );
    symbols.append( symbol );
  }
  sqlite3_finalize( ppStmt );

  return symbols;
}

QStringList QgsStyleV2::symbolsWithTag( int tagid )
{
  QStringList symbols;
  if ( mCurrentDB == NULL )
  {
    QgsDebugMsg( "Cannot open database to get symbols of tagid " + tagid );
    return QStringList();
  }
  char *subquery = sqlite3_mprintf( "SELECT symbol_id FROM tagmap WHERE tag_id=%d;", tagid );
  sqlite3_stmt *ppStmt;
  int nErr = sqlite3_prepare_v2( mCurrentDB, subquery, -1, &ppStmt, NULL );

  // get the symbol <-> tag connection from table 'tagmap'
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    int symbolId = sqlite3_column_int( ppStmt, 0 );
    sqlite3_stmt *ppStmt2;
    char *query = sqlite3_mprintf( "SELECT name FROM symbol WHERE id=%d;", symbolId );
    int sErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt2, NULL );
    while ( sErr == SQLITE_OK && sqlite3_step( ppStmt2 ) == SQLITE_ROW )
    {
      QString symbolName = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt2, 0 ) ) );
      symbols.append( symbolName );
    }
    sqlite3_finalize( ppStmt2 );
  }
  sqlite3_finalize( ppStmt );

  return symbols;
}

int QgsStyleV2::addGroup( QString groupName, int parentid )
{
  if ( mCurrentDB == NULL )
    return 0;
  sqlite3_stmt *ppStmt;

  char *query;
  QByteArray groupArray = groupName.toUtf8();
  if ( parentid == 0 )
  {
    query = sqlite3_mprintf( "INSERT INTO symgroup VALUES (NULL, '%q', NULL);", groupArray.constData() );
  }
  else
  {
    query = sqlite3_mprintf( "INSERT INTO symgroup VALUES (NULL, '%q', %d);", groupArray.constData(), parentid );
  }
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  if ( nErr == SQLITE_OK )
    sqlite3_step( ppStmt );
  sqlite3_finalize( ppStmt );

  return (int)sqlite3_last_insert_rowid( mCurrentDB );
}

int QgsStyleV2::addTag( QString tagname )
{
  if ( mCurrentDB == NULL )
    return 0;
  sqlite3_stmt *ppStmt;

  QByteArray tagArray = tagname.toUtf8();
  char *query = sqlite3_mprintf( "INSERT INTO tag VALUES (NULL, '%q');", tagArray.constData() );
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  if ( nErr == SQLITE_OK )
    sqlite3_step( ppStmt );
  sqlite3_finalize( ppStmt );

  return (int)sqlite3_last_insert_rowid( mCurrentDB );
}

void QgsStyleV2::rename( StyleEntity type, int id, QString newName )
{
  QByteArray nameArray = newName.toUtf8();
  char *query;
  switch ( type )
  {
    case SymbolEntity : query = sqlite3_mprintf( "UPDATE symbol SET name='%q' WHERE id=%d;", nameArray.constData(), id );
                        break;
    case GroupEntity  : query = sqlite3_mprintf( "UPDATE symgroup SET name='%q' WHERE id=%d;", nameArray.constData(), id );
                        break;
    case TagEntity    : query = sqlite3_mprintf( "UPDATE tag SET name='%q' WHERE id=%d;", nameArray.constData(), id );
                        break;
    case ColorrampEntity  : query = sqlite3_mprintf( "UPDATE colorramp SET name='%q' WHERE id=%d;", nameArray.constData(), id );
                        break;
    default           : QgsDebugMsg( "Invalid Style Entity indicated" );
                        return;
  }
  if ( !runEmptyQuery( query ) )
    mErrorString = "Could not rename!";
}

char* QgsStyleV2::getGroupRemoveQuery( int id )
{
  int parentid;
  char *query = sqlite3_mprintf( "SELECT parent FROM symgroup WHERE id=%d;", id );
  sqlite3_stmt *ppStmt;
  int err = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  if ( err == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
    parentid = sqlite3_column_int( ppStmt, 0 );
  sqlite3_finalize( ppStmt );
  if ( parentid )
  {
    query = sqlite3_mprintf( "UPDATE symbol SET groupid=%d WHERE groupid=%d;"
        "UPDATE symgroup SET parent=%d WHERE parent=%d;"
        "DELETE FROM symgroup WHERE id=%d;", parentid, id, parentid, id, id );
  }
  else
  {
    query = sqlite3_mprintf( "UPDATE symbol SET groupid=NULL WHERE groupid=%d;"
         "UPDATE symgroup SET parent=NULL WHERE parent=%d;"
         "DELETE FROM symgroup WHERE id=%d;", id, id, id );
  }
  return query;
}

void QgsStyleV2::remove( StyleEntity type, int id )
{
  char *query;
  switch ( type )
  {
    case SymbolEntity : query = sqlite3_mprintf( "DELETE FROM symbol WHERE id=%d; DELETE FROM tagmap WHERE symbol_id=%d;", id, id );
                        break;
    case GroupEntity  : query = getGroupRemoveQuery( id );
                        break;
    case TagEntity    : query = sqlite3_mprintf( "DELETE FROM tag WHERE id=%d; DELETE FROM tagmap WHERE tag_id=%d;", id, id );
                        break;
    case ColorrampEntity  : query = sqlite3_mprintf( "DELETE FROM colorramp WHERE id=%d;", id );
                        break;
    default           : QgsDebugMsg( "Invalid Style Entity indicated" );
                        return;
  }
  if ( !runEmptyQuery( query ) )
    QgsDebugMsg( "Could not delete entity!" );

}

bool QgsStyleV2::runEmptyQuery( char *query )
{
  char *zErr = 0;
  if ( mCurrentDB == NULL )
    return false;
  int nErr = sqlite3_exec( mCurrentDB, query, NULL, NULL, &zErr );
  if ( nErr )
  {
    QgsDebugMsg( zErr );
    return false;
  }
  return true;
}

bool QgsStyleV2::regroup( QString symbolName, int groupid )
{
  QByteArray array = symbolName.toUtf8();
  char *query;
  query = groupid ? sqlite3_mprintf( "UPDATE symbol SET groupid=%d WHERE name='%q';", groupid, array.constData() )
                  : sqlite3_mprintf( "UPDATE symbol SET groupid=NULL WHERE name='%q';", array.constData() );
  return runEmptyQuery( query );
}

QStringList QgsStyleV2::findSymbols( QString qword )
{
  if ( mCurrentDB == NULL )
  {
    QgsDebugMsg( "Sorry! Cannot open database to search" );
    return QStringList();
  }

  QStringList symbols;
  QStringList tagids;
  QStringList symbolids;

  char *query;
  sqlite3_stmt *ppStmt;
  QByteArray array = qword.toUtf8();

  query = sqlite3_mprintf( "SELECT name FROM symbol WHERE xml LIKE '%%%q%%';", array.constData() );
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QString symbolName = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, 0 ) ) );
    symbols.append( symbolName );
  }
  sqlite3_finalize( ppStmt );

  query = sqlite3_mprintf( "SELECT id FROM tag WHERE name LIKE '%%%q%%';", array.constData() );
  nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    tagids.append( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, 0 ) ) );
  }
  sqlite3_finalize( ppStmt );

  QString dummy = tagids.join( ", " );
  QByteArray dummyArray = dummy.toUtf8();

  query = sqlite3_mprintf( "SELECT symbol_id FROM tagmap WHERE tag_id IN (%q);", dummyArray.constData() );
  nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    symbolids.append( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, 0 ) ) );
  }
  sqlite3_finalize( ppStmt );

  dummy = symbolids.join( ", " );
  dummyArray = dummy.toUtf8();
  query = sqlite3_mprintf( "SELECT name FROM symbol WHERE id IN (%q);", dummyArray.constData() );
  nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QString symbolName = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, 0 ) ) );
    if ( !symbols.contains( symbolName ) )
      symbols.append( symbolName );
  }
  sqlite3_finalize( ppStmt );

  return symbols;
}

bool QgsStyleV2::tagSymbol( QString symbol, QStringList tags )
{
  if ( mCurrentDB == NULL )
  {
    QgsDebugMsg( "Sorry! Cannot open database to tag." );
    return false;
  }

  int symbolid = symbolId( symbol );
  if ( !symbolid )
  {
    QgsDebugMsg( "No such symbol for tagging in database: " + symbol );
    return false;
  }

  char *query;
  int nErr;
  sqlite3_stmt *ppStmt;

  foreach ( const QString &tag, tags )
  {
    int tagid;
    char *zErr = 0;
    QByteArray tagArray = tag.toUtf8();
    // sql: gets the id of the tag if present or insert the tag and get the id of the tag
    query = sqlite3_mprintf( "SELECT id FROM tag WHERE name='%q';", tagArray.constData() );
    nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
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
    query = sqlite3_mprintf( "INSERT INTO tagmap VALUES (%d,%d);", tagid, symbolid );
    nErr = sqlite3_exec( mCurrentDB, query, NULL, NULL, &zErr );
    if ( nErr )
      QgsDebugMsg( zErr );
  }
  return true;
}

bool QgsStyleV2::detagSymbol( QString symbol, QStringList tags )
{
  QByteArray array = symbol.toUtf8();
  char *query;
  int symbolid;
  if ( mCurrentDB == NULL )
  {
    QgsDebugMsg( "Sorry! Cannot open database for detgging." );
    return false;
  }
  query = sqlite3_mprintf( "SELECT id FROM symbol WHERE name='%q';", array.constData() );
  sqlite3_stmt *ppStmt;
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  if ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    symbolid = sqlite3_column_int( ppStmt, 0 );
  }
  sqlite3_finalize( ppStmt );

  foreach ( const QString &tag, tags )
  {
    int tagid = 0;
    QByteArray tagArray = tag.toUtf8();
    query = sqlite3_mprintf( "SELECT id FROM tag WHERE name='%q';", tagArray.constData() );
    sqlite3_stmt *ppStmt2;
    nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt2, NULL );
    if ( nErr == SQLITE_OK && sqlite3_step( ppStmt2 ) == SQLITE_ROW )
    {
      tagid = sqlite3_column_int( ppStmt2, 0 );
    }
    sqlite3_finalize( ppStmt2 );

    if ( tagid )
    {
      // remove from the tagmap
      query = sqlite3_mprintf( "DELETE FROM tagmap WHERE tag_id=%d AND symbol_id=%d;", tagid, symbolid );
      runEmptyQuery( query );
    }
  }

  // TODO Perform tag cleanup 
  // check the number of entries for a given tag in the tagmap
  // if the count is 0, then remove( TagEntity, tagid )
  return true;
}

QStringList QgsStyleV2::tagsOfSymbol( QString symbol )
{
  if ( mCurrentDB == NULL )
  {
    QgsDebugMsg( "Sorry! Cannot open database for getting the tags." );
    return QStringList();
  }
  QStringList tagList;
  char *query;
  sqlite3_stmt *ppStmt;
  int symbolid = symbolId( symbol );
  if ( !symbolid )
    return QStringList();

  // get the ids of tags for the symbol
  query = sqlite3_mprintf( "SELECT tag_id FROM tagmap WHERE symbol_id=%d;", symbolid );
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    int tagid = sqlite3_column_int( ppStmt, 0 );
    char *subquery;
    sqlite3_stmt *ppStmt2;
    subquery = sqlite3_mprintf( "SELECT name FROM tag WHERE id=%d;", tagid );
    int pErr = sqlite3_prepare_v2( mCurrentDB, subquery, -1, &ppStmt2, NULL );
    if ( pErr == SQLITE_OK && sqlite3_step( ppStmt2 ) == SQLITE_ROW )
    {
      QString tag = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt2, 0 ) ) );
      tagList.append( tag );
    }
    sqlite3_finalize( ppStmt2 );
  }
  sqlite3_finalize( ppStmt );

  return tagList;
}

int QgsStyleV2::symbolId( QString name )
{
  int symbolid = 0;
  char *query;
  sqlite3_stmt *ppStmt;
  QByteArray array = name.toUtf8();
  query = sqlite3_mprintf( "SELECT id FROM symbol WHERE name='%q';", array.constData() );
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  if ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    symbolid = sqlite3_column_int( ppStmt, 0 );
  }
  sqlite3_finalize( ppStmt );
  return symbolid;
}

int QgsStyleV2::groupId( QString name )
{
  int groupid = 0;
  char *query;
  sqlite3_stmt *ppStmt;
  QByteArray array = name.toUtf8();
  query = sqlite3_mprintf( "SELECT id FROM symgroup WHERE name='%q';", array.constData() );
  int nErr = sqlite3_prepare_v2( mCurrentDB, query, -1, &ppStmt, NULL );
  if ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    groupid = sqlite3_column_int( ppStmt, 0 );
  }
  sqlite3_finalize( ppStmt );
  return groupid;
}
