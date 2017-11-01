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
  , mValidConstraint( true )
  , mIsBlockingCommit( false )
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
  mFeature = feature;
  setValue( feature.attribute( mFieldIdx ) );
}

void QgsEditorWidgetWrapper::valueChanged( const QString &value )
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

void QgsEditorWidgetWrapper::updateConstraintWidgetStatus( ConstraintResult constraintResult )
{
  switch ( constraintResult )
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

  QgsField field = layer->fields().at( index );
  QString expression = field.constraints().constraintExpression();

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

  mConstraintFailureReason = errors.join( QStringLiteral( ", " ) );

  if ( toEmit )
  {
    QString errStr = errors.isEmpty() ? tr( "Constraint checks passed" ) : mConstraintFailureReason;

    QString description = descriptions.join( QStringLiteral( ", " ) );
    QString expressionDesc;
    if ( expressions.size() > 1 )
      expressionDesc = "( " + expressions.join( QStringLiteral( " ) AND ( " ) ) + " )";
    else if ( !expressions.isEmpty() )
      expressionDesc = expressions.at( 0 );

    ConstraintResult result = !hardConstraintsOk ? ConstraintResultFailHard
                              : ( !softConstraintsOk ? ConstraintResultFailSoft : ConstraintResultPass );
    updateConstraintWidgetStatus( result );
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
  widget()->setToolTip( hintText );
}
