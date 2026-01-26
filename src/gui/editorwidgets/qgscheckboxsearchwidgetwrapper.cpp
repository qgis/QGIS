/***************************************************************************
    qgscheckboxsearchwidgetwrapper.cpp
     ---------------------------------
    Date                 : 2016-05-23
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscheckboxsearchwidgetwrapper.h"

#include "qgscheckboxwidgetfactory.h"
#include "qgsfields.h"
#include "qgsvectorlayer.h"

#include <QCheckBox>
#include <QSettings>

#include "moc_qgscheckboxsearchwidgetwrapper.cpp"

QgsCheckboxSearchWidgetWrapper::QgsCheckboxSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsSearchWidgetWrapper( vl, fieldIdx, parent )

{
}

bool QgsCheckboxSearchWidgetWrapper::applyDirectly()
{
  return true;
}

QString QgsCheckboxSearchWidgetWrapper::expression() const
{
  return mExpression;
}

QVariant QgsCheckboxSearchWidgetWrapper::value() const
{
  QVariant v;

  const QMetaType::Type fieldType = layer()->fields().at( mFieldIdx ).type();

  if ( mCheckBox )
  {
    if ( fieldType == QMetaType::Type::Bool )
    {
      v = mCheckBox->isChecked();
    }
    else
    {
      v = mCheckBox->isChecked() ? config( u"CheckedState"_s, true ) : config( u"UncheckedState"_s, false );
    }
  }

  return v;
}

QgsSearchWidgetWrapper::FilterFlags QgsCheckboxSearchWidgetWrapper::supportedFlags() const
{
  return EqualTo | IsNull | IsNotNull;
}

QgsSearchWidgetWrapper::FilterFlags QgsCheckboxSearchWidgetWrapper::defaultFlags() const
{
  return EqualTo;
}

QString QgsCheckboxSearchWidgetWrapper::createExpression( QgsSearchWidgetWrapper::FilterFlags flags ) const
{
  const QMetaType::Type fldType = layer()->fields().at( mFieldIdx ).type();
  const QString fieldName = createFieldIdentifier();

  //clear any unsupported flags
  flags &= supportedFlags();
  if ( flags & IsNull )
    return fieldName + " IS NULL";

  if ( flags & IsNotNull )
    return fieldName + " IS NOT NULL";

  const QVariant v = value();
  if ( !v.isValid() )
    return QString();

  switch ( fldType )
  {
    case QMetaType::Type::Int:
    case QMetaType::Type::UInt:
    case QMetaType::Type::Double:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::ULongLong:
    case QMetaType::Type::Bool:
    {
      if ( flags & EqualTo )
        return fieldName + '=' + v.toString();
      else if ( flags & NotEqualTo )
        return fieldName + "<>" + v.toString();
      break;
    }

    default:
    {
      if ( flags & EqualTo )
        return fieldName + "='" + v.toString() + '\'';
      else if ( flags & NotEqualTo )
        return fieldName + "<>'" + v.toString() + '\'';
      break;
    }
  }

  return QString();
}

void QgsCheckboxSearchWidgetWrapper::clearWidget()
{
  if ( mCheckBox )
  {
    whileBlocking( mCheckBox )->setCheckState( Qt::PartiallyChecked );
  }
}

void QgsCheckboxSearchWidgetWrapper::setEnabled( bool enabled )
{
  if ( mCheckBox )
  {
    mCheckBox->setEnabled( enabled );
  }
}

bool QgsCheckboxSearchWidgetWrapper::valid() const
{
  return true;
}

void QgsCheckboxSearchWidgetWrapper::setExpression( const QString &expression )
{
  QString exp = expression;
  const QString fieldName = layer()->fields().at( mFieldIdx ).name();
  const QMetaType::Type fieldType = layer()->fields().at( mFieldIdx ).type();

  QString str;
  switch ( fieldType )
  {
    case QMetaType::Type::Bool:
    case QMetaType::Type::Int:
    case QMetaType::Type::UInt:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::ULongLong:
    case QMetaType::Type::Double:
      str = u"%1 = %2"_s.arg( QgsExpression::quotedColumnRef( fieldName ), exp );
      break;

    default:
      str = u"%1 = '%2'"_s
              .arg( QgsExpression::quotedColumnRef( fieldName ), exp.replace( '\'', "''"_L1 ) );
      break;
  }
  mExpression = str;
}

void QgsCheckboxSearchWidgetWrapper::stateChanged( int )
{
  if ( mCheckBox )
  {
    mCheckBox->setTristate( false );

    QString exp;
    const QVariant currentValue = value();
    if ( currentValue.userType() == QMetaType::Type::Bool )
    {
      exp = currentValue.toBool() ? u"TRUE"_s : u"FALSE"_s;
    }
    else
    {
      exp = currentValue.toString();
    }

    setExpression( exp );
    emit valueChanged();
    emit expressionChanged( mExpression );
  }
}

QWidget *QgsCheckboxSearchWidgetWrapper::createWidget( QWidget *parent )
{
  QCheckBox *c = new QCheckBox( parent );
  c->setChecked( Qt::PartiallyChecked );
  return c;
}

void QgsCheckboxSearchWidgetWrapper::initWidget( QWidget *editor )
{
  mCheckBox = qobject_cast<QCheckBox *>( editor );

  if ( mCheckBox )
  {
    mCheckBox->setChecked( Qt::PartiallyChecked );
    connect( mCheckBox, &QCheckBox::stateChanged, this, &QgsCheckboxSearchWidgetWrapper::stateChanged );
  }
}
