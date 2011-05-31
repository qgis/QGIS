/***************************************************************************
                qgspgutil.cpp - PostgreSQL Utility Functions
                     --------------------------------------
               Date                 : 2004-11-21
               Copyright            : (C) 2004 by Gary E.Sherman
               Email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspgutil.h"

QgsPgUtil *QgsPgUtil::mInstance = 0;
QgsPgUtil * QgsPgUtil::instance()
{
  if ( mInstance == 0 )
  {
    mInstance = new QgsPgUtil();
  }
  return mInstance;
}

QgsPgUtil::QgsPgUtil()
{
}

QgsPgUtil::~QgsPgUtil()
{
}

void QgsPgUtil::setConnection( PGconn *con )
{
  mPgConnection = con;
}
PGconn *QgsPgUtil::connection()
{
  return mPgConnection;
}

QString QgsPgUtil::quotedIdentifier( QString ident )
{
  ident.replace( '"', "\"\"" );
  return ident.prepend( "\"" ).append( "\"" );
}

QString QgsPgUtil::quotedValue( QString value )
{
  if ( value.isNull() )
    return "NULL";

  // FIXME: use PQescapeStringConn
  value.replace( "'", "''" );
  return value.prepend( "'" ).append( "'" );
}
