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
QgsDetailedItemData::QgsDetailedItemData()
    : mCheckableFlag( false )
    , mCheckedFlag( false )
    , mEnabledFlag( true )
    , mRenderAsWidgetFlag( false )
{

}

void QgsDetailedItemData::setTitle( const QString& theTitle )
{
  mTitle = theTitle;
}

void QgsDetailedItemData::setDetail( const QString& theDetail )
{
  mDetail = theDetail;
}

void QgsDetailedItemData::setCategory( const QString& theCategory )
{
  mCategory = theCategory;
}

void QgsDetailedItemData::setIcon( const QPixmap& theIcon )
{
  mPixmap = theIcon;
}

void QgsDetailedItemData::setCheckable( const bool theFlag )
{
  mCheckableFlag = theFlag;
}

void QgsDetailedItemData::setChecked( const bool theFlag )
{
  mCheckedFlag = theFlag;
}

void QgsDetailedItemData::setRenderAsWidget( const bool theFlag )
{
  mRenderAsWidgetFlag = theFlag;
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

void QgsDetailedItemData::setEnabled( bool theFlag )
{
  mEnabledFlag = theFlag;
}

bool QgsDetailedItemData::isEnabled() const
{
  return mEnabledFlag;
}
