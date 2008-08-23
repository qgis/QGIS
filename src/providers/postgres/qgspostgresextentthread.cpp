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
#include "qgsrect.h"


#include "qgspostgresextentthread.h"
#include "qgsproviderextentcalcevent.h"


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
  connInfo = s;
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
//  QString connInfo;
  
  std::cout << "QgsPostgresExtentThread: Started running." << std::endl;

  // Open another connection to the database
  PGconn *connection = PQconnectdb(connInfo.toUtf8());

  // get the extents

  QString sql = "select extent(" + geometryColumn + ") from " + tableName;
  if(sqlWhereClause.length() > 0)
  {
    sql += " where " + sqlWhereClause;
  }

#if WASTE_TIME
  sql = "select xmax(extent(" + geometryColumn + ")) as xmax,"
    "xmin(extent(" + geometryColumn + ")) as xmin,"
    "ymax(extent(" + geometryColumn + ")) as ymax," "ymin(extent(" + geometryColumn + ")) as ymin" " from " + tableName;
#endif

#ifdef QGISDEBUG 
  qDebug("+++++++++QgsPostgresExtentThread::run -  Getting extents using schema.table: " + sql.toUtf8());
#endif


  std::cout << "QgsPostgresExtentThread: About to issue query." << std::endl;

  PGresult *result = PQexec(connection, sql.toUtf8());
  
  std::cout << "QgsPostgresExtentThread: Query completed." << std::endl;

  
    
  std::string box3d = PQgetvalue(result, 0, 0);
  std::string s;

  box3d = box3d.substr(box3d.find_first_of("(")+1);
  box3d = box3d.substr(box3d.find_first_not_of(" "));
  s = box3d.substr(0, box3d.find_first_of(" "));
  double minx = strtod(s.c_str(), NULL);

  box3d = box3d.substr(box3d.find_first_of(" ")+1);
  s = box3d.substr(0, box3d.find_first_of(" "));
  double miny = strtod(s.c_str(), NULL);

  box3d = box3d.substr(box3d.find_first_of(",")+1);
  box3d = box3d.substr(box3d.find_first_not_of(" "));
  s = box3d.substr(0, box3d.find_first_of(" "));
  double maxx = strtod(s.c_str(), NULL);

  box3d = box3d.substr(box3d.find_first_of(" ")+1);
  s = box3d.substr(0, box3d.find_first_of(" "));
  double maxy = strtod(s.c_str(), NULL);

  layerExtent = new QgsRect(minx, miny, maxx, maxy);
  
/*
  layerExtent.setXMaximum(maxx);
  layerExtent.setXMinimum(minx);
  layerExtent.setYmax(maxy);
  layerExtent.setYmin(miny);
*/
  

#ifdef QGISDEBUG
  std::cout << "QgsPostgresExtentThread: Set extents to: " 
        << layerExtent->xMin() << ", " << layerExtent->yMin() <<
    " " << layerExtent->xMax() << ", " << layerExtent->yMax() << std::endl;
#endif

  // clear query result
  PQclear(result);


  // Send some events (instead of a signal) as it is thread-safe
  
  // First we tell the object that invoked us that we have some new extents for her
  // Second we tell the application that the extents have changed, so that it
  // can go on and do any visual housekeeping (e.g. update the overview window)

  std::cout << "QgsPostgresExtentThread: About to create and dispatch event " << QGis::ProviderExtentCalcEvent << " to callback" << std::endl;
  
  QgsProviderExtentCalcEvent * e1 = new QgsProviderExtentCalcEvent ( layerExtent );
  QApplication::postEvent( (QObject *)callbackObject, e1);
  
//  QApplication::postEvent(qApp->mainWidget(), e1);
  
  std::cout << "QgsPostgresExtentThread: Posted event " << QGis::ProviderExtentCalcEvent << " to callback" << std::endl;

  
  std::cout << "QgsPostgresExtentThread: About to finish connection." << std::endl;

  // ending the thread, clean up
  PQfinish(connection);
  
  std::cout << "QgsPostgresExtentThread: About to complete running." << std::endl;
  
    
}

