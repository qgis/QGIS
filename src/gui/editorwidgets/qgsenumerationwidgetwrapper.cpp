/***************************************************************************
    qgsenumerationwidgetwrapper.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsenumerationwidgetwrapper.h"

#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

QgsEnumerationWidgetWrapper::QgsEnumerationWidgetWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )

{
}


QVariant QgsEnumerationWidgetWrapper::value() const
{
  QVariant value;

  if ( mComboBox )
    value = mComboBox->currentData();

  return value;
}

void QgsEnumerationWidgetWrapper::showIndeterminateState()
{
  if ( mComboBox )
  {
    whileBlocking( mComboBox )->setCurrentIndex( -1 );
  }
}

QWidget *QgsEnumerationWidgetWrapper::createWidget( QWidget *parent )
{
  return new QComboBox( parent );
}

void QgsEnumerationWidgetWrapper::initWidget( QWidget *editor )
{
  mComboBox = qobject_cast<QComboBox *>( editor );

  if ( mComboBox )
  {
    QStringList enumValues;
    layer()->dataProvider()->enumValues( fieldIdx(), enumValues );

    const auto constEnumValues = enumValues;
    for ( const QString &s : constEnumValues )
    {
      mComboBox->addItem( s, s );
    }
    connect( mComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
             this, static_cast<void ( QgsEditorWidgetWrapper::* )()>( &QgsEditorWidgetWrapper::emitValueChanged ) );
  }
}

bool QgsEnumerationWidgetWrapper::valid() const
{
  return mComboBox;
}

void QgsEnumerationWidgetWrapper::updateValues( const QVariant &value, const QVariantList & )
{
  if ( mComboBox )
  {
    mComboBox->setCurrentIndex( mComboBox->findData( value ) );
  }
}

