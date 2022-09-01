/***************************************************************************
    qgscolorwidgetwrapper.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorwidgetwrapper.h"
#include "qgscolorbutton.h"
#include <QLayout>


QgsColorWidgetWrapper::QgsColorWidgetWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )

{
}


QVariant QgsColorWidgetWrapper::value() const
{
  QColor c;
  if ( mColorButton )
    c = mColorButton->color();

  return c.isValid() ? QVariant( c ) : QVariant( QVariant::Color );
}

void QgsColorWidgetWrapper::showIndeterminateState()
{
  if ( mColorButton )
  {
    whileBlocking( mColorButton )->setColor( QColor() );
  }
}

QWidget *QgsColorWidgetWrapper::createWidget( QWidget *parent )
{
  QWidget *container = new QWidget( parent );
  QHBoxLayout *layout = new QHBoxLayout();
  container->setLayout( layout );
  layout->setContentsMargins( 0, 0, 0, 0 );
  QgsColorButton *button = new QgsColorButton();
  button->setContext( QStringLiteral( "editor" ) );
  layout->addWidget( button );
  layout->addStretch();
  container->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );
  return container;
}

void QgsColorWidgetWrapper::initWidget( QWidget *editor )
{
  mColorButton = qobject_cast<QgsColorButton *>( editor );
  if ( !mColorButton )
  {
    mColorButton = editor->findChild<QgsColorButton *>();
  }

  mColorButton->setShowNull( true );
  connect( mColorButton, &QgsColorButton::colorChanged, this, static_cast<void ( QgsEditorWidgetWrapper::* )()>( &QgsEditorWidgetWrapper::emitValueChanged ) );
}

bool QgsColorWidgetWrapper::valid() const
{
  return mColorButton;
}

void QgsColorWidgetWrapper::updateValues( const QVariant &value, const QVariantList & )
{
  if ( mColorButton )
    mColorButton->setColor( !QgsVariantUtils::isNull( value ) ? QColor( value.toString() ) : QColor() );
}

void QgsColorWidgetWrapper::updateConstraintWidgetStatus()
{
  // nothing
}
