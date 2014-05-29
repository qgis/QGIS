/***************************************************************************
    qgscheckboxwidget.cpp
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

#include "qgscheckboxwidget.h"

QgsCheckboxWidget::QgsCheckboxWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    :  QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
{
}


QVariant QgsCheckboxWidget::value()
{
  QVariant v;

  if ( mGroupBox )
    v = mGroupBox->isChecked() ? config( "CheckedState" ) : config( "UncheckedState" );

  if ( mCheckBox )
    v = mCheckBox->isChecked() ? config( "CheckedState" ) : config( "UncheckedState" );


  return v;
}

QWidget* QgsCheckboxWidget::createWidget( QWidget* parent )
{
  return new QCheckBox( parent );
}

void QgsCheckboxWidget::initWidget( QWidget* editor )
{
  mCheckBox = qobject_cast<QCheckBox*>( editor );
  mGroupBox = qobject_cast<QGroupBox*>( editor );

  if ( mCheckBox )
    connect( mCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( valueChanged( bool ) ) );
  if ( mGroupBox )
    connect( mGroupBox, SIGNAL( toggled( bool ) ), this, SLOT( valueChanged( bool ) ) );
}

void QgsCheckboxWidget::setValue( const QVariant& value )
{
  if ( mGroupBox )
  {
    mGroupBox->setChecked( value == config( "CheckedState" ) );
  }

  if ( mCheckBox )
  {
    mCheckBox->setChecked( value == config( "CheckedState" ) );
  }
}
