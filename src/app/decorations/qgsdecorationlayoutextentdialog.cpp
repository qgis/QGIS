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

#include "qgisapp.h"
#include "qgsdecorationlayoutextent.h"
#include "qgsfillsymbol.h"
#include "qgsgui.h"
#include "qgsguiutils.h"
#include "qgshelp.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgssymbolselectordialog.h"
#include "qgstextformatwidget.h"

#include "moc_qgsdecorationlayoutextentdialog.cpp"

QgsDecorationLayoutExtentDialog::QgsDecorationLayoutExtentDialog( QgsDecorationLayoutExtent &deco, QWidget *parent )
  : QDialog( parent )
  , mDeco( deco )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsDecorationLayoutExtentDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsDecorationLayoutExtentDialog::buttonBox_rejected );

  mSymbolButton->setSymbolType( Qgis::SymbolType::Fill );

  updateGuiElements();
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsDecorationLayoutExtentDialog::apply );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDecorationLayoutExtentDialog::showHelp );

  mSymbolButton->setMapCanvas( QgisApp::instance()->mapCanvas() );
  mSymbolButton->setMessageBar( QgisApp::instance()->messageBar() );
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
  mDeco.setSymbol( mSymbolButton->clonedSymbol<QgsFillSymbol>() );
  mDeco.setTextFormat( mButtonFontStyle->textFormat() );
  mDeco.setLabelExtents( mCheckBoxLabelExtents->isChecked() );
}

void QgsDecorationLayoutExtentDialog::buttonBox_accepted()
{
  apply();
  accept();
}

void QgsDecorationLayoutExtentDialog::apply()
{
  updateDecoFromGui();
  mDeco.update();
}

void QgsDecorationLayoutExtentDialog::buttonBox_rejected()
{
  reject();
}

void QgsDecorationLayoutExtentDialog::showHelp()
{
  QgsHelp::openHelp( u"map_views/map_view.html#layoutextents-decoration"_s );
}
