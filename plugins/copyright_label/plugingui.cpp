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
#include <qtextedit.h>
#include <qsimplerichtext.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>

//qt includes

//standard includes

PluginGui::PluginGui() : PluginGuiBase()
{
  //programmatically hide orientation selection for now
  cboOrientation->hide();
  textLabel15->hide();
}

PluginGui::PluginGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: PluginGuiBase( parent, name, modal, fl )
{
  //programmatically hide orientation selection for now
  cboOrientation->hide();
  textLabel15->hide();
}  
PluginGui::~PluginGui()
{
}

void PluginGui::pbnOK_clicked()
{
  //
  // If you have a produced a raster layer using your plugin, you can ask qgis to
  // add it to the view using:
  // emit drawRasterLayer(QString("layername"));
  // or for a vector layer
  // emit drawVectorLayer(QString("pathname"),QString("layername"),QString("provider name (either ogr or postgres"));
  //
  //close the dialog
  //emit refreshCanvas();
  emit changeFont(txtCopyrightText->currentFont());
  emit changeLabel(txtCopyrightText->text());
  emit changeColor(txtCopyrightText->color());
  emit changePlacement(cboPlacement->currentText());
  emit enableCopyrightLabel(cboxEnabled->isChecked());
  
  done(1);
} 
void PluginGui::pbnCancel_clicked()
{
 close(1);
}

void PluginGui::setEnabled(bool theBool)
{
  cboxEnabled->setChecked(theBool);
}

