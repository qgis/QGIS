/***************************************************************************
    qgsslconnect.cpp - thin wrapper class to connect to spatialite databases
    ----------------------
    begin                : May 2015
    copyright            : (C) 2015 by Jürgen fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsslconnect.h"

#include <sqlite3.h>
#include <spatialite.h>

QHash<sqlite3 *, void *> QgsSLConnect::mSLconns;

int QgsSLConnect::sqlite3_open( const char *filename, sqlite3 **ppDb )
{
#if defined(SPATIALITE_VERSION_GE_4_0_0)
  void *conn = spatialite_alloc_connection();
#else
  spatialite_init( 0 );
#endif

  int res = ::sqlite3_open( filename, ppDb );

#if defined(SPATIALITE_VERSION_GE_4_0_0)
  if ( res == SQLITE_OK )
  {
    spatialite_init_ex( *ppDb, conn, 0 );
    mSLconns.insert( *ppDb, conn );
  }
#endif

  return res;
}

int QgsSLConnect::sqlite3_close( sqlite3 *db )
{
  int res = ::sqlite3_close( db );

#if defined(SPATIALITE_VERSION_GE_4_0_0)
  if ( mSLconns.contains( db ) )
    spatialite_cleanup_ex( mSLconns.take( db ) );
#endif

  return res;
}

int QgsSLConnect::sqlite3_open_v2( const char *filename, sqlite3 **ppDb, int flags, const char *zVfs )
{
#if defined(SPATIALITE_VERSION_GE_4_0_0)
  void *conn = spatialite_alloc_connection();
#else
  spatialite_init( 0 );
#endif

  int res = ::sqlite3_open_v2( filename, ppDb, flags, zVfs );

#if defined(SPATIALITE_VERSION_GE_4_0_0)
  if ( res == SQLITE_OK )
  {
    spatialite_init_ex( *ppDb, conn, 0 );
    mSLconns.insert( *ppDb, conn );
  }
#endif

  return res;
}

int QgsSLConnect::sqlite3_close_v2( sqlite3 *db )
{
  int res = ::sqlite3_close( db );

#if defined(SPATIALITE_VERSION_GE_4_0_0)
  if ( mSLconns.contains( db ) )
    spatialite_cleanup_ex( mSLconns.take( db ) );
#endif

  return res;
}
