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
#include "qgslegendpatchshape.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgsmarkersymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgsruntimeprofiler.h"
#include "qgsabstract3dsymbol.h"
#include "qgs3dsymbolregistry.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>
#include <QFile>
#include <QTextStream>
#include <QByteArray>
#include <QFileInfo>

#include <sqlite3.h>
#include "qgssqliteutils.h"

#define STYLE_CURRENT_VERSION  "2"

/**
 * Columns available in the legend patch table.
 */
enum LegendPatchTable
{
  LegendPatchTableId, //!< Legend patch ID
  LegendPatchTableName, //!< Legend patch name
  LegendPatchTableXML, //!< Legend patch definition (as XML)
  LegendPatchTableFavoriteId, //!< Legend patch is favorite flag
};

/**
 * Columns available in the 3d symbol table.
 */
enum Symbol3DTable
{
  Symbol3DTableId, //!< 3d symbol ID
  Symbol3DTableName, //!< 3d symbol name
  Symbol3DTableXML, //!< 3d symbol definition (as XML)
  Symbol3DTableFavoriteId, //!< 3d symbol is favorite flag
};


QgsStyle *QgsStyle::sDefaultStyle = nullptr;

QgsStyle::QgsStyle()
{
  std::unique_ptr< QgsSimpleMarkerSymbolLayer > simpleMarker = std::make_unique< QgsSimpleMarkerSymbolLayer >( Qgis::MarkerShape::Circle,
      1.6, 0, Qgis::ScaleMethod::ScaleArea, QColor( 84, 176, 74 ), QColor( 61, 128, 53 ) );
  simpleMarker->setStrokeWidth( 0.4 );
  mPatchMarkerSymbol = std::make_unique< QgsMarkerSymbol >( QgsSymbolLayerList() << simpleMarker.release() );

  std::unique_ptr< QgsSimpleLineSymbolLayer > simpleLine = std::make_unique< QgsSimpleLineSymbolLayer >( QColor( 84, 176, 74 ), 0.6 );
  mPatchLineSymbol = std::make_unique< QgsLineSymbol >( QgsSymbolLayerList() << simpleLine.release() );

  std::unique_ptr< QgsGradientFillSymbolLayer > gradientFill = std::make_unique< QgsGradientFillSymbolLayer >( QColor( 66, 150, 63 ), QColor( 84, 176, 74 ) );
  std::unique_ptr< QgsSimpleLineSymbolLayer > simpleOutline = std::make_unique< QgsSimpleLineSymbolLayer >( QColor( 56, 128, 54 ), 0.26 );
  mPatchFillSymbol = std::make_unique< QgsFillSymbol >( QgsSymbolLayerList() << gradientFill.release() << simpleOutline.release() );
}

QgsStyle::~QgsStyle()
{
  clear();
}

bool QgsStyle::addEntity( const QString &name, const QgsStyleEntityInterface *entity, bool update )
{
  switch ( entity->type() )
  {
    case SymbolEntity:
      if ( !static_cast< const QgsStyleSymbolEntity * >( entity )->symbol() )
        return false;
      return addSymbol( name, static_cast< const QgsStyleSymbolEntity * >( entity )->symbol()->clone(), update );

    case ColorrampEntity:
      if ( !static_cast< const QgsStyleColorRampEntity * >( entity )->ramp() )
        return false;
      return addColorRamp( name, static_cast< const QgsStyleColorRampEntity * >( entity )->ramp()->clone(), update );

    case TextFormatEntity:
      return addTextFormat( name, static_cast< const QgsStyleTextFormatEntity * >( entity )->format(), update );

    case LabelSettingsEntity:
      return addLabelSettings( name, static_cast< const QgsStyleLabelSettingsEntity * >( entity )->settings(), update );

    case LegendPatchShapeEntity:
      return addLegendPatchShape( name, static_cast< const QgsStyleLegendPatchShapeEntity * >( entity )->shape(), update );

    case Symbol3DEntity:
      return addSymbol3D( name, static_cast< const QgsStyleSymbol3DEntity * >( entity )->symbol()->clone(), update );

    case TagEntity:
    case SmartgroupEntity:
      break;

  }
  return false;
}

QgsStyle *QgsStyle::defaultStyle() // static
{
  if ( !sDefaultStyle )
  {
    QgsScopedRuntimeProfile profile( tr( "Load default style database" ) );
    QString styleFilename = QgsApplication::userStylePath();

    // copy default style if user style doesn't exist
    if ( !QFile::exists( styleFilename ) )
    {
      sDefaultStyle = new QgsStyle;
      sDefaultStyle->createDatabase( styleFilename );
      if ( QFile::exists( QgsApplication::defaultStylePath() ) )
      {
        if ( sDefaultStyle->importXml( QgsApplication::defaultStylePath() ) )
        {
          sDefaultStyle->createStyleMetadataTableIfNeeded();
        }
      }
    }
    else
    {
      sDefaultStyle = new QgsStyle;
      if ( sDefaultStyle->load( styleFilename ) )
      {
        sDefaultStyle->upgradeIfRequired();
      }
    }
  }
  return sDefaultStyle;
}

void QgsStyle::cleanDefaultStyle() // static
{
  delete sDefaultStyle;
  sDefaultStyle = nullptr;
}

void QgsStyle::clear()
{
  qDeleteAll( mSymbols );
  qDeleteAll( mColorRamps );
  qDeleteAll( m3dSymbols );

  mSymbols.clear();
  mColorRamps.clear();
  mTextFormats.clear();
  m3dSymbols.clear();

  mCachedTags.clear();
  mCachedFavorites.clear();
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
    QgsDebugMsg( QStringLiteral( "Couldn't convert symbol to valid XML!" ) );
    return false;
  }

  QByteArray xmlArray;
  QTextStream stream( &xmlArray );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  stream.setCodec( "UTF-8" );
#endif
  symEl.save( stream, 4 );
  QString query = qgs_sqlite3_mprintf( "INSERT INTO symbol VALUES (NULL, '%q', '%q', %d);",
                                       name.toUtf8().constData(), xmlArray.constData(), ( favorite ? 1 : 0 ) );

  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( QStringLiteral( "Couldn't insert symbol into the database!" ) );
    return false;
  }

  mCachedFavorites[ SymbolEntity ].insert( name, favorite );

  tagSymbol( SymbolEntity, name, tags );

  emit symbolSaved( name, symbol );
  emit entityAdded( SymbolEntity, name );

  return true;
}

bool QgsStyle::removeSymbol( const QString &name )
{
  return removeEntityByName( SymbolEntity, name );
}

bool QgsStyle::renameEntity( QgsStyle::StyleEntity type, const QString &oldName, const QString &newName )
{
  switch ( type )
  {
    case SymbolEntity:
      return renameSymbol( oldName, newName );

    case ColorrampEntity:
      return renameColorRamp( oldName, newName );

    case TextFormatEntity:
      return renameTextFormat( oldName, newName );

    case LabelSettingsEntity:
      return renameLabelSettings( oldName, newName );

    case LegendPatchShapeEntity:
      return renameLegendPatchShape( oldName, newName );

    case Symbol3DEntity:
      return renameSymbol3D( oldName, newName );

    case TagEntity:
    case SmartgroupEntity:
      return false;
  }
  return false;
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

QStringList QgsStyle::symbolNames() const
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

bool QgsStyle::addTextFormat( const QString &name, const QgsTextFormat &format, bool update )
{
  // delete previous text format (if any)
  if ( mTextFormats.contains( name ) )
  {
    // TODO remove groups and tags?
    mTextFormats.remove( name );
    mTextFormats.insert( name, format );
    if ( update )
      updateSymbol( TextFormatEntity, name );
  }
  else
  {
    mTextFormats.insert( name, format );
    if ( update )
      saveTextFormat( name, format, false, QStringList() );
  }

  return true;
}

bool QgsStyle::addLabelSettings( const QString &name, const QgsPalLayerSettings &settings, bool update )
{
  // delete previous label settings (if any)
  if ( mLabelSettings.contains( name ) )
  {
    // TODO remove groups and tags?
    mLabelSettings.remove( name );
    mLabelSettings.insert( name, settings );
    if ( update )
      updateSymbol( LabelSettingsEntity, name );
  }
  else
  {
    mLabelSettings.insert( name, settings );
    if ( update )
      saveLabelSettings( name, settings, false, QStringList() );
  }

  return true;
}

bool QgsStyle::addLegendPatchShape( const QString &name, const QgsLegendPatchShape &shape, bool update )
{
  // delete previous legend patch shape (if any)
  if ( mLegendPatchShapes.contains( name ) )
  {
    // TODO remove groups and tags?
    mLegendPatchShapes.remove( name );
    mLegendPatchShapes.insert( name, shape );
    if ( update )
      updateSymbol( LegendPatchShapeEntity, name );
  }
  else
  {
    mLegendPatchShapes.insert( name, shape );
    if ( update )
      saveLegendPatchShape( name, shape, false, QStringList() );
  }

  return true;
}

bool QgsStyle::addSymbol3D( const QString &name, QgsAbstract3DSymbol *symbol, bool update )
{
  // delete previous symbol (if any)
  if ( m3dSymbols.contains( name ) )
  {
    // TODO remove groups and tags?
    delete m3dSymbols.take( name );
    m3dSymbols.insert( name, symbol );
    if ( update )
      updateSymbol( Symbol3DEntity, name );
  }
  else
  {
    m3dSymbols.insert( name, symbol );
    if ( update )
      saveSymbol3D( name, symbol, false, QStringList() );
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
    QgsDebugMsg( QStringLiteral( "Couldn't convert color ramp to valid XML!" ) );
    return false;
  }

  QByteArray xmlArray;
  QTextStream stream( &xmlArray );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  stream.setCodec( "UTF-8" );
#endif
  rampEl.save( stream, 4 );
  QString query = qgs_sqlite3_mprintf( "INSERT INTO colorramp VALUES (NULL, '%q', '%q', %d);",
                                       name.toUtf8().constData(), xmlArray.constData(), ( favorite ? 1 : 0 ) );
  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( QStringLiteral( "Couldn't insert colorramp into the database!" ) );
    return false;
  }

  mCachedFavorites[ ColorrampEntity ].insert( name, favorite );

  tagSymbol( ColorrampEntity, name, tags );

  emit rampAdded( name );
  emit entityAdded( ColorrampEntity, name );

  return true;
}

