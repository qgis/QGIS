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

//qt includes
#include <QColorDialog>

//standard includes

QgsCopyrightLabelPluginGui::QgsCopyrightLabelPluginGui( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  //programmatically hide orientation selection for now
  cboOrientation->hide();
  textLabel15->hide();
}

QgsCopyrightLabelPluginGui::~QgsCopyrightLabelPluginGui()
{
}

void QgsCopyrightLabelPluginGui::on_buttonBox_accepted()
{
  //hide the dialog before we send all our signals
  hide();
  //close the dialog
  emit changeFont( txtCopyrightText->currentFont() );
#if QT_VERSION < 0x040300
  emit changeLabel( txtCopyrightText->text() );
#else
  emit changeLabel( txtCopyrightText->toPlainText() );
#endif
  emit changeColor( pbnColorChooser->color() );
  emit changePlacement( cboPlacement->currentIndex() );
  emit enableCopyrightLabel( cboxEnabled->isChecked() );

  accept();
}

void QgsCopyrightLabelPluginGui::on_buttonBox_rejected()
{
  reject();
}

void QgsCopyrightLabelPluginGui::on_pbnColorChooser_clicked()
{
  QColor c = QColorDialog::getColor();
  if ( c.isValid() )
  {
    pbnColorChooser->setColor( c );
    QTextCursor cursor = txtCopyrightText->textCursor();
    txtCopyrightText->selectAll();
    txtCopyrightText->setTextColor( c );
    txtCopyrightText->setTextCursor( cursor );
  }
}

void QgsCopyrightLabelPluginGui::setEnabled( bool theBool )
{
  cboxEnabled->setChecked( theBool );
}

void QgsCopyrightLabelPluginGui::setText( QString theTextQString )
{
  txtCopyrightText->setPlainText( theTextQString );
}

void QgsCopyrightLabelPluginGui::setPlacementLabels( QStringList& labels )
{
  cboPlacement->clear();
  cboPlacement->addItems( labels );
}

void QgsCopyrightLabelPluginGui::setPlacement( int placementIndex )
{
  cboPlacement->setCurrentIndex( placementIndex );
}

void QgsCopyrightLabelPluginGui::setColor( QColor color )
{
  pbnColorChooser->setColor( color );
  QTextCursor cursor = txtCopyrightText->textCursor();
  txtCopyrightText->selectAll();
  txtCopyrightText->setTextColor( color );
  txtCopyrightText->setTextCursor( cursor );
}
