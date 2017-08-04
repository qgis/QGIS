/***************************************************************************
                         qgsstatusbar.cpp
                         ----------------
    begin                : May 2017
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

#include "qgsstatusbar.h"
#include <QLayout>
#include <QLabel>
#include <QTimer>

QgsStatusBar::QgsStatusBar( QWidget *parent )
  : QWidget( parent )
{
  mLayout = new QHBoxLayout();
  mLayout->setMargin( 0 );
  mLayout->setContentsMargins( 2, 0, 2, 0 );
  mLayout->setSpacing( 6 );

  mLabel = new QLabel( QString() );
  mLayout->addWidget( mLabel, 1 );
  setLayout( mLayout );
}

void QgsStatusBar::addPermanentWidget( QWidget *widget, int stretch, Anchor anchor )
{
  switch ( anchor )
  {
    case AnchorLeft:
      mLayout->insertWidget( 0, widget, stretch, Qt::AlignLeft );
      break;

    case AnchorRight:
      mLayout->addWidget( widget, stretch, Qt::AlignLeft );
      break;
  }
}

void QgsStatusBar::removeWidget( QWidget *widget )
{
  mLayout->removeWidget( widget );
}

QString QgsStatusBar::currentMessage() const
{
  return mLabel->text();
}

void QgsStatusBar::showMessage( const QString &text, int timeout )
{
  mLabel->setText( text );
  if ( timeout > 0 )
  {
    if ( !mTempMessageTimer )
    {
      mTempMessageTimer = new QTimer( this );
      connect( mTempMessageTimer, &QTimer::timeout, this, &QgsStatusBar::clearMessage );
    }
    mTempMessageTimer->start( timeout );
  }
  else if ( mTempMessageTimer )
  {
    delete mTempMessageTimer;
    mTempMessageTimer = nullptr;
  }
}

void QgsStatusBar::clearMessage()
{
  mLabel->setText( QString() );
}
