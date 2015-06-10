/***************************************************************************
                          qgsbusyindicatordialog.cpp
                          --------------------------
    begin                : Mar 27, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakcarto dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsbusyindicatordialog.h"

#include <QDialog>
#include <QLayout>
#include <QLabel>
#include <QProgressBar>

QgsBusyIndicatorDialog::QgsBusyIndicatorDialog( const QString& message, QWidget* parent, Qt::WindowFlags fl )
    : QDialog( parent, fl ), mMessage( QString( message ) ), mMsgLabel( 0 )
{
  setWindowTitle( tr( "QGIS" ) );
  setLayout( new QVBoxLayout() );
  setWindowModality( Qt::WindowModal );
  setMinimumWidth( 250 );
  mMsgLabel = new QLabel( mMessage );
  layout()->addWidget( mMsgLabel );

  QProgressBar* pb = new QProgressBar();
  pb->setMaximum( 0 ); // show as busy indicator
  layout()->addWidget( pb );

  if ( mMessage.isEmpty() )
  {
    mMsgLabel->hide();
  }
}

QgsBusyIndicatorDialog::~QgsBusyIndicatorDialog()
{
}

void QgsBusyIndicatorDialog::setMessage( const QString& message )
{
  if ( !message.isEmpty() )
  {
    mMessage = QString( message );
    mMsgLabel->setText( mMessage );
    mMsgLabel->show();
  }
}
