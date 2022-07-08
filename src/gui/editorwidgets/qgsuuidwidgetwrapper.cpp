/***************************************************************************
    qgsuuidwidgetwrapper.cpp
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

#include "qgsuuidwidgetwrapper.h"

#include <QUuid>

QgsUuidWidgetWrapper::QgsUuidWidgetWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )

{
}

QString QgsUuidWidgetWrapper::createUiid( int maxLength )
{
  QString uuid = QUuid::createUuid().toString();

  if ( maxLength <= 0 || maxLength >= uuid.length() )
  {
    return uuid;
  }
  else
  {
    // trim left "{" and remove -'s... they are wasted characters given that we have a limited length!
    return uuid.replace( '-', QString() ).mid( 1, maxLength );
  }
}

QVariant QgsUuidWidgetWrapper::value() const
{
  QVariant v;

  if ( mLineEdit )
    v = mLineEdit->text();
  if ( mLabel )
    v = mLabel->text();

  return v;
}

QWidget *QgsUuidWidgetWrapper::createWidget( QWidget *parent )
{
  return new QLineEdit( parent );
}

void QgsUuidWidgetWrapper::initWidget( QWidget *editor )
{
  mLineEdit = qobject_cast<QLineEdit *>( editor );
  mLabel = qobject_cast<QLabel *>( editor );
  if ( mLineEdit )
    mLineEdit->setEnabled( false );
}

bool QgsUuidWidgetWrapper::valid() const
{
  return mLineEdit || mLabel;
}

void QgsUuidWidgetWrapper::updateValues( const QVariant &value, const QVariantList & )
{
  if ( value.isNull() )
  {
    int maxLength = 0;
    if ( field().type() == QVariant::String && field().length() > 0 )
    {
      maxLength = field().length();
    }
    const QString uuid = createUiid( maxLength );
    if ( mLineEdit )
      mLineEdit->setText( uuid );
    if ( mLabel )
      mLabel->setText( uuid );

    emitValueChanged();
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
