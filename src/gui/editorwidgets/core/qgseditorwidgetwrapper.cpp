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
#include "qgsfields.h"
#include "qgsvectorlayerutils.h"

#include <QTableView>

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
    widget()->setStyleSheet( QStringLiteral( "background-color: #dd7777;" ) );
}

void QgsEditorWidgetWrapper::updateConstraint( const QgsFeature &ft )
{
  bool toEmit( false );
  QgsField field = layer()->fields().at( mFieldIdx );

  QString expression = field.constraintExpression();
  QStringList expressions, descriptions;
  QVariant value = ft.attribute( mFieldIdx );

  mConstraintFailureReason.clear();

  QStringList errors;

  if ( ! expression.isEmpty() )
  {
    expressions << expression;
    descriptions << field.constraintDescription();

    QgsExpressionContext context = layer()->createExpressionContext();
    context.setFeature( ft );

    QgsExpression expr( expression );

    mValidConstraint = expr.evaluate( &context ).toBool();

    if ( expr.hasParserError() )
    {
      errors << tr( "Parser error: %1" ).arg( expr.parserErrorString() );
    }
    else if ( expr.hasEvalError() )
    {
      errors << tr( "Evaluation error: %1" ).arg( expr.evalErrorString() );
    }
    else if ( ! mValidConstraint )
    {
      errors << tr( "%1 check failed" ).arg( field.constraintDescription() );
    }

    toEmit = true;
  }
  else
    mValidConstraint = true;

  if ( field.constraints() & QgsField::ConstraintNotNull )
  {
    descriptions << QStringLiteral( "NotNull" );
    if ( !expression.isEmpty() )
    {
      expressions << field.name() + " IS NOT NULL";
    }
    else
    {
      expressions << QStringLiteral( "NotNull" );
    }

    mValidConstraint = mValidConstraint && !value.isNull();

    if ( value.isNull() )
    {
      errors << tr( "Value is NULL" );
    }

    toEmit = true;
  }

  if ( field.constraints() & QgsField::ConstraintUnique )
  {
    descriptions << QStringLiteral( "Unique" );
    if ( !expression.isEmpty() )
    {
      expressions << field.name() + " IS UNIQUE";
    }
    else
    {
      expression = QStringLiteral( "Unique" );
    }

    bool alreadyExists = QgsVectorLayerUtils::valueExists( layer(), mFieldIdx, value, QgsFeatureIds() << ft.id() );
    mValidConstraint = mValidConstraint && !alreadyExists;

    if ( alreadyExists )
    {
      errors << tr( "Value is not unique" );
    }

    toEmit = true;
  }

  if ( toEmit )
  {
    QString errStr = errors.isEmpty() ? tr( "Constraint checks passed" ) : errors.join( '\n' );
    mConstraintFailureReason = errors.join( ", " );
    QString description;
    if ( descriptions.size() > 1 )
      description = "( " + descriptions.join( " ) AND ( " ) + " )";
    else if ( !descriptions.isEmpty() )
      description = descriptions.at( 0 );
    QString expressionDesc;
    if ( expressions.size() > 1 )
      expressionDesc = "( " + expressions.join( " ) AND ( " ) + " )";
    else if ( !expressions.isEmpty() )
      expressionDesc = expressions.at( 0 );

    updateConstraintWidgetStatus( mValidConstraint );
    emit constraintStatusChanged( expressionDesc, description, errStr, mValidConstraint );
  }
}

bool QgsEditorWidgetWrapper::isValidConstraint() const
{
  return mValidConstraint;
}

QString QgsEditorWidgetWrapper::constraintFailureReason() const
{
  return mConstraintFailureReason;
}

bool QgsEditorWidgetWrapper::isInTable( const QWidget* parent )
{
  if ( !parent ) return false;
  if ( qobject_cast<const QTableView*>( parent ) ) return true;
  return isInTable( parent->parentWidget() );
}
