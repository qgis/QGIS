/***************************************************************************
      qgspostgrescountthread.cpp  -  Multithreaded PostgreSQL layer count
                                     retrieval
                             -------------------
    begin                : Feb 1, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <fstream>
#include <QEvent>
#include <QApplication>

#include "qgis.h"
#include "qgsrectangle.h"

#include "qgsprovidercountcalcevent.h"

#include "qgspostgrescountthread.h"
#include "qgslogger.h"


/*
QgsPostgresCountThread::QgsPostgresCountThread()
 : QgsPostgresProvider(), QThread()
{



}


QgsPostgresCountThread::~QgsPostgresCountThread()
{
}
*/

void QgsPostgresCountThread::setConnInfo( QString s )
{
  connectionInfo = s;
}

void QgsPostgresCountThread::setTableName( QString s )
{
  tableName = s;
}

void QgsPostgresCountThread::setSqlWhereClause( QString s )
{
  sqlWhereClause = s;
}

void QgsPostgresCountThread::setGeometryColumn( QString s )
{
  geometryColumn = s;
}

//void QgsPostgresCountThread::setCallback( QgsPostgresProvider* o )
//{
//  callbackObject = o;
//}


void QgsPostgresCountThread::run()
{
//  // placeholders for now.
//  QString connectionInfo;

  QgsDebugMsg( "QgsPostgresCountThread: Started running." );

  // Open another connection to the database
  PGconn *connection = PQconnectdb( connectionInfo.toUtf8() );

  // get the extents

  QString sql = "select count(*) from " + tableName;
  if ( sqlWhereClause.length() > 0 )
  {
    sql += " where " + sqlWhereClause;
  }


  QgsDebugMsg( "QgsPostgresCountThread: About to issue query." );

  PGresult *result = PQexec( connection, sql.toUtf8() );

  QgsDebugMsg( "QgsPostgresCountThread: Query completed." );


  featuresCounted = QString( PQgetvalue( result, 0, 0 ) ).toLong();
  PQclear( result );

  QgsDebugMsg( QString( "QgsPostgresCountThread: Exact Number of features: %1" ).arg( featuresCounted ) );


  // Send some events (instead of a signal) as it is thread-safe

  // First we tell the object that invoked us that we have some new extents for her
  // Second we tell the application that the extents have changed, so that it
  // can go on and do any visual housekeeping (e.g. update the overview window)

  QgsDebugMsg( QString( "About to create and dispatch event %1 to callback" ).arg( QGis::ProviderCountCalcEvent ) );

  QgsProviderCountCalcEvent* e1 = new QgsProviderCountCalcEvent( featuresCounted );
  QApplication::postEvent(( QObject * )callbackObject, e1 );

//  QApplication::postEvent(qApp->mainWidget(), e1);

  QgsDebugMsg( QString( "Posted event %1 to callback" ).arg( QGis::ProviderCountCalcEvent ) );


  QgsDebugMsg( "About to finish connection." );

  // ending the thread, clean up
  PQfinish( connection );

  QgsDebugMsg( "About to complete running." );


}

