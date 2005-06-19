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
class sqlite3;
/*!
 * \class QgsContextHelp
 * \brief Provides a context based help browser for a dialog.
 *
 * The help text is stored in SQLite and accessed by a context identifier
 * unique to each dialog.
 */
class QgsContextHelp : public QgsContextHelpBase{
Q_OBJECT
  public:
  //! Constructor
  QgsContextHelp(QString &contextId, QWidget *parent, const char *name);
  //! Destructor
  ~QgsContextHelp();
  public slots:
    //! Slot called when a link is clicked
    void linkClicked ( const QString &link );
  private:
  int connectDb(QString &);
  sqlite3 *db;
};
#endif //QGSCONTEXTHELP_H
