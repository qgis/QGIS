/***************************************************************************
    qgsslconnect.h - thin wrapper class to connect to spatialite databases
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

#ifndef QGSSLCONNECT_H
#define QGSSLCONNECT_H

#include <QHash>

struct sqlite3;

class CORE_EXPORT QgsSLConnect
{
  public:
    static int sqlite3_open( const char *filename, sqlite3 **ppDb );
    static int sqlite3_close( sqlite3* );

    static int sqlite3_open_v2( const char *filename, sqlite3 **ppDb, int flags, const char *zVfs );
    static int sqlite3_close_v2( sqlite3* );

#if defined(SPATIALITE_HAS_INIT_EX)
  private:
    static QHash<sqlite3 *, void *> mSLconns;
#endif
};

#endif

