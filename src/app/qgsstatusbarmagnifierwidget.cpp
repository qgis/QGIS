/***************************************************************************
                         qgsstatusbarmagnifierwidget.cpp
    begin                : April 2016
    copyright            : (C) 2016 Paul Blottiere, Oslandia
    email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>

#include <qgsapplication.h>
#include "qgsstatusbarmagnifierwidget.h"
#include "qgsmapcanvas.h"
#include "qgsdoublespinbox.h"

QgsStatusBarMagnifierWidget::QgsStatusBarMagnifierWidget( QWidget* parent,
    QgsMapCanvas *canvas ) :
    QWidget( parent ),
    mCanvas( canvas ),
    mMagnifier( 100 ),
    mMagnifierMin( 100 ),
    mMagnifierMax( 1000 )
{
  // label
  mLabel = new QLabel( this );
  mLabel->setMinimumWidth( 10 );
  mLabel->setMargin( 3 );
  mLabel->setAlignment( Qt::AlignCenter );
  mLabel->setFrameStyle( QFrame::NoFrame );
  mLabel->setText( tr( "Magnifier" ) );
  mLabel->setToolTip( tr( "Magnifier" ) );

  mSpinBox = new QgsDoubleSpinBox( this );
  mSpinBox->setSuffix( "%" );
  mSpinBox->setClearValue( mMagnifierMin );
  mSpinBox->setKeyboardTracking( false );
  mSpinBox->setMaximumWidth( 120 );
  mSpinBox->setDecimals( 0 );
  mSpinBox->setRange( mMagnifierMin, mMagnifierMax );
  mSpinBox->setWrapping( false );
  mSpinBox->setSingleStep( 50 );
  mSpinBox->setToolTip( tr( "Magnifier level" ) );

  connect( mSpinBox, SIGNAL( valueChanged( double ) ), this,
           SLOT( updateMagnifier() ) );

  // layout
  mLayout = new QHBoxLayout( this );
  mLayout->addWidget( mLabel );
  mLayout->addWidget( mSpinBox );
  mLayout->setContentsMargins( 0, 0, 0, 0 );
  mLayout->setAlignment( Qt::AlignRight );
  mLayout->setSpacing( 0 );

  setLayout( mLayout );

  updateMagnifier();
}

QgsStatusBarMagnifierWidget::~QgsStatusBarMagnifierWidget()
{
}

double QgsStatusBarMagnifierWidget::magnificationLevel()
{
  return mMagnifier;
}

void QgsStatusBarMagnifierWidget::setFont( const QFont& myFont )
{
  mLabel->setFont( myFont );
  mSpinBox->setFont( myFont );
}

bool QgsStatusBarMagnifierWidget::setMagnificationLevel( int level )
{
  bool rc = false;

  if ( level >= mMagnifierMin && level <= mMagnifierMax )
  {
    mSpinBox->setValue( level );
    rc = true;
  }

  return rc;
}

void QgsStatusBarMagnifierWidget::updateMagnifier()
{
  // get current data
  mMagnifier = mSpinBox->value();

  // update map canvas
  mCanvas->setMagnificationFactor( mMagnifier / double( mMagnifierMin ) );
}
