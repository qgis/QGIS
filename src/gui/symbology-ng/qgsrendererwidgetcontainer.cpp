/***************************************************************************
    qgsrendererwidgetcontainer.cpp
    ---------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                :
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QDialogButtonBox>
#include <QPushButton>

#include "qgsrendererwidgetcontainer.h"

QgsRendererWidgetContainer::QgsRendererWidgetContainer( QWidget *widget, const QString& title, QWidget *parent )
    : QWidget( parent )
    , mWidget( widget )
{
  setupUi( this );
  mWidgetLayout->addWidget( widget );
  mWidgetLayout->setContentsMargins( 0, 0, 0, 0 );
  mTitleText->setText( title );
  QPushButton* button = mButtonBox->button( QDialogButtonBox::Close );
  button->setDefault( true );
  connect( button, SIGNAL( pressed() ), this, SLOT( accept() ) );
}

QWidget *QgsRendererWidgetContainer::widget()
{
  return mWidget;
}

void QgsRendererWidgetContainer::accept()
{
  emit accepted( this );
}

void QgsRendererWidgetContainer::emitWidgetChanged()
{
  emit widgetChanged( this );
}

void QgsRendererWidgetContainer::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Escape )
  {
    accept();
  }
}
