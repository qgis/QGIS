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
#ifdef Q_OS_MACX
#define QGSCONTEXTHELP_REUSE 1
#endif
/*!
 * \class QgsContextHelp
 * \brief Provides a context based help browser for a dialog.
 *
 * The help text is stored in SQLite and accessed by a context identifier
 * unique to each dialog. This is a singleton class which invokes the help
 * viewer using QProcess and ensures that only one viewer is open.
 * The viewer will be terminated if open when the main application quits.
 *
 * If the compile-time flag QGSCONTEXTHELP_REUSE is defined, the help viewer
 * will be reused if it is still open. If this flag is not set, the viewer
 * process will be terminated if open and restarted; this makes it the top
 * window for window managers such as Linux/GNOME which will make a window
 * active but not bring it to the top if raised programatically.
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

  QProcess *start(int contextId);
  void showContext(int contextId);

  static QgsContextHelp *gContextHelp; // Singleton instance
  QProcess *mProcess;
#ifdef QGSCONTEXTHELP_REUSE
  // Communications socket when reusing existing process
  QSocket *mSocket;
#else
  // Replacement process when terminating and restarting
  QProcess *mNextProcess;
#endif
};
#endif //QGSCONTEXTHELP_H
