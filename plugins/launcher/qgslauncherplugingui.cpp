/***************************************************************************
  qgslauncherplugingui.cpp - GUI for the launcher plugin 
  --------------------------------------
  Date                 : 09-Apr-2004
  Copyright            : (C) 2004 by Gary E.Sherman
  Email                : sherman at mrcc.com
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgslauncherplugingui.h"
#include <iostream>
//qt includes
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qprocess.h>
#include <qtextedit.h>
#include <qtabwidget.h>
#include <qsettings.h>

QgsLauncherPluginGui::QgsLauncherPluginGui( QWidget* parent , 
    const char* name , bool modal , WFlags fl  )
    : QgsLauncherPluginGuiBase( parent, name, modal, fl )
{
  // populate the combobox from the settings file
  QSettings settings;
  int count = settings.readNumEntry("/qgis/launcher_plugin/command_count"); 
  QString commandNum;
  for(int i=0; i < count; i++)
  {
    std::cerr << "Reading key " 
      << "/qgis/launcher_plugin/" << commandNum.setNum(i) << std::endl; 
    cmbCommands->insertItem(settings.readEntry("/qgis/launcher_plugin/command" 
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
    txtOutput->setText("");
    proc = new QProcess(this);
    connect( proc, SIGNAL(readyReadStdout()),
        this, SLOT(readFromStdout()) );
    connect( proc, SIGNAL(readyReadStderr()),
        this, SLOT(readFromStderr()) );
    connect( proc, SIGNAL(processExited()),
        this, SLOT(processFinished()) );
    for(int i=0; i < args.size(); i++)
    {
      proc->addArgument(args[i]);
    }
    // If the command fails to start, throw up a critical error msg
    if(! proc->start())
    {
      QMessageBox::critical(this, "Unable to Launch", "Unable to start " + args[0] + ".\n" +
          "Check to make sure it is executable and you have permissions.");
    }else
    {
      // add the program to the combo list
      //cmbCommands->insertItem(cmbCommands->currentText(), 0);
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
      te->setWordWrap(QTextEdit::NoWrap);
      // Create the tab page containing the textedit box. The
      // tab title is the first argument (program name)
      tabOutput->insertTab(te, args[0], 0);
      // set the current output to be the new tab
      tabOutput->setCurrentPage(0); 
      // point the output stream to the new tab
      textOutput = te;
      // Show the command in the textedit box for future reference
      textOutput->append("COMMAND: " + cmbCommands->currentText());
    }

  }else
  {
    QMessageBox::warning(this, 
        "Nothing to execute","Please specify the name of a program or script to execute");
  }

} 
void QgsLauncherPluginGui::readFromStdout()
{
  if(proc->canReadLineStdout())
  {
    QString line;
    while((line = proc->readLineStdout()) != QString::null)
    {
      textOutput->append(line);
    }
  }
}
void QgsLauncherPluginGui::readFromStderr()
{
  if(proc->canReadLineStderr())
  {
    QString line;
    while((line = proc->readLineStderr()) != QString::null)
    {
      textOutput->append(line);
    }
  }
}
void QgsLauncherPluginGui::processFinished()
{
  delete proc;
  // scroll the output to the top
  textOutput->setCursorPosition(0, 0);
}

void QgsLauncherPluginGui::cleanUp()
{
  // Write out the command list to the settings file and then exit
  QSettings settings;

  settings.writeEntry("/qgis/launcher_plugin/command_count", 
      cmbCommands->count());
  // write the commands in the combo box to the settings file (~/.qt/qgisrc)
  for(int i = 0; i < cmbCommands->count(); i++)
  {
    QString commandNumber;
    settings.writeEntry("/qgis/launcher_plugin/command" 
        + commandNumber.setNum(i), cmbCommands->text(i));
  }
  reject();
}
