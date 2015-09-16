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
#include "qgsgrassvectormap.h"
#include "qgsgrassvectormaplayer.h"

extern "C"
{
#include <grass/version.h>
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
    , mHasTable( false )
    , mKeyColumn( -1 )
    , mUsers( 0 )
{
}

void QgsGrassVectorMapLayer::clear()
{
  QgsDebugMsg( "entered" );
  mFields.clear();
  mAttributes.clear();
  mMinMax.clear();
  mKeyColumn = -1;
  mValid = false;
  G_free( mFieldInfo );
  mFieldInfo = 0;
}

void QgsGrassVectorMapLayer::load()
{
  QgsDebugMsg( "entered" );
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

    QgsGrass::lock();
    dbDriver *databaseDriver = 0;
    QString error = QString( "Cannot open database %1 by driver %2" ).arg( mFieldInfo->database ).arg( mFieldInfo->driver );
    G_TRY
    {
      databaseDriver = db_start_driver_open_database( mFieldInfo->driver, mFieldInfo->database );
    }
    G_CATCH( QgsGrass::Exception &e )
    {
      QgsGrass::warning( error + " : " + e.what() );
    }

    if ( !databaseDriver )
    {
      QgsDebugMsg( error );
    }
    else
    {
      QgsDebugMsg( "Database opened -> open select cursor" );
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
        int nRecords = db_get_num_rows( &databaseCursor );
        QgsDebugMsg( QString( "Number of records: %1" ).arg( nRecords ) );

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
          mFields.append( QgsField( db_get_column_name( column ), qtype, ctypeStr,
                                    db_get_column_length( column ), db_get_column_precision( column ) ) );
          mMinMax << minMax;
          if ( G_strcasecmp( db_get_column_name( column ), mFieldInfo->key ) == 0 )
          {
            mKeyColumn = i;
          }
        }

        if ( mKeyColumn < 0 )
        {
          mFields.clear();
          QgsGrass::warning( QObject::tr( "Key column '%1' not found in the table '%2'" ).arg( mFieldInfo->key ).arg( mFieldInfo->table ) );
        }
        else
        {
          mHasTable = true;
          // Read attributes to the memory
          while ( true )
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

              QgsDebugMsgLevel( QString( "column = %1 value = %2" ).arg( db_get_column_name( column ) ).arg( db_get_string( &dbstr ) ), 3 );

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
                  default:
                    variant = QVariant( QByteArray( db_get_string( &dbstr ) ) );
                }
              }
              QgsDebugMsgLevel( QString( "column = %1 variant = %2" ).arg( db_get_column_name( column ) ).arg( variant.toString() ), 3 );
              values << variant;
            }
            mAttributes.insert( cat, values );
          }
        }
        mValid = true;
        db_close_cursor( &databaseCursor );
        db_close_database_shutdown_driver( databaseDriver );
        db_free_string( &dbstr );

        QgsDebugMsg( QString( "mFields.size = %1" ).arg( mFields.size() ) );
        QgsDebugMsg( QString( "number of attributes = %1" ).arg( mAttributes.size() ) );
      }
    }
    QgsGrass::unlock();
  }

  // Add cat if no attribute fields exist (otherwise qgis crashes)
  if ( mFields.size() == 0 )
  {
    mKeyColumn = 0;
    mFields.append( QgsField( "cat", QVariant::Int, "integer" ) );
    QPair<double, double> minMax( 0, 0 );

    int cidx = Vect_cidx_get_field_index( mMap->map(), mField );
    if ( cidx >= 0 )
    {
      int ncats, cat, type, id;

      ncats = Vect_cidx_get_num_cats_by_index( mMap->map(), cidx );

      if ( ncats > 0 )
      {
        Vect_cidx_get_cat_by_index( mMap->map(), cidx, 0, &cat, &type, &id );
        minMax.first = cat;

        Vect_cidx_get_cat_by_index( mMap->map(), cidx, ncats - 1, &cat, &type, &id );
        minMax.second = cat;
      }
    }
    mMinMax << minMax;
  }

  // TODO
  // add topo field for editing
  mFields.append( QgsField( "topo_symbol", QVariant::Int, "integer" ) );

  QgsDebugMsg( QString( "layer loaded mFields.size() = %1 mAttributes.size() = %2" ).arg( mFields.size() ).arg( mAttributes.size() ) );
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
  mMap->closeLayer( this );
}
