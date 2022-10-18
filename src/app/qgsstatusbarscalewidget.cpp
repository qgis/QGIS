/***************************************************************************
                         qgsstatusbarscalewidget.cpp
    begin                : May 2016
    copyright            : (C) 2016 Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QValidator>

#include "qgsstatusbarscalewidget.h"

#include "qgsmapcanvas.h"
#include "qgsscalecombobox.h"
#include "qgsproject.h"
#include "qgsprojectviewsettings.h"

QgsStatusBarScaleWidget::QgsStatusBarScaleWidget( QgsMapCanvas *canvas, QWidget *parent )
  : QWidget( parent )
  , mMapCanvas( canvas )
{
  // add a label to show current scale
  mLabel = new QLabel();
  mLabel->setObjectName( QStringLiteral( "mScaleLabel" ) );
  mLabel->setMinimumWidth( 10 );
  //mScaleLabel->setMaximumHeight( 20 );
  mLabel->setMargin( 3 );
  mLabel->setAlignment( Qt::AlignCenter );
  mLabel->setFrameStyle( QFrame::NoFrame );
  mLabel->setText( tr( "Scale" ) );
  mLabel->setToolTip( tr( "Current map scale" ) );

  mScale = new QgsScaleComboBox();
  mScale->setObjectName( QStringLiteral( "mScaleEdit" ) );
  // seems setFont() change font only for popup not for line edit,
  // so we need to set font for it separately
  mScale->setMinimumWidth( 10 );
  mScale->setContentsMargins( 0, 0, 0, 0 );
  mScale->setToolTip( tr( "Current map scale" ) );

  // layout
  mLayout = new QHBoxLayout( this );
  mLayout->addWidget( mLabel );
  mLayout->addWidget( mScale );
  mLayout->setContentsMargins( 0, 0, 0, 0 );
  mLayout->setAlignment( Qt::AlignRight );
  mLayout->setSpacing( 0 );

  setLayout( mLayout );

  connect( mScale, &QgsScaleComboBox::scaleChanged, this, &QgsStatusBarScaleWidget::userScale );
  connect( mMapCanvas, &QgsMapCanvas::scaleLockChanged, this, &QgsStatusBarScaleWidget::setLocked );
}

void QgsStatusBarScaleWidget::setScale( double scale )
{
  mScale->blockSignals( true );
  mScale->setScale( scale );
  mScale->blockSignals( false );

  if ( mScale->width() > mScale->minimumWidth() )
  {
    mScale->setMinimumWidth( mScale->width() );
  }
}

bool QgsStatusBarScaleWidget::isLocked() const
{
  return !mScale->isEnabled();
}

void QgsStatusBarScaleWidget::setLocked( bool state )
{
  mScale->setDisabled( state );
}

void QgsStatusBarScaleWidget::setFont( const QFont &font )
{
  mLabel->setFont( font );
  mScale->lineEdit()->setFont( font );
}

void QgsStatusBarScaleWidget::updateScales()
{
  if ( QgsProject::instance()->viewSettings()->useProjectScales() )
  {
    const QVector< double > scales = QgsProject::instance()->viewSettings()->mapScales();
    QStringList textScales;
    textScales.reserve( scales.size() );
    for ( const double scale : scales )
      textScales << QStringLiteral( "1:%1" ).arg( QLocale().toString( scale, 'f', 0 ) );
    mScale->updateScales( textScales );
  }
  else
  {
    // use global scales
    mScale->updateScales();
  }
}

void QgsStatusBarScaleWidget::userScale() const
{
  mMapCanvas->zoomScale( mScale->scale() );
}
