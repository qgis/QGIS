/***************************************************************************
                          qgsrunprocess.h 

 A class that runs an external program

                             -------------------
    begin                : Jan 2005
    copyright            : (C) 2005 by Gavin Macaulay
    email                : gavin at macaulay dot co dot nz
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

#ifndef QGSRUNPROCESS_H
#define QGSRUNPROCESS_H

#include <qobject.h>

class QProcess;
class QgsMessageViewer;
class QStringList;

/* A class than executes an external program/script, etc and
 * optionally captures the standard output and error from the
 * process and displays them in a dialog box.
 */
class QgsRunProcess: public QObject
{
  Q_OBJECT;

 public:
  // This class deletes itself, so to ensure that it is only created
  // using new, the Named Consturctor Idiom is used, and one needs to
  // use the create() static function to get an instance of this class.

  // The arg parameter is passed to QProcess::setArguments(), so it
  // needs to be appropriate for that function. If capture is true,
  // the standard output and error from the process will be displayed
  // in a dialog box.
  static QgsRunProcess* create(const QStringList& args, bool capture)
    { return new QgsRunProcess(args, capture); }

 public slots:
  void stdoutAvailable();
  void stderrAvailable();
  void processExit();
  void dialogGone();

 private:
  QgsRunProcess(const QStringList& args, bool capture);
  ~QgsRunProcess();

  // Deletes the instance of the class
  void die();

  QProcess* mProcess;
  QgsMessageViewer* mLogViewer;
};

#endif
