/***************************************************************************
    qgseditorwidgetwrapper.cpp
     --------------------------------------
    Date                 : 20.4.2013
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

#include "qgseditorwidgetwrapper.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsfield.h"

#include <QWidget>

QgsEditorWidgetWrapper::QgsEditorWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    : QgsWidgetWrapper( vl, editor, parent )
    , mValidConstraint( true )
    , mFieldIdx( fieldIdx )
{
}

int QgsEditorWidgetWrapper::fieldIdx() const
{
  return mFieldIdx;
}

QgsField QgsEditorWidgetWrapper::field() const
{
  if ( mFieldIdx < layer()->fields().count() )
    return layer()->fields().at( mFieldIdx );
  else
    return QgsField();
}

QVariant QgsEditorWidgetWrapper::defaultValue() const
{
  mDefaultValue = layer()->dataProvider()->defaultValue( mFieldIdx );

  return mDefaultValue;
}

QgsEditorWidgetWrapper* QgsEditorWidgetWrapper::fromWidget( QWidget* widget )
{
  return qobject_cast<QgsEditorWidgetWrapper*>( widget->property( "EWV2Wrapper" ).value<QgsWidgetWrapper*>() );
}

void QgsEditorWidgetWrapper::setEnabled( bool enabled )
{
  QWidget* wdg = widget();
  if ( wdg )
  {
    wdg->setEnabled( enabled );
  }
}

void QgsEditorWidgetWrapper::setFeature( const QgsFeature& feature )
{
  mFeature = feature;
  setValue( feature.attribute( mFieldIdx ) );
}

void QgsEditorWidgetWrapper::valueChanged( const QString& value )
{
  emit valueChanged( QVariant( value ) );
}

void QgsEditorWidgetWrapper::valueChanged( int value )
{
  emit valueChanged( QVariant( value ) );
}

void QgsEditorWidgetWrapper::valueChanged( double value )
{
  emit valueChanged( QVariant( value ) );
}

void QgsEditorWidgetWrapper::valueChanged( bool value )
{
  emit valueChanged( QVariant( value ) );
}

void QgsEditorWidgetWrapper::valueChanged( qlonglong value )
{
  emit valueChanged( QVariant( value ) );
}

void QgsEditorWidgetWrapper::valueChanged()
{
  emit valueChanged( value() );
}

void QgsEditorWidgetWrapper::updateConstraintWidgetStatus( bool constraintValid )
{
  if ( constraintValid )
    widget()->setStyleSheet( QString() );
  else
    widget()->setStyleSheet( "background-color: #dd7777;" );
}

void QgsEditorWidgetWrapper::updateConstraint( const QgsFeature &ft )
{
  bool toEmit( false );
  QString errStr( tr( "predicate is True" ) );
  QString expression = layer()->editFormConfig()->expression( mFieldIdx );
  QString description;
  QVariant value = ft.attribute( mFieldIdx );

  if ( ! expression.isEmpty() )
  {
    description = layer()->editFormConfig()->expressionDescription( mFieldIdx );

    QgsExpressionContext context =
      QgsExpressionContextUtils::createFeatureBasedContext( ft, *ft.fields() );
    context << QgsExpressionContextUtils::layerScope( layer() );

    context.setFeature( ft );
    QgsExpression expr( expression );

    mValidConstraint = expr.evaluate( &context ).toBool();

    if ( expr.hasParserError() )
      errStr = expr.parserErrorString();
    else if ( expr.hasEvalError() )
      errStr = expr.evalErrorString();
    else if ( ! mValidConstraint )
      errStr = tr( "predicate is False" );

    toEmit = true;
  }
  else
    mValidConstraint = true;

  if ( layer()->editFormConfig()->notNull( mFieldIdx ) )
  {
    if ( !expression.isEmpty() )
    {
      QString fieldName = ft.fields()->field( mFieldIdx ).name();
      expression = "( " + expression + " ) AND ( " + fieldName + " IS NOT NULL)";
      description = "( " + description + " ) AND NotNull";
    }
    else
    {
      description = "NotNull";
      expression = "NotNull";
    }

    mValidConstraint = mValidConstraint && !value.isNull();

    if ( value.isNull() )
      errStr = tr( "predicate is False" );

    toEmit = true;
  }

  if ( toEmit )
  {
    updateConstraintWidgetStatus( mValidConstraint );
    emit constraintStatusChanged( expression, description, errStr, mValidConstraint );
  }
}

bool QgsEditorWidgetWrapper::isValidConstraint() const
{
  return mValidConstraint;
}
