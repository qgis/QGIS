/***************************************************************************
                          qgscontexthelp.h
               Display context help for a dialog by invoking the
                                 QgsHelpViewer
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
#include <qobject.h>
class QProcess;
class QSocket;
/*!
 * \class QgsContextHelp
 * \brief Provides a context based help browser for a dialog.
 *
 * The help text is stored in SQLite and accessed by a context identifier
 * unique to each dialog. This is a singleton class which invokes the help
 * viewer using QProcess. The viewer will be reused as long as it is open.
 * The viewer will be terminated if open when the main application quits.
 */
class QgsContextHelp : public QObject {
  Q_OBJECT
public:
  static void run(int contextId);

private slots:
  void readPort();
  void processExited();

private:
  //! Constructor
  QgsContextHelp(int contextId);
  //! Destructor
  ~QgsContextHelp();

  void showContext(int contextId);

  static QgsContextHelp *gContextHelp; // Singleton instance
  QProcess *mProcess;
  QSocket *mSocket;
};
#endif //QGSCONTEXTHELP_H
