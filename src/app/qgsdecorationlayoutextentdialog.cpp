/***************************************************************************
                          qgsdecorationlayoutextentdialog.cpp
                              ----------------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdecorationlayoutextentdialog.h"

#include "qgsdecorationlayoutextent.h"

#include "qgslogger.h"
#include "qgshelp.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgssymbolselectordialog.h"
#include "qgisapp.h"
#include "qgsguiutils.h"
#include "qgssettings.h"
#include "qgstextformatwidget.h"

QgsDecorationLayoutExtentDialog::QgsDecorationLayoutExtentDialog( QgsDecorationLayoutExtent &deco, QWidget *parent )
  : QDialog( parent )
  , mDeco( deco )
{
  setupUi( this );

  mSymbolButton->setSymbolType( QgsSymbol::Fill );

  QgsSettings settings;
  restoreGeometry( settings.value( "/Windows/DecorationLayoutExtent/geometry" ).toByteArray() );

  connect( mCheckBoxLabelExtents, &QCheckBox::toggled, mButtonFontStyle, &QPushButton::setEnabled );
  mCheckBoxLabelExtents->setChecked( false );
  mButtonFontStyle->setEnabled( false );

  updateGuiElements();
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsDecorationLayoutExtentDialog::apply );

  mSymbolButton->setMapCanvas( QgisApp::instance()->mapCanvas() );
}

void QgsDecorationLayoutExtentDialog::updateGuiElements()
{
  grpEnable->setChecked( mDeco.enabled() );
  mSymbolButton->setSymbol( mDeco.symbol()->clone() );
  mButtonFontStyle->setTextFormat( mDeco.textFormat() );
  mCheckBoxLabelExtents->setChecked( mDeco.labelExtents() );
}

void QgsDecorationLayoutExtentDialog::updateDecoFromGui()
{
  mDeco.setEnabled( grpEnable->isChecked() );
  mDeco.setSymbol( mSymbolButton->clonedSymbol< QgsFillSymbol >() );
  mDeco.setTextFormat( mButtonFontStyle->textFormat() );
  mDeco.setLabelExtents( mCheckBoxLabelExtents->isChecked() );
}

QgsDecorationLayoutExtentDialog::~QgsDecorationLayoutExtentDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/Windows/DecorationLayoutExtent/geometry" ), saveGeometry() );
}

void QgsDecorationLayoutExtentDialog::on_buttonBox_accepted()
{
  apply();
  accept();
}

void QgsDecorationLayoutExtentDialog::apply()
{
  updateDecoFromGui();
  mDeco.update();
}

void QgsDecorationLayoutExtentDialog::on_buttonBox_rejected()
{
  reject();
}
