/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "qgslauncherplugingui.h"

//qt includes
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qprocess.h>
#include <qtextedit.h>
//standard includes

QgsLauncherPluginGui::QgsLauncherPluginGui() : QgsLauncherPluginGuiBase()
{

}

  QgsLauncherPluginGui::QgsLauncherPluginGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: QgsLauncherPluginGuiBase( parent, name, modal, fl )
{

}  
QgsLauncherPluginGui::~QgsLauncherPluginGui()
{
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
    for(int i=0; i < args.size(); i++)
    {
      proc->addArgument(args[i]);
    }
    if(! proc->start())
    {
      QMessageBox::critical(this, "Unable to Launch", "Unable to start " + args[0] + ".\n" +
          "Check to make sure it is executable and you have permissions.");
    }else
    {
      // add the program to the combo list
      cmbCommands->insertItem(cmbCommands->currentText(), 0);
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
      txtOutput->setText(txtOutput->text() + line + "\n");
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
      txtOutput->setText(txtOutput->text() + line + "\n");
    }
  }
}
