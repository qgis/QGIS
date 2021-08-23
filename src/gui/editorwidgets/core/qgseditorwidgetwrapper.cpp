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
#include "qgsvectorlayerjoinbuffer.h"
#include "qgsvectorlayerjoininfo.h"

#include <QTableView>

QgsEditorWidgetWrapper::QgsEditorWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsWidgetWrapper( vl, editor, parent )
  , mFieldIdx( fieldIdx )
  , mValidConstraint( true )
  , mIsBlockingCommit( false )
{
}

int QgsEditorWidgetWrapper::fieldIdx() const
{
  return mFieldIdx;
}

QgsField QgsEditorWidgetWrapper::field() const
{
  QgsVectorLayer *vl = layer();
  if ( vl && mFieldIdx < vl->fields().count() )
    return vl->fields().at( mFieldIdx );
  else
    return QgsField();
}

QVariant QgsEditorWidgetWrapper::defaultValue() const
{
  mDefaultValue = layer()->dataProvider()->defaultValueClause( mFieldIdx );

  return mDefaultValue;
}

QgsEditorWidgetWrapper *QgsEditorWidgetWrapper::fromWidget( QWidget *widget )
{
  return qobject_cast<QgsEditorWidgetWrapper *>( widget->property( "EWV2Wrapper" ).value<QgsWidgetWrapper *>() );
}

void QgsEditorWidgetWrapper::setEnabled( bool enabled )
{
  QWidget *wdg = widget();
  if ( wdg )
  {
    wdg->setEnabled( enabled );
  }
}

void QgsEditorWidgetWrapper::setFeature( const QgsFeature &feature )
{
  setFormFeature( feature );
  QVariantList newAdditionalFieldValues;
  const QStringList constAdditionalFields = additionalFields();
  for ( const QString &fieldName : constAdditionalFields )
    newAdditionalFieldValues << feature.attribute( fieldName );
  setValues( feature.attribute( mFieldIdx ), newAdditionalFieldValues );
}

void QgsEditorWidgetWrapper::setValue( const QVariant &value )
{
  isRunningDeprecatedSetValue = true;
  updateValues( value, QVariantList() );
  isRunningDeprecatedSetValue = false;
}

void QgsEditorWidgetWrapper::setValues( const QVariant &value, const QVariantList &additionalValues )
{
  updateValues( value, additionalValues );
}

void QgsEditorWidgetWrapper::emitValueChanged()
{
  Q_NOWARN_DEPRECATED_PUSH
  emit valueChanged( value() );
  Q_NOWARN_DEPRECATED_POP
  emit valuesChanged( value(), additionalFieldValues() );
}

void QgsEditorWidgetWrapper::parentFormValueChanged( const QString &attribute, const QVariant &value )
{
  Q_UNUSED( attribute )
  Q_UNUSED( value )
}

void QgsEditorWidgetWrapper::updateConstraintWidgetStatus()
{
  if ( !mConstraintResultVisible )
  {
    widget()->setStyleSheet( QString() );
  }
  else
  {
    switch ( mConstraintResult )
    {
      case ConstraintResultPass:
        widget()->setStyleSheet( QString() );
        break;

      case ConstraintResultFailHard:
        widget()->setStyleSheet( QStringLiteral( "background-color: #FFE0B2;" ) );
        break;

      case ConstraintResultFailSoft:
        widget()->setStyleSheet( QStringLiteral( "background-color: #FFECB3;" ) );
        break;
    }
  }
}

bool QgsEditorWidgetWrapper::setFormFeatureAttribute( const QString &attributeName, const QVariant &attributeValue )
{
  return mFormFeature.setAttribute( attributeName, attributeValue );
}

void QgsEditorWidgetWrapper::updateValues( const QVariant &value, const QVariantList &additionalValues )
{
  // this method should be made pure virtual in QGIS 4
  Q_UNUSED( additionalValues );
  Q_NOWARN_DEPRECATED_PUSH
  // avoid infinite recursive loop
  if ( !isRunningDeprecatedSetValue )
    setValue( value );
  Q_NOWARN_DEPRECATED_POP
}

QgsEditorWidgetWrapper::ConstraintResult QgsEditorWidgetWrapper::constraintResult() const
{
  return mConstraintResult;
}

bool QgsEditorWidgetWrapper::constraintResultVisible() const
{
  return mConstraintResultVisible;
}

