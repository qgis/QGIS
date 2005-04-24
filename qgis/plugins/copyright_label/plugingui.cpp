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

QgsCopyrightLabelPluginGui::QgsCopyrightLabelPluginGui() : QgsCopyrightLabelPluginGuiBase()
{
  //programmatically hide orientation selection for now
  cboOrientation->hide();
  textLabel15->hide();
}

QgsCopyrightLabelPluginGui::QgsCopyrightLabelPluginGui( QWidget* parent , const char* name , bool modal , WFlags fl  )
: QgsCopyrightLabelPluginGuiBase( parent, name, modal, fl )
{
  //programmatically hide orientation selection for now
  cboOrientation->hide();
  textLabel15->hide();
}  
QgsCopyrightLabelPluginGui::~QgsCopyrightLabelPluginGui()
{
}

void QgsCopyrightLabelPluginGui::pbnOK_clicked()
{
  //hide the dialog before we send all our signals
  hide();
  //close the dialog
  emit changeFont(txtCopyrightText->currentFont());
  emit changeLabel(txtCopyrightText->text());
  emit changeColor(txtCopyrightText->color());
  emit changePlacement(cboPlacement->currentText());
  emit enableCopyrightLabel(cboxEnabled->isChecked());
  
  done(1);
} 
void QgsCopyrightLabelPluginGui::pbnCancel_clicked()
{
 close(1);
}

void QgsCopyrightLabelPluginGui::setEnabled(bool theBool)
{
  cboxEnabled->setChecked(theBool);
}

void QgsCopyrightLabelPluginGui::setText(QString theTextQString)
{
  txtCopyrightText->setText(theTextQString);
}

void QgsCopyrightLabelPluginGui::setPlacement(QString thePlacementQString)
{
  cboPlacement->setCurrentText(tr(thePlacementQString));
}
