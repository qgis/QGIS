/***************************************************************************
  qgspostgresconn.cpp  -  connection class to PostgreSQL/PostGIS
                             -------------------
    begin                : 2011/01/28
    copyright            : (C) 2011 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspostgresresult.h"

#include <QObject>

QgsPostgresResult::~QgsPostgresResult()
{
  if ( mRes )
    ::PQclear( mRes );
  mRes = nullptr;
}

QgsPostgresResult &QgsPostgresResult::operator=( PGresult *result )
{
  if ( mRes )
    ::PQclear( mRes );
  mRes = result;
  return *this;
}

QgsPostgresResult &QgsPostgresResult::operator=( const QgsPostgresResult &src )
{
  if ( this == &src )
    return *this;

  if ( mRes )
    ::PQclear( mRes );
  mRes = src.result();
  return *this;
}

ExecStatusType QgsPostgresResult::PQresultStatus()
{
  return mRes ? ::PQresultStatus( mRes ) : PGRES_FATAL_ERROR;
}

QString QgsPostgresResult::PQresultErrorMessage()
{
  return mRes ? QString::fromUtf8( ::PQresultErrorMessage( mRes ) ) : QObject::tr( "no result buffer" );
}

int QgsPostgresResult::PQntuples()
{
  Q_ASSERT( mRes );
  return ::PQntuples( mRes );
}

QString QgsPostgresResult::PQgetvalue( int row, int col )
{
  Q_ASSERT( mRes );
  return PQgetisnull( row, col )
         ? QString()
         : QString::fromUtf8( ::PQgetvalue( mRes, row, col ) );
}

bool QgsPostgresResult::PQgetisnull( int row, int col )
{
  Q_ASSERT( mRes );
  return ::PQgetisnull( mRes, row, col );
}

int QgsPostgresResult::PQnfields()
{
  Q_ASSERT( mRes );
  return ::PQnfields( mRes );
}

QString QgsPostgresResult::PQfname( int col )
{
  Q_ASSERT( mRes );
  return QString::fromUtf8( ::PQfname( mRes, col ) );
}

Oid QgsPostgresResult::PQftable( int col )
{
  Q_ASSERT( mRes );
  return ::PQftable( mRes, col );
}

int QgsPostgresResult::PQftablecol( int col )
{
  Q_ASSERT( mRes );
  return ::PQftablecol( mRes, col );
}

Oid QgsPostgresResult::PQftype( int col )
{
  Q_ASSERT( mRes );
  return ::PQftype( mRes, col );
}

int QgsPostgresResult::PQfmod( int col )
{
  Q_ASSERT( mRes );
  return ::PQfmod( mRes, col );
}

Oid QgsPostgresResult::PQoidValue()
{
  Q_ASSERT( mRes );
  return ::PQoidValue( mRes );
}
