/***************************************************************************
    qgsuniquevaluewidget.cpp
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

#include "qgsuniquevaluewidget.h"

#include "qgsvectorlayer.h"

#include <QCompleter>

QgsUniqueValuesWidget::QgsUniqueValuesWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    :  QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
{
}

QVariant QgsUniqueValuesWidget::value()
{
  QVariant value;

  if ( mComboBox )
    value = mComboBox->itemData( mComboBox->currentIndex() );

  if ( mLineEdit )
    value = mLineEdit->text();

  return value;
}

QWidget* QgsUniqueValuesWidget::createWidget( QWidget* parent )
{
  if ( config( "Editable" ).toBool() )
    return new QLineEdit( parent );
  else
    return new QComboBox( parent );
}

void QgsUniqueValuesWidget::initWidget( QWidget* editor )
{
  mComboBox = qobject_cast<QComboBox*>( editor );
  mLineEdit = qobject_cast<QLineEdit*>( editor );

  QStringList sValues;

  QList<QVariant> values;

  layer()->uniqueValues( fieldIdx(), values );

  Q_FOREACH( QVariant v, values )
  {
    if ( mComboBox )
    {
      mComboBox->addItem( v.toString(), v );
    }

    sValues << v.toString();
  }

  if ( mLineEdit )
  {
    QCompleter* c = new QCompleter( sValues );
    c->setCompletionMode( QCompleter::PopupCompletion );
    mLineEdit->setCompleter( c );

    connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( valueChanged( QString ) ) );
  }

  if ( mComboBox )
  {
    connect( mComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( valueChanged() ) );
  }
}

void QgsUniqueValuesWidget::setValue( const QVariant& value )
{
  if ( mComboBox )
  {
    mComboBox->setCurrentIndex( mComboBox->findData( value ) );
  }

  if ( mLineEdit )
  {
    mLineEdit->setText( value.toString() );
  }
}
