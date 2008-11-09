/***************************************************************************
      qgspostgresextentthread.cpp  -  Multithreaded PostgreSQL layer extents
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
#include <cstdlib>

#include <QEvent>
#include <QApplication>
#include <QEvent>

#include "qgis.h"
#include "qgsrectangle.h"


#include "qgspostgresextentthread.h"
#include "qgsproviderextentcalcevent.h"
#include "qgslogger.h"


/*
QgsPostgresExtentThread::QgsPostgresExtentThread()
 : QgsPostgresProvider(), QThread()
{



}


QgsPostgresExtentThread::~QgsPostgresExtentThread()
{
}
*/

void QgsPostgresExtentThread::setConnInfo( QString s )
{
  connectionInfo = s;
}

void QgsPostgresExtentThread::setTableName( QString s )
{
  tableName = s;
}

void QgsPostgresExtentThread::setSqlWhereClause( QString s )
{
  sqlWhereClause = s;
}

void QgsPostgresExtentThread::setGeometryColumn( QString s )
{
  geometryColumn = s;
}

//void QgsPostgresExtentThread::setCallback( QgsPostgresProvider* o )
//{
//  callbackObject = o;
//}


void QgsPostgresExtentThread::run()
{
//  // placeholders for now.
//  QString connectionInfo;

  QgsDebugMsg( "Started running." );

  // Open another connection to the database
  PGconn *connection = PQconnectdb( connectionInfo.toUtf8() );

  // get the extents

  QString sql = "select extent(" + geometryColumn + ") from " + tableName;
  if ( sqlWhereClause.length() > 0 )
  {
    sql += " where " + sqlWhereClause;
  }

#if WASTE_TIME
  sql = "select xmax(extent(" + geometryColumn + ")) as xmax,"
        "xmin(extent(" + geometryColumn + ")) as xmin,"
        "ymax(extent(" + geometryColumn + ")) as ymax," "ymin(extent(" + geometryColumn + ")) as ymin" " from " + tableName;
#endif

#ifdef QGISDEBUG
  qDebug( "+++++++++QgsPostgresExtentThread::run -  Getting extents using schema.table: " + sql.toUtf8() );
#endif


  QgsDebugMsg( "About to issue query." );

  PGresult *result = PQexec( connection, sql.toUtf8() );

  QgsDebugMsg( "Query completed." );



  std::string box3d = PQgetvalue( result, 0, 0 );
  std::string s;

  box3d = box3d.substr( box3d.find_first_of( "(" ) + 1 );
  box3d = box3d.substr( box3d.find_first_not_of( " " ) );
  s = box3d.substr( 0, box3d.find_first_of( " " ) );
  double minx = strtod( s.c_str(), NULL );

  box3d = box3d.substr( box3d.find_first_of( " " ) + 1 );
  s = box3d.substr( 0, box3d.find_first_of( " " ) );
  double miny = strtod( s.c_str(), NULL );

  box3d = box3d.substr( box3d.find_first_of( "," ) + 1 );
  box3d = box3d.substr( box3d.find_first_not_of( " " ) );
  s = box3d.substr( 0, box3d.find_first_of( " " ) );
  double maxx = strtod( s.c_str(), NULL );

  box3d = box3d.substr( box3d.find_first_of( " " ) + 1 );
  s = box3d.substr( 0, box3d.find_first_of( " " ) );
  double maxy = strtod( s.c_str(), NULL );

  layerExtent = new QgsRectangle( minx, miny, maxx, maxy );

  /*
    layerExtent.setXMaximum(maxx);
    layerExtent.setXMinimum(minx);
    layerExtent.setYMaximum(maxy);
    layerExtent.setYMinimum(miny);
  */


  QgsDebugMsg( QString( "Set extents to: %1, %2 %3, %4" ).arg( layerExtent->xMinimum() ).arg( layerExtent->yMinimum() ).arg( layerExtent->xMaximum() ).arg( layerExtent->yMaximum() ) );

  // clear query result
  PQclear( result );


  // Send some events (instead of a signal) as it is thread-safe

  // First we tell the object that invoked us that we have some new extents for her
  // Second we tell the application that the extents have changed, so that it
  // can go on and do any visual housekeeping (e.g. update the overview window)

  QgsDebugMsg( QString( "About to create and dispatch event %1 to callback" ).arg( QGis::ProviderExtentCalcEvent ) );

  QgsProviderExtentCalcEvent * e1 = new QgsProviderExtentCalcEvent( layerExtent );
  QApplication::postEvent(( QObject * )callbackObject, e1 );

//  QApplication::postEvent(qApp->mainWidget(), e1);

  QgsDebugMsg( QString( "Posted event %1 to callback" ).arg( QGis::ProviderExtentCalcEvent ) );


  QgsDebugMsg( "About to finish connection." );

  // ending the thread, clean up
  PQfinish( connection );

  QgsDebugMsg( "About to complete running." );


}

