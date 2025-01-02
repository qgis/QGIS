/***************************************************************************
  qgsdefaultvalue.cpp

 ---------------------
 begin                : 19.9.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsdefaultvalue.h"
#include "moc_qgsdefaultvalue.cpp"

QgsDefaultValue::QgsDefaultValue( const QString &expression, bool applyOnUpdate )
  : mExpression( expression )
  , mApplyOnUpdate( applyOnUpdate )
{

}

bool QgsDefaultValue::operator==( const QgsDefaultValue &other ) const
{
  return mExpression == other.mExpression
         && mApplyOnUpdate == other.mApplyOnUpdate;
}

QString QgsDefaultValue::expression() const
{
  return mExpression;
}

void QgsDefaultValue::setExpression( const QString &expression )
{
  mExpression = expression;
}

bool QgsDefaultValue::applyOnUpdate() const
{
  return mApplyOnUpdate;
}

void QgsDefaultValue::setApplyOnUpdate( bool applyOnUpdate )
{
  mApplyOnUpdate = applyOnUpdate;
}

bool QgsDefaultValue::isValid() const
{
  return !mExpression.isEmpty();
}

QgsDefaultValue::operator bool() const
{
  return !mExpression.isEmpty();
}
