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
/* $Id$ */
#include "qgspgutil.h"

QgsPgUtil *QgsPgUtil::mInstance = 0;
QgsPgUtil * QgsPgUtil::instance()
{
  if(mInstance == 0)
  {
    mInstance = new QgsPgUtil();
  }
  return mInstance;
}
QgsPgUtil::QgsPgUtil()
{
  // load the reserved word map
  initReservedWords();
}
QgsPgUtil::~QgsPgUtil()
{
}
bool QgsPgUtil::isReserved(QString word)
{
  // uppercase the word before testing it since all our reserved words are
  // stored in uppercase
  
  QStringList::iterator it = mReservedWords.find(word.upper());
  return (it != mReservedWords.end());
}
void QgsPgUtil::setConnection(PGconn *con)
{
  mPgConnection = con;
}
PGconn *QgsPgUtil::connection()
{
  return mPgConnection;
}
void QgsPgUtil::initReservedWords()
{
  // create the reserved word list by loading
  // the words into a QStringList. We code them here
  // for now rather than deal with the complexities
  // of finding and loading from a text file
  // in the install path
  mReservedWords << "ALL"
    << "ANALYSE"
    << "ANALYZE"
    << "AND"
    << "ANY"
    << "ARRAY"
    << "AS"
    << "ASC"
    << "AUTHORIZATION"
    << "BETWEEN"
    << "BINARY"
    << "BOTH"
    << "CASE"
    << "CAST"
    << "CHECK"
    << "COLLATE"
    << "COLUMN"
    << "CONSTRAINT"
    << "CREATE"
    << "CROSS"
    << "CURRENT_DATE"
    << "CURRENT_TIME"
    << "CURRENT_TIMESTAMP"
    << "CURRENT_USER"
    << "DEFAULT"
    << "DEFERRABLE"
    << "DESC"
    << "DISTINCT"
    << "DO"
    << "ELSE"
    << "END"
    << "EXCEPT"
    << "FALSE"
    << "FOR"
    << "FOREIGN"
    << "FREEZE"
    << "FROM"
    << "FULL"
    << "GRANT"
    << "GROUP"
    << "HAVING"
    << "ILIKE"
    << "IN"
    << "INITIALLY"
    << "INNER"
    << "INTERSECT"
    << "INTO"
    << "IS"
    << "ISNULL"
    << "JOIN"
    << "LEADING"
    << "LEFT"
    << "LIKE"
    << "LIMIT"
    << "LOCALTIME"
    << "LOCALTIMESTAMP"
    << "NATURAL"
    << "NEW"
    << "NOT"
    << "NOTNULL"
    << "NULL"
    << "OFF"
    << "OFFSET"
    << "OLD"
    << "ON"
    << "ONLY"
    << "OR"
    << "ORDER"
    << "OUTER"
    << "OVERLAPS"
    << "PLACING"
    << "PRIMARY"
    << "REFERENCES"
    << "RIGHT"
    << "SELECT"
    << "SESSION_USER"
    << "SIMILAR"
    << "SOME"
    << "TABLE"
    << "THEN"
    << "TO"
    << "TRAILING"
    << "TRUE"
    << "UNION"
    << "UNIQUE"
    << "USER"
    << "USING"
    << "VERBOSE"
    << "WHEN"
    << "WHERE";
}
