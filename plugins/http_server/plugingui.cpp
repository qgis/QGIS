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
#include "plugingui.h"

//qt includes
#include <qtextedit.h>
#include <qcheckbox.h>
#include <qspinbox.h> 

//standard includes

PluginGui::PluginGui() : PluginGuiBase()
{

}

    PluginGui::PluginGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: PluginGuiBase( parent, name, modal, fl )
{

}  
PluginGui::~PluginGui()
{
}

void PluginGui::pbnOK_clicked()
{
  bool myFlag = cboEnableServer->isChecked();
  int myPortInt = spinPort->value();
  emit enabledChanged(myFlag);
  emit portChanged(myPortInt);
  
  //disconnect any connections to this dialog
  disconnect( this, 0, 0, 0 );
  //close the dialog
  done(1);
} 
void PluginGui::pbnApply_clicked()
{
  bool myFlag = cboEnableServer->isChecked();
  int myPortInt = spinPort->value();
  emit enabledChanged(myFlag);
  emit portChanged(myPortInt);
}
void PluginGui::pbnCancel_clicked()
{
  close(1);
}

void PluginGui::newConnect(QString theMessage)
{
  teLogs->append( theMessage  );
}
void PluginGui::endConnect(QString theMessage)
{
  teLogs->append( theMessage);
}
void PluginGui::wroteToClient(QString theMessage)
{
  teLogs->append( theMessage );
}
void PluginGui::requestReceived(QString theString)
{
  teDebug->append(theString);
}

void PluginGui::cboxEnableServer_toggled( bool )
{

}


void PluginGui::spinPort_valueChanged( int )
{

}
