/***************************************************************************
                         qgscomposertablebackgroundcolorsdialog.cpp
                         ------------------------------------------
    begin                : August 2015
    copyright            : (C) 2015 by Nyall Dawson
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

#include "qgscomposertablebackgroundcolorsdialog.h"
#include "qgscomposertablev2.h"
#include "qgscomposition.h"
#include <QSettings>
#include <QCheckBox>
#include <QPushButton>

QgsComposerTableBackgroundColorsDialog::QgsComposerTableBackgroundColorsDialog( QgsComposerTableV2* table, QWidget* parent, Qt::WindowFlags flags )
    : QDialog( parent, flags )
    , mComposerTable( table )
{
  setupUi( this );

  mCheckBoxMap.insert( QgsComposerTableV2::OddColumns, mOddColumnsCheckBox );
  mCheckBoxMap.insert( QgsComposerTableV2::EvenColumns, mEvenColumnsCheckBox );
  mCheckBoxMap.insert( QgsComposerTableV2::OddRows, mOddRowsCheckBox );
  mCheckBoxMap.insert( QgsComposerTableV2::EvenRows, mEvenRowsCheckBox );
  mCheckBoxMap.insert( QgsComposerTableV2::FirstColumn, mFirstColumnCheckBox );
  mCheckBoxMap.insert( QgsComposerTableV2::LastColumn, mLastColumnCheckBox );
  mCheckBoxMap.insert( QgsComposerTableV2::HeaderRow, mHeaderRowCheckBox );
  mCheckBoxMap.insert( QgsComposerTableV2::FirstRow, mFirstRowCheckBox );
  mCheckBoxMap.insert( QgsComposerTableV2::LastRow, mLastRowCheckBox );

  mColorButtonMap.insert( QgsComposerTableV2::OddColumns, mOddColumnsColorButton );
  mColorButtonMap.insert( QgsComposerTableV2::EvenColumns, mEvenColumnsColorButton );
  mColorButtonMap.insert( QgsComposerTableV2::OddRows, mOddRowsColorButton );
  mColorButtonMap.insert( QgsComposerTableV2::EvenRows, mEvenRowsColorButton );
  mColorButtonMap.insert( QgsComposerTableV2::FirstColumn, mFirstColumnColorButton );
  mColorButtonMap.insert( QgsComposerTableV2::LastColumn, mLastColumnColorButton );
  mColorButtonMap.insert( QgsComposerTableV2::HeaderRow, mHeaderRowColorButton );
  mColorButtonMap.insert( QgsComposerTableV2::FirstRow, mFirstRowColorButton );
  mColorButtonMap.insert( QgsComposerTableV2::LastRow, mLastRowColorButton );

  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/ComposerTableBackgroundColorsDialog/geometry" ).toByteArray() );

  setGuiElementValues();
}

QgsComposerTableBackgroundColorsDialog::~QgsComposerTableBackgroundColorsDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/ComposerTableBackgroundColorsDialog/geometry", saveGeometry() );
}

void QgsComposerTableBackgroundColorsDialog::apply()
{
  if ( !mComposerTable )
    return;

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Table background customisation" ), QgsComposerMultiFrameMergeCommand::TableCellStyle );
  }

  Q_FOREACH ( QgsComposerTableV2::CellStyleGroup styleGroup, mCheckBoxMap.keys() )
  {
    QgsComposerTableStyle style;
    style.enabled = mCheckBoxMap.value( styleGroup )->isChecked();
    style.cellBackgroundColor = mColorButtonMap.value( styleGroup )->color();

    mComposerTable->setCellStyle( styleGroup, style );
  }

  mComposerTable->setBackgroundColor( mDefaultColorButton->color() );

  if ( composition )
  {
    composition->endMultiFrameCommand();
  }

  mComposerTable->update();
}

void QgsComposerTableBackgroundColorsDialog::on_buttonBox_accepted()
{
  apply();
  accept();
}

void QgsComposerTableBackgroundColorsDialog::on_buttonBox_rejected()
{
  reject();
}

void QgsComposerTableBackgroundColorsDialog::setGuiElementValues()
{
  if ( !mComposerTable )
    return;

  Q_FOREACH ( QgsComposerTableV2::CellStyleGroup styleGroup, mCheckBoxMap.keys() )
  {
    mCheckBoxMap.value( styleGroup )->setChecked( mComposerTable->cellStyle( styleGroup )->enabled );
    mColorButtonMap.value( styleGroup )->setEnabled( mComposerTable->cellStyle( styleGroup )->enabled );
    mColorButtonMap.value( styleGroup )->setColor( mComposerTable->cellStyle( styleGroup )->cellBackgroundColor );
    mColorButtonMap.value( styleGroup )->setAllowAlpha( true );
    mColorButtonMap.value( styleGroup )->setColorDialogTitle( tr( "Select background color" ) );
  }

  mDefaultColorButton->setColor( mComposerTable->backgroundColor() );
  mDefaultColorButton->setAllowAlpha( true );
  mDefaultColorButton->setColorDialogTitle( tr( "Select background color" ) );
  mDefaultColorButton->setShowNoColor( true );
  mDefaultColorButton->setNoColorString( tr( "No background" ) );
}
