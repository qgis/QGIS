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
#include "qgssettings.h"

#include <QCheckBox>
#include <QPushButton>

QgsComposerTableBackgroundColorsDialog::QgsComposerTableBackgroundColorsDialog( QgsComposerTableV2 *table, QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
  , mComposerTable( table )
{
  setupUi( this );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsComposerTableBackgroundColorsDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsComposerTableBackgroundColorsDialog::buttonBox_rejected );

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

  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsComposerTableBackgroundColorsDialog::apply );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/ComposerTableBackgroundColorsDialog/geometry" ) ).toByteArray() );

  setGuiElementValues();
}

QgsComposerTableBackgroundColorsDialog::~QgsComposerTableBackgroundColorsDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/ComposerTableBackgroundColorsDialog/geometry" ), saveGeometry() );
}

void QgsComposerTableBackgroundColorsDialog::apply()
{
  if ( !mComposerTable )
    return;

  QgsComposition *composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Table background customisation" ), QgsComposerMultiFrameMergeCommand::TableCellStyle );
  }

  QMap< QgsComposerTableV2::CellStyleGroup, QCheckBox * >::const_iterator checkBoxIt = mCheckBoxMap.constBegin();
  for ( ; checkBoxIt != mCheckBoxMap.constEnd(); ++checkBoxIt )
  {
    QgsComposerTableStyle style;
    style.enabled = checkBoxIt.value()->isChecked();
    if ( QgsColorButton *button = mColorButtonMap.value( checkBoxIt.key() ) )
      style.cellBackgroundColor = button->color();

    mComposerTable->setCellStyle( checkBoxIt.key(), style );
  }

  mComposerTable->setBackgroundColor( mDefaultColorButton->color() );

  if ( composition )
  {
    composition->endMultiFrameCommand();
  }

  mComposerTable->update();
}

void QgsComposerTableBackgroundColorsDialog::buttonBox_accepted()
{
  apply();
  accept();
}

void QgsComposerTableBackgroundColorsDialog::buttonBox_rejected()
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
    QgsColorButton *button = mColorButtonMap.value( styleGroup );
    if ( !button )
      continue;
    button->setEnabled( mComposerTable->cellStyle( styleGroup )->enabled );
    button->setColor( mComposerTable->cellStyle( styleGroup )->cellBackgroundColor );
    button->setAllowOpacity( true );
    button->setColorDialogTitle( tr( "Select Background Color" ) );
  }

  mDefaultColorButton->setColor( mComposerTable->backgroundColor() );
  mDefaultColorButton->setAllowOpacity( true );
  mDefaultColorButton->setColorDialogTitle( tr( "Select Background Color" ) );
  mDefaultColorButton->setShowNoColor( true );
  mDefaultColorButton->setNoColorString( tr( "No background" ) );
}