bool QgsStyle::removeColorRamp( const QString &name )
{
  return removeEntityByName( ColorrampEntity, name );
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

QStringList QgsStyle::colorRampNames() const
{
  return mColorRamps.keys();
}

void QgsStyle::handleDeferred3DSymbolCreation()
{
  for ( auto it = mDeferred3DsymbolElements.constBegin(); it != mDeferred3DsymbolElements.constEnd(); ++it )
  {
    const QString symbolType = it.value().attribute( QStringLiteral( "type" ) );
    std::unique_ptr< QgsAbstract3DSymbol > symbol( QgsApplication::symbol3DRegistry()->createSymbol( symbolType ) );
    if ( symbol )
    {
      symbol->readXml( it.value(), QgsReadWriteContext() );
      addSymbol3D( it.key(), symbol.release(), false );
      emit entityAdded( Symbol3DEntity, it.key() );
    }
    else
    {
      QgsDebugMsg( "Cannot open 3d symbol " + it.key() );
      continue;
    }
  }
  mDeferred3DsymbolElements.clear();
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
  QString query = qgs_sqlite3_mprintf( "CREATE TABLE symbol("\
                                       "id INTEGER PRIMARY KEY,"\
                                       "name TEXT UNIQUE,"\
                                       "xml TEXT,"\
                                       "favorite INTEGER);"\
                                       "CREATE TABLE colorramp("\
                                       "id INTEGER PRIMARY KEY,"\
                                       "name TEXT UNIQUE,"\
                                       "xml TEXT,"\
                                       "favorite INTEGER);"\
                                       "CREATE TABLE textformat("\
                                       "id INTEGER PRIMARY KEY,"\
                                       "name TEXT UNIQUE,"\
                                       "xml TEXT,"\
                                       "favorite INTEGER);"\
                                       "CREATE TABLE labelsettings("\
                                       "id INTEGER PRIMARY KEY,"\
                                       "name TEXT UNIQUE,"\
                                       "xml TEXT,"\
                                       "favorite INTEGER);"\
                                       "CREATE TABLE legendpatchshapes("\
                                       "id INTEGER PRIMARY KEY,"\
                                       "name TEXT UNIQUE,"\
                                       "xml TEXT,"\
                                       "favorite INTEGER);"\
                                       "CREATE TABLE symbol3d("\
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
                                       "CREATE TABLE tftagmap("\
                                       "tag_id INTEGER NOT NULL,"\
                                       "textformat_id INTEGER);"\
                                       "CREATE TABLE lstagmap("\
                                       "tag_id INTEGER NOT NULL,"\
                                       "labelsettings_id INTEGER);"\
                                       "CREATE TABLE lpstagmap("\
                                       "tag_id INTEGER NOT NULL,"\
                                       "legendpatchshape_id INTEGER);"\
                                       "CREATE TABLE symbol3dtagmap("\
                                       "tag_id INTEGER NOT NULL,"\
                                       "symbol3d_id INTEGER);"\
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

  // make sure text format table exists
  QString query = qgs_sqlite3_mprintf( "SELECT name FROM sqlite_master WHERE name='textformat'" );
  sqlite3_statement_unique_ptr statement;
  int rc;
  statement = mCurrentDB.prepare( query, rc );
  if ( rc != SQLITE_OK || sqlite3_step( statement.get() ) != SQLITE_ROW )
  {
    query = qgs_sqlite3_mprintf( "CREATE TABLE textformat("\
                                 "id INTEGER PRIMARY KEY,"\
                                 "name TEXT UNIQUE,"\
                                 "xml TEXT,"\
                                 "favorite INTEGER);"\
                                 "CREATE TABLE tftagmap("\
                                 "tag_id INTEGER NOT NULL,"\
                                 "textformat_id INTEGER);" );
    runEmptyQuery( query );
  }
  // make sure label settings table exists
  query = qgs_sqlite3_mprintf( "SELECT name FROM sqlite_master WHERE name='labelsettings'" );
  statement = mCurrentDB.prepare( query, rc );
  if ( rc != SQLITE_OK || sqlite3_step( statement.get() ) != SQLITE_ROW )
  {
    query = qgs_sqlite3_mprintf( "CREATE TABLE labelsettings("\
                                 "id INTEGER PRIMARY KEY,"\
                                 "name TEXT UNIQUE,"\
                                 "xml TEXT,"\
                                 "favorite INTEGER);"\
                                 "CREATE TABLE lstagmap("\
                                 "tag_id INTEGER NOT NULL,"\
                                 "labelsettings_id INTEGER);" );
    runEmptyQuery( query );
  }
  // make sure legend patch shape table exists
  query = qgs_sqlite3_mprintf( "SELECT name FROM sqlite_master WHERE name='legendpatchshapes'" );
  statement = mCurrentDB.prepare( query, rc );
  if ( rc != SQLITE_OK || sqlite3_step( statement.get() ) != SQLITE_ROW )
  {
    query = qgs_sqlite3_mprintf( "CREATE TABLE legendpatchshapes("\
                                 "id INTEGER PRIMARY KEY,"\
                                 "name TEXT UNIQUE,"\
                                 "xml TEXT,"\
                                 "favorite INTEGER);"\
                                 "CREATE TABLE lpstagmap("\
                                 "tag_id INTEGER NOT NULL,"\
                                 "legendpatchshape_id INTEGER);" );
    runEmptyQuery( query );
  }
  // make sure 3d symbol table exists
  query = qgs_sqlite3_mprintf( "SELECT name FROM sqlite_master WHERE name='symbol3d'" );
  statement = mCurrentDB.prepare( query, rc );
  if ( rc != SQLITE_OK || sqlite3_step( statement.get() ) != SQLITE_ROW )
  {
    query = qgs_sqlite3_mprintf( "CREATE TABLE symbol3d("\
                                 "id INTEGER PRIMARY KEY,"\
                                 "name TEXT UNIQUE,"\
                                 "xml TEXT,"\
                                 "favorite INTEGER);"\
                                 "CREATE TABLE symbol3dtagmap("\
                                 "tag_id INTEGER NOT NULL,"\
                                 "symbol3d_id INTEGER);" );
    runEmptyQuery( query );
  }

  // Make sure there are no Null fields in parenting symbols and groups
  query = qgs_sqlite3_mprintf( "UPDATE symbol SET favorite=0 WHERE favorite IS NULL;"
                               "UPDATE colorramp SET favorite=0 WHERE favorite IS NULL;"
                               "UPDATE textformat SET favorite=0 WHERE favorite IS NULL;"
                               "UPDATE labelsettings SET favorite=0 WHERE favorite IS NULL;"
                               "UPDATE legendpatchshapes SET favorite=0 WHERE favorite IS NULL;"
                               "UPDATE symbol3d SET favorite=0 WHERE favorite IS NULL;"
                             );
  runEmptyQuery( query );

  {
    QgsScopedRuntimeProfile profile( tr( "Load symbols" ) );
    // First create all the main symbols
    query = qgs_sqlite3_mprintf( "SELECT * FROM symbol" );
    statement = mCurrentDB.prepare( query, rc );

    while ( rc == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
    {
      QDomDocument doc;
      QString symbolName = statement.columnAsText( SymbolName );
      QgsScopedRuntimeProfile profile( symbolName );
      QString xmlstring = statement.columnAsText( SymbolXML );
      if ( !doc.setContent( xmlstring ) )
      {
        QgsDebugMsg( "Cannot open symbol " + symbolName );
        continue;
      }

      QDomElement symElement = doc.documentElement();
      QgsSymbol *symbol = QgsSymbolLayerUtils::loadSymbol( symElement, QgsReadWriteContext() );
      if ( symbol )
        mSymbols.insert( symbolName, symbol );
    }
  }

  {
    QgsScopedRuntimeProfile profile( tr( "Load color ramps" ) );
    query = qgs_sqlite3_mprintf( "SELECT * FROM colorramp" );
    statement = mCurrentDB.prepare( query, rc );
    while ( rc == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
    {
      QDomDocument doc;
      const QString rampName = statement.columnAsText( ColorrampName );
      QgsScopedRuntimeProfile profile( rampName );
      QString xmlstring = statement.columnAsText( ColorrampXML );
      if ( !doc.setContent( xmlstring ) )
      {
        QgsDebugMsg( "Cannot open symbol " + rampName );
        continue;
      }
      QDomElement rampElement = doc.documentElement();
      QgsColorRamp *ramp = QgsSymbolLayerUtils::loadColorRamp( rampElement );
      if ( ramp )
        mColorRamps.insert( rampName, ramp );
    }
  }

  {
    QgsScopedRuntimeProfile profile( tr( "Load text formats" ) );
    query = qgs_sqlite3_mprintf( "SELECT * FROM textformat" );
    statement = mCurrentDB.prepare( query, rc );
    while ( rc == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
    {
      QDomDocument doc;
      const QString formatName = statement.columnAsText( TextFormatName );
      QgsScopedRuntimeProfile profile( formatName );
      const QString xmlstring = statement.columnAsText( TextFormatXML );
      if ( !doc.setContent( xmlstring ) )
      {
        QgsDebugMsg( "Cannot open text format " + formatName );
        continue;
      }
      QDomElement formatElement = doc.documentElement();
      QgsTextFormat format;
      format.readXml( formatElement, QgsReadWriteContext() );
      mTextFormats.insert( formatName, format );
    }
  }

  {
    QgsScopedRuntimeProfile profile( tr( "Load label settings" ) );
    query = qgs_sqlite3_mprintf( "SELECT * FROM labelsettings" );
    statement = mCurrentDB.prepare( query, rc );
    while ( rc == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
    {
      QDomDocument doc;
      const QString settingsName = statement.columnAsText( LabelSettingsName );
      QgsScopedRuntimeProfile profile( settingsName );
      const QString xmlstring = statement.columnAsText( LabelSettingsXML );
      if ( !doc.setContent( xmlstring ) )
      {
        QgsDebugMsg( "Cannot open label settings " + settingsName );
        continue;
      }
      QDomElement settingsElement = doc.documentElement();
      QgsPalLayerSettings settings;
      settings.readXml( settingsElement, QgsReadWriteContext() );
      mLabelSettings.insert( settingsName, settings );
    }
  }

  {
    QgsScopedRuntimeProfile profile( tr( "Load legend patch shapes" ) );
    query = qgs_sqlite3_mprintf( "SELECT * FROM legendpatchshapes" );
    statement = mCurrentDB.prepare( query, rc );
    while ( rc == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
    {
      QDomDocument doc;
      const QString settingsName = statement.columnAsText( LegendPatchTableName );
      QgsScopedRuntimeProfile profile( settingsName );
      const QString xmlstring = statement.columnAsText( LegendPatchTableXML );
      if ( !doc.setContent( xmlstring ) )
      {
        QgsDebugMsg( "Cannot open legend patch shape " + settingsName );
        continue;
      }
      QDomElement settingsElement = doc.documentElement();
      QgsLegendPatchShape shape;
      shape.readXml( settingsElement, QgsReadWriteContext() );
      mLegendPatchShapes.insert( settingsName, shape );
    }
  }

  {
    QgsScopedRuntimeProfile profile( tr( "Load 3D symbols shapes" ) );
    query = qgs_sqlite3_mprintf( "SELECT * FROM symbol3d" );
    statement = mCurrentDB.prepare( query, rc );

    const bool registry3dPopulated = !QgsApplication::symbol3DRegistry()->symbolTypes().empty();

    while ( rc == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
    {
      QDomDocument doc;
      const QString settingsName = statement.columnAsText( Symbol3DTableName );
      QgsScopedRuntimeProfile profile( settingsName );
      const QString xmlstring = statement.columnAsText( Symbol3DTableXML );
      if ( !doc.setContent( xmlstring ) )
      {
        QgsDebugMsg( "Cannot open 3d symbol " + settingsName );
        continue;
      }
      QDomElement settingsElement = doc.documentElement();

      if ( !registry3dPopulated )
      {
        mDeferred3DsymbolElements.insert( settingsName, settingsElement );
      }
      else
      {
        const QString symbolType = settingsElement.attribute( QStringLiteral( "type" ) );
        std::unique_ptr< QgsAbstract3DSymbol > symbol( QgsApplication::symbol3DRegistry()->createSymbol( symbolType ) );
        if ( symbol )
        {
          symbol->readXml( settingsElement, QgsReadWriteContext() );
          m3dSymbols.insert( settingsName, symbol.release() );
        }
        else
        {
          QgsDebugMsg( "Cannot open 3d symbol " + settingsName );
          continue;
        }
      }
    }
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
    QgsDebugMsg( QStringLiteral( "Symbol of new name already exists" ) );
    return false;
  }

  QgsSymbol *symbol = mSymbols.take( oldName );
  if ( !symbol )
    return false;

  mSymbols.insert( newName, symbol );

  if ( !mCurrentDB )
  {
    QgsDebugMsg( QStringLiteral( "Sorry! Cannot open database to tag." ) );
    return false;
  }

  int symbolid = symbolId( oldName );
  if ( !symbolid )
  {
    QgsDebugMsg( QStringLiteral( "No such symbol for tagging in database: " ) + oldName );
    return false;
  }

  mCachedTags[ SymbolEntity ].remove( oldName );
  mCachedFavorites[ SymbolEntity ].remove( oldName );

  const bool result = rename( SymbolEntity, symbolid, newName );
  if ( result )
  {
    emit symbolRenamed( oldName, newName );
    emit entityRenamed( SymbolEntity, oldName, newName );
  }

  return result;
}

bool QgsStyle::renameColorRamp( const QString &oldName, const QString &newName )
{
  if ( mColorRamps.contains( newName ) )
  {
    QgsDebugMsg( QStringLiteral( "Color ramp of new name already exists." ) );
    return false;
  }

  QgsColorRamp *ramp = mColorRamps.take( oldName );
  if ( !ramp )
    return false;

  mColorRamps.insert( newName, ramp );
  mCachedTags[ ColorrampEntity ].remove( oldName );
  mCachedFavorites[ ColorrampEntity ].remove( oldName );

  int rampid = 0;
  sqlite3_statement_unique_ptr statement;
  QString query = qgs_sqlite3_mprintf( "SELECT id FROM colorramp WHERE name='%q'", oldName.toUtf8().constData() );
  int nErr;
  statement = mCurrentDB.prepare( query, nErr );
  if ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    rampid = sqlite3_column_int( statement.get(), 0 );
  }
  const bool result = rename( ColorrampEntity, rampid, newName );
  if ( result )
  {
    emit rampRenamed( oldName, newName );
    emit entityRenamed( ColorrampEntity, oldName, newName );
  }

  return result;
}

bool QgsStyle::saveTextFormat( const QString &name, const QgsTextFormat &format, bool favorite, const QStringList &tags )
{
  // insert it into the database
  QDomDocument doc( QStringLiteral( "dummy" ) );
  QDomElement formatElem = format.writeXml( doc, QgsReadWriteContext() );

  if ( formatElem.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "Couldn't convert text format to valid XML!" ) );
    return false;
  }

  QByteArray xmlArray;
  QTextStream stream( &xmlArray );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  stream.setCodec( "UTF-8" );
#endif
  formatElem.save( stream, 4 );
  QString query = qgs_sqlite3_mprintf( "INSERT INTO textformat VALUES (NULL, '%q', '%q', %d);",
                                       name.toUtf8().constData(), xmlArray.constData(), ( favorite ? 1 : 0 ) );
  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( QStringLiteral( "Couldn't insert text format into the database!" ) );
    return false;
  }

  mCachedFavorites[ TextFormatEntity ].insert( name, favorite );

  tagSymbol( TextFormatEntity, name, tags );

  emit textFormatAdded( name );
  emit entityAdded( TextFormatEntity, name );

  return true;
}

bool QgsStyle::removeTextFormat( const QString &name )
{
  return removeEntityByName( TextFormatEntity, name );
}

bool QgsStyle::renameTextFormat( const QString &oldName, const QString &newName )
{
  if ( mTextFormats.contains( newName ) )
  {
    QgsDebugMsg( QStringLiteral( "Text format of new name already exists." ) );
    return false;
  }

  if ( !mTextFormats.contains( oldName ) )
    return false;
  QgsTextFormat format = mTextFormats.take( oldName );

  mTextFormats.insert( newName, format );
  mCachedTags[ TextFormatEntity ].remove( oldName );
  mCachedFavorites[ TextFormatEntity ].remove( oldName );

  int textFormatId = 0;
  sqlite3_statement_unique_ptr statement;
  QString query = qgs_sqlite3_mprintf( "SELECT id FROM textformat WHERE name='%q'", oldName.toUtf8().constData() );
  int nErr;
  statement = mCurrentDB.prepare( query, nErr );
  if ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    textFormatId = sqlite3_column_int( statement.get(), 0 );
  }
  const bool result = rename( TextFormatEntity, textFormatId, newName );
  if ( result )
  {
    emit textFormatRenamed( oldName, newName );
    emit entityRenamed( TextFormatEntity, oldName, newName );
  }

  return result;
}

bool QgsStyle::saveLabelSettings( const QString &name, const QgsPalLayerSettings &settings, bool favorite, const QStringList &tags )
{
  // insert it into the database
  QDomDocument doc( QStringLiteral( "dummy" ) );
  QDomElement settingsElem = settings.writeXml( doc, QgsReadWriteContext() );

  if ( settingsElem.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "Couldn't convert label settings to valid XML!" ) );
    return false;
  }

  QByteArray xmlArray;
  QTextStream stream( &xmlArray );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  stream.setCodec( "UTF-8" );
#endif
  settingsElem.save( stream, 4 );
  QString query = qgs_sqlite3_mprintf( "INSERT INTO labelsettings VALUES (NULL, '%q', '%q', %d);",
                                       name.toUtf8().constData(), xmlArray.constData(), ( favorite ? 1 : 0 ) );
  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( QStringLiteral( "Couldn't insert label settings into the database!" ) );
    return false;
  }

  mCachedFavorites[ LabelSettingsEntity ].insert( name, favorite );

  tagSymbol( LabelSettingsEntity, name, tags );

  emit labelSettingsAdded( name );
  emit entityAdded( LabelSettingsEntity, name );

  return true;
}

