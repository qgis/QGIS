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

#include "qgshelp.h"
#include "qgssettings.h"

//qt includes
#include <QColorDialog>
#include <QColor>
#include <QFont>
#include <QDialogButtonBox>
#include <QPushButton>

QgsDecorationCopyrightDialog::QgsDecorationCopyrightDialog( QgsDecorationCopyright &deco, QWidget *parent )
  : QDialog( parent )
  , mDeco( deco )
{
  setupUi( this );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsDecorationCopyrightDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsDecorationCopyrightDialog::buttonBox_rejected );
  connect( pbnColorChooser, &QgsColorButton::colorChanged, this, &QgsDecorationCopyrightDialog::pbnColorChooser_colorChanged );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDecorationCopyrightDialog::showHelp );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/DecorationCopyright/geometry" ) ).toByteArray() );

  QPushButton *applyButton = buttonBox->button( QDialogButtonBox::Apply );
  connect( applyButton, &QAbstractButton::clicked, this, &QgsDecorationCopyrightDialog::apply );

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
  wgtUnitSelection->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderPercentage << QgsUnitTypes::RenderPixels );
  wgtUnitSelection->setUnit( mDeco.mMarginUnit );

  // color
  pbnColorChooser->setAllowOpacity( true );
  pbnColorChooser->setColor( mDeco.mColor );
  pbnColorChooser->setContext( QStringLiteral( "gui" ) );
  pbnColorChooser->setColorDialogTitle( tr( "Select Text color" ) );

  QTextCursor cursor = txtCopyrightText->textCursor();
  txtCopyrightText->selectAll();
  txtCopyrightText->setTextColor( mDeco.mColor );
  txtCopyrightText->setTextCursor( cursor );
}

QgsDecorationCopyrightDialog::~QgsDecorationCopyrightDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/DecorationCopyright/geometry" ), saveGeometry() );
}

void QgsDecorationCopyrightDialog::buttonBox_accepted()
{
  apply();
  accept();
}

void QgsDecorationCopyrightDialog::buttonBox_rejected()
{
  reject();
}

void QgsDecorationCopyrightDialog::pbnColorChooser_colorChanged( const QColor &c )
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
  mDeco.mColor = pbnColorChooser->color();
  mDeco.setPlacement( static_cast< QgsDecorationItem::Placement>( cboPlacement->currentData().toInt() ) );
  mDeco.mMarginUnit = wgtUnitSelection->unit();
  mDeco.mMarginHorizontal = spnHorizontal->value();
  mDeco.mMarginVertical = spnVertical->value();
  mDeco.setEnabled( grpEnable->isChecked() );
  mDeco.update();
}

void QgsDecorationCopyrightDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#copyright-label" ) );
}
