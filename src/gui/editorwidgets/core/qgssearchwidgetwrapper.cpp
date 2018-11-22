/***************************************************************************
    qgssearchwidgetwrapper.cpp
     --------------------------------------
    Date                 : 10.6.2015
    Copyright            : (C) 2015 Karolina Alexiou
    Email                : carolinegr at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssearchwidgetwrapper.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsfields.h"

#include <QWidget>

QList<QgsSearchWidgetWrapper::FilterFlag> QgsSearchWidgetWrapper::exclusiveFilterFlags()
{
  return QList<FilterFlag>()
         << EqualTo
         << NotEqualTo
         << GreaterThan
         << LessThan
         << GreaterThanOrEqualTo
         << LessThanOrEqualTo
         << Between
         << IsNotBetween
         << Contains
         << DoesNotContain
         << IsNull
         << IsNotNull
         << StartsWith
         << EndsWith;
}

QList<QgsSearchWidgetWrapper::FilterFlag> QgsSearchWidgetWrapper::nonExclusiveFilterFlags()
{
  return QList<FilterFlag>()
         << CaseInsensitive;
}

QString QgsSearchWidgetWrapper::toString( QgsSearchWidgetWrapper::FilterFlag flag )
{
  switch ( flag )
  {
    case EqualTo:
      return QObject::tr( "Equal to (=)" );
    case NotEqualTo:
      return QObject::tr( "Not equal to (≠)" );
    case GreaterThan:
      return QObject::tr( "Greater than (>)" );
    case LessThan:
      return QObject::tr( "Less than (<)" );
    case GreaterThanOrEqualTo:
      return QObject::tr( "Greater than or equal to (≥)" );
    case LessThanOrEqualTo:
      return QObject::tr( "Less than or equal to (≤)" );
    case Between:
      return QObject::tr( "Between (inclusive)" );
    case IsNotBetween:
      return QObject::tr( "Not between (inclusive)" );
    case CaseInsensitive:
      return QObject::tr( "Case insensitive" );
    case Contains:
      return QObject::tr( "Contains" );
    case DoesNotContain:
      return QObject::tr( "Does not contain" );
    case IsNull:
      return QObject::tr( "Is missing (null)" );
    case IsNotNull:
      return QObject::tr( "Is not missing (not null)" );
    case StartsWith:
      return QObject::tr( "Starts with" );
    case EndsWith:
      return QObject::tr( "Ends with" );
  }
  return QString();
}

QgsSearchWidgetWrapper::QgsSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsWidgetWrapper( vl, nullptr, parent )
  , mExpression( QString() )
  , mFieldIdx( fieldIdx )
{
}

QgsSearchWidgetWrapper::FilterFlags QgsSearchWidgetWrapper::supportedFlags() const
{
  return EqualTo;
}

QgsSearchWidgetWrapper::FilterFlags QgsSearchWidgetWrapper::defaultFlags() const
{
  return FilterFlags();
}

QString QgsSearchWidgetWrapper::createFieldIdentifier() const
{
  QString field = QgsExpression::quotedColumnRef( layer()->fields().at( mFieldIdx ).name() );
  if ( mAggregate.isEmpty() )
    return field;
  else
    return QStringLiteral( "relation_aggregate('%1','%2',%3)" ).arg( context().relation().id(), mAggregate, field );
}

void QgsSearchWidgetWrapper::setFeature( const QgsFeature &feature )
{
  Q_UNUSED( feature )
}

void QgsSearchWidgetWrapper::clearExpression()
{
  mExpression = QStringLiteral( "TRUE" );
}

QString QgsSearchWidgetWrapper::aggregate() const
{
  return mAggregate;
}

void QgsSearchWidgetWrapper::setAggregate( const QString &aggregate )
{
  mAggregate = aggregate;
}

