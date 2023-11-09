/***************************************************************************
    qgseditorconfigwidget.cpp
     --------------------------------------
    Date                 : 24.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgseditorconfigwidget.h"
#include "qgspropertyoverridebutton.h"
#include "qgsexpressioncontextutils.h"

QgsEditorConfigWidget::QgsEditorConfigWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QWidget( parent )
  , mLayer( vl )
  , mField( fieldIdx )

{
}

int QgsEditorConfigWidget::field()
{
  return mField;
}

QgsVectorLayer *QgsEditorConfigWidget::layer()
{
  return mLayer;
}

QgsExpressionContext QgsEditorConfigWidget::createExpressionContext() const
{
  return QgsExpressionContext( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
}

void QgsEditorConfigWidget::initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsWidgetWrapper::Property key )
{
  button->blockSignals( true );
  button->init( key, mPropertyCollection, QgsWidgetWrapper::propertyDefinitions(), mLayer );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsEditorConfigWidget::updateProperty );
  button->registerExpressionContextGenerator( this );
  button->blockSignals( false );
}

void QgsEditorConfigWidget::updateDataDefinedButtons()
{
  const auto propertyOverrideButtons { findChildren< QgsPropertyOverrideButton * >() };
  for ( QgsPropertyOverrideButton *button : propertyOverrideButtons )
  {
    updateDataDefinedButton( button );
  }
}

void QgsEditorConfigWidget::updateDataDefinedButton( QgsPropertyOverrideButton *button )
{
  if ( !button )
    return;

  if ( button->propertyKey() < 0 )
    return;

  const QgsWidgetWrapper::Property key = static_cast< QgsWidgetWrapper::Property >( button->propertyKey() );
  whileBlocking( button )->setToProperty( mPropertyCollection.property( key ) );
}

void QgsEditorConfigWidget::updateProperty()
{
  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  const QgsWidgetWrapper::Property key = static_cast<  QgsWidgetWrapper::Property >( button->propertyKey() );
  mPropertyCollection.setProperty( key, button->toProperty() );
  emit changed();
}

