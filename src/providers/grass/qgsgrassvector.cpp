/***************************************************************************
                          qgsgrassvector.cpp
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

#include "qgsgrassvector.h"
#include "moc_qgsgrassvector.cpp"

extern "C"
{
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <grass/version.h>
#if defined( _MSC_VER ) && defined( M_PI_4 )
#undef M_PI_4 //avoid redefinition warning
#endif
#include <grass/gprojects.h>
#include <grass/vector.h>
#include <grass/raster.h>
}

QgsGrassVectorLayer::QgsGrassVectorLayer( QObject *parent )
  : QObject( parent )
{
}

QgsGrassVectorLayer::QgsGrassVectorLayer( const QgsGrassObject &grassObject, int number, struct field_info *fieldInfo, QObject *parent )
  : QObject( parent )
  , mGrassObject( grassObject )
  , mNumber( number )
{
  if ( fieldInfo )
  {
    mName = fieldInfo->name;
    mDriver = fieldInfo->driver;
    mDatabase = fieldInfo->database;
    mTable = fieldInfo->table;
    mKey = fieldInfo->key;
  }
}

int QgsGrassVectorLayer::typeCount( int type ) const
{
  int count = 0;
  for ( auto it = mTypeCounts.constBegin(); it != mTypeCounts.constEnd(); ++it )
  {
    if ( it.key() & type )
    {
      count += it.value();
    }
  }
  return count;
}

int QgsGrassVectorLayer::type() const
{
  int type = 0;
  for ( auto it = mTypeCounts.constBegin(); it != mTypeCounts.constEnd(); ++it )
  {
    if ( it.value() > 0 )
    {
      type |= it.key();
    }
  }
  return type;
}

QList<int> QgsGrassVectorLayer::types() const
{
  QList<int> types;
  for ( auto it = mTypeCounts.constBegin(); it != mTypeCounts.constEnd(); ++it )
  {
    if ( it.value() > 0 )
    {
      types << it.key();
    }
  }
  return types;
}

QgsFields QgsGrassVectorLayer::fields()
{
  QString dblnPath = mGrassObject.mapsetPath() + "/vector/" + mGrassObject.name() + "/dbln";
  QgsDebugMsgLevel( "dblnPath = " + dblnPath, 2 );
  QFileInfo dblnFileInfo( dblnPath );
  if ( !dblnFileInfo.exists() )
  {
    QgsDebugError( "dbln does not exist" );
    mFields.clear();
    mFieldsTimeStamp.setSecsSinceEpoch( 0 );
    return mFields;
  }
  if ( dblnFileInfo.lastModified() > mFieldsTimeStamp && !mDriver.isEmpty()
       && !mDatabase.isEmpty() && !mTable.isEmpty() && !mKey.isEmpty() )
  {
    QgsDebugMsgLevel( "reload fields", 2 );
    mError.clear();
    mFields.clear();
    mFieldsTimeStamp = dblnFileInfo.lastModified();

    QgsDebugMsgLevel( "open database " + mDatabase + " by driver " + mDriver, 2 );
    QgsGrass::lock();
    QgsGrass::setMapset( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset() );
    dbDriver *driver = db_start_driver_open_database( mDriver.toUtf8().constData(), mDatabase.toUtf8().constData() );

    if ( !driver )
    {
      mError = QObject::tr( "Cannot open database %1 by driver %2" ).arg( mDatabase, mDatabase );
      QgsDebugError( mError );
    }
    else
    {
      QgsDebugMsgLevel( "Database opened -> describe table " + mTable, 2 );

      dbString tableName;
      db_init_string( &tableName );
      db_set_string( &tableName, mTable.toUtf8().constData() );

      dbTable *table = nullptr;
      if ( db_describe_table( driver, &tableName, &table ) != DB_OK )
      {
        mError = QObject::tr( "Cannot describe table %1" ).arg( mTable );
        QgsDebugError( mError );
      }
      else
      {
        int nCols = db_get_table_number_of_columns( table );

        for ( int c = 0; c < nCols; c++ )
        {
          dbColumn *column = db_get_table_column( table, c );

          int ctype = db_sqltype_to_Ctype( db_get_column_sqltype( column ) );
          QString type;
          QMetaType::Type qtype = QMetaType::Type::QString; //default to string to prevent compiler warnings
          switch ( ctype )
          {
            case DB_C_TYPE_INT:
              type = QStringLiteral( "int" );
              qtype = QMetaType::Type::Int;
              break;
            case DB_C_TYPE_DOUBLE:
              type = QStringLiteral( "double" );
              qtype = QMetaType::Type::Double;
              break;
            case DB_C_TYPE_STRING:
              type = QStringLiteral( "string" );
              qtype = QMetaType::Type::QString;
              break;
            case DB_C_TYPE_DATETIME:
              type = QStringLiteral( "datetime" );
              qtype = QMetaType::Type::QString;
              break;
          }
          mFields.append( QgsField( db_get_column_name( column ), qtype, type, db_get_column_length( column ), 0 ) );
        }
      }
      db_close_database_shutdown_driver( driver );
    }
    QgsGrass::unlock();
  }
  QgsDebugMsgLevel( QString( "mFields.size() = %1" ).arg( mFields.size() ), 2 );
  return mFields;
}


/*********************** QgsGrassVector ***********************/
QgsGrassVector::QgsGrassVector( const QString &gisdbase, const QString &location, const QString &mapset, const QString &name, QObject *parent )
  : QObject( parent )
  , mGrassObject( gisdbase, location, mapset, name )
  , mNodeCount( 0 )
{
}

