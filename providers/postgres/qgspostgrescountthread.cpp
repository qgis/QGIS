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
#include <qevent.h>
#include <qapplication.h>

#include "../../src/qgis.h"
#include "../../src/qgsrect.h"
#include "../../src/qgsmapcanvas.h"

#include "../../src/qgsprovidercountcalcevent.h"

#include "qgspostgrescountthread.h"


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
  connInfo = s;
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
//  QString connInfo;
  
  std::cout << "QgsPostgresCountThread: Started running." << std::endl;

  // Open another connection to the database
  PGconn *connection = PQconnectdb((const char *) connInfo);

  // get the extents

  QString sql = "select count(*) from " + tableName;
  if(sqlWhereClause.length() > 0)
  {
    sql += " where " + sqlWhereClause;
  }


  std::cout << "QgsPostgresCountThread: About to issue query." << std::endl;

  PGresult *result = PQexec(connection, (const char *) sql);
  
  std::cout << "QgsPostgresCountThread: Query completed." << std::endl;
  
    
  numberFeatures = QString(PQgetvalue(result, 0, 0)).toLong();
  PQclear(result);

#ifdef QGISDEBUG
      std::cerr << "QgsPostgresCountThread: Exact Number of features: " << 
        numberFeatures << std::endl;
#endif


  // Send some events (instead of a signal) as it is thread-safe
  
  // First we tell the object that invoked us that we have some new extents for her
  // Second we tell the application that the extents have changed, so that it
  // can go on and do any visual housekeeping (e.g. update the overview window)

  std::cout << "QgsPostgresCountThread: About to create and dispatch event " << QGis::ProviderCountCalcEvent << " to callback" << std::endl;
  
  QgsProviderCountCalcEvent* e1 = new QgsProviderCountCalcEvent(numberFeatures);
  QApplication::postEvent( (QObject *)callbackObject, e1);
  
//  QApplication::postEvent(qApp->mainWidget(), e1);
  
  std::cout << "QgsPostgresCountThread: Posted event " << QGis::ProviderCountCalcEvent << " to callback" << std::endl;

   
  std::cout << "QgsPostgresCountThread: About to finish connection." << std::endl;

  // ending the thread, clean up
  PQfinish(connection);
  
  std::cout << "QgsPostgresCountThread: About to complete running." << std::endl;
  
    
}

