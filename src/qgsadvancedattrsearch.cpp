/***************************************************************************
                        qgsadvancedattrsearch.cpp
                dialog for entering advanced search strings
                          --------------------
    begin                : 2005-09-08
    copyright            : (C) 2005 by Martin Dobias
    email                : won.der at centrum.sk
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

#include "qgsadvancedattrsearch.h"
#include <qmessagebox.h>

QgsAdvancedAttrSearch::QgsAdvancedAttrSearch(QWidget *parent, const char *name)
    :QgsAdvancedAttrSearchBase(parent, name)
{
}

void QgsAdvancedAttrSearch::showHelp()
{
  QMessageBox::information(this, tr("Help on using search strings"), tr(
"Search strings are basically a subset of SQL language.\n\
Short overview of syntax:\n\
- atoms:\n\
  + column reference\n\
  + text (enclosed in single quotes)\n\
  + numbers (optionally in single quotes)\n\
- operators:\n\
  + comparison: <, >, <=, >=, =, != (resp. <>)\n\
  + regular expressions: ~, LIKE\n\
  + logical: AND, OR, NOT\n\
  + arithmetic: +, -, *, /\n\
- parentheses ()\n\
- example: name = 'John' OR (age >= 10 AND surname ~ 'ow')"));
  
}

