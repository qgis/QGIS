/***************************************************************************
   qgsscalewidget.cpp
    --------------------------------------
   Date                 : 08.01.2015
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QHBoxLayout>

#include "qgsapplication.h"
#include "qgsscalewidget.h"
#include "qgsmapcanvas.h"

QgsScaleWidget::QgsScaleWidget( QWidget *parent )
    : QWidget( parent )
    , mCanvas( NULL )
    , mShowCurrentScaleButton( false )
{
  QHBoxLayout* layout = new QHBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 2 );

  mScaleComboBox = new QgsScaleComboBox( this );
  layout->addWidget( mScaleComboBox );

  mCurrentScaleButton = new QToolButton( this );
  mCurrentScaleButton->setIcon( QgsApplication::getThemeIcon( "/mActionMapIdentification.svg" ) );
  layout->addWidget( mCurrentScaleButton );
  mCurrentScaleButton->hide();

  connect( mScaleComboBox, SIGNAL( scaleChanged() ), this, SIGNAL( scaleChanged() ) );
  connect( mCurrentScaleButton, SIGNAL( clicked() ), this, SLOT( setScaleFromCanvas() ) );
}


QgsScaleWidget::~QgsScaleWidget()
{
}

void QgsScaleWidget::setShowCurrentScaleButton( bool showCurrentScaleButton )
{
  mShowCurrentScaleButton = showCurrentScaleButton;
  mCurrentScaleButton->setVisible( mShowCurrentScaleButton && mCanvas );
}

void QgsScaleWidget::setMapCanvas( QgsMapCanvas* canvas )
{
  mCanvas = canvas;
  mCurrentScaleButton->setVisible( mShowCurrentScaleButton && mCanvas );
}

void QgsScaleWidget::setScaleFromCanvas()
{
  if ( !mCanvas )
    return;

  setScale( 1/mCanvas->scale() );
}



