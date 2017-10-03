/***************************************************************************
     qgsdetailedlistdata.cpp  -  A data represenation for a rich QItemData subclass
                             -------------------
    begin                : Sat May 17 2008
    copyright            : (C) 2008 Tim Sutton
    email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdetaileditemdata.h"

void QgsDetailedItemData::setTitle( const QString &title )
{
  mTitle = title;
}

void QgsDetailedItemData::setDetail( const QString &detail )
{
  mDetail = detail;
}

void QgsDetailedItemData::setCategory( const QString &category )
{
  mCategory = category;
}

void QgsDetailedItemData::setIcon( const QPixmap &icon )
{
  mPixmap = icon;
}

void QgsDetailedItemData::setCheckable( const bool flag )
{
  mCheckableFlag = flag;
}

void QgsDetailedItemData::setChecked( const bool flag )
{
  mCheckedFlag = flag;
}

void QgsDetailedItemData::setRenderAsWidget( const bool flag )
{
  mRenderAsWidgetFlag = flag;
}

QString QgsDetailedItemData::title() const
{
  return mTitle;
}

QString QgsDetailedItemData::detail() const
{
  return mDetail;
}

QString QgsDetailedItemData::category() const
{
  return mCategory;
}

QPixmap QgsDetailedItemData::icon() const
{
  return mPixmap;
}

bool QgsDetailedItemData::isCheckable() const
{
  return mCheckableFlag;
}

bool QgsDetailedItemData::isChecked() const
{
  return mCheckedFlag;
}

bool QgsDetailedItemData::isRenderedAsWidget() const
{
  return mRenderAsWidgetFlag;
}

void QgsDetailedItemData::setEnabled( bool flag )
{
  mEnabledFlag = flag;
}

bool QgsDetailedItemData::isEnabled() const
{
  return mEnabledFlag;
}
