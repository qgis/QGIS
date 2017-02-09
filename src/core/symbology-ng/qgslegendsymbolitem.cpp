/***************************************************************************
  qgslegendsymbolitem.cpp
  --------------------------------------
  Date                 : August 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslegendsymbolitem.h"

#include "qgssymbol.h"

QgsLegendSymbolItem::QgsLegendSymbolItem()
    : mSymbol( nullptr )
    , mCheckable( false )
    , mOriginalSymbolPointer( nullptr )
    , mScaleMinDenom( -1 )
    , mScaleMaxDenom( -1 )
    , mLevel( 0 )
{
}

QgsLegendSymbolItem::QgsLegendSymbolItem( QgsSymbol* symbol, const QString& label, const QString& ruleKey, bool checkable, int scaleMinDenom, int scaleMaxDenom, int level, const QString& parentRuleKey )
    : mSymbol( symbol ? symbol->clone() : nullptr )
    , mLabel( label )
    , mKey( ruleKey )
    , mCheckable( checkable )
    , mOriginalSymbolPointer( symbol )
    , mScaleMinDenom( scaleMinDenom )
    , mScaleMaxDenom( scaleMaxDenom )
    , mLevel( level )
    , mParentKey( parentRuleKey )
{
}

QgsLegendSymbolItem::QgsLegendSymbolItem( const QgsLegendSymbolItem& other )
    : mSymbol( nullptr )
    , mOriginalSymbolPointer( nullptr )
{
  *this = other;
}

QgsLegendSymbolItem::~QgsLegendSymbolItem()
{
  delete mSymbol;
}

QgsLegendSymbolItem& QgsLegendSymbolItem::operator=( const QgsLegendSymbolItem & other )
{
  if ( this == &other )
    return *this;

  setSymbol( other.mSymbol );
  mLabel = other.mLabel;
  mKey = other.mKey;
  mCheckable = other.mCheckable;
  mOriginalSymbolPointer = other.mOriginalSymbolPointer;
  mScaleMinDenom = other.mScaleMinDenom;
  mScaleMaxDenom = other.mScaleMaxDenom;
  mLevel = other.mLevel;
  mParentKey = other.mParentKey;

  return *this;
}

bool QgsLegendSymbolItem::isScaleOK( double scale ) const
{
  if ( scale <= 0 )
    return true;
  if ( mScaleMinDenom <= 0 && mScaleMaxDenom <= 0 )
    return true;
  if ( mScaleMinDenom > 0 && mScaleMinDenom > scale )
    return false;
  if ( mScaleMaxDenom > 0 && mScaleMaxDenom < scale )
    return false;
  return true;
}

void QgsLegendSymbolItem::setSymbol( QgsSymbol* s )
{
  delete mSymbol;
  mSymbol = s ? s->clone() : nullptr;
  mOriginalSymbolPointer = s;
}
