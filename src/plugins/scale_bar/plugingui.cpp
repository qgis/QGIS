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
#include "qgscontexthelp.h"
#include <QComboBox>
#include <QColorDialog>

QgsScaleBarPluginGui::QgsScaleBarPluginGui( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
}

QgsScaleBarPluginGui::~QgsScaleBarPluginGui()
{
}

QSpinBox * QgsScaleBarPluginGui::getSpinSize()
{
  return spnSize;
}

void QgsScaleBarPluginGui::on_buttonBox_accepted()
{
  hide();
  emit changePlacement( cboPlacement->currentIndex() );
  emit changePreferredSize( spnSize->value() );
  emit changeSnapping( chkSnapping->isChecked() );
  emit changeEnabled( chkEnable->isChecked() );
  emit changeStyle( cboStyle->currentIndex() );
  emit changeColor( pbnChangeColor->color() );
  emit refreshCanvas();
  accept();
}

void QgsScaleBarPluginGui::on_pbnChangeColor_clicked()
{
  QColor color = QColorDialog::getColor( pbnChangeColor->color(), this );

  if ( color.isValid() )
    setColor( color );
}

void QgsScaleBarPluginGui::on_buttonBox_rejected()
{
  reject();
}

void QgsScaleBarPluginGui::setPlacementLabels( QStringList& labels )
{
  cboPlacement->clear();
  cboPlacement->addItems( labels );
}

void QgsScaleBarPluginGui::setPlacement( int placementIndex )
{
  cboPlacement->setCurrentIndex( placementIndex );
}

void QgsScaleBarPluginGui::setPreferredSize( int thePreferredSize )
{
  spnSize->setValue( thePreferredSize );
}

void QgsScaleBarPluginGui::setSnapping( bool theSnapping )
{
  chkSnapping->setChecked( theSnapping );
}
void QgsScaleBarPluginGui::setEnabled( bool theBool )
{
  chkEnable->setChecked( theBool );
}

void QgsScaleBarPluginGui::setStyleLabels( QStringList& labels )
{
  cboStyle->clear();
  cboStyle->addItems( labels );
}

void QgsScaleBarPluginGui::setStyle( int styleIndex )
{
  cboStyle->setCurrentIndex( styleIndex );
}

void QgsScaleBarPluginGui::setColor( QColor theQColor )
{
  pbnChangeColor->setColor( theQColor );
}
