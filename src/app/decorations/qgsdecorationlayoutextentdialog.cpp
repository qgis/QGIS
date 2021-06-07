/***************************************************************************
                          qgsdecorationlayoutextentdialog.cpp
                              ----------------------------
    begin                : May 2017
    copyright            : (C) 2020 by Wang Peng
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
#include "qgsgui.h"

QgsDecorationLayoutExtentDialog::QgsDecorationLayoutExtentDialog( QgsDecorationLayoutExtent &deco, QWidget *parent )
  : QDialog( parent )
  , mDeco( deco )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsDecorationLayoutExtentDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsDecorationLayoutExtentDialog::buttonBox_rejected );

  mSymbolButton->setSymbolType( QgsSymbol::Fill );

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
  mDeco.setSymbol( mSymbolButton->clonedSymbol< QgsFillSymbol >() );
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
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#layout-extents" ) );
}
