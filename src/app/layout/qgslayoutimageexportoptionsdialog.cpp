/***************************************************************************
                         qgslayoutimageexportoptionsdialog.cpp
                         -------------------------------------
    begin                : December 2017
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

#include "qgslayoutimageexportoptionsdialog.h"
#include "qgis.h"
#include "qgssettings.h"
#include "qgsgui.h"

#include <QCheckBox>
#include <QPushButton>

QgsLayoutImageExportOptionsDialog::QgsLayoutImageExportOptionsDialog( QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setupUi( this );
  connect( mWidthSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutImageExportOptionsDialog::mWidthSpinBox_valueChanged );
  connect( mHeightSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutImageExportOptionsDialog::mHeightSpinBox_valueChanged );
  connect( mResolutionSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutImageExportOptionsDialog::mResolutionSpinBox_valueChanged );

  connect( mClipToContentGroupBox, &QGroupBox::toggled, this, &QgsLayoutImageExportOptionsDialog::clipToContentsToggled );

  QgsGui::enableAutoGeometryRestore( this );
}

void QgsLayoutImageExportOptionsDialog::setResolution( double resolution )
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

double QgsLayoutImageExportOptionsDialog::resolution() const
{
  return mResolutionSpinBox->value();
}

void QgsLayoutImageExportOptionsDialog::setImageSize( QSizeF size )
{
  mImageSize = size;
  mWidthSpinBox->blockSignals( true );
  mHeightSpinBox->blockSignals( true );
  mWidthSpinBox->setValue( size.width() * mResolutionSpinBox->value() / 25.4 );
  mHeightSpinBox->setValue( size.height() * mResolutionSpinBox->value() / 25.4 );
  mWidthSpinBox->blockSignals( false );
  mHeightSpinBox->blockSignals( false );
}

int QgsLayoutImageExportOptionsDialog::imageWidth() const
{
  return mWidthSpinBox->value();
}

int QgsLayoutImageExportOptionsDialog::imageHeight() const
{
  return mHeightSpinBox->value();
}

void QgsLayoutImageExportOptionsDialog::setCropToContents( bool crop )
{
  mClipToContentGroupBox->setChecked( crop );
}

bool QgsLayoutImageExportOptionsDialog::cropToContents() const
{
  return mClipToContentGroupBox->isChecked();
}

void QgsLayoutImageExportOptionsDialog::setGenerateWorldFile( bool generate )
{
  mGenerateWorldFile->setChecked( generate );
}

bool QgsLayoutImageExportOptionsDialog::generateWorldFile() const
{
  return mGenerateWorldFile->isChecked();
}

void QgsLayoutImageExportOptionsDialog::setAntialiasing( bool antialias )
{
  mAntialiasingCheckBox->setChecked( antialias );
}

bool QgsLayoutImageExportOptionsDialog::antialiasing() const
{
  return mAntialiasingCheckBox->isChecked();
}

void QgsLayoutImageExportOptionsDialog::getCropMargins( int &topMargin, int &rightMargin, int &bottomMargin, int &leftMargin ) const
{
  topMargin = mTopMarginSpinBox->value();
  rightMargin = mRightMarginSpinBox->value();
  bottomMargin = mBottomMarginSpinBox->value();
  leftMargin = mLeftMarginSpinBox->value();
}

void QgsLayoutImageExportOptionsDialog::setCropMargins( int topMargin, int rightMargin, int bottomMargin, int leftMargin )
{
  mTopMarginSpinBox->setValue( topMargin );
  mRightMarginSpinBox->setValue( rightMargin );
  mBottomMarginSpinBox->setValue( bottomMargin );
  mLeftMarginSpinBox->setValue( leftMargin );
}

void QgsLayoutImageExportOptionsDialog::mWidthSpinBox_valueChanged( int value )
{
  mHeightSpinBox->blockSignals( true );
  mResolutionSpinBox->blockSignals( true );
  mHeightSpinBox->setValue( mImageSize.height() * value / mImageSize.width() );
  mResolutionSpinBox->setValue( value * 25.4 / mImageSize.width() );
  mHeightSpinBox->blockSignals( false );
  mResolutionSpinBox->blockSignals( false );
}

void QgsLayoutImageExportOptionsDialog::mHeightSpinBox_valueChanged( int value )
{
  mWidthSpinBox->blockSignals( true );
  mResolutionSpinBox->blockSignals( true );
  mWidthSpinBox->setValue( mImageSize.width() * value / mImageSize.height() );
  mResolutionSpinBox->setValue( value * 25.4 / mImageSize.height() );
  mWidthSpinBox->blockSignals( false );
  mResolutionSpinBox->blockSignals( false );
}

void QgsLayoutImageExportOptionsDialog::mResolutionSpinBox_valueChanged( int value )
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

void QgsLayoutImageExportOptionsDialog::clipToContentsToggled( bool state )
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
