/***************************************************************************
    qgscheckboxwidgetwrapper.cpp
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

#include "qgscheckboxwidgetwrapper.h"

QgsCheckboxWidgetWrapper::QgsCheckboxWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mCheckBox( nullptr )
    , mGroupBox( nullptr )
{
}


QVariant QgsCheckboxWidgetWrapper::value() const
{
  QVariant v;

  if ( mGroupBox )
    v = mGroupBox->isChecked() ? config( "CheckedState" ) : config( "UncheckedState" );

  if ( mCheckBox )
    v = mCheckBox->isChecked() ? config( "CheckedState" ) : config( "UncheckedState" );


  return v;
}

void QgsCheckboxWidgetWrapper::showIndeterminateState()
{
  if ( mCheckBox )
  {
    whileBlocking( mCheckBox )->setCheckState( Qt::PartiallyChecked );
  }
}

QWidget* QgsCheckboxWidgetWrapper::createWidget( QWidget* parent )
{
  return new QCheckBox( parent );
}

void QgsCheckboxWidgetWrapper::initWidget( QWidget* editor )
{
  mCheckBox = qobject_cast<QCheckBox*>( editor );
  mGroupBox = qobject_cast<QGroupBox*>( editor );

  if ( mCheckBox )
    connect( mCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( valueChanged( bool ) ) );
  if ( mGroupBox )
    connect( mGroupBox, SIGNAL( toggled( bool ) ), this, SLOT( valueChanged( bool ) ) );
}

bool QgsCheckboxWidgetWrapper::valid() const
{
  return mCheckBox || mGroupBox;
}

void QgsCheckboxWidgetWrapper::setValue( const QVariant& value )
{
  bool state = ( value == config( "CheckedState" ) );
  if ( mGroupBox )
  {
    mGroupBox->setChecked( state );
  }

  if ( mCheckBox )
  {
    mCheckBox->setChecked( state );
  }
}