bool QgsStyle::removeLabelSettings( const QString &name )
{
  return removeEntityByName( LabelSettingsEntity, name );
}

bool QgsStyle::renameLabelSettings( const QString &oldName, const QString &newName )
{
  if ( mLabelSettings.contains( newName ) )
  {
    QgsDebugMsg( QStringLiteral( "Label settings of new name already exists." ) );
    return false;
  }

  if ( !mLabelSettings.contains( oldName ) )
    return false;
  QgsPalLayerSettings settings = mLabelSettings.take( oldName );

  mLabelSettings.insert( newName, settings );
  mCachedTags[ LabelSettingsEntity ].remove( oldName );
  mCachedFavorites[ LabelSettingsEntity ].remove( oldName );

  int labelSettingsId = 0;
  sqlite3_statement_unique_ptr statement;
  QString query = qgs_sqlite3_mprintf( "SELECT id FROM labelsettings WHERE name='%q'", oldName.toUtf8().constData() );
  int nErr;
  statement = mCurrentDB.prepare( query, nErr );
  if ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    labelSettingsId = sqlite3_column_int( statement.get(), 0 );
  }
  const bool result = rename( LabelSettingsEntity, labelSettingsId, newName );
  if ( result )
  {
    emit labelSettingsRenamed( oldName, newName );
    emit entityRenamed( LabelSettingsEntity, oldName, newName );
  }

  return result;
}

bool QgsStyle::saveLegendPatchShape( const QString &name, const QgsLegendPatchShape &shape, bool favorite, const QStringList &tags )
{
  // insert it into the database
  QDomDocument doc( QStringLiteral( "dummy" ) );
  QDomElement shapeElem = doc.createElement( QStringLiteral( "shape" ) );
  shape.writeXml( shapeElem, doc, QgsReadWriteContext() );

  QByteArray xmlArray;
  QTextStream stream( &xmlArray );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  stream.setCodec( "UTF-8" );
#endif
  shapeElem.save( stream, 4 );
  QString query = qgs_sqlite3_mprintf( "INSERT INTO legendpatchshapes VALUES (NULL, '%q', '%q', %d);",
                                       name.toUtf8().constData(), xmlArray.constData(), ( favorite ? 1 : 0 ) );
  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( QStringLiteral( "Couldn't insert legend patch shape into the database!" ) );
    return false;
  }

  mCachedFavorites[ LegendPatchShapeEntity ].insert( name, favorite );

  tagSymbol( LegendPatchShapeEntity, name, tags );

  emit entityAdded( LegendPatchShapeEntity, name );

  return true;
}

bool QgsStyle::renameLegendPatchShape( const QString &oldName, const QString &newName )
{
  if ( mLegendPatchShapes.contains( newName ) )
  {
    QgsDebugMsg( QStringLiteral( "Legend patch shape of new name already exists." ) );
    return false;
  }

  if ( !mLegendPatchShapes.contains( oldName ) )
    return false;
  QgsLegendPatchShape shape = mLegendPatchShapes.take( oldName );

  mLegendPatchShapes.insert( newName, shape );
  mCachedTags[ LegendPatchShapeEntity ].remove( oldName );
  mCachedFavorites[ LegendPatchShapeEntity ].remove( oldName );

  int labelSettingsId = 0;
  sqlite3_statement_unique_ptr statement;
  QString query = qgs_sqlite3_mprintf( "SELECT id FROM legendpatchshapes WHERE name='%q'", oldName.toUtf8().constData() );
  int nErr;
  statement = mCurrentDB.prepare( query, nErr );
  if ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    labelSettingsId = sqlite3_column_int( statement.get(), 0 );
  }
  const bool result = rename( LegendPatchShapeEntity, labelSettingsId, newName );
  if ( result )
  {
    emit entityRenamed( LegendPatchShapeEntity, oldName, newName );
  }

  return result;
}

QgsLegendPatchShape QgsStyle::defaultPatch( Qgis::SymbolType type, QSizeF size ) const
{
  if ( type == Qgis::SymbolType::Hybrid )
    return QgsLegendPatchShape();

  if ( mDefaultPatchCache[ static_cast< int >( type ) ].contains( size ) )
    return mDefaultPatchCache[ static_cast< int >( type ) ].value( size );

  QgsGeometry geom;
  switch ( type )
  {
    case Qgis::SymbolType::Marker:
      geom = QgsGeometry( std::make_unique< QgsPoint >( static_cast< int >( size.width() ) / 2, static_cast< int >( size.height() ) / 2 ) );
      break;

    case Qgis::SymbolType::Line:
    {
      // we're adding 0.5 to get rid of blurred preview:
      // drawing antialiased lines of width 1 at (x,0)-(x,100) creates 2px line
      double y = static_cast< int >( size.height() ) / 2 + 0.5;
      geom = QgsGeometry( std::make_unique< QgsLineString >( ( QVector< double >() << 0 << size.width() ),
                          ( QVector< double >() << y << y ) ) );
      break;
    }

    case Qgis::SymbolType::Fill:
    {
      geom = QgsGeometry( std::make_unique< QgsPolygon >(
                            new QgsLineString( QVector< double >() << 0 << static_cast< int >( size.width() ) << static_cast< int >( size.width() ) << 0 << 0,
                                QVector< double >() << static_cast< int >( size.height() ) << static_cast< int >( size.height() ) << 0 << 0 << static_cast< int >( size.height() ) ) ) );
      break;
    }

    case Qgis::SymbolType::Hybrid:
      break;
  }

  QgsLegendPatchShape res = QgsLegendPatchShape( type, geom, false );
  mDefaultPatchCache[ static_cast< int >( type ) ][size ] = res;
  return res;
}

QList<QList<QPolygonF> > QgsStyle::defaultPatchAsQPolygonF( Qgis::SymbolType type, QSizeF size ) const
{
  if ( type == Qgis::SymbolType::Hybrid )
    return QList<QList<QPolygonF> >();

  if ( mDefaultPatchQPolygonFCache[ static_cast< int >( type ) ].contains( size ) )
    return mDefaultPatchQPolygonFCache[ static_cast< int >( type ) ].value( size );

  QList<QList<QPolygonF> > res = defaultPatch( type, size ).toQPolygonF( type, size );
  mDefaultPatchQPolygonFCache[ static_cast< int >( type ) ][size ] = res;
  return res;
}

QgsTextFormat QgsStyle::defaultTextFormat( QgsStyle::TextFormatContext ) const
{
  return textFormat( QStringLiteral( "Default" ) );
}

