/***************************************************************************
 *   Copyright (C) 2004 by Gary E. Sherman
 *   sherman at mrcc dot com
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef PLUGINGUI_H
#define PLUGINGUI_H

#include <qgslauncherpluginguibase.uic.h>
#include <memory>
class QTextEdit;
class QProcess;
/**
  \class QgsLauncherPluginGui
  \brief Launches external programs from QGIS.

  The GUI handles the starting of processes and display of output from 
  stdout and stderr

@author Gary Sherman
*/
class QgsLauncherPluginGui : public QgsLauncherPluginGuiBase
{
Q_OBJECT
public:
  //! Default constructor
    //QgsLauncherPluginGui();
    QgsLauncherPluginGui( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~QgsLauncherPluginGui();
public slots:
  //! Run the program specified in the command combo box
  void runProgram();
  //! Choose a program to run by browsing the file system
  void chooseProgram();
  //! Slot to read program output from stdout
  void readFromStdout();
  //! Slot to read program output from stderr
  void readFromStderr();
  //! Slot to delete the QProcess object after the program has exited
  void processFinished();
  //! Slot to save the previous command list and close the dialog
  void cleanUp();

private:
  //! QProcess object used to launch a program
  QProcess *proc;    
  //! Pointer to current text output widget
  QTextEdit *textOutput;
  //! Flag to indicate if this is the first run
  bool firstRun;
signals:
   void drawRasterLayer(QString);
   void drawVectorrLayer(QString,QString,QString);
};

#endif
