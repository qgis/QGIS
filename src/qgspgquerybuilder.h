/***************************************************************************
    qgspgquerybuilder.h - Subclassed PostgreSQL query builder 
     --------------------------------------
    Date                 : 2004-11-19
    Copyright            : (C) 2004 by Gary E.Sherman
    Email                : sherman at mrcc.com
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPGQUERYBUILDER_H
#define QGSPGQUERYBUILDER_H
#include <map>
#include <vector>
extern "C"
{
#include <libpq-fe.h>
}

#ifdef WIN32
#include "qgspgquerybuilderbase.h"
#else
#include "qgspgquerybuilderbase.uic.h"
#endif
class QgsField;
class QgsPgQueryBuilder : public QgsPgQueryBuilderBase
{ 
  Q_OBJECT
  public:
    //! Default constructor
    QgsPgQueryBuilder(QWidget *parent = 0, const char *name=0);
    //! Constructor which also takes the table name and PG connection
    // pointer
    QgsPgQueryBuilder(QString tableName, PGconn *con, 
        QWidget *parent = 0, const char *name=0);
    ~QgsPgQueryBuilder();
    void setConnection(PGconn *con);
  private:
    //! Populate the field list for the selected table
    void populateFields();
    //! Get sample distinct values for the selected field. The sample size
    // is limited to an arbitrary value
    void getSampleValues();
    //! Get all distinct values for the field
    void getAllValues();
    //! Test the constructed sql statement to see if the database likes it
    void testSql();

    // private members
    
    //! PostgreSQL conneciton
   PGconn *mPgConnection;
   //! Table name
   QString mTableName;
   //! Vector of QgsField objects
   std::vector<QgsField> mFields;
   //! Map that holds field information
   std::map<QString, QgsField> mFieldMap;

};
#endif //QGSPGQUERYBUILDER_H