bool QgsStyle::saveSymbol3D( const QString &name, QgsAbstract3DSymbol *symbol, bool favorite, const QStringList &tags )
{
  // insert it into the database
  QDomDocument doc( QStringLiteral( "dummy" ) );
  QDomElement elem = doc.createElement( QStringLiteral( "symbol" ) );
  elem.setAttribute( QStringLiteral( "type" ), symbol->type() );
  symbol->writeXml( elem, QgsReadWriteContext() );

  QByteArray xmlArray;
  QTextStream stream( &xmlArray );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  stream.setCodec( "UTF-8" );
#endif
  elem.save( stream, 4 );
  QString query = qgs_sqlite3_mprintf( "INSERT INTO symbol3d VALUES (NULL, '%q', '%q', %d);",
                                       name.toUtf8().constData(), xmlArray.constData(), ( favorite ? 1 : 0 ) );
  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( QStringLiteral( "Couldn't insert 3d symbol into the database!" ) );
    return false;
  }

  mCachedFavorites[ Symbol3DEntity ].insert( name, favorite );

  tagSymbol( Symbol3DEntity, name, tags );

  emit entityAdded( Symbol3DEntity, name );

  return true;
}

bool QgsStyle::renameSymbol3D( const QString &oldName, const QString &newName )
{
  if ( m3dSymbols.contains( newName ) )
  {
    QgsDebugMsg( QStringLiteral( "3d symbol of new name already exists." ) );
    return false;
  }

  if ( !m3dSymbols.contains( oldName ) )
    return false;
  QgsAbstract3DSymbol *symbol = m3dSymbols.take( oldName );

  m3dSymbols.insert( newName, symbol );
  mCachedTags[Symbol3DEntity ].remove( oldName );
  mCachedFavorites[ Symbol3DEntity ].remove( oldName );

  int labelSettingsId = 0;
  sqlite3_statement_unique_ptr statement;
  QString query = qgs_sqlite3_mprintf( "SELECT id FROM symbol3d WHERE name='%q'", oldName.toUtf8().constData() );
  int nErr;
  statement = mCurrentDB.prepare( query, nErr );
  if ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    labelSettingsId = sqlite3_column_int( statement.get(), 0 );
  }
  const bool result = rename( Symbol3DEntity, labelSettingsId, newName );
  if ( result )
  {
    emit entityRenamed( Symbol3DEntity, oldName, newName );
  }

  return result;
}

QStringList QgsStyle::symbol3DNames() const
{
  return m3dSymbols.keys();
}

QStringList QgsStyle::symbolsOfFavorite( StyleEntity type ) const
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( QStringLiteral( "Cannot Open database for getting favorite symbols" ) );
    return QStringList();
  }

  QString query;
  switch ( type )
  {
    case TagEntity:
    case SmartgroupEntity:
      QgsDebugMsg( QStringLiteral( "No such style entity" ) );
      return QStringList();

    default:
      query = qgs_sqlite3_mprintf( QStringLiteral( "SELECT name FROM %1 WHERE favorite=1" ).arg( entityTableName( type ) ).toLocal8Bit().data() );
      break;
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
    QgsDebugMsg( QStringLiteral( "Cannot open database to get symbols of tagid %1" ).arg( tagid ) );
    return QStringList();
  }

  QString subquery;
  switch ( type )
  {
    case TagEntity:
    case SmartgroupEntity:
      QgsDebugMsg( QStringLiteral( "Unknown Entity" ) );
      return QStringList();

    default:
      subquery = qgs_sqlite3_mprintf( QStringLiteral( "SELECT %1 FROM %2 WHERE tag_id=%d" ).arg( tagmapEntityIdFieldName( type ),
                                      tagmapTableName( type ) ).toLocal8Bit().data(), tagid );
      break;
  }

  int nErr;
  sqlite3_statement_unique_ptr statement;
  statement = mCurrentDB.prepare( subquery, nErr );

  // get the symbol <-> tag connection from the tag map table
  QStringList symbols;
  while ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    int id = sqlite3_column_int( statement.get(), 0 );

    const QString query = qgs_sqlite3_mprintf( QStringLiteral( "SELECT name FROM %1 WHERE id=%d" ).arg( entityTableName( type ) ).toLocal8Bit().data(), id );

    int rc;
    sqlite3_statement_unique_ptr statement2;
    statement2 = mCurrentDB.prepare( query, rc );
    while ( rc == SQLITE_OK && sqlite3_step( statement2.get() ) == SQLITE_ROW )
    {
      symbols << statement2.columnAsText( 0 );
    }
  }

  return symbols;
}

int QgsStyle::addTag( const QString &tagname )
{
  if ( !mCurrentDB )
    return 0;
  sqlite3_statement_unique_ptr statement;

  QString query = qgs_sqlite3_mprintf( "INSERT INTO tag VALUES (NULL, '%q')", tagname.toUtf8().constData() );
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

  QString query = qgs_sqlite3_mprintf( "SELECT name FROM tag" );
  int nError;
  statement = mCurrentDB.prepare( query, nError );

  QStringList tagList;
  while ( nError == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    tagList << statement.columnAsText( 0 );
  }

  return tagList;
}

bool QgsStyle::rename( StyleEntity type, int id, const QString &newName )
{
  const QString query = qgs_sqlite3_mprintf( QStringLiteral( "UPDATE %1 SET name='%q' WHERE id=%d" ).arg( entityTableName( type ) ).toLocal8Bit().data(), newName.toUtf8().constData(), id );

  const bool result = runEmptyQuery( query );
  if ( !result )
  {
    mErrorString = QStringLiteral( "Could not rename!" );
  }
  else
  {
    mCachedTags.clear();
    mCachedFavorites.clear();

    switch ( type )
    {
      case TagEntity:
      {
        emit groupsModified();
        break;
      }

      case SmartgroupEntity:
      {
        emit groupsModified();
        break;
      }

      default:
        break;
    }
  }
  return result;
}

bool QgsStyle::remove( StyleEntity type, int id )
{
  bool groupRemoved = false;
  QString query;
  switch ( type )
  {
    case TagEntity:
      query = qgs_sqlite3_mprintf( "DELETE FROM tag WHERE id=%d; DELETE FROM tagmap WHERE tag_id=%d", id, id );
      groupRemoved = true;
      break;
    case SmartgroupEntity:
      query = qgs_sqlite3_mprintf( "DELETE FROM smartgroup WHERE id=%d", id );
      groupRemoved = true;
      break;

    default:
      query = qgs_sqlite3_mprintf( QStringLiteral( "DELETE FROM %1 WHERE id=%d; DELETE FROM %2 WHERE %3=%d" ).arg(
                                     entityTableName( type ),
                                     tagmapTableName( type ),
                                     tagmapEntityIdFieldName( type )
                                   ).toLocal8Bit().data(), id, id );
      break;
  }

  bool result = false;
  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( QStringLiteral( "Could not delete entity!" ) );
  }
  else
  {
    mCachedTags.clear();
    mCachedFavorites.clear();

    if ( groupRemoved )
    {
      QgsSettings settings;
      settings.setValue( QStringLiteral( "qgis/symbolsListGroupsIndex" ), 0 );

      emit groupsModified();
    }
    result = true;
  }
  return result;
}

bool QgsStyle::removeEntityByName( QgsStyle::StyleEntity type, const QString &name )
{
  switch ( type )
  {
    case QgsStyle::TagEntity:
    case QgsStyle::SmartgroupEntity:
      return false;

    case QgsStyle::SymbolEntity:
    {
      std::unique_ptr< QgsSymbol > symbol( mSymbols.take( name ) );
      if ( !symbol )
        return false;

      break;
    }

    case QgsStyle::Symbol3DEntity:
    {
      std::unique_ptr< QgsAbstract3DSymbol > symbol( m3dSymbols.take( name ) );
      if ( !symbol )
        return false;

      break;
    }

    case QgsStyle::ColorrampEntity:
    {
      std::unique_ptr< QgsColorRamp > ramp( mColorRamps.take( name ) );
      if ( !ramp )
        return false;
      break;
    }

    case QgsStyle::TextFormatEntity:
    {
      if ( !mTextFormats.contains( name ) )
        return false;

      mTextFormats.remove( name );
      break;
    }

    case QgsStyle::LabelSettingsEntity:
    {
      if ( !mLabelSettings.contains( name ) )
        return false;

      mLabelSettings.remove( name );
      break;
    }

    case QgsStyle::LegendPatchShapeEntity:
    {
      if ( !mLegendPatchShapes.contains( name ) )
        return false;

      mLegendPatchShapes.remove( name );
      break;
    }
  }

  if ( !mCurrentDB )
  {
    QgsDebugMsg( QStringLiteral( "Sorry! Cannot open database to modify." ) );
    return false;
  }

  const int id = entityId( type, name );
  if ( !id )
  {
    QgsDebugMsg( "No matching entity for deleting in database: " + name );
  }

  const bool result = remove( type, id );
  if ( result )
  {
    mCachedTags[ type ].remove( name );
    mCachedFavorites[ type ].remove( name );

    switch ( type )
    {
      case SymbolEntity:
        emit symbolRemoved( name );
        break;

      case ColorrampEntity:
        emit rampRemoved( name );
        break;

      case TextFormatEntity:
        emit textFormatRemoved( name );
        break;

      case LabelSettingsEntity:
        emit labelSettingsRemoved( name );
        break;

      default:
        // these specific signals should be discouraged -- don't add them for new entity types!
        break;
    }
    emit entityRemoved( type, name );
  }
  return result;
}

bool QgsStyle::runEmptyQuery( const QString &query )
{
  if ( !mCurrentDB )
    return false;

  char *zErr = nullptr;
  int nErr = sqlite3_exec( mCurrentDB.get(), query.toUtf8().constData(), nullptr, nullptr, &zErr );

  if ( nErr != SQLITE_OK )
  {
    QgsDebugMsg( zErr );
    sqlite3_free( zErr );
  }

  return nErr == SQLITE_OK;
}

bool QgsStyle::addFavorite( StyleEntity type, const QString &name )
{
  QString query;

  switch ( type )
  {
    case TagEntity:
    case SmartgroupEntity:
      QgsDebugMsg( QStringLiteral( "Wrong entity value. cannot apply group" ) );
      return false;

    default:
      query = qgs_sqlite3_mprintf( QStringLiteral( "UPDATE %1 SET favorite=1 WHERE name='%q'" ).arg( entityTableName( type ) ).toLocal8Bit().data(),
                                   name.toUtf8().constData() );
      break;
  }

  const bool res = runEmptyQuery( query );
  if ( res )
  {
    switch ( type )
    {
      case TagEntity:
      case SmartgroupEntity:
        break;

      default:
        mCachedFavorites[ type ].insert( name, true );
        break;
    }
    emit favoritedChanged( type, name, true );
  }

  return res;
}

bool QgsStyle::removeFavorite( StyleEntity type, const QString &name )
{
  QString query;

  switch ( type )
  {
    case TagEntity:
    case SmartgroupEntity:
      QgsDebugMsg( QStringLiteral( "Wrong entity value. cannot apply group" ) );
      return false;

    default:
      query = qgs_sqlite3_mprintf( QStringLiteral( "UPDATE %1 SET favorite=0 WHERE name='%q'" ).arg( entityTableName( type ) ).toLocal8Bit().data(), name.toUtf8().constData() );
      break;
  }

  const bool res = runEmptyQuery( query );
  if ( res )
  {
    mCachedFavorites[ type ].insert( name, false );
    emit favoritedChanged( type, name, false );
  }

  return res;
}

