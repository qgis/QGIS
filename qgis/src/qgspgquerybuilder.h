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

extern "C"
{
#include <libpq-fe.h>
}

#ifdef WIN32
#include "qgspgquerybuilderbase.h"
#else
#include "qgspgquerybuilderbase.uic.h"
#endif

class QgsPgQueryBuilder : public QgsPgQueryBuilderBase
{ 
  Q_OBJECT
  public:
    QgsPgQueryBuilder(QWidget *parent = 0, const char *name=0);
    ~QgsPgQueryBuilder();
    void setConnection(PGconn *con);
  private:
    void populateFields();
    void getSampleValues();
    void getAllValues();
    void testSql();

    // private members
   PGconn *mPgConnection;

};
#endif //QGSPGQUERYBUILDER_H
