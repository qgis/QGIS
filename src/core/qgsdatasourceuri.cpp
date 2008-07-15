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
#include "qgslogger.h"

#include <QStringList>
#include <QRegExp>

QgsDataSourceURI::QgsDataSourceURI()
{
  // do nothing
}

QgsDataSourceURI::QgsDataSourceURI(QString uri)
{
  int i = 0;
  while( i<uri.length() )
  {
    skipBlanks(uri, i);

    if( uri[i] == '=' )
    {
      QgsDebugMsg("parameter name expected before =");
      i++;
      continue;
    }

    int start = i;

    while( i<uri.length() && uri[i]!='=' && !uri[i].isSpace() )
      i++;

    QString pname = uri.mid(start, i-start);

    skipBlanks(uri, i);

    if( uri[i]!='=' ) {
      QgsDebugMsg("= expected after parameter name");
      return;
    }

    i++;

    if( pname=="sql" ) {
      // rest of line is a sql where clause
      skipBlanks(uri, i);
      mSql = uri.mid(i);
      break;
    } else {
      QString pval = getValue(uri, i);

      if( pname=="table" ) {
        if( uri[i] == '.' ) {
          i++;

          mSchema = pval;
          mTable = getValue(uri, i);
        } else {
          mSchema = "";
          mTable = pval;
        }

        if( uri[i] == '(' ) {
          i++;

          int start = i;
          QString col;
          while( i<uri.length() && uri[i]!=')')
            i++;

          if( i==uri.length() ) {
            QgsDebugMsg("closing parenthesis missing");
          }

          mGeometryColumn = uri.mid(start, i-start);

          i++;
        }
      } else if( pname=="service" ) {
        QgsDebugMsg("service keyword ignored");
      } else if(pname=="user") {
        mUsername = pval;
      } else if(pname=="password") {
        mPassword = pval;
      } else if(pname=="connect_timeout") {
        QgsDebugMsg("connection timeout ignored");
      } else if(pname=="dbname") {
        mDatabase = pval;
      } else if(pname=="host") {
        mHost = pval;
      } else if(pname=="hostaddr") {
        QgsDebugMsg("database host ip address ignored");
      } else if(pname=="port") {
        mPort = pval;
      } else if(pname=="tty") {
        QgsDebugMsg("backend debug tty ignored");
      } else if(pname=="options") {
        QgsDebugMsg("backend debug options ignored");
      } else if(pname=="sslmode") {
        QgsDebugMsg("sslmode ignored");
      } else if(pname=="krbsrvname") {
        QgsDebugMsg("kerberos server name ignored");
      } else if(pname=="gsslib") {
        QgsDebugMsg("gsslib ignored");
      } else {
        QgsDebugMsg( "invalid connection option \"" + pname + "\" ignored");
      }
    }
  }
}

QString QgsDataSourceURI::username() const
{
  return mUsername;
}

QString QgsDataSourceURI::schema() const
{
  return mSchema;
}

QString QgsDataSourceURI::table() const
{
  return mTable;
}

QString QgsDataSourceURI::sql() const
{
  return mSql;
}

QString QgsDataSourceURI::geometryColumn() const
{
  return mGeometryColumn;
}

void QgsDataSourceURI::setSql(QString sql)
{
  mSql = sql;
}

void QgsDataSourceURI::clearSchema()
{
  mSchema = "";
}

void QgsDataSourceURI::skipBlanks(const QString &uri, int &i)
{
	// skip space before value
	while( i<uri.length() && uri[i].isSpace() )
		i++;
}

QString QgsDataSourceURI::getValue(const QString &uri, int &i)
{
  skipBlanks(uri, i);

  // Get the parameter value
  QString pval;
  if( uri[i] == '\'' || uri[i]=='"' ) {
    QChar delim = uri[i];

    i++;

    // value is quoted
    for (;;)
    {
      if( i==uri.length() ) {
        QgsDebugMsg("unterminated quoted string in connection info string");
        return pval;
      }

      if( uri[i] == '\\') {
        i++;
        if( i==uri.length() )
          continue;
      } else if(uri[i]==delim) {
        i++;
        break;
      }

      pval += uri[i++];
    }
  } else {
    // value is not quoted
    while( i<uri.length() ) {
      if( uri[i].isSpace() ) {
        // end of value
        break;
      }

      if( uri[i] == '\\' ) {
        i++;
        if( i==uri.length() )
          break;
      }

      pval += uri[i++];
    }
  }

  skipBlanks(uri, i);

  return pval;
}

QString QgsDataSourceURI::connInfo() const
{
  QString connInfo = "dbname='"+mDatabase+"'";

  if( mHost != "" )
  {
    connInfo += " host=" + mHost;
    if( mPort!="" )
      connInfo += " port=" + mPort;
    connInfo += " sslmode=prefer";
  }

  if( mUsername != "" )
  {
    connInfo += " user='" + mUsername + "'";	//needs to be escaped

    if( mPassword != "" )
    {
      QString p = mPassword; 
      p.replace('\\', "\\\\");
      p.replace('\'', "\\'");
      connInfo += " password='"+p+"'";
    }
  }

  return connInfo;
}

QString QgsDataSourceURI::uri() const
{ 
  return connInfo()
       + QString(" table=%1 (%2) sql=%3")
                .arg( quotedTablename() )
                .arg( mGeometryColumn )
                .arg( mSql );
}

QString QgsDataSourceURI::quotedTablename() const
{
  if(mSchema!="")
    return QString("\"%1\".\"%2\"").arg(mSchema).arg(mTable);
  else
    return QString("\"%1\"").arg(mTable);
}

void QgsDataSourceURI::setConnection(const QString &host,
                                     const QString &port,
                                     const QString &database,
                                     const QString &username,
                                     const QString &password)
{
  mHost = host;
  mDatabase = database;
  mPort = port;
  mUsername = username;
  mPassword = password;
}
  
void QgsDataSourceURI::setDataSource(const QString &schema,
                                     const QString &table,
                                     const QString &geometryColumn,
                                     const QString &sql)
{
  mSchema = schema;
  mTable = table;
  mGeometryColumn = geometryColumn;
  mSql = sql;
}
