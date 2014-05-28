/***************************************************************************
    qgsenumerationwidget.cpp
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

#include "qgsenumerationwidget.h"

#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

QgsEnumerationWidget::QgsEnumerationWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    :  QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
{
}


QVariant QgsEnumerationWidget::value()
{
  QVariant value;

  if ( mComboBox )
    value = mComboBox->itemData( mComboBox->currentIndex() );

  return value;
}

QWidget* QgsEnumerationWidget::createWidget( QWidget* parent )
{
  return new QComboBox( parent );
}

void QgsEnumerationWidget::initWidget( QWidget* editor )
{
  mComboBox = qobject_cast<QComboBox*>( editor );

  if ( mComboBox )
  {
    QStringList enumValues;
    layer()->dataProvider()->enumValues( fieldIdx(), enumValues );

    Q_FOREACH( const QString& s, enumValues )
    {
      mComboBox->addItem( s, s );
    }
    connect( mComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( valueChanged() ) );
  }
}

void QgsEnumerationWidget::setValue( const QVariant& value )
{
  if ( mComboBox )
  {
    mComboBox->setCurrentIndex( mComboBox->findData( value ) );
  }
}

