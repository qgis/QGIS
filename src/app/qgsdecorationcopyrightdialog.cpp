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

QgsDecorationCopyrightDialog::QgsDecorationCopyrightDialog( QgsDecorationCopyright& deco, QWidget* parent )
    : QDialog( parent ), mDeco( deco )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/DecorationCopyright/geometry" ).toByteArray() );

  //programmatically hide orientation selection for now
  cboOrientation->hide();
  textLabel15->hide();

  cboxEnabled->setChecked( mDeco.enabled() );
  // text
  txtCopyrightText->setPlainText( mDeco.mLabelQString );
  // placement
  cboPlacement->clear();
  cboPlacement->addItems( mDeco.mPlacementLabels );
  cboPlacement->setCurrentIndex( mDeco.mPlacementIndex );
  // color
  pbnColorChooser->setColor( mDeco.mLabelQColor );
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
  mDeco.mQFont = txtCopyrightText->currentFont();
  mDeco.mLabelQString = txtCopyrightText->toPlainText();
  mDeco.mLabelQColor = pbnColorChooser->color();
  mDeco.mPlacementIndex = cboPlacement->currentIndex();
  mDeco.setEnabled( cboxEnabled->isChecked() );

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

void QgsDecorationCopyrightDialog::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( metaObject()->className() );
}
