/***************************************************************************
    qgstexteditwrapper.cpp
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

#include "qgsdefaultsearchwidgetwrapper.h"

#include "qgsfield.h"
#include "qgsfieldvalidator.h"

#include <QSettings>
#include <QSizePolicy>

QgsDefaultSearchWidgetWrapper::QgsDefaultSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mLineEdit( NULL )
{
}

QVariant QgsDefaultSearchWidgetWrapper::value()
{
  return mLineEdit->text();
}

QWidget* QgsDefaultSearchWidgetWrapper::createWidget( QWidget* parent )
{
  return new QgsFilterLineEdit( parent );
}

void QgsDefaultSearchWidgetWrapper::initWidget( QWidget* widget )
{
  mLineEdit = qobject_cast<QgsFilterLineEdit*>( widget );
  mLineEdit->setSizePolicy( QSizePolicy ::Expanding , QSizePolicy ::Fixed );
  connect( widget, SIGNAL( textChanged( QString ) ), this, SLOT( valueChanged( QString ) ) );
}

void QgsDefaultSearchWidgetWrapper::setValue( const QVariant& value )
{
  mLineEdit->setText( value.toString() ); //FIXME no null check :(
}

void QgsDefaultSearchWidgetWrapper::setEnabled( bool enabled )
{
  mLineEdit->setReadOnly( !enabled );
#if 0
  if ( enabled )
    mLineEdit->setPalette( mWritablePalette );
  else
    mLineEdit->setPalette( mReadOnlyPalette );
#endif
}