QStringList QgsStyle::findSymbols( StyleEntity type, const QString &qword )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( QStringLiteral( "Sorry! Cannot open database to search" ) );
    return QStringList();
  }

  // first find symbols with matching name
  QString item;
  switch ( type )
  {
    case TagEntity:
    case SmartgroupEntity:
      return QStringList();

    default:
      item = entityTableName( type );
      break;
  }

  QString query = qgs_sqlite3_mprintf( "SELECT name FROM %q WHERE name LIKE '%%%q%%'",
                                       item.toUtf8().constData(), qword.toUtf8().constData() );

  sqlite3_statement_unique_ptr statement;
  int nErr; statement = mCurrentDB.prepare( query, nErr );

  QSet< QString > symbols;
  while ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    symbols << statement.columnAsText( 0 );
  }

  // next add symbols with matching tags
  query = qgs_sqlite3_mprintf( "SELECT id FROM tag WHERE name LIKE '%%%q%%'", qword.toUtf8().constData() );
  statement = mCurrentDB.prepare( query, nErr );

  QStringList tagids;
  while ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    tagids << statement.columnAsText( 0 );
  }

  QString dummy = tagids.join( QLatin1String( ", " ) );
  query = qgs_sqlite3_mprintf( QStringLiteral( "SELECT %1 FROM %2 WHERE tag_id IN (%q)" ).arg( tagmapEntityIdFieldName( type ),
                               tagmapTableName( type ) ).toLocal8Bit().data(), dummy.toUtf8().constData() );

  statement = mCurrentDB.prepare( query, nErr );

  QStringList symbolids;
  while ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    symbolids << statement.columnAsText( 0 );
  }

  dummy = symbolids.join( QLatin1String( ", " ) );
  query = qgs_sqlite3_mprintf( "SELECT name FROM %q  WHERE id IN (%q)",
                               item.toUtf8().constData(), dummy.toUtf8().constData() );
  statement = mCurrentDB.prepare( query, nErr );
  while ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    symbols << statement.columnAsText( 0 );
  }

  return qgis::setToList( symbols );
}

bool QgsStyle::tagSymbol( StyleEntity type, const QString &symbol, const QStringList &tags )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( QStringLiteral( "Sorry! Cannot open database to tag." ) );
    return false;
  }

  int symbolid = 0;
  switch ( type )
  {
    case TagEntity:
    case SmartgroupEntity:
      return false;

    default:
      symbolid = entityId( type, symbol );
      break;
  }

  if ( !symbolid )
  {
    QgsDebugMsg( QStringLiteral( "No such symbol for tagging in database: " ) + symbol );
    return false;
  }

  QString tag;
  const auto constTags = tags;
  for ( const QString &t : constTags )
  {
    tag = t.trimmed();
    if ( !tag.isEmpty() )
    {
      // sql: gets the id of the tag if present or insert the tag and get the id of the tag
      int tagid( tagId( tag ) );
      if ( ! tagid )
      {
        tagid = addTag( tag );
      }

      // Now map the tag to the symbol if it's not already tagged
      if ( !symbolHasTag( type, symbol, tag ) )
      {
        QString query = qgs_sqlite3_mprintf( QStringLiteral( "INSERT INTO %1 VALUES (%d,%d)" ).arg( tagmapTableName( type ) ).toLocal8Bit().data(), tagid, symbolid );

        char *zErr = nullptr;
        int nErr;
        nErr = sqlite3_exec( mCurrentDB.get(), query.toUtf8().constData(), nullptr, nullptr, &zErr );
        if ( nErr )
        {
          QgsDebugMsg( zErr );
          sqlite3_free( zErr );
        }
      }
    }
  }

  clearCachedTags( type, symbol );
  emit entityTagsChanged( type, symbol, tagsOfSymbol( type, symbol ) );

  return true;
}

bool QgsStyle::detagSymbol( StyleEntity type, const QString &symbol, const QStringList &tags )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( QStringLiteral( "Sorry! Cannot open database for detagging." ) );
    return false;
  }

  switch ( type )
  {
    case TagEntity:
    case SmartgroupEntity:
      return false;

    default:
      break;
  }

  const int symbolid = entityId( type, symbol );
  if ( symbolid == 0 )
    return false;

  int nErr;
  QString query;
  const auto constTags = tags;
  for ( const QString &tag : constTags )
  {
    query = qgs_sqlite3_mprintf( "SELECT id FROM tag WHERE name='%q'", tag.toUtf8().constData() );

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
      const QString query = qgs_sqlite3_mprintf( QStringLiteral( "DELETE FROM %1 WHERE tag_id=%d AND %2=%d" ).arg( tagmapTableName( type ), tagmapEntityIdFieldName( type ) ).toLocal8Bit().data(), tagid, symbolid );
      runEmptyQuery( query );
    }
  }

  clearCachedTags( type, symbol );
  emit entityTagsChanged( type, symbol, tagsOfSymbol( type, symbol ) );

  // TODO Perform tag cleanup
  // check the number of entries for a given tag in the tagmap
  // if the count is 0, then remove( TagEntity, tagid )
  return true;
}

bool QgsStyle::detagSymbol( StyleEntity type, const QString &symbol )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( QStringLiteral( "Sorry! Cannot open database for detagging." ) );
    return false;
  }

  switch ( type )
  {
    case TagEntity:
    case SmartgroupEntity:
      return false;

    default:
      break;
  }

  const int symbolid = entityId( type, symbol );
  if ( symbolid  == 0 )
  {
    return false;
  }

  // remove all tags
  const QString query = qgs_sqlite3_mprintf( QStringLiteral( "DELETE FROM %1 WHERE %2=%d" ).arg( tagmapTableName( type ),
                        tagmapEntityIdFieldName( type ) ).toLocal8Bit().data(), symbolid );
  runEmptyQuery( query );

  clearCachedTags( type, symbol );
  emit entityTagsChanged( type, symbol, QStringList() );

  // TODO Perform tag cleanup
  // check the number of entries for a given tag in the tagmap
  // if the count is 0, then remove( TagEntity, tagid )
  return true;
}

QStringList QgsStyle::tagsOfSymbol( StyleEntity type, const QString &symbol )
{
  switch ( type )
  {
    case TagEntity:
    case SmartgroupEntity:
      return QStringList();

    default:
      if ( mCachedTags[ type ].contains( symbol ) )
        return mCachedTags[ type ].value( symbol );
      break;
  }

  if ( !mCurrentDB )
  {
    QgsDebugMsg( QStringLiteral( "Sorry! Cannot open database for getting the tags." ) );
    return QStringList();
  }

  int symbolid = entityId( type, symbol );
  if ( !symbolid )
    return QStringList();

  // get the ids of tags for the symbol
  const QString query = qgs_sqlite3_mprintf( QStringLiteral( "SELECT tag_id FROM %1 WHERE %2=%d" ).arg( tagmapTableName( type ),
                        tagmapEntityIdFieldName( type ) ).toLocal8Bit().data(), symbolid );

  sqlite3_statement_unique_ptr statement;
  int nErr; statement = mCurrentDB.prepare( query, nErr );

  QStringList tagList;
  while ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    QString subquery = qgs_sqlite3_mprintf( "SELECT name FROM tag WHERE id=%d", sqlite3_column_int( statement.get(), 0 ) );

    sqlite3_statement_unique_ptr statement2;
    int pErr;
    statement2 = mCurrentDB.prepare( subquery, pErr );
    if ( pErr == SQLITE_OK && sqlite3_step( statement2.get() ) == SQLITE_ROW )
    {
      tagList << statement2.columnAsText( 0 );
    }
  }

  // update cache
  mCachedTags[ type ].insert( symbol, tagList );

  return tagList;
}

bool QgsStyle::isFavorite( QgsStyle::StyleEntity type, const QString &name )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( QStringLiteral( "Sorry! Cannot open database for getting the tags." ) );
    return false;
  }

  switch ( type )
  {
    case TagEntity:
    case SmartgroupEntity:
      return false;

    default:
      if ( mCachedFavorites[ type ].contains( name ) )
        return mCachedFavorites[ type ].value( name );
      break;
  }

  const QStringList names = allNames( type );
  if ( !names.contains( name ) )
    return false; // entity doesn't exist

  // for efficiency, retrieve names of all favorited symbols and store them in cache
  const QStringList favorites = symbolsOfFavorite( type );
  bool res = false;
  for ( const QString &n : names )
  {
    const bool isFav = favorites.contains( n );
    if ( n == name )
      res = isFav;

    mCachedFavorites[ type ].insert( n, isFav );
  }
  return res;
}

bool QgsStyle::symbolHasTag( StyleEntity type, const QString &symbol, const QString &tag )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( QStringLiteral( "Sorry! Cannot open database for getting the tags." ) );
    return false;
  }

  int symbolid = 0;
  switch ( type )
  {
    case TagEntity:
    case SmartgroupEntity:
      return false;

    default:
      symbolid = entityId( type, symbol );
      break;
  }

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
  const QString query = qgs_sqlite3_mprintf( QStringLiteral( "SELECT tag_id FROM %1 WHERE tag_id=%d AND %2=%d" ).arg( tagmapTableName( type ),
                        tagmapEntityIdFieldName( type ) ).toLocal8Bit().data(), tagid, symbolid );

  sqlite3_statement_unique_ptr statement;
  int nErr; statement = mCurrentDB.prepare( query, nErr );

  return ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW );
}

QString QgsStyle::tag( int id ) const
{
  if ( !mCurrentDB )
    return QString();

  sqlite3_statement_unique_ptr statement;

  QString query = qgs_sqlite3_mprintf( "SELECT name FROM tag WHERE id=%d", id );
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
  QString lowerName( name.toLower() );
  QString query = qgs_sqlite3_mprintf( "SELECT id FROM %q WHERE LOWER(name)='%q'", table.toUtf8().constData(), lowerName.toUtf8().constData() );

  sqlite3_statement_unique_ptr statement;
  int nErr; statement = mCurrentDB.prepare( query, nErr );

  int id = 0;
  if ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    id = sqlite3_column_int( statement.get(), 0 );
  }
  else
  {
    // Try the name without lowercase conversion
    QString query = qgs_sqlite3_mprintf( "SELECT id FROM %q WHERE name='%q'", table.toUtf8().constData(), name.toUtf8().constData() );

    sqlite3_statement_unique_ptr statement;
    int nErr; statement = mCurrentDB.prepare( query, nErr );
    if ( nErr == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
    {
      id = sqlite3_column_int( statement.get(), 0 );
    }
  }

  return id;
}

