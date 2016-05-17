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

#include "qgsdecorationcopyrightdialog.h"
#include "qgsdecorationcopyright.h"

#include "qgscontexthelp.h"

//qt includes
#include <QColorDialog>
#include <QColor>
#include <QFont>
#include <QSettings>
#include <QDialogButtonBox>
#include <QPushButton>

QgsDecorationCopyrightDialog::QgsDecorationCopyrightDialog( QgsDecorationCopyright& deco, QWidget* parent )
    : QDialog( parent )
    , mDeco( deco )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/DecorationCopyright/geometry" ).toByteArray() );

  QPushButton* applyButton = buttonBox->button( QDialogButtonBox::Apply );
  connect( applyButton, SIGNAL( clicked() ), this, SLOT( apply() ) );

  grpEnable->setChecked( mDeco.enabled() );
  // text
  txtCopyrightText->setPlainText( mDeco.mLabelQString );
  // placement
  cboPlacement->addItem( tr( "Top left" ), QgsDecorationItem::TopLeft );
  cboPlacement->addItem( tr( "Top right" ), QgsDecorationItem::TopRight );
  cboPlacement->addItem( tr( "Bottom left" ), QgsDecorationItem::BottomLeft );
  cboPlacement->addItem( tr( "Bottom right" ), QgsDecorationItem::BottomRight );
  cboPlacement->setCurrentIndex( cboPlacement->findData( mDeco.placement() ) );
  spnHorizontal->setValue( mDeco.mMarginHorizontal );
  spnVertical->setValue( mDeco.mMarginVertical );
  wgtUnitSelection->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::Percentage << QgsSymbolV2::Pixel );
  wgtUnitSelection->setUnit( mDeco.mMarginUnit );

  // color
  pbnColorChooser->setColor( mDeco.mLabelQColor );
  pbnColorChooser->setContext( "gui" );
  pbnColorChooser->setColorDialogTitle( tr( "Select text color" ) );

  QTextCursor cursor = txtCopyrightText->textCursor();
  txtCopyrightText->selectAll();
  txtCopyrightText->setTextColor( mDeco.mLabelQColor );
  txtCopyrightText->setTextCursor( cursor );
}

QgsDecorationCopyrightDialog::~QgsDecorationCopyrightDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/DecorationCopyright/geometry", saveGeometry() );
}

void QgsDecorationCopyrightDialog::on_buttonBox_accepted()
{
  apply();
  accept();
}

void QgsDecorationCopyrightDialog::on_buttonBox_rejected()
{
  reject();
}

void QgsDecorationCopyrightDialog::on_pbnColorChooser_colorChanged( const QColor& c )
{
  QTextCursor cursor = txtCopyrightText->textCursor();
  txtCopyrightText->selectAll();
  txtCopyrightText->setTextColor( c );
  txtCopyrightText->setTextCursor( cursor );
}

void QgsDecorationCopyrightDialog::apply()
{
  mDeco.mQFont = txtCopyrightText->currentFont();
  mDeco.mLabelQString = txtCopyrightText->toPlainText();
  mDeco.mLabelQColor = pbnColorChooser->color();
  mDeco.setPlacement( static_cast< QgsDecorationItem::Placement>( cboPlacement->itemData( cboPlacement->currentIndex() ).toInt() ) );
  mDeco.mMarginUnit = wgtUnitSelection->unit();
  mDeco.mMarginHorizontal = spnHorizontal->value();
  mDeco.mMarginVertical = spnVertical->value();
  mDeco.setEnabled( grpEnable->isChecked() );
  mDeco.update();
}

void QgsDecorationCopyrightDialog::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( metaObject()->className() );
}