void QgsEditorWidgetWrapper::setConstraintResultVisible( bool constraintResultVisible )
{
  if ( mConstraintResultVisible == constraintResultVisible )
    return;

  mConstraintResultVisible = constraintResultVisible;

  updateConstraintWidgetStatus();

  emit constraintResultVisibleChanged( mConstraintResultVisible );
}

void QgsEditorWidgetWrapper::updateConstraint( const QgsFeature &ft, QgsFieldConstraints::ConstraintOrigin constraintOrigin )
{
  updateConstraint( layer(), mFieldIdx, ft, constraintOrigin );
}

void QgsEditorWidgetWrapper::updateConstraint( const QgsVectorLayer *layer, int index, const QgsFeature &ft, QgsFieldConstraints::ConstraintOrigin constraintOrigin )
{
  QStringList errors;
  QStringList softErrors;
  QStringList expressions;
  QStringList descriptions;
  bool toEmit( false );
  bool hardConstraintsOk( true );
  bool softConstraintsOk( true );

  const QgsField field = layer->fields().at( index );
  const QString expression = field.constraints().constraintExpression();

  if ( ft.isValid() )
  {
    if ( ! expression.isEmpty() )
    {
      expressions << expression;
      descriptions << field.constraints().constraintDescription();
      toEmit = true;
    }

    if ( field.constraints().constraints() & QgsFieldConstraints::ConstraintNotNull )
    {
      descriptions << tr( "Not NULL" );
      if ( !expression.isEmpty() )
      {
        expressions << field.name() + QStringLiteral( " IS NOT NULL" );
      }
      else
      {
        expressions << QStringLiteral( "IS NOT NULL" );
      }
      toEmit = true;
    }

    if ( field.constraints().constraints() & QgsFieldConstraints::ConstraintUnique )
    {
      descriptions << tr( "Unique" );
      if ( !expression.isEmpty() )
      {
        expressions << field.name() + QStringLiteral( " IS UNIQUE" );
      }
      else
      {
        expressions << QStringLiteral( "IS UNIQUE" );
      }
      toEmit = true;
    }

    hardConstraintsOk = QgsVectorLayerUtils::validateAttribute( layer, ft, index, errors, QgsFieldConstraints::ConstraintStrengthHard, constraintOrigin );

    softConstraintsOk = QgsVectorLayerUtils::validateAttribute( layer, ft, index, softErrors, QgsFieldConstraints::ConstraintStrengthSoft, constraintOrigin );
    errors << softErrors;
  }
  else // invalid feature
  {
    if ( ! expression.isEmpty() )
    {
      hardConstraintsOk = true;
      softConstraintsOk = false;

      errors << QStringLiteral( "Invalid feature" );

      toEmit = true;
    }
  }

  mValidConstraint = hardConstraintsOk && softConstraintsOk;
  mIsBlockingCommit = !hardConstraintsOk;

  mConstraintFailureReason = errors.join( QLatin1String( ", " ) );

  if ( toEmit )
  {
    const QString errStr = errors.isEmpty() ? tr( "Constraint checks passed" ) : mConstraintFailureReason;

    const QString description = descriptions.join( QLatin1String( ", " ) );
    QString expressionDesc;
    if ( expressions.size() > 1 )
      expressionDesc = "( " + expressions.join( QLatin1String( " ) AND ( " ) ) + " )";
    else if ( !expressions.isEmpty() )
      expressionDesc = expressions.at( 0 );

    const ConstraintResult result = !hardConstraintsOk ? ConstraintResultFailHard
                                    : ( !softConstraintsOk ? ConstraintResultFailSoft : ConstraintResultPass );
    //set the constraint result
    mConstraintResult = result;
    updateConstraintWidgetStatus();
    emit constraintStatusChanged( expressionDesc, description, errStr, result );
  }
}

bool QgsEditorWidgetWrapper::isValidConstraint() const
{
  return mValidConstraint;
}

bool QgsEditorWidgetWrapper::isBlockingCommit() const
{
  return mIsBlockingCommit;
}


QString QgsEditorWidgetWrapper::constraintFailureReason() const
{
  return mConstraintFailureReason;
}

bool QgsEditorWidgetWrapper::isInTable( const QWidget *parent )
{
  if ( !parent ) return false;
  if ( qobject_cast<const QTableView *>( parent ) ) return true;
  return isInTable( parent->parentWidget() );
}

void QgsEditorWidgetWrapper::setHint( const QString &hintText )
{
  if ( QWidget *w = widget() )
    w->setToolTip( hintText );
}
