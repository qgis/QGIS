/***************************************************************************
    qgssymbolanimationsettingswidget.cpp
    ---------------------
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbolanimationsettingswidget.h"
#include "qgssymbol.h"

#include <QDialogButtonBox>

QgsSymbolAnimationSettingsWidget::QgsSymbolAnimationSettingsWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  mFrameRateSpin->setClearValue( 10 );
  mFrameRateSpin->setShowClearButton( true );

  connect( mFrameRateSpin, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [ this ]
  {
    if ( !mBlockUpdates )
      emit widgetChanged();
  } );
  connect( mIsAnimatedGroup, &QGroupBox::toggled, this, [ this ]
  {
    if ( !mBlockUpdates )
      emit widgetChanged();
  } );
}

void QgsSymbolAnimationSettingsWidget::setAnimationSettings( const QgsSymbolAnimationSettings &settings )
{
  mBlockUpdates = true;
  mIsAnimatedGroup->setChecked( settings.isAnimated() );
  mFrameRateSpin->setValue( settings.frameRate() );
  mBlockUpdates = false;
}

QgsSymbolAnimationSettings QgsSymbolAnimationSettingsWidget::animationSettings() const
{
  QgsSymbolAnimationSettings settings;
  settings.setIsAnimated( mIsAnimatedGroup->isChecked() );
  settings.setFrameRate( mFrameRateSpin->value() );
  return settings;
}


//
// QgsSymbolAnimationSettingsDialog
//

QgsSymbolAnimationSettingsDialog::QgsSymbolAnimationSettingsDialog( QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
{
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsSymbolAnimationSettingsWidget( );
  vLayout->addWidget( mWidget );
  QDialogButtonBox *bbox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal );
  connect( bbox, &QDialogButtonBox::accepted, this, &QgsSymbolAnimationSettingsDialog::accept );
  connect( bbox, &QDialogButtonBox::rejected, this, &QgsSymbolAnimationSettingsDialog::reject );
  vLayout->addWidget( bbox );
  setLayout( vLayout );
  setWindowTitle( tr( "Animation Settings" ) );
}

void QgsSymbolAnimationSettingsDialog::setAnimationSettings( const QgsSymbolAnimationSettings &settings )
{
  mWidget->setAnimationSettings( settings );
}

QgsSymbolAnimationSettings QgsSymbolAnimationSettingsDialog::animationSettings() const
{
  return mWidget->animationSettings();
}

