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

#include <QCheckBox>
#include <QSpinBox>
#include <QList>

Q_GUI_EXPORT extern int qt_defaultDpiX();

QgsMapSaveDialog::QgsMapSaveDialog( QWidget *parent, QgsMapCanvas *mapCanvas, const QString &decorations )
  : QDialog( parent )
{
  setupUi( this );

  mExtent = mapCanvas->mapSettings().visibleExtent();
  mDpi = mapCanvas->mapSettings().outputDpi();
  mSize = mapCanvas->mapSettings().outputSize();

  mResolutionSpinBox->setValue( qt_defaultDpiX() );

  mExtentGroupBox->setOutputCrs( mapCanvas->mapSettings().destinationCrs() );
  mExtentGroupBox->setCurrentExtent( mapCanvas->mapSettings().visibleExtent(), mapCanvas->mapSettings().destinationCrs() );
  mExtentGroupBox->setOutputExtentFromCurrent();

  mScaleWidget->setScale( 1 / mapCanvas->mapSettings().scale() );
  mScaleWidget->setMapCanvas( mapCanvas );
  mScaleWidget->setShowCurrentScaleButton( true );

  mDrawDecorations->setText( QString( "Draw active decorations: %1" ).arg( !decorations.isEmpty() ? decorations : tr( "none" ) ) );

  connect( mResolutionSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsMapSaveDialog::updateDpi );
  connect( mExtentGroupBox, &QgsExtentGroupBox::extentChanged, this, &QgsMapSaveDialog::updateExtent );
  connect( mScaleWidget, &QgsScaleWidget::scaleChanged, this, &QgsMapSaveDialog::updateScale );

  updateOutputSize();
}

void QgsMapSaveDialog::updateDpi( int dpi )
{
  mSize *= ( double )dpi / mDpi;
  mDpi = dpi;

  updateOutputSize();
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
  mOutputSize->setText( QString( "Output size: %1 x %2 pixels" ).arg( mSize.width() ).arg( mSize.height() ) );
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
