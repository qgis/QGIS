/***************************************************************************
                          qgscontexthelp.cpp
                    Display context help for a dialog
                             -------------------
    begin                : 2005-06-19
    copyright            : (C) 2005 by Gary E.Sherman
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
/* $Id$ */
#include <qstring.h>
#include "qgscontexthelp.h"
QgsContextHelp::QgsContextHelp(QString &contextId, QWidget *parent, const char *name)
  : QgsContextHelpBase(parent, name)
{
  // get the context id from the database and display the document
}
QgsContextHelp::~QgsContextHelp()
{
}
void QgsContextHelp::linkClicked ( const QString &link )
{
}
