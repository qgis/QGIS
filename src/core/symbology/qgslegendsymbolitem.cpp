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

#include "qgsdatadefinedsizelegend.h"
#include "qgssymbol.h"

QgsLegendSymbolItem::QgsLegendSymbolItem( QgsSymbol *symbol, const QString &label, const QString &ruleKey, bool checkable, int scaleMinDenom, int scaleMaxDenom, int level, const QString &parentRuleKey )
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

QgsLegendSymbolItem::QgsLegendSymbolItem( const QgsLegendSymbolItem &other )
{
  *this = other;
}

QgsLegendSymbolItem::~QgsLegendSymbolItem()
{
  delete mSymbol;
  delete mDataDefinedSizeLegendSettings;
}

QgsLegendSymbolItem &QgsLegendSymbolItem::operator=( const QgsLegendSymbolItem &other )
{
  if ( this == &other )
    return *this;

  delete mSymbol;
  mSymbol = other.mSymbol ? other.mSymbol->clone() : nullptr;
  mLabel = other.mLabel;
  mKey = other.mKey;
  mCheckable = other.mCheckable;
  delete mDataDefinedSizeLegendSettings;
  mDataDefinedSizeLegendSettings = other.mDataDefinedSizeLegendSettings ? new QgsDataDefinedSizeLegend( *other.mDataDefinedSizeLegendSettings ) : nullptr;
  mOriginalSymbolPointer = other.mOriginalSymbolPointer;
  mScaleMinDenom = other.mScaleMinDenom;
  mScaleMaxDenom = other.mScaleMaxDenom;
  mLevel = other.mLevel;
  mParentKey = other.mParentKey;
  mUserData = other.mUserData;

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

void QgsLegendSymbolItem::setSymbol( QgsSymbol *s )
{
  delete mSymbol;
  mSymbol = s ? s->clone() : nullptr;
  mOriginalSymbolPointer = s;
}

void QgsLegendSymbolItem::setDataDefinedSizeLegendSettings( QgsDataDefinedSizeLegend *settings )
{
  delete mDataDefinedSizeLegendSettings;
  mDataDefinedSizeLegendSettings = settings;
}

QgsDataDefinedSizeLegend *QgsLegendSymbolItem::dataDefinedSizeLegendSettings() const
{
  return mDataDefinedSizeLegendSettings;
}

void QgsLegendSymbolItem::setUserData( int key, QVariant &value )
{
  mUserData.insert( key, value );
}

QVariant QgsLegendSymbolItem::userData( int key ) const
{
  return mUserData.value( key, QVariant() );
}
