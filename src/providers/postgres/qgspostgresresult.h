/***************************************************************************
  qgspostgresconn.h  -  connection class to PostgreSQL/PostGIS
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

#ifndef QGSPOSTGRESRESULT_H
#define QGSPOSTGRESRESULT_H


#include <QString>

extern "C"
{
#include <libpq-fe.h>
}

class QgsPostgresResult
{
  public:
    explicit QgsPostgresResult( PGresult *result = nullptr ) : mRes( result ) {}
    ~QgsPostgresResult();

    QgsPostgresResult &operator=( PGresult *result );
    QgsPostgresResult &operator=( const QgsPostgresResult &src );

    QgsPostgresResult( const QgsPostgresResult &rh ) = delete;

    ExecStatusType PQresultStatus();
    QString PQresultErrorMessage();

    int PQntuples();
    QString PQgetvalue( int row, int col );
    bool PQgetisnull( int row, int col );

    int PQnfields();
    QString PQfname( int col );
    Oid PQftable( int col );
    Oid PQftype( int col );
    int PQfmod( int col );
    int PQftablecol( int col );
    Oid PQoidValue();

    PGresult *result() const { return mRes; }

  private:
    PGresult *mRes = nullptr;

};

struct PGException
{
    explicit PGException( QgsPostgresResult &r ) : mWhat( r.PQresultErrorMessage() )
    {
    }
    QString errorMessage() const
    {
      return mWhat;
    }
  private:
    QString mWhat;
};
#endif
