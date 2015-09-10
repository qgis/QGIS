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

extern "C"
{
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <grass/version.h>
#include <grass/gprojects.h>

#if GRASS_VERSION_MAJOR < 7
#include <grass/Vect.h>
#else
#include <grass/vector.h>
#include <grass/raster.h>
#endif
}

QgsGrassVectorLayer::QgsGrassVectorLayer( QObject * parent )
    : QObject( parent )
    , mNumber( 0 )
{
}

QgsGrassVectorLayer::QgsGrassVectorLayer( const QgsGrassObject &grassObject, int number, struct field_info *fieldInfo, QObject * parent )
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

QgsGrassVectorLayer::~QgsGrassVectorLayer()
{
}

int QgsGrassVectorLayer::typeCount( int type ) const
{
  int count = 0;
  foreach ( int t, mTypeCounts.keys() )
  {
    if ( t & type )
    {
      count += mTypeCounts.value( t );
    }
  }
  return count;
}

int QgsGrassVectorLayer::type() const
{
  int type = 0;
  foreach ( int t, mTypeCounts.keys() )
  {
    if ( mTypeCounts.value( t ) > 0 )
    {
      type |= t;
    }
  }
  return type;
}

QList<int> QgsGrassVectorLayer::types() const
{
  QList<int> types;
  foreach ( int t, mTypeCounts.keys() )
  {
    if ( mTypeCounts.value( t ) > 0 )
    {
      types << t;
    }
  }
  return types;
}

QgsFields QgsGrassVectorLayer::fields()
{
  QString dblnPath = mGrassObject.mapsetPath() + "/vector/" + mGrassObject.name() + "/dbln";
  QgsDebugMsg( "dblnPath = " + dblnPath );
  QFileInfo dblnFileInfo( dblnPath );
  if ( !dblnFileInfo.exists() )
  {
    QgsDebugMsg( "dbln does not exist" );
    mFields.clear();
    mFieldsTimeStamp.setTime_t( 0 );
    return mFields;
  }
  if ( dblnFileInfo.lastModified() >  mFieldsTimeStamp && !mDriver.isEmpty()
       && !mDatabase.isEmpty() && !mTable.isEmpty() && !mKey.isEmpty() )
  {
    QgsDebugMsg( "reload fields" );
    mError.clear();
    mFields.clear();
    mFieldsTimeStamp = dblnFileInfo.lastModified();

    QgsDebugMsg( "open database " + mDatabase + " by driver " + mDriver );
    QgsGrass::lock();
    QgsGrass::setMapset( mGrassObject.gisdbase(), mGrassObject.location(),  mGrassObject.mapset() );
    dbDriver *driver = db_start_driver_open_database( mDriver.toUtf8().data(), mDatabase.toUtf8().data() );

    if ( !driver )
    {
      mError = QObject::tr( "Cannot open database %1 by driver %2" ).arg( mDatabase ).arg( mDatabase );
      QgsDebugMsg( mError );
    }
    else
    {
      QgsDebugMsg( "Database opened -> describe table " + mTable );

      dbString tableName;
      db_init_string( &tableName );
      db_set_string( &tableName, mTable.toUtf8().data() );

      dbTable *table;
      if ( db_describe_table( driver, &tableName, &table ) != DB_OK )
      {
        mError = QObject::tr( "Cannot describe table %1" ).arg( mTable );
        QgsDebugMsg( mError );
      }
      else
      {
        int nCols = db_get_table_number_of_columns( table );

        for ( int c = 0; c < nCols; c++ )
        {
          dbColumn *column = db_get_table_column( table, c );

          int ctype = db_sqltype_to_Ctype( db_get_column_sqltype( column ) );
          QString type;
          QVariant::Type qtype = QVariant::String; //default to string to prevent compiler warnings
          switch ( ctype )
          {
            case DB_C_TYPE_INT:
              type = "int";
              qtype = QVariant::Int;
              break;
            case DB_C_TYPE_DOUBLE:
              type = "double";
              qtype = QVariant::Double;
              break;
            case DB_C_TYPE_STRING:
              type = "string";
              qtype = QVariant::String;
              break;
            case DB_C_TYPE_DATETIME:
              type = "datetime";
              qtype = QVariant::String;
              break;
          }
          mFields.append( QgsField( db_get_column_name( column ), qtype, type, db_get_column_length( column ), 0 ) );
        }
      }
      db_close_database_shutdown_driver( driver );
    }
    QgsGrass::unlock();
  }
  QgsDebugMsg( QString( "mFields.size() = %1" ).arg( mFields.size() ) );
  return mFields;
}


/*********************** QgsGrassVector ***********************/
QgsGrassVector::QgsGrassVector( const QString& gisdbase, const QString& location, const QString& mapset,
                                const QString& name, QObject *parent )
    : QObject( parent )
    , mGrassObject( gisdbase, location, mapset, name )
{
}

