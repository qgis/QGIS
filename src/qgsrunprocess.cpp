/***************************************************************************
                          qgsrunprocess.cpp

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

#include "qgsrunprocess.h"

#ifdef WIN32
#include "qgsmessageviewer.h"
#else
#include "qgsmessageviewer.uic.h"
#endif

#include <qstring.h>
#include <qmessagebox.h>
#include <qprocess.h>
#include <qtextedit.h>

QgsRunProcess::QgsRunProcess(const QStringList& args,
			     bool capture) : mProcess(NULL), mLogViewer(NULL)
{
  // Make up a string from the command and arguments that we'll use
  // for display purposes
  QString whole_cmd;
  for (int i = 0; i < args.count(); ++i)
    whole_cmd += "[" + args[i] + "] ";
  qDebug("Running command: %s\n", whole_cmd.ascii());

  mProcess = new QProcess;
  mProcess->setArguments(args);

  if (capture)
  {
    connect(mProcess, SIGNAL(readyReadStdout()), this, SLOT(stdoutAvailable()));
    connect(mProcess, SIGNAL(readyReadStderr()), this, SLOT(stderrAvailable()));
    // We only care if the process has finished if we are capturing
    // the output from the process, hence this connect() call is
    // inside the capture if() statement.
    connect(mProcess, SIGNAL(processExited()), this, SLOT(processExit()));
  }

  if (!mProcess->start())
  {
    QMessageBox::critical(0, tr("Unable to run command"), 
			  tr("Unable to run the command") + "\n" + whole_cmd +
			  "\n", QMessageBox::Ok, QMessageBox::NoButton);
    // Didn't work, so no need to hang around
    die();
  }
  else if (capture)
  {
    // Create a dialog box to display the output. Use the
    // QgsMessageViewer dialog, but tweak its behaviour to suit our
    // needs. It will delete itself when the dialog box is closed.
    mLogViewer = new QgsMessageViewer(0, "", false, Qt::WDestructiveClose);
    mLogViewer->txtMessage->setTextFormat(Qt::LogText);
    mLogViewer->setCaption(whole_cmd);
    mLogViewer->txtMessage->append( "<b>" + tr("Starting") + " " + whole_cmd + "...</b>" );
    mLogViewer->show();
    // Be told when the dialog box is closed (it gets destroyed when
    // closed because of the Qt flag used when it was created above).
    connect(mLogViewer, SIGNAL(destroyed()), this, SLOT(dialogGone()));
  }
  else
    // We're not capturing the output from the process, so we don't
    // need to exist anymore.
    die();
}

QgsRunProcess::~QgsRunProcess() 
{ 
  delete mProcess; 
}

void QgsRunProcess::die()
{
  delete this;
}

void QgsRunProcess::stdoutAvailable()
{
  // Add the new output to the dialog box
  if (mProcess->canReadLineStdout())
  {
    QString line;
    while ((line = mProcess->readLineStdout()) != QString::null)
      mLogViewer->txtMessage->append(line);
  }
}

void QgsRunProcess::stderrAvailable()
{
  // Add the new output to the dialog box, but colour it red
  if (mProcess->canReadLineStderr())
  {
    QString line;
    mLogViewer->txtMessage->append("<font color=red>");
    while ((line = mProcess->readLineStderr()) != QString::null)
      mLogViewer->txtMessage->append(line);
    mLogViewer->txtMessage->append("</font>");
  }
}

void QgsRunProcess::processExit()
{
  // Because we catch the dialog box going (the dialogGone()
  // function), and delete this instance, control will only pass to
  // this function if the dialog box still exists when the process
  // exits, so it's always safe to use the pointer to the dialog box
  // (unless it was never created in the first case, which is what the
  // test against 0 is for).

  if (mLogViewer != 0)
    mLogViewer->txtMessage->append( "<b>" + tr("Done") + "</b>" );

  // Since the dialog box takes care of deleting itself, and the
  // process has gone, there's no need for this instance to stay
  // around, so we disappear too.
  die();
}

void QgsRunProcess::dialogGone()
{
  // The dialog has gone, so the user is no longer interested in the
  // output from the process. Since the process will run happily
  // without the QProcess object, this instance and its data can then
  // go too.
  die();
}
