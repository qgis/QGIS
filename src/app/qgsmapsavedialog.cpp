/***************************************************************************
                         qgsmapsavedialog.cpp
                         -------------------------------------
    begin                : April 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapsavedialog.h"

#include "qgis.h"
#include "qgsscalecalculator.h"
#include "qgsdecorationitem.h"
#include "qgsextentgroupbox.h"
#include "qgsmapsettings.h"
#include "qgsmapsettingsutils.h"

#include <QCheckBox>
#include <QSpinBox>
#include <QList>

Q_GUI_EXPORT extern int qt_defaultDpiX();

QgsMapSaveDialog::QgsMapSaveDialog( QWidget *parent, QgsMapCanvas *mapCanvas, const QString &activeDecorations, DialogType type )
  : QDialog( parent )
{
  setupUi( this );

  // Use unrotated visible extent to insure output size and scale matches canvas
  QgsMapSettings ms = mapCanvas->mapSettings();
  ms.setRotation( 0 );
  mExtent = ms.visibleExtent();
  mDpi = ms.outputDpi();
  mSize = ms.outputSize();

  mResolutionSpinBox->setValue( qt_defaultDpiX() );

  mExtentGroupBox->setOutputCrs( ms.destinationCrs() );
  mExtentGroupBox->setCurrentExtent( mExtent, ms.destinationCrs() );
  mExtentGroupBox->setOutputExtentFromCurrent();

  mScaleWidget->setScale( 1 / ms.scale() );
  mScaleWidget->setMapCanvas( mapCanvas );
  mScaleWidget->setShowCurrentScaleButton( true );

  mDrawDecorations->setText( tr( "Draw active decorations: %1" ).arg( !activeDecorations.isEmpty() ? activeDecorations : tr( "none" ) ) );

  connect( mResolutionSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsMapSaveDialog::updateDpi );
  connect( mOutputWidthSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsMapSaveDialog::updateOutputWidth );
  connect( mOutputHeightSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsMapSaveDialog::updateOutputHeight );
  connect( mExtentGroupBox, &QgsExtentGroupBox::extentChanged, this, &QgsMapSaveDialog::updateExtent );
  connect( mScaleWidget, &QgsScaleWidget::scaleChanged, this, &QgsMapSaveDialog::updateScale );

  updateOutputSize();

  if ( type == QgsMapSaveDialog::Pdf )
  {
    mSaveWorldFile->setVisible( false );

    QStringList layers = QgsMapSettingsUtils::containsAdvancedEffects( mapCanvas->mapSettings() );
    if ( !layers.isEmpty() )
    {
      // Limit number of items to avoid extreme dialog height
      if ( layers.count() >= 10 )
      {
        layers = layers.mid( 0, 9 );
        layers << QChar( 0x2026 );
      }
      mInfo->setText( tr( "The following layer(s) use advanced effects:\n%1\nRasterizing map is recommended for proper rendering." ).arg(
                        QChar( 0x2022 ) + QString( " " ) + layers.join( QString( "\n" ) + QChar( 0x2022 ) + QString( " " ) ) ) );
      mSaveAsRaster->setChecked( true );
    }
    else
    {
      mSaveAsRaster->setChecked( false );
    }
    mSaveAsRaster->setVisible( true );

    this->setWindowTitle( tr( "Save map as PDF" ) );
  }
}

void QgsMapSaveDialog::updateDpi( int dpi )
{
  mSize *= ( double )dpi / mDpi;
  mDpi = dpi;

  updateOutputSize();
}

void QgsMapSaveDialog::updateOutputWidth( int width )
{
  double scale = ( double )width / mSize.width();
  double adjustment = ( ( mExtent.width() * scale ) - mExtent.width() ) / 2;

  mExtent.setXMinimum( mExtent.xMinimum() - adjustment );
  mExtent.setXMaximum( mExtent.xMaximum() + adjustment );

  whileBlocking( mExtentGroupBox )->setOutputExtentFromUser( mExtent, mExtentGroupBox->currentCrs() );

  mSize.setWidth( width );
}

void QgsMapSaveDialog::updateOutputHeight( int height )
{
  double scale = ( double )height / mSize.height();
  double adjustment = ( ( mExtent.height() * scale ) - mExtent.height() ) / 2;

  mExtent.setYMinimum( mExtent.yMinimum() - adjustment );
  mExtent.setYMaximum( mExtent.yMaximum() + adjustment );

  whileBlocking( mExtentGroupBox )->setOutputExtentFromUser( mExtent, mExtentGroupBox->currentCrs() );

  mSize.setHeight( height );
}

void QgsMapSaveDialog::updateExtent( const QgsRectangle &extent )
{
  mSize.setWidth( mSize.width() * extent.width() / mExtent.width() );
  mSize.setHeight( mSize.height() * extent.height() / mExtent.height() );
  mExtent = extent;

  updateOutputSize();
}

void QgsMapSaveDialog::updateScale( double scale )
{
  QgsScaleCalculator calculator;
  calculator.setMapUnits( mExtentGroupBox->currentCrs().mapUnits() );
  calculator.setDpi( mDpi );

  double oldScale = 1 / ( calculator.calculate( mExtent, mSize.width() ) );
  double scaleRatio = oldScale / scale;
  mExtent.scale( scaleRatio );
  mExtentGroupBox->setOutputExtentFromUser( mExtent, mExtentGroupBox->currentCrs() );
}

void QgsMapSaveDialog::updateOutputSize()
{
  whileBlocking( mOutputWidthSpinBox )->setValue( mSize.width() );
  whileBlocking( mOutputHeightSpinBox )->setValue( mSize.height() );
}

QgsRectangle QgsMapSaveDialog::extent() const
{
  return mExtentGroupBox->outputExtent();
}

int QgsMapSaveDialog::dpi() const
{
  return mResolutionSpinBox->value();
}

QSize QgsMapSaveDialog::size() const
{
  return mSize;
}

bool QgsMapSaveDialog::drawAnnotations() const
{
  return mDrawAnnotations->isChecked();
}

bool QgsMapSaveDialog::drawDecorations() const
{
  return mDrawDecorations->isChecked();
}

bool QgsMapSaveDialog::saveWorldFile() const
{
  return mSaveWorldFile->isChecked();
}

bool QgsMapSaveDialog::saveAsRaster() const
{
  return mSaveAsRaster->isChecked();
}
