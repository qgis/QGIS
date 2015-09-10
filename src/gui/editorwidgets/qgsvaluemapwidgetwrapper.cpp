/***************************************************************************
    qgsvaluemapwidgetwrapper.cpp
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

#include "qgsvaluemapwidgetwrapper.h"

QgsValueMapWidgetWrapper::QgsValueMapWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mComboBox( NULL )
{
}


QVariant QgsValueMapWidgetWrapper::value()
{
  QVariant v;

  if ( mComboBox )
    v = mComboBox->itemData( mComboBox->currentIndex() );

  return v;
}

QWidget* QgsValueMapWidgetWrapper::createWidget( QWidget* parent )
{
  return new QComboBox( parent );
}

void QgsValueMapWidgetWrapper::initWidget( QWidget* editor )
{
  mComboBox = qobject_cast<QComboBox*>( editor );

  if ( mComboBox )
  {
    const QgsEditorWidgetConfig cfg = config();
    QgsEditorWidgetConfig::ConstIterator it = cfg.constBegin();

    while ( it != cfg.constEnd() )
    {
      mComboBox->addItem( it.key(), it.value() );
      ++it;
    }
    connect( mComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( valueChanged() ) );
  }
}

bool QgsValueMapWidgetWrapper::valid()
{
  return mComboBox;
}

void QgsValueMapWidgetWrapper::setValue( const QVariant& value )
{
  if ( mComboBox )
    mComboBox->setCurrentIndex( mComboBox->findData( value ) );
}
