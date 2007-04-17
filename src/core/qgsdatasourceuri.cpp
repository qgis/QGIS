/***************************************************************************
      qgsdatasourceuri.h  -  Structure to contain the component parts
                             of a data source URI
                             -------------------
    begin                : Dec 5, 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id: qgsdatasourceuri.h 5839 2006-09-19 18:04:21Z wonder $ */

#include "qgsdatasourceuri.h"

#include <QStringList>
#include <QRegExp>

QgsDataSourceURI::QgsDataSourceURI()
{
  // do nothing
}

QgsDataSourceURI::QgsDataSourceURI(QString uri)
{
  // URI looks like this:
  //  host=192.168.1.5 dbname=test port=5342 user=gsherman password=xxx table=tablename
  // (optionally at the end there might be: sql=..... )
  
  // TODO: improve parsing
  
  // A little bit of backwards compability. At r6193 the schema/table 
  // names became fully quoted, but with the problem that earlier 
  // project files then didn't load. This bit of code puts in the 
  // quotes that are now required. 
  QString uriModified = uri; 
  int start = uriModified.indexOf("table=\""); 
  if (start == -1) 
    { 
      // Need to put in some "'s 
      start = uriModified.indexOf("table="); 
      uriModified.insert(start+6, '"'); 
      int start_dot = uriModified.indexOf('.', start+7); 
      if (start_dot != -1) 
        { 
          uriModified.insert(start_dot, '"'); 
          uriModified.insert(start_dot+2, '"'); 
        } 
      // and one at the end 
      int end = uriModified.indexOf(' ',start); 
      if (end != -1) 
        uriModified.insert(end, '"'); 
    }  
  
  // Strip the table and sql statement name off and store them
  int sqlStart = uriModified.find(" sql");
  int tableStart = uriModified.find("table=");
  
  // set table name
  table = uriModified.mid(tableStart + 6, sqlStart - tableStart -6);

  // set sql where clause
  if(sqlStart > -1)
  { 
    sql = uriModified.mid(sqlStart + 5);
  }
  else
  {
    sql = QString::null;
  }
  
  // calculate the schema if specified

  // Pick up some stuff from the uriModified: basically two bits of text 
  // inside double quote marks, separated by a . 
  QRegExp reg("\"(.+)\"\\.\"(.+)\".+\\((.+)\\)");
  reg.indexIn(table); 
  QStringList stuff = reg.capturedTexts(); 

  schema = stuff[1]; 
  table = stuff[2]; 
  geometryColumn = stuff[3]; 

  // set connection info
  connInfo = uriModified.left(uriModified.find("table="));
  
  // parse the connection info
  QStringList conParts = QStringList::split(" ", connInfo);
  QStringList parm = QStringList::split("=", conParts[0]);
  if(parm.size() == 2)
  {
    host = parm[1];
  }
  parm = QStringList::split("=", conParts[1]);
  if(parm.size() == 2)
  {
    database = parm[1];
  }
  parm = QStringList::split("=", conParts[2]);
  if(parm.size() == 2)
  {
    port = parm[1];
  }

  parm = QStringList::split("=", conParts[3]);
  if(parm.size() == 2)
  {
    username = parm[1];
  }
  
  // The password can have '=' and ' ' characters in it, so we can't 
  // use the split on '=' and ' ' technique - use indexOf() 
  // instead. 
  QString key="password='"; 
  int i = connInfo.indexOf(key); 
  if (i != -1) 
  { 
    QString pass = connInfo.mid(i+key.length()); 
    // Now walk through the string till we find a ' character, but 
    // need to allow for an escaped ' character (which will be the 
    // \' character pair). 
    int n = 0; 
    bool escaped = false; 
    while (n < pass.length() && (pass[n] != '\'' || escaped)) 
    { 
      if (pass[n] == '\\') 
        escaped = true; 
      else 
        escaped = false; 
      n++; 
    } 
    // The -1 is to remove the trailing ' character 
    password = pass.left(n); 
  } 
}


QString QgsDataSourceURI::text() const
{
  return QString("host=" + host + 
      " dbname=" + database + 
      " port=" + port + 
      " user=" + username + 
      " password='" + password + 
      "' table=" + schema + '.' + table + 
      " (" + geometryColumn + ")" +
      " sql=" + sql);
}

void QgsDataSourceURI::setConnection(const QString& aHost,
                                     const QString& aPort,
                                     const QString& aDatabase,
                                     const QString& aUsername,
                                     const QString& aPassword)
{
  host = aHost;
  database = aDatabase;
  port = aPort;
  username = aUsername;
  password = aPassword;
}
  
void QgsDataSourceURI::setDataSource(const QString& aSchema,
                                     const QString& aTable,
                                     const QString& aGeometryColumn,
                                     const QString& aSql)
{
  schema = aSchema;
  table = aTable;
  geometryColumn = aGeometryColumn;
  sql = aSql;
}
