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
/*!
 * \class QgsPgQueryBuilder
 * \brief Query Builder for PostgreSQL layers.
 *
 * The query builder allows interactive creation of a SQL for limiting the
 * features displayed in a database layer.  The fields in the table are
 * displayed and sample values (or all values) can be viewed to aid in
 * constructing the query. A test function returns the number of features that
 * will be returned.
 *
 */
class QgsPgQueryBuilder : public QgsPgQueryBuilderBase { Q_OBJECT public:
  //! Default constructor - not very useful
  QgsPgQueryBuilder(QWidget *parent = 0, const char *name=0);

  /*! Constructor which also takes the table name and PG connection pointer
  * @param tableName Name of the table being queried
  * @param con PostgreSQL connection from the Add PostGIS Layer dialog
  * @param parent Parent widget
  * @param name Name of the widget
  */
  QgsPgQueryBuilder(QString tableName, PGconn *con, QWidget *parent = 0,
      const char *name=0); ~QgsPgQueryBuilder(); 
 
 /*!
  * Set the connection used the by query builder
  * @param con Active PostgreSQL connection
  */ 
  void setConnection(PGconn *con); 

  private:
  /*! 
   * Populate the field list for the selected table
   */ 
  void populateFields();

  /*! 
   * Get sample distinct values for the selected field. The sample size is
   * limited to an arbitrary value (currently set to 25). The values
   * are inserted into the values list box.
   */
  void getSampleValues();
  
  /*! 
   * Get all distinct values for the field. Values are inserted
   * into the value list box
   */
  void getAllValues();
  
  /*! Test the constructed sql statement to see if the database likes it.
   * The number of rows that would be returned is displayed in a message box.
   * The test uses a "select count(*) from ..." query to test the SQL 
   * statement.
   */
  void testSql();

  // private members

  //! PostgreSQL connection object
  PGconn *mPgConnection;
  //! Table name
  QString mTableName;
  //! Vector of QgsField objects
  std::vector<QgsField> mFields;
  //! Map that holds field information, keyed by field name
  std::map<QString, QgsField> mFieldMap;

};
#endif //QGSPGQUERYBUILDER_H
