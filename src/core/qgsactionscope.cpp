/***************************************************************************
  qgsactionscope.cpp - QgsActionScope

 ---------------------
 begin                : 1.11.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsactionscope.h"
#include "qgsexpressioncontext.h"

QgsActionScope::QgsActionScope()
  : mExpressionContextScope( nullptr )
{
}

QgsActionScope::QgsActionScope( const QString &id, const QString &title, const QString &description, const QgsExpressionContextScope &expressionContextScope )
  : mId( id )
  , mTitle( title )
  , mDescription( description )
  , mExpressionContextScope( expressionContextScope )
{
}

bool QgsActionScope::operator==( const QgsActionScope &other ) const
{
  return other.mId == mId;
}

QgsExpressionContextScope QgsActionScope::expressionContextScope() const
{
  return mExpressionContextScope;
}

void QgsActionScope::setExpressionContextScope( const QgsExpressionContextScope &expressionContextScope )
{
  mExpressionContextScope = expressionContextScope;
}

QString QgsActionScope::id() const
{
  return mId;
}

void QgsActionScope::setId( const QString &name )
{
  mId = name;
}

bool QgsActionScope::isValid() const
{
  return !mId.isNull();
}

QString QgsActionScope::title() const
{
  return mTitle;
}

void QgsActionScope::setTitle( const QString &title )
{
  mTitle = title;
}

QString QgsActionScope::description() const
{
  return mDescription;
}

void QgsActionScope::setDescription( const QString &description )
{
  mDescription = description;
}

uint qHash( const QgsActionScope &key, uint seed )
{
  uint hash = seed;

  hash |= qHash( key.expressionContextScope().variableNames().join( ',' ), seed );
  hash |= qHash( key.id(), seed );

  return hash;
}
