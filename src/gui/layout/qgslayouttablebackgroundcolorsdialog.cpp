/***************************************************************************
                         qgslayouttablebackgroundcolorsdialog.cpp
                         ----------------------------------------
    begin                : November 2017
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

#include "qgslayouttablebackgroundcolorsdialog.h"
#include "qgslayouttable.h"
#include "qgslayout.h"
#include "qgssettings.h"
#include "qgshelp.h"
#include "qgsgui.h"

#include <QCheckBox>
#include <QPushButton>

QgsLayoutTableBackgroundColorsDialog::QgsLayoutTableBackgroundColorsDialog( QgsLayoutTable *table, QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
  , mTable( table )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsLayoutTableBackgroundColorsDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsLayoutTableBackgroundColorsDialog::buttonBox_rejected );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsLayoutTableBackgroundColorsDialog::showHelp );

  mCheckBoxMap.insert( QgsLayoutTable::OddColumns, mOddColumnsCheckBox );
  mCheckBoxMap.insert( QgsLayoutTable::EvenColumns, mEvenColumnsCheckBox );
  mCheckBoxMap.insert( QgsLayoutTable::OddRows, mOddRowsCheckBox );
  mCheckBoxMap.insert( QgsLayoutTable::EvenRows, mEvenRowsCheckBox );
  mCheckBoxMap.insert( QgsLayoutTable::FirstColumn, mFirstColumnCheckBox );
  mCheckBoxMap.insert( QgsLayoutTable::LastColumn, mLastColumnCheckBox );
  mCheckBoxMap.insert( QgsLayoutTable::HeaderRow, mHeaderRowCheckBox );
  mCheckBoxMap.insert( QgsLayoutTable::FirstRow, mFirstRowCheckBox );
  mCheckBoxMap.insert( QgsLayoutTable::LastRow, mLastRowCheckBox );

  mColorButtonMap.insert( QgsLayoutTable::OddColumns, mOddColumnsColorButton );
  mColorButtonMap.insert( QgsLayoutTable::EvenColumns, mEvenColumnsColorButton );
  mColorButtonMap.insert( QgsLayoutTable::OddRows, mOddRowsColorButton );
  mColorButtonMap.insert( QgsLayoutTable::EvenRows, mEvenRowsColorButton );
  mColorButtonMap.insert( QgsLayoutTable::FirstColumn, mFirstColumnColorButton );
  mColorButtonMap.insert( QgsLayoutTable::LastColumn, mLastColumnColorButton );
  mColorButtonMap.insert( QgsLayoutTable::HeaderRow, mHeaderRowColorButton );
  mColorButtonMap.insert( QgsLayoutTable::FirstRow, mFirstRowColorButton );
  mColorButtonMap.insert( QgsLayoutTable::LastRow, mLastRowColorButton );

  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsLayoutTableBackgroundColorsDialog::apply );

  setGuiElementValues();
}

void QgsLayoutTableBackgroundColorsDialog::apply()
{
  if ( !mTable )
    return;

  mTable->beginCommand( tr( "Change Table Background" ), QgsLayoutMultiFrame::UndoTableCellStyle );
  for ( auto checkBoxIt = mCheckBoxMap.constBegin(); checkBoxIt != mCheckBoxMap.constEnd(); ++checkBoxIt )
  {
    QgsLayoutTableStyle style;
    style.enabled = checkBoxIt.value()->isChecked();
    if ( QgsColorButton *button = mColorButtonMap.value( checkBoxIt.key() ) )
      style.cellBackgroundColor = button->color();

    mTable->setCellStyle( checkBoxIt.key(), style );
  }

  mTable->setBackgroundColor( mDefaultColorButton->color() );
  mTable->endCommand();
  mTable->update();
}

void QgsLayoutTableBackgroundColorsDialog::buttonBox_accepted()
{
  apply();
  accept();
}

void QgsLayoutTableBackgroundColorsDialog::buttonBox_rejected()
{
  reject();
}

void QgsLayoutTableBackgroundColorsDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "print_composer/composer_items/composer_attribute_table.html#appearance" ) );
}

void QgsLayoutTableBackgroundColorsDialog::setGuiElementValues()
{
  if ( !mTable )
    return;

  for ( auto it = mCheckBoxMap.constBegin(); it != mCheckBoxMap.constEnd(); ++it )
  {
    it.value()->setChecked( mTable->cellStyle( it.key() )->enabled );
    QgsColorButton *button = mColorButtonMap.value( it.key() );
    if ( !button )
      continue;
    button->setEnabled( mTable->cellStyle( it.key() )->enabled );
    button->setColor( mTable->cellStyle( it.key() )->cellBackgroundColor );
    button->setAllowOpacity( true );
    button->setColorDialogTitle( tr( "Select Background Color" ) );
  }

  mDefaultColorButton->setColor( mTable->backgroundColor() );
  mDefaultColorButton->setAllowOpacity( true );
  mDefaultColorButton->setColorDialogTitle( tr( "Select Background Color" ) );
  mDefaultColorButton->setShowNoColor( true );
  mDefaultColorButton->setNoColorString( tr( "No Background" ) );
}
