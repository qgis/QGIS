/***************************************************************************
                         qgscomposerimageexportoptionsdialog.cpp
                         ---------------------------------------
    begin                : September 2015
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

#include "qgscomposerimageexportoptionsdialog.h"
#include <QSettings>
#include <QCheckBox>
#include <QPushButton>

QgsComposerImageExportOptionsDialog::QgsComposerImageExportOptionsDialog( QWidget* parent, Qt::WindowFlags flags )
    : QDialog( parent, flags )
{
  setupUi( this );

  connect( mClipToContentGroupBox, SIGNAL( toggled( bool ) ), this, SLOT( clipToContentsToggled( bool ) ) );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/ComposerImageExportOptionsDialog/geometry" ).toByteArray() );
}

QgsComposerImageExportOptionsDialog::~QgsComposerImageExportOptionsDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/ComposerImageExportOptionsDialog/geometry", saveGeometry() );
}

void QgsComposerImageExportOptionsDialog::setResolution( int resolution )
{
  mResolutionSpinBox->setValue( resolution );

  if ( mImageSize.isValid() )
  {
    mWidthSpinBox->blockSignals( true );
    mHeightSpinBox->blockSignals( true );
    if ( mClipToContentGroupBox->isChecked() )
    {
      mWidthSpinBox->setValue( 0 );
      mHeightSpinBox->setValue( 0 );
    }
    else
    {
      mWidthSpinBox->setValue( mImageSize.width() * resolution / 25.4 );
      mHeightSpinBox->setValue( mImageSize.height() * resolution / 25.4 );
    }
    mWidthSpinBox->blockSignals( false );
    mHeightSpinBox->blockSignals( false );
  }
}

int QgsComposerImageExportOptionsDialog::resolution() const
{
  return mResolutionSpinBox->value();
}

void QgsComposerImageExportOptionsDialog::setImageSize( QSizeF size )
{
  mImageSize = size;
  mWidthSpinBox->blockSignals( true );
  mHeightSpinBox->blockSignals( true );
  mWidthSpinBox->setValue( size.width() * mResolutionSpinBox->value() / 25.4 );
  mHeightSpinBox->setValue( size.height() * mResolutionSpinBox->value() / 25.4 );
  mWidthSpinBox->blockSignals( false );
  mHeightSpinBox->blockSignals( false );
}

int QgsComposerImageExportOptionsDialog::imageWidth() const
{
  return mWidthSpinBox->value();
}

int QgsComposerImageExportOptionsDialog::imageHeight() const
{
  return mHeightSpinBox->value();
}

void QgsComposerImageExportOptionsDialog::setCropToContents( bool crop )
{
  mClipToContentGroupBox->setChecked( crop );
}

bool QgsComposerImageExportOptionsDialog::cropToContents() const
{
  return mClipToContentGroupBox->isChecked();
}

void QgsComposerImageExportOptionsDialog::getCropMargins( int& topMargin, int& rightMargin, int& bottomMargin, int& leftMargin ) const
{
  topMargin = mTopMarginSpinBox->value();
  rightMargin = mRightMarginSpinBox->value();
  bottomMargin = mBottomMarginSpinBox->value();
  leftMargin = mLeftMarginSpinBox->value();
}

void QgsComposerImageExportOptionsDialog::setCropMargins( int topMargin, int rightMargin, int bottomMargin, int leftMargin )
{
  mTopMarginSpinBox->setValue( topMargin );
  mRightMarginSpinBox->setValue( rightMargin );
  mBottomMarginSpinBox->setValue( bottomMargin );
  mLeftMarginSpinBox->setValue( leftMargin );
}

void QgsComposerImageExportOptionsDialog::on_mWidthSpinBox_valueChanged( int value )
{
  mHeightSpinBox->blockSignals( true );
  mResolutionSpinBox->blockSignals( true );
  mHeightSpinBox->setValue( mImageSize.height() * value / mImageSize.width() );
  mResolutionSpinBox->setValue( value * 25.4 / mImageSize.width() );
  mHeightSpinBox->blockSignals( false );
  mResolutionSpinBox->blockSignals( false );
}

void QgsComposerImageExportOptionsDialog::on_mHeightSpinBox_valueChanged( int value )
{
  mWidthSpinBox->blockSignals( true );
  mResolutionSpinBox->blockSignals( true );
  mWidthSpinBox->setValue( mImageSize.width() * value / mImageSize.height() );
  mResolutionSpinBox->setValue( value * 25.4 / mImageSize.height() );
  mWidthSpinBox->blockSignals( false );
  mResolutionSpinBox->blockSignals( false );
}

void QgsComposerImageExportOptionsDialog::on_mResolutionSpinBox_valueChanged( int value )
{
  mWidthSpinBox->blockSignals( true );
  mHeightSpinBox->blockSignals( true );
  if ( mClipToContentGroupBox->isChecked() )
  {
    mWidthSpinBox->setValue( 0 );
    mHeightSpinBox->setValue( 0 );
  }
  else
  {
    mWidthSpinBox->setValue( mImageSize.width() * value / 25.4 );
    mHeightSpinBox->setValue( mImageSize.height() * value / 25.4 );
  }
  mWidthSpinBox->blockSignals( false );
  mHeightSpinBox->blockSignals( false );
}

void QgsComposerImageExportOptionsDialog::clipToContentsToggled( bool state )
{
  mWidthSpinBox->setEnabled( !state );
  mHeightSpinBox->setEnabled( !state );

  if ( state )
  {
    whileBlocking( mWidthSpinBox )->setValue( 0 );
    whileBlocking( mHeightSpinBox )->setValue( 0 );
  }
  else
  {
    whileBlocking( mWidthSpinBox )->setValue( mImageSize.width() * mResolutionSpinBox->value() / 25.4 );
    whileBlocking( mHeightSpinBox )->setValue( mImageSize.height() * mResolutionSpinBox->value() / 25.4 );
  }
}
