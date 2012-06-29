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
}

bool QgsStyleV2::addSymbol( QString name, QgsSymbolV2* symbol )
{
  if ( !symbol || name.count() == 0 )
    return false;

  if ( mSymbols.contains( name ) )
    delete mSymbols.value( name );

  mSymbols.insert( name, symbol );
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

  mColorRamps.insert( name, colorRamp );
  return true;
}

bool QgsStyleV2::removeColorRamp( QString name )
{
  if ( !mColorRamps.contains( name ) )
    return false;

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

sqlite3* QgsStyleV2::openDB( QString filename )
{
  sqlite3 *db;
  int rc;

  QByteArray namearray = filename.toUtf8();
  rc = sqlite3_open( namearray.constData(), &db );
  if ( rc )
  {
    mErrorString = "Couldn't open the style DB: " + QString( sqlite3_errmsg( db ) );
    sqlite3_close( db );
    return NULL;
  }

  return db;
}

bool QgsStyleV2::load( QString filename )
{
  mErrorString = QString();

  // Open the sqlite DB
  sqlite3* db = openDB( filename );
  if ( db == NULL )
    return false;
  // First create all the Main symbols
  sqlite3_stmt *ppStmt;
  const char *query = "SELECT * FROM symbol;";
  int nError = sqlite3_prepare_v2( db, query, -1, &ppStmt, NULL );
  while ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QDomDocument doc;
    QString symbol_name = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, SymbolName ) ) );
    QString xmlstring = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, SymbolXML ) ) );
    if( !doc.setContent( xmlstring ) )
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
  nError = sqlite3_prepare_v2( db, rquery, -1, &ppStmt, NULL );
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
      addColorRamp( ramp_name, ramp );
  }

  sqlite3_close( db );
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
  return true;
}

bool QgsStyleV2::renameColorRamp( QString oldName, QString newName )
{
  if ( !mColorRamps.contains( oldName ) )
    return NULL;

  mColorRamps.insert( newName, mColorRamps.take( oldName ) );
  return true;
}

QgsSymbolGroupMap QgsStyleV2::groupNames( QString parent )
{
  QgsDebugMsg( "Request for groupNames for parent " + parent );
  // get the name list from the sqlite db and return as a QStringList
  sqlite3* db = openDB( mFileName );
  if( db == NULL )
  {
    QgsDebugMsg( "Cannot open database for listing groups" );
    return QgsSymbolGroupMap();
  }
  char *query;
  int nError;
  sqlite3_stmt *ppStmt;

  if ( parent == "" || parent == QString() )
  {
    query = sqlite3_mprintf( "SELECT * FROM symgroup WHERE parent IS NULL;" );
  }
  else
  {
    QByteArray parentArray = parent.toUtf8();
    char *subquery = sqlite3_mprintf( "SELECT * FROM symgroup WHERE name='%q';", parentArray.constData() );
    nError = sqlite3_prepare_v2( db, subquery, -1, &ppStmt, NULL );
    if ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
    {
      query = sqlite3_mprintf( "SELECT * FROM symgroup WHERE parent=%d;", sqlite3_column_int( ppStmt, SymgroupId ) );
    }
    sqlite3_finalize( ppStmt );
  }

  QgsSymbolGroupMap groupNames;

  nError = sqlite3_prepare_v2( db, query, -1, &ppStmt, NULL );
  while ( nError == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QString group = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, SymgroupName ) ) );
    groupNames.insert( sqlite3_column_int( ppStmt, SymgroupId ), group );
  }
  sqlite3_finalize( ppStmt );
  sqlite3_close( db );
  return groupNames;
}

QStringList QgsStyleV2::symbolsOfGroup( int groupid )
{
  sqlite3 *db = openDB( mFileName );
  if( db == NULL )
  {
    QgsDebugMsg( "Cannot Open Db for getting group symbols of groupid: " + groupid );
    return QStringList();
  }

  QStringList symbols;
  sqlite3_stmt *ppStmt;
  char *query;
  query = groupid ? sqlite3_mprintf( "SELECT name FROM symbol WHERE groupid=%d;", groupid ) :
          sqlite3_mprintf( "SELECT name FROM symbol WHERE groupid IS NULL;");
  int nErr = sqlite3_prepare_v2( db, query, -1, &ppStmt, NULL );
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QString symbol = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, 0 ) ) );
    symbols.append( symbol );
  }
  sqlite3_finalize( ppStmt );
  sqlite3_close( db );

  return symbols;
}

