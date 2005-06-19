/***************************************************************************
                          qgscontexthelp.h
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
#ifndef QGSCONTEXTHELP_H
#define QGSCONTEXTHELP_H
#include "qgscontexthelpbase.uic.h"
class QString;
class QgsContextHelp : public QgsContextHelpBase{
Q_OBJECT
  public:
  QgsContextHelp(QString &contextId, QWidget *parent, const char *name);
  ~QgsContextHelp();
  public slots:
    void linkClicked ( const QString &link );
};
#endif //QGSCONTEXTHELP_H
