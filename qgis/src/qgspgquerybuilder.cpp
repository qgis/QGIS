/***************************************************************************
    qgspgquerybuilder.cpp - PostgreSQL Query Builder
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
#include "qgspgquerybuilder.h"

QgsPgQueryBuilder::QgsPgQueryBuilder(QWidget *parent, const char *name)
  : QgsPgQueryBuilderBase(parent, name)
{
}

QgsPgQueryBuilder::~QgsPgQueryBuilder()
{
}

void QgsPgQueryBuilder::populateFields()
{
}

void QgsPgQueryBuilder::getSampleValues()
{
}

void QgsPgQueryBuilder::getAllValues()
{
}

void QgsPgQueryBuilder::testSql()
{
}

void QgsPgQueryBuilder::setConnection(PGconn *con)
{
  mPgConnection = con;
}


