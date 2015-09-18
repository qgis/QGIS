/***************************************************************************
    qgsenumerationwidgetwrapper.cpp
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

#include "qgsenumerationwidgetwrapper.h"

#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

QgsEnumerationWidgetWrapper::QgsEnumerationWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mComboBox( NULL )
{
}


QVariant QgsEnumerationWidgetWrapper::value()
{
  QVariant value;

  if ( mComboBox )
    value = mComboBox->itemData( mComboBox->currentIndex() );

  return value;
}

QWidget* QgsEnumerationWidgetWrapper::createWidget( QWidget* parent )
{
  return new QComboBox( parent );
}

void QgsEnumerationWidgetWrapper::initWidget( QWidget* editor )
{
  mComboBox = qobject_cast<QComboBox*>( editor );

  if ( mComboBox )
  {
    QStringList enumValues;
    layer()->dataProvider()->enumValues( fieldIdx(), enumValues );

    Q_FOREACH ( const QString& s, enumValues )
    {
      mComboBox->addItem( s, s );
    }
    connect( mComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( valueChanged() ) );
  }
}

bool QgsEnumerationWidgetWrapper::valid()
{
  return mComboBox;
}

void QgsEnumerationWidgetWrapper::setValue( const QVariant& value )
{
  if ( mComboBox )
  {
    mComboBox->setCurrentIndex( mComboBox->findData( value ) );
  }
}

