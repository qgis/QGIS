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

  // delete previous symbol (if any)
  if ( mSymbols.contains( name ) )
    delete mSymbols.value( name );

  mSymbols.insert( name, symbol );
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
  sqlite3_stmt *ppStmt;
  int nError;
  char *query;

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
  char *query = sqlite3_mprintf( "SELECT name FROM symbol WHERE groupid=%d;", groupid );
  int nErr = sqlite3_prepare_v2( db, query, -1, &ppStmt, NULL );
  while ( nErr == SQLITE_OK && sqlite3_step( ppStmt ) == SQLITE_ROW )
  {
    QString symbol = QString( reinterpret_cast<const char*>( sqlite3_column_text( ppStmt, 0 ) ) );
    symbols.append( symbol );
  }

  return symbols;
}
