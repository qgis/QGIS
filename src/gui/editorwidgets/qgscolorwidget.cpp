/***************************************************************************
    qgscolorwidget.cpp
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

#include "qgscolorwidget.h"

QgsColorWidget::QgsColorWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) :
    QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
{
}


QVariant QgsColorWidget::value()
{
  QVariant v;

  if ( mColorButton )
    v = mColorButton->color();

  return v;
}

QWidget*QgsColorWidget::createWidget( QWidget* parent )
{
  return new QgsColorButton( parent );
}

void QgsColorWidget::initWidget( QWidget* editor )
{
  mColorButton = qobject_cast<QgsColorButton*>( editor );

  connect( mColorButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( valueChanged() ) );
}

void QgsColorWidget::setValue( const QVariant& value )
{
  if ( mColorButton )
    mColorButton->setColor( QColor( value.toString() ) );
}
