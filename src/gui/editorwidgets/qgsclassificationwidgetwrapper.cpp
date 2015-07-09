/***************************************************************************
    qgsclassificationwidgetwrapper.cpp
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

#include "qgsclassificationwidgetwrapper.h"

#include "qgscategorizedsymbolrendererv2.h"
#include "qgsvectorlayer.h"

QgsClassificationWidgetWrapper::QgsClassificationWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    :  QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mComboBox( NULL )
{
}

QVariant QgsClassificationWidgetWrapper::value()
{
  return mComboBox->itemData( mComboBox->currentIndex() );
}

QWidget*QgsClassificationWidgetWrapper::createWidget( QWidget* parent )
{
  return new QComboBox( parent );
}

void QgsClassificationWidgetWrapper::initWidget( QWidget* editor )
{
  mComboBox = qobject_cast<QComboBox*>( editor );

  if ( mComboBox )
  {
    const QgsCategorizedSymbolRendererV2 *csr = dynamic_cast<const QgsCategorizedSymbolRendererV2 *>( layer()->rendererV2() );
    if ( csr )
    {
      const QgsCategoryList categories = csr->categories();
      for ( int i = 0; i < categories.size(); i++ )
      {
        QString label = categories[i].label();
        QString value = categories[i].value().toString();
        if ( label.isEmpty() )
          label = value;

        mComboBox->addItem( label, value );
      }
    }

    connect( mComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( valueChanged() ) );
  }
}

void QgsClassificationWidgetWrapper::setValue( const QVariant& value )
{
  mComboBox->setCurrentIndex( mComboBox->findData( value ) );
}
