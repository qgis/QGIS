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

QgsColorWidgetWrapper::QgsColorWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) :
    QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
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
  return new QgsColorButton( parent );
}

void QgsColorWidgetWrapper::initWidget( QWidget* editor )
{
  mColorButton = qobject_cast<QgsColorButton*>( editor );

  connect( mColorButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( valueChanged() ) );
}

void QgsColorWidgetWrapper::setValue( const QVariant& value )
{
  if ( mColorButton )
    mColorButton->setColor( QColor( value.toString() ) );
}
