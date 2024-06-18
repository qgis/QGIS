/***************************************************************************
    qgsdefaultsearchwidgettwrapper.cpp
     --------------------------------------
    Date                 : 31.5.2015
    Copyright            : (C) 2015 Karolina Alexiou (carolinux)
    Email                : carolinegr at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdefaultsearchwidgetwrapper.h"

#include "qgsfields.h"
#include "qgsfieldvalidator.h"
#include "qgsexpression.h"
#include "qgsfieldvalueslineedit.h"
#include "qgssettings.h"
#include "qgsapplication.h"

#include <QHBoxLayout>

QgsDefaultSearchWidgetWrapper::QgsDefaultSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsSearchWidgetWrapper( vl, fieldIdx, parent )
  , mCaseString( QStringLiteral( "LIKE" ) )
{
}

QString QgsDefaultSearchWidgetWrapper::expression() const
{
  return mExpression;
}

void QgsDefaultSearchWidgetWrapper::setCaseString( int caseSensitiveCheckState )
{
  if ( caseSensitiveCheckState == Qt::Checked )
  {
    mCaseString = QStringLiteral( "LIKE" );
  }
  else
  {
    mCaseString = QStringLiteral( "ILIKE" );
  }
  // need to update also the line edit
  setExpression( mLineEdit->text() );

  if ( applyDirectly() )
    emit expressionChanged( mExpression );
}

void QgsDefaultSearchWidgetWrapper::setExpression( const QString &expression )
{
  const QMetaType::Type fldType = layer()->fields().at( mFieldIdx ).type();
  const bool numeric = ( fldType == QMetaType::Type::Int || fldType == QMetaType::Type::Double || fldType == QMetaType::Type::LongLong );

  QString exp = expression;
  const QString nullValue = QgsApplication::nullRepresentation();
  const QString fieldName = layer()->fields().at( mFieldIdx ).name();
  QString str;
  if ( exp == nullValue )
  {
    str = QStringLiteral( "%1 IS NULL" ).arg( QgsExpression::quotedColumnRef( fieldName ) );
  }
  else
  {
    str = QStringLiteral( "%1 %2 '%3'" )
          .arg( QgsExpression::quotedColumnRef( fieldName ),
                numeric ? QStringLiteral( "=" ) : mCaseString,
                numeric ?
                exp.replace( '\'', QLatin1String( "''" ) )
                :
                '%' + exp.replace( '\'', QLatin1String( "''" ) ) + '%' ); // escape quotes
  }
  mExpression = str;
}

QWidget *QgsDefaultSearchWidgetWrapper::createWidget( QWidget *parent )
{
  return new QWidget( parent );
}

bool QgsDefaultSearchWidgetWrapper::applyDirectly()
{
  return false;
}

QgsSearchWidgetWrapper::FilterFlags QgsDefaultSearchWidgetWrapper::supportedFlags() const
{
  FilterFlags flags = EqualTo | NotEqualTo | IsNull | IsNotNull;

  const QMetaType::Type fldType = layer()->fields().at( mFieldIdx ).type();
  switch ( fldType )
  {
    case QMetaType::Type::Int:
    case QMetaType::Type::UInt:
    case QMetaType::Type::Double:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::ULongLong:
    case QMetaType::Type::QDate:
    case QMetaType::Type::QDateTime:
    case QMetaType::Type::QTime:
      flags |= GreaterThan | LessThan | GreaterThanOrEqualTo | LessThanOrEqualTo | Between | IsNotBetween;
      break;

    case QMetaType::Type::QString:
      flags |= Contains | DoesNotContain | StartsWith | EndsWith;
      break;

    default:
      break;
  }
  return flags;
}

QgsSearchWidgetWrapper::FilterFlags QgsDefaultSearchWidgetWrapper::defaultFlags() const
{
  const QMetaType::Type fldType = layer()->fields().at( mFieldIdx ).type();
  switch ( fldType )
  {
    //numeric
    case QMetaType::Type::Int:
    case QMetaType::Type::UInt:
    case QMetaType::Type::Double:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::ULongLong:

    // date
    case QMetaType::Type::QDate:
    case QMetaType::Type::QDateTime:
    case QMetaType::Type::QTime:
      return EqualTo;

    case QMetaType::Type::QString:
      return Contains;

    default:
      break;
  }
  return EqualTo;
}

QString QgsDefaultSearchWidgetWrapper::createExpression( QgsSearchWidgetWrapper::FilterFlags flags ) const
{
  //clear any unsupported flags
  flags &= supportedFlags();

  const QMetaType::Type fldType = layer()->fields().at( mFieldIdx ).type();
  const QString fieldName = createFieldIdentifier();

  if ( flags & IsNull )
    return fieldName + " IS NULL";
  if ( flags & IsNotNull )
    return fieldName + " IS NOT NULL";

  switch ( fldType )
  {
    case QMetaType::Type::Int:
    case QMetaType::Type::UInt:
    case QMetaType::Type::Double:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::ULongLong:
    {
      if ( flags & EqualTo )
        return fieldName + '=' + mLineEdit->text();
      else if ( flags & NotEqualTo )
        return fieldName + "<>" + mLineEdit->text();
      else if ( flags & GreaterThan )
        return fieldName + '>' + mLineEdit->text();
      else if ( flags & LessThan )
        return fieldName + '<' + mLineEdit->text();
      else if ( flags & GreaterThanOrEqualTo )
        return fieldName + ">=" + mLineEdit->text();
      else if ( flags & LessThanOrEqualTo )
        return fieldName + "<=" + mLineEdit->text();
      break;
    }

    case QMetaType::Type::QDate:
    case QMetaType::Type::QDateTime:
    case QMetaType::Type::QTime:
    {
      if ( flags & EqualTo )
        return fieldName + "='" + mLineEdit->text() + '\'';
      else if ( flags & NotEqualTo )
        return fieldName + "<>'" + mLineEdit->text() + '\'';
      else if ( flags & GreaterThan )
        return fieldName + ">'" + mLineEdit->text() + '\'';
      else if ( flags & LessThan )
        return fieldName + "<'" + mLineEdit->text() + '\'';
      else if ( flags & GreaterThanOrEqualTo )
        return fieldName + ">='" + mLineEdit->text() + '\'';
      else if ( flags & LessThanOrEqualTo )
        return fieldName + "<='" + mLineEdit->text() + '\'';
      break;
    }

    case QMetaType::Type::QString:
    {
      // case insensitive!
      if ( flags & EqualTo || flags & NotEqualTo )
      {
        if ( mCheckbox && mCheckbox->isChecked() )
          return fieldName + ( ( flags & EqualTo ) ? "=" : "<>" )
                 + QgsExpression::quotedString( mLineEdit->text() );
        else
          return QStringLiteral( "lower(%1)" ).arg( fieldName )
                 + ( ( flags & EqualTo ) ? "=" : "<>" ) +
                 QStringLiteral( "lower(%1)" ).arg( QgsExpression::quotedString( mLineEdit->text() ) );
      }
      else if ( flags & Contains || flags & DoesNotContain || flags & StartsWith || flags & EndsWith )
      {
        QString exp = fieldName + ( mCheckbox && mCheckbox->isChecked() ? " LIKE " : " ILIKE " );
        QString value = QgsExpression::quotedString( mLineEdit->text() );
        value.chop( 1 );
        value = value.remove( 0, 1 );
        exp += '\'';
        if ( !flags.testFlag( StartsWith ) )
          exp += '%';
        exp += value;
        if ( !flags.testFlag( EndsWith ) )
          exp += '%';
        exp += '\'';
        if ( flags & DoesNotContain )
          exp.prepend( "NOT (" ).append( ')' );
        return exp;
      }

      break;
    }

    default:
      break;
  }

  return QString();
}

void QgsDefaultSearchWidgetWrapper::clearWidget()
{
  mLineEdit->setText( QString() );
}

void QgsDefaultSearchWidgetWrapper::setEnabled( bool enabled )
{
  mLineEdit->setEnabled( enabled );
  if ( mCheckbox )
    mCheckbox->setEnabled( enabled );
}

void QgsDefaultSearchWidgetWrapper::initWidget( QWidget *widget )
{
  mContainer = widget;
  mContainer->setLayout( new QHBoxLayout() );
  mContainer->layout()->setContentsMargins( 0, 0, 0, 0 );
  const QMetaType::Type fldType = layer()->fields().at( mFieldIdx ).type();

  if ( fldType == QMetaType::Type::QString )
  {
    mLineEdit = new QgsFieldValuesLineEdit();
    static_cast< QgsFieldValuesLineEdit * >( mLineEdit )->setLayer( layer() );
    static_cast< QgsFieldValuesLineEdit * >( mLineEdit )->setAttributeIndex( mFieldIdx );
  }
  else
  {
    mLineEdit = new QgsFilterLineEdit();
  }
  mContainer->layout()->addWidget( mLineEdit );
  mContainer->setFocusProxy( mLineEdit );

  if ( fldType == QMetaType::Type::QString )
  {
    mCheckbox = new QCheckBox( QStringLiteral( "Case sensitive" ) );
    mContainer->layout()->addWidget( mCheckbox );
    connect( mCheckbox, &QCheckBox::stateChanged, this, &QgsDefaultSearchWidgetWrapper::setCaseString );
    mCheckbox->setChecked( Qt::Unchecked );
  }

  connect( mLineEdit, &QLineEdit::textChanged, this, &QgsDefaultSearchWidgetWrapper::textChanged );
  connect( mLineEdit, &QLineEdit::returnPressed, this, &QgsDefaultSearchWidgetWrapper::filterChanged );
  connect( mLineEdit, &QLineEdit::textEdited, this, &QgsSearchWidgetWrapper::valueChanged );

  mCaseString = QStringLiteral( "ILIKE" );
}

bool QgsDefaultSearchWidgetWrapper::valid() const
{
  return true;
}

QgsFilterLineEdit *QgsDefaultSearchWidgetWrapper::lineEdit()
{
  return mLineEdit;
}

QCheckBox *QgsDefaultSearchWidgetWrapper::caseSensitiveCheckBox()
{
  return mCheckbox;
}

void QgsDefaultSearchWidgetWrapper::filterChanged()
{
  emit expressionChanged( mExpression );
}

void QgsDefaultSearchWidgetWrapper::textChanged( const QString &text )
{
  if ( text.isEmpty() )
    emit valueCleared();

  setExpression( text );
}
