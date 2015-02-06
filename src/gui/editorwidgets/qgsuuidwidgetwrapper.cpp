/***************************************************************************
    qgsuuidwidgetwrapper.cpp
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

#include "qgsuuidwidgetwrapper.h"

#include <QUuid>

QgsUuidWidgetWrapper::QgsUuidWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mLabel( NULL )
    , mLineEdit( NULL )
{
}

QVariant QgsUuidWidgetWrapper::value()
{
  QVariant v;

  if ( mLineEdit )
    v = mLineEdit->text();
  if ( mLabel )
    v = mLabel->text();

  return v;
}

QWidget* QgsUuidWidgetWrapper::createWidget( QWidget* parent )
{
  return new QLineEdit( parent );
}

void QgsUuidWidgetWrapper::initWidget( QWidget* editor )
{
  mLineEdit = qobject_cast<QLineEdit*>( editor );
  mLabel = qobject_cast<QLabel*>( editor );
  if ( mLineEdit )
    mLineEdit->setEnabled( false );
}

void QgsUuidWidgetWrapper::setValue( const QVariant& value )
{
  if ( value.isNull() )
  {
    if ( mLineEdit )
      mLineEdit->setText( QUuid::createUuid().toString() );
    if ( mLabel )
      mLabel->setText( QUuid::createUuid().toString() );

    valueChanged();
  }
  else
  {
    if ( mLineEdit )
      mLineEdit->setText( value.toString() );
    if ( mLabel )
      mLabel->setText( value.toString() );
  }
}

void QgsUuidWidgetWrapper::setEnabled( bool enabled )
{
  Q_UNUSED( enabled )
  // Do nothing... it is always disabled
}