QString QgsStyle::getName( const QString &table, int id ) const
{
  QString query = qgs_sqlite3_mprintf( "SELECT name FROM %q WHERE id='%q'", table.toUtf8().constData(), QString::number( id ).toUtf8().constData() );

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

int QgsStyle::entityId( QgsStyle::StyleEntity type, const QString &name )
{
  return getId( entityTableName( type ), name );
}

int QgsStyle::colorrampId( const QString &name )
{
  return getId( QStringLiteral( "colorramp" ), name );
}

QgsTextFormat QgsStyle::textFormat( const QString &name ) const
{
  return mTextFormats.value( name );
}

int QgsStyle::textFormatCount() const
{
  return mTextFormats.count();
}

QStringList QgsStyle::textFormatNames() const
{
  return mTextFormats.keys();
}

int QgsStyle::textFormatId( const QString &name )
{
  return getId( QStringLiteral( "textformat" ), name );
}

QgsPalLayerSettings QgsStyle::labelSettings( const QString &name ) const
{
  return mLabelSettings.value( name );
}

QgsLegendPatchShape QgsStyle::legendPatchShape( const QString &name ) const
{
  return mLegendPatchShapes.value( name );
}

int QgsStyle::legendPatchShapesCount() const
{
  return mLegendPatchShapes.count();
}

Qgis::SymbolType QgsStyle::legendPatchShapeSymbolType( const QString &name ) const
{
  if ( !mLegendPatchShapes.contains( name ) )
    return Qgis::SymbolType::Hybrid;

  return mLegendPatchShapes.value( name ).symbolType();
}

QgsAbstract3DSymbol *QgsStyle::symbol3D( const QString &name ) const
{
  return m3dSymbols.contains( name ) ? m3dSymbols.value( name )->clone() : nullptr;
}

int QgsStyle::symbol3DCount() const
{
  return m3dSymbols.count();
}

QList<QgsWkbTypes::GeometryType> QgsStyle::symbol3DCompatibleGeometryTypes( const QString &name ) const
{
  if ( !m3dSymbols.contains( name ) )
    return QList<QgsWkbTypes::GeometryType>();

  return m3dSymbols.value( name )->compatibleGeometryTypes();
}

QgsWkbTypes::GeometryType QgsStyle::labelSettingsLayerType( const QString &name ) const
{
  if ( !mLabelSettings.contains( name ) )
    return QgsWkbTypes::UnknownGeometry;

  return mLabelSettings.value( name ).layerType;
}

int QgsStyle::labelSettingsCount() const
{
  return mLabelSettings.count();
}

QStringList QgsStyle::labelSettingsNames() const
{
  return mLabelSettings.keys();
}

int QgsStyle::labelSettingsId( const QString &name )
{
  return getId( QStringLiteral( "labelsettings" ), name );
}

QStringList QgsStyle::legendPatchShapeNames() const
{
  return mLegendPatchShapes.keys();
}

const QgsSymbol *QgsStyle::previewSymbolForPatchShape( const QgsLegendPatchShape &shape ) const
{
  switch ( shape.symbolType() )
  {
    case Qgis::SymbolType::Marker:
      return mPatchMarkerSymbol.get();

    case Qgis::SymbolType::Line:
      return mPatchLineSymbol.get();

    case Qgis::SymbolType::Fill:
      return mPatchFillSymbol.get();

    case Qgis::SymbolType::Hybrid:
      break;
  }
  return nullptr;
}

int QgsStyle::tagId( const QString &name )
{
  return getId( QStringLiteral( "tag" ), name );
}

int QgsStyle::smartgroupId( const QString &name )
{
  return getId( QStringLiteral( "smartgroup" ), name );
}

QStringList QgsStyle::allNames( QgsStyle::StyleEntity type ) const
{
  switch ( type )
  {
    case SymbolEntity:
      return symbolNames();

    case ColorrampEntity:
      return colorRampNames();

    case TextFormatEntity:
      return textFormatNames();

    case LabelSettingsEntity:
      return labelSettingsNames();

    case LegendPatchShapeEntity:
      return legendPatchShapeNames();

    case Symbol3DEntity:
      return symbol3DNames();

    case TagEntity:
      return tags();

    case SmartgroupEntity:
      return smartgroupNames();
  }
  return QStringList();
}

int QgsStyle::addSmartgroup( const QString &name, const QString &op, const QgsSmartConditionMap &conditions )
{
  return addSmartgroup( name, op, conditions.values( QStringLiteral( "tag" ) ),
                        conditions.values( QStringLiteral( "!tag" ) ),
                        conditions.values( QStringLiteral( "name" ) ),
                        conditions.values( QStringLiteral( "!name" ) ) );
}

int QgsStyle::addSmartgroup( const QString &name, const QString &op, const QStringList &matchTag, const QStringList &noMatchTag, const QStringList &matchName, const QStringList &noMatchName )
{
  QDomDocument doc( QStringLiteral( "dummy" ) );
  QDomElement smartEl = doc.createElement( QStringLiteral( "smartgroup" ) );
  smartEl.setAttribute( QStringLiteral( "name" ), name );
  smartEl.setAttribute( QStringLiteral( "operator" ), op );

  auto addCondition = [&doc, &smartEl]( const QString & constraint, const QStringList & parameters )
  {
    for ( const QString &param : parameters )
    {
      QDomElement condEl = doc.createElement( QStringLiteral( "condition" ) );
      condEl.setAttribute( QStringLiteral( "constraint" ), constraint );
      condEl.setAttribute( QStringLiteral( "param" ), param );
      smartEl.appendChild( condEl );
    }
  };
  addCondition( QStringLiteral( "tag" ), matchTag );
  addCondition( QStringLiteral( "!tag" ), noMatchTag );
  addCondition( QStringLiteral( "name" ), matchName );
  addCondition( QStringLiteral( "!name" ), noMatchName );

  QByteArray xmlArray;
  QTextStream stream( &xmlArray );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  stream.setCodec( "UTF-8" );
#endif
  smartEl.save( stream, 4 );
  QString query = qgs_sqlite3_mprintf( "INSERT INTO smartgroup VALUES (NULL, '%q', '%q')",
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
    QgsDebugMsg( QStringLiteral( "Couldn't add the smart group into the database!" ) );
    return 0;
  }
}

QgsSymbolGroupMap QgsStyle::smartgroupsListMap()
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( QStringLiteral( "Cannot open database for listing groups" ) );
    return QgsSymbolGroupMap();
  }

  QString query = qgs_sqlite3_mprintf( "SELECT * FROM smartgroup" );

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

QStringList QgsStyle::smartgroupNames() const
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( QStringLiteral( "Cannot open database for listing groups" ) );
    return QStringList();
  }

  QString query = qgs_sqlite3_mprintf( "SELECT name FROM smartgroup" );

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

  QString query = qgs_sqlite3_mprintf( "SELECT xml FROM smartgroup WHERE id=%d", id );

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
      QgsDebugMsg( QStringLiteral( "Cannot open smartgroup id: %1" ).arg( id ) );
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
        resultNames = allNames( type ).filter( param, Qt::CaseInsensitive );
      }
      else if ( constraint == QLatin1String( "!tag" ) )
      {
        resultNames = allNames( type );
        const QStringList unwanted = symbolsWithTag( type, tagId( param ) );
        for ( const QString &name : unwanted )
        {
          resultNames.removeAll( name );
        }
      }
      else if ( constraint == QLatin1String( "!name" ) )
      {
        const QStringList all = allNames( type );
        for ( const QString &str : all )
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
          for ( const QString &result : std::as_const( resultNames ) )
          {
            if ( dummy.contains( result ) )
              symbols << result;
          }
        }
      }
    } // DOM loop ends here
  }

  // return sorted, unique list
  QStringList unique = qgis::setToList( qgis::listToSet( symbols ) );
  std::sort( unique.begin(), unique.end() );
  return unique;
}

