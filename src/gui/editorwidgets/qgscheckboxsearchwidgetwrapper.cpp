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

#include "qgsfields.h"
#include "qgscheckboxwidgetfactory.h"
#include "qgsvectorlayer.h"

#include <QSettings>
#include <QCheckBox>

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

  if ( mCheckBox )
    v = mCheckBox->isChecked() ? config( QStringLiteral( "CheckedState" ), true ) : config( QStringLiteral( "UncheckedState" ), false );

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
  const QVariant::Type fldType = layer()->fields().at( mFieldIdx ).type();
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
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Double:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Bool:
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

  const QString str = QStringLiteral( "%1 = '%3'" )
                      .arg( QgsExpression::quotedColumnRef( fieldName ),
                            exp.replace( '\'', QLatin1String( "''" ) )
                          );
  mExpression = str;
}

void QgsCheckboxSearchWidgetWrapper::stateChanged( int )
{
  if ( mCheckBox )
  {
    mCheckBox->setTristate( false );
    const QString exp = value().toString();
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