QgsGrassVector::QgsGrassVector( const QgsGrassObject &grassObject, QObject *parent )
  : QObject( parent )
  , mGrassObject( grassObject )
  , mNodeCount( 0 )
{
}

bool QgsGrassVector::openHead()
{
  const auto constMLayers = mLayers;
  for ( QgsGrassVectorLayer *layer : constMLayers )
  {
    layer->deleteLater();
  }
  mLayers.clear();

  QgsGrass::lock();
  QgsDebugMsgLevel( "mGrassObject = " + mGrassObject.toString(), 2 );
  QStringList list;

  // Set location
  QgsGrass::setLocation( mGrassObject.gisdbase(), mGrassObject.location() );

  /* Open vector */
  QgsGrass::resetError();
  Vect_set_open_level( 2 );

  // TODO: We are currently using vectDestroyMapStruct in G_CATCH blocks because we know
  // that it cannot call another G_fatal_error, but once we switch to hypothetical Vect_destroy_map_struct
  // it should be verified if it can still be in G_CATCH
  struct Map_info *map = nullptr;
  int level = -1;

  // Vect_open_old_head GRASS is raising fatal error if topo exists but it is in different (older) version.
  // It means that even we could open it on level one, it ends with exception,
  // but we need level 2 anyway to get list of layers, so it does not matter, only the error message may be misleading.

  G_TRY
  {
    map = QgsGrass::vectNewMapStruct();

    level = Vect_open_old_head( map, mGrassObject.name().toUtf8().constData(), mGrassObject.mapset().toUtf8().constData() );
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    QgsDebugError( QString( "Cannot open GRASS vectvectorTypesor: %1" ).arg( e.what() ) );
    QgsGrass::vectDestroyMapStruct( map );
    mError = e.what();
    QgsGrass::unlock();
    return false;
  }

  // TODO: Handle errors as exceptions. Do not open QMessageBox here! This method is also used in browser
  // items which are populated in threads and creating dialog QPixmap is causing crash or even X server freeze.
  if ( level == 1 )
  {
    QgsDebugError( "Cannot open vector on level 2" );
    // Do not open QMessageBox here!
    //QMessageBox::warning( 0, QObject::tr( "Warning" ), QObject::tr( "Cannot open vector %1 in mapset %2 on level 2 (topology not available, try to rebuild topology using v.build module)." ).arg( mapName ).arg( mapset ) );
    // Vect_close here is correct, it should work, but it seems to cause
    // crash on win http://trac.osgeo.org/qgis/ticket/2003
    // disabled on win test it
#ifndef Q_OS_WIN
    Vect_close( map );
#endif
    QgsGrass::vectDestroyMapStruct( map );
    QgsGrass::unlock();
    mError = tr( "Cannot open vector on level 2" );
    return false;
  }
  else if ( level < 1 )
  {
    QgsDebugError( "Cannot open vector" );
    // Do not open QMessageBox here!
    //QMessageBox::warning( 0, QObject::tr( "Warning" ), QObject::tr( "Cannot open vector %1 in mapset %2" ).arg( mapName ).arg( mapset ) );
    QgsGrass::vectDestroyMapStruct( map );
    QgsGrass::unlock();
    mError = tr( "Cannot open vector" );
    return false;
  }

  QgsDebugMsgLevel( "GRASS vector successfully opened", 2 );

  G_TRY
  {
    // Get layers
    int ncidx = Vect_cidx_get_num_fields( map );

    for ( int i = 0; i < ncidx; i++ )
    {
      const int field = Vect_cidx_get_field_number( map, i );
      if ( field <= 0 )
        continue;

      QgsDebugMsgLevel( QString( "i = %1 layer = %2" ).arg( i ).arg( field ), 2 );

      struct field_info *fieldInfo = Vect_get_field( map, field );

      QgsGrassVectorLayer *layer = new QgsGrassVectorLayer( mGrassObject, field, fieldInfo, this );
      const auto typeMap = QgsGrass::vectorTypeMap();
      for ( auto it = typeMap.constBegin(); it != typeMap.constEnd(); ++it )
      {
        int count = Vect_cidx_get_type_count( map, field, it.key() );
        if ( count > 0 )
        {
          QgsDebugMsgLevel( QString( "type = %1 count = %2" ).arg( it.key() ).arg( count ), 2 );
          layer->setTypeCount( it.key(), count );
        }
      }
      mLayers.append( layer );
    }
    QgsDebugMsgLevel( "standard layers listed: " + list.join( "," ), 2 );

    // Get primitives
    const auto typeMap = QgsGrass::vectorTypeMap();
    for ( auto it = typeMap.constBegin(); it != typeMap.constEnd(); ++it )
    {
      if ( it.key() == GV_AREA )
      {
        continue;
      }
      int count = Vect_get_num_primitives( map, it.key() );
      if ( count > 0 )
      {
        QgsDebugMsgLevel( QString( "primitive type = %1 count = %2" ).arg( it.key() ).arg( count ), 2 );
        mTypeCounts[it.key()] = count;
      }
    }
    mNodeCount = Vect_get_num_nodes( map );
    QgsDebugMsgLevel( QString( "mNodeCount = %2" ).arg( mNodeCount ), 2 );

    Vect_close( map );
    QgsGrass::vectDestroyMapStruct( map );
    QgsGrass::unlock();
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    QgsDebugError( QString( "Cannot get vector layers: %1" ).arg( e.what() ) );
    QgsGrass::vectDestroyMapStruct( map );
    mError = e.what();
    QgsGrass::unlock();
    return false;
  }
  return true;
}

int QgsGrassVector::typeCount( int type ) const
{
  int count = 0;
  for ( auto it = mTypeCounts.constBegin(); it != mTypeCounts.constEnd(); ++it )
  {
    if ( it.key() & type )
    {
      count += it.value();
    }
  }
  return count;
}

int QgsGrassVector::maxLayerNumber() const
{
  int max = 0;
  const auto constMLayers = mLayers;
  for ( QgsGrassVectorLayer *layer : constMLayers )
  {
    max = std::max( max, layer->number() );
  }
  return max;
}