QgsGrassVector::QgsGrassVector( const QgsGrassObject& grassObject, QObject *parent )
    : QObject( parent )
    , mGrassObject( grassObject )
{
}

bool QgsGrassVector::openHead()
{
  foreach ( QgsGrassVectorLayer *layer, mLayers )
  {
    layer->deleteLater();
  }
  mLayers.clear();

  QgsGrass::lock();
  QgsDebugMsg( "mGrassObject = " + mGrassObject.toString() );
  QStringList list;

  // Set location
  QgsGrass::setLocation( mGrassObject.gisdbase(), mGrassObject.location() );

  /* Open vector */
  QgsGrass::resetError();
  //Vect_set_open_level( 2 );

  // TODO: We are currently using vectDestroyMapStruct in G_CATCH blocks because we know
  // that it cannot call another G_fatal_error, but once we switch to hypothetical Vect_destroy_map_struct
  // it should be verified if it can still be in G_CATCH
  struct Map_info *map = 0;
  int level = -1;

  // Vect_open_old_head GRASS is raising fatal error if topo exists but it is in different (older) version.
  // It means that even we could open it on level one, it ends with exception,
  // but we need level 2 anyway to get list of layers, so it does not matter, only the error message may be misleading.

  G_TRY
  {
    map = QgsGrass::vectNewMapStruct();
    level = Vect_open_old_head( map, ( char * ) mGrassObject.name().toUtf8().data(), ( char * ) mGrassObject.mapset().toUtf8().data() );
  }
  G_CATCH( QgsGrass::Exception &e )
  {
    QgsDebugMsg( QString( "Cannot open GRASS vectvectorTypesor: %1" ).arg( e.what() ) );
    QgsGrass::vectDestroyMapStruct( map );
    mError = e.what();
    QgsGrass::unlock();
    return false;
  }

  // TODO: Handle errors as exceptions. Do not open QMessageBox here! This method is also used in browser
  // items which are populated in threads and creating dialog QPixmap is causing crash or even X server freeze.
  if ( level == 1 )
  {
    QgsDebugMsg( "Cannot open vector on level 2" );
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
    QgsDebugMsg( "Cannot open vector" );
    // Do not open QMessageBox here!
    //QMessageBox::warning( 0, QObject::tr( "Warning" ), QObject::tr( "Cannot open vector %1 in mapset %2" ).arg( mapName ).arg( mapset ) );
    QgsGrass::vectDestroyMapStruct( map );
    QgsGrass::unlock();
    mError = tr( "Cannot open vector" );
    return false;
  }

  QgsDebugMsg( "GRASS vector successfully opened" );

  G_TRY
  {
    // Get layers
    int ncidx = Vect_cidx_get_num_fields( map );

    for ( int i = 0; i < ncidx; i++ )
    {
      int field = Vect_cidx_get_field_number( map, i );
      QgsDebugMsg( QString( "i = %1 layer = %2" ).arg( i ).arg( field ) );

      struct field_info *fieldInfo = Vect_get_field( map, field ); // should work also with field = 0

      QgsGrassVectorLayer *layer = new QgsGrassVectorLayer( mGrassObject, field, fieldInfo, this );
      foreach ( int type, QgsGrass::vectorTypeMap().keys() )
      {
        int count = Vect_cidx_get_type_count( map, field, type );
        if ( count > 0 )
        {
          QgsDebugMsg( QString( "type = %1 count = %2" ).arg( type ).arg( count ) );
          layer->setTypeCount( type, count );
        }
      }
      mLayers.append( layer );
    }
    QgsDebugMsg( "standard layers listed: " + list.join( "," ) );

    // Get primitives
    foreach ( int type, QgsGrass::vectorTypeMap().keys() )
    {
      if ( type == GV_AREA )
      {
        continue;
      }
      int count = Vect_get_num_primitives( map, type );
      if ( count > 0 )
      {
        QgsDebugMsg( QString( "primitive type = %1 count = %2" ).arg( type ).arg( count ) );
        mTypeCounts[type] = count;
      }
    }
    mNodeCount = Vect_get_num_nodes( map );
    QgsDebugMsg( QString( "mNodeCount = %2" ).arg( mNodeCount ) );

    Vect_close( map );
    QgsGrass::vectDestroyMapStruct( map );
    QgsGrass::unlock();
  }
  G_CATCH( QgsGrass::Exception &e )
  {
    QgsDebugMsg( QString( "Cannot get vector layers: %1" ).arg( e.what() ) );
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
  foreach ( int t, mTypeCounts.keys() )
  {
    if ( t & type )
    {
      count += mTypeCounts.value( t );
    }
  }
  return count;
}
