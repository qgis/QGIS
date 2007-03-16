/***************************************************************************
  qgslauncherplugingui.cpp - GUI for the launcher plugin 
  --------------------------------------
  Date                 : 09-Apr-2004
  Copyright            : (C) 2004 by Gary E.Sherman
  Email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id: qgslauncherplugingui.cpp 1242 2004-04-23 06:51:34Z gsherman $ */

#include "qgslauncherplugingui.h"
#include <iostream>
//qt includes
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <QProcess>
#include <QTextEdit>
#include <qtabwidget.h>
#include <QSettings>

QgsLauncherPluginGui::QgsLauncherPluginGui( QWidget* parent , 
    const char* name , bool modal , Qt::WFlags fl  )
    : QDialog( parent, name, modal, fl )
{
  setupUi(this);
  // populate the combobox from the settings file
  QSettings settings;
  int count = settings.readNumEntry("launcher_plugin/command_count"); 
  QString commandNum;
  for(int i=0; i < count; i++)
  {
//    std::cerr << "Reading key /qgis/launcher_plugin/" << commandNum.setNum(i) << std::endl; 
    cmbCommands->insertItem(settings.readEntry("launcher_plugin/command" 
          + commandNum.setNum(i)));
  }
  // Set flag to indicate first run is pending
  firstRun = true;
}  
QgsLauncherPluginGui::~QgsLauncherPluginGui()
{
  // delete the QProcess object
  //delete proc;
}

void QgsLauncherPluginGui::chooseProgram()
{
  QString program = QFileDialog::getOpenFileName();
  if(program != QString::null)
  {
    cmbCommands->setCurrentText(program);
  }
} 
void QgsLauncherPluginGui::runProgram()
{
  QStringList args = QStringList::split(" ", cmbCommands->currentText());
  if(args.size() != 0)
  {
    QStringList arguments;
    txtOutput->setText("");
    proc = new QProcess(this);
    connect( proc, SIGNAL(readyReadStandardOutput()),
        this, SLOT(readFromStdout()) );
    connect( proc, SIGNAL(readyReadStandardError()),
        this, SLOT(readFromStderr()) );
    connect( proc, SIGNAL(finished( int, QProcess::ExitStatus )),
        this, SLOT(processFinished(int, QProcess::ExitStatus)) );
    connect( proc, SIGNAL(error(QProcess::ProcessError)),
          this, SLOT(processError(QProcess::ProcessError)));
    for(int i=1; i < args.size(); i++)
    {
      arguments << args[i];
    }
    // If the command fails to start, throw up a critical error msg
    programName = args[0];

    
      // add the program to the combo list
      if(cmbCommands->findText(cmbCommands->currentText()) == -1)
      {
        cmbCommands->insertItem(cmbCommands->currentText(), 0);
      }
      // If this is first time around, remove the default tab
      if(firstRun)
      {
        tabOutput->removePage(tabOutput->currentPage());
        firstRun = false;
      }
      // create the tab and textedit box for this process
      QTextEdit *te = new QTextEdit();
      // set textedit properties
      te->setReadOnly(true);
      te->setFamily("Bitstream Vera Sans Mono");
      te->setPointSize(11);
      te->setWordWrapMode(QTextOption::NoWrap);
      // Create the tab page containing the textedit box. The
      // tab title is the first argument (program name)
      tabOutput->insertTab(te, args[0], 0);
      // set the current output to be the new tab
      tabOutput->setCurrentPage(0); 
      // point the output stream to the new tab
      textOutput = te;
      // Show the command in the textedit box for future reference
      textOutput->append("COMMAND: " + cmbCommands->currentText());
      proc->start(programName, arguments);
    

  }else
  {
    QMessageBox::warning(this, 
        "Nothing to execute","Please specify the name of a program or script to execute");
  }

} 
void QgsLauncherPluginGui::readFromStdout()
{
  //if(proc->readyReadStandardOutput())
  {
  //  QString line;
   // while((line = proc->readLineStdout()) != QString::null)
    {
      textOutput->append(proc->readAllStandardOutput());
    }
  }
}
void QgsLauncherPluginGui::readFromStderr()
{
  //if(proc->readyReadStandardError())
  {
//    QString line;
 //   while((line = proc->readLineStderr()) != QString::null)
    {
      textOutput->append(proc->readAllStandardError());
    }
  }
}
void QgsLauncherPluginGui::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  delete proc;
  // scroll the output to the top
//  textOutput->setCursorPosition(0, 0);
}

void QgsLauncherPluginGui::processError(QProcess::ProcessError err)
{
    //  QMessageBox::critical(this, "Unable to Launch", "Unable to start " 
    //     + programName + ".\nCheck to make sure it is executable and you have permissions.");
   textOutput->append( "Unable to start " 
         + programName + ".\nCheck to make sure it is executable and you have permissions.");


}

void QgsLauncherPluginGui::cleanUp()
{
  // Write out the command list to the settings file and then exit
  QSettings settings;

  settings.writeEntry("launcher_plugin/command_count", 
      cmbCommands->count());
  // write the commands in the combo box to the settings file (~/.qt/qgisrc)
  for(int i = 0; i < cmbCommands->count(); i++)
  {
    QString commandNumber;
    settings.writeEntry("launcher_plugin/command" 
        + commandNumber.setNum(i), cmbCommands->text(i));
  }
  reject();
}

void QgsLauncherPluginGui::on_btnClearCommandList_clicked()
{
  cmbCommands->clear();
}
