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

#include "qgsfield.h"
#include "qgscheckboxwidgetfactory.h"
#include "qgsvectorlayer.h"

#include <QSettings>
#include <QCheckBox>

QgsCheckboxSearchWidgetWrapper::QgsCheckboxSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsSearchWidgetWrapper( vl, fieldIdx, parent )
    , mCheckBox( nullptr )
    , mLayer( nullptr )
{
}

bool QgsCheckboxSearchWidgetWrapper::applyDirectly()
{
  return true;
}

QString QgsCheckboxSearchWidgetWrapper::expression()
{
  return mExpression;
}

QVariant QgsCheckboxSearchWidgetWrapper::value() const
{
  QVariant v;

  if ( mCheckBox )
    v = mCheckBox->isChecked() ? config( "CheckedState" ) : config( "UncheckedState" );

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
  QVariant::Type fldType = layer()->fields().at( mFieldIdx ).type();
  QString fieldName = QgsExpression::quotedColumnRef( layer()->fields().at( mFieldIdx ).name() );

  //clear any unsupported flags
  flags &= supportedFlags();
  if ( flags & IsNull )
    return fieldName + " IS NULL";

  if ( flags & IsNotNull )
    return fieldName + " IS NOT NULL";

  QVariant v = value();
  if ( !v.isValid() )
    return QString();

  switch ( fldType )
  {
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Double:
    case QVariant::LongLong:
    case QVariant::ULongLong:
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

void QgsCheckboxSearchWidgetWrapper::setExpression( QString exp )
{
  QString fieldName = layer()->fields().at( mFieldIdx ).name();

  QString str = QString( "%1 = '%3'" )
                .arg( QgsExpression::quotedColumnRef( fieldName ),
                      exp.replace( '\'', "''" )
                    );
  mExpression = str;
}

void QgsCheckboxSearchWidgetWrapper::stateChanged( int )
{
  if ( mCheckBox )
  {
    mCheckBox->setTristate( false );
    QString exp = value().toString();
    setExpression( exp );
    emit valueChanged();
    emit expressionChanged( mExpression );
  }
}

QWidget* QgsCheckboxSearchWidgetWrapper::createWidget( QWidget* parent )
{
  QCheckBox* c = new QCheckBox( parent );
  c->setChecked( Qt::PartiallyChecked );
  return c;
}

void QgsCheckboxSearchWidgetWrapper::initWidget( QWidget* editor )
{
  mCheckBox = qobject_cast<QCheckBox*>( editor );

  if ( mCheckBox )
  {
    mCheckBox->setChecked( Qt::PartiallyChecked );
    connect( mCheckBox, SIGNAL( stateChanged( int ) ), this, SLOT( stateChanged( int ) ) );
  }
}