QgsSmartConditionMap QgsStyle::smartgroup( int id )
{
  if ( !mCurrentDB )
  {
    QgsDebugMsg( QStringLiteral( "Cannot open database for listing groups" ) );
    return QgsSmartConditionMap();
  }

  QgsSmartConditionMap condition;

  QString query = qgs_sqlite3_mprintf( "SELECT xml FROM smartgroup WHERE id=%d", id );

  sqlite3_statement_unique_ptr statement;
  int nError;
  statement = mCurrentDB.prepare( query, nError );
  if ( nError == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    QDomDocument doc;
    QString xmlstr = statement.columnAsText( 0 );
    if ( !doc.setContent( xmlstr ) )
    {
      QgsDebugMsg( QStringLiteral( "Cannot open smartgroup id: %1" ).arg( id ) );
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
    QgsDebugMsg( QStringLiteral( "Cannot open database for listing groups" ) );
    return QString();
  }

  QString op;

  QString query = qgs_sqlite3_mprintf( "SELECT xml FROM smartgroup WHERE id=%d", id );

  int nError;
  sqlite3_statement_unique_ptr statement;
  statement = mCurrentDB.prepare( query, nError );
  if ( nError == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
  {
    QDomDocument doc;
    QString xmlstr = statement.columnAsText( 0 );
    if ( !doc.setContent( xmlstr ) )
    {
      QgsDebugMsg( QStringLiteral( "Cannot open smartgroup id: %1" ).arg( id ) );
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
    QgsDebugMsg( QStringLiteral( "Invalid filename for style export." ) );
    return false;
  }

  QDomDocument doc( QStringLiteral( "qgis_style" ) );
  QDomElement root = doc.createElement( QStringLiteral( "qgis_style" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( STYLE_CURRENT_VERSION ) );
  doc.appendChild( root );

  const QStringList favoriteSymbols = symbolsOfFavorite( SymbolEntity );
  const QStringList favoriteColorramps = symbolsOfFavorite( ColorrampEntity );
  const QStringList favoriteTextFormats = symbolsOfFavorite( TextFormatEntity );
  const QStringList favoriteLegendShapes = symbolsOfFavorite( LegendPatchShapeEntity );
  const QStringList favorite3DSymbols = symbolsOfFavorite( Symbol3DEntity );

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

  // save text formats
  QDomElement textFormatsElem = doc.createElement( QStringLiteral( "textformats" ) );
  for ( auto it = mTextFormats.constBegin(); it != mTextFormats.constEnd(); ++it )
  {
    QDomElement textFormatEl = doc.createElement( QStringLiteral( "textformat" ) );
    textFormatEl.setAttribute( QStringLiteral( "name" ), it.key() );
    QDomElement textStyleEl = it.value().writeXml( doc, QgsReadWriteContext() );
    textFormatEl.appendChild( textStyleEl );
    QStringList tags = tagsOfSymbol( TextFormatEntity, it.key() );
    if ( tags.count() > 0 )
    {
      textFormatEl.setAttribute( QStringLiteral( "tags" ), tags.join( ',' ) );
    }
    if ( favoriteTextFormats.contains( it.key() ) )
    {
      textFormatEl.setAttribute( QStringLiteral( "favorite" ), QStringLiteral( "1" ) );
    }
    textFormatsElem.appendChild( textFormatEl );
  }

  // save label settings
  QDomElement labelSettingsElem = doc.createElement( QStringLiteral( "labelsettings" ) );
  for ( auto it = mLabelSettings.constBegin(); it != mLabelSettings.constEnd(); ++it )
  {
    QDomElement labelSettingsEl = doc.createElement( QStringLiteral( "labelsetting" ) );
    labelSettingsEl.setAttribute( QStringLiteral( "name" ), it.key() );
    QDomElement defEl = it.value().writeXml( doc, QgsReadWriteContext() );
    labelSettingsEl.appendChild( defEl );
    QStringList tags = tagsOfSymbol( LabelSettingsEntity, it.key() );
    if ( tags.count() > 0 )
    {
      labelSettingsEl.setAttribute( QStringLiteral( "tags" ), tags.join( ',' ) );
    }
    if ( favoriteTextFormats.contains( it.key() ) )
    {
      labelSettingsEl.setAttribute( QStringLiteral( "favorite" ), QStringLiteral( "1" ) );
    }
    labelSettingsElem.appendChild( labelSettingsEl );
  }

  // save legend patch shapes
  QDomElement legendPatchShapesElem = doc.createElement( QStringLiteral( "legendpatchshapes" ) );
  for ( auto it = mLegendPatchShapes.constBegin(); it != mLegendPatchShapes.constEnd(); ++it )
  {
    QDomElement legendPatchShapeEl = doc.createElement( QStringLiteral( "legendpatchshape" ) );
    legendPatchShapeEl.setAttribute( QStringLiteral( "name" ), it.key() );
    QDomElement defEl = doc.createElement( QStringLiteral( "definition" ) );
    it.value().writeXml( defEl, doc, QgsReadWriteContext() );
    legendPatchShapeEl.appendChild( defEl );
    QStringList tags = tagsOfSymbol( LegendPatchShapeEntity, it.key() );
    if ( tags.count() > 0 )
    {
      legendPatchShapeEl.setAttribute( QStringLiteral( "tags" ), tags.join( ',' ) );
    }
    if ( favoriteLegendShapes.contains( it.key() ) )
    {
      legendPatchShapeEl.setAttribute( QStringLiteral( "favorite" ), QStringLiteral( "1" ) );
    }
    legendPatchShapesElem.appendChild( legendPatchShapeEl );
  }

  // save symbols and attach tags
  QDomElement symbols3DElem = doc.createElement( QStringLiteral( "symbols3d" ) );
  for ( auto it = m3dSymbols.constBegin(); it != m3dSymbols.constEnd(); ++it )
  {
    QDomElement symbolEl = doc.createElement( QStringLiteral( "symbol3d" ) );
    symbolEl.setAttribute( QStringLiteral( "name" ), it.key() );
    QDomElement defEl = doc.createElement( QStringLiteral( "definition" ) );
    defEl.setAttribute( QStringLiteral( "type" ), it.value()->type() );
    it.value()->writeXml( defEl, QgsReadWriteContext() );
    symbolEl.appendChild( defEl );
    QStringList tags = tagsOfSymbol( Symbol3DEntity, it.key() );
    if ( tags.count() > 0 )
    {
      symbolEl.setAttribute( QStringLiteral( "tags" ), tags.join( ',' ) );
    }
    if ( favorite3DSymbols.contains( it.key() ) )
    {
      symbolEl.setAttribute( QStringLiteral( "favorite" ), QStringLiteral( "1" ) );
    }
    symbols3DElem.appendChild( symbolEl );
  }

  root.appendChild( symbolsElem );
  root.appendChild( rampsElem );
  root.appendChild( textFormatsElem );
  root.appendChild( labelSettingsElem );
  root.appendChild( legendPatchShapesElem );
  root.appendChild( symbols3DElem );

  // save
  QFile f( filename );
  if ( !f.open( QFile::WriteOnly | QIODevice::Truncate ) )
  {
    mErrorString = "Couldn't open file for writing: " + filename;
    return false;
  }

  QTextStream ts( &f );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  ts.setCodec( "UTF-8" );
#endif
  doc.save( ts, 2 );
  f.close();

  mFileName = filename;
  return true;
}

bool QgsStyle::importXml( const QString &filename )
{
  return importXml( filename, -1 );
}

bool QgsStyle::importXml( const QString &filename, int sinceVersion )
{
  mErrorString = QString();
  QDomDocument doc( QStringLiteral( "style" ) );
  QFile f( filename );
  if ( !f.open( QFile::ReadOnly ) )
  {
    mErrorString = QStringLiteral( "Unable to open the specified file" );
    QgsDebugMsg( QStringLiteral( "Error opening the style XML file." ) );
    return false;
  }

  if ( !doc.setContent( &f ) )
  {
    mErrorString = QStringLiteral( "Unable to understand the style file: %1" ).arg( filename );
    QgsDebugMsg( QStringLiteral( "XML Parsing error" ) );
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

  const QString version = docEl.attribute( QStringLiteral( "version" ) );
  if ( version != QLatin1String( STYLE_CURRENT_VERSION ) && version != QLatin1String( "0" ) && version != QLatin1String( "1" ) )
  {
    mErrorString = "Unknown style file version: " + version;
    return false;
  }

  QgsSymbolMap symbols;

  QDomElement symbolsElement = docEl.firstChildElement( QStringLiteral( "symbols" ) );
  QDomElement e = symbolsElement.firstChildElement();

  // gain speed by re-grouping the INSERT statements in a transaction
  QString query = qgs_sqlite3_mprintf( "BEGIN TRANSACTION;" );
  runEmptyQuery( query );

  if ( version == QLatin1String( STYLE_CURRENT_VERSION ) || version == QLatin1String( "1" ) )
  {
    // For the new style, load symbols individually
    for ( ; !e.isNull(); e = e.nextSiblingElement() )
    {
      const int entityAddedVersion = e.attribute( QStringLiteral( "addedVersion" ) ).toInt();
      if ( entityAddedVersion != 0 && sinceVersion != -1 && entityAddedVersion <= sinceVersion )
      {
        // skip the symbol, should already be present
        continue;
      }

      if ( e.tagName() == QLatin1String( "symbol" ) )
      {
        QString name = e.attribute( QStringLiteral( "name" ) );
        QStringList tags;
        if ( e.hasAttribute( QStringLiteral( "tags" ) ) )
        {
          tags = e.attribute( QStringLiteral( "tags" ) ).split( ',' );
        }
        bool favorite = false;
        if ( e.hasAttribute( QStringLiteral( "favorite" ) ) && e.attribute( QStringLiteral( "favorite" ) ) == QLatin1String( "1" ) )
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
  for ( ; !e.isNull(); e = e.nextSiblingElement() )
  {
    const int entityAddedVersion = e.attribute( QStringLiteral( "addedVersion" ) ).toInt();
    if ( entityAddedVersion != 0 && sinceVersion != -1 && entityAddedVersion <= sinceVersion )
    {
      // skip the ramp, should already be present
      continue;
    }

    if ( e.tagName() == QLatin1String( "colorramp" ) )
    {
      QString name = e.attribute( QStringLiteral( "name" ) );
      QStringList tags;
      if ( e.hasAttribute( QStringLiteral( "tags" ) ) )
      {
        tags = e.attribute( QStringLiteral( "tags" ) ).split( ',' );
      }
      bool favorite = false;
      if ( e.hasAttribute( QStringLiteral( "favorite" ) ) && e.attribute( QStringLiteral( "favorite" ) ) == QLatin1String( "1" ) )
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
  }

  // load text formats

  // this is ONLY safe to do if we have a QGuiApplication-- it requires QFontDatabase, which is not available otherwise!
  if ( qobject_cast< QGuiApplication * >( QCoreApplication::instance() ) )
  {
    if ( version == STYLE_CURRENT_VERSION )
    {
      const QDomElement textFormatElement = docEl.firstChildElement( QStringLiteral( "textformats" ) );
      e = textFormatElement.firstChildElement();
      for ( ; !e.isNull(); e = e.nextSiblingElement() )
      {
        const int entityAddedVersion = e.attribute( QStringLiteral( "addedVersion" ) ).toInt();
        if ( entityAddedVersion != 0 && sinceVersion != -1 && entityAddedVersion <= sinceVersion )
        {
          // skip the format, should already be present
          continue;
        }

        if ( e.tagName() == QLatin1String( "textformat" ) )
        {
          QString name = e.attribute( QStringLiteral( "name" ) );
          QStringList tags;
          if ( e.hasAttribute( QStringLiteral( "tags" ) ) )
          {
            tags = e.attribute( QStringLiteral( "tags" ) ).split( ',' );
          }
          bool favorite = false;
          if ( e.hasAttribute( QStringLiteral( "favorite" ) ) && e.attribute( QStringLiteral( "favorite" ) ) == QLatin1String( "1" ) )
          {
            favorite = true;
          }

          QgsTextFormat format;
          const QDomElement styleElem = e.firstChildElement();
          format.readXml( styleElem, QgsReadWriteContext() );
          addTextFormat( name, format );
          if ( mCurrentDB )
          {
            saveTextFormat( name, format, favorite, tags );
          }
        }
        else
        {
          QgsDebugMsg( "unknown tag: " + e.tagName() );
        }
      }
    }

    // load label settings
    if ( version == STYLE_CURRENT_VERSION )
    {
      const QDomElement labelSettingsElement = docEl.firstChildElement( QStringLiteral( "labelsettings" ) );
      e = labelSettingsElement.firstChildElement();
      for ( ; !e.isNull(); e = e.nextSiblingElement() )
      {
        const int entityAddedVersion = e.attribute( QStringLiteral( "addedVersion" ) ).toInt();
        if ( entityAddedVersion != 0 && sinceVersion != -1 && entityAddedVersion <= sinceVersion )
        {
          // skip the settings, should already be present
          continue;
        }

        if ( e.tagName() == QLatin1String( "labelsetting" ) )
        {
          QString name = e.attribute( QStringLiteral( "name" ) );
          QStringList tags;
          if ( e.hasAttribute( QStringLiteral( "tags" ) ) )
          {
            tags = e.attribute( QStringLiteral( "tags" ) ).split( ',' );
          }
          bool favorite = false;
          if ( e.hasAttribute( QStringLiteral( "favorite" ) ) && e.attribute( QStringLiteral( "favorite" ) ) == QLatin1String( "1" ) )
          {
            favorite = true;
          }

          QgsPalLayerSettings settings;
          const QDomElement styleElem = e.firstChildElement();
          settings.readXml( styleElem, QgsReadWriteContext() );
          addLabelSettings( name, settings );
          if ( mCurrentDB )
          {
            saveLabelSettings( name, settings, favorite, tags );
          }
        }
        else
        {
          QgsDebugMsg( "unknown tag: " + e.tagName() );
        }
      }
    }
  }

  // load legend patch shapes
  if ( version == STYLE_CURRENT_VERSION )
  {
    const QDomElement legendPatchShapesElement = docEl.firstChildElement( QStringLiteral( "legendpatchshapes" ) );
    e = legendPatchShapesElement.firstChildElement();
    for ( ; !e.isNull(); e = e.nextSiblingElement() )
    {
      const int entityAddedVersion = e.attribute( QStringLiteral( "addedVersion" ) ).toInt();
      if ( entityAddedVersion != 0 && sinceVersion != -1 && entityAddedVersion <= sinceVersion )
      {
        // skip the shape, should already be present
        continue;
      }

      if ( e.tagName() == QLatin1String( "legendpatchshape" ) )
      {
        QString name = e.attribute( QStringLiteral( "name" ) );
        QStringList tags;
        if ( e.hasAttribute( QStringLiteral( "tags" ) ) )
        {
          tags = e.attribute( QStringLiteral( "tags" ) ).split( ',' );
        }
        bool favorite = false;
        if ( e.hasAttribute( QStringLiteral( "favorite" ) ) && e.attribute( QStringLiteral( "favorite" ) ) == QLatin1String( "1" ) )
        {
          favorite = true;
        }

        QgsLegendPatchShape shape;
        const QDomElement shapeElem = e.firstChildElement();
        shape.readXml( shapeElem, QgsReadWriteContext() );
        addLegendPatchShape( name, shape );
        if ( mCurrentDB )
        {
          saveLegendPatchShape( name, shape, favorite, tags );
        }
      }
      else
      {
        QgsDebugMsg( "unknown tag: " + e.tagName() );
      }
    }
  }

  // load 3d symbols
  if ( version == STYLE_CURRENT_VERSION )
  {
    const QDomElement symbols3DElement = docEl.firstChildElement( QStringLiteral( "symbols3d" ) );
    e = symbols3DElement.firstChildElement();
    for ( ; !e.isNull(); e = e.nextSiblingElement() )
    {
      const int entityAddedVersion = e.attribute( QStringLiteral( "addedVersion" ) ).toInt();
      if ( entityAddedVersion != 0 && sinceVersion != -1 && entityAddedVersion <= sinceVersion )
      {
        // skip the symbol, should already be present
        continue;
      }

      if ( e.tagName() == QLatin1String( "symbol3d" ) )
      {
        QString name = e.attribute( QStringLiteral( "name" ) );
        QStringList tags;
        if ( e.hasAttribute( QStringLiteral( "tags" ) ) )
        {
          tags = e.attribute( QStringLiteral( "tags" ) ).split( ',' );
        }
        bool favorite = false;
        if ( e.hasAttribute( QStringLiteral( "favorite" ) ) && e.attribute( QStringLiteral( "favorite" ) ) == QLatin1String( "1" ) )
        {
          favorite = true;
        }

        const QDomElement symbolElem = e.firstChildElement();
        const QString type = symbolElem.attribute( QStringLiteral( "type" ) );
        std::unique_ptr< QgsAbstract3DSymbol > sym( QgsApplication::symbol3DRegistry()->createSymbol( type ) );
        if ( sym )
        {
          sym->readXml( symbolElem, QgsReadWriteContext() );
          QgsAbstract3DSymbol *newSym = sym.get();
          addSymbol3D( name, sym.release() );
          if ( mCurrentDB )
          {
            saveSymbol3D( name, newSym, favorite, tags );
          }
        }
      }
      else
      {
        QgsDebugMsg( "unknown tag: " + e.tagName() );
      }
    }
  }

  query = qgs_sqlite3_mprintf( "COMMIT TRANSACTION;" );
  runEmptyQuery( query );

  mFileName = filename;
  return true;
}

bool QgsStyle::isXmlStyleFile( const QString &path )
{
  QFileInfo fileInfo( path );

  if ( fileInfo.suffix().compare( QLatin1String( "xml" ), Qt::CaseInsensitive ) != 0 )
    return false;

  // sniff the first line of the file to see if it's a style file
  if ( !QFile::exists( path ) )
    return false;

  QFile inputFile( path );
  if ( !inputFile.open( QIODevice::ReadOnly ) )
    return false;

  QTextStream stream( &inputFile );
  const QString line = stream.readLine();
  return line == QLatin1String( "<!DOCTYPE qgis_style>" );
}

bool QgsStyle::updateSymbol( StyleEntity type, const QString &name )
{
  QDomDocument doc( QStringLiteral( "dummy" ) );
  QDomElement symEl;
  QByteArray xmlArray;
  QTextStream stream( &xmlArray );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  stream.setCodec( "UTF-8" );
#endif

  QString query;

  switch ( type )
  {
    case SymbolEntity:
    {
      // check if it is an existing symbol
      if ( !symbolNames().contains( name ) )
      {
        QgsDebugMsg( QStringLiteral( "Update request received for unavailable symbol" ) );
        return false;
      }

      symEl = QgsSymbolLayerUtils::saveSymbol( name, symbol( name ), doc, QgsReadWriteContext() );
      if ( symEl.isNull() )
      {
        QgsDebugMsg( QStringLiteral( "Couldn't convert symbol to valid XML!" ) );
        return false;
      }
      symEl.save( stream, 4 );
      query = qgs_sqlite3_mprintf( "UPDATE symbol SET xml='%q' WHERE name='%q';",
                                   xmlArray.constData(), name.toUtf8().constData() );
      break;
    }

    case Symbol3DEntity:
    {
      // check if it is an existing symbol
      if ( !symbol3DNames().contains( name ) )
      {
        QgsDebugMsg( QStringLiteral( "Update request received for unavailable symbol" ) );
        return false;
      }

      symEl = doc.createElement( QStringLiteral( "symbol" ) );
      symEl.setAttribute( QStringLiteral( "type" ), m3dSymbols.value( name )->type() );
      m3dSymbols.value( name )->writeXml( symEl, QgsReadWriteContext() );
      if ( symEl.isNull() )
      {
        QgsDebugMsg( QStringLiteral( "Couldn't convert symbol to valid XML!" ) );
        return false;
      }
      symEl.save( stream, 4 );
      query = qgs_sqlite3_mprintf( "UPDATE symbol3d SET xml='%q' WHERE name='%q';",
                                   xmlArray.constData(), name.toUtf8().constData() );
      break;
    }

    case ColorrampEntity:
    {
      if ( !colorRampNames().contains( name ) )
      {
        QgsDebugMsg( QStringLiteral( "Update requested for unavailable color ramp." ) );
        return false;
      }

      std::unique_ptr< QgsColorRamp > ramp( colorRamp( name ) );
      symEl = QgsSymbolLayerUtils::saveColorRamp( name, ramp.get(), doc );
      if ( symEl.isNull() )
      {
        QgsDebugMsg( QStringLiteral( "Couldn't convert color ramp to valid XML!" ) );
        return false;
      }
      symEl.save( stream, 4 );
      query = qgs_sqlite3_mprintf( "UPDATE colorramp SET xml='%q' WHERE name='%q';",
                                   xmlArray.constData(), name.toUtf8().constData() );
      break;
    }

    case TextFormatEntity:
    {
      if ( !textFormatNames().contains( name ) )
      {
        QgsDebugMsg( QStringLiteral( "Update requested for unavailable text format." ) );
        return false;
      }

      QgsTextFormat format( textFormat( name ) );
      symEl = format.writeXml( doc, QgsReadWriteContext() );
      if ( symEl.isNull() )
      {
        QgsDebugMsg( QStringLiteral( "Couldn't convert text format to valid XML!" ) );
        return false;
      }
      symEl.save( stream, 4 );
      query = qgs_sqlite3_mprintf( "UPDATE textformat SET xml='%q' WHERE name='%q';",
                                   xmlArray.constData(), name.toUtf8().constData() );
      break;
    }

    case LabelSettingsEntity:
    {
      if ( !labelSettingsNames().contains( name ) )
      {
        QgsDebugMsg( QStringLiteral( "Update requested for unavailable label settings." ) );
        return false;
      }

      QgsPalLayerSettings settings( labelSettings( name ) );
      symEl = settings.writeXml( doc, QgsReadWriteContext() );
      if ( symEl.isNull() )
      {
        QgsDebugMsg( QStringLiteral( "Couldn't convert label settings to valid XML!" ) );
        return false;
      }
      symEl.save( stream, 4 );
      query = qgs_sqlite3_mprintf( "UPDATE labelsettings SET xml='%q' WHERE name='%q';",
                                   xmlArray.constData(), name.toUtf8().constData() );
      break;
    }

    case LegendPatchShapeEntity:
    {
      if ( !legendPatchShapeNames().contains( name ) )
      {
        QgsDebugMsg( QStringLiteral( "Update requested for unavailable legend patch shape." ) );
        return false;
      }

      QgsLegendPatchShape shape( legendPatchShape( name ) );
      symEl = doc.createElement( QStringLiteral( "shape" ) );
      shape.writeXml( symEl, doc, QgsReadWriteContext() );
      symEl.save( stream, 4 );
      query = qgs_sqlite3_mprintf( "UPDATE legendpatchshapes SET xml='%q' WHERE name='%q';",
                                   xmlArray.constData(), name.toUtf8().constData() );
      break;
    }

    case TagEntity:
    case SmartgroupEntity:
    {
      QgsDebugMsg( QStringLiteral( "Updating the unsupported StyleEntity" ) );
      return false;
    }
  }


  if ( !runEmptyQuery( query ) )
  {
    QgsDebugMsg( QStringLiteral( "Couldn't update symbol into the database!" ) );
    return false;
  }
  else
  {
    switch ( type )
    {
      case SymbolEntity:
        emit symbolChanged( name );
        break;

      case ColorrampEntity:
        emit rampChanged( name );
        break;

      case TextFormatEntity:
        emit textFormatChanged( name );
        break;

      case LabelSettingsEntity:
        emit labelSettingsChanged( name );
        break;

      case LegendPatchShapeEntity:
      case TagEntity:
      case SmartgroupEntity:
      case Symbol3DEntity:
        break;
    }
    emit entityChanged( type, name );
  }
  return true;
}

void QgsStyle::clearCachedTags( QgsStyle::StyleEntity type, const QString &name )
{
  mCachedTags[ type ].remove( name );
}

bool QgsStyle::createStyleMetadataTableIfNeeded()
{
  // make sure metadata table exists
  QString query = qgs_sqlite3_mprintf( "SELECT name FROM sqlite_master WHERE name='stylemetadata'" );
  sqlite3_statement_unique_ptr statement;
  int rc;
  statement = mCurrentDB.prepare( query, rc );

  if ( rc != SQLITE_OK || sqlite3_step( statement.get() ) != SQLITE_ROW )
  {
    // no metadata table
    query = qgs_sqlite3_mprintf( "CREATE TABLE stylemetadata("\
                                 "id INTEGER PRIMARY KEY,"\
                                 "key TEXT UNIQUE,"\
                                 "value TEXT);" );
    runEmptyQuery( query );
    query = qgs_sqlite3_mprintf( "INSERT INTO stylemetadata VALUES (NULL, '%q', '%q')", "version", QString::number( Qgis::versionInt() ).toUtf8().constData() );
    runEmptyQuery( query );
    return true;
  }
  else
  {
    return false;
  }
}

void QgsStyle::upgradeIfRequired()
{
  // make sure metadata table exists
  int dbVersion = 0;
  if ( !createStyleMetadataTableIfNeeded() )
  {
    const QString query = qgs_sqlite3_mprintf( "SELECT value FROM stylemetadata WHERE key='version'" );
    int rc;
    sqlite3_statement_unique_ptr statement = mCurrentDB.prepare( query, rc );
    if ( rc == SQLITE_OK && sqlite3_step( statement.get() ) == SQLITE_ROW )
    {
      dbVersion = statement.columnAsText( 0 ).toInt();
    }
  }

  if ( dbVersion < Qgis::versionInt() )
  {
    // do upgrade
    if ( importXml( QgsApplication::defaultStylePath(), dbVersion ) )
    {
      const QString query = qgs_sqlite3_mprintf( "UPDATE stylemetadata SET value='%q' WHERE key='version'", QString::number( Qgis::versionInt() ).toUtf8().constData() );
      runEmptyQuery( query );
    }
  }
}

QString QgsStyle::entityTableName( QgsStyle::StyleEntity type )
{
  switch ( type )
  {
    case SymbolEntity:
      return QStringLiteral( "symbol" );

    case ColorrampEntity:
      return QStringLiteral( "colorramp" );

    case TextFormatEntity:
      return QStringLiteral( "textformat" );

    case LabelSettingsEntity:
      return QStringLiteral( "labelsettings" );

    case LegendPatchShapeEntity:
      return QStringLiteral( "legendpatchshapes" );

    case Symbol3DEntity:
      return QStringLiteral( "symbol3d" );

    case TagEntity:
      return QStringLiteral( "tag" );

    case SmartgroupEntity:
      return QStringLiteral( "smartgroup" );
  }
  return QString();
}

QString QgsStyle::tagmapTableName( QgsStyle::StyleEntity type )
{
  switch ( type )
  {
    case SymbolEntity:
      return QStringLiteral( "tagmap" );

    case ColorrampEntity:
      return QStringLiteral( "ctagmap" );

    case TextFormatEntity:
      return QStringLiteral( "tftagmap" );

    case LabelSettingsEntity:
      return QStringLiteral( "lstagmap" );

    case LegendPatchShapeEntity:
      return QStringLiteral( "lpstagmap" );

    case Symbol3DEntity:
      return QStringLiteral( "symbol3dtagmap" );

    case TagEntity:
    case SmartgroupEntity:
      break;
  }
  return QString();
}

QString QgsStyle::tagmapEntityIdFieldName( QgsStyle::StyleEntity type )
{
  switch ( type )
  {
    case SymbolEntity:
      return QStringLiteral( "symbol_id" );

    case ColorrampEntity:
      return QStringLiteral( "colorramp_id" );

    case TextFormatEntity:
      return QStringLiteral( "textformat_id" );

    case LabelSettingsEntity:
      return QStringLiteral( "labelsettings_id" );

    case LegendPatchShapeEntity:
      return QStringLiteral( "legendpatchshape_id" );

    case Symbol3DEntity:
      return QStringLiteral( "symbol3d_id" );

    case TagEntity:
    case SmartgroupEntity:
      break;
  }
  return QString();
}

QgsStyle::StyleEntity QgsStyleSymbolEntity::type() const
{
  return QgsStyle::SymbolEntity;
}

QgsStyle::StyleEntity QgsStyleColorRampEntity::type() const
{
  return QgsStyle::ColorrampEntity;
}

QgsStyle::StyleEntity QgsStyleTextFormatEntity::type() const
{
  return QgsStyle::TextFormatEntity;
}

QgsStyle::StyleEntity QgsStyleLabelSettingsEntity::type() const
{
  return QgsStyle::LabelSettingsEntity;
}

QgsStyle::StyleEntity QgsStyleLegendPatchShapeEntity::type() const
{
  return QgsStyle::LegendPatchShapeEntity;
}

QgsStyle::StyleEntity QgsStyleSymbol3DEntity::type() const
{
  return QgsStyle::Symbol3DEntity;
}
