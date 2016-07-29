/***************************************************************************
                          qgsgrassvectormaplayer.cpp
                             -------------------
    begin                : September, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QFileInfo>

#include "qgslogger.h"

#include "qgsgrass.h"
#include "qgsgrasswin.h"
#include "qgsgrassvectormap.h"
#include "qgsgrassvectormaplayer.h"

extern "C"
{
#include <grass/version.h>
#if defined(_MSC_VER) && defined(M_PI_4)
#undef M_PI_4 //avoid redefinition warning
#endif
#include <grass/gprojects.h>
#include <grass/gis.h>
#include <grass/dbmi.h>
#if GRASS_VERSION_MAJOR < 7
#include <grass/Vect.h>
#else
#include <grass/vector.h>
#define BOUND_BOX bound_box
#endif
}

QgsGrassVectorMapLayer::QgsGrassVectorMapLayer( QgsGrassVectorMap *map, int field )
    : mField( field )
    , mValid( false )
    , mMap( map )
    , mFieldInfo( 0 )
    , mDriver( 0 )
    , mHasTable( false )
    , mKeyColumn( -1 )
    , mUsers( 0 )
{
}

void QgsGrassVectorMapLayer::clear()
{
  mTableFields.clear();
  mFields.clear();
  mAttributeFields.clear();
  mAttributes.clear();
  mMinMax.clear();
  mKeyColumn = -1;
  mValid = false;
  G_free( mFieldInfo );
  mFieldInfo = 0;
}

int QgsGrassVectorMapLayer::cidxFieldIndex()
{
  if ( !mMap->map() )
  {
    return -1;
  }
  return Vect_cidx_get_field_index( mMap->map(), mField );
}

int QgsGrassVectorMapLayer::cidxFieldNumCats()
{
  if ( !mMap->map() || cidxFieldIndex() < 0 )
  {
    return 0;
  }
  return Vect_cidx_get_num_cats_by_index( mMap->map(), cidxFieldIndex() );
}

void QgsGrassVectorMapLayer::load()
{
  clear();

  if ( !mMap )
  {
    return;
  }

  // Attributes are not loaded for topo layers in which case field == 0
  if ( mField == 0 )
  {
    return;
  }

  QgsDebugMsg( QString( "cidxFieldIndex() = %1 cidxFieldNumCats() = %2" ).arg( cidxFieldIndex() ).arg( cidxFieldNumCats() ) );

  mFieldInfo = Vect_get_field( mMap->map(), mField ); // should work also with field = 0

  if ( !mFieldInfo )
  {
    QgsDebugMsg( "No field info -> no attribute table" );
  }
  else
  {
    QgsDebugMsg( "Field info found -> open database" );

    QFileInfo di( mMap->grassObject().mapsetPath() + "/vector/" + mMap->grassObject().name() + "/dbln" );
    mLastLoaded = di.lastModified();

    QString error;
    dbDriver *databaseDriver = openDriver( error );

    if ( !databaseDriver || !error.isEmpty() )
    {
      QgsDebugMsg( error );
    }
    else
    {
      QgsDebugMsg( "Database opened -> open select cursor" );
      QgsGrass::lock(); // not sure if lock is necessary
      dbString dbstr;
      db_init_string( &dbstr );
      db_set_string( &dbstr, ( char * )"select * from " );
      db_append_string( &dbstr, mFieldInfo->table );

      QgsDebugMsg( QString( "SQL: %1" ).arg( db_get_string( &dbstr ) ) );
      dbCursor databaseCursor;
      if ( db_open_select_cursor( databaseDriver, &dbstr, &databaseCursor, DB_SCROLL ) != DB_OK )
      {
        db_close_database_shutdown_driver( databaseDriver );
        QgsGrass::warning( "Cannot select attributes from table '" + QString( mFieldInfo->table ) + "'" );
      }
      else
      {
#ifdef QGISDEBUG
        int nRecords = db_get_num_rows( &databaseCursor );
        QgsDebugMsg( QString( "Number of records: %1" ).arg( nRecords ) );
#endif

        dbTable  *databaseTable = db_get_cursor_table( &databaseCursor );
        int nColumns = db_get_table_number_of_columns( databaseTable );

        // Read columns' description
        for ( int i = 0; i < nColumns; i++ )
        {
          QPair<double, double> minMax( DBL_MAX, -DBL_MAX );

          dbColumn *column = db_get_table_column( databaseTable, i );

          int ctype = db_sqltype_to_Ctype( db_get_column_sqltype( column ) );
          QVariant::Type qtype = QVariant::String; //default to string
          QgsDebugMsg( QString( "column = %1 ctype = %2" ).arg( db_get_column_name( column ) ).arg( ctype ) );

          QString ctypeStr;
          switch ( ctype )
          {
            case DB_C_TYPE_INT:
              ctypeStr = "integer";
              qtype = QVariant::Int;
              break;
            case DB_C_TYPE_DOUBLE:
              ctypeStr = "double";
              qtype = QVariant::Double;
              break;
            case DB_C_TYPE_STRING:
              ctypeStr = "string";
              qtype = QVariant::String;
              break;
            case DB_C_TYPE_DATETIME:
              ctypeStr = "datetime";
              qtype = QVariant::String;
              break;
          }
          mTableFields.append( QgsField( db_get_column_name( column ), qtype, ctypeStr,
                                         db_get_column_length( column ), db_get_column_precision( column ) ) );
          mMinMax << minMax;
          if ( G_strcasecmp( db_get_column_name( column ), mFieldInfo->key ) == 0 )
          {
            mKeyColumn = i;
          }
        }

        if ( mKeyColumn < 0 )
        {
          mTableFields.clear();
          QgsGrass::warning( QObject::tr( "Key column '%1' not found in the table '%2'" ).arg( mFieldInfo->key, mFieldInfo->table ) );
        }
        else
        {
          mHasTable = true;
          // Read attributes to the memory
          for ( ;; )
          {
            int more;

            if ( db_fetch( &databaseCursor, DB_NEXT, &more ) != DB_OK )
            {
              QgsDebugMsg( "Cannot fetch DB record" );
              break;
            }
            if ( !more )
            {
              break; // no more records
            }

            // Check cat value
            dbColumn *column = db_get_table_column( databaseTable, mKeyColumn );
            dbValue *value = db_get_column_value( column );

            if ( db_test_value_isnull( value ) )
            {
              continue;
            }
            int cat = db_get_value_int( value );
            if ( cat < 0 )
            {
              continue;
            }

            QList<QVariant> values;
            for ( int i = 0; i < nColumns; i++ )
            {
              column = db_get_table_column( databaseTable, i );
              int sqltype = db_get_column_sqltype( column );
              int ctype = db_sqltype_to_Ctype( sqltype );
              value = db_get_column_value( column );
              db_convert_value_to_string( value, sqltype, &dbstr );

              QgsDebugMsgLevel( QString( "column = %1 value = %2" ).arg( db_get_column_name( column ), db_get_string( &dbstr ) ), 3 );

              QVariant variant;
              if ( !db_test_value_isnull( value ) )
              {
                int iv;
                double dv;
                //layer.mAttributes[layer.nAttributes].values[i] = strdup( db_get_string( &dbstr ) );
                switch ( ctype )
                {
                  case DB_C_TYPE_INT:
                    iv = db_get_value_int( value );
                    variant = QVariant( iv );
                    mMinMax[i].first = qMin( mMinMax[i].first, ( double )iv );
                    mMinMax[i].second = qMin( mMinMax[i].second, ( double )iv );
                    break;
                  case DB_C_TYPE_DOUBLE:
                    dv = db_get_value_double( value );
                    variant = QVariant( dv );
                    mMinMax[i].first = qMin( mMinMax[i].first, dv );
                    mMinMax[i].second = qMin( mMinMax[i].second, dv );
                    break;
                  case DB_C_TYPE_STRING:
                    // Store as byte array so that codec may be used later
                    variant = QVariant( QByteArray( db_get_value_string( value ) ) );
                    break;
                  case DB_C_TYPE_DATETIME:
                    variant = QVariant( QByteArray( db_get_string( &dbstr ) ) );
                    break;
                  default:
                    variant = QVariant( QByteArray( db_get_string( &dbstr ) ) );
                }
              }
              QgsDebugMsgLevel( QString( "column = %1 variant = %2" ).arg( db_get_column_name( column ), variant.toString() ), 3 );
              values << variant;
            }
            mAttributes.insert( cat, values );
          }
        }
        mValid = true;
        db_close_cursor( &databaseCursor );
        db_close_database_shutdown_driver( databaseDriver );
        db_free_string( &dbstr );

        QgsDebugMsg( QString( "mTableFields.size = %1" ).arg( mTableFields.size() ) );
        QgsDebugMsg( QString( "number of attributes = %1" ).arg( mAttributes.size() ) );
      }
      QgsGrass::unlock();
    }
  }

  // Add cat if no attribute fields exist (otherwise qgis crashes)
  if ( mTableFields.size() == 0 )
  {
    mKeyColumn = 0;
    mTableFields.append( QgsField( "cat", QVariant::Int, "integer" ) );
    QPair<double, double> minMax( 0, 0 );

    if ( cidxFieldIndex() >= 0 )
    {
      int ncats, cat, type, id;

      ncats = Vect_cidx_get_num_cats_by_index( mMap->map(), cidxFieldIndex() );

      if ( ncats > 0 )
      {
        Vect_cidx_get_cat_by_index( mMap->map(), cidxFieldIndex(), 0, &cat, &type, &id );
        minMax.first = cat;

        Vect_cidx_get_cat_by_index( mMap->map(), cidxFieldIndex(), ncats - 1, &cat, &type, &id );
        minMax.second = cat;
      }
    }
    mMinMax << minMax;
  }
  mFields = mTableFields;
  mAttributeFields = mTableFields;

  QgsDebugMsg( QString( "layer loaded mTableFields.size() = %1 mAttributes.size() = %2" ).arg( mTableFields.size() ).arg( mAttributes.size() ) );
  mValid = true;
}

void QgsGrassVectorMapLayer::addUser()
{
  mUsers++;
  QgsDebugMsg( QString( "user added mUsers = %1" ).arg( mUsers ) );
}

void QgsGrassVectorMapLayer::removeUser()
{
  mUsers--;
  QgsDebugMsg( QString( "user removed mUsers = %1" ).arg( mUsers ) );
}

void QgsGrassVectorMapLayer::close()
{
  QgsDebugMsg( "close" );
  //removeUser(); // removed by map
  if ( mMap )
  {
    mMap->closeLayer( this );
  }
}

QStringList QgsGrassVectorMapLayer::fieldNames( QgsFields & fields )
{
  QStringList list;
  Q_FOREACH ( const QgsField& field, fields )
  {
    list << field.name();
  }
  return list;
}

void QgsGrassVectorMapLayer::updateFields()
{

  // update fields to pass layer/buffer check when committing
  for ( int i = mFields.size() - 1; i >= 0; i-- )
  {
    QgsField field = mFields.at( i );
    if ( field.name() == QgsGrassVectorMap::topoSymbolFieldName() )
    {
      continue;
    }
    if ( mTableFields.indexFromName( field.name() ) == -1 )
    {
      mFields.remove( i );
    }
  }
  Q_FOREACH ( const QgsField& field, mTableFields )
  {
    if ( mFields.indexFromName( field.name() ) == -1 )
    {
      mFields.append( field );
    }
  }

#if 0
  // keep mapping to original fields skipping virtual topo symbol field
  mAttributeIndexes.clear();
  for ( int i = 0; i < mFields.size(); i++ )
  {
    int index = mAttributeFields.indexFromName( mFields[i].name() );
    if ( index != -1 )
    {
      mAttributeIndexes[i] = index;
      QgsDebugMsg( QString( "mAttributeIndexes[%1] = %2" ).arg( i ).arg( index ) );
    }
  }
#endif
}

QString QgsGrassVectorMapLayer::quotedValue( QVariant value )
{
  if ( value.isNull() )
  {
    return "NULL";
  }

  switch ( value.type() )
  {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Double:
      return value.toString();

    case QVariant::Bool:
      return value.toBool() ? "TRUE" : "FALSE";

    default:
    case QVariant::String:
      QString v = value.toString();
      v.replace( "'", "''" );
      if ( v.contains( "\\" ) )
      {
        v.replace( "\\", "\\\\" );
      }
      return v.prepend( "'" ).append( "'" );
  }
}

dbDriver * QgsGrassVectorMapLayer::openDriver( QString &error )
{
  dbDriver * driver = 0;

  if ( !mFieldInfo )
  {
    error = tr( "No field info" );
    QgsDebugMsg( error );
  }
  else
  {
    QgsDebugMsg( "Field info found -> open database" );
    QString err = QString( "Cannot open database %1 by driver %2" ).arg( mFieldInfo->database, mFieldInfo->driver );
    QgsGrass::lock();
    G_TRY
    {
      setMapset();
      driver = db_start_driver_open_database( mFieldInfo->driver, mFieldInfo->database );
      if ( !driver )
      {
        error = err;
        QgsDebugMsg( error );
      }
    }
    G_CATCH( QgsGrass::Exception &e )
    {
      error = err + " : " + e.what();
      QgsDebugMsg( error );
    }
    QgsGrass::unlock();

    if ( driver )
    {
      QgsDebugMsg( "Database opened" );
#ifdef Q_OS_WIN
      // Driver on Windows opens black window:
      // https://lists.osgeo.org/pipermail/grass-dev/2015-October/076831.html
      QgsGrassWin::hideWindow( driver->pid );
#endif
    }
  }
  return driver;
}

void QgsGrassVectorMapLayer::addTopoField( QgsFields &fields )
{
  QString comment = tr( "Virtual topology symbol field" );
  QgsField topoField = QgsField( QgsGrassVectorMap::topoSymbolFieldName(), QVariant::Int, "integer", 0, 0, comment );
  fields.append( topoField );
}

void QgsGrassVectorMapLayer::startEdit()
{

  // add topo field which is present until closeEdit when data are reloaded
  addTopoField( mFields );

  QString error;
  mDriver = openDriver( error );
  if ( !error.isEmpty() )
  {
    // TODO: warning here is causing dead lock, QGIS starts renderer which is hanging on openLayer()
    // waiting for lock
    //QgsGrass::warning( error );
    QgsDebugMsg( error );
  }
}

void QgsGrassVectorMapLayer::closeEdit()
{

  if ( mDriver )
  {
    QgsDebugMsg( "close driver" );
    db_close_database_shutdown_driver( mDriver );
    QgsDebugMsg( "driver closed" );
    mDriver = 0;
  }
}

QVariant QgsGrassVectorMapLayer::attribute( int cat, int index )
{
  // It may happen that the table exists but record for the cat is missing
  if (( !hasTable() && index == 0 ) || ( hasTable() && index == keyColumn() ) )
  {
    QgsDebugMsgLevel( QString( "set attribute %1 to cat %2" ).arg( index ).arg( cat ), 3 );
    return QVariant( cat );
  }

  if ( !hasTable() )
  {
    return QVariant();
  }
  else
  {
    // during editing are accessed only original columns with original indices,
    // layer/buffer do the mapping
    if ( !mAttributes.contains( cat ) )
    {
      QgsDebugMsgLevel( QString( "cat %1 not found in attributes" ).arg( cat ), 3 );
      return QVariant();
    }
    QVariant value = mAttributes.value( cat ).value( index );
    QgsDebugMsgLevel( QString( "cat = %1 index = %2 value = %3" ).arg( cat ).arg( index ).arg( value.toString() ), 3 );
    return value;
  }
}


//------------------------------- Database utils ---------------------------------
void QgsGrassVectorMapLayer::setMapset()
{
  QgsGrass::setMapset( mMap->grassObject().gisdbase(), mMap->grassObject().location(), mMap->grassObject().mapset() );
}

void QgsGrassVectorMapLayer::executeSql( const QString &sql, QString &error )
{
  QgsDebugMsg( "sql = " + sql );

  if ( !mDriver )
  {
    error = tr( "Driver is not open" );
    QgsDebugMsg( error );
    return;
  }

  dbString dbstr;
  db_init_string( &dbstr );
  db_set_string( &dbstr, sql.toLatin1().data() );

  QgsDebugMsg( QString( "SQL: %1" ).arg( db_get_string( &dbstr ) ) );

  int ret = db_execute_immediate( mDriver, &dbstr );
  if ( ret != DB_OK )
  {
    error = QString::fromLatin1( db_get_error_msg() );
    QgsDebugMsg( error );
  }

  db_free_string( &dbstr );  //if ( index < 0 || index > )
  return;
}

void QgsGrassVectorMapLayer::createTable( const QgsFields &fields, QString &error )
{
  QgsDebugMsg( QString( "fields.size() = %1" ).arg( fields.size() ) );

  // Read attributes
  if ( mFieldInfo )
  {
    error = tr( "The table for this field already exists" );
    QgsDebugMsg( error );
    return;
  }

  QgsDebugMsg( "Field info not found -> create new table" );

  // We must set mapset before Vect_default_field_info
  setMapset();

  int nLinks = Vect_get_num_dblinks( mMap->map() );
  if ( nLinks == 0 )
  {
    mFieldInfo = Vect_default_field_info( mMap->map(), mField, 0, GV_1TABLE );
  }
  else
  {
    mFieldInfo = Vect_default_field_info( mMap->map(), mField, 0, GV_MTABLE );
  }
  if ( !mFieldInfo )
  {
    error = tr( "Cannot create field info" );
    QgsDebugMsg( error );
    return;
  }

  mDriver = openDriver( error );
  if ( !error.isEmpty() )
  {
    QgsDebugMsg( error );
    mFieldInfo = 0;
    return;
  }

  QgsDebugMsg( "Database opened -> create table" );

  QgsFields catFields;
  catFields.append( QgsField( mFieldInfo->key, QVariant::Int, "integer" ) );
  Q_FOREACH ( const QgsField& field, fields )
  {
    catFields.append( field );
  }

  try
  {
    QgsGrass::createTable( mDriver, mFieldInfo->table, catFields );

  }
  catch ( QgsGrass::Exception &e )
  {
    error = QString( e.what() );
    QgsDebugMsg( error );
    db_close_database_shutdown_driver( mDriver );
    mFieldInfo = 0;
    return;
  }

  if ( mFieldInfo )
  {
    int ret = Vect_map_add_dblink( mMap->map(), mField, 0, mFieldInfo->table, mFieldInfo->key,
                                   mFieldInfo->database, mFieldInfo->driver );

    if ( ret == -1 )
    {
      error = tr( "Cannot create link to the table." );
      QgsDebugMsg( error );
      // delete created table
      QString query = QString( "DROP TABLE %1" ).arg( mFieldInfo->table );
      QString dropError;
      executeSql( query, dropError );
      if ( !dropError.isEmpty() )
      {
        QgsDebugMsg( dropError );
        error += " " + tr( "Created table %1 could not be deleted" ).arg( mFieldInfo->table ) + " " + dropError;
        QgsDebugMsg( error );
      }
      db_close_database_shutdown_driver( mDriver );
      mFieldInfo = 0;
    }
  }

  if ( mFieldInfo )
  {
    Q_FOREACH ( const QgsField& field, fields )
    {
      mTableFields.append( field );
      mAttributeFields.append( field );
    }
    mHasTable = true;
    mKeyColumn = 0;
    insertCats( error );
    if ( !error.isEmpty() )
    {
      QgsDebugMsg( error );
    }
  }
  QgsDebugMsg( "Table successfully created" );
}

void QgsGrassVectorMapLayer::addColumn( const QgsField &field, QString &error )
{
  QgsDebugMsg( QString( "field.name() = %1 field.type() = %2" ).arg( field.name() ).arg( field.type() ) );

  if ( !mFieldInfo ) // table does not exist yet
  {
    // create new table
    QgsFields fields;
    fields.append( field );
    createTable( fields, error );
    if ( !error.isEmpty() )
    {
      QgsDebugMsg( error );
      return;
    }
  }
  else // the table alread exists
  {
    QString type = field.typeName();
    if ( type == "varchar" )
    {
      if ( field.length() > 0 )
      {
        type = QString( "%1(%2)" ).arg( type ).arg( field.length() );
      }
    }
    QString query = QString( "ALTER TABLE %1 ADD COLUMN %2 %3" ).arg( mFieldInfo->table, field.name(), type );
    executeSql( query, error );

    if ( error.isEmpty() )
    {
      mTableFields.append( field );

      int index = mAttributeFields.indexFromName( field.name() );
      if ( index != -1 )
      {
        // the column is already in attributes (delete column undo)
        QgsDebugMsg( "insert old values" );
        printCachedAttributes();
        QStringList errors;
        Q_FOREACH ( int cat, mAttributes.keys() )
        {
          QVariant value = mAttributes.value( cat ).value( index );
          QString valueString = quotedValue( value );
          QString query = QString( "UPDATE %1 SET %2 = %3 WHERE %4 = %5" )
                          .arg( mFieldInfo->table, field.name(), valueString, keyColumnName() ).arg( cat );
          QString err;
          executeSql( query, err );
          if ( !err.isEmpty() )
          {
            errors << err;
          }
          if ( errors.size() > 5 )
          {
            error = tr( "Errors updating restored column, update interrupted" ) + " : " + errors.join( "; " );
            break;
          }
        }
      }
      else
      {
        // really new column
        mAttributeFields.append( field );
        Q_FOREACH ( int cat, mAttributes.keys() )
        {
          mAttributes[cat].append( QVariant() );
        }
      }
    }
  }
}

void QgsGrassVectorMapLayer::deleteColumn( const QgsField &field, QString &error )
{
  QgsDebugMsg( QString( "field.name() = %1" ).arg( field.name() ) );

  if ( field.name() == QgsGrassVectorMap::topoSymbolFieldName() )
  {
    error = tr( "%1 field cannot be deleted, it is temporary virtual field used for topology symbol." ).arg( field.name() );
    return;
  }

  // SQLite does not support DROP COLUMN
  if ( QString( mFieldInfo->driver ) == "sqlite" )
  {
    QStringList columns;
    Q_FOREACH ( const QgsField& f, mTableFields )
    {
      if ( f.name() != field.name() )
      {
        columns << f.name();
      }
    }
    QStringList queries;
    queries << "BEGIN TRANSACTION";
    queries << QString( "CREATE TEMPORARY TABLE %1_tmp_drop_column AS SELECT %2 FROM %1" ).arg( mFieldInfo->table, columns.join( "," ) );
    queries << QString( "DROP TABLE %1" ).arg( mFieldInfo->table );
    queries << QString( "CREATE TABLE %1 AS SELECT * FROM %1_tmp_drop_column" ).arg( mFieldInfo->table );
    queries << QString( "DROP TABLE %1_tmp_drop_column" ).arg( mFieldInfo->table );
    queries << QString( "CREATE UNIQUE INDEX %1_%2 ON %1 (%2)" ).arg( mFieldInfo->table, mFieldInfo->key );
    queries << "COMMIT";
    // Execute one after another to get possible error
    Q_FOREACH ( const QString& query, queries )
    {
      QgsDebugMsg( "query = " + query );
      executeSql( query, error );
      if ( !error.isEmpty() )
      {
        break;
      }
    }
  }
  else
  {
    QString query = QString( "ALTER TABLE %1 DROP COLUMN %2" ).arg( mFieldInfo->table, field.name() );
    QgsDebugMsg( "query = " + query );
    executeSql( query, error );
  }

  if ( error.isEmpty() )
  {
    QgsDebugMsg( "error = " + error );
    int index = mTableFields.indexFromName( field.name() );
    if ( index != -1 )
    {
      mTableFields.remove( index );
    }
  }
}

void QgsGrassVectorMapLayer::insertCats( QString &error )
{
  int cidxIndex = Vect_cidx_get_field_index( map()->map(), mField );
  if ( cidxIndex >= 0 ) // cats attached to lines already exist
  {
    int nCats = Vect_cidx_get_num_cats_by_index( map()->map(), cidxIndex );
    QgsDebugMsg( QString( "nCats = %1" ).arg( nCats ) );
    for ( int i = 0; i < nCats; i++ )
    {
      int cat, type, id;
      Vect_cidx_get_cat_by_index( map()->map(), cidxIndex, i, &cat, &type, &id );
      QgsFeature feature;
      insertAttributes( cat, feature, error );
      if ( !error.isEmpty() )
      {
        QgsDebugMsg( error );
        break;
      }
    }
  }
}

void QgsGrassVectorMapLayer::insertAttributes( int cat, const QgsFeature &feature, QString &error )
{
  QgsDebugMsg( QString( "mField = %1 cat = %2" ).arg( mField ).arg( cat ) );

  if ( !mHasTable )
  {
    error = tr( "no table" );
    return;
  }
  QStringList names;
  QStringList values;

  names << mFieldInfo->key;
  values << QString::number( cat );

  QList<QVariant> cacheValues;
  cacheValues.reserve( mAttributeFields.size() );
  for ( int i = 0; i < mAttributeFields.size(); ++i )
  {
    cacheValues << QVariant();
  }

  if ( feature.fields() )
  {
    // append feature attributes if not null
    for ( int i = 0; i < feature.fields()->size(); i++ )
    {
      QString name = feature.fields()->at( i ).name();
      QVariant valueVariant = feature.attributes().value( i );

      if ( name != QgsGrassVectorMap::topoSymbolFieldName() )
      {
        int cacheIndex = mAttributeFields.indexFromName( name );
        if ( cacheIndex < 0 ) // should not happen
        {
          error = QString( "Field %1 not found in cached attributes" ).arg( name );
          return;
        }
        else
        {
          cacheValues[cacheIndex] = valueVariant;
        }
      }

      if ( name == mFieldInfo->key )
      {
        continue;
      }

      if ( !valueVariant.isNull() )
      {
        names << name;
        values << quotedValue( valueVariant );
      }
    }
  }

  QString query = QString( "INSERT INTO %1 ( %2 ) VALUES ( %3 )" ).arg( mFieldInfo->table,
                  names.join( ", " ), values.join( "," ) );
  executeSql( query, error );
  if ( error.isEmpty() )
  {
    mAttributes[cat] = cacheValues;
  }
  printCachedAttributes();
}

void QgsGrassVectorMapLayer::reinsertAttributes( int cat, QString &error )
{
  QgsDebugMsg( QString( "mField = %1 cat = %2" ).arg( mField ).arg( cat ) );

  if ( !mHasTable )
  {
    error = tr( "no table" );
    return;
  }

  if ( mAttributes.contains( cat ) )
  {
    QStringList names;
    QStringList values;

    names << mFieldInfo->key;
    values << QString::number( cat );

    if ( mAttributes.contains( cat ) )
    {
      Q_FOREACH ( const QgsField& f, mTableFields )
      {
        QString name = f.name();
        if ( name == mFieldInfo->key )
        {
          continue;
        }
        int index = mAttributeFields.indexFromName( name );
        QVariant valueVariant = mAttributes.value( cat ).value( index );
        if ( !valueVariant.isNull() )
        {
          names << name;
          values << quotedValue( valueVariant );
        }
      }
    }

    QString query = QString( "INSERT INTO %1 ( %2 ) VALUES ( %3 )" ).arg( mFieldInfo->table, names.join( ", " ), values.join( "," ) );
    executeSql( query, error );
  }
  else
  {
    QgsDebugMsg( "cat not found in mAttributes -> don't restore" );
  }
  printCachedAttributes();
}

void QgsGrassVectorMapLayer::updateAttributes( int cat, QgsFeature &feature, QString &error, bool nullValues )
{
  Q_UNUSED( nullValues )
  QgsDebugMsg( QString( "mField = %1 cat = %2" ).arg( mField ).arg( cat ) );

  if ( !mHasTable )
  {
    error = tr( "Table does not exist" );
    return;
  }
  if ( !feature.isValid() || !feature.fields() )
  {
    error = tr( "Feature invalid" );
    return;
  }

  QStringList updates;
  QMap<int, QVariant> cacheUpdates;
  // append feature attributes if not null
  for ( int i = 0; i < feature.fields()->size(); i++ )
  {
    QString name = feature.fields()->at( i ).name();
    if ( name == mFieldInfo->key )
    {
      continue;
    }
    QVariant valueVariant = feature.attributes().value( i );

    int cacheIndex = mAttributeFields.indexFromName( name );

    // Merging old and new attributes currently not allowed (entering changing cat)
#if 0
    if ( valueVariant.isNull() && !nullValues )
    {
      // update feature null values by existing values
      if ( cacheIndex != -1 )
      {
        feature.setAttribute( i, mAttributes[cat].at( cacheIndex ) );
      }
      continue;
    }
#endif

    updates << name + " = " + quotedValue( valueVariant );


    if ( cacheIndex == -1 )
    {
      QgsDebugMsg( "cannot find cache index for attribute " + name );
    }
    else
    {
      cacheUpdates[cacheIndex] = valueVariant;
    }
  }

  if ( updates.isEmpty() )
  {
    QgsDebugMsg( "nothing to update" );
    return;
  }

  QString query = QString( "UPDATE %1 SET %2 WHERE %3 = %4" ).arg( mFieldInfo->table,
                  updates.join( ", " ), mFieldInfo->key ).arg( cat );

  executeSql( query, error );
  if ( error.isEmpty() )
  {
    Q_FOREACH ( int index, cacheUpdates.keys() )
    {
      mAttributes[cat][index] = cacheUpdates[index];
    }
  }
  printCachedAttributes();
}

void QgsGrassVectorMapLayer::deleteAttribute( int cat, QString &error )
{
  QgsDebugMsg( QString( "mField = %1 cat = %2" ).arg( mField ).arg( cat ) );

  QString query = QString( "DELETE FROM %1 WHERE %2 = %3" ).arg( mFieldInfo->table, mFieldInfo->key ).arg( cat );
  executeSql( query, error );
}

bool QgsGrassVectorMapLayer::recordExists( int cat, QString &error )
{
  QgsDebugMsg( QString( "mField = %1 cat = %2" ).arg( mField ).arg( cat ) );
  if ( !mDriver )
  {
    error = tr( "Driver is not open" );
    QgsDebugMsg( error );
    return false;
  }

  QgsDebugMsg( "Database open -> select record" );

  // DBF driver in GRASS does not support count(*)
  dbValue value;
  int nValues = db_select_value( mDriver, mFieldInfo->table, mFieldInfo->key, cat, mFieldInfo->key, &value );
  if ( nValues == -1 )
  {
    error = tr( "Cannot select record from table" );
    return false;
  }

  return nValues > 0;
}

bool QgsGrassVectorMapLayer::isOrphan( int cat, QString &error )
{
  QgsDebugMsg( QString( "mField = %1 cat = %2" ).arg( mField ).arg( cat ) );


  // Check first if another line with such cat exists
  int fieldIndex = Vect_cidx_get_field_index( mMap->map(), mField );
  if ( fieldIndex >= 0 )
  {
    // There is a bug in GRASS: https://lists.osgeo.org/pipermail/grass-dev/2015-October/076921.html
    // -> check num cats first
    if ( Vect_cidx_get_num_cats_by_index( mMap->map(), fieldIndex ) == 0 )
    {
      QgsDebugMsg( "no more cats" );
      return true;
    }

    int t, id;
    int ret = Vect_cidx_find_next( mMap->map(), fieldIndex, cat,
                                   GV_POINTS | GV_LINES | GV_FACE, 0, &t, &id );

    if ( ret >= 0 )
    {
      QgsDebugMsg( "category exists" );
      return false;
    }
  }

  return recordExists( cat, error );
}

void QgsGrassVectorMapLayer::changeAttributeValue( int cat, QgsField field, QVariant value, QString &error )
{
  QgsDebugMsg( QString( "cat = %1 field.name() = %2 value = %3" ).arg( cat ).arg( field.name(), value.toString() ) );

  if ( !mDriver )
  {
    error = tr( "Driver is not open" );
    QgsDebugMsg( error );
    return;
  }

  bool exists = recordExists( cat, error );
  if ( !error.isEmpty() )
  {
    error = tr( "Cannot check if record exists" ) + ": " + error;
    return;
  }

  dbString dbstr;
  db_init_string( &dbstr );
  QString valueString = quotedValue( value );
  QString query;

  if ( exists )
  {
    query = QString( "UPDATE %1 SET %2 = %3 WHERE %4 = %5" ).arg( mFieldInfo->table,
            field.name(), valueString, mFieldInfo->key ).arg( cat );
  }
  else
  {
    QStringList names;
    QStringList values;
    names << mFieldInfo->key;
    values << QString::number( cat );
    names << field.name();
    values << quotedValue( value );
    query = QString( "INSERT INTO %1 ( %2 ) VALUES ( %3 )" ).arg( mFieldInfo->table,
            names.join( ", " ), values.join( "," ) );
  }

  QgsDebugMsg( QString( "query: %1" ).arg( query ) );

  // For some strange reason, mEncoding->fromUnicode(query) does not work,
  // but probably it is not correct, because Qt widgets will use current locales for input
  //  -> it is possible to edit only in current locales at present
  // QCString qcs = mEncoding->fromUnicode(query);

  QByteArray qcs = query.toUtf8();
  QgsDebugMsg( QString( "qcs: %1" ).arg( qcs.data() ) );

  char *cs = new char[qcs.length() + 1];
  strcpy( cs, ( const char * )qcs );
  db_set_string( &dbstr, cs );
  delete[] cs;

  QgsDebugMsg( QString( "SQL: %1" ).arg( db_get_string( &dbstr ) ) );

  int ret = db_execute_immediate( mDriver, &dbstr );
  db_free_string( &dbstr );
  if ( ret != DB_OK )
  {
    error = QString::fromLatin1( db_get_error_msg() );
    QgsDebugMsg( error );
    return;
  }


  // update cached attributes which may be used by another feature/layer
  if ( !mAttributes.contains( cat ) )
  {
    QgsDebugMsgLevel( QString( "cat %1 not found in attributes -> insert" ).arg( cat ), 3 );
    QList<QVariant> values;
    for ( int i = 0; i < mAttributeFields.size(); i++ )
    {
      values << QVariant();
    }
    mAttributes.insert( cat, values );
  }
  int index = mAttributeFields.indexFromName( field.name() );
  if ( index == -1 )
  {
    error = tr( "Field %1 not found in cached attributes" ).arg( field.name() );
    return;
  }
  mAttributes[cat][index] = value;
  printCachedAttributes();
}

void QgsGrassVectorMapLayer::printCachedAttributes()
{
#ifdef QGISDEBUG
  QgsDebugMsgLevel( QString( "mAttributes.size() = %1" ).arg( mAttributes.size() ), 4 );
  QStringList names;
  Q_FOREACH ( const QgsField& field, mAttributeFields )
  {
    names << field.name();
  }
  QgsDebugMsgLevel( names.join( "|" ), 4 );

  Q_FOREACH ( int cat, mAttributes.keys() )
  {
    QStringList values;
    for ( int i = 0; i <  mAttributes.value( cat ).size(); i++ )
    {
      values << mAttributes.value( cat ).value( i ).toString();
    }
    QgsDebugMsgLevel( QString( "cat = %1 : %2" ).arg( cat ).arg( values.join( "|" ) ), 4 );
  }
#endif
}
