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
#include <QComboBox>
#include <QColorDialog>

QgsScaleBarPluginGui::QgsScaleBarPluginGui() : QDialog()
{
  setupUi(this);

}

QgsScaleBarPluginGui::QgsScaleBarPluginGui( QWidget* parent , const char* name , bool modal , Qt::WFlags fl  )
: QDialog(parent, name, modal, fl)
{
  setupUi(this);
}  
QgsScaleBarPluginGui::~QgsScaleBarPluginGui()
{
}

QSpinBox * QgsScaleBarPluginGui::getSpinSize()
{
    return spnSize;
}

void QgsScaleBarPluginGui::on_pbnOK_clicked()
{
  hide();
  emit changePlacement(cboPlacement->currentText());
  emit changePreferredSize(spnSize->value());
  emit changeSnapping(chkSnapping->isChecked());
  emit changeEnabled(chkEnable->isChecked());
  emit changeStyle(cboStyle->currentText());
  emit changeColour(pbnChangeColour->palette().color(QPalette::Button));
  emit refreshCanvas();
  done(1);
} 
void QgsScaleBarPluginGui::on_pbnChangeColour_clicked()
{
  QColor colour = QColorDialog::getColor(
		   pbnChangeColour->palette().color(QPalette::Button), this);
  
  if (colour.isValid())
    setColour(colour);
}
void QgsScaleBarPluginGui::on_pbnCancel_clicked()
{
 close(1);
}

void QgsScaleBarPluginGui::setPlacement(QString thePlacementQString)
{
  cboPlacement->setCurrentText(tr(thePlacementQString));
}

void QgsScaleBarPluginGui::setPreferredSize(int thePreferredSize)
{
  spnSize->setValue(thePreferredSize);
}

void QgsScaleBarPluginGui::setSnapping(bool theSnapping)
{
  chkSnapping->setChecked(theSnapping);
}
void QgsScaleBarPluginGui::setEnabled(bool theBool)
{
  chkEnable->setChecked(theBool);
}

void QgsScaleBarPluginGui::setStyle(QString theStyleQString)
{
  if ((tr(theStyleQString))=="Tick Down")
  {
    cboStyle->setCurrentItem(0);
  }
  else if ((tr(theStyleQString))=="Tick Up")
  {
    cboStyle->setCurrentItem(1);
  }
  else if ((tr(theStyleQString))=="Box")
  {
    cboStyle->setCurrentItem(2);
  }
  else if ((tr(theStyleQString))=="Bar")
  {
    cboStyle->setCurrentItem(3);
  }
}

void QgsScaleBarPluginGui::setColour(QColor theQColor)
{
  QPalette palette;
  palette.setColor(QPalette::Button, theQColor);
  pbnChangeColour->setPalette(palette);
}
