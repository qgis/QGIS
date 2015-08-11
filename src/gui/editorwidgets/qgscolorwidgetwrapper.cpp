/***************************************************************************
    qgscolorwidgetwrapper.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorwidgetwrapper.h"

QgsColorWidgetWrapper::QgsColorWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mColorButton( NULL )
{
}


QVariant QgsColorWidgetWrapper::value()
{
  QVariant v;

  if ( mColorButton )
    v = mColorButton->color();

  return v;
}

QWidget* QgsColorWidgetWrapper::createWidget( QWidget* parent )
{
  QgsColorButtonV2* button = new QgsColorButtonV2( parent );
  button->setContext( QString( "editor" ) );
  return button;
}

void QgsColorWidgetWrapper::initWidget( QWidget* editor )
{
  mColorButton = qobject_cast<QgsColorButtonV2*>( editor );

  connect( mColorButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( valueChanged() ) );
}

bool QgsColorWidgetWrapper::valid()
{
  return mColorButton;
}

void QgsColorWidgetWrapper::setValue( const QVariant& value )
{
  if ( mColorButton )
    mColorButton->setColor( QColor( value.toString() ) );
}