QgsSymbolTagMap QgsStyleV2::symbolTags()
{
  QgsSymbolTagMap tags;
  sqlite3* db = openDB( mFileName );
  if ( db == NULL )
  {
    QgsDebugMsg( "Cannot open DB to get the tags" );
    return QgsSymbolTagMap();
  }
  sqlite3_stmt *ppStmt;
  char *query = sqlite3_mprintf( "SELECT * FROM tag;" );
  int nErr = sqlite3_prepare_v2( db, query, -1, &ppStmt, NULL );
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QString tag = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, TagName ) ) );
    tags.insert( sqlite3_column_int( ppStmt, TagId ), tag );
  }
  sqlite3_finalize( ppStmt );
  sqlite3_close( db );
  return tags;
}

QStringList QgsStyleV2::symbolsWithTag( int tagid )
{
  QStringList symbols;
  sqlite3 *db = openDB( mFileName );
  if ( db == NULL )
  {
    QgsDebugMsg( "Cannot open DB to get symbols of tagid " + tagid );
    return QStringList();
  }
  char *subquery = sqlite3_mprintf( "SELECT symbol_id FROM tagmap WHERE tag_id=%d;", tagid );
  sqlite3_stmt *ppStmt;
  int nErr = sqlite3_prepare_v2( db, subquery, -1, &ppStmt, NULL );
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    int symbolId = sqlite3_column_int( ppStmt, 0 );
    sqlite3_stmt *ppStmt2;
    char *query = sqlite3_mprintf( "SELECT name FROM symbol WHERE id=%d;", symbolId );
    int sErr = sqlite3_prepare_v2( db, query, -1, &ppStmt2, NULL );
    while ( sErr == SQLITE_OK && sqlite3_step( ppStmt2 ) == SQLITE_ROW )
    {
      QString symbolName = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt2, 0 ) ) );
      symbols.append( symbolName );
    }
    sqlite3_finalize( ppStmt2 );
  }
  sqlite3_finalize( ppStmt );
  sqlite3_close( db );

  return symbols;
}

int QgsStyleV2::addGroup( QString groupName, int parentid )
{
  sqlite3 *db = openDB( mFileName );
  if ( db == NULL )
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
  int nErr = sqlite3_prepare_v2( db, query, -1, &ppStmt, NULL );
  if ( nErr == SQLITE_OK )
    sqlite3_step( ppStmt );
  sqlite3_finalize( ppStmt );

  int groupid = (int)sqlite3_last_insert_rowid( db );
  sqlite3_close( db );
  return groupid;
}

int QgsStyleV2::addTag( QString tagname )
{
  sqlite3 *db = openDB( mFileName );
  if ( db == NULL )
    return 0;
  sqlite3_stmt *ppStmt;

  QByteArray tagArray = tagname.toUtf8();
  char *query = sqlite3_mprintf( "INSERT INTO tag VALUES (NULL, '%q');", tagArray.constData() );
  int nErr = sqlite3_prepare_v2( db, query, -1, &ppStmt, NULL );
  if ( nErr == SQLITE_OK )
    sqlite3_step( ppStmt );
  sqlite3_finalize( ppStmt );

  int tagid = (int)sqlite3_last_insert_rowid( db );
  sqlite3_close( db );
  return tagid;
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
  sqlite3 * db = openDB( mFileName );
  char *query = sqlite3_mprintf( "SELECT parent FROM symgroup WHERE id=%d;", id );
  sqlite3_stmt *ppStmt;
  int err = sqlite3_prepare_v2( db, query, -1, &ppStmt, NULL );
  if ( err == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
    parentid = sqlite3_column_int( ppStmt, 0 );
  sqlite3_finalize( ppStmt );
  sqlite3_close( db );
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
    mErrorString = "Could not delete entity!";

}

bool QgsStyleV2::runEmptyQuery( char *query )
{
  sqlite3 *db = openDB( mFileName );
  char *zErr = 0;
  if ( db == NULL )
    return false;
  int nErr = sqlite3_exec( db, query, NULL, NULL, &zErr );
  if ( nErr )
  {
    QgsDebugMsg( zErr );
    return false;
  }
  sqlite3_close( db );
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
  QByteArray array = qword.toUtf8();
  char *query;
  query = sqlite3_mprintf( "SELECT name FROM symbol WHERE xml LIKE '%%%q%%';", array.constData() );

  QStringList symbols;
  sqlite3 *db = openDB( mFileName );
  if ( db == NULL )
  {
    QgsDebugMsg( "Sorry! Cannot open DB to search" );
    return QStringList();
  }
  sqlite3_stmt *ppStmt;
  int nErr = sqlite3_prepare_v2( db, query, -1, &ppStmt, NULL );
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QString symbolName = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, 0 ) ) );
    symbols.append( symbolName );
  }
  sqlite3_finalize( ppStmt );
  sqlite3_close( db );

  return symbols;
}
